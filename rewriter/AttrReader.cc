#include "rewriter/AttrReader.h"
#include "ast/Helpers.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/util/Util.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

bool isTNilableOrUntyped(const ast::ExpressionPtr &expr) {
    auto send = ast::cast_tree<ast::Send>(expr);
    return send != nullptr && (send->fun == core::Names::nilable() || send->fun == core::Names::untyped()) &&
           ast::MK::isTApproximate(send->recv);
}

ast::Send *findReturnSpecSend(ast::Send *sharedSig) {
    ENFORCE(ASTUtil::castSig(sharedSig), "We weren't given a send node that's a valid signature");

    auto *block = sharedSig->block();
    auto body = ast::cast_tree<ast::Send>(block->body);

    while (body->fun != core::Names::returns() && body->fun != core::Names::void_()) {
        body = ast::cast_tree<ast::Send>(body->recv);
    }

    return body;
}

ast::Send *findReturnsSend(ast::Send *sig) {
    auto body = findReturnSpecSend(sig);

    return body->fun == core::Names::returns() ? body : nullptr;
}

bool hasNilableOrUntypedReturns(ast::ExpressionPtr &sharedSig) {
    auto *sig = ASTUtil::castSig(sharedSig);

    ENFORCE(sig != nullptr, "We weren't given a send node that's a valid signature");

    auto *body = findReturnsSend(sig);

    ENFORCE(body->fun == core::Names::returns());
    if (body->numPosArgs() != 1) {
        return false;
    }
    return isTNilableOrUntyped(body->getPosArg(0));
}

ast::ExpressionPtr dupReturnsType(ast::Send *sharedSig) {
    auto *sig = ASTUtil::castSig(sharedSig);

    ENFORCE(sig != nullptr, "We weren't given a send node that's a valid signature");

    auto *body = findReturnsSend(sig);

    ENFORCE(body->fun == core::Names::returns());
    if (body->numPosArgs() != 1) {
        return nullptr;
    }
    return body->getPosArg(0).deepCopy();
}

// This will raise an error if we've given a type that's not what we want
void ensureSafeSig(core::MutableContext ctx, const core::NameRef attrFun, ast::Send *sig) {
    // Loop down the chain of recv's until we get to the inner 'sig' node.
    auto *block = sig->block();
    auto body = ast::cast_tree<ast::Send>(block->body);
    auto cur = body;
    while (cur != nullptr) {
        if (cur->fun == core::Names::typeParameters()) {
            if (auto e = ctx.beginIndexerError(sig->loc, core::errors::Rewriter::BadAttrType)) {
                e.setHeader("The type for an `{}` cannot contain `{}`", attrFun.show(ctx), "type_parameters");
            }
            auto &arg = body->getPosArg(0);
            arg = ast::MK::Untyped(arg.loc());
        }
        cur = ast::cast_tree<ast::Send>(cur->recv);
    }
}

// To convert a sig into a writer sig with argument `name`, we copy the `returns(...)`
// value into the `sig {params(...)}` using whatever name we have for the setter.
ast::ExpressionPtr toWriterSigForName(ast::Send *sharedSig, const core::NameRef name, core::LocOffsets nameLoc) {
    ENFORCE(ASTUtil::castSig(sharedSig), "We weren't given a send node that's a valid signature");

    // There's a bit of work here because deepCopy gives us back an Expression when we know it's a Send.
    ast::ExpressionPtr sigExpr = sharedSig->deepCopy();
    auto sig = ast::cast_tree<ast::Send>(sigExpr);
    ENFORCE(sig != nullptr, "Just deep copied this, so it should be non-null");

    auto *body = findReturnsSend(sig);

    ENFORCE(body->fun == core::Names::returns());
    if (body->numPosArgs() != 1) {
        return nullptr;
    }
    ast::ExpressionPtr resultType = body->getPosArg(0).deepCopy();
    ast::Send *cur = body;
    while (cur != nullptr) {
        auto recv = ast::cast_tree<ast::ConstantLit>(cur->recv);
        if ((cur->recv.isSelfReference()) || (recv && recv->symbol() == core::Symbols::Sorbet())) {
            auto loc = resultType.loc();
            // These will be kwargs for the `param` call, given `numPosArgs` below.
            auto paramArgs = ast::MK::SendArgs(ast::MK::Symbol(nameLoc, name), move(resultType));
            const auto numPosArgs = 0;
            auto params = ast::MK::Send(loc, move(cur->recv), core::Names::params(), loc.copyWithZeroLength(),
                                        numPosArgs, move(paramArgs));
            cur->recv = move(params);
            break;
        }

        cur = ast::cast_tree<ast::Send>(cur->recv);
    }
    return sigExpr;
}
} // namespace

// Converts something like
//
//     sig {returns(String)}
//     attr_accessor :foo, :bar
//
// Into something like
//
//     sig {returns(String)}                  (1)
//     def foo; @foo; end
//     sig {params(foo: String).returns(String)}     (2)
//     def foo=(foo); @foo = foo; end
//
//     sig {returns(String)}                  (3)
//     def bar; @bar; end
//     sig {params(bar: String).returns(String)}     (4)
//     def bar=(bar); @bar = bar; end
//
// We have to do a bit of work, because the one `sig` we have will have to be
// duplicated onto all but the first synthesized method. For example, sig (1)
// above will actually be untouched in the syntax tree, but (2), (3), and (4)
// will have to be synthesized. Handling this case gets a little tricky
// considering that this Rewriter pass handles all three of attr_reader,
// attr_writer, and attr_accessor. Also the `sig` might contain an
// explicit checked (or on_failure) call as in
// `sig {returns(String}.checked(:always)}` that needs to be preserved.
//
// Also note that the burden is on the user to provide an accurate type signature.
// All attr_accessor's should probably have `T.nilable(...)` to account for a
// read-before-write.
vector<ast::ExpressionPtr> AttrReader::run(core::MutableContext ctx, ast::Send *send, ast::ExpressionPtr *prevStat) {
    vector<ast::ExpressionPtr> empty;

    if (ctx.state.cacheSensitiveOptions.runningUnderAutogen) {
        return empty;
    }

    bool makeReader = false;
    bool makeWriter = false;
    if (send->fun == core::Names::attr() || send->fun == core::Names::attrReader() ||
        send->fun == core::Names::attrAccessor()) {
        makeReader = true;
    }
    if (send->fun == core::Names::attrWriter() || send->fun == core::Names::attrAccessor()) {
        makeWriter = true;
    }
    if (!makeReader && !makeWriter) {
        return empty;
    }

    auto loc = send->loc;
    vector<ast::ExpressionPtr> stats;

    ast::Send *sig = nullptr;
    if (prevStat) {
        sig = ASTUtil::castSig(*prevStat);
        if (sig != nullptr) {
            auto ret = findReturnSpecSend(sig);
            ENFORCE(ret->fun == core::Names::void_() || ret->fun == core::Names::returns());
            if (ret->fun == core::Names::void_()) {
                sig = nullptr;
                if (makeReader) {
                    if (auto e = ctx.beginIndexerError(ret->loc, core::errors::Rewriter::VoidAttrReader)) {
                        auto what = makeWriter ? "attr_accessor" : "attr_reader";
                        e.setHeader("An `{}` cannot be `{}`", what, "void");
                    }
                }
            } else {
                ensureSafeSig(ctx, send->fun, sig);
            }
        }
    }

    bool declareIvars = false;
    if (sig != nullptr && hasNilableOrUntypedReturns(*prevStat)) {
        declareIvars = true;
    }

    bool usedPrevSig = false;

    if (makeReader) {
        for (auto &arg : send->posArgs()) {
            auto [name, argLoc] = ASTUtil::getAttrName(ctx, send->fun, arg);
            if (!name.exists()) {
                return empty;
            }
            core::NameRef varName = name.addAt(ctx);

            if (sig != nullptr) {
                if (usedPrevSig) {
                    stats.emplace_back(sig->deepCopy());
                } else {
                    usedPrevSig = true;
                }
            }

            ast::MethodDef::Flags flags;
            flags.isAttrBestEffortUIOnly = true;
            auto reader = ast::MK::Method0(loc, loc, name, ast::MK::Instance(argLoc, varName), flags);
            stats.emplace_back(std::move(reader));
        }
    }

    if (makeWriter) {
        for (auto &arg : send->posArgs()) {
            auto [name, argLoc] = ASTUtil::getAttrName(ctx, send->fun, arg);
            if (!name.exists()) {
                return empty;
            }

            core::NameRef varName = name.addAt(ctx);
            core::NameRef setName = name.addEq(ctx);

            if (sig != nullptr) {
                if (usedPrevSig) {
                    auto writerSig = toWriterSigForName(sig, name, argLoc);
                    if (!writerSig) {
                        return empty;
                    }
                    stats.emplace_back(move(writerSig));
                } else {
                    usedPrevSig = true;
                }
            }

            ast::ExpressionPtr body;
            if (declareIvars) {
                body = ast::MK::Assign(loc, ast::MK::Instance(argLoc, varName),
                                       ast::MK::Let(loc, ast::MK::Local(loc, name), dupReturnsType(sig)));
            } else {
                body = ast::MK::Assign(loc, ast::MK::Instance(argLoc, varName), ast::MK::Local(loc, name));
            }
            ast::MethodDef::Flags flags;
            flags.isAttrBestEffortUIOnly = true;
            stats.emplace_back(ast::MK::Method1(loc, loc, setName, ast::MK::Local(argLoc, name), move(body), flags));
        }
    }

    return stats;
}

}; // namespace sorbet::rewriter
