#include "namer/namer.h"
#include "ast/ArgParsing.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "common/typecase.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/core.h"
#include "core/errors/namer.h"
#include "flattener/flatten.h"

using namespace std;

namespace sorbet::namer {

/**
 * Used with TreeMap to insert all the class and method symbols into the symbol
 * table.
 */
class NameInserter {
    friend class Namer;

    core::SymbolRef squashNames(core::MutableContext ctx, core::SymbolRef owner, unique_ptr<ast::Expression> &node) {
        auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(node.get());
        if (constLit == nullptr) {
            if (auto *id = ast::cast_tree<ast::ConstantLit>(node.get())) {
                return id->symbol.data(ctx)->dealias(ctx);
            }
            if (auto *uid = ast::cast_tree<ast::UnresolvedIdent>(node.get())) {
                if (uid->kind != ast::UnresolvedIdent::Class || uid->name != core::Names::singleton()) {
                    if (auto e = ctx.state.beginError(node->loc, core::errors::Namer::DynamicConstant)) {
                        e.setHeader("Unsupported constant scope");
                    }
                }
                // emitted via `class << self` blocks
            } else if (ast::isa_tree<ast::EmptyTree>(node.get())) {
                // ::Foo
            } else if (node->isSelfReference()) {
                // self::Foo
            } else {
                if (auto e = ctx.state.beginError(node->loc, core::errors::Namer::DynamicConstant)) {
                    e.setHeader("Dynamic constant references are unsupported");
                }
            }
            node = ast::MK::EmptyTree();
            return owner;
        }

        auto newOwner = squashNames(ctx, owner, constLit->scope);
        core::SymbolRef existing = newOwner.data(ctx)->findMember(ctx, constLit->cnst);
        if (!existing.exists()) {
            if (!newOwner.data(ctx)->isClassOrModule()) {
                if (auto e = ctx.state.beginError(node->loc, core::errors::Namer::InvalidClassOwner)) {
                    auto constLitName = constLit->cnst.data(ctx)->show(ctx);
                    auto newOwnerName = newOwner.data(ctx)->show(ctx);
                    e.setHeader("Can't nest `{}` under `{}` because `{}` is not a class or module", constLitName,
                                newOwnerName, newOwnerName);
                    e.addErrorLine(newOwner.data(ctx)->loc(), "`{}` defined here", newOwnerName);
                }
                node = ast::MK::EmptyTree();
                return owner;
            }
            existing = ctx.state.enterClassSymbol(constLit->loc, newOwner, constLit->cnst);
            existing.data(ctx)->singletonClass(ctx); // force singleton class into existance
        }

        node.release();
        unique_ptr<ast::UnresolvedConstantLit> constTmp(constLit);
        node = make_unique<ast::ConstantLit>(constLit->loc, existing, std::move(constTmp));
        return existing;
    }

    unique_ptr<ast::Expression> arg2Symbol(core::MutableContext ctx, int pos, ast::ParsedArg parsedArg) {
        if (pos < ctx.owner.data(ctx)->arguments().size()) {
            // TODO: check that flags match;
            unique_ptr<ast::Reference> localExpr = make_unique<ast::Local>(parsedArg.loc, parsedArg.local);
            if (parsedArg.default_) {
                localExpr = make_unique<ast::OptionalArg>(parsedArg.loc, move(localExpr), move(parsedArg.default_));
            }

            ctx.owner.data(ctx)->arguments()[pos].loc = parsedArg.loc;
            return move(localExpr);
        }

        core::NameRef name;
        if (parsedArg.keyword) {
            name = parsedArg.local._name;
        } else if (parsedArg.block) {
            name = core::Names::blkArg();
        } else {
            name = ctx.state.freshNameUnique(core::UniqueNameKind::PositionalArg, core::Names::arg(), pos + 1);
        }
        // we know right now that pos >= arguments().size() because otherwise we would have hit the early return at the
        // beginning of this method
        auto &argInfo = ctx.state.enterMethodArgumentSymbol(parsedArg.loc, ctx.owner, name);
        // if enterMethodArgumentSymbol did not emplace a new argument into the list, then it means it's reusing an
        // existing one, which means we've seen a repeated kwarg (as it treats identically named kwargs as
        // identical). We know that we need to match the arity of the function as written, so if we don't have as many
        // arguments as we expect, clone the one we got back from enterMethodArgumentSymbol in the position we expect
        if (ctx.owner.dataAllowingNone(ctx)->arguments().size() == pos) {
            auto argCopy = argInfo.deepCopy();
            argCopy.name = ctx.state.freshNameUnique(core::UniqueNameKind::MangledKeywordArg, argInfo.name, pos + 1);
            ctx.owner.dataAllowingNone(ctx)->arguments().emplace_back(move(argCopy));
            auto localExpr = make_unique<ast::Local>(parsedArg.loc, parsedArg.local);
            return move(localExpr);
        }
        // at this point, we should have at least pos + 1 arguments, and arguments[pos] should be the thing we got back
        // from enterMethodArgumentSymbol
        ENFORCE(ctx.owner.data(ctx)->arguments().size() >= pos + 1);
        unique_ptr<ast::Reference> localExpr = make_unique<ast::Local>(parsedArg.loc, parsedArg.local);

        if (parsedArg.default_) {
            argInfo.flags.isDefault = true;
            localExpr = make_unique<ast::OptionalArg>(parsedArg.loc, move(localExpr), move(parsedArg.default_));
        }

        if (parsedArg.keyword) {
            argInfo.flags.isKeyword = true;
        }
        if (parsedArg.block) {
            argInfo.flags.isBlock = true;
        }
        if (parsedArg.repeated) {
            argInfo.flags.isRepeated = true;
        }

        return move(localExpr);
    }

    bool addAncestor(core::MutableContext ctx, unique_ptr<ast::ClassDef> &klass, unique_ptr<ast::Expression> &node) {
        auto send = ast::cast_tree<ast::Send>(node.get());
        if (send == nullptr) {
            ENFORCE(node.get() != nullptr);
            return false;
        }

        ast::ClassDef::ANCESTORS_store *dest;
        if (send->fun == core::Names::include()) {
            dest = &klass->ancestors;
        } else if (send->fun == core::Names::extend()) {
            dest = &klass->singletonAncestors;
        } else {
            return false;
        }
        if (!send->recv->isSelfReference()) {
            // ignore `something.include`
            return false;
        }

        if (send->args.empty()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::IncludeMutipleParam)) {
                e.setHeader("`{}` requires at least one argument", send->fun.data(ctx)->show(ctx));
            }
            return false;
        }

        if (send->block != nullptr) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::IncludePassedBlock)) {
                e.setHeader("`{}` can not be passed a block", send->fun.data(ctx)->show(ctx));
            }
            return false;
        }

        for (auto it = send->args.rbegin(); it != send->args.rend(); it++) {
            // Reverse order is intentional: that's how Ruby does it.
            auto &arg = *it;
            if (ast::isa_tree<ast::EmptyTree>(arg.get())) {
                continue;
            }
            if (arg->isSelfReference()) {
                dest->emplace_back(std::move(arg));
                continue;
            }
            if (isValidAncestor(arg.get())) {
                dest->emplace_back(std::move(arg));
            } else {
                if (auto e = ctx.state.beginError(arg->loc, core::errors::Namer::AncestorNotConstant)) {
                    e.setHeader("`{}` must only contain constant literals", send->fun.data(ctx)->show(ctx));
                }
                arg = ast::MK::EmptyTree();
            }
        }

        return true;
    }

    void aliasMethod(core::MutableContext ctx, core::Loc loc, core::SymbolRef owner, core::NameRef newName,
                     core::SymbolRef method) {
        core::SymbolRef alias = ctx.state.enterMethodSymbol(loc, owner, newName);
        alias.data(ctx)->resultType = core::make_type<core::AliasType>(method);
    }

    core::SymbolRef methodOwner(core::MutableContext ctx) {
        core::SymbolRef owner = ctx.owner.data(ctx)->enclosingClass(ctx);
        if (owner == core::Symbols::root()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        return owner;
    }
    bool isValidAncestor(ast::Expression *exp) {
        if (ast::isa_tree<ast::EmptyTree>(exp) || exp->isSelfReference() || ast::isa_tree<ast::ConstantLit>(exp)) {
            return true;
        }
        if (auto lit = ast::cast_tree<ast::UnresolvedConstantLit>(exp)) {
            return isValidAncestor(lit->scope.get());
        }
        return false;
    }

public:
    unique_ptr<ast::ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> klass) {
        auto *ident = ast::cast_tree<ast::UnresolvedIdent>(klass->name.get());

        if ((ident != nullptr) && ident->name == core::Names::singleton()) {
            ENFORCE(ident->kind == ast::UnresolvedIdent::Class);
            klass->symbol = ctx.owner.data(ctx)->enclosingClass(ctx).data(ctx)->singletonClass(ctx);
        } else {
            if (klass->symbol == core::Symbols::todo()) {
                klass->symbol = squashNames(ctx, ctx.owner.data(ctx)->enclosingClass(ctx), klass->name);
            } else {
                // Desugar populates a top-level root() ClassDef.
                // Nothing else should have been typeAlias by now.
                ENFORCE(klass->symbol == core::Symbols::root());
            }
            bool isModule = klass->kind == ast::ClassDefKind::Module;
            if (!klass->symbol.data(ctx)->isClassOrModule()) {
                // we might have already mangled the class symbol, so see if we have a symbol that is a class already
                auto klassSymbol =
                    ctx.state.lookupClassSymbol(klass->symbol.data(ctx)->owner, klass->symbol.data(ctx)->name);
                if (klassSymbol.exists()) {
                    klass->symbol = klassSymbol;
                } else {
                    if (auto e = ctx.state.beginError(klass->loc, core::errors::Namer::ModuleKindRedefinition)) {
                        e.setHeader("Redefining constant `{}`", klass->symbol.data(ctx)->show(ctx));
                        e.addErrorLine(klass->symbol.data(ctx)->loc(), "Previous definition");
                    }
                    auto origName = klass->symbol.data(ctx)->name;
                    ctx.state.mangleRenameSymbol(klass->symbol, klass->symbol.data(ctx)->name);
                    klass->symbol =
                        ctx.state.enterClassSymbol(klass->declLoc, klass->symbol.data(ctx)->owner, origName);
                    klass->symbol.data(ctx)->setIsModule(isModule);

                    auto oldSymCount = ctx.state.symbolsUsed();
                    auto newSingleton =
                        klass->symbol.data(ctx)->singletonClass(ctx); // force singleton class into existence
                    ENFORCE(newSingleton._id >= oldSymCount,
                            "should be a fresh symbol. Otherwise we could be reusing an existing singletonClass");
                }
            } else if (klass->symbol.data(ctx)->isClassModuleSet() &&
                       isModule != klass->symbol.data(ctx)->isClassOrModuleModule()) {
                if (auto e = ctx.state.beginError(klass->loc, core::errors::Namer::ModuleKindRedefinition)) {
                    e.setHeader("`{}` was previously defined as a `{}`", klass->symbol.data(ctx)->show(ctx),
                                klass->symbol.data(ctx)->isClassOrModuleModule() ? "module" : "class");
                }
            } else {
                klass->symbol.data(ctx)->setIsModule(isModule);
            }
        }
        return klass;
    }

    bool handleNamerDSL(core::MutableContext ctx, unique_ptr<ast::ClassDef> &klass, unique_ptr<ast::Expression> &line) {
        if (addAncestor(ctx, klass, line)) {
            return true;
        }

        auto *send = ast::cast_tree<ast::Send>(line.get());
        if (send == nullptr) {
            return false;
        }
        if (send->fun == core::Names::declareFinal()) {
            klass->symbol.data(ctx)->setClassFinal();
            klass->symbol.data(ctx)->singletonClass(ctx).data(ctx)->setClassFinal();
        }
        if (send->fun == core::Names::declareSealed()) {
            klass->symbol.data(ctx)->setClassSealed();

            auto classOfKlass = klass->symbol.data(ctx)->singletonClass(ctx);
            auto sealedSubclasses =
                ctx.state.enterMethodSymbol(send->loc, classOfKlass, core::Names::sealedSubclasses());
            auto &blkArg =
                ctx.state.enterMethodArgumentSymbol(core::Loc::none(), sealedSubclasses, core::Names::blkArg());
            blkArg.flags.isBlock = true;

            // T.noreturn here represents the zero-length list of subclasses of this sealed class.
            // We will use T.any to record subclasses when they're resolved.
            sealedSubclasses.data(ctx)->resultType = core::Types::arrayOf(ctx, core::Types::bottom());
        }
        if (send->fun == core::Names::declareInterface() || send->fun == core::Names::declareAbstract()) {
            klass->symbol.data(ctx)->setClassAbstract();
            klass->symbol.data(ctx)->singletonClass(ctx).data(ctx)->setClassAbstract();
        }
        if (send->fun == core::Names::declareInterface()) {
            klass->symbol.data(ctx)->setClassInterface();
            if (klass->kind == ast::Class) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InterfaceClass)) {
                    e.setHeader("Classes can't be interfaces. Use `abstract!` instead of `interface!`");
                    e.replaceWith("Change `interface!` to `abstract!`", send->loc, "abstract!");
                }
            }
        }

        // explicitly keep the namer dsl functions present
        return false;
    }

    // This decides if we need to keep a node around incase the current LSP query needs type information for it
    bool shouldLeaveAncestorForIDE(const unique_ptr<ast::Expression> &anc) {
        // used in Desugar <-> resolver to signal classes that did not have explicit superclass
        if (ast::isa_tree<ast::EmptyTree>(anc.get()) || anc->isSelfReference()) {
            return false;
        }
        auto rcl = ast::cast_tree<ast::ConstantLit>(anc.get());
        if (rcl && rcl->symbol == core::Symbols::todo()) {
            return false;
        }
        return true;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> klass) {
        if (klass->kind == ast::Class && !klass->symbol.data(ctx)->superClass().exists() &&
            klass->symbol != core::Symbols::BasicObject()) {
            klass->symbol.data(ctx)->setSuperClass(core::Symbols::todo());
        }

        // In Ruby 2.5 they changed this class to have a different superclass
        // from 2.4. Since we don't have a good story around versioned ruby rbis
        // yet, lets just force the superclass regardless of version.
        if (klass->symbol == core::Symbols::Net_IMAP()) {
            klass->symbol.data(ctx)->setSuperClass(core::Symbols::Net_Protocol());
        }

        klass->symbol.data(ctx)->addLoc(ctx, klass->declLoc);
        klass->symbol.data(ctx)->singletonClass(ctx); // force singleton class into existence

        auto toRemove = remove_if(klass->rhs.begin(), klass->rhs.end(),
                                  [&](unique_ptr<ast::Expression> &line) { return handleNamerDSL(ctx, klass, line); });
        klass->rhs.erase(toRemove, klass->rhs.end());

        if (!klass->ancestors.empty()) {
            /* Superclass is typeAlias in parent scope, mixins are typeAlias in inner scope */
            for (auto &anc : klass->ancestors) {
                if (!isValidAncestor(anc.get())) {
                    if (auto e = ctx.state.beginError(anc->loc, core::errors::Namer::AncestorNotConstant)) {
                        e.setHeader("Superclasses must only contain constant literals");
                    }
                    anc = ast::MK::EmptyTree();
                } else if (shouldLeaveAncestorForIDE(anc) &&
                           (klass->kind == ast::Module || anc != klass->ancestors.front())) {
                    klass->rhs.emplace_back(ast::MK::KeepForIDE(anc->deepCopy()));
                }
            }
        }
        if (!klass->singletonAncestors.empty()) {
            for (auto &sanc : klass->singletonAncestors) {
                if (shouldLeaveAncestorForIDE(sanc)) {
                    klass->rhs.emplace_back(ast::MK::KeepForIDE(sanc->deepCopy()));
                }
            }
        }
        ast::InsSeq::STATS_store ideSeqs;
        if (ast::isa_tree<ast::ConstantLit>(klass->name.get())) {
            ideSeqs.emplace_back(ast::MK::KeepForIDE(klass->name->deepCopy()));
        }
        if (klass->kind == ast::Class && !klass->ancestors.empty() &&
            shouldLeaveAncestorForIDE(klass->ancestors.front())) {
            ideSeqs.emplace_back(ast::MK::KeepForIDE(klass->ancestors.front()->deepCopy()));
        }

        // make sure we've added a static init symbol so we have it ready for the flatten pass later
        if (klass->symbol == core::Symbols::root()) {
            ctx.state.staticInitForFile(klass->loc);
        } else {
            ctx.state.staticInitForClass(klass->symbol, klass->loc);
        }

        if (klass->symbol != core::Symbols::root() && !klass->declLoc.file().data(ctx).isRBI() &&
            ast::BehaviorHelpers::checkClassDefinesBehavior(klass)) {
            // TODO(dmitry) This won't find errors in fast-incremental mode.
            auto prevLoc = classBehaviorLocs.find(klass->symbol);
            if (prevLoc == classBehaviorLocs.end()) {
                classBehaviorLocs[klass->symbol] = klass->declLoc;
            } else if (prevLoc->second.file() != klass->declLoc.file()) {
                if (auto e = ctx.state.beginError(klass->declLoc, core::errors::Namer::MultipleBehaviorDefs)) {
                    e.setHeader("`{}` has behavior defined in multiple files", klass->symbol.data(ctx)->show(ctx));
                    e.addErrorLine(prevLoc->second, "Previous definition");
                }
            }
        }
        return ast::MK::InsSeq(klass->declLoc, std::move(ideSeqs), std::move(klass));
    }

    ast::MethodDef::ARGS_store fillInArgs(core::MutableContext ctx, vector<ast::ParsedArg> parsedArgs) {
        ast::MethodDef::ARGS_store args;
        bool inShadows = false;
        bool intrinsic = isIntrinsic(ctx, ctx.owner);
        bool swapArgs = intrinsic && (ctx.owner.data(ctx)->arguments().size() == 1);
        core::ArgInfo swappedArg;
        if (swapArgs) {
            // When we're filling in an intrinsic method, we want to overwrite the block arg that used
            // to exist with the block arg that we got from desugaring the method def in the RBI files.
            ENFORCE(ctx.owner.data(ctx)->arguments()[0].flags.isBlock);
            swappedArg = move(ctx.owner.data(ctx)->arguments()[0]);
            ctx.owner.data(ctx)->arguments().clear();
        }

        int i = -1;
        for (auto &arg : parsedArgs) {
            i++;
            auto localVariable = arg.local;

            if (arg.shadow) {
                inShadows = true;
                auto localExpr = make_unique<ast::Local>(arg.loc, localVariable);
                args.emplace_back(move(localExpr));
            } else {
                ENFORCE(!inShadows, "shadow argument followed by non-shadow argument!");

                if (swapArgs && arg.block) {
                    // see commnent on if (swapArgs) above
                    ctx.owner.data(ctx)->arguments().emplace_back(move(swappedArg));
                }

                auto expr = arg2Symbol(ctx, i, move(arg));
                args.emplace_back(move(expr));
                ENFORCE(i < ctx.owner.data(ctx)->arguments().size());
            }
        }

        return args;
    }

    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> original) {
        ast::MethodDef *mdef;
        if (original->args.size() == 1 && (mdef = ast::cast_tree<ast::MethodDef>(original->args[0].get())) != nullptr) {
            switch (original->fun._id) {
                case core::Names::private_()._id:
                case core::Names::privateClassMethod()._id:
                    mdef->symbol.data(ctx)->setPrivate();
                    break;
                case core::Names::protected_()._id:
                    mdef->symbol.data(ctx)->setProtected();
                    break;
                case core::Names::public_()._id:
                    mdef->symbol.data(ctx)->setPublic();
                    break;
                default:
                    return original;
            }
            return std::move(original->args[0]);
        }
        return original;
    }

    // Allow stub symbols created to hold intrinsics to be filled in
    // with real types from code
    bool isIntrinsic(core::Context ctx, core::SymbolRef sym) {
        auto data = sym.data(ctx);
        return data->intrinsic != nullptr && data->resultType == nullptr;
    }

    bool paramsMatch(core::MutableContext ctx, core::SymbolRef method, const vector<ast::ParsedArg> &parsedArgs) {
        auto sym = method.data(ctx)->dealias(ctx);
        if (sym.data(ctx)->arguments().size() != parsedArgs.size()) {
            return false;
        }
        for (int i = 0; i < parsedArgs.size(); i++) {
            auto &methodArg = parsedArgs[i];
            auto &symArg = sym.data(ctx)->arguments()[i];

            if (symArg.flags.isKeyword != methodArg.keyword || symArg.flags.isBlock != methodArg.block ||
                symArg.flags.isRepeated != methodArg.repeated ||
                (symArg.flags.isKeyword && symArg.name != methodArg.local._name)) {
                return false;
            }
        }

        return true;
    }

    void paramMismatchErrors(core::MutableContext ctx, core::Loc loc, const vector<ast::ParsedArg> &parsedArgs) {
        auto sym = ctx.owner.data(ctx)->dealias(ctx);
        if (!sym.data(ctx)->isMethod()) {
            return;
        }
        if (sym.data(ctx)->arguments().size() != parsedArgs.size()) {
            if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                if (sym != ctx.owner) {
                    // Subtracting 1 because of the block arg we added everywhere.
                    // Eventually we should be more principled about how we report this.
                    e.setHeader(
                        "Method alias `{}` redefined without matching argument count. Expected: `{}`, got: `{}`",
                        ctx.owner.data(ctx)->show(ctx), sym.data(ctx)->arguments().size() - 1, parsedArgs.size() - 1);
                    e.addErrorLine(ctx.owner.data(ctx)->loc(), "Previous alias definition");
                    e.addErrorLine(sym.data(ctx)->loc(), "Dealiased definition");
                } else {
                    // Subtracting 1 because of the block arg we added everywhere.
                    // Eventually we should be more principled about how we report this.
                    e.setHeader("Method `{}` redefined without matching argument count. Expected: `{}`, got: `{}`",
                                sym.show(ctx), sym.data(ctx)->arguments().size() - 1, parsedArgs.size() - 1);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
            }
            return;
        }
        for (int i = 0; i < parsedArgs.size(); i++) {
            auto &methodArg = parsedArgs[i];
            auto &symArg = sym.data(ctx)->arguments()[i];

            if (symArg.flags.isKeyword != methodArg.keyword) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader(
                        "Method `{}` redefined with mismatched argument attribute `{}`. Expected: `{}`, got: `{}`",
                        sym.show(ctx), "isKeyword", symArg.flags.isKeyword, methodArg.keyword);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return;
            }
            if (symArg.flags.isBlock != methodArg.block) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader(
                        "Method `{}` redefined with mismatched argument attribute `{}`. Expected: `{}`, got: `{}`",
                        sym.show(ctx), "isBlock", symArg.flags.isBlock, methodArg.block);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return;
            }
            if (symArg.flags.isRepeated != methodArg.repeated) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader(
                        "Method `{}` redefined with mismatched argument attribute `{}`. Expected: `{}`, got: `{}`",
                        sym.show(ctx), "isRepeated", symArg.flags.isRepeated, methodArg.repeated);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return;
            }
            if (symArg.flags.isKeyword && symArg.name != methodArg.local._name) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader("Method `{}` redefined with mismatched argument name. Expected: `{}`, got: `{}`",
                                sym.show(ctx), symArg.name.show(ctx), methodArg.local._name.show(ctx));
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return;
            }
        }
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> method) {
        core::SymbolRef owner = methodOwner(ctx);

        if (method->isSelf()) {
            if (owner.data(ctx)->isClassOrModule()) {
                owner = owner.data(ctx)->singletonClass(ctx);
            }
        }
        ENFORCE(owner.data(ctx)->isClassOrModule());

        auto parsedArgs = ast::ArgParsing::parseArgs(ctx, method->args);

        // There are three symbols in play here, because there's:
        //
        // - sym, the symbol which, if it exists, is the 'correct' symbol for the method in question, as it matches
        //   in both name and argument structure
        // - currentSym, which is the whatever the non-mangled name currently points to in that context
        // - replacedSym, which is whatever name was the name that sym replaced.
        //
        // In the context of something like
        //   def f(); end
        //   def f(x); end
        //   def f(x, y); end
        // when the namer is in incremental mode and looking at `def f(x)`, then `sym` refers to `f(x)`, `currentSym`
        // refers to `def f(x, y)` (which is the symbol that is "currently" non-mangled and available under the name
        // `f`, because the namer has already run in this context) and `replacedSym` refers to `def f()`, because that's
        // the symbol that `f` referred to immediately before `def f(x)` became the then-current symbol. If we weren't
        // running in incremental mode, then `sym` wouldn't exist yet (we would be about to create it!) and `currentSym`
        // would be `def f()`, because we have not yet progressed far enough in the file to see any other definition of
        // `f`.
        auto sym =
            ctx.state.lookupMethodSymbolWithHash(owner, method->name, ast::ArgParsing::hashArgs(ctx, parsedArgs));
        auto currentSym = ctx.state.lookupMethodSymbol(owner, method->name);
        if (!sym.exists() && currentSym.exists()) {
            // we don't have a method definition with the right argument structure, so we need to mangle the existing
            // one and create a new one
            if (!isIntrinsic(ctx, currentSym)) {
                paramMismatchErrors(ctx.withOwner(currentSym), method->declLoc, parsedArgs);
                ctx.state.mangleRenameSymbol(currentSym, method->name);
            } else {
                // ...unless it's an intrinsic, because we allow multiple incompatible definitions of those in code
                sym.data(ctx)->addLoc(ctx, method->declLoc);
            }
        }
        if (sym.exists()) {
            // if the symbol does exist, then we're running in incremental mode, and we need to compare it to the
            // previously defined equivalent to re-report any errors
            auto replacedSym = ctx.state.findRenamedSymbol(owner, sym);
            if (replacedSym.exists() && !paramsMatch(ctx, replacedSym, parsedArgs) && !isIntrinsic(ctx, replacedSym)) {
                paramMismatchErrors(ctx.withOwner(replacedSym), method->declLoc, parsedArgs);
            }
            sym.data(ctx)->addLoc(ctx, method->declLoc);
            method->symbol = sym;
            method->args = fillInArgs(ctx.withOwner(method->symbol), move(parsedArgs));
            return method;
        }

        // we'll only get this far if we're not in incremental mode, so enter a new symbol and fill in the data
        // appropriately
        method->symbol = ctx.state.enterMethodSymbol(method->declLoc, owner, method->name);
        method->args = fillInArgs(ctx.withOwner(method->symbol), move(parsedArgs));
        method->symbol.data(ctx)->addLoc(ctx, method->declLoc);
        if (method->isDSLSynthesized()) {
            method->symbol.data(ctx)->setDSLSynthesized();
        }
        return method;
    }

    unique_ptr<ast::MethodDef> postTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> method) {
        ENFORCE(method->args.size() == method->symbol.data(ctx)->arguments().size());
        ENFORCE(method->args.size() == method->symbol.data(ctx)->arguments().size(), "{}: {} != {}",
                method->name.showRaw(ctx), method->args.size(), method->symbol.data(ctx)->arguments().size());
        // Not all information is unfortunately available in the symbol. Original argument names aren't.
        // method->args.clear();
        return method;
    }

    unique_ptr<ast::Expression> postTransformUnresolvedIdent(core::MutableContext ctx,
                                                             unique_ptr<ast::UnresolvedIdent> nm) {
        ENFORCE(nm->kind != ast::UnresolvedIdent::Local, "Unresolved local left after `name_locals`");

        if (nm->kind == ast::UnresolvedIdent::Global) {
            auto sym = ctx.state.lookupSymbol(core::Symbols::root(), nm->name);
            if (!sym.exists()) {
                sym = ctx.state.enterFieldSymbol(nm->loc, core::Symbols::root(), nm->name);
            }
            return make_unique<ast::Field>(nm->loc, sym);
        } else {
            return nm;
        }
    }

    // Returns the SymbolRef corresponding to the class `self.class`, unless the
    // context is a class, in which case return it.
    core::SymbolRef contextClass(core::GlobalState &gs, core::SymbolRef ofWhat) const {
        core::SymbolRef owner = ofWhat;
        while (true) {
            ENFORCE(owner.exists(), "non-existing owner in contextClass");
            const auto &data = owner.data(gs);

            if (data->isClassOrModule()) {
                break;
            }
            if (data->name == core::Names::staticInit()) {
                owner = data->owner.data(gs)->attachedClass(gs);
            } else {
                owner = data->owner;
            }
        }
        return owner;
    }

    unique_ptr<ast::Assign> fillAssign(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        // forbid dynamic constant definition
        auto ownerData = ctx.owner.data(ctx);
        if (!ownerData->isClassOrModule() && !ownerData->isDSLSynthesized()) {
            if (auto e = ctx.state.beginError(asgn->loc, core::errors::Namer::DynamicConstantAssignment)) {
                e.setHeader("Dynamic constant assignment");
            }
        }

        auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        ENFORCE(lhs);
        core::SymbolRef scope = squashNames(ctx, contextClass(ctx, ctx.owner), lhs->scope);
        if (!scope.data(ctx)->isClassOrModule()) {
            if (auto e = ctx.state.beginError(asgn->loc, core::errors::Namer::InvalidClassOwner)) {
                auto constLitName = lhs->cnst.data(ctx)->show(ctx);
                auto scopeName = scope.data(ctx)->show(ctx);
                e.setHeader("Can't nest `{}` under `{}` because `{}` is not a class or module", constLitName, scopeName,
                            scopeName);
                e.addErrorLine(scope.data(ctx)->loc(), "`{}` defined here", scopeName);
            }
            // Mangle this one out of the way, and re-enter a symbol with this name as a class.
            auto scopeName = scope.data(ctx)->name;
            ctx.state.mangleRenameSymbol(scope, scopeName);
            scope = ctx.state.enterClassSymbol(lhs->scope->loc, scope.data(ctx)->owner, scopeName);
            scope.data(ctx)->singletonClass(ctx); // force singleton class into existance
        }

        auto sym = ctx.state.lookupStaticFieldSymbol(scope, lhs->cnst);
        auto currSym = ctx.state.lookupSymbol(scope, lhs->cnst);
        auto name = sym.exists() ? sym.data(ctx)->name : lhs->cnst;
        if (!sym.exists() && currSym.exists()) {
            if (auto e = ctx.state.beginError(asgn->loc, core::errors::Namer::ModuleKindRedefinition)) {
                e.setHeader("Redefining constant `{}`", lhs->cnst.data(ctx)->show(ctx));
                e.addErrorLine(asgn->loc, "Previous definition");
            }
            ctx.state.mangleRenameSymbol(currSym, currSym.data(ctx)->name);
        }
        if (sym.exists()) {
            // if sym exists, then currSym should definitely exist
            ENFORCE(currSym.exists());
            auto renamedSym = ctx.state.findRenamedSymbol(scope, sym);
            if (renamedSym.exists()) {
                if (auto e = ctx.state.beginError(sym.data(ctx)->loc(), core::errors::Namer::ModuleKindRedefinition)) {
                    e.setHeader("Redefining constant `{}`", renamedSym.data(ctx)->name.show(ctx));
                    e.addErrorLine(asgn->loc, "Previous definition");
                }
            }
        }
        core::SymbolRef cnst = ctx.state.enterStaticFieldSymbol(lhs->loc, scope, name);
        auto loc = lhs->loc;
        unique_ptr<ast::UnresolvedConstantLit> lhsU(lhs);
        asgn->lhs.release();
        asgn->lhs = make_unique<ast::ConstantLit>(loc, cnst, std::move(lhsU));
        return asgn;
    }

    unique_ptr<ast::Expression> handleTypeMemberDefinition(core::MutableContext ctx, const ast::Send *send,
                                                           unique_ptr<ast::Assign> asgn,
                                                           const ast::UnresolvedConstantLit *typeName) {
        ENFORCE(asgn->lhs.get() == typeName &&
                asgn->rhs.get() == send); // this method assumes that `asgn` owns `send` and `typeName`
        core::Variance variance = core::Variance::Invariant;
        bool isTypeTemplate = send->fun == core::Names::typeTemplate();
        if (!ctx.owner.data(ctx)->isClassOrModule()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                e.setHeader("Types must be defined in class or module scopes");
            }
            return make_unique<ast::EmptyTree>();
        }
        if (ctx.owner == core::Symbols::root()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::RootTypeMember)) {
                e.setHeader("`{}` cannot be used at the top-level", "type_member");
            }
            auto send =
                ast::MK::Send1(asgn->loc, ast::MK::T(asgn->loc), core::Names::typeAlias(), ast::MK::Untyped(asgn->loc));
            return handleAssignment(ctx, make_unique<ast::Assign>(asgn->loc, std::move(asgn->lhs), std::move(send)));
        }

        auto onSymbol = isTypeTemplate ? ctx.owner.data(ctx)->singletonClass(ctx) : ctx.owner;
        if (!send->args.empty()) {
            if (send->args.size() > 2) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Too many args in type definition");
                }
                auto send = ast::MK::Send1(asgn->loc, ast::MK::T(asgn->loc), core::Names::typeAlias(),
                                           ast::MK::Untyped(asgn->loc));
                return handleAssignment(ctx,
                                        make_unique<ast::Assign>(asgn->loc, std::move(asgn->lhs), std::move(send)));
            }

            auto lit = ast::cast_tree<ast::Literal>(send->args[0].get());
            if (lit != nullptr && lit->isSymbol(ctx)) {
                core::NameRef name = lit->asSymbol(ctx);

                if (name == core::Names::covariant()) {
                    variance = core::Variance::CoVariant;
                } else if (name == core::Names::contravariant()) {
                    variance = core::Variance::ContraVariant;
                } else if (name == core::Names::invariant()) {
                    variance = core::Variance::Invariant;
                } else {
                    if (auto e = ctx.state.beginError(lit->loc, core::errors::Namer::InvalidTypeDefinition)) {
                        e.setHeader("Invalid variance kind, only `{}` and `{}` are supported",
                                    ":" + core::Names::covariant().show(ctx),
                                    ":" + core::Names::contravariant().show(ctx));
                    }
                }
            } else {
                if (send->args.size() != 1 || ast::cast_tree<ast::Hash>(send->args[0].get()) == nullptr) {
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                        e.setHeader("Invalid param, must be a :symbol");
                    }
                }
            }
        }

        core::SymbolRef sym;
        auto existingTypeMember = ctx.state.lookupTypeMemberSymbol(onSymbol, typeName->cnst);
        if (existingTypeMember.exists()) {
            // if we already have a type member but it was constructed in a different file from the one we're looking
            // at, then we need to raise an error
            if (existingTypeMember.data(ctx)->loc().file() != asgn->loc.file()) {
                if (auto e = ctx.state.beginError(asgn->loc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Duplicate type member `{}`", typeName->cnst.data(ctx)->show(ctx));
                    e.addErrorLine(existingTypeMember.data(ctx)->loc(), "Also defined here");
                }
                if (auto e = ctx.state.beginError(existingTypeMember.data(ctx)->loc(),
                                                  core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Duplicate type member `{}`", typeName->cnst.data(ctx)->show(ctx));
                    e.addErrorLine(asgn->loc, "Also defined here");
                }
            }

            // otherwise, we're looking at a type member defined in this class in the same file, which means all we need
            // to do is find out whether there was a redefinition the first time, and in that case display the same
            // error
            auto oldSym = ctx.state.findRenamedSymbol(onSymbol, existingTypeMember);
            if (oldSym.exists()) {
                if (auto e = ctx.state.beginError(typeName->loc, core::errors::Namer::ModuleKindRedefinition)) {
                    e.setHeader("Redefining constant `{}`", oldSym.data(ctx)->show(ctx));
                    e.addErrorLine(oldSym.data(ctx)->loc(), "Previous definition");
                }
            }
            // if we have more than one type member with the same name, then we have messed up somewhere
            ENFORCE(absl::c_find_if(onSymbol.data(ctx)->typeMembers(), [&](auto mem) {
                        return mem.data(ctx)->name == existingTypeMember.data(ctx)->name;
                    }) != onSymbol.data(ctx)->typeMembers().end());
            sym = existingTypeMember;
        } else {
            auto oldSym = onSymbol.data(ctx)->findMemberNoDealias(ctx, typeName->cnst);
            if (oldSym.exists()) {
                if (auto e = ctx.state.beginError(typeName->loc, core::errors::Namer::ModuleKindRedefinition)) {
                    e.setHeader("Redefining constant `{}`", oldSym.data(ctx)->show(ctx));
                    e.addErrorLine(oldSym.data(ctx)->loc(), "Previous definition");
                }
                ctx.state.mangleRenameSymbol(oldSym, oldSym.data(ctx)->name);
            }
            sym = ctx.state.enterTypeMember(asgn->loc, onSymbol, typeName->cnst, variance);

            // The todo bounds will be fixed by the resolver in ResolveTypeParamsWalk.
            auto todo = core::make_type<core::ClassType>(core::Symbols::todo());
            sym.data(ctx)->resultType = core::make_type<core::LambdaParam>(sym, todo, todo);

            if (isTypeTemplate) {
                auto context = ctx.owner.data(ctx)->enclosingClass(ctx);
                oldSym = context.data(ctx)->findMemberNoDealias(ctx, typeName->cnst);
                if (oldSym.exists() &&
                    !(oldSym.data(ctx)->loc() == asgn->loc || oldSym.data(ctx)->loc().isTombStoned(ctx))) {
                    if (auto e = ctx.state.beginError(typeName->loc, core::errors::Namer::ModuleKindRedefinition)) {
                        e.setHeader("Redefining constant `{}`", typeName->cnst.data(ctx)->show(ctx));
                        e.addErrorLine(oldSym.data(ctx)->loc(), "Previous definition");
                    }
                    ctx.state.mangleRenameSymbol(oldSym, typeName->cnst);
                }
                auto alias = ctx.state.enterStaticFieldSymbol(asgn->loc, context, typeName->cnst);
                alias.data(ctx)->resultType = core::make_type<core::AliasType>(sym);
            }
        }

        if (!send->args.empty()) {
            auto *hash = ast::cast_tree<ast::Hash>(send->args.back().get());
            if (hash) {
                int i = -1;

                bool fixed = false;
                bool bounded = false;

                for (auto &keyExpr : hash->keys) {
                    i++;
                    auto key = ast::cast_tree<ast::Literal>(keyExpr.get());
                    core::NameRef name;
                    if (key != nullptr && key->isSymbol(ctx)) {
                        switch (key->asSymbol(ctx)._id) {
                            case core::Names::fixed()._id:
                                sym.data(ctx)->setFixed();
                                fixed = true;
                                break;

                            case core::Names::lower()._id:
                            case core::Names::upper()._id:
                                bounded = true;
                                break;
                        }
                    }
                }

                // one of fixed or bounds were provided
                if (fixed != bounded) {
                    asgn->lhs = ast::MK::Constant(asgn->lhs->loc, sym);

                    // Leave it in the tree for the resolver to chew on.
                    return asgn;
                } else if (fixed) {
                    // both fixed and bounds were specified
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                        e.setHeader("Type member is defined with bounds and `{}`", "fixed");
                    }
                } else {
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                        e.setHeader("Missing required param `{}`", "fixed");
                    }
                }
            }
        }

        return asgn;
    }

    unique_ptr<ast::Expression> handleAssignment(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        auto *send = ast::cast_tree<ast::Send>(asgn->rhs.get());
        auto ret = fillAssign(ctx, std::move(asgn));
        if (send->fun == core::Names::typeAlias()) {
            auto id = ast::cast_tree<ast::ConstantLit>(ret->lhs.get());
            ENFORCE(id != nullptr, "fillAssign did not make lhs into a ConstantLit");

            auto sym = id->symbol;
            ENFORCE(sym.exists(), "fillAssign did not make symbol for ConstantLit");

            if (sym.data(ctx)->isStaticField()) {
                sym.data(ctx)->setTypeAlias();
            }
        }
        return ret;
    }

    unique_ptr<ast::Expression> postTransformAssign(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        if (lhs == nullptr) {
            return asgn;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn->rhs.get());
        if (send == nullptr) {
            return fillAssign(ctx, std::move(asgn));
        }

        if (!send->recv->isSelfReference()) {
            return handleAssignment(ctx, std::move(asgn));
        }

        auto *typeName = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        if (typeName == nullptr) {
            return fillAssign(ctx, std::move(asgn));
        }

        switch (send->fun._id) {
            case core::Names::typeTemplate()._id:
                return handleTypeMemberDefinition(ctx, send, std::move(asgn), typeName);
            case core::Names::typeMember()._id:
                return handleTypeMemberDefinition(ctx, send, std::move(asgn), typeName);
            default:
                return fillAssign(ctx, std::move(asgn));
        }
    }

private:
    UnorderedMap<core::SymbolRef, core::Loc> classBehaviorLocs;
};

std::vector<ast::ParsedFile> Namer::run(core::MutableContext ctx, std::vector<ast::ParsedFile> trees) {
    NameInserter nameInserter;
    for (auto &tree : trees) {
        auto file = tree.file;
        try {
            ast::ParsedFile ast;
            {
                Timer timeit(ctx.state.tracer(), "naming", {{"file", (string)file.data(ctx).path()}});
                core::ErrorRegion errs(ctx.state, file);
                tree.tree = ast::TreeMap::apply(ctx, nameInserter, std::move(tree.tree));
            }
        } catch (SorbetException &) {
            Exception::failInFuzzer();
            if (auto e = ctx.state.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
                e.setHeader("Exception naming file: `{}` (backtrace is above)", file.data(ctx).path());
            }
        }
    }
    return trees;
}

}; // namespace sorbet::namer
