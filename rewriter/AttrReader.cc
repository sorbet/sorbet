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

pair<core::NameRef, core::LocOffsets> getName(core::MutableContext ctx, ast::TreePtr &name) {
    core::LocOffsets loc;
    core::NameRef res;
    if (auto *lit = ast::cast_tree<ast::Literal>(name)) {
        if (lit->isSymbol(ctx)) {
            res = lit->asSymbol(ctx);
            loc = lit->loc;
            ENFORCE(core::Loc(ctx.file, loc).source(ctx).size() > 1 && core::Loc(ctx.file, loc).source(ctx)[0] == ':');
            loc = core::LocOffsets{loc.beginPos() + 1, loc.endPos()};
        } else if (lit->isString(ctx)) {
            core::NameRef nameRef = lit->asString(ctx);
            auto shortName = nameRef.data(ctx)->shortName(ctx);
            bool validAttr = (isalpha(shortName.front()) || shortName.front() == '_') &&
                             absl::c_all_of(shortName, [](char c) { return isalnum(c) || c == '_'; });
            if (validAttr) {
                res = nameRef;
            } else {
                if (auto e = ctx.beginError(name->loc, core::errors::Rewriter::BadAttrArg)) {
                    e.setHeader("Bad attribute name \"{}\"", absl::CEscape(shortName));
                }
                res = core::Names::empty();
            }
            loc = lit->loc;
        }
    }
    if (!res.exists()) {
        if (auto e = ctx.beginError(name->loc, core::errors::Rewriter::BadAttrArg)) {
            e.setHeader("arg must be a Symbol or String");
        }
    }
    return make_pair(res, loc);
}

// these helpers work on a purely syntactic level. for instance, this function determines if an expression is `T`,
// either with no scope or with the root scope (i.e. `::T`). this might not actually refer to the `T` that we define for
// users, but we don't know that information in the Rewriter passes.
bool isT(const ast::TreePtr &expr) {
    auto *t = ast::cast_tree_const<ast::UnresolvedConstantLit>(expr);
    if (t == nullptr || t->cnst != core::Names::Constants::T()) {
        return false;
    }
    auto &scope = t->scope;
    if (ast::isa_tree<ast::EmptyTree>(scope)) {
        return true;
    }
    auto root = ast::cast_tree_const<ast::ConstantLit>(scope);
    return root != nullptr && root->symbol == core::Symbols::root();
}

bool isTNilable(const ast::TreePtr &expr) {
    auto *nilable = ast::cast_tree_const<ast::Send>(expr);
    return nilable != nullptr && nilable->fun == core::Names::nilable() && isT(nilable->recv);
}

bool hasNilableReturns(core::MutableContext ctx, ast::TreePtr &sharedSig) {
    ENFORCE(ASTUtil::castSig(sharedSig, core::Names::returns()),
            "We weren't given a send node that's a valid signature");

    auto &send = ast::cast_tree_nonnull<ast::Send>(sharedSig);
    auto &block = ast::cast_tree_nonnull<ast::Block>(send.block);
    auto &body = ast::cast_tree_nonnull<ast::Send>(block.body);

    ENFORCE(body.fun == core::Names::returns());
    if (body.args.size() != 1) {
        return false;
    }
    return isTNilable(body.args[0]);
}

ast::TreePtr dupReturnsType(core::MutableContext ctx, ast::Send *sharedSig) {
    ENFORCE(ASTUtil::castSig(sharedSig, core::Names::returns()),
            "We weren't given a send node that's a valid signature");

    auto block = ast::cast_tree_const<ast::Block>(sharedSig->block);
    auto body = ast::cast_tree_const<ast::Send>(block->body);

    ENFORCE(body->fun == core::Names::returns());
    if (body->args.size() != 1) {
        return nullptr;
    }
    return body->args[0]->deepCopy();
}

// This will raise an error if we've given a type that's not what we want
void ensureSafeSig(core::MutableContext ctx, const core::NameRef attrFun, ast::Send *sig) {
    // Loop down the chain of recv's until we get to the inner 'sig' node.
    auto *block = ast::cast_tree<ast::Block>(sig->block);
    auto *body = ast::cast_tree<ast::Send>(block->body);
    auto *cur = body;
    while (cur != nullptr) {
        if (cur->fun == core::Names::typeParameters()) {
            if (auto e = ctx.beginError(sig->loc, core::errors::Rewriter::BadAttrType)) {
                e.setHeader("The type for an `{}` cannot contain `{}`", attrFun.show(ctx), "type_parameters");
            }
            body->args[0] = ast::MK::Untyped(body->args[0]->loc);
        }
        cur = ast::cast_tree<ast::Send>(cur->recv);
    }
}

// To convert a sig into a writer sig with argument `name`, we copy the `returns(...)`
// value into the `sig {params(...)}` using whatever name we have for the setter.
ast::TreePtr toWriterSigForName(core::MutableContext ctx, ast::Send *sharedSig, const core::NameRef name,
                                core::LocOffsets nameLoc) {
    ENFORCE(ASTUtil::castSig(sharedSig, core::Names::returns()),
            "We weren't given a send node that's a valid signature");

    // There's a bit of work here because deepCopy gives us back an Expression when we know it's a Send.
    ast::TreePtr sigExpr = sharedSig->deepCopy();
    auto *sig = ast::cast_tree<ast::Send>(sigExpr);
    ENFORCE(sig != nullptr, "Just deep copied this, so it should be non-null");

    // Loop down the chain of recv's until we get to the inner 'sig' node.
    auto block = ast::cast_tree<ast::Block>(sig->block);
    auto body = ast::cast_tree<ast::Send>(block->body);

    ENFORCE(body->fun == core::Names::returns());
    if (body->args.size() != 1) {
        return nullptr;
    }
    ast::TreePtr resultType = body->args[0]->deepCopy();
    ast::Send *cur = body;
    while (cur != nullptr) {
        auto recv = ast::cast_tree<ast::ConstantLit>(cur->recv);
        if ((cur->recv->isSelfReference()) || (recv && recv->symbol == core::Symbols::Sorbet())) {
            auto loc = resultType->loc;
            auto hash = ast::MK::Hash1(cur->loc, ast::MK::Symbol(nameLoc, name), move(resultType));
            auto params = ast::MK::Send1(loc, move(cur->recv), core::Names::params(), move(hash));
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
// attr_writer, and attr_accessor.
//
// Also note that the burden is on the user to provide an accurate type signature.
// All attr_accessor's should probably have `T.nilable(...)` to account for a
// read-before-write.
vector<ast::TreePtr> AttrReader::run(core::MutableContext ctx, ast::Send *send, ast::TreePtr *prevStat) {
    vector<ast::TreePtr> empty;

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
    vector<ast::TreePtr> stats;

    ast::Send *sig = nullptr;
    if (prevStat) {
        sig = ASTUtil::castSig(*prevStat, core::Names::returns());
        if (sig != nullptr) {
            ensureSafeSig(ctx, send->fun, sig);
        }
    }

    bool declareIvars = false;
    if (sig != nullptr && hasNilableReturns(ctx, *prevStat)) {
        declareIvars = true;
    }

    bool usedPrevSig = false;

    if (makeReader) {
        for (auto &arg : send->args) {
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

            stats.emplace_back(
                ast::MK::SyntheticMethod0(loc, core::Loc(ctx.file, loc), name, ast::MK::Instance(argLoc, varName)));
        }
    }

    if (makeWriter) {
        for (auto &arg : send->args) {
            auto [name, argLoc] = getName(ctx, arg);
            if (!name.exists()) {
                return empty;
            }

            core::NameRef varName = name.addAt(ctx);
            core::NameRef setName = name.addEq(ctx);

            if (sig != nullptr) {
                if (usedPrevSig) {
                    auto writerSig = toWriterSigForName(ctx, sig, name, argLoc);
                    if (!writerSig) {
                        return empty;
                    }
                    stats.emplace_back(move(writerSig));
                } else {
                    usedPrevSig = true;
                }
            }

            ast::TreePtr body;
            if (declareIvars) {
                body = ast::MK::Assign(loc, ast::MK::Instance(argLoc, varName),
                                       ast::MK::Let(loc, ast::MK::Local(loc, name), dupReturnsType(ctx, sig)));
            } else {
                body = ast::MK::Assign(loc, ast::MK::Instance(argLoc, varName), ast::MK::Local(loc, name));
            }
            stats.emplace_back(ast::MK::SyntheticMethod1(loc, core::Loc(ctx.file, loc), setName,
                                                         ast::MK::Local(argLoc, name), move(body)));
        }
    }

    return stats;
}

}; // namespace sorbet::rewriter
