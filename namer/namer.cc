#include "namer/namer.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "ast/ArgParsing.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "class_flatten/class_flatten.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/concurrency/WorkerPool.h"
#include "common/sort/sort.h"
#include "common/timers/Timer.h"
#include "core/Context.h"
#include "core/FileHash.h"
#include "core/FoundDefinitions.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/core.h"
#include "core/errors/namer.h"
#include "core/lsp/TypecheckEpochManager.h"

using namespace std;

namespace sorbet::namer {

namespace {

struct SymbolFinderResult {
    ast::ParsedFile tree;
    unique_ptr<core::FoundDefinitions> names;
};

void swap(SymbolFinderResult &a, SymbolFinderResult &b) {
    a.tree.swap(b.tree);
    a.names.swap(b.names);
}

core::ClassOrModuleRef methodOwner(core::Context ctx, core::SymbolRef owner, bool isSelfMethod) {
    ENFORCE(owner.exists() && owner != core::Symbols::todo());
    auto enclosingClass = owner.enclosingClass(ctx);
    if (enclosingClass == core::Symbols::root()) {
        // Root methods end up going on object
        enclosingClass = core::Symbols::Object();
    }

    if (isSelfMethod) {
        enclosingClass = enclosingClass.data(ctx)->lookupSingletonClass(ctx);
    }
    ENFORCE(enclosingClass.exists());
    return enclosingClass;
}

// Returns the SymbolRef corresponding to the class `self.class`, unless the
// context is a class, in which case return it.
core::ClassOrModuleRef contextClass(const core::GlobalState &gs, core::SymbolRef ofWhat) {
    ENFORCE(ofWhat.exists() && ofWhat != core::Symbols::todo());
    core::SymbolRef owner = ofWhat;
    while (true) {
        ENFORCE(owner.exists(), "non-existing owner in contextClass");
        if (owner.isClassOrModule()) {
            return owner.asClassOrModuleRef();
        }
        if (owner.name(gs) == core::Names::staticInit()) {
            owner = owner.owner(gs).asClassOrModuleRef().data(gs)->attachedClass(gs);
        } else {
            owner = owner.owner(gs);
        }
    }
}

bool isMangleRenameUniqueName(core::GlobalState &gs, core::NameRef name) {
    return name.kind() == core::NameKind::UNIQUE &&
           name.dataUnique(gs)->uniqueNameKind == core::UniqueNameKind::MangleRename;
}

/**
 * Used with TreeWalk to locate all of the class, method, static field, and type member symbols defined in the tree.
 * Does not mutate GlobalState, which allows us to parallelize this process.
 * Does not report any errors, which lets us cache its output.
 * Produces a vector of symbols to insert, and a vector of modifiers to those symbols.
 */
class SymbolFinder {
    unique_ptr<core::FoundDefinitions> foundDefs = make_unique<core::FoundDefinitions>();
    // The tree doesn't have symbols yet, so `ctx.owner`, which is a SymbolRef, is meaningless.
    // Instead, we track the owner manually via FoundDefinitionRefs.
    vector<core::FoundDefinitionRef> ownerStack;
    // `private` with no arguments toggles the visibility of all methods below in the class def.
    // This tracks those as they appear.
    vector<optional<core::FoundModifier>> methodVisiStack = {nullopt};

    void findClassModifiers(core::Context ctx, core::FoundDefinitionRef klass, ast::ExpressionPtr &line) {
        auto *send = ast::cast_tree<ast::Send>(line);
        if (send == nullptr) {
            return;
        }

        switch (send->fun.rawId()) {
            case core::Names::declareFinal().rawId():
            case core::Names::declareSealed().rawId():
            case core::Names::declareInterface().rawId():
            case core::Names::declareAbstract().rawId(): {
                core::FoundModifier mod;
                mod.kind = core::FoundModifier::Kind::Class;
                mod.owner = klass;
                mod.loc = send->loc;
                mod.name = send->fun;
                foundDefs->addModifier(move(mod));
                break;
            }
            default:
                break;
        }
    }

    core::FoundDefinitionRef getOwnerRaw() {
        if (ownerStack.empty()) {
            return core::FoundDefinitionRef::root();
        }
        return ownerStack.back();
    }

    pair<core::FoundDefinitionRef, optional<bool>> getOwnerSkippingMethods() {
        auto owner = getOwnerRaw();
        if (owner.kind() != core::FoundDefinitionRef::Kind::Method) {
            return {owner, nullopt};
        }

        bool isSelfMethod = owner.method(*foundDefs).flags.isSelfMethod;
        ENFORCE(!ownerStack.empty());
        auto it = ownerStack.rbegin() + 1;
        if (it == ownerStack.rend()) {
            return {core::FoundDefinitionRef::root(), isSelfMethod};
        }
        ENFORCE(it->kind() != core::FoundDefinitionRef::Kind::Method);
        return {*it, isSelfMethod};
    }

    core::FoundDefinitionRef getOwner() {
        return getOwnerSkippingMethods().first;
    }

    core::FoundDefinitionRef defineScope(core::FoundDefinitionRef owner, const ast::ExpressionPtr &node) {
        if (auto *id = ast::cast_tree<ast::ConstantLit>(node)) {
            // Already defined. Insert a foundname so we can reference it.
            auto sym = id->symbol;
            ENFORCE(sym.exists());
            return foundDefs->addSymbol(sym.asClassOrModuleRef());
        } else if (auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(node)) {
            core::FoundClass found;
            found.owner = defineScope(owner, constLit->scope);
            found.name = constLit->cnst;
            found.loc = constLit->loc;
            found.declLoc = constLit->loc;
            found.classKind = core::FoundClass::Kind::Unknown;
            return foundDefs->addClass(move(found));
        } else {
            // Either EmptyTree (no more scope) or something is ill-formed (arbitrary expr for const scope?)
            // Fall back to just using `owner`.
            return owner;
        }
    }

public:
    unique_ptr<core::FoundDefinitions> getAndClearFoundDefinitions() {
        ownerStack.clear();
        auto rv = move(foundDefs);
        foundDefs = make_unique<core::FoundDefinitions>();
        return rv;
    }

    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        core::FoundClass found;
        found.classKind = ast::ClassDef::kindToFoundClassKind(klass.kind);
        found.loc = klass.loc;
        found.declLoc = klass.declLoc;

        auto *ident = ast::cast_tree<ast::UnresolvedIdent>(klass.name);
        if ((ident != nullptr) && ident->name == core::Names::singleton()) {
            found.owner = getOwner();
            found.name = ident->name;
        } else {
            if (klass.symbol == core::Symbols::todo()) {
                const auto &constLit = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(klass.name);
                found.owner = defineScope(getOwner(), constLit.scope);
                found.name = constLit.cnst;
            } else {
                // Desugar populates a top-level root() ClassDef.
                // Nothing else should have been typeAlias by now.
                ENFORCE(klass.symbol == core::Symbols::root());
                auto owner = foundDefs->addSymbol(klass.symbol);
                found.owner = owner;
                found.name = klass.symbol.data(ctx)->name;
            }
        }

        ownerStack.emplace_back(foundDefs->addClass(move(found)));
        methodVisiStack.emplace_back(nullopt);
    }

    void postTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        core::FoundDefinitionRef klassName = ownerStack.back();
        ownerStack.pop_back();
        methodVisiStack.pop_back();

        for (auto &exp : klass.rhs) {
            findClassModifiers(ctx, klassName, exp);
            addAncestor(ctx, klass, exp);
        }

        if (!klass.ancestors.empty()) {
            /* Superclass is typeAlias in parent scope, mixins are typeAlias in inner scope */
            for (auto &anc : klass.ancestors) {
                if (!isValidAncestor(anc)) {
                    if (auto e = ctx.beginError(anc.loc(), core::errors::Namer::AncestorNotConstant)) {
                        e.setHeader("Superclasses must only contain constant literals");
                    }
                    anc = ast::MK::EmptyTree();
                }
            }
        }

        auto &foundClass = klassName.klass(*foundDefs);
        if (foundClass.name != core::Names::Constants::Root() && !ctx.file.data(ctx).isRBI() &&
            ast::BehaviorHelpers::checkClassDefinesBehavior(klass)) {
            // TODO(dmitry) This won't find errors in fast-incremental mode.
            foundClass.definesBehavior = true;
        }
    }

    void addAncestor(core::Context ctx, ast::ClassDef &klass, ast::ExpressionPtr &node) {
        auto send = ast::cast_tree<ast::Send>(node);
        if (send == nullptr) {
            ENFORCE(node.get() != nullptr);
            return;
        }

        ast::ClassDef::ANCESTORS_store *dest;
        if (send->fun == core::Names::include()) {
            dest = &klass.ancestors;
        } else if (send->fun == core::Names::extend()) {
            dest = &klass.singletonAncestors;
        } else {
            return;
        }
        if (!send->recv.isSelfReference()) {
            // ignore `something.include`
            return;
        }

        const auto numPosArgs = send->numPosArgs();
        if (numPosArgs == 0) {
            if (auto e = ctx.beginError(send->loc, core::errors::Namer::IncludeMutipleParam)) {
                e.setHeader("`{}` requires at least one argument", send->fun.show(ctx));
            }
            return;
        }

        if (send->hasBlock()) {
            if (auto e = ctx.beginError(send->loc, core::errors::Namer::IncludePassedBlock)) {
                e.setHeader("`{}` can not be passed a block", send->fun.show(ctx));
            }
            return;
        }

        for (auto i = numPosArgs - 1; i >= 0; --i) {
            // Reverse order is intentional: that's how Ruby does it.
            auto &arg = send->getPosArg(i);
            if (ast::isa_tree<ast::EmptyTree>(arg)) {
                continue;
            }
            if (arg.isSelfReference()) {
                dest->emplace_back(arg.deepCopy());
                continue;
            }
            if (isValidAncestor(arg)) {
                dest->emplace_back(arg.deepCopy());
            } else {
                if (auto e = ctx.beginError(arg.loc(), core::errors::Namer::AncestorNotConstant)) {
                    e.setHeader("`{}` must only contain constant literals", send->fun.show(ctx));
                }
            }
        }
    }

    bool isValidAncestor(ast::ExpressionPtr &exp) {
        if (ast::isa_tree<ast::EmptyTree>(exp) || exp.isSelfReference() || ast::isa_tree<ast::ConstantLit>(exp)) {
            return true;
        }
        if (auto lit = ast::cast_tree<ast::UnresolvedConstantLit>(exp)) {
            return isValidAncestor(lit->scope);
        }
        return false;
    }

    void preTransformBlock(core::Context ctx, ast::ExpressionPtr &block) {
        methodVisiStack.emplace_back(nullopt);
    }

    void postTransformBlock(core::Context ctx, ast::ExpressionPtr &block) {
        methodVisiStack.pop_back();
    }

    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &method = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        core::FoundMethod foundMethod;
        foundMethod.owner = getOwner();
        foundMethod.name = method.name;
        foundMethod.loc = method.loc;
        foundMethod.declLoc = method.declLoc;
        foundMethod.flags = method.flags;
        foundMethod.parsedArgs = ast::ArgParsing::parseArgs(method.args);
        foundMethod.arityHash = ast::ArgParsing::hashArgs(ctx, foundMethod.parsedArgs);
        auto def = foundDefs->addMethod(move(foundMethod));

        // After flatten, method defs have been hoisted and reordered, so instead we look for the
        // keep_def / keep_self_def calls, which will still be ordered correctly relative to
        // visibility modifiers.
        ownerStack.emplace_back(def);
        methodVisiStack.emplace_back(nullopt);
    }

    void postTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        methodVisiStack.pop_back();
        ownerStack.pop_back();
    }

    void addMethodModifiers(core::Context ctx, core::NameRef modifierName,
                            absl::Span<const ast::ExpressionPtr> sendArgs) {
        if (sendArgs.empty()) {
            return;
        }

        if (sendArgs.size() == 1) {
            if (auto *array = ast::cast_tree<ast::Array>(sendArgs[0])) {
                for (auto &e : array->elems) {
                    addMethodModifier(ctx, modifierName, e);
                }
                return;
            }
        }

        for (auto &arg : sendArgs) {
            addMethodModifier(ctx, modifierName, arg);
        }
    }

    void postTransformSend(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &original = ast::cast_tree_nonnull<ast::Send>(tree);
        auto ownerIsMethod = getOwnerRaw().kind() == core::FoundDefinitionRef::Kind::Method;

        switch (original.fun.rawId()) {
            case core::Names::privateClassMethod().rawId():
            case core::Names::publicClassMethod().rawId(): {
                if (ownerIsMethod) {
                    break;
                }
                addMethodModifiers(ctx, original.fun, original.posArgs());
                break;
            }
            case core::Names::packagePrivate().rawId():
            case core::Names::packagePrivateClassMethod().rawId():
            case core::Names::private_().rawId():
            case core::Names::protected_().rawId():
            case core::Names::public_().rawId():
                if (ownerIsMethod) {
                    break;
                }
                if (!original.hasPosArgs()) {
                    ENFORCE(!methodVisiStack.empty());
                    methodVisiStack.back() = optional<core::FoundModifier>{core::FoundModifier{
                        core::FoundModifier::Kind::Method,
                        getOwner(),
                        original.loc,
                        original.fun,
                        core::NameRef::noName(),
                    }};
                } else {
                    addMethodModifiers(ctx, original.fun, original.posArgs());
                }
                break;
            case core::Names::privateConstant().rawId(): {
                if (ownerIsMethod) {
                    break;
                }
                for (auto &arg : original.posArgs()) {
                    addConstantModifier(ctx, original.fun, arg);
                }
                break;
            }
            case core::Names::keepDef().rawId(): {
                // ^ visibility toggle doesn't look at `self.*` methods, only instance methods
                // (need to use `class << self` to use nullary private with singleton class methods)

                if (original.numPosArgs() != 3) {
                    break;
                }

                ENFORCE(!methodVisiStack.empty());
                if (!methodVisiStack.back().has_value()) {
                    break;
                }

                auto recv = ast::cast_tree<ast::ConstantLit>(original.recv);
                if (recv == nullptr || recv->symbol != core::Symbols::Sorbet_Private_Static()) {
                    break;
                }

                auto methodName = unwrapLiteralToMethodName(ctx, original.getPosArg(1));
                foundDefs->addModifier(methodVisiStack.back()->withTarget(methodName));

                break;
            }
            case core::Names::aliasMethod().rawId(): {
                if (ownerIsMethod) {
                    break;
                }
                addMethodAlias(ctx, original);
                break;
            }
        }
    }

    void postTransformRuntimeMethodDefinition(core::Context, ast::ExpressionPtr &tree) {
        auto &original = ast::cast_tree_nonnull<ast::RuntimeMethodDefinition>(tree);

        // visibility toggle doesn't look at `self.*` methods, only instance methods
        // (need to use `class << self` to use nullary private with singleton class methods)
        if (original.isSelfMethod) {
            return;
        }

        ENFORCE(!methodVisiStack.empty());
        if (!methodVisiStack.back().has_value()) {
            return;
        }

        foundDefs->addModifier(methodVisiStack.back()->withTarget(original.name));
    }

    void addMethodModifier(core::Context ctx, core::NameRef modifierName, const ast::ExpressionPtr &arg) {
        auto target = unwrapLiteralToMethodName(ctx, arg);
        if (target.exists()) {
            foundDefs->addModifier(core::FoundModifier{
                core::FoundModifier::Kind::Method,
                getOwner(),
                arg.loc(),
                /*name*/ modifierName,
                target,
            });
        }
    }

    void addMethodAlias(core::Context ctx, const ast::Send &send) {
        if (!send.recv.isSelfReference()) {
            return;
        }

        if (send.numPosArgs() != 2) {
            return;
        }

        auto parsedArgs = InlinedVector<core::NameRef, 2>{};

        for (const auto &arg : send.posArgs()) {
            auto lit = ast::cast_tree<ast::Literal>(arg);
            if (lit == nullptr || !lit->isSymbol()) {
                continue;
            }
            core::NameRef name = lit->asSymbol();

            parsedArgs.emplace_back(name);
        }

        if (parsedArgs.size() != 2) {
            return;
        }

        auto fromName = parsedArgs[0];

        core::FoundMethod foundMethod;
        foundMethod.owner = getOwner();
        foundMethod.name = fromName;
        foundMethod.loc = send.loc;
        foundMethod.declLoc = send.loc;
        foundMethod.arityHash = core::ArityHash::aliasMethodHash();
        foundDefs->addMethod(move(foundMethod));
    }

    void addConstantModifier(core::Context ctx, core::NameRef modifierName, const ast::ExpressionPtr &arg) {
        auto target = core::NameRef::noName();
        if (auto sym = ast::cast_tree<ast::Literal>(arg)) {
            if (sym->isSymbol()) {
                target = sym->asSymbol();
            } else if (sym->isString()) {
                target = sym->asString();
            }
        }

        if (target.exists()) {
            foundDefs->addModifier(core::FoundModifier{
                core::FoundModifier::Kind::ClassOrStaticField,
                getOwner(),
                arg.loc(),
                /*name*/ modifierName,
                target,
            });
        }
    }

    core::NameRef unwrapLiteralToMethodName(core::Context ctx, const ast::ExpressionPtr &expr) {
        if (auto sym = ast::cast_tree<ast::Literal>(expr)) {
            // this handles the `private :foo` case
            if (!sym->isSymbol()) {
                return core::NameRef::noName();
            }
            return sym->asSymbol();
        } else if (auto *def = ast::cast_tree<ast::RuntimeMethodDefinition>(expr)) {
            return def->name;
        } else if (auto send = ast::cast_tree<ast::Send>(expr)) {
            if (send->fun != core::Names::keepDef() && send->fun != core::Names::keepSelfDef()) {
                return core::NameRef::noName();
            }

            auto recv = ast::cast_tree<ast::ConstantLit>(send->recv);
            if (recv == nullptr) {
                return core::NameRef::noName();
            }

            if (recv->symbol != core::Symbols::Sorbet_Private_Static()) {
                return core::NameRef::noName();
            }

            if (send->numPosArgs() != 3) {
                return core::NameRef::noName();
            }

            return unwrapLiteralToMethodName(ctx, send->getPosArg(1));
        } else {
            ENFORCE(!ast::isa_tree<ast::MethodDef>(expr), "methods inside sends should be gone");
            return core::NameRef::noName();
        }
    }

    core::FoundDefinitionRef fillAssign(core::Context ctx, const ast::Assign &asgn) {
        auto &lhs = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(asgn.lhs);

        core::FoundStaticField found;
        found.owner = defineScope(getOwner(), lhs.scope);
        found.name = lhs.cnst;
        found.asgnLoc = asgn.loc;
        found.lhsLoc = lhs.loc;
        return foundDefs->addStaticField(move(found));
    }

    core::FoundDefinitionRef handleTypeMemberDefinition(core::Context ctx, const ast::Send *send,
                                                        const ast::Assign &asgn,
                                                        const ast::UnresolvedConstantLit *typeName) {
        ENFORCE(ast::cast_tree<ast::UnresolvedConstantLit>(asgn.lhs) == typeName &&
                ast::cast_tree<ast::Send>(asgn.rhs) ==
                    send); // this method assumes that `asgn` owns `send` and `typeName`

        core::FoundTypeMember found;
        found.owner = getOwner();
        found.asgnLoc = asgn.loc;
        found.nameLoc = typeName->loc;
        found.name = typeName->cnst;
        // Store name rather than core::Variance type so that we can defer reporting an error until later.
        found.varianceName = core::NameRef();
        found.isTypeTemplate = send->fun == core::Names::typeTemplate();

        if (send->numPosArgs() > 1) {
            // Too many arguments. Define a static field that we'll use for this type member later.
            core::FoundStaticField staticField;
            staticField.owner = found.owner;
            staticField.name = found.name;
            staticField.asgnLoc = found.asgnLoc;
            staticField.lhsLoc = asgn.lhs.loc();
            staticField.isTypeAlias = true;
            return foundDefs->addStaticField(move(staticField));
        }

        if (send->hasPosArgs() || send->block() != nullptr) {
            // If there are positional arguments, there might be a variance annotation
            if (send->numPosArgs() > 0) {
                auto *lit = ast::cast_tree<ast::Literal>(send->getPosArg(0));
                if (lit != nullptr && lit->isSymbol()) {
                    found.varianceName = lit->asSymbol();
                    found.litLoc = lit->loc;
                }
            }

            if (send->block() != nullptr) {
                if (const auto *hash = ast::cast_tree<ast::Hash>(send->block()->body)) {
                    for (const auto &keyExpr : hash->keys) {
                        const auto *key = ast::cast_tree<ast::Literal>(keyExpr);
                        if (key != nullptr && key->isSymbol()) {
                            switch (key->asSymbol().rawId()) {
                                case core::Names::fixed().rawId():
                                    found.isFixed = true;
                                    break;
                            }
                        }
                    }
                }
            }
        }
        return foundDefs->addTypeMember(move(found));
    }

    core::FoundDefinitionRef handleAssignment(core::Context ctx, const ast::Assign &asgn) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(asgn.rhs);
        auto foundRef = fillAssign(ctx, asgn);
        ENFORCE(foundRef.kind() == core::FoundDefinitionRef::Kind::StaticField);
        auto &staticField = foundRef.staticField(*foundDefs);
        staticField.isTypeAlias = send.fun == core::Names::typeAlias();
        return foundRef;
    }

    // Returns `true` if `asgn` is a field declaration.
    bool handleFieldDeclaration(core::Context ctx, ast::Assign &asgn) {
        auto *uid = ast::cast_tree<ast::UnresolvedIdent>(asgn.lhs);
        if (uid == nullptr) {
            return false;
        }

        if (uid->kind != ast::UnresolvedIdent::Kind::Instance && uid->kind != ast::UnresolvedIdent::Kind::Class) {
            return false;
        }

        auto *recur = &asgn.rhs;
        while (auto *outer = ast::cast_tree<ast::InsSeq>(*recur)) {
            recur = &outer->expr;
        }

        auto *cast = ast::cast_tree<ast::Cast>(*recur);
        if (cast == nullptr) {
            return false;
        }

        // Resolver will issues errors about non-let declarations of variables, but
        // will still associate types with the variables; we need to ensure that
        // there are entries in the symbol table for those variables.

        core::FoundField found;
        found.kind = uid->kind == ast::UnresolvedIdent::Kind::Instance ? core::FoundField::Kind::InstanceVariable
                                                                       : core::FoundField::Kind::ClassVariable;
        auto [owner, isSelfMethod] = getOwnerSkippingMethods();
        auto onSingletonClass = isSelfMethod.value_or(found.kind == core::FoundField::Kind::InstanceVariable);
        found.onSingletonClass = onSingletonClass;
        found.fromWithinMethod = isSelfMethod.has_value();
        found.owner = owner;
        found.loc = uid->loc;
        found.name = uid->name;
        foundDefs->addField(move(found));
        return true;
    }

    void postTransformAssign(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);

        if (handleFieldDeclaration(ctx, asgn)) {
            return;
        }

        auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn.lhs);
        if (lhs == nullptr) {
            return;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn.rhs);
        if (send == nullptr) {
            fillAssign(ctx, asgn);
        } else if (!send->recv.isSelfReference()) {
            handleAssignment(ctx, asgn);
        } else if (!ast::isa_tree<ast::EmptyTree>(lhs->scope)) {
            handleAssignment(ctx, asgn);
        } else {
            switch (send->fun.rawId()) {
                case core::Names::typeTemplate().rawId():
                case core::Names::typeMember().rawId():
                    handleTypeMemberDefinition(ctx, send, asgn, lhs);
                    break;
                default:
                    fillAssign(ctx, asgn);
                    break;
            }
        }
    }
};

using BehaviorLocs = InlinedVector<core::Loc, 1>;
using ClassBehaviorLocsMap = UnorderedMap<core::ClassOrModuleRef, BehaviorLocs>;

bool isBadHasAttachedClass(core::Context ctx, core::NameRef name) {
    auto owner = ctx.owner.asClassOrModuleRef();
    return name == core::Names::Constants::AttachedClass() && owner != core::Symbols::Class() &&
           owner.data(ctx)->isClass() && !owner.data(ctx)->isSingletonClass(ctx);
}

/**
 * Defines symbols for all of the definitions found via SymbolFinder. Single threaded.
 */
class SymbolDefiner {
public:
    struct State {
        // See getOwnerSymbol for how both of these work.
        vector<core::ClassOrModuleRef> definedClasses;

        State() = default;
        State(const State &) = delete;
        State &operator=(const State &) = delete;
        State(State &&) = default;
        State &operator=(State &&) = default;
    };

private:
    const core::FoundDefinitions &foundDefs;
    const optional<core::FoundDefHashes> oldFoundHashes;

    // Get the symbol for an already-defined owner. Limited to refs that can own things (classes and methods).
    core::ClassOrModuleRef getOwnerSymbol(const SymbolDefiner::State &state, core::FoundDefinitionRef ref) {
        switch (ref.kind()) {
            case core::FoundDefinitionRef::Kind::Symbol:
                return ref.symbol();
            case core::FoundDefinitionRef::Kind::Class:
                ENFORCE(ref.idx() < state.definedClasses.size());
                return state.definedClasses[ref.idx()];
            case core::FoundDefinitionRef::Kind::Method:
            case core::FoundDefinitionRef::Kind::Empty:
            case core::FoundDefinitionRef::Kind::StaticField:
            case core::FoundDefinitionRef::Kind::TypeMember:
            case core::FoundDefinitionRef::Kind::Field:
                Exception::raise("Invalid owner reference {}", core::FoundDefinitionRef::kindToString(ref.kind()));
        }
    }

    // Allow stub symbols created to hold intrinsics to be filled in
    // with real types from code
    bool isIntrinsic(const core::MethodData &data) {
        return data->hasIntrinsic() && !data->hasSig();
    }

    string_view prettySymbolKind(core::MutableContext ctx, core::SymbolRef::Kind kind) {
        switch (kind) {
            case core::SymbolRef::Kind::ClassOrModule:
                return "class or module";
            case core::SymbolRef::Kind::FieldOrStaticField: {
                // TODO(jez) Split core::Field into two
                // Currently we lie and always say static field here
                return "static field";
            }
            case core::SymbolRef::Kind::TypeMember:
                return "type member or type template";
            case core::SymbolRef::Kind::Method:
                // Consider adding a test and removing this assertion if it fires
                ENFORCE(false, "Should not call prettySymbolKind on method in namer")
                return "method";
            case core::SymbolRef::Kind::TypeArgument:
                // Consider adding a test and removing this assertion if it fires
                ENFORCE(false, "Should not call prettySymbolKind on type arguument in namer");
                return "type argument";
        }
    }

    void emitRedefinedConstantError(core::MutableContext ctx, core::LocOffsets errorLoc, core::NameRef name,
                                    core::SymbolRef::Kind kind, core::SymbolRef prevSymbol) {
        using Kind = core::SymbolRef::Kind;
        ENFORCE(
            kind != Kind::ClassOrModule,
            "ClassOrModule symbols should always be entered first, so they should never need to mangle something else");
        if (auto e = ctx.beginError(errorLoc, core::errors::Namer::ConstantKindRedefinition)) {
            auto prevSymbolKind = prettySymbolKind(ctx, prevSymbol.kind());
            if (prevSymbol.kind() == Kind::ClassOrModule && prevSymbol.asClassOrModuleRef().data(ctx)->isDeclared()) {
                prevSymbolKind = prevSymbol.asClassOrModuleRef().showKind(ctx);
            }

            if (prevSymbol.kind() == Kind::ClassOrModule && kind == Kind::FieldOrStaticField) {
                e.setHeader("Cannot initialize the {} `{}` by constant assignment", prevSymbolKind, name.show(ctx));
            } else {
                e.setHeader("Redefining constant `{}` as a {}", name.show(ctx), prettySymbolKind(ctx, kind));
            }
            e.addErrorLine(prevSymbol.loc(ctx), "Previously defined as a {} here", prevSymbolKind);

            if (kind == Kind::FieldOrStaticField && prevSymbol.kind() == Kind::ClassOrModule) {
                e.addErrorNote("Sorbet does not allow treating constant assignments as class or module definitions,\n"
                               "    even if the initializer computes a `{}` object at runtime. See the docs for more.",
                               "Module");
            }
        }
    }

    void emitRedefinedConstantError(core::MutableContext ctx, core::LocOffsets errorLoc, core::SymbolRef symbol,
                                    core::SymbolRef prevSymbol) {
        emitRedefinedConstantError(ctx, errorLoc, symbol.name(ctx), symbol.kind(), prevSymbol);
    }

    void defineArg(core::MutableContext ctx, core::MethodData &methodData, int pos, const core::ParsedArg &parsedArg) {
        if (pos < methodData->arguments.size()) {
            // TODO: check that flags match;
            if (parsedArg.loc.exists()) {
                methodData->arguments[pos].loc = ctx.locAt(parsedArg.loc);
            }
            return;
        }

        core::NameRef name;
        if (parsedArg.flags.isKeyword) {
            if (parsedArg.flags.isRepeated) {
                if (parsedArg.local._name == core::Names::fwdKwargs()) {
                    name = core::Names::fwdKwargs();
                } else {
                    name = core::Names::kwargs();
                }
            } else {
                name = parsedArg.local._name;
            }
        } else if (parsedArg.flags.isBlock) {
            name = core::Names::blkArg();
        } else {
            name = ctx.state.freshNameUnique(core::UniqueNameKind::PositionalArg, core::Names::arg(), pos + 1);
        }
        // we know right now that pos >= arguments.size() because otherwise we would have hit the early return at the
        // beginning of this method
        auto &argInfo = ctx.state.enterMethodArgumentSymbol(ctx.locAt(parsedArg.loc), ctx.owner.asMethodRef(), name);
        // if enterMethodArgumentSymbol did not emplace a new argument into the list, then it means it's reusing an
        // existing one, which means we've seen a repeated kwarg (as it treats identically named kwargs as
        // identical). We know that we need to match the arity of the function as written, so if we don't have as many
        // arguments as we expect, clone the one we got back from enterMethodArgumentSymbol in the position we expect
        if (methodData->arguments.size() == pos) {
            auto argCopy = argInfo.deepCopy();
            argCopy.name = ctx.state.freshNameUnique(core::UniqueNameKind::MangledKeywordArg, argInfo.name, pos + 1);
            methodData->arguments.emplace_back(move(argCopy));
            return;
        }
        // at this point, we should have at least pos + 1 arguments, and arguments[pos] should be the thing we got back
        // from enterMethodArgumentSymbol
        ENFORCE(methodData->arguments.size() >= pos + 1);

        argInfo.flags = parsedArg.flags;
    }

    void defineArgs(core::MutableContext ctx, const vector<core::ParsedArg> &parsedArgs) {
        auto methodData = ctx.owner.asMethodRef().data(ctx);
        bool inShadows = false;
        bool intrinsic = isIntrinsic(methodData);
        bool swapArgs = intrinsic && (methodData->arguments.size() == 1);
        core::ArgInfo swappedArg;
        if (swapArgs) {
            // When we're filling in an intrinsic method, we want to overwrite the block arg that used
            // to exist with the block arg that we got from desugaring the method def in the RBI files.
            ENFORCE(methodData->arguments[0].flags.isBlock);
            swappedArg = move(methodData->arguments[0]);
            methodData->arguments.clear();
        }

        int i = -1;
        for (auto &arg : parsedArgs) {
            i++;
            if (arg.flags.isShadow) {
                inShadows = true;
            } else {
                ENFORCE(!inShadows, "shadow argument followed by non-shadow argument!");

                if (swapArgs && arg.flags.isBlock) {
                    // see commnent on if (swapArgs) above
                    methodData->arguments.emplace_back(move(swappedArg));
                }

                defineArg(ctx, methodData, i, arg);
                ENFORCE(i < methodData->arguments.size());
            }
        }
    }

    bool paramsMatch(core::MutableContext ctx, core::MethodRef method, const vector<core::ParsedArg> &parsedArgs) {
        auto sym = method.data(ctx)->dealiasMethod(ctx);
        if (sym.data(ctx)->arguments.size() != parsedArgs.size()) {
            return false;
        }
        for (int i = 0; i < parsedArgs.size(); i++) {
            auto &methodArg = parsedArgs[i];
            auto &symArg = sym.data(ctx)->arguments[i];

            if (symArg.flags.isKeyword != methodArg.flags.isKeyword ||
                symArg.flags.isBlock != methodArg.flags.isBlock ||
                symArg.flags.isRepeated != methodArg.flags.isRepeated ||
                (symArg.flags.isKeyword && symArg.name != methodArg.local._name)) {
                return false;
            }
        }

        return true;
    }

    void paramMismatchErrors(core::MutableContext ctx, core::Loc loc, const vector<core::ParsedArg> &parsedArgs) {
        auto sym = ctx.owner.dealias(ctx);
        if (!sym.isMethod()) {
            return;
        }
        auto symMethod = sym.asMethodRef();
        if (symMethod.data(ctx)->arguments.size() != parsedArgs.size()) {
            if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                if (sym != ctx.owner) {
                    // Subtracting 1 because of the block arg we added everywhere.
                    // Eventually we should be more principled about how we report this.
                    e.setHeader(
                        "Method alias `{}` redefined without matching argument count. Expected: `{}`, got: `{}`",
                        ctx.owner.show(ctx), symMethod.data(ctx)->arguments.size() - 1, parsedArgs.size() - 1);
                    e.addErrorLine(ctx.owner.loc(ctx), "Previous alias definition");
                    e.addErrorLine(symMethod.data(ctx)->loc(), "Dealiased definition");
                } else {
                    // Subtracting 1 because of the block arg we added everywhere.
                    // Eventually we should be more principled about how we report this.
                    e.setHeader("Method `{}` redefined without matching argument count. Expected: `{}`, got: `{}`",
                                symMethod.show(ctx), symMethod.data(ctx)->arguments.size() - 1, parsedArgs.size() - 1);
                    e.addErrorLine(symMethod.data(ctx)->loc(), "Previous definition");
                }
            }
            return;
        }
        for (int i = 0; i < parsedArgs.size(); i++) {
            auto &methodArg = parsedArgs[i];
            auto &symArg = symMethod.data(ctx)->arguments[i];

            if (symArg.flags.isKeyword != methodArg.flags.isKeyword) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader("Method `{}` redefined with argument `{}` as a {} argument", sym.show(ctx),
                                methodArg.local.toString(ctx), methodArg.flags.isKeyword ? "keyword" : "non-keyword");
                    e.addErrorLine(
                        sym.loc(ctx),
                        "The corresponding argument `{}` in the previous definition was {}a keyword argument",
                        symArg.show(ctx), symArg.flags.isKeyword ? "" : "not ");
                }
                return;
            }
            // because of how we synthesize block args, this condition should always be true. In particular: the
            // last thing in our list of arguments will always be a block arg, either an explicit one or an implicit
            // one, and the only situation in which a block arg will ever be seen is as the last argument in the
            // list. Consequently, the only situation in which a block arg will be matched up with a non-block arg
            // is when the lists are different lengths: but in that case, we'll have bailed out of this function
            // already with the "without matching argument count" error above. So, as long as we have maintained the
            // intended invariants around methods and arguments, we do not need to ever issue an error about
            // non-matching isBlock-ness.
            ENFORCE(symArg.flags.isBlock == methodArg.flags.isBlock);
            if (symArg.flags.isRepeated != methodArg.flags.isRepeated) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader("Method `{}` redefined with argument `{}` as a {} argument", sym.show(ctx),
                                methodArg.local.toString(ctx), methodArg.flags.isRepeated ? "splat" : "non-splat");
                    e.addErrorLine(sym.loc(ctx),
                                   "The corresponding argument `{}` in the previous definition was {}a splat argument",
                                   symArg.show(ctx), symArg.flags.isRepeated ? "" : "not ");
                }
                return;
            }
            if (symArg.flags.isKeyword && symArg.name != methodArg.local._name) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader(
                        "Method `{}` redefined with mismatched keyword argument name. Expected: `{}`, got: `{}`",
                        sym.show(ctx), symArg.name.show(ctx), methodArg.local._name.show(ctx));
                    e.addErrorLine(sym.loc(ctx), "Previous definition");
                }
                return;
            }
        }
    }

    core::MethodRef defineMethod(core::MutableContext ctx, const core::FoundMethod &method) {
        auto owner = methodOwner(ctx, ctx.owner, method.flags.isSelfMethod);

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
        // when the namer is in incremental mode and looking at `def f(x)`, then `sym` refers to `f(x)`,
        // `currentSym` refers to `def f(x, y)` (which is the symbol that is "currently" non-mangled and available
        // under the name `f`, because the namer has already run in this context) and `replacedSym` refers to `def
        // f()`, because that's the symbol that `f` referred to immediately before `def f(x)` became the
        // then-current symbol. If we weren't running in incremental mode, then `sym` wouldn't exist yet (we would
        // be about to create it!) and `currentSym` would be `def f()`, because we have not yet progressed far
        // enough in the file to see any other definition of `f`.
        auto &parsedArgs = method.parsedArgs;
        auto symTableSize = ctx.state.methodsUsed();
        auto declLoc = ctx.locAt(method.declLoc);
        auto sym = ctx.state.enterMethodSymbol(declLoc, owner, method.name);
        const bool isNewSymbol = symTableSize != ctx.state.methodsUsed();
        if (!isNewSymbol) {
            // See if this is == to the method we're defining now, or if we have a redefinition error.
            auto matchingSym = ctx.state.lookupMethodSymbolWithHash(owner, method.name, method.arityHash);
            if (!matchingSym.exists()) {
                // we don't have a method definition with the right argument structure, so we need to mangle the
                // existing one and create a new one
                if (!isIntrinsic(sym.data(ctx))) {
                    paramMismatchErrors(ctx.withOwner(sym), declLoc, parsedArgs);
                    ctx.state.mangleRenameMethod(sym, method.name);
                    // Re-enter a new symbol.
                    sym = ctx.state.enterMethodSymbol(declLoc, owner, method.name);
                } else {
                    // ...unless it's an intrinsic, because we allow multiple incompatible definitions of those in code
                    // TODO(jvilk): Wouldn't this always fail since `!sym.exists()`?
                    matchingSym.data(ctx)->addLoc(ctx, declLoc);
                }
            } else {
                // if the symbol does exist, then we're running in incremental mode, and we need to compare it to
                // the previously defined equivalent to re-report any errors
                auto replacedSym = ctx.state.findRenamedSymbol(owner, matchingSym); // OK, this is a method
                if (replacedSym.exists() && !paramsMatch(ctx, replacedSym.asMethodRef(), parsedArgs) &&
                    !isIntrinsic(replacedSym.asMethodRef().data(ctx))) {
                    paramMismatchErrors(ctx.withOwner(replacedSym), declLoc, parsedArgs);
                }
                sym = matchingSym;
            }
        }

        defineArgs(ctx.withOwner(sym), parsedArgs);
        sym.data(ctx)->addLoc(ctx, declLoc);
        if (method.flags.isRewriterSynthesized) {
            sym.data(ctx)->flags.isRewriterSynthesized = true;
        }
        ENFORCE(ctx.state.lookupMethodSymbolWithHash(owner, method.name, method.arityHash).exists());
        return sym;
    }

    core::MethodRef insertMethod(core::MutableContext ctx, const core::FoundMethod &method) {
        auto symbol = defineMethod(ctx, method);
        auto name = symbol.data(ctx)->name;
        if (name.kind() == core::NameKind::UNIQUE &&
            name.dataUnique(ctx)->uniqueNameKind == core::UniqueNameKind::MangleRenameOverload) {
            // These name kinds are only created in resolver, which means that we must be running on
            // the fast path but not with the incremental namer.
            // If we let the rest of insertMethod run, it will mark this method as public even if it was
            // already private. Then modifyMethod will attempt to mark a method private by name
            // (instead of by name and arity hash), which will have the effect of NOT re-marking the
            // MangleRenameOverload method as private (it will remain public). Then later in
            // resolver, the visibility of the MangleRenameOverload is propagated to all overloads,
            // which will make them all public.
            //
            // Any case where the visibility would have actually changed would have come via an
            // edit, which would then trigger an incremental namer run. So we know that this
            // typecheck is only for the purpose of e.g. answering a hover, and we don't actually
            // need to mutate GlobalState for any changes. So we can just short circuit.
            //
            // One way to change this in the future might be to make FoundModifier attempt to record
            // something about the method arity in addition to just the method's name. But for the
            // time being this hack suffices. (See the commit where this comment was added a test)
            return symbol;
        }

        // Methods defined at the top level default to private (on Object)
        // Also, the `initialize` method defaults to private
        auto implicitlyPrivate =
            (ctx.owner.enclosingClass(ctx) == core::Symbols::root()) ||
            (!symbol.data(ctx)->owner.data(ctx)->attachedClass(ctx).exists() && name == core::Names::initialize());
        if (implicitlyPrivate) {
            symbol.data(ctx)->flags.isPrivate = true;
        } else {
            // All other methods default to public (their visibility might be changed later)
            symbol.data(ctx)->setMethodPublic();
        }
        return symbol;
    }

    void modifyMethod(core::MutableContext ctx, const core::FoundModifier &mod) {
        ENFORCE(mod.kind == core::FoundModifier::Kind::Method);

        core::ClassOrModuleRef owner;
        if (mod.name == core::Names::privateClassMethod() || mod.name == core::Names::publicClassMethod() ||
            mod.name == core::Names::packagePrivateClassMethod()) {
            owner = ctx.selfClass();
        } else {
            owner = ctx.owner.enclosingClass(ctx);
        }

        auto method = ctx.state.lookupMethodSymbol(owner, mod.target);
        if (method.exists()) {
            switch (mod.name.rawId()) {
                case core::Names::private_().rawId():
                case core::Names::privateClassMethod().rawId():
                    method.data(ctx)->flags.isPrivate = true;
                    break;
                case core::Names::packagePrivate().rawId():
                case core::Names::packagePrivateClassMethod().rawId():
                    method.data(ctx)->flags.isPackagePrivate = true;
                    break;
                case core::Names::protected_().rawId():
                    method.data(ctx)->flags.isProtected = true;
                    break;
                case core::Names::public_().rawId():
                case core::Names::publicClassMethod().rawId():
                    method.data(ctx)->setMethodPublic();
                    break;
                default:
                    break;
            }
        }
    }

    void modifyConstant(core::MutableContext ctx, const core::FoundModifier &mod) {
        ENFORCE(mod.kind == core::FoundModifier::Kind::ClassOrStaticField);

        auto owner = ctx.owner.enclosingClass(ctx);
        auto constantNameRef = ctx.state.lookupNameConstant(mod.target);
        auto constant = ctx.state.lookupSymbol(owner, constantNameRef);
        if (constant.exists() && mod.name == core::Names::privateConstant()) {
            if (constant.isClassOrModule()) {
                constant.asClassOrModuleRef().data(ctx)->flags.isPrivate = true;
            } else if (constant.isStaticField(ctx)) {
                constant.asFieldRef().data(ctx)->flags.isStaticFieldPrivate = true;
            } else if (constant.isTypeMember()) {
                // Visibility on type members is special (even more restrictive than private),
                // so we ignore requests to mark type members private.
            }
        }
    }

    core::ClassOrModuleRef getClassSymbol(core::MutableContext ctx, const State &state, const core::FoundClass &klass) {
        core::ClassOrModuleRef symbol;
        if (klass.name == core::Names::Constants::Root()) {
            symbol = core::Symbols::root();
        } else if (klass.name == core::Names::singleton()) {
            symbol = ctx.owner.asClassOrModuleRef().data(ctx)->singletonClass(ctx);
        } else {
            auto owner = getOwnerSymbol(state, klass.owner);
            auto member = owner.data(ctx)->findMember(ctx, klass.name);
            if (member.exists()) {
                // If member exists with this name, it must be a class or module, because we never mangle-rename them.
                symbol = member.asClassOrModuleRef();
            } else {
                auto newClass = ctx.state.enterClassSymbol(ctx.locAt(klass.declLoc), owner, klass.name);
                symbol = newClass;
            }
        }
        ENFORCE(symbol.exists());

        const bool isUnknown = klass.classKind == core::FoundClass::Kind::Unknown;
        const bool isModule = klass.classKind == core::FoundClass::Kind::Module;
        auto declLoc = ctx.locAt(klass.declLoc);

        if (symbol.data(ctx)->isClassModuleSet() && !isUnknown && isModule != symbol.data(ctx)->isModule()) {
            if (auto e = ctx.state.beginError(declLoc, core::errors::Namer::ModuleKindRedefinition)) {
                e.setHeader("`{}` was previously defined as a `{}`", symbol.show(ctx),
                            symbol.data(ctx)->isModule() ? "module" : "class");

                for (auto loc : symbol.data(ctx)->locs()) {
                    if (loc != declLoc) {
                        e.addErrorLine(loc, "Previous definition");
                    }
                }
            }

            // Even though the declaration is erroneous, the class/module is technically "declared".
            symbol.data(ctx)->setDeclared();
        } else if (!isUnknown) {
            symbol.data(ctx)->setIsModule(isModule);
            symbol.data(ctx)->setDeclared();
        }

        return symbol;
    }

    core::ClassOrModuleRef insertClass(core::MutableContext ctx, const State &state, const core::FoundClass &klass,
                                       bool willDeleteOldDefs, ClassBehaviorLocsMap &classBehaviorLocs) {
        auto symbol = getClassSymbol(ctx, state, klass);

        if (klass.classKind == core::FoundClass::Kind::Class && !symbol.data(ctx)->superClass().exists() &&
            symbol != core::Symbols::BasicObject()) {
            symbol.data(ctx)->setSuperClass(core::Symbols::todo());
        }

        // In Ruby 2.5 they changed this class to have a different superclass
        // from 2.4. Since we don't have a good story around versioned ruby rbis
        // yet, lets just force the superclass regardless of version.
        if (symbol == core::Symbols::Net_IMAP()) {
            symbol.data(ctx)->setSuperClass(core::Symbols::Net_Protocol());
        }

        const bool isUnknown = klass.classKind == core::FoundClass::Kind::Unknown;
        const bool isModule = klass.classKind == core::FoundClass::Kind::Module;

        // Don't add locs for <root>; 1) they aren't useful and 2) they'll end up with O(files in project) locs!
        if (symbol != core::Symbols::root()) {
            // If the kind is unknown, it means it was only a FoundClass for a class or static field scope, not
            // the class def itself. We want to generally treat these as usage locs, not definition locs.
            //
            // At best, we only want to keep one loc in the codebase per unknown class for perf reasons. We don't
            // want to store O(files) locs for something like Opus. So if the existing loc (which would have been
            // brought into existence by getClassSymbol()) is not from this file, we don't add a new loc.
            //
            // If the unknown class loc is from the same file, it's still possible that it is from a real
            // definition in that file. In which case, we check the declared bit on the class.
            // We only set the loc if the class is not declared.
            bool updateLoc =
                !isUnknown || (!symbol.data(ctx)->isDeclared() && symbol.data(ctx)->loc().file() == ctx.file);
            if (updateLoc) {
                symbol.data(ctx)->addLoc(ctx, ctx.locAt(klass.declLoc));
            }

            if (!isUnknown) {
                if (klass.definesBehavior) {
                    auto &behaviorLocs = classBehaviorLocs[symbol];
                    behaviorLocs.emplace_back(ctx.locAt(klass.declLoc));
                    symbol.data(ctx)->flags.isBehaviorDefining = true;
                }

                auto singletonClass = symbol.data(ctx)->singletonClass(ctx); // force singleton class into existence
                singletonClass.data(ctx)->addLoc(ctx, ctx.locAt(klass.declLoc));

                // This willDeleteOldDefs condition is a hack to improve performance when editing within a method body.
                // Ideally, we would be able to make finalizeSymbols fast/incremental enough to run on all edits.
                if (willDeleteOldDefs) {
                    // Reset resultType to nullptr for idempotency on the fast path--it will always be
                    // re-entered in resolver.
                    symbol.data(ctx)->resultType = nullptr;
                    singletonClass.data(ctx)->resultType = nullptr;
                    // TODO(jez) This is a gross hack. We are basically re-implementing the logic in
                    // singletonClass to reset the <AttachedClass> type template to what it used to be.
                    // Is there a better way to accomplish this? (This is largely the same as the bad locs problem
                    // above; we can probably be more principled about what state calling `singletonClass` sets
                    // up/resets.)
                    if (!isModule) {
                        auto todo = core::make_type<core::ClassType>(core::Symbols::todo());
                        auto tp = singletonClass.data(ctx)
                                      ->members()[core::Names::Constants::AttachedClass()]
                                      .asTypeMemberRef();
                        tp.data(ctx)->resultType = core::make_type<core::LambdaParam>(tp, todo, todo);
                    }
                }
            }
        }

        // make sure we've added a static init symbol so we have it ready for the flatten pass later
        if (symbol == core::Symbols::root()) {
            ctx.state.staticInitForFile(ctx.locAt(klass.loc));
        } else if (!isUnknown) {
            ctx.state.staticInitForClass(symbol, ctx.locAt(klass.loc));
        }

        return symbol;
    }

    void modifyClass(core::MutableContext ctx, const core::FoundModifier &mod) {
        ENFORCE(mod.kind == core::FoundModifier::Kind::Class);
        const auto fun = mod.name;
        auto symbolData = ctx.owner.asClassOrModuleRef().data(ctx);
        if (fun == core::Names::declareFinal()) {
            symbolData->flags.isFinal = true;
            symbolData->singletonClass(ctx).data(ctx)->flags.isFinal = true;
        }
        if (fun == core::Names::declareSealed()) {
            symbolData->flags.isSealed = true;

            auto classOfKlass = symbolData->singletonClass(ctx);
            auto sealedSubclasses =
                ctx.state.enterMethodSymbol(ctx.locAt(mod.loc), classOfKlass, core::Names::sealedSubclasses());
            auto &blkArg =
                ctx.state.enterMethodArgumentSymbol(core::Loc::none(), sealedSubclasses, core::Names::blkArg());
            blkArg.flags.isBlock = true;

            // T.noreturn here represents the zero-length list of subclasses of this sealed class.
            // We will use T.any to record subclasses when they're resolved.
            vector<core::TypePtr> targs{core::Types::bottom()};
            sealedSubclasses.data(ctx)->resultType =
                core::make_type<core::AppliedType>(core::Symbols::Set(), move(targs));
        }
        if (fun == core::Names::declareInterface() || fun == core::Names::declareAbstract()) {
            symbolData->flags.isAbstract = true;
            symbolData->singletonClass(ctx).data(ctx)->flags.isAbstract = true;
        }
        if (fun == core::Names::declareInterface()) {
            symbolData->flags.isInterface = true;
            if (!symbolData->isModule()) {
                if (auto e = ctx.beginError(mod.loc, core::errors::Namer::InterfaceClass)) {
                    e.setHeader("Classes can't be interfaces. Use `abstract!` instead of `interface!`");
                    e.replaceWith("Change `interface!` to `abstract!`", ctx.locAt(mod.loc), "abstract!");
                }
            }
        }
    }

    core::FieldRef insertField(core::MutableContext ctx, const core::FoundField &field) {
        // resolver checks a whole bunch of various error conditions here; we just want to
        // know where to define the field.
        ENFORCE(ctx.owner.isClassOrModule(), "Actual {}", ctx.owner.show(ctx));
        auto scope = ctx.owner.enclosingClass(ctx);
        // cf. methodOwner in this file.
        if (field.fromWithinMethod && scope == core::Symbols::root()) {
            scope = core::Symbols::Object();
        }
        if (field.onSingletonClass) {
            scope = scope.data(ctx)->lookupSingletonClass(ctx);
        }
        ENFORCE(scope.exists());

        auto existing = scope.data(ctx)->findMember(ctx, field.name);
        if (existing.exists() && existing.isFieldOrStaticField()) {
            auto prior = existing.asFieldRef();
            // There was a previous declaration of this field in this file.  Ignore it;
            // let resolver deal with issuing the appropriate errors.
            if (prior.data(ctx)->resultType == core::Types::todo()) {
                return prior;
            }

            // We are on the fast path and there was a previous declaration.
            prior.data(ctx)->resultType = core::Types::todo();
            prior.data(ctx)->addLoc(ctx, ctx.locAt(field.loc));
            return prior;
        }

        core::FieldRef sym;
        if (field.kind == core::FoundField::Kind::InstanceVariable) {
            sym = ctx.state.enterFieldSymbol(ctx.locAt(field.loc), scope, field.name);
        } else {
            sym = ctx.state.enterStaticFieldSymbol(ctx.locAt(field.loc), scope, field.name);
        }

        sym.data(ctx)->resultType = core::Types::todo();
        return sym;
    }

    core::FieldRef insertStaticField(core::MutableContext ctx, const State &state,
                                     const core::FoundStaticField &staticField) {
        ENFORCE(ctx.owner.isClassOrModule());

        auto scope = getOwnerSymbol(state, staticField.owner);
        auto name = staticField.name;
        auto sym = ctx.state.lookupStaticFieldSymbol(scope, name);
        auto currSym = ctx.state.lookupSymbol(scope, name);
        if (!sym.exists() && currSym.exists()) {
            emitRedefinedConstantError(ctx, staticField.asgnLoc, name, core::SymbolRef::Kind::FieldOrStaticField,
                                       currSym);
            name = ctx.state.nextMangledName(scope, name);
        }
        if (sym.exists()) {
            ENFORCE(currSym.exists());
            name = sym.data(ctx)->name;
            if (isMangleRenameUniqueName(ctx, name)) {
                ENFORCE(currSym != sym);
                emitRedefinedConstantError(ctx, staticField.asgnLoc, sym, currSym);
            }
        }
        sym = ctx.state.enterStaticFieldSymbol(ctx.locAt(staticField.lhsLoc), scope, name);
        // Reset resultType to nullptr for idempotency on the fast path--it will always be
        // re-entered in resolver.
        sym.data(ctx)->resultType = nullptr;

        if (staticField.isTypeAlias) {
            sym.data(ctx)->flags.isStaticFieldTypeAlias = true;
        }

        return sym;
    }

    core::SymbolRef insertTypeMember(core::MutableContext ctx, const State &state,
                                     const core::FoundTypeMember &typeMember) {
        if (ctx.owner == core::Symbols::root()) {
            core::FoundStaticField staticField;
            staticField.owner = typeMember.owner;
            staticField.name = typeMember.name;
            staticField.asgnLoc = typeMember.asgnLoc;
            staticField.lhsLoc = typeMember.asgnLoc;
            staticField.isTypeAlias = true;
            return insertStaticField(ctx, state, staticField);
        }

        core::Variance variance = core::Variance::Invariant;
        const bool isTypeTemplate = typeMember.isTypeTemplate;

        auto onSymbol = isTypeTemplate ? ctx.owner.asClassOrModuleRef().data(ctx)->singletonClass(ctx)
                                       : ctx.owner.asClassOrModuleRef();

        core::NameRef foundVariance = typeMember.varianceName;
        if (foundVariance.exists()) {
            if (foundVariance == core::Names::covariant()) {
                variance = core::Variance::CoVariant;
            } else if (foundVariance == core::Names::contravariant()) {
                variance = core::Variance::ContraVariant;
            } else {
                if (auto e = ctx.beginError(typeMember.litLoc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Invalid variance kind, only `{}` and `{}` are supported",
                                ":" + core::Names::covariant().show(ctx), ":" + core::Names::contravariant().show(ctx));
                }
            }
        }

        core::TypeMemberRef sym;
        auto existingTypeMember = ctx.state.lookupTypeMemberSymbol(onSymbol, typeMember.name);
        if (existingTypeMember.exists()) {
            // if we already have a type member but it was constructed in a different file from the one we're
            // looking at, then we need to raise an error
            if (existingTypeMember.data(ctx)->loc().file() != ctx.file) {
                if (auto e = ctx.beginError(typeMember.asgnLoc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Duplicate type member `{}`", typeMember.name.show(ctx));
                    e.addErrorLine(existingTypeMember.data(ctx)->loc(), "Also defined here");
                }
                if (auto e = ctx.state.beginError(existingTypeMember.data(ctx)->loc(),
                                                  core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Duplicate type member `{}`", typeMember.name.show(ctx));
                    e.addErrorLine(ctx.locAt(typeMember.asgnLoc), "Also defined here");
                }
            }

            // otherwise, we're looking at a type member defined in this class in the same file, which means all we
            // need to do is find out whether there was a redefinition the first time, and in that case display the
            // same error
            auto name = existingTypeMember.data(ctx)->name;
            if (isMangleRenameUniqueName(ctx, name)) {
                auto oldSym = ctx.state.lookupSymbol(onSymbol, typeMember.name);
                emitRedefinedConstantError(ctx, typeMember.nameLoc, existingTypeMember, oldSym);
            }
            // if we have more than one type member with the same name, then we have messed up somewhere
            if (::sorbet::debug_mode) {
                auto members = onSymbol.data(ctx)->typeMembers();
                ENFORCE(absl::c_find_if(members, [&](auto mem) {
                            return mem.data(ctx)->name == existingTypeMember.data(ctx)->name;
                        }) != members.end());
            }
            sym = existingTypeMember;
        } else {
            auto name = typeMember.name;
            auto oldSym = onSymbol.data(ctx)->findMemberNoDealias(ctx, name);
            if (oldSym.exists()) {
                emitRedefinedConstantError(ctx, typeMember.nameLoc, oldSym.name(ctx), core::SymbolRef::Kind::TypeMember,
                                           oldSym);
                name = ctx.state.nextMangledName(onSymbol, name);
            }
            sym = ctx.state.enterTypeMember(ctx.locAt(typeMember.asgnLoc), onSymbol, name, variance);

            // The todo bounds will be fixed by the resolver in ResolveTypeParamsWalk.
            auto todo = core::make_type<core::ClassType>(core::Symbols::todo());
            sym.data(ctx)->resultType = core::make_type<core::LambdaParam>(sym, todo, todo);

            if (isTypeTemplate) {
                auto typeTemplateAliasName = typeMember.name;
                auto context = ctx.owner.enclosingClass(ctx);
                auto oldSym = context.data(ctx)->findMemberNoDealias(ctx, typeTemplateAliasName);
                if (oldSym.exists() &&
                    !(oldSym.loc(ctx) == ctx.locAt(typeMember.asgnLoc) || oldSym.loc(ctx).isTombStoned(ctx))) {
                    emitRedefinedConstantError(ctx, typeMember.nameLoc, name, core::SymbolRef::Kind::TypeMember,
                                               oldSym);
                    typeTemplateAliasName = ctx.state.nextMangledName(context, typeTemplateAliasName);
                }
                // This static field with an AliasType is how we get `MyTypeTemplate` to resolve,
                // because resolver does not usually look on the singleton class to resolve constant
                // literals, but type_template's are only ever entered on the singleton class.
                auto alias =
                    ctx.state.enterStaticFieldSymbol(ctx.locAt(typeMember.asgnLoc), context, typeTemplateAliasName);
                alias.data(ctx)->resultType = core::make_type<core::AliasType>(core::SymbolRef(sym));
            }
        }

        if (typeMember.isFixed) {
            sym.data(ctx)->flags.isFixed = true;
        }

        return sym;
    }

    bool matchesFullNameHash(core::Context ctx, core::NameRef memberNameToHash, core::FullNameHash oldNameHash) {
        if (memberNameToHash.kind() == core::NameKind::UNIQUE) {
            auto &uniqueData = memberNameToHash.dataUnique(ctx);
            if (uniqueData->uniqueNameKind == core::UniqueNameKind::MangleRename ||
                uniqueData->uniqueNameKind == core::UniqueNameKind::MangleRenameOverload ||
                uniqueData->uniqueNameKind == core::UniqueNameKind::Overload) {
                memberNameToHash = uniqueData->original;
            }
        }

        auto fieldFullNameHash = core::FullNameHash(ctx, memberNameToHash);
        return fieldFullNameHash == oldNameHash;
    }

    void deleteSymbolViaFullNameHash(core::MutableContext ctx, core::ClassOrModuleRef owner,
                                     core::FullNameHash oldNameHash) {
        // We have to accumulate a list of fields to delete, instead of deleting them in the loop
        // below, because deleting a field invalidates the members() iterator.
        vector<core::SymbolRef> toDelete;

        // Note: this loop is accidentally quadratic. We run deleteFieldViaFullNameHash once per field
        // previously defined in this file, then in each call look at each member of that field's owner.
        for (const auto &[memberName, memberSym] : owner.data(ctx)->members()) {
            if (memberSym.isClassOrModule()) {
                // Safeguard against a collision function in our `FullNameHash` function (e.g., what
                // if `foo()` and `::Foo` have the same `FullNameHash`, but were given two different
                // `NameRef` IDs and thus don't collide in the `members` list?).
                //
                // Any other collisions are fine, because we will already be re-entering the new
                // definitions later on in incremental namer. It's only classes that we definitely
                // never want to delete accidentally.
                continue;
            }

            if (!matchesFullNameHash(ctx, memberName, oldNameHash)) {
                continue;
            }

            toDelete.emplace_back(memberSym);
        }

        for (auto oldSymbol : toDelete) {
            oldSymbol.removeLocsForFile(ctx, ctx.file);
            if (oldSymbol.locs(ctx).empty()) {
                switch (oldSymbol.kind()) {
                    case core::SymbolRef::Kind::Method:
                        ctx.state.deleteMethodSymbol(oldSymbol.asMethodRef());
                        break;
                    case core::SymbolRef::Kind::FieldOrStaticField:
                        ctx.state.deleteFieldSymbol(oldSymbol.asFieldRef());
                        break;
                    case core::SymbolRef::Kind::TypeMember:
                        ctx.state.deleteTypeMemberSymbol(oldSymbol.asTypeMemberRef());
                        break;
                    case core::SymbolRef::Kind::ClassOrModule:
                    case core::SymbolRef::Kind::TypeArgument:
                        ENFORCE(false);
                        break;
                }
            }
        }
    }

    void deleteStaticFieldViaFullNameHash(core::MutableContext ctx, const SymbolDefiner::State &state,
                                          const core::FoundStaticFieldHash &oldDefHash) {
        ENFORCE(oldDefHash.nameHash.isDefined(), "Can't delete rename if old hash is not defined");

        auto owner = getOwnerSymbol(state, oldDefHash.owner());
        deleteSymbolViaFullNameHash(ctx, owner, oldDefHash.nameHash);
    }

    void deleteTypeMemberViaFullNameHash(core::MutableContext ctx, const SymbolDefiner::State &state,
                                         const core::FoundTypeMemberHash &oldDefHash) {
        ENFORCE(oldDefHash.nameHash.isDefined(), "Can't delete rename if old hash is not defined");

        // Changes to classes/modules take the slow path, so getOwnerSymbol is okay to call here
        auto owner = getOwnerSymbol(state, oldDefHash.owner());
        if (oldDefHash.isTypeTemplate) {
            // Also have to delete the static-field-class-alias that forwards from a constant
            // literal on the attached class to the type template on the singleton class
            deleteSymbolViaFullNameHash(ctx, owner, oldDefHash.nameHash);

            owner = owner.data(ctx)->singletonClass(ctx);
        }

        deleteSymbolViaFullNameHash(ctx, owner, oldDefHash.nameHash);
    }

    void deleteFieldViaFullNameHash(core::MutableContext ctx, const SymbolDefiner::State &state,
                                    const core::FoundFieldHash &oldDefHash) {
        ENFORCE(oldDefHash.nameHash.isDefined(), "Can't delete via hash if old hash is not defined");

        // Changes to classes/modules take the slow path, so getOwnerSymbol is okay to call here
        auto owner = getOwnerSymbol(state, oldDefHash.owner());
        if (oldDefHash.onSingletonClass) {
            owner = owner.data(ctx)->singletonClass(ctx);
        }

        deleteSymbolViaFullNameHash(ctx, owner, oldDefHash.nameHash);
    }

    void deleteMethodViaFullNameHash(core::MutableContext ctx, const SymbolDefiner::State &state,
                                     const core::FoundMethodHash &oldDefHash) {
        ENFORCE(oldDefHash.nameHash.isDefined(), "Can't delete via hash if old hash is not defined");

        // Changes to classes/modules take the slow path, so getOwnerSymbol is okay to call here
        auto ownerSymbol = getOwnerSymbol(state, oldDefHash.owner());
        auto owner = methodOwner(ctx, ownerSymbol, oldDefHash.useSingletonClass);

        deleteSymbolViaFullNameHash(ctx, owner, oldDefHash.nameHash);
    }

public:
    void deleteOldDefinitions(core::MutableContext ctx, const SymbolDefiner::State &state) {
        Timer timeit(ctx.state.tracer(), "deleteOldDefinitions");
        if (oldFoundHashes.has_value()) {
            const auto &oldFoundHashesVal = oldFoundHashes.value();

            for (const auto &oldStaticFieldHash : oldFoundHashesVal.staticFieldHashes) {
                deleteStaticFieldViaFullNameHash(ctx, state, oldStaticFieldHash);
            }

            for (const auto &oldTypeMemberHash : oldFoundHashesVal.typeMemberHashes) {
                deleteTypeMemberViaFullNameHash(ctx, state, oldTypeMemberHash);
            }

            for (auto klass : state.definedClasses) {
                // In the process of recovering from "parent type member must be re-declared in child" errors,
                // GlobalPass will create type members even if there isn't a `type_member` in the child class' body.
                // There won't be an entry in the old typeMemberHashes for these type members, so we have to
                // delete them manually.
                auto currentClass = klass;
                vector<core::TypeMemberRef> toDelete;
                do {
                    const auto &typeMembers = currentClass.data(ctx)->typeMembers();
                    toDelete.reserve(typeMembers.size());
                    for (const auto &typeMember : typeMembers) {
                        if (typeMember.data(ctx)->name == core::Names::Constants::AttachedClass()) {
                            // <AttachedClass> type templates are created when the singleton class was created.
                            // Since we don't delete and re-add classes on the fast path, we both can and must leave
                            // these `<AttachedClass>` type templates alone
                            continue;
                        }
                        toDelete.emplace_back(typeMember);
                    }

                    currentClass = currentClass.data(ctx)->lookupSingletonClass(ctx);
                } while (currentClass.exists());

                for (auto oldTypeMember : toDelete) {
                    oldTypeMember.data(ctx)->removeLocsForFile(ctx.file);
                    if (oldTypeMember.data(ctx)->locs().empty()) {
                        ctx.state.deleteTypeMemberSymbol(oldTypeMember);
                    }
                }
            }

            for (const auto &oldFieldHash : oldFoundHashesVal.fieldHashes) {
                deleteFieldViaFullNameHash(ctx, state, oldFieldHash);
            }

            for (const auto &oldMethodHash : oldFoundHashesVal.methodHashes) {
                // Since we've already processed all the non-method symbols (which includes classes), we now
                // guarantee that deleteViaFullNameHash can use getOwnerSymbol to lookup an old owner
                // ref in the new definedClasses vector.
                deleteMethodViaFullNameHash(ctx, state, oldMethodHash);
            }
        }
    }

    SymbolDefiner(const core::FoundDefinitions &foundDefs, optional<core::FoundDefHashes> oldFoundHashes)
        : foundDefs(foundDefs), oldFoundHashes(move(oldFoundHashes)) {}

    SymbolDefiner::State enterClassDefinitions(core::MutableContext ctx, bool willDeleteOldDefs,
                                               ClassBehaviorLocsMap &classBehaviorLocs) {
        SymbolDefiner::State state;
        state.definedClasses.reserve(foundDefs.klasses().size());

        for (const auto &klass : foundDefs.klasses()) {
            state.definedClasses.emplace_back(insertClass(ctx.withOwner(getOwnerSymbol(state, klass.owner)), state,
                                                          klass, willDeleteOldDefs, classBehaviorLocs));
        }

        return state;
    }

    void enterNewDefinitions(core::MutableContext ctx, SymbolDefiner::State &&state) {
        // We have to defer defining non-class constant symbols until this (second) phase of incremental
        // namer so that we don't delete and immediately re-enter a symbol (possibly keeping it
        // alive, if it had multiple locs at the time of deletion) before SymbolDefiner has had a
        // chance to process _all_ files.

        for (auto ref : foundDefs.nonClassConstants()) {
            switch (ref.kind()) {
                case core::FoundDefinitionRef::Kind::StaticField: {
                    const auto &staticField = ref.staticField(foundDefs);
                    insertStaticField(ctx.withOwner(getOwnerSymbol(state, staticField.owner)), state, staticField);
                    break;
                }
                case core::FoundDefinitionRef::Kind::TypeMember: {
                    const auto &typeMember = ref.typeMember(foundDefs);
                    insertTypeMember(ctx.withOwner(getOwnerSymbol(state, typeMember.owner)), state, typeMember);
                    break;
                }
                case core::FoundDefinitionRef::Kind::Class:
                case core::FoundDefinitionRef::Kind::Empty:
                case core::FoundDefinitionRef::Kind::Method:
                case core::FoundDefinitionRef::Kind::Field:
                case core::FoundDefinitionRef::Kind::Symbol:
                    ENFORCE(false, "Unexpected definition ref {}", core::FoundDefinitionRef::kindToString(ref.kind()));
                    break;
            }
        }

        for (auto &method : foundDefs.methods()) {
            if (method.arityHash.isAliasMethod()) {
                // We need alias methods in the FoundDefinitions list not so that we can actually
                // create method symbols for them yet, but just so we can know which alias methods
                // to delete on the fast path. Alias methods will be defined later, in resolver.
                continue;
            }
            insertMethod(ctx.withOwner(getOwnerSymbol(state, method.owner)), method);
        }

        for (auto &field : foundDefs.fields()) {
            insertField(ctx.withOwner(getOwnerSymbol(state, field.owner)), field);
        }

        for (const auto &modifier : foundDefs.modifiers()) {
            const auto owner = getOwnerSymbol(state, modifier.owner);
            switch (modifier.kind) {
                case core::FoundModifier::Kind::Method:
                    modifyMethod(ctx.withOwner(owner), modifier);
                    break;
                case core::FoundModifier::Kind::Class:
                    modifyClass(ctx.withOwner(owner), modifier);
                    break;
                case core::FoundModifier::Kind::ClassOrStaticField:
                    modifyConstant(ctx.withOwner(owner), modifier);
                    break;
            }
        }
    }
};

/**
 * Inserts newly created symbols (from SymbolDefiner) into a tree.
 */
class TreeSymbolizer {
    friend class Namer;

    core::SymbolRef squashNamesInner(core::Context ctx, core::SymbolRef owner, ast::ExpressionPtr &node,
                                     bool firstName) {
        auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(node);
        if (constLit == nullptr) {
            if (auto *id = ast::cast_tree<ast::ConstantLit>(node)) {
                return id->symbol.dealias(ctx);
            }
            if (auto *uid = ast::cast_tree<ast::UnresolvedIdent>(node)) {
                if (uid->kind != ast::UnresolvedIdent::Kind::Class || uid->name != core::Names::singleton()) {
                    if (auto e = ctx.beginError(node.loc(), core::errors::Namer::DynamicConstant)) {
                        e.setHeader("Unsupported constant scope");
                    }
                }
                // emitted via `class << self` blocks
            } else if (ast::isa_tree<ast::EmptyTree>(node)) {
                // ::Foo
            } else if (node.isSelfReference()) {
                // self::Foo
            } else {
                if (auto e = ctx.beginError(node.loc(), core::errors::Namer::DynamicConstant)) {
                    e.setHeader("Dynamic constant references are unsupported");
                }
            }
            node = ast::MK::EmptyTree();
            return owner;
        }

        const bool firstNameRecursive = false;
        auto newOwner = squashNamesInner(ctx, owner, constLit->scope, firstNameRecursive);
        ENFORCE(newOwner.exists());

        core::SymbolRef existing = ctx.state.lookupClassSymbol(newOwner.asClassOrModuleRef(), constLit->cnst);
        if (firstName && !existing.exists() && newOwner.isClassOrModule()) {
            existing = ctx.state.lookupStaticFieldSymbol(newOwner.asClassOrModuleRef(), constLit->cnst);
            if (existing.exists()) {
                existing = existing.dealias(ctx.state);
            }
        }

        // NameInserter should have created this symbol
        ENFORCE(existing.exists());

        node = ast::make_expression<ast::ConstantLit>(constLit->loc, existing, std::move(node));
        return existing;
    }

    core::SymbolRef squashNames(core::Context ctx, core::SymbolRef owner, ast::ExpressionPtr &node) {
        const bool firstName = true;
        return squashNamesInner(ctx, owner, node, firstName);
    }

    ast::ExpressionPtr arg2Symbol(int pos, const core::ParsedArg &parsedArg, ast::ExpressionPtr arg) {
        ast::ExpressionPtr localExpr = ast::make_expression<ast::Local>(parsedArg.loc, parsedArg.local);
        if (parsedArg.flags.isDefault) {
            localExpr =
                ast::MK::OptionalArg(parsedArg.loc, move(localExpr), ast::ArgParsing::getDefault(parsedArg, move(arg)));
        }
        return localExpr;
    }

public:
    TreeSymbolizer() {}

    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        auto *ident = ast::cast_tree<ast::UnresolvedIdent>(klass.name);

        if ((ident != nullptr) && ident->name == core::Names::singleton()) {
            ENFORCE(ident->kind == ast::UnresolvedIdent::Kind::Class);
            klass.symbol = ctx.owner.enclosingClass(ctx).data(ctx)->lookupSingletonClass(ctx);
            ENFORCE(klass.symbol.exists());
        } else {
            auto symbol = klass.symbol;
            if (symbol == core::Symbols::todo()) {
                auto squashedSymbol = squashNames(ctx, ctx.owner.enclosingClass(ctx), klass.name);
                ENFORCE(squashedSymbol.exists());
                klass.symbol = squashedSymbol.asClassOrModuleRef();
            } else {
                // Desugar populates a top-level root() ClassDef.
                // Nothing else should have been typeAlias by now.
                ENFORCE(symbol == core::Symbols::root());
            }
        }
    }

    // This decides if we need to keep a node around incase the current LSP query needs type information for it
    bool shouldLeaveAncestorForIDE(const ast::ExpressionPtr &anc) {
        // used in Desugar <-> resolver to signal classes that did not have explicit superclass
        if (ast::isa_tree<ast::EmptyTree>(anc) || anc.isSelfReference()) {
            return false;
        }
        auto rcl = ast::cast_tree<ast::ConstantLit>(anc);
        if (rcl && rcl->symbol == core::Symbols::todo()) {
            return false;
        }
        return true;
    }

    void postTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        ENFORCE(klass.symbol != core::Symbols::todo());

        // NameDefiner should have forced this class's singleton class into existence.
        ENFORCE(klass.symbol.data(ctx)->lookupSingletonClass(ctx).exists());

        // This is an invariant that namer must introduce (all ClassDef's have `<static-init>` methods).
        // ENFORCE'ing it here makes certain errors apparent earlier.
        auto allowMissing = true;
        ENFORCE(ctx.state.lookupStaticInitForClass(klass.symbol, allowMissing).exists());

        auto loc = klass.declLoc;
        ast::InsSeq::STATS_store retSeqs;
        retSeqs.emplace_back(std::move(tree));
        if (ast::isa_tree<ast::ConstantLit>(klass.name)) {
            retSeqs.emplace_back(ast::MK::KeepForIDE(loc.copyWithZeroLength(), klass.name.deepCopy()));
        }
        if (klass.kind == ast::ClassDef::Kind::Class && !klass.ancestors.empty() &&
            shouldLeaveAncestorForIDE(klass.ancestors.front())) {
            retSeqs.emplace_back(ast::MK::KeepForIDE(loc.copyWithZeroLength(), klass.ancestors.front().deepCopy()));
        }

        tree = ast::MK::InsSeq(loc, std::move(retSeqs), ast::MK::EmptyTree());
    }

    ast::MethodDef::ARGS_store fillInArgs(vector<core::ParsedArg> parsedArgs, ast::MethodDef::ARGS_store oldArgs) {
        ast::MethodDef::ARGS_store args;
        int i = -1;
        for (auto &arg : parsedArgs) {
            i++;
            auto localVariable = arg.local;

            if (arg.flags.isShadow) {
                auto localExpr = ast::make_expression<ast::Local>(arg.loc, localVariable);
                args.emplace_back(move(localExpr));
            } else {
                ENFORCE(i < oldArgs.size());
                auto expr = arg2Symbol(i, arg, move(oldArgs[i]));
                args.emplace_back(move(expr));
            }
        }

        return args;
    }

    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &method = ast::cast_tree_nonnull<ast::MethodDef>(tree);

        auto owner = methodOwner(ctx, ctx.owner, method.flags.isSelfMethod);
        auto parsedArgs = ast::ArgParsing::parseArgs(method.args);
        auto sym = ctx.state.lookupMethodSymbolWithHash(owner, method.name, ast::ArgParsing::hashArgs(ctx, parsedArgs));
        ENFORCE(sym.exists());
        method.symbol = sym;
        method.args = fillInArgs(move(parsedArgs), std::move(method.args));
    }

    void postTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &method = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        ENFORCE(method.symbol != core::Symbols::todoMethod());

        ENFORCE(method.args.size() == method.symbol.data(ctx)->arguments.size(), "{}: {} != {}",
                method.name.showRaw(ctx), method.args.size(), method.symbol.data(ctx)->arguments.size());
    }

    ast::ExpressionPtr handleAssignment(core::Context ctx, ast::ExpressionPtr tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);
        auto &lhs = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(asgn.lhs);

        auto maybeScope = squashNames(ctx, contextClass(ctx, ctx.owner), lhs.scope);
        ENFORCE(maybeScope.exists());

        if (!maybeScope.isClassOrModule()) {
            auto scopeName = maybeScope.name(ctx);
            maybeScope = ctx.state.lookupClassSymbol(maybeScope.owner(ctx).asClassOrModuleRef(), scopeName);
        }
        auto scope = maybeScope.asClassOrModuleRef();

        core::SymbolRef cnst = ctx.state.lookupStaticFieldSymbol(scope, lhs.cnst);
        ENFORCE(cnst.exists());
        auto loc = lhs.loc;
        asgn.lhs = ast::make_expression<ast::ConstantLit>(loc, cnst, std::move(asgn.lhs));

        return tree;
    }

    void lazyTypeMemberAutocorrect(core::Context ctx, core::ErrorBuilder &e, const ast::Send *send,
                                   core::Loc kwArgsLoc) {
        auto deleteLoc = kwArgsLoc;
        auto prefix = deleteLoc.adjustLen(ctx, -2, 2);
        if (prefix.source(ctx) == ", ") {
            deleteLoc = prefix.join(deleteLoc);
        }
        prefix = deleteLoc.adjustLen(ctx, -1, 1);
        if (prefix.source(ctx) == " ") {
            deleteLoc = prefix.join(deleteLoc);
        }

        auto surroundingLoc = deleteLoc.adjust(ctx, -1, 1);
        auto surroundingSrc = surroundingLoc.source(ctx);
        if (surroundingSrc.has_value() && absl::StartsWith(surroundingSrc.value(), "(") &&
            absl::EndsWith(surroundingSrc.value(), ")")) {
            deleteLoc = surroundingLoc;
        }

        auto multiline = false;
        auto indent = ""s;
        auto sendLoc = ctx.locAt(send->loc);
        auto [beginDetail, endDetail] = sendLoc.position(ctx);
        if (endDetail.line > beginDetail.line && send->funLoc.exists() && !send->funLoc.empty() &&
            send->numPosArgs() == 0) {
            deleteLoc = core::Loc(ctx.file, send->funLoc.endPos(), send->loc.endPos());
            multiline = true;
            auto [_, indentLen] = sendLoc.copyEndWithZeroLength().findStartOfLine(ctx);
            indent = string(indentLen, ' ');
        }

        auto edits = vector<core::AutocorrectSuggestion::Edit>{};
        edits.emplace_back(core::AutocorrectSuggestion::Edit{deleteLoc, ""});
        auto insertLoc = ctx.locAt(send->loc).copyEndWithZeroLength();
        auto kwArgsSource = kwArgsLoc.source(ctx).value();
        if (multiline) {
            auto [kwBeginDetail, kwEndDetail] = kwArgsLoc.position(ctx);
            if (kwEndDetail.line > kwBeginDetail.line) {
                auto reindentedSource = absl::StrReplaceAll(kwArgsSource, {{"\n", "\n  "}});
                if (kwBeginDetail.line == beginDetail.line) {
                    reindentedSource = absl::StrReplaceAll(reindentedSource, {{"\n", "\n  "}});
                }
                edits.emplace_back(core::AutocorrectSuggestion::Edit{
                    insertLoc, fmt::format(" do\n{0}  {{\n{0}    {1},\n{0}  }}\n{0}end", indent, reindentedSource)});
            } else {
                edits.emplace_back(core::AutocorrectSuggestion::Edit{
                    insertLoc, fmt::format(" do\n{0}  {{{1}}}\n{0}end", indent, kwArgsSource)});
            }
        } else {
            edits.emplace_back(
                core::AutocorrectSuggestion::Edit{insertLoc, fmt::format(" {{ {{{}}} }}", kwArgsSource)});
        }
        e.addAutocorrect(core::AutocorrectSuggestion{
            fmt::format("Convert `{}` to block form", send->fun.show(ctx)),
            move(edits),
        });
    }

    ast::ExpressionPtr handleTypeMemberDefinition(core::Context ctx, ast::ExpressionPtr tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);
        auto *send = ast::cast_tree<ast::Send>(asgn.rhs);
        ENFORCE(send != nullptr);
        const auto *typeName = ast::cast_tree<ast::UnresolvedConstantLit>(asgn.lhs);
        ENFORCE(typeName != nullptr);

        if (!ctx.owner.isClassOrModule()) {
            if (auto e = ctx.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                e.setHeader("Types must be defined in class or module scopes");
            }
            return ast::MK::EmptyTree();
        }
        if (ctx.owner == core::Symbols::root() || isBadHasAttachedClass(ctx, typeName->cnst)) {
            if (ctx.owner == core::Symbols::root()) {
                if (auto e = ctx.beginError(send->loc, core::errors::Namer::RootTypeMember)) {
                    e.setHeader("`{}` cannot be used at the top-level", "type_member");
                }
            } else {
            }
            auto send = ast::MK::Send0Block(asgn.loc, ast::MK::T(asgn.loc), core::Names::typeAlias(),
                                            asgn.loc.copyWithZeroLength(),
                                            ast::MK::Block0(asgn.loc, ast::MK::Untyped(asgn.loc)));

            return handleAssignment(ctx,
                                    ast::make_expression<ast::Assign>(asgn.loc, std::move(asgn.lhs), std::move(send)));
        }

        if (isBadHasAttachedClass(ctx, typeName->cnst)) {
            // We could go out of our way to try to not even define the <AttachedClass> type member,
            // in an attempt to maintain an invariant that <AttachedClass> is only ever defined on
            // - modules
            // - class singleton classes
            // - ::Class itself
            // But then in GlobalPass, resolveTypeMember would simply mangle rename things so that
            // the <AttachedClass> symbol pointed to a type member symbol anyways (instead of a
            // static field or type alias symbol). So let's just report an error and then continue
            // to let the type member be defined, shedding a single tear that we can't enforce the
            // invariant we might have otherwise wanted.
            if (auto e = ctx.beginError(asgn.loc, core::errors::Namer::HasAttachedClassInClass)) {
                // This is the simple way to explain the error to users, even though the
                // condition above is more complicated. The one exception to the way this error
                // is phrased: `::Class` itself, which is a `class`, is allowed to use `has_attached_class!`.
                //
                // But since `::Class` is final (even according to the VM), and since we've
                // already marked `::Class` with `has_attached_class!`, it's not worth leaking
                // that special case to the user.
                e.setHeader("`{}` can only be used inside a `{}`, not a `{}`",
                            core::Names::declareHasAttachedClass().show(ctx), "module", "class");
            }
        }

        bool isTypeTemplate = send->fun == core::Names::typeTemplate();
        auto onSymbol =
            isTypeTemplate ? ctx.owner.asClassOrModuleRef().data(ctx)->lookupSingletonClass(ctx) : ctx.owner;
        ENFORCE(onSymbol.exists());
        core::SymbolRef sym = ctx.state.lookupTypeMemberSymbol(onSymbol.asClassOrModuleRef(), typeName->cnst);
        ENFORCE(sym.exists());

        // Simulates how squashNames in handleAssignment also creates a ConstantLit
        // (simpler than squashNames, because type members are not allowed to use any sort of
        // `A::B = type_member` syntax)
        asgn.lhs = ast::make_expression<ast::ConstantLit>(asgn.lhs.loc(), sym, move(asgn.lhs));

        if (send->hasPosArgs() || send->hasKwArgs() || send->block() != nullptr) {
            if (send->numPosArgs() > 1) {
                if (auto e = ctx.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Too many args in type definition");
                }
                auto send = ast::MK::Send1(asgn.loc, ast::MK::T(asgn.loc), core::Names::typeAlias(),
                                           asgn.loc.copyWithZeroLength(), ast::MK::Untyped(asgn.loc));
                return handleAssignment(
                    ctx, ast::make_expression<ast::Assign>(asgn.loc, std::move(asgn.lhs), std::move(send)));
            }

            if (send->hasKwArgs()) {
                const auto numKwArgs = send->numKwArgs();
                auto kwArgsLoc = ctx.locAt(send->getKwKey(0).loc().join(send->getKwValue(numKwArgs - 1).loc()));
                if (auto e = ctx.state.beginError(kwArgsLoc, core::errors::Namer::OldTypeMemberSyntax)) {
                    e.setHeader("The `{}` syntax for bounds has changed to use a block instead of keyword args",
                                send->fun.show(ctx));

                    if (kwArgsLoc.exists() && send->block() == nullptr) {
                        lazyTypeMemberAutocorrect(ctx, e, send, kwArgsLoc);
                    } else {
                        e.addErrorNote("Provide these keyword args in a block that returns a `{}` literal", "Hash");
                    }
                }
            }

            if (send->block() != nullptr) {
                bool fixed = false;
                bool bounded = false;

                if (const auto *hash = ast::cast_tree<ast::Hash>(send->block()->body)) {
                    for (const auto &keyExpr : hash->keys) {
                        const auto *key = ast::cast_tree<ast::Literal>(keyExpr);
                        if (key == nullptr || !key->isSymbol()) {
                            if (auto e = ctx.beginError(keyExpr.loc(), core::errors::Namer::InvalidTypeDefinition)) {
                                e.setHeader("Hash provided in block to `{}` must have symbol keys",
                                            send->fun.show(ctx));
                            }
                            return tree;
                        }

                        switch (key->asSymbol().rawId()) {
                            case core::Names::fixed().rawId():
                                fixed = true;
                                break;

                            case core::Names::lower().rawId():
                            case core::Names::upper().rawId():
                                bounded = true;
                                break;

                            default:
                                if (auto e =
                                        ctx.beginError(keyExpr.loc(), core::errors::Namer::InvalidTypeDefinition)) {
                                    e.setHeader("Unknown key `{}` provided in block to `{}`", key->asSymbol().show(ctx),
                                                send->fun.show(ctx));
                                }
                                return tree;
                        }
                    }
                } else {
                    if (auto e =
                            ctx.beginError(send->block()->body.loc(), core::errors::Namer::InvalidTypeDefinition)) {
                        e.setHeader("Block given to `{}` must contain a single `{}` literal", send->fun.show(ctx),
                                    "Hash");
                        return tree;
                    }
                }

                // one of fixed or bounds were provided
                if (fixed != bounded) {
                    asgn.lhs = ast::MK::Constant(asgn.lhs.loc(), sym);

                    // Leave it in the tree for the resolver to chew on.
                    return tree;
                } else if (fixed) {
                    // both fixed and bounds were specified
                    if (auto e = ctx.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                        e.setHeader("Type member is defined with bounds and `{}`", "fixed");
                    }
                } else {
                    if (auto e = ctx.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                        e.setHeader("Missing required param `{}`", "fixed");
                    }
                }
            }

            if (send->numPosArgs() > 0) {
                auto *lit = ast::cast_tree<ast::Literal>(send->getPosArg(0));
                if (!lit || !lit->isSymbol()) {
                    if (auto e = ctx.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                        e.setHeader("Invalid param, must be a :symbol");
                    }
                }
            }
        }

        return tree;
    }

    void postTransformAssign(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);

        auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn.lhs);
        if (lhs == nullptr) {
            return;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn.rhs);
        if (send == nullptr) {
            tree = handleAssignment(ctx, std::move(tree));
        } else if (!send->recv.isSelfReference()) {
            tree = handleAssignment(ctx, std::move(tree));
        } else if (!ast::isa_tree<ast::EmptyTree>(lhs->scope)) {
            tree = handleAssignment(ctx, std::move(tree));
        } else {
            switch (send->fun.rawId()) {
                case core::Names::typeTemplate().rawId():
                case core::Names::typeMember().rawId(): {
                    tree = handleTypeMemberDefinition(ctx, std::move(tree));
                    break;
                }
                default: {
                    tree = handleAssignment(ctx, std::move(tree));
                    break;
                }
            }
        }
    }
};

vector<SymbolFinderResult> findSymbols(const core::GlobalState &gs, vector<ast::ParsedFile> trees,
                                       WorkerPool &workers) {
    Timer timeit(gs.tracer(), "naming.findSymbols");
    auto resultq = make_shared<BlockingBoundedQueue<vector<SymbolFinderResult>>>(trees.size());
    auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
    vector<SymbolFinderResult> allFoundDefinitions;
    allFoundDefinitions.reserve(trees.size());
    for (auto &tree : trees) {
        fileq->push(move(tree), 1);
    }

    workers.multiplexJob("findSymbols", [&gs, fileq, resultq]() {
        Timer timeit(gs.tracer(), "naming.findSymbolsWorker");
        SymbolFinder finder;
        vector<SymbolFinderResult> output;
        ast::ParsedFile job;
        for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
            if (result.gotItem()) {
                Timer timeit(gs.tracer(), "naming.findSymbolsOne", {{"file", string(job.file.data(gs).path())}});
                core::Context ctx(gs, core::Symbols::root(), job.file);
                ast::TreeWalk::apply(ctx, finder, job.tree);
                SymbolFinderResult jobOutput{move(job), finder.getAndClearFoundDefinitions()};
                output.emplace_back(move(jobOutput));
            }
        }
        if (!output.empty()) {
            resultq->push(move(output), output.size());
        }
    });
    trees.clear();

    {
        vector<SymbolFinderResult> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem()) {
                allFoundDefinitions.insert(allFoundDefinitions.end(), make_move_iterator(threadResult.begin()),
                                           make_move_iterator(threadResult.end()));
            }
        }
    }
    fast_sort(allFoundDefinitions,
              [](const auto &lhs, const auto &rhs) -> bool { return lhs.tree.file < rhs.tree.file; });

    return allFoundDefinitions;
}

void populateFoundDefHashes(core::Context ctx, core::FoundDefinitions &foundDefs,
                            core::FoundDefHashes &foundHashesOut) {
    ENFORCE(foundHashesOut.staticFieldHashes.empty());
    foundHashesOut.staticFieldHashes.reserve(foundDefs.staticFields().size());
    for (const auto &staticField : foundDefs.staticFields()) {
        auto owner = staticField.owner;
        ENFORCE(owner.kind() == core::FoundDefinitionRef::Kind::Class ||
                    owner.kind() == core::FoundDefinitionRef::Kind::Symbol,
                "kind={}", core::FoundDefinitionRef::kindToString(owner.kind()));
        auto ownerIsSymbol = owner.kind() == core::FoundDefinitionRef::Kind::Symbol;
        auto fullNameHash = core::FullNameHash(ctx, staticField.name);
        foundHashesOut.staticFieldHashes.emplace_back(owner.idx(), ownerIsSymbol, fullNameHash);
    }

    ENFORCE(foundHashesOut.typeMemberHashes.empty());
    foundHashesOut.typeMemberHashes.reserve(foundDefs.typeMembers().size());
    for (const auto &typeMember : foundDefs.typeMembers()) {
        auto owner = typeMember.owner;
        ENFORCE(owner.kind() == core::FoundDefinitionRef::Kind::Class ||
                    owner.kind() == core::FoundDefinitionRef::Kind::Symbol,
                "kind={}", core::FoundDefinitionRef::kindToString(owner.kind()));
        auto ownerIsSymbol = owner.kind() == core::FoundDefinitionRef::Kind::Symbol;
        auto fullNameHash = core::FullNameHash(ctx, typeMember.name);
        foundHashesOut.typeMemberHashes.emplace_back(owner.idx(), ownerIsSymbol, typeMember.isTypeTemplate,
                                                     fullNameHash);
    }

    ENFORCE(foundHashesOut.methodHashes.empty());
    foundHashesOut.methodHashes.reserve(foundDefs.methods().size());
    for (const auto &method : foundDefs.methods()) {
        auto owner = method.owner;
        ENFORCE(owner.kind() == core::FoundDefinitionRef::Kind::Class, "kind={}",
                core::FoundDefinitionRef::kindToString(owner.kind()));
        auto ownerIsSymbol = owner.kind() == core::FoundDefinitionRef::Kind::Symbol;
        auto fullNameHash = core::FullNameHash(ctx, method.name);
        foundHashesOut.methodHashes.emplace_back(owner.idx(), ownerIsSymbol, method.flags.isSelfMethod, fullNameHash,
                                                 method.arityHash);
    }

    ENFORCE(foundHashesOut.fieldHashes.empty());
    foundHashesOut.fieldHashes.reserve(foundDefs.fields().size());
    for (const auto &field : foundDefs.fields()) {
        auto owner = field.owner;
        ENFORCE(owner.kind() == core::FoundDefinitionRef::Kind::Class, "kind={}",
                core::FoundDefinitionRef::kindToString(owner.kind()));
        auto ownerIsSymbol = owner.kind() == core::FoundDefinitionRef::Kind::Symbol;
        auto fullNameHash = core::FullNameHash(ctx, field.name);
        foundHashesOut.fieldHashes.emplace_back(owner.idx(), ownerIsSymbol, field.onSingletonClass,
                                                field.kind == core::FoundField::Kind::InstanceVariable,
                                                field.fromWithinMethod, fullNameHash);
    }
}

void findConflictingClassDefs(const core::GlobalState &gs, ClassBehaviorLocsMap &classBehaviorLocs) {
    vector<pair<core::ClassOrModuleRef, BehaviorLocs>> conflicts;
    for (auto &[ref, locs] : classBehaviorLocs) {
        if (locs.size() < 2) {
            continue;
        }
        fast_sort(locs, [](const auto &lhs, const auto &rhs) -> bool { return lhs.file() < rhs.file(); });
        // In rare cases we see multiple defs in same file. Ignore them.
        auto last = unique(locs.begin(), locs.end(),
                           [](const auto &lhs, const auto &rhs) -> bool { return lhs.file() == rhs.file(); });
        locs.erase(last, locs.end());
        if (locs.size() < 2) {
            continue;
        }
        conflicts.emplace_back(make_pair(ref, std::move(locs)));
    }
    classBehaviorLocs.clear();

    fast_sort(conflicts, [](const auto &lhs, const auto &rhs) -> bool { return lhs.first.id() < rhs.first.id(); });
    for (const auto &[ref, locs] : conflicts) {
        core::Loc mainLoc = locs[0];
        core::Context ctx(gs, core::Symbols::root(), mainLoc.file());
        if (auto e = ctx.beginError(mainLoc.offsets(), core::errors::Namer::MultipleBehaviorDefs)) {
            e.setHeader("`{}` has behavior defined in multiple files", ref.show(ctx));
            for (auto it = locs.begin() + 1; it != locs.end(); ++it) {
                e.addErrorLine(*it, "Previous definition");
            }
        }
    }
}

ast::ParsedFilesOrCancelled defineSymbols(core::GlobalState &gs, vector<SymbolFinderResult> allFoundDefinitions,
                                          WorkerPool &workers,
                                          UnorderedMap<core::FileRef, core::FoundDefHashes> &&oldFoundHashesForFiles,
                                          core::FoundDefHashes *foundHashesOut) {
    Timer timeit(gs.tracer(), "naming.defineSymbols");
    vector<ast::ParsedFile> output;
    output.reserve(allFoundDefinitions.size());
    const auto &epochManager = *gs.epochManager;
    uint32_t count = 0;
    uint32_t foundMethods = 0;
    ClassBehaviorLocsMap classBehaviorLocs;
    UnorderedMap<core::FileRef, SymbolDefiner::State> incrementalDefinitions;
    auto willDeleteOldDefs = !oldFoundHashesForFiles.empty();
    for (auto &fileFoundDefinitions : allFoundDefinitions) {
        foundMethods += fileFoundDefinitions.names->methods().size();
        count++;
        // defineSymbols is really fast. Avoid this mildly expensive check for most turns of the loop.
        if (count % 250 == 0 && epochManager.wasTypecheckingCanceled()) {
            for (; count <= allFoundDefinitions.size(); count++) {
                output.emplace_back(move(allFoundDefinitions[count - 1].tree));
            }
            return ast::ParsedFilesOrCancelled::cancel(move(output), workers);
        }
        auto fref = fileFoundDefinitions.tree.file;
        core::MutableContext ctx(gs, core::Symbols::root(), fref);

        auto frefIt = oldFoundHashesForFiles.find(fref);
        auto oldFoundHashes =
            frefIt == oldFoundHashesForFiles.end() ? optional<core::FoundDefHashes>() : std::move(frefIt->second);
        SymbolDefiner symbolDefiner(*fileFoundDefinitions.names, move(oldFoundHashes));
        auto state = symbolDefiner.enterClassDefinitions(ctx, willDeleteOldDefs, classBehaviorLocs);
        if (willDeleteOldDefs) {
            symbolDefiner.deleteOldDefinitions(ctx, state);
        }
        incrementalDefinitions[fref] = move(state);
        output.emplace_back(move(fileFoundDefinitions.tree));
        if (foundHashesOut != nullptr) {
            populateFoundDefHashes(ctx, *fileFoundDefinitions.names, *foundHashesOut);
        }
    }

    findConflictingClassDefs(gs, classBehaviorLocs);
    ENFORCE(incrementalDefinitions.size() == allFoundDefinitions.size());
    prodCounterAdd("types.input.foundmethods.total", foundMethods);
    count = 0;
    for (auto &fileFoundDefinitions : allFoundDefinitions) {
        count++;
        if (count % 250 == 0 && epochManager.wasTypecheckingCanceled()) {
            return ast::ParsedFilesOrCancelled::cancel(move(output), workers);
        }

        auto fref = fileFoundDefinitions.tree.file;
        // The contents of this don't matter for incremental definition.  The
        // old definitions should only matter when deleting old symbols (which
        // happened in the previous phase of incremental namer), not here when
        // entering symbols.
        optional<core::FoundDefHashes> oldFoundHashes;
        core::MutableContext ctx(gs, core::Symbols::root(), fref);

        SymbolDefiner symbolDefiner(*fileFoundDefinitions.names, oldFoundHashes);
        symbolDefiner.enterNewDefinitions(ctx, move(incrementalDefinitions[fref]));
    }
    return output;
}

struct SymbolizeTreesResult {
    vector<ast::ParsedFile> trees;
};

vector<ast::ParsedFile> symbolizeTrees(const core::GlobalState &gs, vector<ast::ParsedFile> trees,
                                       WorkerPool &workers) {
    Timer timeit(gs.tracer(), "naming.symbolizeTrees");
    auto resultq = make_shared<BlockingBoundedQueue<SymbolizeTreesResult>>(trees.size());
    auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
    for (auto &tree : trees) {
        fileq->push(move(tree), 1);
    }

    workers.multiplexJob("symbolizeTrees", [&gs, fileq, resultq]() {
        Timer timeit(gs.tracer(), "naming.symbolizeTreesWorker");
        TreeSymbolizer inserter;
        SymbolizeTreesResult output;
        ast::ParsedFile job;
        for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
            if (result.gotItem()) {
                Timer timeit(gs.tracer(), "naming.symbolizeTreesOne", {{"file", string(job.file.data(gs).path())}});
                core::Context ctx(gs, core::Symbols::root(), job.file);
                ast::TreeWalk::apply(ctx, inserter, job.tree);
                output.trees.emplace_back(move(job));
            }
        }
        if (!output.trees.empty()) {
            resultq->push(move(output), output.trees.size());
        }
    });
    trees.clear();

    {
        SymbolizeTreesResult threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem()) {
                trees.insert(trees.end(), make_move_iterator(threadResult.trees.begin()),
                             make_move_iterator(threadResult.trees.end()));
            }
        }
    }
    fast_sort(trees, [](const auto &lhs, const auto &rhs) -> bool { return lhs.file < rhs.file; });
    return trees;
}

} // namespace

ast::ParsedFilesOrCancelled
Namer::runInternal(core::GlobalState &gs, vector<ast::ParsedFile> trees, WorkerPool &workers,
                   UnorderedMap<core::FileRef, core::FoundDefHashes> &&oldFoundHashesForFiles,
                   core::FoundDefHashes *foundHashesOut) {
    auto foundDefs = findSymbols(gs, move(trees), workers);
    if (gs.epochManager->wasTypecheckingCanceled()) {
        trees.reserve(foundDefs.size());
        for (auto &def : foundDefs) {
            trees.emplace_back(move(def.tree));
        }
        return ast::ParsedFilesOrCancelled::cancel(move(trees), workers);
    }
    if (foundHashesOut != nullptr) {
        ENFORCE(foundDefs.size() == 1,
                "Producing foundMethodHashes is meant to only happen when hashing a single file");
    }
    auto result = defineSymbols(gs, move(foundDefs), workers, std::move(oldFoundHashesForFiles), foundHashesOut);
    if (!result.hasResult()) {
        return result;
    }
    trees = symbolizeTrees(gs, move(result.result()), workers);
    return trees;
}

ast::ParsedFilesOrCancelled Namer::run(core::GlobalState &gs, vector<ast::ParsedFile> trees, WorkerPool &workers,
                                       core::FoundDefHashes *foundHashesOut) {
    // In non-incremental namer, there are no old FoundDefHashes; just defineSymbols like normal.
    auto oldFoundHashesForFiles = UnorderedMap<core::FileRef, core::FoundDefHashes>{};
    return runInternal(gs, move(trees), workers, std::move(oldFoundHashesForFiles), foundHashesOut);
}

ast::ParsedFilesOrCancelled
Namer::runIncremental(core::GlobalState &gs, std::vector<ast::ParsedFile> trees,
                      UnorderedMap<core::FileRef, core::FoundDefHashes> &&oldFoundHashesForFiles, WorkerPool &workers) {
    // foundHashesOut is only used when namer is run via hashing.cc to compute a FileHash for each file
    // The incremental namer mode should never be used for hashing.
    auto foundHashesOut = nullptr;
    return runInternal(gs, move(trees), workers, std::move(oldFoundHashesForFiles), foundHashesOut);
}

}; // namespace sorbet::namer
