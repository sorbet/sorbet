#include "namer/namer.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "ast/ArgParsing.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "class_flatten/class_flatten.h"
#include "common/Timer.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/concurrency/WorkerPool.h"
#include "common/sort.h"
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

    core::FoundDefinitionRef getOwner() {
        if (ownerStack.empty()) {
            return core::FoundDefinitionRef::root();
        }
        return ownerStack.back();
    }

    // Returns index to foundDefs containing the given name. Recursively inserts class refs for its owners.
    core::FoundDefinitionRef squashNames(core::Context ctx, const ast::ExpressionPtr &node) {
        if (auto *id = ast::cast_tree<ast::ConstantLit>(node)) {
            // Already defined. Insert a foundname so we can reference it.
            auto sym = id->symbol.dealias(ctx);
            ENFORCE(sym.exists());
            return foundDefs->addSymbol(sym);
        } else if (auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(node)) {
            core::FoundClassRef found;
            found.owner = squashNames(ctx, constLit->scope);
            found.name = constLit->cnst;
            found.loc = constLit->loc;
            return foundDefs->addClassRef(move(found));
        } else {
            // `class <<self`, `::Foo`, `self::Foo`
            // Return non-existent nameref as placeholder.
            return core::FoundDefinitionRef();
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
        found.owner = getOwner();
        found.classKind = klass.kind;
        found.loc = klass.loc;
        found.declLoc = klass.declLoc;

        auto *ident = ast::cast_tree<ast::UnresolvedIdent>(klass.name);
        if ((ident != nullptr) && ident->name == core::Names::singleton()) {
            core::FoundClassRef foundRef;
            foundRef.name = ident->name;
            foundRef.loc = ident->loc;
            found.klass = foundDefs->addClassRef(move(foundRef));
        } else {
            if (klass.symbol == core::Symbols::todo()) {
                found.klass = squashNames(ctx, klass.name);
            } else {
                // Desugar populates a top-level root() ClassDef.
                // Nothing else should have been typeAlias by now.
                ENFORCE(klass.symbol == core::Symbols::root());
                found.klass = foundDefs->addSymbol(klass.symbol);
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
        }
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
        foundDefs->addMethod(move(foundMethod));

        // After flatten, method defs have been hoisted and reordered, so instead we look for the
        // keep_def / keep_self_def calls, which will still be ordered correctly relative to
        // visibility modifiers.
    }

    void postTransformSend(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &original = ast::cast_tree_nonnull<ast::Send>(tree);

        switch (original.fun.rawId()) {
            case core::Names::privateClassMethod().rawId(): {
                for (auto &arg : original.posArgs()) {
                    addMethodModifier(ctx, original.fun, arg);
                }
                break;
            }
            case core::Names::packagePrivate().rawId():
            case core::Names::packagePrivateClassMethod().rawId():
            case core::Names::private_().rawId():
            case core::Names::protected_().rawId():
            case core::Names::public_().rawId():
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
                    for (auto &arg : original.posArgs()) {
                        addMethodModifier(ctx, original.fun, arg);
                    }
                }
                break;
            case core::Names::privateConstant().rawId(): {
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
        found.owner = getOwner();
        found.klass = squashNames(ctx, lhs.scope);
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
        found.isTypeTemplete = send->fun == core::Names::typeTemplate();

        if (send->numPosArgs() > 1) {
            // Too many arguments. Define a static field that we'll use for this type åmember later.
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

    void postTransformAssign(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);

        auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn.lhs);
        if (lhs == nullptr) {
            return;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn.rhs);
        if (send == nullptr) {
            fillAssign(ctx, asgn);
        } else if (!send->recv.isSelfReference()) {
            handleAssignment(ctx, asgn);
        } else {
            switch (send->fun.rawId()) {
                case core::Names::typeTemplate().rawId():
                    handleTypeMemberDefinition(ctx, send, asgn, lhs);
                    break;
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

/**
 * Defines symbols for all of the definitions found via SymbolFinder. Single threaded.
 */
class SymbolDefiner {
    const core::FoundDefinitions foundDefs;
    const optional<core::FoundMethodHashes> oldFoundMethodHashes;
    // See getOwnerSymbol
    vector<core::ClassOrModuleRef> definedClasses;
    // See getOwnerSymbol
    vector<core::MethodRef> definedMethods;

    // Returns a symbol to the referenced name. Name must be a class or module.
    // Prerequisite: Owner is a class or module.
    core::SymbolRef squashNames(core::MutableContext ctx, core::FoundDefinitionRef ref, core::ClassOrModuleRef owner) {
        switch (ref.kind()) {
            case core::FoundDefinitionRef::Kind::Empty:
                return owner;
            case core::FoundDefinitionRef::Kind::Symbol: {
                return ref.symbol();
            }
            case core::FoundDefinitionRef::Kind::ClassRef: {
                auto &klassRef = ref.klassRef(foundDefs);
                auto newOwner = squashNames(ctx, klassRef.owner, owner);
                return getOrDefineSymbol(ctx.withOwner(newOwner), klassRef.name, klassRef.loc);
            }
            default:
                Exception::raise("Invalid name reference");
        }
    }

    // Get the symbol for an already-defined owner. Limited to refs that can own things (classes and methods).
    core::SymbolRef getOwnerSymbol(core::FoundDefinitionRef ref) {
        switch (ref.kind()) {
            case core::FoundDefinitionRef::Kind::Symbol:
                return ref.symbol();
            case core::FoundDefinitionRef::Kind::Class:
                ENFORCE(ref.idx() < definedClasses.size());
                return definedClasses[ref.idx()];
            case core::FoundDefinitionRef::Kind::Method:
                ENFORCE(ref.idx() < definedMethods.size());
                return definedMethods[ref.idx()];
            default:
                Exception::raise("Invalid owner reference");
        }
    }

    // Allow stub symbols created to hold intrinsics to be filled in
    // with real types from code
    bool isIntrinsic(const core::MethodData &data) {
        return data->hasIntrinsic() && !data->hasSig();
    }

    void emitRedefinedConstantError(core::MutableContext ctx, core::Loc errorLoc, std::string constantName,
                                    core::Loc prevDefinitionLoc) {
        if (auto e = ctx.state.beginError(errorLoc, core::errors::Namer::ModuleKindRedefinition)) {
            e.setHeader("Redefining constant `{}`", constantName);
            e.addErrorLine(prevDefinitionLoc, "Previous definition");
        }
    }

    void emitRedefinedConstantError(core::MutableContext ctx, core::Loc errorLoc, core::SymbolRef symbol,
                                    core::SymbolRef renamedSymbol) {
        emitRedefinedConstantError(ctx, errorLoc, symbol.show(ctx), renamedSymbol.loc(ctx));
    }

    core::ClassOrModuleRef ensureIsClass(core::MutableContext ctx, core::SymbolRef scope, core::NameRef name,
                                         core::LocOffsets loc) {
        // Common case: Everything is fine, user is trying to define a symbol on a class or module.
        if (scope.isClassOrModule()) {
            // Check if original symbol was mangled away. If so, complain.
            auto renamedSymbol = ctx.state.findRenamedSymbol(scope.asClassOrModuleRef().data(ctx)->owner, scope);
            if (renamedSymbol.exists()) {
                if (auto e = ctx.beginError(loc, core::errors::Namer::InvalidClassOwner)) {
                    auto constLitName = name.show(ctx);
                    auto scopeName = scope.show(ctx);
                    e.setHeader("Can't nest `{}` under `{}` because `{}` is not a class or module", constLitName,
                                scopeName, scopeName);
                    e.addErrorLine(renamedSymbol.loc(ctx), "`{}` defined here", scopeName);
                }
            }
            return scope.asClassOrModuleRef();
        }

        // Check if class was already mangled.
        auto klassSymbol = ctx.state.lookupClassSymbol(scope.owner(ctx).asClassOrModuleRef(), scope.name(ctx));
        if (klassSymbol.exists()) {
            return klassSymbol;
        }

        if (auto e = ctx.beginError(loc, core::errors::Namer::InvalidClassOwner)) {
            auto constLitName = name.show(ctx);
            auto newOwnerName = scope.show(ctx);
            e.setHeader("Can't nest `{}` under `{}` because `{}` is not a class or module", constLitName, newOwnerName,
                        newOwnerName);
            e.addErrorLine(scope.loc(ctx), "`{}` defined here", newOwnerName);
        }
        // Mangle this one out of the way, and re-enter a symbol with this name as a class.
        auto scopeName = scope.name(ctx);
        ctx.state.mangleRenameSymbol(scope, scopeName);
        auto scopeKlass = ctx.state.enterClassSymbol(ctx.locAt(loc), scope.owner(ctx).asClassOrModuleRef(), scopeName);
        scopeKlass.data(ctx)->singletonClass(ctx); // force singleton class into existance
        return scopeKlass;
    }

    // Gets the symbol with the given name, or defines it as a class if it does not exist.
    core::SymbolRef getOrDefineSymbol(core::MutableContext ctx, core::NameRef name, core::LocOffsets loc) {
        if (name == core::Names::singleton()) {
            return ctx.owner.enclosingClass(ctx).data(ctx)->singletonClass(ctx);
        }

        auto scope = ensureIsClass(ctx, ctx.owner, name, loc);
        core::SymbolRef existing = scope.data(ctx)->findMember(ctx, name);
        if (!existing.exists()) {
            existing = ctx.state.enterClassSymbol(ctx.locAt(loc), scope, name);
            existing.asClassOrModuleRef().data(ctx)->singletonClass(ctx); // force singleton class into existance
        }

        return existing;
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
                    ctx.state.mangleRenameSymbol(sym, method.name);
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
                auto replacedSym = ctx.state.findRenamedSymbol(owner, matchingSym);
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
            // the fast path with an existing GlobalState.
            // When modifyMethod is called later, it won't be able to find the correct method entry.
            // Let's leave the method visibility what it was.
            // TODO(jez) After #5808 lands, can we delete this check?
            return symbol;
        }

        auto implicitlyPrivate = ctx.owner.enclosingClass(ctx) == core::Symbols::root();
        if (implicitlyPrivate) {
            // Methods defined at the top level default to private (on Object)
            symbol.data(ctx)->flags.isPrivate = true;
        } else {
            // All other methods default to public (their visibility might be changed later)
            symbol.data(ctx)->setMethodPublic();
        }
        return symbol;
    }

    void modifyMethod(core::MutableContext ctx, const core::FoundModifier &mod) {
        ENFORCE(mod.kind == core::FoundModifier::Kind::Method);

        auto owner = ctx.owner.enclosingClass(ctx);
        if (mod.name == core::Names::privateClassMethod() || mod.name == core::Names::packagePrivateClassMethod()) {
            owner = owner.data(ctx)->singletonClass(ctx);
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

    core::ClassOrModuleRef getClassSymbol(core::MutableContext ctx, const core::FoundClass &klass) {
        core::SymbolRef symbol = squashNames(ctx, klass.klass, ctx.owner.enclosingClass(ctx));
        ENFORCE(symbol.exists());

        const bool isModule = klass.classKind == ast::ClassDef::Kind::Module;
        auto declLoc = ctx.locAt(klass.declLoc);
        if (!symbol.isClassOrModule()) {
            // we might have already mangled the class symbol, so see if we have a symbol that is a class already
            auto klassSymbol = ctx.state.lookupClassSymbol(symbol.owner(ctx).asClassOrModuleRef(), symbol.name(ctx));
            if (klassSymbol.exists()) {
                return klassSymbol;
            }

            emitRedefinedConstantError(ctx, ctx.locAt(klass.loc), symbol.show(ctx), symbol.loc(ctx));

            auto origName = symbol.name(ctx);
            ctx.state.mangleRenameSymbol(symbol, symbol.name(ctx));
            klassSymbol = ctx.state.enterClassSymbol(declLoc, symbol.owner(ctx).asClassOrModuleRef(), origName);
            klassSymbol.data(ctx)->setIsModule(isModule);

            auto oldSymCount = ctx.state.classAndModulesUsed();
            auto newSingleton = klassSymbol.data(ctx)->singletonClass(ctx); // force singleton class into existence
            ENFORCE(newSingleton.id() >= oldSymCount,
                    "should be a fresh symbol. Otherwise we could be reusing an existing singletonClass");
            return klassSymbol;
        }

        auto klassSymbol = symbol.asClassOrModuleRef();
        if (klassSymbol.data(ctx)->isClassModuleSet() && isModule != klassSymbol.data(ctx)->isModule()) {
            if (auto e = ctx.state.beginError(declLoc, core::errors::Namer::ModuleKindRedefinition)) {
                e.setHeader("`{}` was previously defined as a `{}`", symbol.show(ctx),
                            klassSymbol.data(ctx)->isModule() ? "module" : "class");

                for (auto loc : klassSymbol.data(ctx)->locs()) {
                    if (loc != declLoc) {
                        e.addErrorLine(loc, "Previous definition");
                    }
                }
            }
        } else {
            klassSymbol.data(ctx)->setIsModule(isModule);
            auto renamed = ctx.state.findRenamedSymbol(klassSymbol.data(ctx)->owner, symbol);
            if (renamed.exists()) {
                emitRedefinedConstantError(ctx, ctx.locAt(klass.loc), symbol, renamed);
            }
        }
        return klassSymbol;
    }

    core::ClassOrModuleRef insertClass(core::MutableContext ctx, const core::FoundClass &klass) {
        auto symbol = getClassSymbol(ctx, klass);

        if (klass.classKind == ast::ClassDef::Kind::Class && !symbol.data(ctx)->superClass().exists() &&
            symbol != core::Symbols::BasicObject()) {
            symbol.data(ctx)->setSuperClass(core::Symbols::todo());
        }

        // In Ruby 2.5 they changed this class to have a different superclass
        // from 2.4. Since we don't have a good story around versioned ruby rbis
        // yet, lets just force the superclass regardless of version.
        if (symbol == core::Symbols::Net_IMAP()) {
            symbol.data(ctx)->setSuperClass(core::Symbols::Net_Protocol());
        }

        // Don't add locs for <root>; 1) they aren't useful and 2) they'll end up with O(files in
        // project) locs!
        if (symbol != core::Symbols::root()) {
            symbol.data(ctx)->addLoc(ctx, ctx.locAt(klass.declLoc));
        }
        symbol.data(ctx)->singletonClass(ctx); // force singleton class into existence

        // make sure we've added a static init symbol so we have it ready for the flatten pass later
        if (symbol == core::Symbols::root()) {
            ctx.state.staticInitForFile(ctx.locAt(klass.loc));
        } else {
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

    core::FieldRef insertStaticField(core::MutableContext ctx, const core::FoundStaticField &staticField) {
        ENFORCE(ctx.owner.isClassOrModule());

        auto scope = ensureIsClass(ctx, squashNames(ctx, staticField.klass, contextClass(ctx, ctx.owner)),
                                   staticField.name, staticField.asgnLoc);
        auto sym = ctx.state.lookupStaticFieldSymbol(scope, staticField.name);
        auto currSym = ctx.state.lookupSymbol(scope, staticField.name);
        auto name = sym.exists() ? sym.data(ctx)->name : staticField.name;
        if (!sym.exists() && currSym.exists()) {
            emitRedefinedConstantError(ctx, ctx.locAt(staticField.asgnLoc), staticField.name.show(ctx),
                                       currSym.loc(ctx));
            ctx.state.mangleRenameSymbol(currSym, currSym.name(ctx));
        }
        if (sym.exists()) {
            ENFORCE(currSym.exists());
            auto renamedSym = ctx.state.findRenamedSymbol(scope, sym);
            if (renamedSym.exists()) {
                emitRedefinedConstantError(ctx, ctx.locAt(staticField.asgnLoc), renamedSym.name(ctx).show(ctx),
                                           renamedSym.loc(ctx));
            }
        }
        sym = ctx.state.enterStaticFieldSymbol(ctx.locAt(staticField.lhsLoc), scope, name);

        if (staticField.isTypeAlias) {
            sym.data(ctx)->flags.isStaticFieldTypeAlias = true;
        }

        return sym;
    }

    core::SymbolRef insertTypeMember(core::MutableContext ctx, const core::FoundTypeMember &typeMember) {
        if (ctx.owner == core::Symbols::root()) {
            core::FoundStaticField staticField;
            staticField.owner = typeMember.owner;
            staticField.name = typeMember.name;
            staticField.asgnLoc = typeMember.asgnLoc;
            staticField.lhsLoc = typeMember.asgnLoc;
            staticField.isTypeAlias = true;
            return insertStaticField(ctx, staticField);
        }

        core::Variance variance = core::Variance::Invariant;
        const bool isTypeTemplate = typeMember.isTypeTemplete;

        auto onSymbol = isTypeTemplate ? ctx.owner.asClassOrModuleRef().data(ctx)->singletonClass(ctx)
                                       : ctx.owner.asClassOrModuleRef();

        core::NameRef foundVariance = typeMember.varianceName;
        if (foundVariance.exists()) {
            if (foundVariance == core::Names::covariant()) {
                variance = core::Variance::CoVariant;
            } else if (foundVariance == core::Names::contravariant()) {
                variance = core::Variance::ContraVariant;
            } else if (foundVariance == core::Names::invariant()) {
                variance = core::Variance::Invariant;
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
            auto oldSym = ctx.state.findRenamedSymbol(onSymbol, existingTypeMember);
            if (oldSym.exists()) {
                emitRedefinedConstantError(ctx, ctx.locAt(typeMember.nameLoc), oldSym, existingTypeMember);
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
            auto oldSym = onSymbol.data(ctx)->findMemberNoDealias(ctx, typeMember.name);
            if (oldSym.exists()) {
                emitRedefinedConstantError(ctx, ctx.locAt(typeMember.nameLoc), oldSym.show(ctx), oldSym.loc(ctx));
                ctx.state.mangleRenameSymbol(oldSym, oldSym.name(ctx));
            }
            sym = ctx.state.enterTypeMember(ctx.locAt(typeMember.asgnLoc), onSymbol, typeMember.name, variance);

            // The todo bounds will be fixed by the resolver in ResolveTypeParamsWalk.
            auto todo = core::make_type<core::ClassType>(core::Symbols::todo());
            sym.data(ctx)->resultType = core::make_type<core::LambdaParam>(sym, todo, todo);

            if (isTypeTemplate) {
                auto context = ctx.owner.enclosingClass(ctx);
                oldSym = context.data(ctx)->findMemberNoDealias(ctx, typeMember.name);
                if (oldSym.exists() &&
                    !(oldSym.loc(ctx) == ctx.locAt(typeMember.asgnLoc) || oldSym.loc(ctx).isTombStoned(ctx))) {
                    emitRedefinedConstantError(ctx, ctx.locAt(typeMember.nameLoc), typeMember.name.show(ctx),
                                               oldSym.loc(ctx));
                    ctx.state.mangleRenameSymbol(oldSym, typeMember.name);
                }
                // This static field with an AliasType is how we get `MyTypeTemplate` to resolve,
                // because resolver does not usually look on the singleton class to resolve constant
                // literals, but type_template's are only ever entered on the singleton class.
                auto alias = ctx.state.enterStaticFieldSymbol(ctx.locAt(typeMember.asgnLoc), context, typeMember.name);
                alias.data(ctx)->resultType = core::make_type<core::AliasType>(core::SymbolRef(sym));
            }
        }

        if (typeMember.isFixed) {
            sym.data(ctx)->flags.isFixed = true;
        }

        return sym;
    }

    void defineNonMethodSingle(core::MutableContext ctx, core::FoundDefinitionRef ref) {
        switch (ref.kind()) {
            case core::FoundDefinitionRef::Kind::Class: {
                const auto &klass = ref.klass(foundDefs);
                ENFORCE(definedClasses.size() == ref.idx());
                definedClasses.emplace_back(insertClass(ctx.withOwner(getOwnerSymbol(klass.owner)), klass));
                break;
            }
            case core::FoundDefinitionRef::Kind::StaticField: {
                const auto &staticField = ref.staticField(foundDefs);
                insertStaticField(ctx.withOwner(getOwnerSymbol(staticField.owner)), staticField);
                break;
            }
            case core::FoundDefinitionRef::Kind::TypeMember: {
                const auto &typeMember = ref.typeMember(foundDefs);
                insertTypeMember(ctx.withOwner(getOwnerSymbol(typeMember.owner)), typeMember);
                break;
            }
            default:
                ENFORCE(false);
                break;
        }
    }

    // TODO(jez) This is a lot of GlobalState-looking code. Should we move it there?
    // TODO(jez) This no longer uses NameHash. Pick a different name?
    // TODO(jez) This only needs to run once per unique owner, but we run it once per found definition
    void deleteViaFullNameHash(core::MutableContext ctx, const core::FoundMethodHash &oldDefHash) {
        auto ownerRef = core::FoundDefinitionRef(core::FoundDefinitionRef::Kind::Class, oldDefHash.ownerIdx);
        ENFORCE(oldDefHash.nameHash.isDefined(), "Can't delete rename if old hash is not defined");

        // TODO(jez) I don't think we really need the arityHash anymore now that we're only looking
        // at the owner. Worth removing that? Or keep it in, on the chance we might need it later?
        // Honestly, we don't even need the nameHash at this rate, just the ownerIdx

        // Because a change to classes would have take the slow path, should be safe
        // to look up old owner in current foundDefs.
        auto ownerSymbol = getOwnerSymbol(ownerRef);
        ENFORCE(ownerSymbol.isClassOrModule());
        auto owner = methodOwner(ctx, ownerSymbol, oldDefHash.isSelfMethod);
        vector<core::MethodRef> toDelete;
        for (const auto &[memberName, memberSym] : owner.data(ctx)->members()) {
            if (!memberSym.isMethod()) {
                continue;
            }
            auto memberMethod = memberSym.asMethodRef();

            auto memberNameToHash = memberName;
            if (memberNameToHash.kind() == core::NameKind::UNIQUE) {
                auto &uniqueData = memberNameToHash.dataUnique(ctx);
                if (uniqueData->uniqueNameKind == core::UniqueNameKind::MangleRename ||
                    uniqueData->uniqueNameKind == core::UniqueNameKind::Overload) {
                    memberNameToHash = uniqueData->original;
                }
            }

            auto memberFullNameHash = core::FullNameHash(ctx, memberNameToHash);
            if (memberFullNameHash != oldDefHash.nameHash) {
                continue;
            }

            toDelete.emplace_back(memberMethod);
        }

        // TODO(jez) Find somewhere appropriate for this comment
        // Only delete if the thing we found had the hash we expected to find.
        // If we found something else, it likely means that even though there was previously a FoundMethod
        // that would have caused a method with this name to be defined, it was delete and a
        // different method with the same name took its place (that other method might not have been deleted,
        // but if it was, we'll get to delete it on some future iteration.)
        for (auto oldMethod : toDelete) {
            // TODO(jez) Double check that if you end up removing all references in all files on a
            // fast path, that the method does in fact get deleted.
            oldMethod.data(ctx)->removeLocsForFile(ctx.file);
            if (oldMethod.data(ctx)->locs().empty()) {
                ctx.state.deleteMethodSymbol(oldMethod);
            }
        }
    }

public:
    SymbolDefiner(unique_ptr<core::FoundDefinitions> foundDefs, optional<core::FoundMethodHashes> oldFoundMethodHashes)
        : foundDefs(move(*foundDefs)), oldFoundMethodHashes(move(oldFoundMethodHashes)) {}

    void run(core::MutableContext ctx) {
        definedClasses.reserve(foundDefs.klasses().size());
        definedMethods.reserve(foundDefs.methods().size());

        for (auto ref : foundDefs.nonMethodDefinitions()) {
            defineNonMethodSingle(ctx, ref);
        }

        // TODO(jez) This currently interleaves deleting and defining across files.
        // It's possible that this causes problems at some point? Though I haven't found a test case.
        // That being said, if it does cause problems, we should be able to not interleave, and have
        // all the `nonMethodDefinitions` from all files get defined, then delete all the old
        // methods, then define all the methods.
        if (oldFoundMethodHashes.has_value()) {
            for (const auto &oldMethodHash : oldFoundMethodHashes.value()) {
                // Since we've already processed all the non-method symbols (which includes classes), we now
                // guarantee that deleteViaFullNameHash can use getOwnerSymbol to lookup an old owner
                // ref in the new definedClasses vector.
                deleteViaFullNameHash(ctx, oldMethodHash);
            }
        }

        for (auto &method : foundDefs.methods()) {
            if (method.arityHash.isAliasMethod()) {
                // TODO(jez) Update this comment on the fast path namer branch
                // alias methods will be defined in resolver.
                continue;
            }
            definedMethods.emplace_back(insertMethod(ctx.withOwner(getOwnerSymbol(method.owner)), method));
        }

        for (const auto &modifier : foundDefs.modifiers()) {
            const auto owner = getOwnerSymbol(modifier.owner);
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

    void populateFoundMethodHashes(core::Context ctx, core::FoundMethodHashes &foundMethodHashesOut) {
        for (const auto &method : foundDefs.methods()) {
            auto owner = method.owner;
            auto fullNameHash = core::FullNameHash(ctx, method.name);
            foundMethodHashesOut.emplace_back(owner.idx(), fullNameHash, method.arityHash, method.flags.isSelfMethod);
        }
    }
};

using BehaviorLocs = InlinedVector<core::Loc, 1>;
using ClassBehaviorLocsMap = UnorderedMap<core::ClassOrModuleRef, BehaviorLocs>;

/**
 * Inserts newly created symbols (from SymbolDefiner) into a tree.
 */
class TreeSymbolizer {
    friend class Namer;

    bool bestEffort;

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
        if (!newOwner.exists()) {
            ENFORCE(this->bestEffort);
            return core::Symbols::noSymbol();
        }

        core::SymbolRef existing = ctx.state.lookupClassSymbol(newOwner.asClassOrModuleRef(), constLit->cnst);
        if (firstName && !existing.exists() && newOwner.isClassOrModule()) {
            existing = ctx.state.lookupStaticFieldSymbol(newOwner.asClassOrModuleRef(), constLit->cnst);
            if (existing.exists()) {
                existing = existing.dealias(ctx.state);
            }
        }

        // NameInserter should have created this symbol, unless we're running in bestEffort mode (stale GlobalState).
        if (!existing.exists()) {
            ENFORCE(this->bestEffort);
            return core::Symbols::noSymbol();
        }

        node = ast::make_expression<ast::ConstantLit>(constLit->loc, existing, std::move(node));
        return existing;
    }

    optional<core::SymbolRef> squashNames(core::Context ctx, core::SymbolRef owner, ast::ExpressionPtr &node) {
        const bool firstName = true;
        auto result = squashNamesInner(ctx, owner, node, firstName);

        // Explicitly convert to optional (unlike what we usually do with SymbolRef's), because
        // the bestEffort mode is subtle, and can easily break invariants that old code used to
        // assume, so hopefully the type system catches them more easily.
        return result.exists() ? make_optional<core::SymbolRef>(result) : nullopt;
    }

    ast::ExpressionPtr arg2Symbol(int pos, core::ParsedArg parsedArg, ast::ExpressionPtr arg) {
        ast::ExpressionPtr localExpr = ast::make_expression<ast::Local>(parsedArg.loc, parsedArg.local);
        if (parsedArg.flags.isDefault) {
            localExpr =
                ast::MK::OptionalArg(parsedArg.loc, move(localExpr), ast::ArgParsing::getDefault(parsedArg, move(arg)));
        }
        return localExpr;
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

public:
    TreeSymbolizer(bool bestEffort) : bestEffort(bestEffort) {}

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
                if (squashedSymbol.has_value()) {
                    klass.symbol = squashedSymbol.value().asClassOrModuleRef();
                } else {
                    ENFORCE(this->bestEffort);
                    // In bestEffort mode, we allow symbols to not exist because we attempted to run
                    // only the non-mutating namer passes (e.g., no SymbolDefiner). We'll delete
                    // this whole node once we get to postTransformClassDef, but in the mean time
                    // delete the RHS so we get there faster.
                    klass.rhs.clear();
                }
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

        if (klass.symbol == core::Symbols::todo()) {
            ENFORCE(this->bestEffort);
            // bestEffort mode runs with a constant GlobalState and does not first run SymbolDefiner
            // (useful for serving LSP queries quickly on stale data). This class wasn't given a
            // symbol in preTransformClassDef. Now that we're in postTransformClassDef, we're
            // allowed to return something that's not a ClassDef node, which we can use to delete
            // the tree.
            tree = ast::MK::EmptyTree();
            return;
        }

        // NameDefiner should have forced this class's singleton class into existence.
        ENFORCE(klass.symbol.data(ctx)->lookupSingletonClass(ctx).exists());

        auto allowMissing = true;
        if (!ctx.state.lookupStaticInitForClass(klass.symbol, allowMissing).exists()) {
            ENFORCE(this->bestEffort);
            // Even though we found a symbol for this, we don't have a <static-init> for it, which is
            // an invariant that namer must introduce (all ClassDef's have `<static-init>` methods).
            tree = ast::MK::EmptyTree();
            return;
        }

        for (auto &exp : klass.rhs) {
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
        auto loc = klass.declLoc;
        ast::InsSeq::STATS_store ideSeqs;
        if (ast::isa_tree<ast::ConstantLit>(klass.name)) {
            ideSeqs.emplace_back(ast::MK::KeepForIDE(loc.copyWithZeroLength(), klass.name.deepCopy()));
        }
        if (klass.kind == ast::ClassDef::Kind::Class && !klass.ancestors.empty() &&
            shouldLeaveAncestorForIDE(klass.ancestors.front())) {
            ideSeqs.emplace_back(ast::MK::KeepForIDE(loc.copyWithZeroLength(), klass.ancestors.front().deepCopy()));
        }

        if (klass.symbol != core::Symbols::root() && !ctx.file.data(ctx).isRBI() &&
            ast::BehaviorHelpers::checkClassDefinesBehavior(klass)) {
            // TODO(dmitry) This won't find errors in fast-incremental mode.
            auto &locs = classBehaviorLocs[klass.symbol];
            locs.emplace_back(ctx.locAt(klass.declLoc));
        }

        ast::InsSeq::STATS_store retSeqs;
        retSeqs.emplace_back(std::move(tree));
        for (auto &stat : ideSeqs) {
            retSeqs.emplace_back(std::move(stat));
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
                auto expr = arg2Symbol(i, move(arg), move(oldArgs[i]));
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
        if (!sym.exists()) {
            ENFORCE(this->bestEffort);
            // We're going to delete this tree when we get to the postTransformMethodDef.
            // Drop the RHS to make it get there faster.
            method.rhs = ast::MK::EmptyTree();
            return;
        }
        method.symbol = sym;
        method.args = fillInArgs(move(parsedArgs), std::move(method.args));
    }

    void postTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &method = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        if (method.symbol == core::Symbols::todoMethod() && this->bestEffort) {
            // See the similar code in postTransformClassDef for an explanation.
            tree = ast::MK::EmptyTree();
            return;
        }

        ENFORCE(method.args.size() == method.symbol.data(ctx)->arguments.size(), "{}: {} != {}",
                method.name.showRaw(ctx), method.args.size(), method.symbol.data(ctx)->arguments.size());
    }

    ast::ExpressionPtr handleAssignment(core::Context ctx, ast::ExpressionPtr tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);
        auto &lhs = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(asgn.lhs);

        auto maybeMaybeScope = squashNames(ctx, contextClass(ctx, ctx.owner), lhs.scope);
        if (!maybeMaybeScope.has_value()) {
            ENFORCE(this->bestEffort);
            // SymbolDefiner will define a name that squashNames will find in all cases except when
            // we're running in non-mutating mode (LSP) and thus couldn't define new symbols.
            // In that case, just delete this assignment.
            return ast::MK::EmptyTree();
        }
        auto maybeScope = maybeMaybeScope.value();

        if (!maybeScope.isClassOrModule()) {
            auto scopeName = maybeScope.name(ctx);
            maybeScope = ctx.state.lookupClassSymbol(maybeScope.owner(ctx).asClassOrModuleRef(), scopeName);
        }
        auto scope = maybeScope.asClassOrModuleRef();

        core::SymbolRef cnst = ctx.state.lookupStaticFieldSymbol(scope, lhs.cnst);
        if (!cnst.exists()) {
            ENFORCE(this->bestEffort);
            return ast::MK::EmptyTree();
        }
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
            edits.emplace_back(core::AutocorrectSuggestion::Edit{insertLoc, fmt::format(" {{{{{}}}}}", kwArgsSource)});
        }
        e.addAutocorrect(core::AutocorrectSuggestion{
            fmt::format("Convert `{}` to block form", send->fun.show(ctx)),
            move(edits),
        });
    }

    ast::ExpressionPtr handleTypeMemberDefinition(core::Context ctx, ast::Send *send, ast::ExpressionPtr tree,
                                                  const ast::UnresolvedConstantLit *typeName) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);

        ENFORCE(asgn.lhs.get() == typeName &&
                asgn.rhs.get() == send); // this method assumes that `asgn` owns `send` and `typeName`
        if (!ctx.owner.isClassOrModule()) {
            if (auto e = ctx.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                e.setHeader("Types must be defined in class or module scopes");
            }
            return ast::MK::EmptyTree();
        }
        if (ctx.owner == core::Symbols::root()) {
            if (auto e = ctx.beginError(send->loc, core::errors::Namer::RootTypeMember)) {
                e.setHeader("`{}` cannot be used at the top-level", "type_member");
            }
            auto send = ast::MK::Send0Block(asgn.loc, ast::MK::T(asgn.loc), core::Names::typeAlias(),
                                            asgn.loc.copyWithZeroLength(),
                                            ast::MK::Block0(asgn.loc, ast::MK::Untyped(asgn.loc)));

            return handleAssignment(ctx,
                                    ast::make_expression<ast::Assign>(asgn.loc, std::move(asgn.lhs), std::move(send)));
        }

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

            bool isTypeTemplate = send->fun == core::Names::typeTemplate();
            auto onSymbol =
                isTypeTemplate ? ctx.owner.asClassOrModuleRef().data(ctx)->lookupSingletonClass(ctx) : ctx.owner;
            if (!onSymbol.exists()) {
                ENFORCE(this->bestEffort);
                return ast::MK::EmptyTree();
            }
            core::SymbolRef sym = ctx.state.lookupTypeMemberSymbol(onSymbol.asClassOrModuleRef(), typeName->cnst);
            if (!sym.exists()) {
                ENFORCE(this->bestEffort);
                return ast::MK::EmptyTree();
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
            return;
        }

        if (!send->recv.isSelfReference()) {
            tree = handleAssignment(ctx, std::move(tree));
            return;
        }

        switch (send->fun.rawId()) {
            case core::Names::typeTemplate().rawId():
            case core::Names::typeMember().rawId(): {
                tree = handleTypeMemberDefinition(ctx, send, std::move(tree), lhs);
                return;
            }
            default: {
                tree = handleAssignment(ctx, std::move(tree));
                return;
            }
        }
    }

    ClassBehaviorLocsMap classBehaviorLocs;
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
                ast::ShallowWalk::apply(ctx, finder, job.tree);
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

ast::ParsedFilesOrCancelled
defineSymbols(core::GlobalState &gs, vector<SymbolFinderResult> allFoundDefinitions, WorkerPool &workers,
              UnorderedMap<core::FileRef, core::FoundMethodHashes> &&oldFoundMethodHashesForFiles,
              core::FoundMethodHashes *foundMethodHashesOut) {
    Timer timeit(gs.tracer(), "naming.defineSymbols");
    vector<ast::ParsedFile> output;
    output.reserve(allFoundDefinitions.size());
    const auto &epochManager = *gs.epochManager;
    uint32_t count = 0;
    uint32_t foundMethods = 0;
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
        auto oldFoundMethodHashes = oldFoundMethodHashesForFiles.find(fref) == oldFoundMethodHashesForFiles.end()
                                        ? optional<core::FoundMethodHashes>()
                                        : std::move(oldFoundMethodHashesForFiles[fref]);
        SymbolDefiner symbolDefiner(move(fileFoundDefinitions.names), move(oldFoundMethodHashes));
        output.emplace_back(move(fileFoundDefinitions.tree));
        symbolDefiner.run(ctx);
        if (foundMethodHashesOut != nullptr) {
            symbolDefiner.populateFoundMethodHashes(ctx, *foundMethodHashesOut);
        }
    }
    prodCounterAdd("types.input.foundmethods.total", foundMethods);
    return output;
}

struct SymbolizeTreesResult {
    vector<ast::ParsedFile> trees;
    ClassBehaviorLocsMap classBehaviorLocs;
};

void mergeClassBehaviorLocs(const core::GlobalState &gs, ClassBehaviorLocsMap &merged,
                            ClassBehaviorLocsMap &threadRes) {
    Timer timeit(gs.tracer(), "naming.symbolizeTreesMergeClass");
    if (merged.empty()) {
        swap(merged, threadRes);
        return;
    }
    for (auto [ref, threadLocs] : threadRes) {
        auto &mergedLocs = merged[ref];
        mergedLocs.insert(mergedLocs.end(), make_move_iterator(threadLocs.begin()),
                          make_move_iterator(threadLocs.end()));
    }
    threadRes.clear();
}

void findConflictingClassDefs(const core::GlobalState &gs, ClassBehaviorLocsMap &map) {
    vector<pair<core::ClassOrModuleRef, BehaviorLocs>> conflicts;
    for (auto &[ref, locs] : map) {
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
    map.clear();

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

vector<ast::ParsedFile> symbolizeTrees(const core::GlobalState &gs, vector<ast::ParsedFile> trees, WorkerPool &workers,
                                       bool bestEffort) {
    Timer timeit(gs.tracer(), "naming.symbolizeTrees");
    auto resultq = make_shared<BlockingBoundedQueue<SymbolizeTreesResult>>(trees.size());
    auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
    for (auto &tree : trees) {
        fileq->push(move(tree), 1);
    }

    workers.multiplexJob("symbolizeTrees", [&gs, fileq, resultq, bestEffort]() {
        Timer timeit(gs.tracer(), "naming.symbolizeTreesWorker");
        TreeSymbolizer inserter(bestEffort);
        SymbolizeTreesResult output;
        ast::ParsedFile job;
        for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
            if (result.gotItem()) {
                Timer timeit(gs.tracer(), "naming.symbolizeTreesOne", {{"file", string(job.file.data(gs).path())}});
                core::Context ctx(gs, core::Symbols::root(), job.file);
                ast::ShallowWalk::apply(ctx, inserter, job.tree);
                output.trees.emplace_back(move(job));
            }
        }
        if (!output.trees.empty()) {
            output.classBehaviorLocs = std::move(inserter.classBehaviorLocs);
            resultq->push(move(output), output.trees.size());
        }
    });
    trees.clear();

    {
        ClassBehaviorLocsMap classBehaviorLocs;
        SymbolizeTreesResult threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem()) {
                trees.insert(trees.end(), make_move_iterator(threadResult.trees.begin()),
                             make_move_iterator(threadResult.trees.end()));
                mergeClassBehaviorLocs(gs, classBehaviorLocs, threadResult.classBehaviorLocs);
            }
        }
        findConflictingClassDefs(gs, classBehaviorLocs);
    }
    fast_sort(trees, [](const auto &lhs, const auto &rhs) -> bool { return lhs.file < rhs.file; });
    return trees;
}

} // namespace

// Designed to be "run as much of namer as is possible with a const GlobalState".
//
// The whole point of `defineSymbols` is to mutate GlobalState, so it clearly makes no sense to run.
// And the whole point of `findSymbols` is to create the FoundDefinition vector that feeds into
// defineSymbols, so that becomes not useful to run either.
//
// That leaves just symbolizeTrees.
//
// The "best effort" indicates that this is currently only used for the sake of serving LSP requests
// with a potentially-stale GlobalState.
ast::ParsedFilesOrCancelled Namer::symbolizeTreesBestEffort(const core::GlobalState &gs, vector<ast::ParsedFile> trees,
                                                            WorkerPool &workers) {
    auto bestEffort = true;
    return symbolizeTrees(gs, move(trees), workers, bestEffort);
}

ast::ParsedFilesOrCancelled Namer::run(core::GlobalState &gs, vector<ast::ParsedFile> trees, WorkerPool &workers,
                                       core::FoundMethodHashes *foundMethodHashesOut) {
    auto foundDefs = findSymbols(gs, move(trees), workers);
    if (gs.epochManager->wasTypecheckingCanceled()) {
        trees.reserve(foundDefs.size());
        for (auto &def : foundDefs) {
            trees.emplace_back(move(def.tree));
        }
        return ast::ParsedFilesOrCancelled::cancel(move(trees), workers);
    }
    if (foundMethodHashesOut != nullptr) {
        ENFORCE(foundDefs.size() == 1,
                "Producing foundMethodHashes is meant to only happen when hashing a single file");
    }
    // There were no old FoundMethodHashes; just defineSymbols like normal.
    auto oldFoundMethodHashesForFiles = UnorderedMap<core::FileRef, core::FoundMethodHashes>{};
    auto result =
        defineSymbols(gs, move(foundDefs), workers, std::move(oldFoundMethodHashesForFiles), foundMethodHashesOut);
    if (!result.hasResult()) {
        return result;
    }
    auto bestEffort = false;
    trees = symbolizeTrees(gs, move(result.result()), workers, bestEffort);
    return trees;
}

ast::ParsedFilesOrCancelled
Namer::runIncremental(core::GlobalState &gs, std::vector<ast::ParsedFile> trees,
                      UnorderedMap<core::FileRef, core::FoundMethodHashes> &&oldFoundMethodHashesForFiles,
                      WorkerPool &workers) {
    // TODO(jez) Clever way to de-dup this with Namer::run ?
    auto foundDefs = findSymbols(gs, move(trees), workers);
    if (gs.epochManager->wasTypecheckingCanceled()) {
        trees.reserve(foundDefs.size());
        for (auto &def : foundDefs) {
            trees.emplace_back(move(def.tree));
        }
        return ast::ParsedFilesOrCancelled::cancel(move(trees), workers);
    }

    // We should never be combining Namer::runIncremental with the namer call that produces FileHashes
    auto foundMethodHashesOut = nullptr;
    auto result =
        defineSymbols(gs, move(foundDefs), workers, std::move(oldFoundMethodHashesForFiles), foundMethodHashesOut);
    if (!result.hasResult()) {
        return result;
    }
    auto bestEffort = false;
    trees = symbolizeTrees(gs, move(result.result()), workers, bestEffort);
    return trees;
}

}; // namespace sorbet::namer
