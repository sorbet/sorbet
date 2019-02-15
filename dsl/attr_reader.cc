#include "dsl/attr_reader.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/dsl.h"
#include "dsl/dsl.h"

using namespace std;

namespace sorbet::dsl {

unique_ptr<ast::Expression> mkTUntyped(core::MutableContext ctx, core::Loc loc) {
    return ast::MK::Send0(loc, ast::MK::T(loc), core::Names::untyped());
}

pair<core::NameRef, core::Loc> getName(core::MutableContext ctx, ast::Expression *name) {
    core::Loc loc;
    core::NameRef res = core::NameRef::noName();
    if (auto lit = ast::cast_tree<ast::Literal>(name)) {
        if (lit->isSymbol(ctx)) {
            res = lit->asSymbol(ctx);
            loc = lit->loc;
            ENFORCE(loc.source(ctx).size() > 1 && loc.source(ctx)[0] == ':');
            loc = core::Loc(loc.file(), loc.beginPos() + 1, loc.endPos());
        } else if (lit->isString(ctx)) {
            res = lit->asString(ctx);
            loc = lit->loc;
        }
    }
    if (!res.exists()) {
        if (auto e = ctx.state.beginError(name->loc, core::errors::DSL::BadAttrArg)) {
            e.setHeader("arg must be a Symbol or String");
        }
    }
    return make_pair(res, loc);
}

// Slightly modified from TypeSyntax::isSig.
// We don't want to depend on resolver so that one day everything in the DSL pass can be standalone.
//
// There must be both a `sig` and a `returns` in order for isSig to be true.
bool isSig(const ast::Send *send) {
    if (send->fun != core::Names::sig()) {
        return false;
    }
    if (send->block.get() == nullptr) {
        return false;
    }
    if (send->args.size() != 0) {
        return false;
    }
    auto block = ast::cast_tree<ast::Block>(send->block.get());
    ENFORCE(block);
    auto body = ast::cast_tree<ast::Send>(block->body.get());
    if (!body) {
        return false;
    }
    if (body->fun != core::Names::returns()) {
        return false;
    }

    return true;
}

// To convert a sig into a writer sig with argument `name`, we copy the `returns(...)`
// value into the `sig {params(...)}` using whatever name we have for the setter.
//
// This change is done in place; it's assumed that the caller created a new sig for us.
unique_ptr<ast::Expression> toWriterSigForName(core::MutableContext ctx, const ast::Send *sharedSig,
                                               const core::NameRef name, core::Loc nameLoc) {
    ENFORCE(isSig(sharedSig), "We weren't given a send node that's a valid signature");

    // There's a bit of work here because deepCopy gives us back an Expression when we know it's a Send.
    unique_ptr<ast::Expression> sigExp = sharedSig->deepCopy();
    auto sigSend = ast::cast_tree<ast::Send>(sigExp.get());
    ENFORCE(sigSend, "Just deep copied this, so it should be non-null");
    unique_ptr<ast::Send> sig(sigSend);
    sigExp.release();

    // Loop down the chain of recv's until we get to the inner 'sig' node.
    auto block = ast::cast_tree<ast::Block>(sig->block.get());
    auto body = ast::cast_tree<ast::Send>(block->body.get());

    ENFORCE(body->fun == core::Names::returns());
    ENFORCE(body->args.size() == 1);
    unique_ptr<ast::Expression> resultType = body->args[0]->deepCopy();
    ast::Send *cur = body;
    while (cur != nullptr) {
        auto recv = ast::cast_tree<ast::ConstantLit>(cur->recv.get());
        if (ast::isa_tree<ast::Self>(cur->recv.get()) ||
            (recv && recv->typeAliasOrConstantSymbol() == core::Symbols::Sorbet())) {
            auto loc = resultType->loc;
            auto hash = ast::MK::Hash1(cur->loc, ast::MK::Symbol(nameLoc, name), move(resultType));
            auto params = ast::MK::Send1(loc, move(cur->recv), core::Names::params(), move(hash));
            cur->recv = move(params);
            break;
        }

        cur = ast::cast_tree<ast::Send>(cur->recv.get());
    }
    return sig;
}

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
// considering that this DSL pass handles all three of attr_reader,
// attr_writer, and attr_accessor.
//
// Also note that the burden is on the user to provide an accurate type signature.
// All attr_accessor's should probably have `T.nilable(...)` to account for a
// read-before-write.
vector<unique_ptr<ast::Expression>> AttrReader::replaceDSL(core::MutableContext ctx, ast::Send *send,
                                                           const ast::Expression *prevStat) {
    vector<unique_ptr<ast::Expression>> empty;

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
    vector<unique_ptr<ast::Expression>> stats;

    auto sig = ast::cast_tree_const<ast::Send>(prevStat);
    bool hasSig = sig && isSig(sig);

    bool usedPrevSig = false;

    if (makeReader) {
        for (auto &arg : send->args) {
            auto [name, argLoc] = getName(ctx, arg.get());
            if (!name.exists()) {
                return empty;
            }
            core::NameRef varName = name.addAt(ctx);

            if (hasSig) {
                ENFORCE(sig != nullptr);

                if (usedPrevSig) {
                    stats.emplace_back(sig->deepCopy());
                } else {
                    usedPrevSig = true;
                }
            }

            stats.emplace_back(
                ast::MK::Method0(loc, loc, name, ast::MK::Instance(argLoc, varName), ast::MethodDef::DSLSynthesized));
        }
    }

    if (makeWriter) {
        for (auto &arg : send->args) {
            auto [name, argLoc] = getName(ctx, arg.get());
            if (!name.exists()) {
                return empty;
            }

            core::NameRef varName = name.addAt(ctx);
            core::NameRef setName = name.addEq(ctx);

            if (hasSig) {
                ENFORCE(sig != nullptr);

                if (usedPrevSig) {
                    stats.emplace_back(toWriterSigForName(ctx, sig, name, argLoc));
                } else {
                    usedPrevSig = true;
                }
            }

            auto body = ast::MK::Assign(loc, ast::MK::Instance(argLoc, varName), ast::MK::Local(loc, name));
            stats.emplace_back(ast::MK::Method1(loc, loc, setName, ast::MK::Local(argLoc, name), move(body),
                                                ast::MethodDef::DSLSynthesized));
        }
    }

    return stats;
}

}; // namespace sorbet::dsl
