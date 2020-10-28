#include "local_vars.h"
#include "absl/strings/match.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "common/typecase.h"
#include "core/core.h"
#include "core/errors/namer.h"

using namespace std;

namespace sorbet::local_vars {

class LocalNameInserter {
    friend class LocalVars;

    struct ArgFlags {
        bool keyword : 1;
        bool block : 1;
        bool repeated : 1;
        bool shadow : 1;

        // In C++20 we can replace this with bit field initialzers
        ArgFlags() : keyword(false), block(false), repeated(false), shadow(false) {}

        bool isPositional() const {
            return !this->keyword && !this->block && !this->repeated && !this->shadow;
        }
        bool isKeyword() const {
            return this->keyword && !this->repeated;
        }
    };
    CheckSize(ArgFlags, 1, 1);

    struct NamedArg {
        core::NameRef name;
        core::LocalVariable local;
        core::LocOffsets loc;
        ast::TreePtr expr;
        ArgFlags flags;
    };

    // Map through the reference structure, naming the locals, and preserving
    // the outer structure for the namer proper.
    NamedArg nameArg(ast::TreePtr arg) {
        NamedArg named;

        typecase(
            arg.get(),
            [&](ast::UnresolvedIdent *nm) {
                named.name = nm->name;
                named.local = enterLocal(named.name);
                named.loc = arg->loc;
                named.expr = ast::make_tree<ast::Local>(arg->loc, named.local);
            },
            [&](ast::RestArg *rest) {
                named = nameArg(move(rest->expr));
                named.expr = ast::MK::RestArg(arg->loc, move(named.expr));
                named.flags.repeated = true;
            },
            [&](ast::KeywordArg *kw) {
                named = nameArg(move(kw->expr));
                named.expr = ast::MK::KeywordArg(arg->loc, move(named.expr));
                named.flags.keyword = true;
            },
            [&](ast::OptionalArg *opt) {
                named = nameArg(move(opt->expr));
                named.expr = ast::MK::OptionalArg(arg->loc, move(named.expr), move(opt->default_));
            },
            [&](ast::BlockArg *blk) {
                named = nameArg(move(blk->expr));
                named.expr = ast::MK::BlockArg(arg->loc, move(named.expr));
                named.flags.block = true;
            },
            [&](ast::ShadowArg *shadow) {
                named = nameArg(move(shadow->expr));
                named.expr = ast::MK::ShadowArg(arg->loc, move(named.expr));
                named.flags.shadow = true;
            },
            [&](ast::Local *local) {
                named.name = local->localVariable._name;
                named.local = enterLocal(named.name);
                named.loc = arg->loc;
                named.expr = ast::make_tree<ast::Local>(local->loc, named.local);
            });

        return named;
    }

    vector<NamedArg> nameArgs(core::MutableContext ctx, ast::MethodDef::ARGS_store &methodArgs) {
        vector<NamedArg> namedArgs;
        UnorderedSet<core::NameRef> nameSet;
        for (auto &arg : methodArgs) {
            if (!ast::isa_reference(arg)) {
                Exception::raise("Must be a reference!");
            }
            auto named = nameArg(move(arg));
            nameSet.insert(named.name);
            namedArgs.emplace_back(move(named));
        }

        return namedArgs;
    }

    struct LocalFrame {
        struct Arg {
            core::LocalVariable arg;
            ArgFlags flags;
        };
        UnorderedMap<core::NameRef, core::LocalVariable> locals;
        vector<Arg> args;
        std::optional<u4> oldBlockCounter = nullopt;
        u4 localId = 0;
        bool insideBlock = false;
        bool insideMethod = false;
    };

    LocalFrame &pushBlockFrame(bool insideMethod) {
        auto &frame = scopeStack.emplace_back();
        frame.localId = blockCounter;
        frame.insideBlock = true;
        frame.insideMethod = insideMethod;
        ++blockCounter;
        return frame;
    }

    LocalFrame &enterBlock() {
        // NOTE: the base-case for this being a valid initialization is setup by
        // the `create()` static method.
        return pushBlockFrame(scopeStack.back().insideMethod);
    }

    LocalFrame &enterMethod() {
        auto &frame = scopeStack.emplace_back();
        frame.oldBlockCounter = blockCounter;
        frame.insideMethod = true;
        blockCounter = 1;
        return frame;
    }

    LocalFrame &enterClass() {
        auto &frame = scopeStack.emplace_back();
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
    u4 blockCounter{0};

    core::LocalVariable enterLocal(core::NameRef name) {
        auto &frame = scopeStack.back();
        if (!frame.insideBlock) {
            return core::LocalVariable(name, 0);
        }
        return core::LocalVariable(name, frame.localId);
    }

    // Enter names from arguments into the current frame, building a new
    // argument list back up for the original context.
    ast::MethodDef::ARGS_store fillInArgs(vector<NamedArg> namedArgs) {
        ast::MethodDef::ARGS_store args;

        for (auto &named : namedArgs) {
            args.emplace_back(move(named.expr));
            auto frame = scopeStack.back();
            scopeStack.back().locals[named.name] = named.local;
            scopeStack.back().args.emplace_back(LocalFrame::Arg{named.local, named.flags});
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
    ast::TreePtr preTransformClassDef(core::MutableContext ctx, ast::TreePtr tree) {
        enterClass();
        return tree;
    }

    ast::TreePtr postTransformClassDef(core::MutableContext ctx, ast::TreePtr tree) {
        exitScope();
        return tree;
    }

    ast::TreePtr preTransformMethodDef(core::MutableContext ctx, ast::TreePtr tree) {
        enterMethod();

        auto &method = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        method.args = fillInArgs(nameArgs(ctx, method.args));
        return tree;
    }

    ast::TreePtr postTransformMethodDef(core::MutableContext ctx, ast::TreePtr tree) {
        exitScope();
        return tree;
    }

    ast::TreePtr postTransformSend(core::MutableContext ctx, ast::TreePtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::Send>(tree);
        if (original.args.size() == 1 && ast::isa_tree<ast::ZSuperArgs>(original.args[0])) {
            original.numPosArgs = 0;
            original.args.clear();
            if (scopeStack.back().insideMethod) {
                bool seenKeywordArgs = false;
                for (auto arg : scopeStack.back().args) {
                    if (arg.flags.isPositional()) {
                        ENFORCE(!seenKeywordArgs, "Saw positional arg after keyword arg");
                        original.args.emplace_back(ast::make_tree<ast::Local>(original.loc, arg.arg));
                        ++original.numPosArgs;
                    } else if (arg.flags.isKeyword()) {
                        seenKeywordArgs = true;
                        original.args.emplace_back(ast::MK::Symbol(original.loc, arg.arg._name));
                        original.args.emplace_back(ast::make_tree<ast::Local>(original.loc, arg.arg));
                    } else if (arg.flags.repeated || arg.flags.block) {
                        // Explicitly skip for now.
                        // Involves synthesizing a call to callWithSplat, callWithBlock, or callWithSplatAndBlock
                    } else if (arg.flags.shadow) {
                        ENFORCE(false, "Shadow only come from blocks, but super only looks at a method's args");
                    } else {
                        ENFORCE(false, "Unhandled arg kind in ZSuperArgs");
                    }
                }
            } else {
                if (auto e = ctx.beginError(original.loc, core::errors::Namer::SelfOutsideClass)) {
                    e.setHeader("`{}` outside of method", "super");
                }
            }
        }

        return tree;
    }

    ast::TreePtr preTransformBlock(core::MutableContext ctx, ast::TreePtr tree) {
        auto &blk = ast::cast_tree_nonnull<ast::Block>(tree);
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
        blk.args = fillInArgs(nameArgs(ctx, blk.args));

        return tree;
    }

    ast::TreePtr postTransformBlock(core::MutableContext ctx, ast::TreePtr tree) {
        exitScope();
        return tree;
    }

    ast::TreePtr postTransformUnresolvedIdent(core::MutableContext ctx, ast::TreePtr tree) {
        auto &nm = ast::cast_tree_nonnull<ast::UnresolvedIdent>(tree);
        if (nm.kind == ast::UnresolvedIdent::Kind::Local) {
            auto &frame = scopeStack.back();
            core::LocalVariable &cur = frame.locals[nm.name];
            if (!cur.exists()) {
                cur = enterLocal(nm.name);
                frame.locals[nm.name] = cur;
            }
            return ast::make_tree<ast::Local>(nm.loc, cur);
        } else {
            return tree;
        }
    }

private:
    LocalNameInserter() {
        // Setup a block frame that's outside of a method context as the base of
        // the scope stack.
        pushBlockFrame(false);
    }
};

ast::ParsedFile LocalVars::run(core::GlobalState &gs, ast::ParsedFile tree) {
    LocalNameInserter localNameInserter;
    sorbet::core::MutableContext ctx(gs, core::Symbols::root(), tree.file);
    tree.tree = ast::TreeMap::apply(ctx, localNameInserter, move(tree.tree));
    return tree;
}

} // namespace sorbet::local_vars
