#include "rewriter/AttrReader.h"
#include "absl/strings/escaping.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/Util.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

pair<core::NameRef, core::LocOffsets> getName(core::MutableContext ctx, ast::ExpressionPtr &name) {
    core::LocOffsets loc;
    core::NameRef res;
    if (auto *lit = ast::cast_tree<ast::Literal>(name)) {
        if (lit->isSymbol()) {
            res = lit->asSymbol();
            loc = lit->loc;
            ENFORCE(ctx.locAt(loc).exists());
            ENFORCE(ctx.locAt(loc).source(ctx).value().size() > 1 && ctx.locAt(loc).source(ctx).value()[0] == ':');
            loc = core::LocOffsets{loc.beginPos() + 1, loc.endPos()};
        } else if (lit->isString()) {
            core::NameRef nameRef = lit->asString();
            auto shortName = nameRef.shortName(ctx);
            bool validAttr = (isalpha(shortName.front()) || shortName.front() == '_') &&
                             absl::c_all_of(shortName, [](char c) { return isalnum(c) || c == '_'; });
            if (validAttr) {
                res = nameRef;
            } else {
                if (auto e = ctx.beginError(name.loc(), core::errors::Rewriter::BadAttrArg)) {
                    e.setHeader("Bad attribute name \"{}\"", absl::CEscape(shortName));
                }
                res = core::Names::empty();
            }
            loc = lit->loc;
            DEBUG_ONLY({
                auto l = ctx.locAt(loc);
                ENFORCE(l.exists());
                auto source = l.source(ctx).value();
                ENFORCE(source.size() > 2);
                ENFORCE(source[0] == '"' || source[0] == '\'');
                auto lastChar = source[source.size() - 1];
                ENFORCE(lastChar == '"' || lastChar == '\'');
            });
            loc = core::LocOffsets{loc.beginPos() + 1, loc.endPos() - 1};
        }
    }
    if (!res.exists()) {
        if (auto e = ctx.beginError(name.loc(), core::errors::Rewriter::BadAttrArg)) {
            e.setHeader("arg must be a Symbol or String");
        }
    }
    return make_pair(res, loc);
}

// these helpers work on a purely syntactic level. for instance, this function determines if an expression is `T`,
// either with no scope or with the root scope (i.e. `::T`). this might not actually refer to the `T` that we define for
// users, but we don't know that information in the Rewriter passes.
bool isT(const ast::ExpressionPtr &expr) {
    auto *t = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    if (t == nullptr || t->cnst != core::Names::Constants::T()) {
        return false;
    }
    return ast::MK::isRootScope(t->scope);
}

bool isTNilableOrUntyped(const ast::ExpressionPtr &expr) {
    auto *send = ast::cast_tree<ast::Send>(expr);
    return send != nullptr && (send->fun == core::Names::nilable() || send->fun == core::Names::untyped()) &&
           isT(send->recv);
}

ast::Send *findSendReturns(ast::Send *sharedSig) {
    ENFORCE(ASTUtil::castSig(sharedSig), "We weren't given a send node that's a valid signature");

    auto *block = sharedSig->block();
    auto body = ast::cast_tree<ast::Send>(block->body);

    while (body->fun != core::Names::returns() && body->fun != core::Names::void_()) {
        body = ast::cast_tree<ast::Send>(body->recv);
    }

    return body->fun == core::Names::returns() ? body : nullptr;
}

bool hasNilableOrUntypedReturns(ast::ExpressionPtr &sharedSig) {
    ENFORCE(ASTUtil::castSig(sharedSig), "We weren't given a send node that's a valid signature");

    auto *body = findSendReturns(ASTUtil::castSig(sharedSig));

    ENFORCE(body->fun == core::Names::returns());
    if (body->numPosArgs() != 1) {
        return false;
    }
    return isTNilableOrUntyped(body->getPosArg(0));
}

ast::ExpressionPtr dupReturnsType(ast::Send *sharedSig) {
    ENFORCE(ASTUtil::castSig(sharedSig), "We weren't given a send node that's a valid signature");

    auto *body = findSendReturns(ASTUtil::castSig(sharedSig));

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
    auto *body = ast::cast_tree<ast::Send>(block->body);
    auto *cur = body;
    while (cur != nullptr) {
        if (cur->fun == core::Names::typeParameters()) {
            if (auto e = ctx.beginError(sig->loc, core::errors::Rewriter::BadAttrType)) {
                e.setHeader("The type for an `{}` cannot contain `{}`", attrFun.show(ctx), "type_parameters");
            }
            auto &arg = body->getPosArg(0);
            arg = ast::MK::Untyped(arg.loc());
        }
        cur = ast::cast_tree<ast::Send>(cur->recv);
    }
}

ast::Send *findSendChecked(ast::Send *sharedSig) {
    ENFORCE(ASTUtil::castSig(sharedSig), "We weren't given a send node that's a valid signature");

    auto *block = sharedSig->block();
    auto body = ast::cast_tree<ast::Send>(block->body);

    while (body != nullptr && body->fun != core::Names::checked()) {
        body = ast::cast_tree<ast::Send>(body->recv);
    }

    return body;
}

// Heuristic to check if user-provided sig adds no runtime checking on top of an attr_reader.
//
// A user-provided sig causes an attr_reader method to behave differently from a normal attr_reader
// method at runtime.
//
// If the answer would be "maybe adds runtime checking, but hard to tell," we answer that it does add
// checking to be safe. Thus, the naming of this method is important: we can't rename this method to
// `sigIsChecked` and negate all true/false. (Put another way: double negation elimination doesn't
// apply here).
bool sigIsUnchecked(core::MutableContext ctx, ast::Send *sig) {
    // No sig? Then definitely not checked at runtime.
    if (sig == nullptr) {
        return true;
    }

    auto checked = findSendChecked(sig);
    if (checked == nullptr || checked->numPosArgs() != 1) {
        // Unknown: default to false
        return false;
    }

    auto lit = ast::cast_tree<ast::Literal>(checked->getPosArg(0));
    if (lit == nullptr || !lit->isSymbol()) {
        // Unknown: default to false
        return false;
    }

    // Treats `.checked(:tests)` as unknown, therefore not unchecked.
    // Also treats `.checked(:compiled)` as unknown, therefore not unchecked.
    return lit->asSymbol() == core::Names::never();
}

// To convert a sig into a writer sig with argument `name`, we copy the `returns(...)`
// value into the `sig {params(...)}` using whatever name we have for the setter.
ast::ExpressionPtr toWriterSigForName(ast::Send *sharedSig, const core::NameRef name, core::LocOffsets nameLoc) {
    ENFORCE(ASTUtil::castSig(sharedSig), "We weren't given a send node that's a valid signature");

    // There's a bit of work here because deepCopy gives us back an Expression when we know it's a Send.
    ast::ExpressionPtr sigExpr = sharedSig->deepCopy();
    auto *sig = ast::cast_tree<ast::Send>(sigExpr);
    ENFORCE(sig != nullptr, "Just deep copied this, so it should be non-null");

    auto *body = findSendReturns(sig);

    ENFORCE(body->fun == core::Names::returns());
    if (body->numPosArgs() != 1) {
        return nullptr;
    }
    ast::ExpressionPtr resultType = body->getPosArg(0).deepCopy();
    ast::Send *cur = body;
    while (cur != nullptr) {
        auto recv = ast::cast_tree<ast::ConstantLit>(cur->recv);
        if ((cur->recv.isSelfReference()) || (recv && recv->symbol == core::Symbols::Sorbet())) {
            auto loc = resultType.loc();
            auto params = ast::MK::Send0(loc, move(cur->recv), core::Names::params(), loc.copyWithZeroLength());
            ast::cast_tree_nonnull<ast::Send>(params).addKwArg(ast::MK::Symbol(nameLoc, name), move(resultType));
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

    if (ctx.state.runningUnderAutogen) {
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
        if (sig != nullptr && findSendReturns(sig) == nullptr) {
            sig = nullptr;
        } else if (sig != nullptr) {
            ensureSafeSig(ctx, send->fun, sig);
        }
    }

    bool declareIvars = false;
    if (sig != nullptr && hasNilableOrUntypedReturns(*prevStat)) {
        declareIvars = true;
    }

    bool usedPrevSig = false;

    if (makeReader) {
        const auto numPosArgs = send->numPosArgs();
        for (auto i = 0; i < numPosArgs; ++i) {
            auto &arg = send->getPosArg(i);
            auto [name, argLoc] = getName(ctx, arg);
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
            if (sigIsUnchecked(ctx, sig)) {
                flags.isAttrReader = true;
            }
            auto reader = ast::MK::SyntheticMethod0(loc, loc, name, ast::MK::Instance(argLoc, varName), flags);
            stats.emplace_back(std::move(reader));
        }
    }

    if (makeWriter) {
        const auto numPosArgs = send->numPosArgs();
        for (auto i = 0; i < numPosArgs; ++i) {
            auto &arg = send->getPosArg(i);
            auto [name, argLoc] = getName(ctx, arg);
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
                                       ast::MK::Let(loc, ast::MK::ResolvedLocal(loc, name), dupReturnsType(sig)));
            } else {
                body = ast::MK::Assign(loc, ast::MK::Instance(argLoc, varName), ast::MK::ResolvedLocal(loc, name));
            }
            ast::MethodDef::Flags flags;
            flags.isAttrBestEffortUIOnly = true;
            stats.emplace_back(
                ast::MK::SyntheticMethod1(loc, loc, setName, ast::MK::ResolvedLocal(argLoc, name), move(body), flags));
        }
    }

    return stats;
}

}; // namespace sorbet::rewriter
