#include "namer/namer.h"
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
#include "common/typecase.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/core.h"
#include "core/errors/namer.h"
#include "core/lsp/TypecheckEpochManager.h"

using namespace std;

namespace sorbet::namer {

namespace {
class FoundDefinitions;

struct FoundClassRef;
struct FoundClass;
struct FoundStaticField;
struct FoundTypeMember;
struct FoundMethod;

enum class DefinitionKind : u1 {
    Empty = 0,
    Class = 1,
    ClassRef = 2,
    Method = 3,
    StaticField = 4,
    TypeMember = 5,
    Symbol = 6,
};

class FoundDefinitionRef final {
    DefinitionKind _kind;
    u4 _id;

public:
    FoundDefinitionRef(DefinitionKind kind, u4 idx) : _kind(kind), _id(idx) {}
    FoundDefinitionRef() : FoundDefinitionRef(DefinitionKind::Empty, 0) {}
    FoundDefinitionRef(const FoundDefinitionRef &nm) = default;
    FoundDefinitionRef(FoundDefinitionRef &&nm) = default;
    FoundDefinitionRef &operator=(const FoundDefinitionRef &rhs) = default;

    static FoundDefinitionRef root() {
        return FoundDefinitionRef(DefinitionKind::Symbol, core::Symbols::root()._id);
    }

    DefinitionKind kind() const {
        return _kind;
    }

    bool exists() const {
        return _id > 0;
    }

    u4 idx() const {
        return _id;
    }

    FoundClassRef &klassRef(FoundDefinitions &foundDefs);
    const FoundClassRef &klassRef(const FoundDefinitions &foundDefs) const;

    FoundClass &klass(FoundDefinitions &foundDefs);
    const FoundClass &klass(const FoundDefinitions &foundDefs) const;

    FoundMethod &method(FoundDefinitions &foundDefs);
    const FoundMethod &method(const FoundDefinitions &foundDefs) const;

    FoundStaticField &staticField(FoundDefinitions &foundDefs);
    const FoundStaticField &staticField(const FoundDefinitions &foundDefs) const;

    FoundTypeMember &typeMember(FoundDefinitions &foundDefs);
    const FoundTypeMember &typeMember(const FoundDefinitions &foundDefs) const;

    core::SymbolRef symbol() const;
};

struct FoundClassRef final {
    core::NameRef name;
    core::LocOffsets loc;
    // If !owner.exists(), owner is determined by reference site.
    FoundDefinitionRef owner;
};

struct FoundClass final {
    FoundDefinitionRef owner;
    FoundDefinitionRef klass;
    core::LocOffsets loc;
    core::Loc declLoc;
    ast::ClassDef::Kind classKind;
};

struct FoundStaticField final {
    FoundDefinitionRef owner;
    FoundDefinitionRef klass;
    core::NameRef name;
    core::LocOffsets asgnLoc;
    core::LocOffsets lhsLoc;
    bool isTypeAlias = false;
};

struct FoundTypeMember final {
    FoundDefinitionRef owner;
    core::NameRef name;
    core::LocOffsets asgnLoc;
    core::LocOffsets nameLoc;
    core::LocOffsets litLoc;
    core::NameRef varianceName;
    bool isFixed = false;
    bool isTypeTemplete = false;
};

struct FoundMethod final {
    FoundDefinitionRef owner;
    core::NameRef name;
    core::LocOffsets loc;
    core::Loc declLoc;
    ast::MethodDef::Flags flags;
    vector<ast::ParsedArg> parsedArgs;
    vector<u4> argsHash;
};

struct Modifier {
    DefinitionKind kind;
    FoundDefinitionRef owner;
    core::LocOffsets loc;
    // The name of the modification.
    core::NameRef name;
    // For methods: The name of the method being modified.
    core::NameRef methodName;
};

class FoundDefinitions final {
    // Contains references to items in _klasses, _methods, _staticFields, and _typeMembers.
    // Used to determine the order in which symbols are defined in SymbolDefiner.
    vector<FoundDefinitionRef> _definitions;
    // Contains references to classes in general. Separate from `FoundClass` because we sometimes need to define class
    // Symbols for classes that are referenced from but not present in the given file.
    vector<FoundClassRef> _klassRefs;
    // Contains all classes defined in the file.
    vector<FoundClass> _klasses;
    // Contains all methods defined in the file.
    vector<FoundMethod> _methods;
    // Contains all static fields defined in the file.
    vector<FoundStaticField> _staticFields;
    // Contains all type members defined in the file.
    vector<FoundTypeMember> _typeMembers;
    // Contains all method and class modifiers (e.g. private/public/protected).
    vector<Modifier> _modifiers;

    FoundDefinitionRef addDefinition(FoundDefinitionRef ref) {
        _definitions.emplace_back(ref);
        return ref;
    }

public:
    FoundDefinitions() = default;
    FoundDefinitions(FoundDefinitions &&names) = default;
    FoundDefinitions(const FoundDefinitions &names) = delete;
    ~FoundDefinitions() = default;

    FoundDefinitionRef addClass(FoundClass &&klass) {
        const u4 idx = _klasses.size();
        _klasses.emplace_back(move(klass));
        return addDefinition(FoundDefinitionRef(DefinitionKind::Class, idx));
    }

    FoundDefinitionRef addClassRef(FoundClassRef &&klassRef) {
        const u4 idx = _klassRefs.size();
        _klassRefs.emplace_back(move(klassRef));
        return FoundDefinitionRef(DefinitionKind::ClassRef, idx);
    }

    FoundDefinitionRef addMethod(FoundMethod &&method) {
        const u4 idx = _methods.size();
        _methods.emplace_back(move(method));
        return addDefinition(FoundDefinitionRef(DefinitionKind::Method, idx));
    }

    FoundDefinitionRef addStaticField(FoundStaticField &&staticField) {
        const u4 idx = _staticFields.size();
        _staticFields.emplace_back(move(staticField));
        return addDefinition(FoundDefinitionRef(DefinitionKind::StaticField, idx));
    }

    FoundDefinitionRef addTypeMember(FoundTypeMember &&typeMember) {
        const u4 idx = _typeMembers.size();
        _typeMembers.emplace_back(move(typeMember));
        return addDefinition(FoundDefinitionRef(DefinitionKind::TypeMember, idx));
    }

    FoundDefinitionRef addSymbol(core::SymbolRef symbol) {
        return FoundDefinitionRef(DefinitionKind::Symbol, symbol._id);
    }

    void addModifier(Modifier &&mod) {
        _modifiers.emplace_back(move(mod));
    }

    const vector<FoundDefinitionRef> &definitions() const {
        return _definitions;
    }

    const vector<FoundClass> &klasses() const {
        return _klasses;
    }

    const vector<FoundMethod> &methods() const {
        return _methods;
    }

    const vector<Modifier> &modifiers() const {
        return _modifiers;
    }

    friend FoundDefinitionRef;
};

FoundClassRef &FoundDefinitionRef::klassRef(FoundDefinitions &foundDefs) {
    ENFORCE(kind() == DefinitionKind::ClassRef);
    ENFORCE(foundDefs._klassRefs.size() > idx());
    return foundDefs._klassRefs[idx()];
}
const FoundClassRef &FoundDefinitionRef::klassRef(const FoundDefinitions &foundDefs) const {
    ENFORCE(kind() == DefinitionKind::ClassRef);
    ENFORCE(foundDefs._klassRefs.size() > idx());
    return foundDefs._klassRefs[idx()];
}

FoundClass &FoundDefinitionRef::klass(FoundDefinitions &foundDefs) {
    ENFORCE(kind() == DefinitionKind::Class);
    ENFORCE(foundDefs._klasses.size() > idx());
    return foundDefs._klasses[idx()];
}
const FoundClass &FoundDefinitionRef::klass(const FoundDefinitions &foundDefs) const {
    ENFORCE(kind() == DefinitionKind::Class);
    ENFORCE(foundDefs._klasses.size() > idx());
    return foundDefs._klasses[idx()];
}

FoundMethod &FoundDefinitionRef::method(FoundDefinitions &foundDefs) {
    ENFORCE(kind() == DefinitionKind::Method);
    ENFORCE(foundDefs._methods.size() > idx());
    return foundDefs._methods[idx()];
}
const FoundMethod &FoundDefinitionRef::method(const FoundDefinitions &foundDefs) const {
    ENFORCE(kind() == DefinitionKind::Method);
    ENFORCE(foundDefs._methods.size() > idx());
    return foundDefs._methods[idx()];
}

FoundStaticField &FoundDefinitionRef::staticField(FoundDefinitions &foundDefs) {
    ENFORCE(kind() == DefinitionKind::StaticField);
    ENFORCE(foundDefs._staticFields.size() > idx());
    return foundDefs._staticFields[idx()];
}
const FoundStaticField &FoundDefinitionRef::staticField(const FoundDefinitions &foundDefs) const {
    ENFORCE(kind() == DefinitionKind::StaticField);
    ENFORCE(foundDefs._staticFields.size() > idx());
    return foundDefs._staticFields[idx()];
}

FoundTypeMember &FoundDefinitionRef::typeMember(FoundDefinitions &foundDefs) {
    ENFORCE(kind() == DefinitionKind::TypeMember);
    ENFORCE(foundDefs._typeMembers.size() > idx());
    return foundDefs._typeMembers[idx()];
}
const FoundTypeMember &FoundDefinitionRef::typeMember(const FoundDefinitions &foundDefs) const {
    ENFORCE(kind() == DefinitionKind::TypeMember);
    ENFORCE(foundDefs._typeMembers.size() > idx());
    return foundDefs._typeMembers[idx()];
}

core::SymbolRef FoundDefinitionRef::symbol() const {
    ENFORCE(kind() == DefinitionKind::Symbol);
    return core::SymbolRef(nullptr, _id);
}

struct SymbolFinderResult {
    ast::ParsedFile tree;
    unique_ptr<FoundDefinitions> names;
};

core::SymbolRef methodOwner(core::Context ctx, const ast::MethodDef::Flags &flags) {
    core::SymbolRef owner = ctx.owner.data(ctx)->enclosingClass(ctx);
    if (owner == core::Symbols::root()) {
        // Root methods end up going on object
        owner = core::Symbols::Object();
    }

    if (flags.isSelfMethod) {
        if (owner.data(ctx)->isClassOrModule()) {
            owner = owner.data(ctx)->lookupSingletonClass(ctx);
        }
    }
    ENFORCE(owner.exists());
    ENFORCE(owner.data(ctx)->isClassOrModule());
    return owner;
}

// Returns the SymbolRef corresponding to the class `self.class`, unless the
// context is a class, in which case return it.
core::SymbolRef contextClass(const core::GlobalState &gs, core::SymbolRef ofWhat) {
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

/**
 * Used with TreeMap to locate all of the class, method, static field, and type member symbols defined in the tree.
 * Does not mutate GlobalState, which allows us to parallelize this process.
 * Does not report any errors, which lets us cache its output.
 * Produces a vector of symbols to insert, and a vector of modifiers to those symbols.
 */
class SymbolFinder {
    unique_ptr<FoundDefinitions> foundDefs = make_unique<FoundDefinitions>();
    // The tree doesn't have symbols yet, so `ctx.owner`, which is a SymbolRef, is meaningless.
    // Instead, we track the owner manually via FoundDefinitionRefs.
    vector<FoundDefinitionRef> ownerStack;

    void findClassModifiers(core::Context ctx, FoundDefinitionRef klass, ast::TreePtr &line) {
        auto *send = ast::cast_tree<ast::Send>(line);
        if (send == nullptr) {
            return;
        }

        switch (send->fun._id) {
            case core::Names::declareFinal()._id:
            case core::Names::declareSealed()._id:
            case core::Names::declareInterface()._id:
            case core::Names::declareAbstract()._id: {
                Modifier mod;
                mod.kind = DefinitionKind::Class;
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

    FoundDefinitionRef getOwner() {
        if (ownerStack.empty()) {
            return FoundDefinitionRef::root();
        }
        return ownerStack.back();
    }

    // Returns index to foundDefs containing the given name. Recursively inserts class refs for its owners.
    FoundDefinitionRef squashNames(core::Context ctx, const ast::TreePtr &node) {
        if (auto *id = ast::cast_tree_const<ast::ConstantLit>(node)) {
            // Already defined. Insert a foundname so we can reference it.
            auto sym = id->symbol.data(ctx)->dealias(ctx);
            ENFORCE(sym.exists());
            return foundDefs->addSymbol(sym);
        } else if (auto constLit = ast::cast_tree_const<ast::UnresolvedConstantLit>(node)) {
            FoundClassRef found;
            found.owner = squashNames(ctx, constLit->scope);
            found.name = constLit->cnst;
            found.loc = constLit->loc;
            return foundDefs->addClassRef(move(found));
        } else {
            // `class <<self`, `::Foo`, `self::Foo`
            // Return non-existent nameref as placeholder.
            return FoundDefinitionRef();
        }
    }

public:
    unique_ptr<FoundDefinitions> getAndClearFoundDefinitions() {
        ownerStack.clear();
        auto rv = move(foundDefs);
        foundDefs = make_unique<FoundDefinitions>();
        return rv;
    }

    ast::TreePtr preTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto &klass = ast::ref_tree<ast::ClassDef>(tree);

        FoundClass found;
        found.owner = getOwner();
        found.classKind = klass.kind;
        found.loc = klass.loc;
        found.declLoc = klass.declLoc;

        auto *ident = ast::cast_tree<ast::UnresolvedIdent>(klass.name);
        if ((ident != nullptr) && ident->name == core::Names::singleton()) {
            FoundClassRef foundRef;
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
        return tree;
    }

    ast::TreePtr postTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto &klass = ast::ref_tree<ast::ClassDef>(tree);

        FoundDefinitionRef klassName = ownerStack.back();
        ownerStack.pop_back();

        for (auto &exp : klass.rhs) {
            findClassModifiers(ctx, klassName, exp);
        }

        return tree;
    }

    ast::TreePtr preTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        auto &method = ast::ref_tree<ast::MethodDef>(tree);
        FoundMethod foundMethod;
        foundMethod.owner = getOwner();
        foundMethod.name = method.name;
        foundMethod.loc = method.loc;
        foundMethod.declLoc = method.declLoc;
        foundMethod.flags = method.flags;
        foundMethod.parsedArgs = ast::ArgParsing::parseArgs(method.args);
        foundMethod.argsHash = ast::ArgParsing::hashArgs(ctx, foundMethod.parsedArgs);
        ownerStack.emplace_back(foundDefs->addMethod(move(foundMethod)));
        return tree;
    }

    ast::TreePtr postTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        ownerStack.pop_back();
        return tree;
    }

    ast::TreePtr postTransformSend(core::Context ctx, ast::TreePtr tree) {
        auto &original = ast::ref_tree<ast::Send>(tree);

        if (original.args.size() == 1) {
            // Common case: Send is not to a modifier.
            switch (original.fun._id) {
                case core::Names::private_()._id:
                case core::Names::privateClassMethod()._id:
                case core::Names::protected_()._id:
                case core::Names::public_()._id:
                    break;
                default:
                    return tree;
            }

            Modifier methodModifier;
            methodModifier.kind = DefinitionKind::Method;
            methodModifier.owner = getOwner();
            methodModifier.loc = original.loc;
            methodModifier.name = original.fun;
            methodModifier.methodName = unwrapLiteralToMethodName(ctx, original, original.args[0]);
            if (methodModifier.methodName.exists()) {
                foundDefs->addModifier(move(methodModifier));
            }
        }
        return tree;
    }

    core::NameRef unwrapLiteralToMethodName(core::Context ctx, const ast::Send &original, ast::TreePtr &expr) {
        if (auto sym = ast::cast_tree<ast::Literal>(expr)) {
            // this handles the `private :foo` case
            if (!sym->isSymbol(ctx)) {
                return core::NameRef::noName();
            }
            return sym->asSymbol(ctx);
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

            if (send->args.size() != 2) {
                return core::NameRef::noName();
            }

            return unwrapLiteralToMethodName(ctx, original, send->args[1]);
        } else {
            ENFORCE(!ast::isa_tree<ast::MethodDef>(expr), "methods inside sends should be gone");
            return core::NameRef::noName();
        }
    }

    FoundDefinitionRef fillAssign(core::Context ctx, const ast::Assign &asgn) {
        auto &lhs = ast::ref_tree_const<ast::UnresolvedConstantLit>(asgn.lhs);

        FoundStaticField found;
        found.owner = getOwner();
        found.klass = squashNames(ctx, lhs.scope);
        found.name = lhs.cnst;
        found.asgnLoc = asgn.loc;
        found.lhsLoc = lhs.loc;
        return foundDefs->addStaticField(move(found));
    }

    FoundDefinitionRef handleTypeMemberDefinition(core::Context ctx, const ast::Send *send, const ast::Assign &asgn,
                                                  const ast::UnresolvedConstantLit *typeName) {
        ENFORCE(ast::cast_tree_const<ast::UnresolvedConstantLit>(asgn.lhs) == typeName &&
                ast::cast_tree_const<ast::Send>(asgn.rhs) ==
                    send); // this method assumes that `asgn` owns `send` and `typeName`

        FoundTypeMember found;
        found.owner = getOwner();
        found.asgnLoc = asgn.loc;
        found.nameLoc = typeName->loc;
        found.name = typeName->cnst;
        // Store name rather than core::Variance type so that we can defer reporting an error until later.
        found.varianceName = core::NameRef();
        found.isTypeTemplete = send->fun == core::Names::typeTemplate();

        if (send->args.size() > 2) {
            // Too many arguments. Define a static field that we'll use for this type åmember later.
            FoundStaticField staticField;
            staticField.owner = found.owner;
            staticField.name = found.name;
            staticField.asgnLoc = found.asgnLoc;
            staticField.lhsLoc = asgn.lhs->loc;
            staticField.isTypeAlias = true;
            return foundDefs->addStaticField(move(staticField));
        }

        if (!send->args.empty()) {
            auto *lit = ast::cast_tree_const<ast::Literal>(send->args[0]);
            if (lit != nullptr && lit->isSymbol(ctx)) {
                found.varianceName = lit->asSymbol(ctx);
                found.litLoc = lit->loc;
            }

            if (auto *hash = ast::cast_tree_const<ast::Hash>(send->args.back())) {
                for (auto &keyExpr : hash->keys) {
                    auto key = ast::cast_tree_const<ast::Literal>(keyExpr);
                    core::NameRef name;
                    if (key != nullptr && key->isSymbol(ctx)) {
                        switch (key->asSymbol(ctx)._id) {
                            case core::Names::fixed()._id:
                                found.isFixed = true;
                                break;
                        }
                    }
                }
            }
        }
        return foundDefs->addTypeMember(move(found));
    }

    FoundDefinitionRef handleAssignment(core::Context ctx, const ast::Assign &asgn) {
        auto &send = ast::ref_tree_const<ast::Send>(asgn.rhs);
        auto foundRef = fillAssign(ctx, asgn);
        ENFORCE(foundRef.kind() == DefinitionKind::StaticField);
        auto &staticField = foundRef.staticField(*foundDefs);
        staticField.isTypeAlias = send.fun == core::Names::typeAlias();
        return foundRef;
    }

    ast::TreePtr postTransformAssign(core::Context ctx, ast::TreePtr tree) {
        auto &asgn = ast::ref_tree<ast::Assign>(tree);

        auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn.lhs);
        if (lhs == nullptr) {
            return tree;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn.rhs);
        if (send == nullptr) {
            fillAssign(ctx, asgn);
        } else if (!send->recv->isSelfReference()) {
            handleAssignment(ctx, asgn);
        } else {
            switch (send->fun._id) {
                case core::Names::typeTemplate()._id:
                    handleTypeMemberDefinition(ctx, send, asgn, lhs);
                    break;
                case core::Names::typeMember()._id:
                    handleTypeMemberDefinition(ctx, send, asgn, lhs);
                    break;
                default:
                    fillAssign(ctx, asgn);
                    break;
            }
        }
        return tree;
    }
};

/**
 * Defines symbols for all of the definitions found via SymbolFinder. Single threaded.
 */
class SymbolDefiner {
    unique_ptr<const FoundDefinitions> foundDefs;
    vector<core::SymbolRef> definedClasses;
    vector<core::SymbolRef> definedMethods;

    // Returns a symbol to the referenced name. Name must be a class or module.
    core::SymbolRef squashNames(core::MutableContext ctx, FoundDefinitionRef ref, core::SymbolRef owner) {
        // Prerequisite: Owner is a class or module.
        ENFORCE(owner.data(ctx)->isClassOrModule());
        switch (ref.kind()) {
            case DefinitionKind::Empty:
                return owner;
            case DefinitionKind::Symbol: {
                return ref.symbol();
            }
            case DefinitionKind::ClassRef: {
                auto &klassRef = ref.klassRef(*foundDefs);
                auto newOwner = squashNames(ctx, klassRef.owner, owner);
                return getOrDefineSymbol(ctx.withOwner(newOwner), klassRef.name, klassRef.loc);
            }
            default:
                Exception::raise("Invalid name reference");
        }
    }

    // Get the symbol for an already-defined owner. Limited to refs that can own things (classes and methods).
    core::SymbolRef getOwnerSymbol(FoundDefinitionRef ref) {
        switch (ref.kind()) {
            case DefinitionKind::Symbol:
                return ref.symbol();
            case DefinitionKind::Class:
                ENFORCE(ref.idx() < definedClasses.size());
                return definedClasses[ref.idx()];
            case DefinitionKind::Method:
                ENFORCE(ref.idx() < definedMethods.size());
                return definedMethods[ref.idx()];
            default:
                Exception::raise("Invalid owner reference");
        }
    }

    // Allow stub symbols created to hold intrinsics to be filled in
    // with real types from code
    bool isIntrinsic(const core::SymbolData &data) {
        return data->intrinsic != nullptr && !data->hasSig();
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
        emitRedefinedConstantError(ctx, errorLoc, symbol.data(ctx)->show(ctx), renamedSymbol.data(ctx)->loc());
    }

    core::SymbolRef ensureIsClass(core::MutableContext ctx, core::SymbolRef scope, core::NameRef name,
                                  core::LocOffsets loc) {
        // Common case: Everything is fine, user is trying to define a symbol on a class or module.
        if (scope.data(ctx)->isClassOrModule()) {
            // Check if original symbol was mangled away. If so, complain.
            auto renamedSymbol = ctx.state.findRenamedSymbol(scope.data(ctx)->owner, scope);
            if (renamedSymbol.exists()) {
                if (auto e = ctx.state.beginError(core::Loc(ctx.file, loc), core::errors::Namer::InvalidClassOwner)) {
                    auto constLitName = name.data(ctx)->show(ctx);
                    auto scopeName = scope.data(ctx)->show(ctx);
                    e.setHeader("Can't nest `{}` under `{}` because `{}` is not a class or module", constLitName,
                                scopeName, scopeName);
                    e.addErrorLine(renamedSymbol.data(ctx)->loc(), "`{}` defined here", scopeName);
                }
            }
            return scope;
        }

        // Check if class was already mangled.
        auto klassSymbol = ctx.state.lookupClassSymbol(scope.data(ctx)->owner, scope.data(ctx)->name);
        if (klassSymbol.exists()) {
            return klassSymbol;
        }

        if (auto e = ctx.state.beginError(core::Loc(ctx.file, loc), core::errors::Namer::InvalidClassOwner)) {
            auto constLitName = name.data(ctx)->show(ctx);
            auto newOwnerName = scope.data(ctx)->show(ctx);
            e.setHeader("Can't nest `{}` under `{}` because `{}` is not a class or module", constLitName, newOwnerName,
                        newOwnerName);
            e.addErrorLine(scope.data(ctx)->loc(), "`{}` defined here", newOwnerName);
        }
        // Mangle this one out of the way, and re-enter a symbol with this name as a class.
        auto scopeName = scope.data(ctx)->name;
        ctx.state.mangleRenameSymbol(scope, scopeName);
        scope = ctx.state.enterClassSymbol(core::Loc(ctx.file, loc), scope.data(ctx)->owner, scopeName);
        scope.data(ctx)->singletonClass(ctx); // force singleton class into existance
        return scope;
    }

    // Gets the symbol with the given name, or defines it as a class if it does not exist.
    core::SymbolRef getOrDefineSymbol(core::MutableContext ctx, core::NameRef name, core::LocOffsets loc) {
        if (name == core::Names::singleton()) {
            return ctx.owner.data(ctx)->enclosingClass(ctx).data(ctx)->singletonClass(ctx);
        }

        auto scope = ensureIsClass(ctx, ctx.owner, name, loc);
        core::SymbolRef existing = scope.data(ctx)->findMember(ctx, name);
        if (!existing.exists()) {
            existing = ctx.state.enterClassSymbol(core::Loc(ctx.file, loc), scope, name);
            existing.data(ctx)->singletonClass(ctx); // force singleton class into existance
        }

        return existing;
    }

    void defineArg(core::MutableContext ctx, core::SymbolData &methodData, int pos, const ast::ParsedArg &parsedArg) {
        if (pos < methodData->arguments().size()) {
            // TODO: check that flags match;
            methodData->arguments()[pos].loc = core::Loc(ctx.file, parsedArg.loc);
            return;
        }

        core::NameRef name;
        if (parsedArg.flags.isKeyword) {
            name = parsedArg.local._name;
        } else if (parsedArg.flags.isBlock) {
            name = core::Names::blkArg();
        } else {
            name = ctx.state.freshNameUnique(core::UniqueNameKind::PositionalArg, core::Names::arg(), pos + 1);
        }
        // we know right now that pos >= arguments().size() because otherwise we would have hit the early return at the
        // beginning of this method
        auto &argInfo = ctx.state.enterMethodArgumentSymbol(core::Loc(ctx.file, parsedArg.loc), ctx.owner, name);
        // if enterMethodArgumentSymbol did not emplace a new argument into the list, then it means it's reusing an
        // existing one, which means we've seen a repeated kwarg (as it treats identically named kwargs as
        // identical). We know that we need to match the arity of the function as written, so if we don't have as many
        // arguments as we expect, clone the one we got back from enterMethodArgumentSymbol in the position we expect
        if (methodData->arguments().size() == pos) {
            auto argCopy = argInfo.deepCopy();
            argCopy.name = ctx.state.freshNameUnique(core::UniqueNameKind::MangledKeywordArg, argInfo.name, pos + 1);
            methodData->arguments().emplace_back(move(argCopy));
            return;
        }
        // at this point, we should have at least pos + 1 arguments, and arguments[pos] should be the thing we got back
        // from enterMethodArgumentSymbol
        ENFORCE(methodData->arguments().size() >= pos + 1);

        argInfo.flags = parsedArg.flags;
    }

    void defineArgs(core::MutableContext ctx, const vector<ast::ParsedArg> &parsedArgs) {
        auto methodData = ctx.owner.data(ctx);
        bool inShadows = false;
        bool intrinsic = isIntrinsic(methodData);
        bool swapArgs = intrinsic && (methodData->arguments().size() == 1);
        core::ArgInfo swappedArg;
        if (swapArgs) {
            // When we're filling in an intrinsic method, we want to overwrite the block arg that used
            // to exist with the block arg that we got from desugaring the method def in the RBI files.
            ENFORCE(methodData->arguments()[0].flags.isBlock);
            swappedArg = move(methodData->arguments()[0]);
            methodData->arguments().clear();
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
                    methodData->arguments().emplace_back(move(swappedArg));
                }

                defineArg(ctx, methodData, i, arg);
                ENFORCE(i < methodData->arguments().size());
            }
        }
    }

    bool paramsMatch(core::MutableContext ctx, core::SymbolRef method, const vector<ast::ParsedArg> &parsedArgs) {
        auto sym = method.data(ctx)->dealias(ctx);
        if (sym.data(ctx)->arguments().size() != parsedArgs.size()) {
            return false;
        }
        for (int i = 0; i < parsedArgs.size(); i++) {
            auto &methodArg = parsedArgs[i];
            auto &symArg = sym.data(ctx)->arguments()[i];

            if (symArg.flags.isKeyword != methodArg.flags.isKeyword ||
                symArg.flags.isBlock != methodArg.flags.isBlock ||
                symArg.flags.isRepeated != methodArg.flags.isRepeated ||
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

            if (symArg.flags.isKeyword != methodArg.flags.isKeyword) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::RedefinitionOfMethod)) {
                    e.setHeader("Method `{}` redefined with argument `{}` as a {} argument", sym.show(ctx),
                                methodArg.local.toString(ctx), methodArg.flags.isKeyword ? "keyword" : "non-keyword");
                    e.addErrorLine(
                        sym.data(ctx)->loc(),
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
                    e.addErrorLine(sym.data(ctx)->loc(),
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
                    e.addErrorLine(sym.data(ctx)->loc(), "Previous definition");
                }
                return;
            }
        }
    }

    core::SymbolRef defineMethod(core::MutableContext ctx, const FoundMethod &method) {
        core::SymbolRef owner = methodOwner(ctx, method.flags);

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
        auto symTableSize = ctx.state.symbolsUsed();
        auto sym = ctx.state.enterMethodSymbol(method.declLoc, owner, method.name);
        const bool isNewSymbol = symTableSize != ctx.state.symbolsUsed();
        if (!isNewSymbol) {
            // See if this is == to the method we're defining now, or if we have a redefinition error.
            auto matchingSym = ctx.state.lookupMethodSymbolWithHash(owner, method.name, method.argsHash);
            if (!matchingSym.exists()) {
                // we don't have a method definition with the right argument structure, so we need to mangle the
                // existing one and create a new one
                if (!isIntrinsic(sym.data(ctx))) {
                    paramMismatchErrors(ctx.withOwner(sym), method.declLoc, parsedArgs);
                    ctx.state.mangleRenameSymbol(sym, method.name);
                    // Re-enter a new symbol.
                    sym = ctx.state.enterMethodSymbol(method.declLoc, owner, method.name);
                } else {
                    // ...unless it's an intrinsic, because we allow multiple incompatible definitions of those in code
                    // TODO(jvilk): Wouldn't this always fail since `!sym.exists()`?
                    matchingSym.data(ctx)->addLoc(ctx, method.declLoc);
                }
            } else {
                // if the symbol does exist, then we're running in incremental mode, and we need to compare it to
                // the previously defined equivalent to re-report any errors
                auto replacedSym = ctx.state.findRenamedSymbol(owner, matchingSym);
                if (replacedSym.exists() && !paramsMatch(ctx, replacedSym, parsedArgs) &&
                    !isIntrinsic(replacedSym.data(ctx))) {
                    paramMismatchErrors(ctx.withOwner(replacedSym), method.declLoc, parsedArgs);
                }
                sym = matchingSym;
            }
        }

        defineArgs(ctx.withOwner(sym), parsedArgs);
        sym.data(ctx)->addLoc(ctx, method.declLoc);
        if (method.flags.isRewriterSynthesized) {
            sym.data(ctx)->setRewriterSynthesized();
        }
        ENFORCE(ctx.state.lookupMethodSymbolWithHash(owner, method.name, method.argsHash).exists());
        return sym;
    }

    core::SymbolRef insertMethod(core::MutableContext ctx, const FoundMethod &method) {
        auto symbol = defineMethod(ctx, method);
        auto implicitlyPrivate = ctx.owner.data(ctx)->enclosingClass(ctx) == core::Symbols::root();
        if (implicitlyPrivate) {
            // Methods defined at the top level default to private (on Object)
            symbol.data(ctx)->setPrivate();
        } else {
            // All other methods default to public (their visibility might be changed later)
            symbol.data(ctx)->setPublic();
        }
        return symbol;
    }

    void modifyMethod(core::MutableContext ctx, const Modifier &mod) {
        ENFORCE(mod.kind == DefinitionKind::Method);

        auto owner = ctx.owner.data(ctx)->enclosingClass(ctx);
        if (mod.name._id == core::Names::privateClassMethod()._id) {
            owner = owner.data(ctx)->singletonClass(ctx);
        }
        auto method = ctx.state.lookupMethodSymbol(owner, mod.methodName);
        if (method.exists()) {
            switch (mod.name._id) {
                case core::Names::private_()._id:
                case core::Names::privateClassMethod()._id:
                    method.data(ctx)->setPrivate();
                    break;
                case core::Names::protected_()._id:
                    method.data(ctx)->setProtected();
                    break;
                case core::Names::public_()._id:
                    method.data(ctx)->setPublic();
                    break;
                default:
                    break;
            }
        }
    }

    core::SymbolRef getClassSymbol(core::MutableContext ctx, const FoundClass &klass) {
        core::SymbolRef symbol = squashNames(ctx, klass.klass, ctx.owner.data(ctx)->enclosingClass(ctx));
        ENFORCE(symbol.exists());

        const bool isModule = klass.classKind == ast::ClassDef::Kind::Module;
        if (!symbol.data(ctx)->isClassOrModule()) {
            // we might have already mangled the class symbol, so see if we have a symbol that is a class already
            auto klassSymbol = ctx.state.lookupClassSymbol(symbol.data(ctx)->owner, symbol.data(ctx)->name);
            if (klassSymbol.exists()) {
                return klassSymbol;
            }

            emitRedefinedConstantError(ctx, core::Loc(ctx.file, klass.loc), symbol.data(ctx)->show(ctx),
                                       symbol.data(ctx)->loc());

            auto origName = symbol.data(ctx)->name;
            ctx.state.mangleRenameSymbol(symbol, symbol.data(ctx)->name);
            symbol = ctx.state.enterClassSymbol(klass.declLoc, symbol.data(ctx)->owner, origName);
            symbol.data(ctx)->setIsModule(isModule);

            auto oldSymCount = ctx.state.symbolsUsed();
            auto newSingleton = symbol.data(ctx)->singletonClass(ctx); // force singleton class into existence
            ENFORCE(newSingleton._id >= oldSymCount,
                    "should be a fresh symbol. Otherwise we could be reusing an existing singletonClass");
            return symbol;
        } else if (symbol.data(ctx)->isClassModuleSet() && isModule != symbol.data(ctx)->isClassOrModuleModule()) {
            if (auto e = ctx.state.beginError(klass.declLoc, core::errors::Namer::ModuleKindRedefinition)) {
                e.setHeader("`{}` was previously defined as a `{}`", symbol.data(ctx)->show(ctx),
                            symbol.data(ctx)->isClassOrModuleModule() ? "module" : "class");

                for (auto loc : symbol.data(ctx)->locs()) {
                    if (loc != klass.declLoc) {
                        e.addErrorLine(loc, "Previous definition");
                    }
                }
            }
        } else {
            symbol.data(ctx)->setIsModule(isModule);
            auto renamed = ctx.state.findRenamedSymbol(symbol.data(ctx)->owner, symbol);
            if (renamed.exists()) {
                emitRedefinedConstantError(ctx, core::Loc(ctx.file, klass.loc), symbol, renamed);
            }
        }
        return symbol;
    }

    core::SymbolRef insertClass(core::MutableContext ctx, const FoundClass &klass) {
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

        // Don't add locs for root; 1) they aren't useful and 2) it'll end up with O(files in project) locs!
        if (symbol != core::Symbols::root()) {
            symbol.data(ctx)->addLoc(ctx, klass.declLoc);
        }
        symbol.data(ctx)->singletonClass(ctx); // force singleton class into existence

        // make sure we've added a static init symbol so we have it ready for the flatten pass later
        if (symbol == core::Symbols::root()) {
            ctx.state.staticInitForFile(core::Loc(ctx.file, klass.loc));
        } else {
            ctx.state.staticInitForClass(symbol, core::Loc(ctx.file, klass.loc));
        }

        return symbol;
    }

    void modifyClass(core::MutableContext ctx, const Modifier &mod) {
        ENFORCE(mod.kind == DefinitionKind::Class);
        const auto fun = mod.name;
        auto symbolData = ctx.owner.data(ctx);
        if (fun == core::Names::declareFinal()) {
            symbolData->setClassOrModuleFinal();
            symbolData->singletonClass(ctx).data(ctx)->setClassOrModuleFinal();
        }
        if (fun == core::Names::declareSealed()) {
            symbolData->setClassOrModuleSealed();

            auto classOfKlass = symbolData->singletonClass(ctx);
            auto sealedSubclasses = ctx.state.enterMethodSymbol(core::Loc(ctx.file, mod.loc), classOfKlass,
                                                                core::Names::sealedSubclasses());
            auto &blkArg =
                ctx.state.enterMethodArgumentSymbol(core::Loc::none(), sealedSubclasses, core::Names::blkArg());
            blkArg.flags.isBlock = true;

            // T.noreturn here represents the zero-length list of subclasses of this sealed class.
            // We will use T.any to record subclasses when they're resolved.
            sealedSubclasses.data(ctx)->resultType = core::Types::arrayOf(ctx, core::Types::bottom());
        }
        if (fun == core::Names::declareInterface() || fun == core::Names::declareAbstract()) {
            symbolData->setClassOrModuleAbstract();
            symbolData->singletonClass(ctx).data(ctx)->setClassOrModuleAbstract();
        }
        if (fun == core::Names::declareInterface()) {
            symbolData->setClassOrModuleInterface();
            if (!symbolData->isClassOrModuleModule()) {
                if (auto e = ctx.state.beginError(core::Loc(ctx.file, mod.loc), core::errors::Namer::InterfaceClass)) {
                    e.setHeader("Classes can't be interfaces. Use `abstract!` instead of `interface!`");
                    e.replaceWith("Change `interface!` to `abstract!`", core::Loc(ctx.file, mod.loc), "abstract!");
                }
            }
        }
    }

    core::SymbolRef insertStaticField(core::MutableContext ctx, const FoundStaticField &staticField) {
        // forbid dynamic constant definition
        auto ownerData = ctx.owner.data(ctx);
        if (!ownerData->isClassOrModule() && !ownerData->isRewriterSynthesized()) {
            if (auto e = ctx.state.beginError(core::Loc(ctx.file, staticField.asgnLoc),
                                              core::errors::Namer::DynamicConstantAssignment)) {
                e.setHeader("Dynamic constant assignment");
            }
        }

        auto scope = squashNames(ctx, staticField.klass, contextClass(ctx, ctx.owner));
        scope = ensureIsClass(ctx, scope, staticField.name, staticField.asgnLoc);
        auto sym = ctx.state.lookupStaticFieldSymbol(scope, staticField.name);
        auto currSym = ctx.state.lookupSymbol(scope, staticField.name);
        auto name = sym.exists() ? sym.data(ctx)->name : staticField.name;
        if (!sym.exists() && currSym.exists()) {
            emitRedefinedConstantError(ctx, core::Loc(ctx.file, staticField.asgnLoc),
                                       staticField.name.data(ctx)->show(ctx), currSym.data(ctx)->loc());
            ctx.state.mangleRenameSymbol(currSym, currSym.data(ctx)->name);
        }
        if (sym.exists()) {
            ENFORCE(currSym.exists());
            auto renamedSym = ctx.state.findRenamedSymbol(scope, sym);
            if (renamedSym.exists()) {
                emitRedefinedConstantError(ctx, core::Loc(ctx.file, staticField.asgnLoc),
                                           renamedSym.data(ctx)->name.show(ctx), renamedSym.data(ctx)->loc());
            }
        }
        sym = ctx.state.enterStaticFieldSymbol(core::Loc(ctx.file, staticField.lhsLoc), scope, name);

        if (staticField.isTypeAlias && sym.data(ctx)->isStaticField()) {
            sym.data(ctx)->setTypeAlias();
        }

        return sym;
    }

    core::SymbolRef insertTypeMember(core::MutableContext ctx, const FoundTypeMember &typeMember) {
        if (ctx.owner == core::Symbols::root()) {
            FoundStaticField staticField;
            staticField.owner = typeMember.owner;
            staticField.name = typeMember.name;
            staticField.asgnLoc = typeMember.asgnLoc;
            staticField.lhsLoc = typeMember.asgnLoc;
            staticField.isTypeAlias = true;
            return insertStaticField(ctx, staticField);
        }

        core::Variance variance = core::Variance::Invariant;
        const bool isTypeTemplate = typeMember.isTypeTemplete;

        auto onSymbol = isTypeTemplate ? ctx.owner.data(ctx)->singletonClass(ctx) : ctx.owner;

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

        core::SymbolRef sym;
        auto existingTypeMember = ctx.state.lookupTypeMemberSymbol(onSymbol, typeMember.name);
        if (existingTypeMember.exists()) {
            // if we already have a type member but it was constructed in a different file from the one we're
            // looking at, then we need to raise an error
            if (existingTypeMember.data(ctx)->loc().file() != ctx.file) {
                if (auto e = ctx.state.beginError(core::Loc(ctx.file, typeMember.asgnLoc),
                                                  core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Duplicate type member `{}`", typeMember.name.data(ctx)->show(ctx));
                    e.addErrorLine(existingTypeMember.data(ctx)->loc(), "Also defined here");
                }
                if (auto e = ctx.state.beginError(existingTypeMember.data(ctx)->loc(),
                                                  core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Duplicate type member `{}`", typeMember.name.data(ctx)->show(ctx));
                    e.addErrorLine(core::Loc(ctx.file, typeMember.asgnLoc), "Also defined here");
                }
            }

            // otherwise, we're looking at a type member defined in this class in the same file, which means all we
            // need to do is find out whether there was a redefinition the first time, and in that case display the
            // same error
            auto oldSym = ctx.state.findRenamedSymbol(onSymbol, existingTypeMember);
            if (oldSym.exists()) {
                emitRedefinedConstantError(ctx, core::Loc(ctx.file, typeMember.nameLoc), oldSym, existingTypeMember);
            }
            // if we have more than one type member with the same name, then we have messed up somewhere
            ENFORCE(absl::c_find_if(onSymbol.data(ctx)->typeMembers(), [&](auto mem) {
                        return mem.data(ctx)->name == existingTypeMember.data(ctx)->name;
                    }) != onSymbol.data(ctx)->typeMembers().end());
            sym = existingTypeMember;
        } else {
            auto oldSym = onSymbol.data(ctx)->findMemberNoDealias(ctx, typeMember.name);
            if (oldSym.exists()) {
                emitRedefinedConstantError(ctx, core::Loc(ctx.file, typeMember.nameLoc), oldSym.data(ctx)->show(ctx),
                                           oldSym.data(ctx)->loc());
                ctx.state.mangleRenameSymbol(oldSym, oldSym.data(ctx)->name);
            }
            sym =
                ctx.state.enterTypeMember(core::Loc(ctx.file, typeMember.asgnLoc), onSymbol, typeMember.name, variance);

            // The todo bounds will be fixed by the resolver in ResolveTypeParamsWalk.
            auto todo = core::make_type<core::ClassType>(core::Symbols::todo());
            sym.data(ctx)->resultType = core::make_type<core::LambdaParam>(sym, todo, todo);

            if (isTypeTemplate) {
                auto context = ctx.owner.data(ctx)->enclosingClass(ctx);
                oldSym = context.data(ctx)->findMemberNoDealias(ctx, typeMember.name);
                if (oldSym.exists() && !(oldSym.data(ctx)->loc() == core::Loc(ctx.file, typeMember.asgnLoc) ||
                                         oldSym.data(ctx)->loc().isTombStoned(ctx))) {
                    emitRedefinedConstantError(ctx, core::Loc(ctx.file, typeMember.nameLoc),
                                               typeMember.name.data(ctx)->show(ctx), oldSym.data(ctx)->loc());
                    ctx.state.mangleRenameSymbol(oldSym, typeMember.name);
                }
                auto alias =
                    ctx.state.enterStaticFieldSymbol(core::Loc(ctx.file, typeMember.asgnLoc), context, typeMember.name);
                alias.data(ctx)->resultType = core::make_type<core::AliasType>(sym);
            }
        }

        if (typeMember.isFixed) {
            sym.data(ctx)->setFixed();
        }

        return sym;
    }

public:
    SymbolDefiner(unique_ptr<const FoundDefinitions> foundDefs) : foundDefs(move(foundDefs)) {}

    void run(core::MutableContext ctx) {
        definedClasses.reserve(foundDefs->klasses().size());
        definedMethods.reserve(foundDefs->methods().size());

        for (auto &ref : foundDefs->definitions()) {
            switch (ref.kind()) {
                case DefinitionKind::Class: {
                    const auto &klass = ref.klass(*foundDefs);
                    ENFORCE(definedClasses.size() == ref.idx());
                    definedClasses.emplace_back(insertClass(ctx.withOwner(getOwnerSymbol(klass.owner)), klass));
                    break;
                }
                case DefinitionKind::Method: {
                    const auto &method = ref.method(*foundDefs);
                    ENFORCE(definedMethods.size() == ref.idx());
                    definedMethods.emplace_back(insertMethod(ctx.withOwner(getOwnerSymbol(method.owner)), method));
                    break;
                }
                case DefinitionKind::StaticField: {
                    const auto &staticField = ref.staticField(*foundDefs);
                    insertStaticField(ctx.withOwner(getOwnerSymbol(staticField.owner)), staticField);
                    break;
                }
                case DefinitionKind::TypeMember: {
                    const auto &typeMember = ref.typeMember(*foundDefs);
                    insertTypeMember(ctx.withOwner(getOwnerSymbol(typeMember.owner)), typeMember);
                    break;
                }
                default:
                    ENFORCE(false);
                    break;
            }
        }

        // TODO: Split up?
        for (const auto &modifier : foundDefs->modifiers()) {
            const auto owner = getOwnerSymbol(modifier.owner);
            switch (modifier.kind) {
                case DefinitionKind::Method:
                    modifyMethod(ctx.withOwner(owner), modifier);
                    break;
                case DefinitionKind::Class:
                    modifyClass(ctx.withOwner(owner), modifier);
                    break;
                default:
                    Exception::raise("Found invalid modifier");
            }
        }
    }
};

/**
 * Inserts newly created symbols (from SymbolDefiner) into a tree.
 */
class TreeSymbolizer {
    friend class Namer;

    core::SymbolRef squashNames(core::Context ctx, core::SymbolRef owner, ast::TreePtr &node) {
        auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(node);
        if (constLit == nullptr) {
            if (auto *id = ast::cast_tree<ast::ConstantLit>(node)) {
                return id->symbol.data(ctx)->dealias(ctx);
            }
            if (auto *uid = ast::cast_tree<ast::UnresolvedIdent>(node)) {
                if (uid->kind != ast::UnresolvedIdent::Kind::Class || uid->name != core::Names::singleton()) {
                    if (auto e = ctx.beginError(node->loc, core::errors::Namer::DynamicConstant)) {
                        e.setHeader("Unsupported constant scope");
                    }
                }
                // emitted via `class << self` blocks
            } else if (ast::isa_tree<ast::EmptyTree>(node)) {
                // ::Foo
            } else if (node->isSelfReference()) {
                // self::Foo
            } else {
                if (auto e = ctx.beginError(node->loc, core::errors::Namer::DynamicConstant)) {
                    e.setHeader("Dynamic constant references are unsupported");
                }
            }
            node = ast::MK::EmptyTree();
            return owner;
        }

        auto newOwner = squashNames(ctx, owner, constLit->scope);
        core::SymbolRef existing = newOwner.data(ctx)->findMember(ctx, constLit->cnst);
        // NameInserter should have created this symbol.
        ENFORCE(existing.exists());

        node = ast::make_tree<ast::ConstantLit>(constLit->loc, existing, std::move(node));
        return existing;
    }

    ast::TreePtr arg2Symbol(core::Context ctx, int pos, ast::ParsedArg parsedArg, ast::TreePtr arg) {
        ast::TreePtr localExpr = ast::make_tree<ast::Local>(parsedArg.loc, parsedArg.local);
        if (parsedArg.flags.isDefault) {
            localExpr =
                ast::MK::OptionalArg(parsedArg.loc, move(localExpr), ast::ArgParsing::getDefault(parsedArg, move(arg)));
        }
        return localExpr;
    }

    void addAncestor(core::Context ctx, ast::ClassDef &klass, ast::TreePtr &node) {
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
        if (!send->recv->isSelfReference()) {
            // ignore `something.include`
            return;
        }

        if (send->args.empty()) {
            if (auto e = ctx.beginError(send->loc, core::errors::Namer::IncludeMutipleParam)) {
                e.setHeader("`{}` requires at least one argument", send->fun.data(ctx)->show(ctx));
            }
            return;
        }

        if (send->block != nullptr) {
            if (auto e = ctx.beginError(send->loc, core::errors::Namer::IncludePassedBlock)) {
                e.setHeader("`{}` can not be passed a block", send->fun.data(ctx)->show(ctx));
            }
            return;
        }

        for (auto it = send->args.rbegin(); it != send->args.rend(); it++) {
            // Reverse order is intentional: that's how Ruby does it.
            auto &arg = *it;
            if (ast::isa_tree<ast::EmptyTree>(arg)) {
                continue;
            }
            if (arg->isSelfReference()) {
                dest->emplace_back(arg->deepCopy());
                continue;
            }
            if (isValidAncestor(arg)) {
                dest->emplace_back(arg->deepCopy());
            } else {
                if (auto e = ctx.beginError(arg->loc, core::errors::Namer::AncestorNotConstant)) {
                    e.setHeader("`{}` must only contain constant literals", send->fun.data(ctx)->show(ctx));
                }
                arg = ast::MK::EmptyTree();
            }
        }
    }

    bool isValidAncestor(ast::TreePtr &exp) {
        if (ast::isa_tree<ast::EmptyTree>(exp) || exp->isSelfReference() || ast::isa_tree<ast::ConstantLit>(exp)) {
            return true;
        }
        if (auto lit = ast::cast_tree<ast::UnresolvedConstantLit>(exp)) {
            return isValidAncestor(lit->scope);
        }
        return false;
    }

public:
    ast::TreePtr preTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto &klass = ast::ref_tree<ast::ClassDef>(tree);

        auto *ident = ast::cast_tree<ast::UnresolvedIdent>(klass.name);

        if ((ident != nullptr) && ident->name == core::Names::singleton()) {
            ENFORCE(ident->kind == ast::UnresolvedIdent::Kind::Class);
            klass.symbol = ctx.owner.data(ctx)->enclosingClass(ctx).data(ctx)->lookupSingletonClass(ctx);
            ENFORCE(klass.symbol.exists());
        } else {
            if (klass.symbol == core::Symbols::todo()) {
                klass.symbol = squashNames(ctx, ctx.owner.data(ctx)->enclosingClass(ctx), klass.name);
            } else {
                // Desugar populates a top-level root() ClassDef.
                // Nothing else should have been typeAlias by now.
                ENFORCE(klass.symbol == core::Symbols::root());
            }
            if (!klass.symbol.data(ctx)->isClassOrModule()) {
                auto klassSymbol =
                    ctx.state.lookupClassSymbol(klass.symbol.data(ctx)->owner, klass.symbol.data(ctx)->name);
                ENFORCE(klassSymbol.exists());
                klass.symbol = klassSymbol;
            }
        }
        return tree;
    }

    // This decides if we need to keep a node around incase the current LSP query needs type information for it
    bool shouldLeaveAncestorForIDE(const ast::TreePtr &anc) {
        // used in Desugar <-> resolver to signal classes that did not have explicit superclass
        if (ast::isa_tree<ast::EmptyTree>(anc) || anc->isSelfReference()) {
            return false;
        }
        auto rcl = ast::cast_tree_const<ast::ConstantLit>(anc);
        if (rcl && rcl->symbol == core::Symbols::todo()) {
            return false;
        }
        return true;
    }

    ast::TreePtr postTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto &klass = ast::ref_tree<ast::ClassDef>(tree);

        // NameDefiner should have forced this class's singleton class into existence.
        ENFORCE(klass.symbol.data(ctx)->lookupSingletonClass(ctx).exists());

        for (auto &exp : klass.rhs) {
            addAncestor(ctx, klass, exp);
        }

        if (!klass.ancestors.empty()) {
            /* Superclass is typeAlias in parent scope, mixins are typeAlias in inner scope */
            for (auto &anc : klass.ancestors) {
                if (!isValidAncestor(anc)) {
                    if (auto e = ctx.beginError(anc->loc, core::errors::Namer::AncestorNotConstant)) {
                        e.setHeader("Superclasses must only contain constant literals");
                    }
                    anc = ast::MK::EmptyTree();
                }
            }
        }
        ast::InsSeq::STATS_store ideSeqs;
        if (ast::isa_tree<ast::ConstantLit>(klass.name)) {
            ideSeqs.emplace_back(ast::MK::KeepForIDE(klass.name->deepCopy()));
        }
        if (klass.kind == ast::ClassDef::Kind::Class && !klass.ancestors.empty() &&
            shouldLeaveAncestorForIDE(klass.ancestors.front())) {
            ideSeqs.emplace_back(ast::MK::KeepForIDE(klass.ancestors.front()->deepCopy()));
        }

        if (klass.symbol != core::Symbols::root() && !klass.declLoc.file().data(ctx).isRBI() &&
            ast::BehaviorHelpers::checkClassDefinesBehavior(klass)) {
            // TODO(dmitry) This won't find errors in fast-incremental mode.
            auto prevLoc = classBehaviorLocs.find(klass.symbol);
            if (prevLoc == classBehaviorLocs.end()) {
                classBehaviorLocs[klass.symbol] = klass.declLoc;
            } else if (prevLoc->second.file() != klass.declLoc.file()) {
                if (auto e = ctx.state.beginError(klass.declLoc, core::errors::Namer::MultipleBehaviorDefs)) {
                    e.setHeader("`{}` has behavior defined in multiple files", klass.symbol.data(ctx)->show(ctx));
                    e.addErrorLine(prevLoc->second, "Previous definition");
                }
            }
        }

        ast::InsSeq::STATS_store retSeqs;
        auto loc = klass.declLoc;
        retSeqs.emplace_back(std::move(tree));
        for (auto &stat : ideSeqs) {
            retSeqs.emplace_back(std::move(stat));
        }
        return ast::MK::InsSeq(loc.offsets(), std::move(retSeqs), ast::MK::EmptyTree());
    }

    ast::MethodDef::ARGS_store fillInArgs(core::Context ctx, vector<ast::ParsedArg> parsedArgs,
                                          ast::MethodDef::ARGS_store oldArgs) {
        ast::MethodDef::ARGS_store args;
        int i = -1;
        for (auto &arg : parsedArgs) {
            i++;
            auto localVariable = arg.local;

            if (arg.flags.isShadow) {
                auto localExpr = ast::make_tree<ast::Local>(arg.loc, localVariable);
                args.emplace_back(move(localExpr));
            } else {
                ENFORCE(i < oldArgs.size());
                auto expr = arg2Symbol(ctx, i, move(arg), move(oldArgs[i]));
                args.emplace_back(move(expr));
            }
        }

        return args;
    }

    ast::TreePtr preTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        auto &method = ast::ref_tree<ast::MethodDef>(tree);

        core::SymbolRef owner = methodOwner(ctx, method.flags);
        auto parsedArgs = ast::ArgParsing::parseArgs(method.args);
        auto sym = ctx.state.lookupMethodSymbolWithHash(owner, method.name, ast::ArgParsing::hashArgs(ctx, parsedArgs));
        ENFORCE(sym.exists());
        method.symbol = sym;
        method.args = fillInArgs(ctx.withOwner(method.symbol), move(parsedArgs), std::move(method.args));

        return tree;
    }

    ast::TreePtr postTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        auto &method = ast::ref_tree<ast::MethodDef>(tree);
        ENFORCE(method.args.size() == method.symbol.data(ctx)->arguments().size(), "{}: {} != {}",
                method.name.showRaw(ctx), method.args.size(), method.symbol.data(ctx)->arguments().size());
        return tree;
    }

    ast::TreePtr handleAssignment(core::Context ctx, ast::TreePtr tree) {
        auto &asgn = ast::ref_tree<ast::Assign>(tree);
        auto &lhs = ast::ref_tree<ast::UnresolvedConstantLit>(asgn.lhs);

        core::SymbolRef scope = squashNames(ctx, contextClass(ctx, ctx.owner), lhs.scope);
        if (!scope.data(ctx)->isClassOrModule()) {
            auto scopeName = scope.data(ctx)->name;
            scope = ctx.state.lookupClassSymbol(scope.data(ctx)->owner, scopeName);
        }

        core::SymbolRef cnst = ctx.state.lookupStaticFieldSymbol(scope, lhs.cnst);
        ENFORCE(cnst.exists());
        auto loc = lhs.loc;
        asgn.lhs = ast::make_tree<ast::ConstantLit>(loc, cnst, std::move(asgn.lhs));

        return tree;
    }

    ast::TreePtr handleTypeMemberDefinition(core::Context ctx, ast::Send *send, ast::TreePtr tree,
                                            const ast::UnresolvedConstantLit *typeName) {
        auto &asgn = ast::ref_tree<ast::Assign>(tree);

        ENFORCE(asgn.lhs.get() == typeName &&
                asgn.rhs.get() == send); // this method assumes that `asgn` owns `send` and `typeName`
        if (!ctx.owner.data(ctx)->isClassOrModule()) {
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
                                            ast::MK::Block0(asgn.loc, ast::MK::Untyped(asgn.loc)));

            return handleAssignment(ctx, ast::make_tree<ast::Assign>(asgn.loc, std::move(asgn.lhs), std::move(send)));
        }

        if (!send->args.empty()) {
            if (send->args.size() > 2) {
                if (auto e = ctx.state.beginError(core::Loc(ctx.file, send->loc),
                                                  core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Too many args in type definition");
                }
                auto send = ast::MK::Send1(asgn.loc, ast::MK::T(asgn.loc), core::Names::typeAlias(),
                                           ast::MK::Untyped(asgn.loc));
                return handleAssignment(ctx,
                                        ast::make_tree<ast::Assign>(asgn.loc, std::move(asgn.lhs), std::move(send)));
            }

            bool isTypeTemplate = send->fun == core::Names::typeTemplate();
            auto onSymbol = isTypeTemplate ? ctx.owner.data(ctx)->lookupSingletonClass(ctx) : ctx.owner;
            ENFORCE(onSymbol.exists());
            core::SymbolRef sym = ctx.state.lookupTypeMemberSymbol(onSymbol, typeName->cnst);
            ENFORCE(sym.exists());
            auto lit = ast::cast_tree<ast::Literal>(send->args.front());
            if (auto *hash = ast::cast_tree<ast::Hash>(send->args.back())) {
                bool fixed = false;
                bool bounded = false;

                for (auto &keyExpr : hash->keys) {
                    auto key = ast::cast_tree<ast::Literal>(keyExpr);
                    if (key != nullptr && key->isSymbol(ctx)) {
                        switch (key->asSymbol(ctx)._id) {
                            case core::Names::fixed()._id:
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
                    asgn.lhs = ast::MK::Constant(asgn.lhs->loc, sym);

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
            } else if (send->args.size() != 1 || !lit || !lit->isSymbol(ctx)) {
                if (auto e = ctx.beginError(send->loc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Invalid param, must be a :symbol");
                }
            }
        }

        return tree;
    }

    ast::TreePtr postTransformAssign(core::Context ctx, ast::TreePtr tree) {
        auto &asgn = ast::ref_tree<ast::Assign>(tree);

        auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn.lhs);
        if (lhs == nullptr) {
            return tree;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn.rhs);
        if (send == nullptr) {
            return handleAssignment(ctx, std::move(tree));
        }

        if (!send->recv->isSelfReference()) {
            return handleAssignment(ctx, std::move(tree));
        }

        switch (send->fun._id) {
            case core::Names::typeTemplate()._id:
                return handleTypeMemberDefinition(ctx, send, std::move(tree), lhs);
            case core::Names::typeMember()._id:
                return handleTypeMemberDefinition(ctx, send, std::move(tree), lhs);
            default:
                return handleAssignment(ctx, std::move(tree));
        }
    }

private:
    UnorderedMap<core::SymbolRef, core::Loc> classBehaviorLocs;
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
                Timer timeit(gs.tracer(), "naming.findSymbolsOne", {{"file", (string)job.file.data(gs).path()}});
                core::Context ctx(gs, core::Symbols::root(), job.file);
                job.tree = ast::TreeMap::apply(ctx, finder, std::move(job.tree));
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

ast::ParsedFilesOrCancelled defineSymbols(core::GlobalState &gs, vector<SymbolFinderResult> allFoundDefinitions) {
    Timer timeit(gs.tracer(), "naming.defineSymbols");
    vector<ast::ParsedFile> output;
    output.reserve(allFoundDefinitions.size());
    const auto &epochManager = *gs.epochManager;
    u4 count = 0;
    for (auto &fileFoundDefinitions : allFoundDefinitions) {
        count++;
        // defineSymbols is really fast. Avoid this mildly expensive check for most turns of the loop.
        if (count % 250 == 0 && epochManager.wasTypecheckingCanceled()) {
            return ast::ParsedFilesOrCancelled();
        }
        core::MutableContext ctx(gs, core::Symbols::root(), fileFoundDefinitions.tree.file);
        SymbolDefiner symbolDefiner(move(fileFoundDefinitions.names));
        output.emplace_back(move(fileFoundDefinitions.tree));
        symbolDefiner.run(ctx);
    }
    return output;
}

vector<ast::ParsedFile> symbolizeTrees(const core::GlobalState &gs, vector<ast::ParsedFile> trees,
                                       WorkerPool &workers) {
    Timer timeit(gs.tracer(), "naming.symbolizeTrees");
    auto resultq = make_shared<BlockingBoundedQueue<vector<ast::ParsedFile>>>(trees.size());
    auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
    for (auto &tree : trees) {
        fileq->push(move(tree), 1);
    }

    workers.multiplexJob("symbolizeTrees", [&gs, fileq, resultq]() {
        Timer timeit(gs.tracer(), "naming.symbolizeTreesWorker");
        TreeSymbolizer inserter;
        vector<ast::ParsedFile> output;
        ast::ParsedFile job;
        for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
            if (result.gotItem()) {
                Timer timeit(gs.tracer(), "naming.symbolizeTreesOne", {{"file", (string)job.file.data(gs).path()}});
                core::Context ctx(gs, core::Symbols::root(), job.file);
                job.tree = ast::TreeMap::apply(ctx, inserter, std::move(job.tree));
                output.emplace_back(move(job));
            }
        }
        if (!output.empty()) {
            resultq->push(move(output), output.size());
        }
    });
    trees.clear();

    {
        vector<ast::ParsedFile> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem()) {
                trees.insert(trees.end(), make_move_iterator(threadResult.begin()),
                             make_move_iterator(threadResult.end()));
            }
        }
    }
    fast_sort(trees, [](const auto &lhs, const auto &rhs) -> bool { return lhs.file.id() < rhs.file.id(); });
    return trees;
}

} // namespace

ast::ParsedFilesOrCancelled Namer::run(core::GlobalState &gs, vector<ast::ParsedFile> trees, WorkerPool &workers) {
    auto foundDefs = findSymbols(gs, move(trees), workers);
    if (gs.epochManager->wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled();
    }
    auto result = defineSymbols(gs, move(foundDefs));
    if (!result.hasResult()) {
        return result;
    }
    trees = symbolizeTrees(gs, move(result.result()), workers);
    return trees;
}

}; // namespace sorbet::namer
