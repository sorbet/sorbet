#include "namer/namer.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/core.h"
#include "core/errors/namer.h"

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
                return id->constantSymbol().data(ctx)->dealias(ctx);
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
            if (!newOwner.data(ctx)->isClass()) {
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
        node = make_unique<ast::ConstantLit>(constLit->loc, existing, std::move(constTmp), nullptr);
        return existing;
    }

    struct ParsedArg {
        core::NameRef name;
        core::Loc loc;
        unique_ptr<ast::Expression> default_;
        bool keyword = false;
        bool block = false;
        bool repeated = false;
        bool shadow = false;
    };

    ParsedArg parseArg(core::MutableContext ctx, unique_ptr<ast::Reference> arg) {
        ParsedArg parsedArg;

        typecase(arg.get(),
                 [&](ast::UnresolvedIdent *nm) {
                     parsedArg.name = nm->name;
                     parsedArg.loc = nm->loc;
                 },
                 [&](ast::RestArg *rest) {
                     parsedArg = parseArg(ctx, move(rest->expr));
                     parsedArg.repeated = true;
                 },
                 [&](ast::KeywordArg *kw) {
                     parsedArg = parseArg(ctx, move(kw->expr));
                     parsedArg.keyword = true;
                 },
                 [&](ast::OptionalArg *opt) {
                     parsedArg = parseArg(ctx, move(opt->expr));
                     parsedArg.default_ = move(opt->default_);
                 },
                 [&](ast::BlockArg *blk) {
                     parsedArg = parseArg(ctx, move(blk->expr));
                     parsedArg.block = true;
                 },
                 [&](ast::ShadowArg *shadow) {
                     parsedArg = parseArg(ctx, move(shadow->expr));
                     parsedArg.shadow = true;
                 },
                 [&](ast::Local *local) {
                     // Namer replaces args with locals, so to make namer idempotent,
                     // we need to be able to handle Locals here.
                     parsedArg.name = local->localVariable._name;
                     parsedArg.loc = local->loc;
                 });
        return parsedArg;
    }

    pair<core::SymbolRef, unique_ptr<ast::Expression>> arg2Symbol(core::MutableContext ctx, int pos,
                                                                  ParsedArg parsedArg) {
        if (pos < ctx.owner.data(ctx)->arguments().size()) {
            // TODO: check that flags match;
            core::SymbolRef sym = ctx.owner.data(ctx)->arguments()[pos];
            core::LocalVariable localVariable = enterLocal(ctx, parsedArg.name);
            auto localExpr = make_unique<ast::Local>(parsedArg.loc, localVariable);
            return make_pair(sym, move(localExpr));
        }
        core::NameRef name;
        if (parsedArg.keyword) {
            name = parsedArg.name;
        } else if (parsedArg.block) {
            name = core::Names::blkArg();
        } else {
            name = ctx.state.freshNameUnique(core::UniqueNameKind::PositionalArg, core::Names::arg(), pos + 1);
        }
        core::SymbolRef sym = ctx.state.enterMethodArgumentSymbol(parsedArg.loc, ctx.owner, name);
        core::LocalVariable localVariable = enterLocal(ctx, parsedArg.name);
        unique_ptr<ast::Reference> localExpr = make_unique<ast::Local>(parsedArg.loc, localVariable);

        core::SymbolData data = sym.data(ctx);
        if (parsedArg.default_) {
            data->setOptional();
            localExpr = make_unique<ast::OptionalArg>(parsedArg.loc, move(localExpr), move(parsedArg.default_));
        }
        if (parsedArg.keyword) {
            data->setKeyword();
        }
        if (parsedArg.block) {
            data->setBlockArgument();
        }
        if (parsedArg.repeated) {
            data->setRepeated();
        }
        return make_pair(sym, move(localExpr));
    }

    struct LocalFrame {
        UnorderedMap<core::NameRef, core::LocalVariable> locals;
        vector<core::LocalVariable> args;
        bool moduleFunctionActive;
        u4 localId;
        std::optional<u4> oldBlockCounter;
    };

    LocalFrame &enterBlock() {
        scopeStack.emplace_back().localId = blockCounter;
        ++blockCounter;
        return scopeStack.back();
    }

    LocalFrame &enterClassOrMethod() {
        scopeStack.emplace_back().localId = 0;
        scopeStack.back().oldBlockCounter = blockCounter;
        blockCounter = 1;
        return scopeStack.back();
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
            dest = &klass->singleton_ancestors;
        } else {
            return false;
        }
        if (!ast::isa_tree<ast::Self>(send->recv.get())) {
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
            if (ast::isa_tree<ast::Self>(arg.get())) {
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

    void aliasModuleFunction(core::MutableContext ctx, core::Loc loc, core::SymbolRef method) {
        core::SymbolRef owner = method.data(ctx)->owner;
        aliasMethod(ctx, loc, owner.data(ctx)->singletonClass(ctx), method.data(ctx)->name, method);
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
        if (ast::isa_tree<ast::EmptyTree>(exp) || ast::isa_tree<ast::Self>(exp) ||
            ast::isa_tree<ast::ConstantLit>(exp)) {
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
                // Desugar populates a top-level root() ClassDef. Nothing else
                // should have been typeAlias by now.
                ENFORCE(klass->symbol == core::Symbols::root());
            }
            bool isModule = klass->kind == ast::ClassDefKind::Module;
            if (!klass->symbol.data(ctx)->isClass()) {
                if (auto e = ctx.state.beginError(klass->loc, core::errors::Namer::ModuleKindRedefinition)) {
                    e.setHeader("Redefining constant `{}`", klass->symbol.data(ctx)->show(ctx));
                    e.addErrorLine(klass->symbol.data(ctx)->loc(), "Previous definition");
                }
                auto origName = klass->symbol.data(ctx)->name;
                ctx.state.mangleRenameSymbol(klass->symbol, klass->symbol.data(ctx)->name);
                klass->symbol = ctx.state.enterClassSymbol(klass->declLoc, klass->symbol.data(ctx)->owner, origName);

                auto oldSymCount = ctx.state.symbolsUsed();
                auto newSignleton =
                    klass->symbol.data(ctx)->singletonClass(ctx); // force singleton class into existence
                ENFORCE(newSignleton._id >= oldSymCount,
                        "should be a fresh symbol. Otherwise we could be reusing an existing singletonClass");
            } else if (klass->symbol.data(ctx)->isClassModuleSet() &&
                       isModule != klass->symbol.data(ctx)->isClassModule()) {
                if (auto e = ctx.state.beginError(klass->loc, core::errors::Namer::ModuleKindRedefinition)) {
                    e.setHeader("`{}` was previously defined as a `{}`", klass->symbol.data(ctx)->show(ctx),
                                klass->symbol.data(ctx)->isClassModule() ? "module" : "class");
                }
            } else {
                klass->symbol.data(ctx)->setIsModule(isModule);
            }
        }
        enterClassOrMethod();
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
        if (send->fun != core::Names::declareInterface() && send->fun != core::Names::declareAbstract()) {
            return false;
        }

        klass->symbol.data(ctx)->setClassAbstract();
        klass->symbol.data(ctx)->singletonClass(ctx).data(ctx)->setClassAbstract();

        if (send->fun == core::Names::declareInterface()) {
            klass->symbol.data(ctx)->setClassInterface();

            if (klass->kind == ast::Class) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InterfaceClass)) {
                    e.setHeader("Classes can't be interfaces. Use `abstract!` instead of `interface!`");
                }
            }
        }
        return true;
    }

    // This decides if we need to keep a node around incase the current LSP query needs type information for it
    bool shouldLeaveAncestorForIDE(const unique_ptr<ast::Expression> &anc) {
        // used in Desugar <-> resolver to signal classes that did not have explicit superclass
        if (ast::isa_tree<ast::EmptyTree>(anc.get()) || ast::isa_tree<ast::Self>(anc.get())) {
            return false;
        }
        auto rcl = ast::cast_tree<ast::ConstantLit>(anc.get());
        if (rcl && rcl->typeAliasOrConstantSymbol() == core::Symbols::todo()) {
            return false;
        }
        return true;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> klass) {
        exitScope();
        if (klass->kind == ast::Class && !klass->symbol.data(ctx)->superClass.exists() &&
            klass->symbol != core::Symbols::BasicObject()) {
            klass->symbol.data(ctx)->superClass = core::Symbols::todo();
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
        ast::InsSeq::STATS_store ideSeqs;
        if (ast::isa_tree<ast::ConstantLit>(klass->name.get())) {
            ideSeqs.emplace_back(ast::MK::KeepForIDE(klass->name->deepCopy()));
        }
        if (klass->kind == ast::Class && !klass->ancestors.empty() &&
            shouldLeaveAncestorForIDE(klass->ancestors.front())) {
            ideSeqs.emplace_back(ast::MK::KeepForIDE(klass->ancestors.front()->deepCopy()));
        }
        return ast::MK::InsSeq(klass->declLoc, std::move(ideSeqs), std::move(klass));
    }

    core::LocalVariable enterLocal(core::MutableContext ctx, core::NameRef name) {
        if (!ctx.owner.data(ctx)->isBlockSymbol(ctx)) {
            return core::LocalVariable(name, 0);
        }
        return core::LocalVariable(name, scopeStack.back().localId);
    }

    ast::MethodDef::ARGS_store fillInArgs(core::MutableContext ctx, vector<ParsedArg> parsedArgs) {
        ast::MethodDef::ARGS_store args;
        bool inShadows = false;

        if (isIntrinsic(ctx, ctx.owner)) {
            // When we're filling in an intrinsic method, we want to overwrite the block arg that used
            // to exist with the block arg that we got from desugaring the method def in the RBI files.
            ENFORCE(ctx.owner.data(ctx)->arguments().back().data(ctx)->isBlockArgument());
            ctx.owner.data(ctx)->arguments().clear();
        }

        int i = -1;
        for (auto &arg : parsedArgs) {
            i++;
            auto name = arg.name;
            auto localVariable = enterLocal(ctx, name);

            if (arg.shadow) {
                inShadows = true;
                auto localExpr = make_unique<ast::Local>(arg.loc, localVariable);
                args.emplace_back(move(localExpr));
            } else {
                ENFORCE(!inShadows, "shadow argument followed by non-shadow argument!");
                auto pair = arg2Symbol(ctx, i, move(arg));
                core::SymbolRef sym = pair.first;
                args.emplace_back(move(pair.second));
                if (i < ctx.owner.data(ctx)->arguments().size()) {
                    ENFORCE(ctx.owner.data(ctx)->arguments()[i] == sym);
                } else {
                    ctx.owner.data(ctx)->arguments().emplace_back(sym);
                }
            }

            scopeStack.back().locals[name] = localVariable;
            scopeStack.back().args.emplace_back(localVariable);
        }

        return args;
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
                case core::Names::moduleFunction()._id:
                    aliasModuleFunction(ctx, original->loc, mdef->symbol);
                    break;
                default:
                    return original;
            }
            return std::move(original->args[0]);
        }
        if (ast::isa_tree<ast::Self>(original->recv.get())) {
            switch (original->fun._id) {
                case core::Names::moduleFunction()._id: {
                    if (original->args.empty()) {
                        scopeStack.back().moduleFunctionActive = true;
                        break;
                    }
                    for (auto &arg : original->args) {
                        auto lit = ast::cast_tree<ast::Literal>(arg.get());
                        if (lit == nullptr || !lit->isSymbol(ctx)) {
                            if (auto e = ctx.state.beginError(arg->loc, core::errors::Namer::DynamicDSLInvocation)) {
                                e.setHeader("Unsupported argument to `{}`: arguments must be symbol literals",
                                            original->fun.show(ctx));
                            }
                            continue;
                        }
                        core::NameRef name = lit->asSymbol(ctx);

                        core::SymbolRef meth = methodOwner(ctx).data(ctx)->findMember(ctx, name);
                        if (!meth.exists()) {
                            if (auto e = ctx.state.beginError(arg->loc, core::errors::Namer::MethodNotFound)) {
                                e.setHeader("`{}`: no such method: `{}`", original->fun.show(ctx), name.show(ctx));
                            }
                            continue;
                        }
                        aliasModuleFunction(ctx, original->loc, meth);
                    }
                    break;
                }
            }
        }

        return original;
    }

    // Allow stub symbols created to hold intrinsics to be filled in
    // with real types from code
    bool isIntrinsic(core::Context ctx, core::SymbolRef sym) {
        auto data = sym.data(ctx);
        return data->intrinsic != nullptr && data->resultType == nullptr;
    }

    bool paramsMatch(core::MutableContext ctx, core::Loc loc, const vector<ParsedArg> &parsedArgs) {
        auto sym = ctx.owner.data(ctx)->dealias(ctx);
        if (sym.data(ctx)->arguments().size() != parsedArgs.size()) {
            if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                if (sym != ctx.owner) {
                    // TODO(jez) Subtracting 1 because of the block arg we added everywhere.
                    // Eventually we should be more principled about how we report this.
                    e.setHeader(
                        "Method alias `{}` redefined without matching argument count. Expected: `{}`, got: `{}`",
                        ctx.owner.data(ctx)->show(ctx), sym.data(ctx)->arguments().size() - 1, parsedArgs.size() - 1);
                    e.addErrorLine(ctx.owner.data(ctx)->loc(), "Previous alias definition");
                    e.addErrorLine(sym.data(ctx)->loc(), "Dealiased definition");
                } else {
                    // TODO(jez) Subtracting 1 because of the block arg we added everywhere.
                    // Eventually we should be more principled about how we report this.
                    e.setHeader("Method `{}` redefined without matching argument count. Expected: `{}`, got: `{}`",
                                sym.data(ctx)->show(ctx), sym.data(ctx)->arguments().size() - 1, parsedArgs.size() - 1);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
            }
            return false;
        }
        for (int i = 0; i < parsedArgs.size(); i++) {
            auto &methodArg = parsedArgs[i];
            auto symArg = sym.data(ctx)->arguments()[i].data(ctx);

            if (symArg->isKeyword() != methodArg.keyword) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader(
                        "Method `{}` redefined with mismatched argument attribute `{}`. Expected: `{}`, got: `{}`",
                        sym.data(ctx)->show(ctx), "isKeyword", symArg->isKeyword(), methodArg.keyword);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return false;
            }
            if (symArg->isBlockArgument() != methodArg.block) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader(
                        "Method `{}` redefined with mismatched argument attribute `{}`. Expected: `{}`, got: `{}`",
                        sym.data(ctx)->show(ctx), "isBlock", symArg->isBlockArgument(), methodArg.block);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return false;
            }
            if (symArg->isRepeated() != methodArg.repeated) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader(
                        "Method `{}` redefined with mismatched argument attribute `{}`. Expected: `{}`, got: `{}`",
                        sym.data(ctx)->show(ctx), "isRepeated", symArg->isRepeated(), methodArg.repeated);
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return false;
            }
            if (symArg->isKeyword() && symArg->name != methodArg.name) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader("Method `{}` redefined with mismatched argument name. Expected: `{}`, got: `{}`",
                                sym.data(ctx)->show(ctx), symArg->name.show(ctx), methodArg.name.show(ctx));
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return false;
            }
        }

        return true;
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> method) {
        enterClassOrMethod();

        core::SymbolRef owner = methodOwner(ctx);

        if (method->isSelf()) {
            if (owner.data(ctx)->isClass()) {
                owner = owner.data(ctx)->singletonClass(ctx);
            }
        }
        ENFORCE(owner.data(ctx)->isClass());

        vector<ParsedArg> parsedArgs;
        for (auto &arg : method->args) {
            auto *refExp = ast::cast_tree<ast::Reference>(arg.get());
            if (!refExp) {
                Exception::raise("Must be a reference!");
            }
            unique_ptr<ast::Reference> refExpImpl(refExp);
            arg.release();
            parsedArgs.emplace_back(parseArg(ctx, move(refExpImpl)));
        }

        auto sym = owner.data(ctx)->findMemberNoDealias(ctx, method->name);
        if (sym.exists()) {
            if (method->declLoc == sym.data(ctx)->loc()) {
                // TODO remove if the paramsMatch is perfect
                // Reparsing the same file
                method->symbol = sym;
                method->args = fillInArgs(ctx.withOwner(method->symbol), move(parsedArgs));
                return method;
            }
            if (isIntrinsic(ctx, sym) || paramsMatch(ctx.withOwner(sym), method->declLoc, parsedArgs)) {
                sym.data(ctx)->addLoc(ctx, method->declLoc);
            } else {
                ctx.state.mangleRenameSymbol(sym, method->name);
            }
        }
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
        exitScope();
        if (scopeStack.back().moduleFunctionActive) {
            aliasModuleFunction(ctx, method->symbol.data(ctx)->loc(), method->symbol);
        }
        ENFORCE(method->args.size() == method->symbol.data(ctx)->arguments().size(), "{}: {} != {}",
                method->name.toString(ctx), method->args.size(), method->symbol.data(ctx)->arguments().size());
        // Not all information is unfortunately available in the symbol. Original argument names aren't.
        // method->args.clear();
        return method;
    }

    unique_ptr<ast::Block> preTransformBlock(core::MutableContext ctx, unique_ptr<ast::Block> blk) {
        core::SymbolRef owner = ctx.owner;
        if (owner == core::Symbols::noSymbol() || owner == core::Symbols::root()) {
            // Introduce intermediate host for block.
            ENFORCE(blk->loc.exists());
            owner = ctx.state.staticInitForFile(blk->loc);
        } else if (owner.data(ctx)->isClass()) {
            // If we're at class scope, we're actually in the context of the
            // singleton class.
            owner = ctx.state.staticInitForClass(owner, blk->loc);
        }
        blk->symbol =
            ctx.state.enterMethodSymbol(blk->loc, owner,
                                        ctx.state.freshNameUnique(core::UniqueNameKind::Namer, core::Names::blockTemp(),
                                                                  ++(owner.data(ctx)->uniqueCounter)));

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
        vector<ParsedArg> parsedArgs;
        for (auto &arg : blk->args) {
            auto *refExp = ast::cast_tree<ast::Reference>(arg.get());
            if (!refExp) {
                Exception::raise("Must be a reference!");
            }
            unique_ptr<ast::Reference> refExpImpl(refExp);
            arg.release();
            parsedArgs.emplace_back(parseArg(ctx, move(refExpImpl)));
        }
        blk->args = fillInArgs(ctx.withOwner(blk->symbol), move(parsedArgs));

        return blk;
    }

    unique_ptr<ast::Block> postTransformBlock(core::MutableContext ctx, unique_ptr<ast::Block> blk) {
        exitScope();
        return blk;
    }

    unique_ptr<ast::Expression> postTransformUnresolvedIdent(core::MutableContext ctx,
                                                             unique_ptr<ast::UnresolvedIdent> nm) {
        switch (nm->kind) {
            case ast::UnresolvedIdent::Local: {
                auto &frame = scopeStack.back();
                core::LocalVariable &cur = frame.locals[nm->name];
                if (!cur.exists()) {
                    cur = enterLocal(ctx, nm->name);
                    frame.locals[nm->name] = cur;
                }
                return make_unique<ast::Local>(nm->loc, cur);
            }
            case ast::UnresolvedIdent::Global: {
                core::SymbolData root = core::Symbols::root().data(ctx);
                core::SymbolRef sym = root->findMember(ctx, nm->name);
                if (!sym.exists()) {
                    sym = ctx.state.enterFieldSymbol(nm->loc, core::Symbols::root(), nm->name);
                }
                return make_unique<ast::Field>(nm->loc, sym);
            }
            default:
                return nm;
        }
    }

    unique_ptr<ast::Self> postTransformSelf(core::MutableContext ctx, unique_ptr<ast::Self> self) {
        self->claz = ctx.selfClass();
        return self;
    }

    // Returns the SymbolRef corresponding to the class `self.class`, unless the
    // context is a class, in which case return it.
    core::SymbolRef contextClass(core::GlobalState &gs, core::SymbolRef ofWhat) const {
        core::SymbolRef owner = ofWhat;
        while (true) {
            ENFORCE(owner.exists(), "non-existing owner in contextClass");
            const auto &data = owner.data(gs);

            if (data->isClass()) {
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
        // TODO(nelhage): forbid dynamic constant definition
        auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        ENFORCE(lhs);
        core::SymbolRef scope = squashNames(ctx, contextClass(ctx, ctx.owner), lhs->scope);
        if (!scope.data(ctx)->isClass()) {
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

        auto sym = scope.data(ctx)->findMemberNoDealias(ctx, lhs->cnst);
        if (sym.exists() && !sym.data(ctx)->isStaticField()) {
            if (auto e = ctx.state.beginError(asgn->loc, core::errors::Namer::ModuleKindRedefinition)) {
                e.setHeader("Redefining constant `{}`", lhs->cnst.data(ctx)->show(ctx));
                e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
            }
            ctx.state.mangleRenameSymbol(sym, sym.data(ctx)->name);
        }
        core::SymbolRef cnst = ctx.state.enterStaticFieldSymbol(lhs->loc, scope, lhs->cnst);
        auto loc = lhs->loc;
        unique_ptr<ast::UnresolvedConstantLit> lhsU(lhs);
        asgn->lhs.release();
        asgn->lhs = make_unique<ast::ConstantLit>(loc, cnst, std::move(lhsU), nullptr);
        return asgn;
    }

    unique_ptr<ast::Expression> handleTypeMemberDefinition(core::MutableContext ctx, const ast::Send *send,
                                                           unique_ptr<ast::Assign> asgn,
                                                           const ast::UnresolvedConstantLit *typeName) {
        ENFORCE(asgn->lhs.get() == typeName &&
                asgn->rhs.get() == send); // this method assumes that `asgn` owns `send` and `typeName`
        core::Variance variance = core::Variance::Invariant;
        bool isTypeTemplate = send->fun == core::Names::typeTemplate();
        if (!ctx.owner.data(ctx)->isClass()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                e.setHeader("Types must be defined in class or module scopes");
            }
            return make_unique<ast::EmptyTree>();
        }

        auto onSymbol = isTypeTemplate ? ctx.owner.data(ctx)->singletonClass(ctx) : ctx.owner;
        if (!send->args.empty()) {
            if (send->args.size() > 2) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Too many args in type definition");
                }
                return make_unique<ast::EmptyTree>();
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

        auto members = onSymbol.data(ctx)->typeMembers();
        auto it = absl::c_find_if(members, [&](auto mem) { return mem.data(ctx)->name == typeName->cnst; });
        if (it != members.end() && !(it->data(ctx)->loc() == asgn->loc || it->data(ctx)->loc().isTombStoned(ctx))) {
            if (auto e = ctx.state.beginError(typeName->loc, core::errors::Namer::InvalidTypeDefinition)) {
                e.setHeader("Duplicate type member `{}`", typeName->cnst.data(ctx)->show(ctx));
            }
            return make_unique<ast::EmptyTree>();
        }
        auto oldSym = onSymbol.data(ctx)->findMemberNoDealias(ctx, typeName->cnst);
        if (oldSym.exists() && !(oldSym.data(ctx)->loc() == asgn->loc || oldSym.data(ctx)->loc().isTombStoned(ctx))) {
            if (auto e = ctx.state.beginError(typeName->loc, core::errors::Namer::InvalidTypeDefinition)) {
                e.setHeader("Redefining constant `{}`", oldSym.data(ctx)->show(ctx));
                e.addErrorLine(oldSym.data(ctx)->loc(), "Previous definition");
            }
            ctx.state.mangleRenameSymbol(oldSym, oldSym.data(ctx)->name);
        }
        auto sym = ctx.state.enterTypeMember(asgn->loc, onSymbol, typeName->cnst, variance);
        if (isTypeTemplate) {
            auto context = ctx.owner.data(ctx)->enclosingClass(ctx);
            oldSym = context.data(ctx)->findMemberNoDealias(ctx, typeName->cnst);
            if (oldSym.exists() &&
                !(oldSym.data(ctx)->loc() == asgn->loc || oldSym.data(ctx)->loc().isTombStoned(ctx))) {
                if (auto e = ctx.state.beginError(typeName->loc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Redefining constant `{}`", typeName->cnst.data(ctx)->show(ctx));
                    e.addErrorLine(oldSym.data(ctx)->loc(), "Previous definition");
                }
                ctx.state.mangleRenameSymbol(oldSym, typeName->cnst);
            }
            auto alias = ctx.state.enterStaticFieldSymbol(asgn->loc, context, typeName->cnst);
            alias.data(ctx)->resultType = core::make_type<core::AliasType>(sym);
        }

        if (!send->args.empty()) {
            auto *hash = ast::cast_tree<ast::Hash>(send->args.back().get());
            if (hash) {
                int i = -1;
                for (auto &keyExpr : hash->keys) {
                    i++;
                    auto key = ast::cast_tree<ast::Literal>(keyExpr.get());
                    core::NameRef name;
                    if (key != nullptr && key->isSymbol(ctx) && key->asSymbol(ctx) == core::Names::fixed()) {
                        // Leave it in the tree for the resolver to chew on.
                        sym.data(ctx)->setFixed();

                        // TODO(nelhage): This creates an order
                        // dependency in the resolver. See RUBYPLAT-520
                        sym.data(ctx)->resultType = core::Types::untyped(ctx, sym);

                        asgn->lhs = ast::MK::Constant(asgn->lhs->loc, sym);
                        return asgn;
                    }
                }
                if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Missing required param :fixed");
                }
            }
        }
        return make_unique<ast::EmptyTree>();
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
        auto *shouldBeSelf = ast::cast_tree<ast::Self>(send->recv.get());
        if (shouldBeSelf == nullptr) {
            auto ret = fillAssign(ctx, std::move(asgn));
            if (send->fun == core::Names::typeAlias()) {
                core::SymbolRef sym;
                if (auto id = ast::cast_tree<ast::ConstantLit>(ret->lhs.get())) {
                    sym = id->constantSymbol();
                }
                if (sym.exists() && sym.data(ctx)->isStaticField()) {
                    sym.data(ctx)->setStaticTypeAlias();
                }
            }
            return ret;
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
    NameInserter() {
        blockCounter = 0;
        enterBlock();
    }
};

ast::ParsedFile Namer::run(core::MutableContext ctx, ast::ParsedFile tree) {
    NameInserter nameInserter;
    tree.tree = ast::TreeMap::apply(ctx, nameInserter, std::move(tree.tree));
    // This check is FAR to slow to run on large codebases, especially with sanitizers on.
    // But it can be super useful to uncomment when debugging certain issues.
    // ctx.state.sanityCheck();
    return tree;
}

}; // namespace sorbet::namer
