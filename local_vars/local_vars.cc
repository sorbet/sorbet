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
        ast::ExpressionPtr expr;
        ArgFlags flags;
    };

    // Map through the reference structure, naming the locals, and preserving
    // the outer structure for the namer proper.
    NamedArg nameArg(ast::ExpressionPtr arg) {
        NamedArg named;

        typecase(
            arg,
            [&](ast::UnresolvedIdent &nm) {
                named.name = nm.name;
                named.local = enterLocal(named.name);
                named.loc = arg.loc();
                named.expr = ast::make_expression<ast::Local>(arg.loc(), named.local);
            },
            [&](ast::RestArg &rest) {
                named = nameArg(move(rest.expr));
                named.expr = ast::MK::RestArg(arg.loc(), move(named.expr));
                named.flags.repeated = true;
            },
            [&](ast::KeywordArg &kw) {
                named = nameArg(move(kw.expr));
                named.expr = ast::make_expression<ast::KeywordArg>(arg.loc(), move(named.expr));
                named.flags.keyword = true;
            },
            [&](ast::OptionalArg &opt) {
                named = nameArg(move(opt.expr));
                named.expr = ast::MK::OptionalArg(arg.loc(), move(named.expr), move(opt.default_));
            },
            [&](ast::BlockArg &blk) {
                named = nameArg(move(blk.expr));
                named.expr = ast::MK::BlockArg(arg.loc(), move(named.expr));
                named.flags.block = true;
            },
            [&](ast::ShadowArg &shadow) {
                named = nameArg(move(shadow.expr));
                named.expr = ast::MK::ShadowArg(arg.loc(), move(named.expr));
                named.flags.shadow = true;
            },
            [&](const ast::Local &local) {
                named.name = local.localVariable._name;
                named.local = enterLocal(named.name);
                named.loc = arg.loc();
                named.expr = ast::make_expression<ast::Local>(local.loc, named.local);
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

    core::ClassOrModuleRef methodOwner(core::MutableContext ctx) {
        core::ClassOrModuleRef owner = ctx.owner.enclosingClass(ctx);
        if (owner == core::Symbols::root()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        return owner;
    }

public:
    ast::ExpressionPtr preTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr tree) {
        enterClass();
        return tree;
    }

    ast::ExpressionPtr postTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr tree) {
        exitScope();
        return tree;
    }

    ast::ExpressionPtr preTransformMethodDef(core::MutableContext ctx, ast::ExpressionPtr tree) {
        enterMethod();

        auto &method = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        method.args = fillInArgs(nameArgs(ctx, method.args));
        return tree;
    }

    ast::ExpressionPtr postTransformMethodDef(core::MutableContext ctx, ast::ExpressionPtr tree) {
        exitScope();
        return tree;
    }

    ast::ExpressionPtr postTransformSend(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::Send>(tree);
        if (original.args.size() != 1 || !ast::isa_tree<ast::ZSuperArgs>(original.args[0])) {
            return tree;
        }

        if (!scopeStack.back().insideMethod) {
            if (auto e = ctx.beginError(original.loc, core::errors::Namer::SelfOutsideClass)) {
                e.setHeader("`{}` outside of method", "super");
            }
            return tree;
        }

        bool seenKeywordArgs = false;
        bool seenRepeatedArgs = false;
        bool seenBlockArg = false;

        // Scan ahead to see if we have repeat or block args, and to validate that args are of the shape we
        // expect.
        for (auto arg : scopeStack.back().args) {
            ENFORCE(!seenBlockArg, "Block arg was not in final position");
            if (arg.flags.isPositional()) {
                ENFORCE(!seenKeywordArgs, "Saw positional arg after keyword arg");
            } else if (arg.flags.repeated) {
                ENFORCE(!seenKeywordArgs, "Saw repeated arg after keyword arg");
                seenRepeatedArgs = true;
            } else if (arg.flags.isKeyword()) {
                seenKeywordArgs = true;
            } else if (arg.flags.block) {
                seenBlockArg = true;
            } else if (arg.flags.shadow) {
                ENFORCE(false, "Shadow only come from blocks, but super only looks at a method's args");
            } else {
                ENFORCE(false, "Unhandled arg kind in ZSuperArgs");
            }
        }

        // If we don't have repeat or block args, we can rewrite `original` in place.
        // TODO(aprocter): Actually I'm not sure this case will ever fire anymore!
        if (!seenRepeatedArgs && !seenBlockArg) {
            original.numPosArgs = 0;
            original.args.clear();

            for (auto arg : scopeStack.back().args) {
                if (arg.flags.isPositional()) {
                    original.args.emplace_back(ast::make_expression<ast::Local>(original.loc, arg.arg));
                    ++original.numPosArgs;
                } else if (arg.flags.isKeyword()) {
                    original.args.emplace_back(ast::MK::Symbol(original.loc, arg.arg._name));
                    original.args.emplace_back(ast::make_expression<ast::Local>(original.loc, arg.arg));
                }
            }

            return tree;
        } else {
            // So what the desugarer does with this is to box the args in an array and then node2Tree that.
            // (Ignoring kwargs for now.)
            ast::Array::ENTRY_store posArgsEntries;
            ast::ExpressionPtr posArgsArray;

            for (auto arg : scopeStack.back().args) {
                if (arg.flags.isPositional()) {
                    posArgsEntries.emplace_back(ast::make_expression<ast::Local>(original.loc, arg.arg));
                } else if (arg.flags.repeated) {
                    if (!posArgsEntries.empty()) {
                        if (posArgsArray == nullptr) {
                            posArgsArray = ast::MK::Array(original.loc, std::move(posArgsEntries));
                        } else {
                            posArgsArray = ast::MK::Send1(original.loc, std::move(posArgsArray), core::Names::concat(),
                                                          ast::MK::Array(original.loc, std::move(posArgsEntries)));
                        }
                        posArgsArray = ast::MK::Send1(
                            original.loc, std::move(posArgsArray), core::Names::concat(),
                            ast::MK::Splat(original.loc, ast::make_expression<ast::Local>(original.loc, arg.arg)));
                        posArgsEntries.clear();
                    }
                } else if (arg.flags.keyword) {
                    ENFORCE(false, "oopsie, a keyword");
                } // else if (arg.flags.block) {
                  //  ENFORCE(false, "oopsie, a block");
                  //}
            }
            if (!posArgsEntries.empty()) {
                if (posArgsArray == nullptr) {
                    posArgsArray = ast::MK::Array(original.loc, std::move(posArgsEntries));
                } else {
                    posArgsArray = ast::MK::Send1(original.loc, std::move(posArgsArray), core::Names::concat(),
                                                  ast::MK::Array(original.loc, std::move(posArgsEntries)));
                }
            }

            auto method = ast::MK::Literal(
                original.loc, core::make_type<core::LiteralType>(core::Symbols::Symbol(), core::Names::super()));

            ast::Send::ARGS_store sendargs;
            sendargs.emplace_back(std::move(original.recv));
            sendargs.emplace_back(std::move(method));
            sendargs.emplace_back(std::move(posArgsArray));
            // sendargs.emplace_back(std::move(kwargs));

            return ast::MK::Send(original.loc, ast::MK::Constant(original.loc, core::Symbols::Magic()),
                                 core::Names::callWithSplat(), 3, std::move(sendargs));
        }
    }

    ast::ExpressionPtr preTransformBlock(core::MutableContext ctx, ast::ExpressionPtr tree) {
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

    ast::ExpressionPtr postTransformBlock(core::MutableContext ctx, ast::ExpressionPtr tree) {
        exitScope();
        return tree;
    }

    ast::ExpressionPtr postTransformUnresolvedIdent(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto &nm = ast::cast_tree_nonnull<ast::UnresolvedIdent>(tree);
        if (nm.kind == ast::UnresolvedIdent::Kind::Local) {
            auto &frame = scopeStack.back();
            core::LocalVariable &cur = frame.locals[nm.name];
            if (!cur.exists()) {
                cur = enterLocal(nm.name);
                frame.locals[nm.name] = cur;
            }
            return ast::make_expression<ast::Local>(nm.loc, cur);
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
