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
        bool keyword : 1 = false;
        bool block : 1 = false;
        bool repeated : 1 = false;
        bool shadow : 1 = false;

        bool isPositional() const {
            return !this->keyword && !this->block && !this->repeated && !this->shadow;
        }
        bool isPositionalSplat() const {
            return !this->keyword && !this->block && this->repeated && !this->shadow;
        }
        bool isKeyword() const {
            return this->keyword && !this->repeated;
        }
        bool isKeywordSplat() const {
            return this->keyword && this->repeated;
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

    // Replace a send on ZSuperArgs with an explicit forwarding of the enclosing method's arguments.
    ast::ExpressionPtr lowerZSuperArgs(core::MutableContext ctx, ast::ExpressionPtr tree) {
        ENFORCE(ast::isa_tree<ast::Send>(tree));

        auto &original = ast::cast_tree_nonnull<ast::Send>(tree);
        ENFORCE(original.numPosArgs() == 1 && ast::isa_tree<ast::ZSuperArgs>(original.getPosArg(0)));

        ast::ExpressionPtr originalBlock;
        if (auto *rawBlock = original.rawBlock()) {
            originalBlock = move(*rawBlock);
        }

        // Clear out the args (which are just [ZSuperArgs]) in the original send. (Note that we want this cleared even
        // if we error out below, because later `ENFORCE`s will be triggered if we don't.)
        original.clearArgs();

        if (!scopeStack.back().insideMethod) {
            if (auto e = ctx.beginError(original.loc, core::errors::Namer::SelfOutsideClass)) {
                e.setHeader("`{}` outside of method", "super");
            }
            return tree;
        }

        // In the context of a method with a signature like this:
        //
        //    def f(<posargs>,<kwargs>,&<blkvar>)
        //
        // We're rewriting from something of the form:
        //
        //    super(ZSuperArgs)
        //
        // or:
        //
        //    super(ZSuperArgs) do <foo> end
        //
        // (Note that some <blkvar> is always present in the AST even if it was not explicitly written out in the
        // original source.)
        //
        // What we need to produce for the rewrite depends on two things: (1) whether there is a "*args"-style posarg
        // splat in the enclosing method's parameters, and (2) whether there is a "do" attached to the send that we are
        // lowering.
        //
        //    Posarg splat? | "do" attached? | Lower to...
        //    --------------+----------------+------------
        //    Yes           | Yes            | ::<Magic>::<call-with-splat>(..., :super, ...) do <foo> end
        //    Yes           | No             | ::<Magic>::<call-with-splat-and-block>(..., :super, ..., &<blkvar>)
        //    No            | Yes            | super(...) do <foo> end
        //    No            | No             | ::<Magic>::<call-with-block>(..., :super, ..., &<blkvar>)
        //
        // (In particular, note that the <blkvar> is thrown on the floor when a "do" is present.)

        // First, gather positional and keyword args into a vector (ENFORCE-ing for form invariants as we go)...
        ast::Array::ENTRY_store posArgsEntries;
        ast::Hash::ENTRY_store kwArgKeyEntries;
        ast::Hash::ENTRY_store kwArgValueEntries;

        // ...if we hit a splat along the way, however, we'll build it into an expression that evaluates to an array
        // (or a hash, where keyword args are concerned). Otherwise, posArgsArray (resp. kwArgsHash) will be null.
        ast::ExpressionPtr posArgsArray;
        ast::ExpressionPtr kwArgsHash;

        // We'll also look for the block arg, which should always be present at the end of the args.
        ast::ExpressionPtr blockArg;

        for (auto arg : scopeStack.back().args) {
            ENFORCE(blockArg == nullptr, "Block arg was not in final position");

            if (arg.flags.isPositional()) {
                ENFORCE(kwArgKeyEntries.empty(), "Saw positional arg after keyword arg");
                ENFORCE(kwArgsHash == nullptr, "Saw positional arg after keyword splat");

                posArgsEntries.emplace_back(ast::make_expression<ast::Local>(original.loc, arg.arg));
            } else if (arg.flags.isPositionalSplat()) {
                ENFORCE(posArgsArray == nullptr, "Saw multiple positional splats");
                ENFORCE(kwArgKeyEntries.empty(), "Saw positional splat after keyword arg");
                ENFORCE(kwArgsHash == nullptr, "Saw positional splat after keyword splat");

                posArgsArray = ast::MK::Splat(original.loc, ast::make_expression<ast::Local>(original.loc, arg.arg));
                if (!posArgsEntries.empty()) {
                    posArgsArray = ast::MK::Send1(original.loc, ast::MK::Array(original.loc, std::move(posArgsEntries)),
                                                  core::Names::concat(), std::move(posArgsArray));
                    posArgsEntries.clear();
                }
            } else if (arg.flags.isKeyword()) {
                ENFORCE(kwArgsHash == nullptr, "Saw keyword arg after keyword splat");

                kwArgKeyEntries.emplace_back(ast::MK::Literal(
                    original.loc, core::make_type<core::LiteralType>(core::Symbols::Symbol(), arg.arg._name)));
                kwArgValueEntries.emplace_back(ast::make_expression<ast::Local>(original.loc, arg.arg));
            } else if (arg.flags.isKeywordSplat()) {
                ENFORCE(kwArgsHash == nullptr, "Saw multiple keyword splats");

                // TODO(aprocter): is it necessary to duplicate the hash here?
                kwArgsHash =
                    ast::MK::Send1(original.loc, ast::MK::Constant(original.loc, core::Symbols::Magic()),
                                   core::Names::toHashDup(), ast::make_expression<ast::Local>(original.loc, arg.arg));
                if (!kwArgKeyEntries.empty()) {
                    // TODO(aprocter): it might make more sense to replace this with an InsSeq that calls
                    // <Magic>::<merge-hash>, which is what's done in the desugarer.
                    kwArgsHash = ast::MK::Send1(
                        original.loc,
                        ast::MK::Hash(original.loc, std::move(kwArgKeyEntries), std::move(kwArgValueEntries)),
                        core::Names::merge(), std::move(kwArgsHash));
                    kwArgKeyEntries.clear();
                    kwArgValueEntries.clear();
                }
            } else if (arg.flags.block) {
                blockArg = ast::make_expression<ast::Local>(original.loc, arg.arg);
            } else if (arg.flags.shadow) {
                ENFORCE(false, "Shadow only come from blocks, but super only looks at a method's args");
            } else {
                ENFORCE(false, "Unhandled arg kind in ZSuperArgs");
            }
        }

        // At this stage the method should always have a block arg, even if it's synthetic (i.e., wasn't mentioned in
        // the original source).
        ENFORCE(blockArg, "Block argument not present");

        // If there were any posargs after a positional splat, fold them into the splatted array.
        if (posArgsArray != nullptr && !posArgsEntries.empty()) {
            posArgsArray = ast::MK::Send1(original.loc, std::move(posArgsArray), core::Names::concat(),
                                          ast::MK::Array(original.loc, std::move(posArgsEntries)));
            posArgsEntries.clear();
        }

        auto method = ast::MK::Literal(
            original.loc, core::make_type<core::LiteralType>(core::Symbols::Symbol(), core::Names::super()));

        if (posArgsArray != nullptr) {
            // We wrap self with T.unsafe in order to get around the requirement for <call-with-splat> and
            // <call-with-splat-and-block> that the shapes of the splatted hashes be known statically. This is a bit of
            // a hack, but 'super' is currently treated as untyped anyway.
            original.addPosArg(ast::MK::Unsafe(original.loc, std::move(original.recv)));
            original.addPosArg(std::move(method));

            // For <call-with-splat> and <call-with-splat-and-block> posargs are always passed in an array.
            if (posArgsArray == nullptr) {
                posArgsArray = ast::MK::Array(original.loc, std::move(posArgsEntries));
            }
            original.addPosArg(std::move(posArgsArray));

            // For <call-with-splat> and <call-with-splat-and-block>, the kwargs array can either be a
            // [:key, val, :key, val, ...] array or a one-element [kwargshash] array, depending on whether splatting
            // has taken place, or nil (if no kwargs at all).
            ast::ExpressionPtr boxedKwArgs;

            if (kwArgsHash != nullptr) {
                ast::Array::ENTRY_store entries;
                entries.emplace_back(std::move(kwArgsHash));
                boxedKwArgs = ast::MK::Array(original.loc, std::move(entries));
            } else if (!kwArgKeyEntries.empty()) {
                ast::Array::ENTRY_store entries;
                entries.reserve(2 * kwArgKeyEntries.size());
                ENFORCE(kwArgKeyEntries.size() == kwArgValueEntries.size());
                for (size_t i = 0; i < kwArgKeyEntries.size(); i++) {
                    entries.emplace_back(std::move(kwArgKeyEntries[i]));
                    entries.emplace_back(std::move(kwArgValueEntries[i]));
                }
                boxedKwArgs = ast::MK::Array(original.loc, std::move(entries));
            } else {
                boxedKwArgs = ast::MK::Nil(original.loc);
            }
            original.addPosArg(std::move(boxedKwArgs));

            original.recv = ast::MK::Constant(original.loc, core::Symbols::Magic());

            if (originalBlock != nullptr) {
                // <call-with-splat> and "do"
                original.fun = core::Names::callWithSplat();
                // Re-add block argument
                original.setBlock(std::move(originalBlock));
            } else {
                // <call-with-splat-and-block>(..., &blk)
                original.fun = core::Names::callWithSplatAndBlock();
                original.addPosArg(std::move(blockArg));
            }
        } else if (originalBlock == nullptr) {
            // No positional splat and no "do", so we need to forward &<blkvar> with <call-with-block>.
            original.reserveArguments(3 + posArgsEntries.size(), kwArgKeyEntries.size(),
                                      /* hasKwSplat */ kwArgsHash != nullptr, /* hasBlock */ false);
            original.addPosArg(std::move(original.recv));
            original.addPosArg(std::move(method));
            original.addPosArg(std::move(blockArg));

            for (auto &arg : posArgsEntries) {
                original.addPosArg(std::move(arg));
            }
            posArgsEntries.clear();

            if (kwArgsHash != nullptr) {
                original.setKwSplat(std::move(kwArgsHash));
            } else {
                ENFORCE(kwArgKeyEntries.size() == kwArgValueEntries.size());
                for (size_t i = 0; i < kwArgKeyEntries.size(); i++) {
                    original.addKwArg(std::move(kwArgKeyEntries[i]), std::move(kwArgValueEntries[i]));
                }
            }
            kwArgKeyEntries.clear();
            kwArgValueEntries.clear();

            original.recv = ast::MK::Constant(original.loc, core::Symbols::Magic());
            original.fun = core::Names::callWithBlock();
        } else {
            // No positional splat and we have a "do", so we can synthesize an ordinary send.
            original.reserveArguments(posArgsEntries.size(), kwArgKeyEntries.size(),
                                      /* hasKwSplat */ kwArgsHash != nullptr, /* hasBlock */ false);

            for (auto &arg : posArgsEntries) {
                original.addPosArg(std::move(arg));
            }
            posArgsEntries.clear();

            if (kwArgsHash != nullptr) {
                original.setKwSplat(std::move(kwArgsHash));
            } else {
                ENFORCE(kwArgKeyEntries.size() == kwArgValueEntries.size());
                for (size_t i = 0; i < kwArgKeyEntries.size(); i++) {
                    original.addKwArg(std::move(kwArgKeyEntries[i]), std::move(kwArgValueEntries[i]));
                }
            }
            // Re-add original block
            original.setBlock(std::move(originalBlock));
            kwArgKeyEntries.clear();
            kwArgValueEntries.clear();
        }

        return tree;
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
        if (original.numPosArgs() == 1 && ast::isa_tree<ast::ZSuperArgs>(original.getPosArg(0))) {
            return lowerZSuperArgs(ctx, std::move(tree));
        }

        return tree;
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
