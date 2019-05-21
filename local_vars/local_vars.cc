#include "local_vars.h"
#include "ast/treemap/treemap.h"
#include "common/typecase.h"
#include "core/core.h"
#include "core/errors/namer.h"

using namespace std;

namespace sorbet::local_vars {

class LocalNameInserter {
    friend class LocalVars;

    struct NamedArg {
        core::NameRef name;
        core::LocalVariable local;
        core::Loc loc;
        unique_ptr<ast::Reference> expr;
    };

    // Map through the reference structure, naming the locals, and preserving
    // the outer structure for the namer proper.
    NamedArg nameArg(core::MutableContext ctx, unique_ptr<ast::Reference> arg) {
        NamedArg named;

        typecase(
            arg.get(),
            [&](ast::UnresolvedIdent *nm) {
                named.name = nm->name;
                named.local = enterLocal(ctx, named.name);
                named.loc = arg->loc;
                named.expr = make_unique<ast::Local>(arg->loc, named.local);
            },
            [&](ast::RestArg *rest) {
                named = nameArg(ctx, move(rest->expr));
                named.expr = make_unique<ast::RestArg>(arg->loc, move(named.expr));
            },
            [&](ast::KeywordArg *kw) {
                named = nameArg(ctx, move(kw->expr));
                named.expr = make_unique<ast::KeywordArg>(arg->loc, move(named.expr));
            },
            [&](ast::OptionalArg *opt) {
                named = nameArg(ctx, move(opt->expr));
                named.expr = make_unique<ast::OptionalArg>(arg->loc, move(named.expr), move(opt->default_));
            },
            [&](ast::BlockArg *blk) {
                named = nameArg(ctx, move(blk->expr));
                named.expr = make_unique<ast::BlockArg>(arg->loc, move(named.expr));
            },
            [&](ast::ShadowArg *shadow) {
                named = nameArg(ctx, move(shadow->expr));
                named.expr = make_unique<ast::ShadowArg>(arg->loc, move(named.expr));
            },
            [&](ast::Local *local) {
                named.name = local->localVariable._name;
                named.local = enterLocal(ctx, named.name);
                named.loc = arg->loc;
                named.expr = make_unique<ast::Local>(local->loc, named.local);
            });

        return named;
    }

    vector<NamedArg> nameArgs(core::MutableContext ctx, ast::MethodDef::ARGS_store &methodArgs) {
        vector<NamedArg> namedArgs;
        for (auto &arg : methodArgs) {
            auto *refExp = ast::cast_tree<ast::Reference>(arg.get());
            if (!refExp) {
                Exception::raise("Must be a reference!");
            }
            unique_ptr<ast::Reference> refExpImpl(refExp);
            arg.release();
            namedArgs.emplace_back(nameArg(ctx, move(refExpImpl)));
        }

        return namedArgs;
    }

    struct LocalFrame {
        UnorderedMap<core::NameRef, core::LocalVariable> locals;
        vector<core::LocalVariable> args;
        u4 localId;
        std::optional<u4> oldBlockCounter;
    };

    LocalFrame &enterBlock() {
        auto &frame = scopeStack.emplace_back();
        frame.localId = blockCounter;
        ++blockCounter;
        return frame;
    }

    LocalFrame &enterClassOrMethod() {
        auto &frame = scopeStack.emplace_back();
        frame.localId = 0;
        frame.oldBlockCounter = blockCounter;
        blockCounter = 1;
        return frame;
    }

    void exitScope() {
        auto &oldScopeCounter = scopeStack.back().oldBlockCounter;
        if (oldScopeCounter) {
            blockCounter = *oldScopeCounter;
        }
        scopeStack.pop_back();
    }

    vector<LocalFrame> scopeStack;
    // The purpose of this counter is to ensure that every block within a method/class has a unique scope id.
    // For example, a possible assignment of ids is the following:
    //
    // [].map { # $0 }
    // class A
    //   [].each { # $0 }
    //   [].map { # $1 }
    // end
    // [].each { # $1 }
    // def foo
    //   [].each { # $0 }
    //   [].map { # $1 }
    // end
    // [].each { # $2 }
    u4 blockCounter;

    core::LocalVariable enterLocal(core::MutableContext ctx, core::NameRef name) {
        if (!ctx.owner.data(ctx)->isBlockSymbol(ctx)) {
            return core::LocalVariable(name, 0);
        }
        return core::LocalVariable(name, scopeStack.back().localId);
    }

    // Enter names from arguments into the current frame, building a new
    // argument list back up for the original context.
    ast::MethodDef::ARGS_store fillInArgs(core::MutableContext ctx, vector<NamedArg> namedArgs) {
        ast::MethodDef::ARGS_store args;

        for (auto &named : namedArgs) {
            args.emplace_back(move(named.expr));
            auto frame = scopeStack.back();
            scopeStack.back().locals[named.name] = named.local;
            scopeStack.back().args.emplace_back(named.local);
        }

        return args;
    }

    core::SymbolRef methodOwner(core::MutableContext ctx) {
        core::SymbolRef owner = ctx.owner.data(ctx)->enclosingClass(ctx);
        if (owner == core::Symbols::root()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        return owner;
    }

public:
    unique_ptr<ast::ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> klass) {
        enterClassOrMethod();
        return klass;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> klass) {
        exitScope();
        return klass;
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> method) {
        enterClassOrMethod();

        auto namedArgs = nameArgs(ctx, method->args);
        method->args = fillInArgs(ctx.withOwner(method->symbol), move(namedArgs));
        return method;
    }

    unique_ptr<ast::MethodDef> postTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> method) {
        exitScope();
        return method;
    }

    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> original) {
        if (original->args.size() == 1 && ast::isa_tree<ast::ZSuperArgs>(original->args[0].get())) {
            original->args.clear();
            core::SymbolRef method = ctx.owner.data(ctx)->enclosingMethod(ctx);
            if (method.data(ctx)->isMethod()) {
                for (auto arg : scopeStack.back().args) {
                    original->args.emplace_back(make_unique<ast::Local>(original->loc, arg));
                }
            } else {
                if (auto e = ctx.state.beginError(original->loc, core::errors::Namer::SelfOutsideClass)) {
                    e.setHeader("`{}` outside of method", "super");
                }
            }
        }

        return original;
    }

    unique_ptr<ast::Block> preTransformBlock(core::MutableContext ctx, unique_ptr<ast::Block> blk) {
        auto outerArgs = scopeStack.back().args;
        auto &frame = enterBlock();
        frame.args = std::move(outerArgs);
        auto &parent = *(scopeStack.end() - 2);

        // We inherit our parent's locals
        for (auto &binding : parent.locals) {
            frame.locals.insert(binding);
        }

        // If any of our arguments shadow our parent, fillInArgs will overwrite
        // them in `frame.locals`
        auto namedArgs = nameArgs(ctx, blk->args);
        blk->args = fillInArgs(ctx.withOwner(blk->symbol), move(namedArgs));

        return blk;
    }

    unique_ptr<ast::Block> postTransformBlock(core::MutableContext ctx, unique_ptr<ast::Block> blk) {
        exitScope();
        return blk;
    }

    unique_ptr<ast::Expression> postTransformUnresolvedIdent(core::MutableContext ctx,
                                                             unique_ptr<ast::UnresolvedIdent> nm) {
        if (nm->kind == ast::UnresolvedIdent::Local) {
            auto &frame = scopeStack.back();
            core::LocalVariable &cur = frame.locals[nm->name];
            if (!cur.exists()) {
                cur = enterLocal(ctx, nm->name);
                frame.locals[nm->name] = cur;
            }
            return make_unique<ast::Local>(nm->loc, cur);
        } else {
            return nm;
        }
    }

private:
    LocalNameInserter() : blockCounter(0) {
        enterBlock();
    }
};

unique_ptr<ast::Expression> LocalVars::run(core::MutableContext ctx, unique_ptr<ast::Expression> tree) {
    LocalNameInserter localNameInserter;
    return ast::TreeMap::apply(ctx, localNameInserter, move(tree));
}

} // namespace sorbet::local_vars
