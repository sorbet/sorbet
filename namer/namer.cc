#include "namer/namer.h"
#include "absl/algorithm/container.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "core/Context.h"
#include "core/Names/namer.h"
#include "core/Symbols.h"
#include "core/core.h"
#include "core/errors/namer.h"

using namespace std;

namespace sorbet {
namespace namer {

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
                return id->symbol.data(ctx).dealias(ctx);
            }
            if (auto *uid = ast::cast_tree<ast::UnresolvedIdent>(node.get())) {
                // emitted via `class << self` blocks
                ENFORCE(uid->kind == ast::UnresolvedIdent::Class);
                ENFORCE(uid->name == core::Names::singleton());
            } else {
                ENFORCE(ast::isa_tree<ast::EmptyTree>(node.get()), "scope is a ", node->nodeName());
            }
            node = ast::MK::EmptyTree(node->loc);
            return owner;
        }

        auto newOwner = squashNames(ctx, owner, constLit->scope);
        core::SymbolRef existing = newOwner.data(ctx).findMember(ctx, constLit->cnst);
        if (!existing.exists()) {
            existing = ctx.state.enterClassSymbol(constLit->loc, newOwner, constLit->cnst);
            existing.data(ctx).singletonClass(ctx); // force singleton class into existance
        }

        node.release();
        unique_ptr<ast::UnresolvedConstantLit> constTmp(constLit);
        node = make_unique<ast::ConstantLit>(constLit->loc, existing, move(constTmp), nullptr);
        return existing;
    }

    pair<core::SymbolRef, core::NameRef> arg2Symbol(core::MutableContext ctx, ast::Expression *arg, int pos) {
        auto loc = arg->loc;
        bool optional = false, keyword = false, block = false, repeated = false;

        while (true) {
            if (auto *nm = ast::cast_tree<ast::UnresolvedIdent>(arg)) {
                if (pos < ctx.owner.data(ctx).arguments().size()) {
                    core::SymbolRef sym = ctx.owner.data(ctx).arguments()[pos];
                    // TODO: check that flags match;
                    return make_pair(sym, nm->name);
                }
                core::SymbolRef sym = ctx.state.enterMethodArgumentSymbol(loc, ctx.owner, nm->name);
                core::Symbol &data = sym.data(ctx);
                if (optional) {
                    data.setOptional();
                }
                if (keyword) {
                    data.setKeyword();
                }
                if (block) {
                    data.setBlockArgument();
                }
                if (repeated) {
                    data.setRepeated();
                }
                return make_pair(sym, nm->name);
            }
            typecase(arg,
                     [&](ast::RestArg *rest) {
                         repeated = true;
                         arg = rest->expr.get();
                     },
                     [&](ast::KeywordArg *kw) {
                         keyword = true;
                         arg = kw->expr.get();
                     },
                     [&](ast::OptionalArg *opt) {
                         optional = true;
                         arg = opt->expr.get();
                     },
                     [&](ast::BlockArg *opt) {
                         block = true;
                         arg = opt->expr.get();
                     },
                     [&](ast::ShadowArg *opt) { arg = opt->expr.get(); });
        }
    }

    struct LocalFrame {
        UnorderedMap<core::NameRef, core::LocalVariable> locals;
        vector<core::LocalVariable> args;
        bool moduleFunctionActive;
    };

    vector<LocalFrame> scopeStack;
    u4 scopeId;

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
                e.setHeader("`{}` requires at least one argument", send->fun.data(ctx).show(ctx));
            }
            return false;
        }

        if (send->block != nullptr) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::IncludePassedBlock)) {
                e.setHeader("`{}` can not be passed a block", send->fun.data(ctx).show(ctx));
            }
            return false;
        }

        for (auto it = send->args.rbegin(); it != send->args.rend(); it++) {
            // Reverse order is intentional: that's how Ruby does it.
            auto &arg = *it;
            auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(arg.get());
            if (constLit == nullptr) {
                if (auto e = ctx.state.beginError(arg->loc, core::errors::Namer::IncludeNotConstant)) {
                    e.setHeader("`{}` must be passed a constant literal", send->fun.data(ctx).show(ctx));
                }
                return false;
            }
            dest->emplace_back(move(arg));
        }

        return true;
    }

    void aliasMethod(core::MutableContext ctx, core::SymbolRef owner, core::NameRef newName, core::SymbolRef method) {
        core::SymbolRef alias = ctx.state.enterMethodSymbol(method.data(ctx).loc(), owner, newName);
        alias.data(ctx).resultType = make_shared<core::AliasType>(method);
    }

    void aliasModuleFunction(core::MutableContext ctx, core::SymbolRef method) {
        core::SymbolRef owner = method.data(ctx).owner;
        aliasMethod(ctx, owner.data(ctx).singletonClass(ctx), method.data(ctx).name, method);
    }

    core::SymbolRef methodOwner(core::MutableContext ctx) {
        core::SymbolRef owner = ctx.owner.data(ctx).enclosingClass(ctx);
        if (owner == core::Symbols::root()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        return owner;
    }

public:
    unique_ptr<ast::ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> klass) {
        auto *ident = ast::cast_tree<ast::UnresolvedIdent>(klass->name.get());

        if ((ident != nullptr) && ident->name == core::Names::singleton()) {
            ENFORCE(ident->kind == ast::UnresolvedIdent::Class);
            klass->symbol = ctx.contextClass().data(ctx).singletonClass(ctx);
        } else {
            if (klass->symbol == core::Symbols::todo()) {
                klass->symbol = squashNames(ctx, ctx.owner, klass->name);
            } else {
                // Desugar populates a top-level root() ClassDef. Nothing else
                // should have been typeAlias by now.
                ENFORCE(klass->symbol == core::Symbols::root());
            }
            bool isModule = klass->kind == ast::ClassDefKind::Module;
            if (!klass->symbol.data(ctx).isClass()) {
                if (auto e = ctx.state.beginError(klass->loc, core::errors::Namer::ModuleKindRedefinition)) {
                    e.setHeader("Redefining constant `{}`", klass->symbol.data(ctx).show(ctx));
                    e.addErrorLine(klass->symbol.data(ctx).loc(), "Previous definition");
                }
                auto origName = klass->symbol.data(ctx).name;
                ctx.state.mangleRenameSymbol(klass->symbol, klass->symbol.data(ctx).name, core::UniqueNameKind::Namer);
                klass->symbol = ctx.state.enterClassSymbol(klass->declLoc, klass->symbol.data(ctx).owner, origName);
            } else if (klass->symbol.data(ctx).isClassModuleSet() &&
                       isModule != klass->symbol.data(ctx).isClassModule()) {
                if (auto e = ctx.state.beginError(klass->loc, core::errors::Namer::ModuleKindRedefinition)) {
                    e.setHeader("`{}` was previously defined as a `{}`", klass->symbol.data(ctx).show(ctx),
                                klass->symbol.data(ctx).isClassModule() ? "module" : "class");
                }
            } else {
                klass->symbol.data(ctx).setIsModule(isModule);
            }
        }
        scopeStack.emplace_back();
        scopeId = 0;
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

        klass->symbol.data(ctx).setClassAbstract();
        klass->symbol.data(ctx).singletonClass(ctx).data(ctx).setClassAbstract();

        if (send->fun == core::Names::declareInterface()) {
            klass->symbol.data(ctx).setClassInterface();

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
        if (rcl && rcl->symbol == core::Symbols::todo()) {
            return false;
        }
        return true;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> klass) {
        scopeStack.pop_back();
        if (klass->kind == ast::Class && !klass->symbol.data(ctx).superClass.exists() &&
            klass->symbol != core::Symbols::BasicObject()) {
            klass->symbol.data(ctx).superClass = core::Symbols::todo();
        }

        klass->symbol.data(ctx).addLoc(ctx, klass->declLoc);
        klass->symbol.data(ctx).singletonClass(ctx); // force singleton class into existance

        auto toRemove = remove_if(klass->rhs.begin(), klass->rhs.end(),
                                  [&](unique_ptr<ast::Expression> &line) { return handleNamerDSL(ctx, klass, line); });
        klass->rhs.erase(toRemove, klass->rhs.end());

        if (!klass->ancestors.empty()) {
            /* Superclass is typeAlias in parent scope, mixins are typeAlias in inner scope */
            for (auto &anc : klass->ancestors) {
                if (shouldLeaveAncestorForIDE(anc) && (klass->kind == ast::Module || anc != klass->ancestors.front())) {
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
        return ast::MK::InsSeq(klass->declLoc, move(ideSeqs), move(klass));
    }

    core::LocalVariable enterLocal(core::MutableContext ctx, core::NameRef name) {
        if (!ctx.owner.data(ctx).isBlockSymbol(ctx)) {
            return core::LocalVariable(name, 0);
        }
        return core::LocalVariable(name, scopeId);
    }

    void fillInArgs(core::MutableContext ctx, ast::MethodDef::ARGS_store &args) {
        bool inShadows = false;

        int i = -1;
        for (auto &arg : args) {
            i++;
            core::NameRef name;
            core::LocalVariable localVariable;

            if (auto *sarg = ast::cast_tree<ast::ShadowArg>(arg.get())) {
                auto id = ast::cast_tree<ast::UnresolvedIdent>(sarg->expr.get());
                ENFORCE(id != nullptr);
                name = id->name;
                localVariable = enterLocal(ctx, name);
                inShadows = true;
            } else if (auto *local = ast::cast_tree<ast::Local>(arg.get())) {
                name = local->localVariable._name;
                localVariable = local->localVariable;
            } else {
                ENFORCE(!inShadows, "shadow argument followed by non-shadow argument!");
                auto pair = arg2Symbol(ctx, arg.get(), i);
                core::SymbolRef sym = pair.first;
                name = pair.second;
                localVariable = enterLocal(ctx, name);
                if (i < ctx.owner.data(ctx).arguments().size()) {
                    ENFORCE(ctx.owner.data(ctx).arguments()[i] == sym);
                } else {
                    ctx.owner.data(ctx).arguments().push_back(sym);
                }
            }

            scopeStack.back().locals[name] = localVariable;
            scopeStack.back().args.emplace_back(localVariable);

            unique_ptr<ast::Expression> localExpr = make_unique<ast::Local>(arg->loc, localVariable);
            arg.swap(localExpr);
        }
    }

    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> original) {
        if (original->args.size() == 1 && ast::isa_tree<ast::ZSuperArgs>(original->args[0].get())) {
            original->args.clear();
            core::SymbolRef method = ctx.owner.data(ctx).enclosingMethod(ctx);
            if (method.data(ctx).isMethod()) {
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
                    mdef->symbol.data(ctx).setPrivate();
                    break;
                case core::Names::protected_()._id:
                    mdef->symbol.data(ctx).setProtected();
                    break;
                case core::Names::public_()._id:
                    mdef->symbol.data(ctx).setPublic();
                    break;
                case core::Names::moduleFunction()._id:
                    aliasModuleFunction(ctx, mdef->symbol);
                    break;
                default:
                    return original;
            }
            return move(original->args[0]);
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
                                            original->fun.toString(ctx));
                            }
                            continue;
                        }
                        core::NameRef name = lit->asSymbol(ctx);

                        core::SymbolRef meth = methodOwner(ctx).data(ctx).findMember(ctx, name);
                        if (!meth.exists()) {
                            if (auto e = ctx.state.beginError(arg->loc, core::errors::Namer::MethodNotFound)) {
                                e.setHeader("`{}`: no such method: `{}`", original->fun.toString(ctx),
                                            name.toString(ctx));
                            }
                            continue;
                        }
                        aliasModuleFunction(ctx, meth);
                    }
                    break;
                }
                case core::Names::aliasMethod()._id: {
                    vector<core::NameRef> args;
                    for (auto &arg : original->args) {
                        auto lit = ast::cast_tree<ast::Literal>(arg.get());
                        if (lit == nullptr || !lit->isSymbol(ctx)) {
                            if (auto e = ctx.state.beginError(arg->loc, core::errors::Namer::DynamicDSLInvocation)) {
                                e.setHeader("Unsupported argument to `{}`: arguments must be symbol literals",
                                            original->fun.toString(ctx));
                            }
                            continue;
                        }
                        core::NameRef name = lit->asSymbol(ctx);

                        args.push_back(name);
                    }
                    if (original->args.size() != 2) {
                        if (auto e = ctx.state.beginError(original->loc, core::errors::Namer::InvalidAlias)) {
                            e.setHeader("Wrong number of arguments to `{}`; Expected: `{}`, got: `{}`",
                                        original->fun.toString(ctx), 2, original->args.size());
                        }
                        break;
                    }
                    if (args.size() != 2) {
                        break;
                    }
                    core::SymbolRef meth = methodOwner(ctx).data(ctx).findMember(ctx, args[1]);
                    if (!meth.exists()) {
                        if (auto e =
                                ctx.state.beginError(original->args[1]->loc, core::errors::Namer::MethodNotFound)) {
                            e.setHeader("`{}`: no such method: `{}`", original->fun.toString(ctx),
                                        args[1].toString(ctx));
                        }
                        break;
                    }
                    aliasMethod(ctx, methodOwner(ctx), args[0], meth);
                    break;
                }
            }
        }

        return original;
    }

    // Allow stub symbols created to hold intrinsics to be filled in
    // with real types from code
    bool redefinitionOk(core::Context ctx, core::SymbolRef sym) {
        auto &data = sym.data(ctx);
        return data.intrinsic != nullptr && data.arguments().empty() && data.resultType == nullptr;
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> method) {
        scopeStack.emplace_back();
        pushEnclosingArgs();
        ++scopeId;
        core::SymbolRef owner = methodOwner(ctx);

        if (method->isSelf()) {
            if (owner.data(ctx).isClass()) {
                owner = owner.data(ctx).singletonClass(ctx);
            }
        }
        ENFORCE(owner.data(ctx).isClass());

        auto sym = owner.data(ctx).findMember(ctx, method->name);
        if (sym.exists()) {
            if (method->declLoc == sym.data(ctx).loc()) {
                // Reparsing the same file
                method->symbol = sym;
                fillInArgs(ctx.withOwner(method->symbol), method->args);
                return method;
            }
            if (redefinitionOk(ctx, sym)) {
                sym.data(ctx).addLoc(ctx, method->declLoc);
            } else {
                if (!method->loc.file().data(ctx).isRBI()) {
                    if (auto e = ctx.state.beginError(method->declLoc, core::errors::Namer::RedefinitionOfMethod)) {
                        e.setHeader("`{}`: Method redefined", method->name.toString(ctx));
                        e.addErrorLine(sym.data(ctx).loc(), "Previous definition");
                    }
                }
                // TODO Check that the previous args match the new ones instead of
                // just moving the original one to the side
                ctx.state.mangleRenameSymbol(sym, method->name, core::UniqueNameKind::Namer);
            }
        }
        method->symbol = ctx.state.enterMethodSymbol(method->declLoc, owner, method->name);
        fillInArgs(ctx.withOwner(method->symbol), method->args);
        method->symbol.data(ctx).addLoc(ctx, method->declLoc);
        if (method->isDSLSynthesized()) {
            method->symbol.data(ctx).setDSLSynthesized();
        }

        return method;
    }

    unique_ptr<ast::MethodDef> postTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> method) {
        if (unique_ptr<ast::Local> discoveredBlockArg = popEnclosingArgs()) {
            method->args.emplace_back(move(discoveredBlockArg));
        }
        ENFORCE(method->args.size() == method->symbol.data(ctx).arguments().size());
        scopeStack.pop_back();
        if (scopeStack.back().moduleFunctionActive) {
            aliasModuleFunction(ctx, method->symbol);
        }
        ENFORCE(method->args.size() == method->symbol.data(ctx).arguments().size(), method->name.toString(ctx), ": ",
                method->args.size(), " != ", method->symbol.data(ctx).arguments().size());
        // Not all information is unfortunately available in the symbol. Original argument names aren't.
        // method->args.clear();
        return method;
    }

    unique_ptr<ast::Block> preTransformBlock(core::MutableContext ctx, unique_ptr<ast::Block> blk) {
        core::SymbolRef owner = ctx.owner;
        if (owner == core::Symbols::noSymbol() || owner == core::Symbols::root()) {
            // Introduce intermediate host for block.
            ENFORCE(blk->loc.exists());
            owner = ctx.state.staticInitForFile(blk->loc.file());
        } else if (owner.data(ctx).isClass()) {
            // If we're at class scope, we're actually in the context of the
            // singleton class.
            owner = owner.data(ctx).singletonClass(ctx);
        }
        blk->symbol =
            ctx.state.enterMethodSymbol(blk->loc, owner,
                                        ctx.state.freshNameUnique(core::UniqueNameKind::Namer, core::Names::blockTemp(),
                                                                  ++(owner.data(ctx).uniqueCounter)));

        auto outerArgs = scopeStack.back().args;
        scopeStack.emplace_back();
        pushEnclosingArgs();
        scopeStack.back().args = move(outerArgs);
        ++scopeId;
        auto &parent = *(scopeStack.end() - 2);
        auto &frame = scopeStack.back();

        // We inherit our parent's locals
        for (auto &binding : parent.locals) {
            frame.locals.insert(binding);
        }

        // If any of our arguments shadow our parent, fillInArgs will overwrite
        // them in `frame.locals`
        fillInArgs(ctx.withOwner(blk->symbol), blk->args);

        return blk;
    }

    unique_ptr<ast::Block> postTransformBlock(core::MutableContext ctx, unique_ptr<ast::Block> blk) {
        if (unique_ptr<ast::Local> discoveredBlockArg = popEnclosingArgs()) {
            blk->args.emplace_back(move(discoveredBlockArg));
        }
        scopeStack.pop_back();
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
                core::Symbol &root = core::Symbols::root().data(ctx);
                core::SymbolRef sym = root.findMember(ctx, nm->name);
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

    unique_ptr<ast::Assign> fillAssign(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        // TODO(nelhage): forbid dynamic constant definition
        auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        ENFORCE(lhs);
        core::SymbolRef scope = squashNames(ctx, ctx.contextClass(), lhs->scope);
        auto sym = scope.data(ctx).findMemberNoDealias(ctx, lhs->cnst);
        if (sym.exists() && !sym.data(ctx).isStaticField()) {
            if (auto e = ctx.state.beginError(asgn->loc, core::errors::Namer::ModuleKindRedefinition)) {
                e.setHeader("Redefining constant `{}`", lhs->cnst.data(ctx).show(ctx));
                e.addErrorLine(sym.data(ctx).loc(), "Previous definition");
            }
            ctx.state.mangleRenameSymbol(sym, sym.data(ctx).name, core::UniqueNameKind::Namer);
        }
        core::SymbolRef cnst = ctx.state.enterStaticFieldSymbol(lhs->loc, scope, lhs->cnst);
        auto loc = lhs->loc;
        unique_ptr<ast::UnresolvedConstantLit> lhsU(lhs);
        asgn->lhs.release();
        asgn->lhs = make_unique<ast::ConstantLit>(loc, cnst, move(lhsU), nullptr);
        return asgn;
    }

    unique_ptr<ast::Expression> handleTypeMemberDefinition(core::MutableContext ctx, bool makeAlias,
                                                           core::SymbolRef onSymbol, ast::Send *send,
                                                           unique_ptr<ast::Assign> asgn,
                                                           ast::UnresolvedConstantLit *typeName) {
        core::Variance variance = core::Variance::Invariant;

        if (!send->args.empty()) {
            if (send->args.size() > 2) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Too many args in type definition");
                }
                return make_unique<ast::EmptyTree>(asgn->loc);
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
                                    ":" + core::Names::covariant().toString(ctx),
                                    ":" + core::Names::contravariant().toString(ctx));
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

        auto members = onSymbol.data(ctx).typeMembers();
        auto it = absl::c_find_if(members, [&](auto mem) { return mem.data(ctx).name == typeName->cnst; });
        if (it != members.end()) {
            if (auto e = ctx.state.beginError(typeName->loc, core::errors::Namer::InvalidTypeDefinition)) {
                e.setHeader("Duplicate type member `{}`", typeName->cnst.data(ctx).show(ctx));
            }
            return make_unique<ast::EmptyTree>(asgn->loc);
        }
        auto sym = ctx.state.enterTypeMember(asgn->loc, onSymbol, typeName->cnst, variance);
        if (makeAlias) {
            auto alias =
                ctx.state.enterStaticFieldSymbol(asgn->loc, ctx.owner.data(ctx).enclosingClass(ctx), typeName->cnst);
            alias.data(ctx).resultType = make_shared<core::AliasType>(sym);
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
                        sym.data(ctx).setFixed();

                        // TODO(nelhage): This creates an order
                        // dependency in the resolver. See RUBYPLAT-520
                        sym.data(ctx).resultType = core::Types::untyped();

                        asgn->lhs = ast::MK::Constant(asgn->lhs->loc, sym);
                        return asgn;
                    }
                }
                if (auto e = ctx.state.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Missing required param :fixed");
                }
            }
        }
        return make_unique<ast::EmptyTree>(asgn->loc);
    }

    unique_ptr<ast::Expression> postTransformAssign(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        if (lhs == nullptr) {
            return asgn;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn->rhs.get());
        if (send == nullptr) {
            return fillAssign(ctx, move(asgn));
        }
        auto *shouldBeSelf = ast::cast_tree<ast::Self>(send->recv.get());
        if (shouldBeSelf == nullptr) {
            auto ret = fillAssign(ctx, move(asgn));
            if (send->fun == core::Names::typeAlias()) {
                core::SymbolRef sym;
                if (auto id = ast::cast_tree<ast::ConstantLit>(ret->lhs.get())) {
                    sym = id->symbol;
                }
                if (sym.exists() && sym.data(ctx).isStaticField()) {
                    sym.data(ctx).setStaticTypeAlias();
                }
            }
            return ret;
        }

        auto *typeName = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        if (typeName == nullptr) {
            return fillAssign(ctx, move(asgn));
        }

        switch (send->fun._id) {
            case core::Names::typeTemplate()._id:
                return handleTypeMemberDefinition(ctx, true,
                                                  ctx.owner.data(ctx).enclosingClass(ctx).data(ctx).singletonClass(ctx),
                                                  send, move(asgn), typeName);
            case core::Names::typeMember()._id:
                return handleTypeMemberDefinition(ctx, false, ctx.owner.data(ctx).enclosingClass(ctx), send, move(asgn),
                                                  typeName);
            default:
                return fillAssign(ctx, move(asgn));
        }
    }

    unique_ptr<ast::Expression> postTransformYield(core::MutableContext ctx, unique_ptr<ast::Yield> yield) {
        auto method = ctx.owner.data(ctx).enclosingMethod(ctx);
        core::SymbolRef blockArg;
        for (auto arg : method.data(ctx).arguments()) {
            if (arg.data(ctx).isBlockArgument()) {
                blockArg = arg;
                break;
            }
        }
        if (!blockArg) {
            // Found yield, and method doesn't have a blk arg, so let's make one.
            auto name = core::Names::blkArg();
            blockArg = ctx.state.enterMethodArgumentSymbol(yield->loc, method, name);
            blockArg.data(ctx).setBlockArgument();
            blockArg.data(ctx).resultType = core::Types::untyped();
            method.data(ctx).arguments().push_back(blockArg);

            // Save it on ourself so we can stick it into the MethodDef's arg list in postTransformMethodDef
            core::LocalVariable local = enterLocal(ctx, name);
            scopeStack.back().locals[name] = local;
            auto &discoveredBlockArg = peekEnclosingArgs();
            ENFORCE(!discoveredBlockArg, "Already created a Local for this block arg!");
            discoveredBlockArg = make_unique<ast::Local>(yield->loc, local);
        }

        auto name = blockArg.data(ctx).name;
        core::LocalVariable local = enterLocal(ctx, name);
        scopeStack.back().locals[name] = local;
        auto recv = make_unique<ast::Local>(yield->loc, local);
        ast::Send::ARGS_store nargs;
        nargs.emplace_back(move(yield->expr));

        return make_unique<ast::Send>(yield->loc, move(recv), core::Names::call(), move(nargs));
    }

private:
    NameInserter() {
        scopeStack.emplace_back();
        scopeId = 0;
    }

    vector<unique_ptr<ast::Local>> enclosingArgsStack;
    void pushEnclosingArgs() {
        // Create a new box to hold any block arg we might discover
        enclosingArgsStack.emplace_back();
    }
    unique_ptr<ast::Local> popEnclosingArgs() {
        ENFORCE(enclosingArgsStack.size() > 0, "Forgot to pushEnclosingArgs(). Have we pushed in every branch?");
        auto ret = move(enclosingArgsStack.back());
        enclosingArgsStack.pop_back();
        return ret;
    }
    unique_ptr<ast::Local> &peekEnclosingArgs() {
        return enclosingArgsStack.back();
    }
}; // namespace namer

unique_ptr<ast::Expression> Namer::run(core::MutableContext ctx, unique_ptr<ast::Expression> tree) {
    NameInserter nameInserter;
    return ast::TreeMap::apply(ctx, nameInserter, move(tree));
}

} // namespace namer
}; // namespace sorbet
