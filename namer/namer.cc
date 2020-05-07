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

using namespace std;

namespace sorbet::namer {

namespace {
class FoundNames;

struct FoundClassRef;
struct FoundClass;
struct FoundFieldOrVariable;
struct FoundTypeMember;
struct FoundMethod;

enum class NameKind : u1 {
    Root = 0,
    Class = 1,
    ClassRef = 2,
    Method = 3,
    FieldOrVariable = 4,
    TypeMember = 5,
    Symbol = 6,
};

class FoundNameRef final {
    u4 _id;

    // Store NameKind in upper 8 bits of idx.
    static u4 mask(NameKind kind, u4 idx) {
        return (u4)kind << 12 | idx;
    }

public:
    FoundNameRef() : FoundNameRef(NameKind::Root, 0) {}
    FoundNameRef(const FoundNameRef &nm) = default;
    FoundNameRef(FoundNameRef &&nm) = default;
    FoundNameRef &operator=(const FoundNameRef &rhs) = default;

    FoundNameRef(NameKind kind, u4 idx) : _id(mask(kind, idx)) {}

    NameKind kind() const {
        return (NameKind)(_id >> 12);
    }

    u4 idx() const {
        // Mask top bits.
        return _id & 0x0FFF;
    }

    FoundClassRef &klassRef(FoundNames &foundNames);
    const FoundClassRef &klassRef(const FoundNames &foundNames) const;

    FoundClass &klass(FoundNames &foundNames);
    const FoundClass &klass(const FoundNames &foundNames) const;

    FoundMethod &method(FoundNames &foundNames);
    const FoundMethod &method(const FoundNames &foundNames) const;

    FoundFieldOrVariable &fieldOrVariable(FoundNames &foundNames);
    const FoundFieldOrVariable &fieldOrVariable(const FoundNames &foundNames) const;

    FoundTypeMember &typeMember(FoundNames &foundNames);
    const FoundTypeMember &typeMember(const FoundNames &foundNames) const;

    core::SymbolRef symbol(const FoundNames &foundNames) const;

    // TODO: need?
    bool operator==(const FoundNameRef &rhs) const {
        return _id == rhs._id;
    }

    bool operator!=(const FoundNameRef &rhs) const {
        return !(rhs == *this);
    }
};

struct FoundClassRef final {
    FoundNameRef owner;
    core::NameRef name;
    core::Loc loc;
};

struct FoundClass final {
    FoundNameRef classRef;
    core::Loc loc;
    core::Loc declLoc;
    ast::ClassDef::Kind classKind;
};

struct FoundFieldOrVariable final {
    FoundNameRef owner;
    core::NameRef name;
    core::Loc asgnLoc;
    core::Loc lhsLoc;
    bool isTypeAlias = false;
};

struct FoundTypeMember final {
    FoundNameRef owner;
    core::NameRef name;
    core::Loc sendLoc;
    core::Loc asgnLoc;
    core::Loc nameLoc;
    core::NameRef varianceName;
    bool isFixed = false;
    bool isTypeTemplete = false;
    bool tooManyArgs = false;
};

struct FoundMethod final {
    FoundNameRef owner;
    core::NameRef name;
    core::Loc loc;
    core::Loc declLoc;
    ast::MethodDef::Flags flags;
    vector<ast::ParsedArg> parsedArgs;
};

struct Modifier {
    NameKind kind;
    FoundNameRef owner;
    core::Loc loc;
    // The name of the modification.
    core::NameRef name;
    // For methods: The name of the method being modified.
    core::NameRef methodName;
};

class FoundNames final {
    vector<FoundClassRef> _klassRefs;
    vector<FoundClass> _klasses;
    vector<FoundMethod> _methods;
    vector<FoundFieldOrVariable> _fieldOrVariables;
    vector<FoundTypeMember> _typeMembers;
    vector<core::SymbolRef> _symbols;
    vector<Modifier> _modifiers;

public:
    FoundNames() = default;
    FoundNames(FoundNames &&names) = default;
    FoundNames(const FoundNames &names) = delete;
    ~FoundNames() = default;

    FoundNameRef addClass(FoundClass &&klass) {
        const u4 idx = _klasses.size();
        _klasses.push_back(move(klass));
        return FoundNameRef(NameKind::Class, idx);
    }

    FoundNameRef addClassRef(FoundClassRef &&klassRef) {
        const u4 idx = _klassRefs.size();
        _klassRefs.push_back(move(klassRef));
        return FoundNameRef(NameKind::ClassRef, idx);
    }

    FoundNameRef addMethod(FoundMethod &&method) {
        const u4 idx = _methods.size();
        _methods.push_back(move(method));
        return FoundNameRef(NameKind::Method, idx);
    }

    FoundNameRef addFieldOrVariable(FoundFieldOrVariable &&fieldOrVariable) {
        const u4 idx = _fieldOrVariables.size();
        _fieldOrVariables.push_back(move(fieldOrVariable));
        return FoundNameRef(NameKind::FieldOrVariable, idx);
    }

    FoundNameRef addTypeMember(FoundTypeMember &&typeMember) {
        const u4 idx = _typeMembers.size();
        _typeMembers.push_back(move(typeMember));
        return FoundNameRef(NameKind::TypeMember, idx);
    }

    FoundNameRef addSymbol(core::SymbolRef symbol) {
        const u4 idx = _symbols.size();
        _symbols.push_back(move(symbol));
        return FoundNameRef(NameKind::Symbol, idx);
    }

    void addModifier(Modifier &&mod) {
        _modifiers.push_back(move(mod));
    }

    const vector<FoundClass> &klasses() const {
        return _klasses;
    }

    const vector<FoundClassRef> &klassRefs() const {
        return _klassRefs;
    }

    const vector<FoundMethod> &methods() const {
        return _methods;
    }

    const vector<FoundFieldOrVariable> &fieldOrVariables() const {
        return _fieldOrVariables;
    }

    const vector<FoundTypeMember> &typeMembers() const {
        return _typeMembers;
    }

    const vector<Modifier> &modifiers() const {
        return _modifiers;
    }

    const vector<core::SymbolRef> &symbols() const {
        return _symbols;
    }

    friend FoundNameRef;
};

FoundClassRef &FoundNameRef::klassRef(FoundNames &foundNames) {
    ENFORCE(kind() == NameKind::ClassRef);
    ENFORCE(foundNames._klassRefs.size() > idx());
    return foundNames._klassRefs[idx()];
}
const FoundClassRef &FoundNameRef::klassRef(const FoundNames &foundNames) const {
    ENFORCE(kind() == NameKind::ClassRef);
    ENFORCE(foundNames._klassRefs.size() > idx());
    return foundNames._klassRefs[idx()];
}

FoundClass &FoundNameRef::klass(FoundNames &foundNames) {
    ENFORCE(kind() == NameKind::Class);
    ENFORCE(foundNames._klasses.size() > idx());
    return foundNames._klasses[idx()];
}
const FoundClass &FoundNameRef::klass(const FoundNames &foundNames) const {
    ENFORCE(kind() == NameKind::Class);
    ENFORCE(foundNames._klasses.size() > idx());
    return foundNames._klasses[idx()];
}

FoundMethod &FoundNameRef::method(FoundNames &foundNames) {
    ENFORCE(kind() == NameKind::Method);
    ENFORCE(foundNames._methods.size() > idx());
    return foundNames._methods[idx()];
}
const FoundMethod &FoundNameRef::method(const FoundNames &foundNames) const {
    ENFORCE(kind() == NameKind::Method);
    ENFORCE(foundNames._methods.size() > idx());
    return foundNames._methods[idx()];
}

FoundFieldOrVariable &FoundNameRef::fieldOrVariable(FoundNames &foundNames) {
    ENFORCE(kind() == NameKind::FieldOrVariable);
    ENFORCE(foundNames._fieldOrVariables.size() > idx());
    return foundNames._fieldOrVariables[idx()];
}
const FoundFieldOrVariable &FoundNameRef::fieldOrVariable(const FoundNames &foundNames) const {
    ENFORCE(kind() == NameKind::FieldOrVariable);
    ENFORCE(foundNames._fieldOrVariables.size() > idx());
    return foundNames._fieldOrVariables[idx()];
}

FoundTypeMember &FoundNameRef::typeMember(FoundNames &foundNames) {
    ENFORCE(kind() == NameKind::TypeMember);
    ENFORCE(foundNames._typeMembers.size() > idx());
    return foundNames._typeMembers[idx()];
}
const FoundTypeMember &FoundNameRef::typeMember(const FoundNames &foundNames) const {
    ENFORCE(kind() == NameKind::TypeMember);
    ENFORCE(foundNames._typeMembers.size() > idx());
    return foundNames._typeMembers[idx()];
}

core::SymbolRef FoundNameRef::symbol(const FoundNames &foundNames) const {
    ENFORCE(kind() == NameKind::Symbol);
    ENFORCE(foundNames._symbols.size() > idx());
    return foundNames._symbols[idx()];
}

struct NameFinderResult {
    ast::ParsedFile tree;
    unique_ptr<FoundNames> names;
};

/**
 * Used with TreeMap to locate all of the class and method symbols defined in the tree.
 * Does not mutate GlobalState, which allows us to parallelize this process.
 * Does not report any errors, which lets us cache its output.
 * Produces a vector of symbols to insert, and a vector of modifiers to those symbols.
 */
class NameFinder {
    unique_ptr<FoundNames> foundNames = make_unique<FoundNames>();
    // The tree doesn't have symbols yet, so `ctx.owner`, which is a SymbolRef, is meaningless.
    // Instead, we track the owner manually as an index into `foundNames`; the item at that index
    // will define the `owner` symbol.
    // The u4 is actually index + 1 so we reserve 0 for the root owner.
    vector<FoundNameRef> ownerStack;

    void findClassModifiers(core::Context ctx, FoundNameRef klass, unique_ptr<ast::Expression> &line) {
        auto *send = ast::cast_tree<ast::Send>(line.get());
        if (send == nullptr) {
            return;
        }

        switch (send->fun._id) {
            case core::Names::declareFinal()._id:
            case core::Names::declareSealed()._id:
            case core::Names::declareInterface()._id:
            case core::Names::declareAbstract()._id: {
                Modifier mod;
                mod.kind = NameKind::Class;
                mod.owner = klass;
                mod.loc = send->loc;
                mod.name = send->fun;
                foundNames->addModifier(move(mod));
                break;
            }
            default:
                break;
        }
    }

    FoundNameRef getOwner() {
        if (ownerStack.empty()) {
            return FoundNameRef();
        }
        return ownerStack.back();
    }

    // Returns index to foundNames containing the given name. Recursively inserts definitions for its owners.
    FoundNameRef squashNames(core::Context ctx, FoundNameRef owner, const unique_ptr<ast::Expression> &node) {
        if (auto *id = ast::cast_tree<ast::ConstantLit>(node.get())) {
            // Already defined. Insert a foundname so we can reference it.
            auto sym = id->symbol.data(ctx)->dealias(ctx);
            ENFORCE(sym.exists());
            return foundNames->addSymbol(sym);
        } else if (auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(node.get())) {
            FoundClassRef found;
            found.owner = squashNames(ctx, owner, constLit->scope);
            found.name = constLit->cnst;
            found.loc = constLit->loc;
            return foundNames->addClassRef(move(found));
        } else {
            // `class <<self`, `::Foo`, `self::Foo`
            return owner;
        }
    }

public:
    unique_ptr<FoundNames> getAndClearFoundNames() {
        ownerStack.clear();
        // cerr << "Class refs: " << foundNames->klassRefs().size() << "\n";
        // cerr << "Class defs: " << foundNames->klasses().size() << "\n";
        // cerr << "Symbols:  " << foundNames->symbols().size() << "\n";
        auto rv = move(foundNames);
        foundNames = make_unique<FoundNames>();
        ENFORCE(foundNames->klassRefs().empty());
        return rv;
    }

    FoundNameRef enclosingClass(core::Context ctx, FoundNameRef ref) {
        FoundNameRef current = ref;
        while (true) {
            switch (current.kind()) {
                case NameKind::Class:
                case NameKind::ClassRef:
                    return current;
                case NameKind::Symbol:
                    return foundNames->addSymbol(current.symbol(*foundNames).data(ctx)->enclosingClass(ctx));
                case NameKind::Method:
                    current = current.method(*foundNames).owner;
                    break;
                case NameKind::Root:
                    return foundNames->addSymbol(core::Symbols::root().data(ctx)->enclosingClass(ctx));
                default:
                    // Only a class, classref, or method can be an owner.
                    ENFORCE(false);
                    break;
            }
        }
    }

    unique_ptr<ast::ClassDef> preTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> klass) {
        FoundClass found;
        found.classKind = klass->kind;
        found.loc = klass->loc;
        found.declLoc = klass->declLoc;

        auto owner = getOwner();
        auto *ident = ast::cast_tree<ast::UnresolvedIdent>(klass->name.get());
        if ((ident != nullptr) && ident->name == core::Names::singleton()) {
            FoundClassRef foundRef;
            foundRef.owner = owner;
            foundRef.name = ident->name;
            foundRef.loc = ident->loc;
            found.classRef = foundNames->addClassRef(move(foundRef));
        } else {
            if (klass->symbol == core::Symbols::todo()) {
                found.classRef = squashNames(ctx, enclosingClass(ctx, owner), klass->name);
            } else {
                // Desugar populates a top-level root() ClassDef.
                // Nothing else should have been typeAlias by now.
                ENFORCE(klass->symbol == core::Symbols::root());
                found.classRef = foundNames->addSymbol(klass->symbol);
            }
        }

        ownerStack.push_back(foundNames->addClass(move(found)));
        return klass;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> klass) {
        FoundNameRef klassName = ownerStack.back();
        ownerStack.pop_back();

        for (auto &exp : klass->rhs) {
            findClassModifiers(ctx, klassName, exp);
        }

        return klass;
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> method) {
        FoundMethod foundMethod;
        foundMethod.owner = getOwner();
        foundMethod.name = method->name;
        foundMethod.loc = method->loc;
        foundMethod.declLoc = method->declLoc;
        foundMethod.flags = method->flags;
        foundMethod.parsedArgs = ast::ArgParsing::parseArgs(method->args);
        ownerStack.push_back(foundNames->addMethod(move(foundMethod)));
        return method;
    }

    unique_ptr<ast::MethodDef> postTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> method) {
        ownerStack.pop_back();
        return method;
    }

    unique_ptr<ast::Expression> postTransformSend(core::Context ctx, unique_ptr<ast::Send> original) {
        if (original->args.size() == 1) {
            // Common case: Send is not to a modifier.
            switch (original->fun._id) {
                case core::Names::private_()._id:
                case core::Names::privateClassMethod()._id:
                case core::Names::protected_()._id:
                case core::Names::public_()._id:
                    break;
                default:
                    return original;
            }

            Modifier methodModifier;
            methodModifier.kind = NameKind::Method;
            methodModifier.owner = getOwner();
            methodModifier.loc = original->loc;
            methodModifier.name = original->fun;
            methodModifier.methodName = unwrapLiteralToMethodName(ctx, original, original->args[0].get());
            if (methodModifier.methodName.exists()) {
                foundNames->addModifier(move(methodModifier));
            }
        }
        return original;
    }

    core::NameRef unwrapLiteralToMethodName(core::Context ctx, const unique_ptr<ast::Send> &original,
                                            ast::Expression *expr) {
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

            auto recv = ast::cast_tree<ast::ConstantLit>(send->recv.get());
            if (recv == nullptr) {
                return core::NameRef::noName();
            }

            if (recv->symbol != core::Symbols::Sorbet_Private_Static()) {
                return core::NameRef::noName();
            }

            if (send->args.size() != 2) {
                return core::NameRef::noName();
            }

            return unwrapLiteralToMethodName(ctx, original, send->args[1].get());
        } else {
            ENFORCE(!ast::isa_tree<ast::MethodDef>(expr), "methods inside sends should be gone");
            return core::NameRef::noName();
        }
    }

    FoundNameRef fillAssign(core::Context ctx, const unique_ptr<ast::Assign> &asgn) {
        auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        ENFORCE(lhs);

        FoundFieldOrVariable found;
        found.owner = squashNames(ctx, getOwner(), lhs->scope);
        found.name = lhs->cnst;
        found.asgnLoc = asgn->loc;
        found.lhsLoc = lhs->loc;
        return foundNames->addFieldOrVariable(move(found));
    }

    FoundNameRef handleTypeMemberDefinition(core::Context ctx, const ast::Send *send,
                                            const unique_ptr<ast::Assign> &asgn,
                                            const ast::UnresolvedConstantLit *typeName) {
        ENFORCE(asgn->lhs.get() == typeName &&
                asgn->rhs.get() == send); // this method assumes that `asgn` owns `send` and `typeName`

        FoundTypeMember found;
        found.owner = getOwner();
        found.sendLoc = send->loc;
        found.asgnLoc = asgn->loc;
        found.nameLoc = typeName->loc;
        found.name = typeName->cnst;
        // Store name rather than core::Variance type so that we can defer reporting an error until later.
        found.varianceName = core::Names::empty();
        found.isTypeTemplete = send->fun == core::Names::typeTemplate();

        if (!send->args.empty()) {
            if (send->args.size() > 2) {
                // Defer error until next phase.
                found.tooManyArgs = true;
            }

            auto lit = ast::cast_tree<ast::Literal>(send->args[0].get());
            if (lit != nullptr && lit->isSymbol(ctx)) {
                found.varianceName = lit->asSymbol(ctx);
            }

            auto *hash = ast::cast_tree<ast::Hash>(send->args.back().get());
            if (hash) {
                for (auto &keyExpr : hash->keys) {
                    auto key = ast::cast_tree<ast::Literal>(keyExpr.get());
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
        return foundNames->addTypeMember(move(found));
    }

    FoundNameRef handleAssignment(core::Context ctx, const unique_ptr<ast::Assign> &asgn) {
        auto *send = ast::cast_tree<ast::Send>(asgn->rhs.get());
        auto foundRef = fillAssign(ctx, asgn);
        ENFORCE(foundRef.kind() == NameKind::FieldOrVariable);
        auto &fieldOrVariable = foundRef.fieldOrVariable(*foundNames);
        fieldOrVariable.isTypeAlias = send->fun == core::Names::typeAlias();
        return foundRef;
    }

    unique_ptr<ast::Expression> postTransformAssign(core::Context ctx, unique_ptr<ast::Assign> asgn) {
        auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        if (lhs == nullptr) {
            return asgn;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn->rhs.get());
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
        return asgn;
    }
};

/**
 * Defines symbols for all of the names found via NameFinder. Single threaded.
 */
class NameDefiner {
    unique_ptr<const FoundNames> foundNames;
    vector<core::SymbolRef> definedClasses;
    vector<core::SymbolRef> definedMethods;

    // Get the symbol for an already-defined name. Limited to names that can own things (classes and methods).
    core::SymbolRef getSymbol(FoundNameRef ref) {
        switch (ref.kind()) {
            case NameKind::Root:
                return core::Symbols::root();
            case NameKind::Symbol:
                return ref.symbol(*foundNames);
            case NameKind::ClassRef:
                ENFORCE(ref.idx() < definedClasses.size());
                return definedClasses[ref.idx()];
            case NameKind::Class:
                return getSymbol(ref.klass(*foundNames).classRef);
            case NameKind::Method:
                ENFORCE(ref.idx() < definedMethods.size());
                return definedMethods[ref.idx()];
            default:
                Exception::raise("Invalid name reference");
        }
    }

    core::SymbolRef methodOwner(core::MutableContext ctx) {
        core::SymbolRef owner = ctx.owner.data(ctx)->enclosingClass(ctx);
        if (owner == core::Symbols::root()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        return owner;
    }

    // Allow stub symbols created to hold intrinsics to be filled in
    // with real types from code
    bool isIntrinsic(core::Context ctx, core::SymbolRef sym) {
        auto data = sym.data(ctx);
        return data->intrinsic != nullptr && !data->hasSig();
    }

    core::SymbolRef defineClass(core::MutableContext ctx, const FoundClassRef &classRef) {
        auto name = classRef.name;
        auto loc = classRef.loc;
        if (name == core::Names::singleton()) {
            return ctx.owner.data(ctx)->enclosingClass(ctx).data(ctx)->singletonClass(ctx);
        }

        core::SymbolRef existing = ctx.owner.data(ctx)->findMember(ctx, name);
        if (!existing.exists()) {
            if (!ctx.owner.data(ctx)->isClassOrModule()) {
                if (auto e = ctx.state.beginError(loc, core::errors::Namer::InvalidClassOwner)) {
                    auto memberName = name.data(ctx)->show(ctx);
                    auto ownerName = ctx.owner.data(ctx)->show(ctx);
                    e.setHeader("Can't nest `{}` under `{}` because `{}` is not a class or module", memberName,
                                ownerName, ownerName);
                    e.addErrorLine(ctx.owner.data(ctx)->loc(), "`{}` defined here", ownerName);
                }
                return ctx.owner;
            }
            existing = ctx.state.enterClassSymbol(loc, ctx.owner, name);
            existing.data(ctx)->singletonClass(ctx); // force singleton class into existance
        }
        return existing;
    }

    void defineArg(core::MutableContext ctx, int pos, const ast::ParsedArg &parsedArg) {
        if (pos < ctx.owner.data(ctx)->arguments().size()) {
            // TODO: check that flags match;
            ctx.owner.data(ctx)->arguments()[pos].loc = parsedArg.loc;
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
        auto &argInfo = ctx.state.enterMethodArgumentSymbol(parsedArg.loc, ctx.owner, name);
        // if enterMethodArgumentSymbol did not emplace a new argument into the list, then it means it's reusing an
        // existing one, which means we've seen a repeated kwarg (as it treats identically named kwargs as
        // identical). We know that we need to match the arity of the function as written, so if we don't have as many
        // arguments as we expect, clone the one we got back from enterMethodArgumentSymbol in the position we expect
        if (ctx.owner.dataAllowingNone(ctx)->arguments().size() == pos) {
            auto argCopy = argInfo.deepCopy();
            argCopy.name = ctx.state.freshNameUnique(core::UniqueNameKind::MangledKeywordArg, argInfo.name, pos + 1);
            ctx.owner.dataAllowingNone(ctx)->arguments().emplace_back(move(argCopy));
            return;
        }
        // at this point, we should have at least pos + 1 arguments, and arguments[pos] should be the thing we got back
        // from enterMethodArgumentSymbol
        ENFORCE(ctx.owner.data(ctx)->arguments().size() >= pos + 1);

        argInfo.flags = parsedArg.flags;
    }

    void defineArgs(core::MutableContext ctx, const vector<ast::ParsedArg> &parsedArgs) {
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
            if (arg.flags.isShadow) {
                inShadows = true;
            } else {
                ENFORCE(!inShadows, "shadow argument followed by non-shadow argument!");

                if (swapArgs && arg.flags.isBlock) {
                    // see commnent on if (swapArgs) above
                    ctx.owner.data(ctx)->arguments().emplace_back(move(swappedArg));
                }

                defineArg(ctx, i, move(arg));
                ENFORCE(i < ctx.owner.data(ctx)->arguments().size());
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
        core::SymbolRef owner = methodOwner(ctx);

        if (method.flags.isSelfMethod) {
            if (owner.data(ctx)->isClassOrModule()) {
                owner = owner.data(ctx)->singletonClass(ctx);
            }
        }
        ENFORCE(owner.data(ctx)->isClassOrModule());

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
        auto sym = ctx.state.lookupMethodSymbolWithHash(owner, method.name, ast::ArgParsing::hashArgs(ctx, parsedArgs));
        auto currentSym = ctx.state.lookupMethodSymbol(owner, method.name);

        if (!sym.exists() && currentSym.exists()) {
            // we don't have a method definition with the right argument structure, so we need to mangle the
            // existing one and create a new one
            if (!isIntrinsic(ctx, currentSym)) {
                paramMismatchErrors(ctx.withOwner(currentSym), method.declLoc, parsedArgs);
                ctx.state.mangleRenameSymbol(currentSym, method.name);
            } else {
                // ...unless it's an intrinsic, because we allow multiple incompatible definitions of those in code
                // TODO(jvilk): Wouldn't this always fail since `!sym.exists()`?
                sym.data(ctx)->addLoc(ctx, method.declLoc);
            }
        }
        if (sym.exists()) {
            // if the symbol does exist, then we're running in incremental mode, and we need to compare it to the
            // previously defined equivalent to re-report any errors
            auto replacedSym = ctx.state.findRenamedSymbol(owner, sym);
            if (replacedSym.exists() && !paramsMatch(ctx, replacedSym, parsedArgs) && !isIntrinsic(ctx, replacedSym)) {
                paramMismatchErrors(ctx.withOwner(replacedSym), method.declLoc, parsedArgs);
            }
            sym.data(ctx)->addLoc(ctx, method.declLoc);
            defineArgs(ctx.withOwner(sym), move(parsedArgs));
            return sym;
        }

        // we'll only get this far if we're not in incremental mode, so enter a new symbol and fill in the data
        // appropriately
        sym = ctx.state.enterMethodSymbol(method.declLoc, owner, method.name);
        defineArgs(ctx.withOwner(sym), move(parsedArgs));
        sym.data(ctx)->addLoc(ctx, method.declLoc);
        if (method.flags.isRewriterSynthesized) {
            sym.data(ctx)->setRewriterSynthesized();
        }
        ctx.state.tracer().debug("Defining method {}", sym.toString(ctx));
        return sym;
    }

    core::SymbolRef insertMethod(core::MutableContext ctx, const FoundMethod &name) {
        auto symbol = defineMethod(ctx, name);
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
        ENFORCE(mod.kind == NameKind::Method);

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
        core::SymbolRef symbol = getSymbol(klass.classRef);
        ENFORCE(symbol.exists());
        ENFORCE(klass.classKind != ast::ClassDef::Kind::Class ||
                symbol != ctx.owner.data(ctx)->enclosingClass(ctx).data(ctx)->lookupSingletonClass(ctx));

        const bool isModule = klass.classKind == ast::ClassDef::Kind::Module;
        if (!symbol.data(ctx)->isClassOrModule()) {
            // we might have already mangled the class symbol, so see if we have a symbol that is a class already
            auto klassSymbol = ctx.state.lookupClassSymbol(symbol.data(ctx)->owner, symbol.data(ctx)->name);
            if (klassSymbol.exists()) {
                return klassSymbol;
            }

            if (auto e = ctx.state.beginError(klass.loc, core::errors::Namer::ModuleKindRedefinition)) {
                e.setHeader("Redefining constant `{}`", symbol.data(ctx)->show(ctx));
                e.addErrorLine(symbol.data(ctx)->loc(), "Previous definition");
            }
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
                if (auto e = ctx.state.beginError(klass.loc, core::errors::Namer::ModuleKindRedefinition)) {
                    e.setHeader("Redefining constant `{}`", symbol.data(ctx)->show(ctx));
                    e.addErrorLine(renamed.data(ctx)->loc(), "Previous definition");
                }
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

        symbol.data(ctx)->addLoc(ctx, klass.declLoc);
        symbol.data(ctx)->singletonClass(ctx); // force singleton class into existence

        // make sure we've added a static init symbol so we have it ready for the flatten pass later
        if (symbol == core::Symbols::root()) {
            ctx.state.staticInitForFile(klass.loc);
        } else {
            ctx.state.staticInitForClass(symbol, klass.loc);
        }

        ctx.state.tracer().debug("Defining class {}", symbol.data(ctx)->name.toString(ctx));

        return symbol;
    }

    void modifyClass(core::MutableContext ctx, const Modifier &mod) {
        ENFORCE(mod.kind == NameKind::Class);
        const auto fun = mod.name;
        const auto symbol = getSymbol(mod.owner);
        auto symbolData = symbol.data(ctx);
        if (fun == core::Names::declareFinal()) {
            symbolData->setClassFinal();
            symbolData->singletonClass(ctx).data(ctx)->setClassFinal();
        }
        if (fun == core::Names::declareSealed()) {
            symbolData->setClassSealed();

            auto classOfKlass = symbolData->singletonClass(ctx);
            auto sealedSubclasses = ctx.state.enterMethodSymbol(mod.loc, classOfKlass, core::Names::sealedSubclasses());
            auto &blkArg =
                ctx.state.enterMethodArgumentSymbol(core::Loc::none(), sealedSubclasses, core::Names::blkArg());
            blkArg.flags.isBlock = true;

            // T.noreturn here represents the zero-length list of subclasses of this sealed class.
            // We will use T.any to record subclasses when they're resolved.
            sealedSubclasses.data(ctx)->resultType = core::Types::arrayOf(ctx, core::Types::bottom());
        }
        if (fun == core::Names::declareInterface() || fun == core::Names::declareAbstract()) {
            symbolData->setClassAbstract();
            symbolData->singletonClass(ctx).data(ctx)->setClassAbstract();
        }
        if (fun == core::Names::declareInterface()) {
            symbolData->setClassInterface();
            if (!symbolData->isClassOrModuleModule()) {
                if (auto e = ctx.state.beginError(mod.loc, core::errors::Namer::InterfaceClass)) {
                    e.setHeader("Classes can't be interfaces. Use `abstract!` instead of `interface!`");
                    e.replaceWith("Change `interface!` to `abstract!`", mod.loc, "abstract!");
                }
            }
        }
    }

    // Returns the SymbolRef corresponding to the class `self.class`, unless the
    // context is a class, in which case return it.
    core::SymbolRef contextClass(const core::GlobalState &gs, core::SymbolRef ofWhat) const {
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

    core::SymbolRef insertFieldOrVariable(core::MutableContext ctx, const FoundFieldOrVariable &fieldOrVariable) {
        // forbid dynamic constant definition
        // TODO: Does this check break w/ our overloading of `owner`?
        auto ownerData = ctx.owner.data(ctx);
        if (!ownerData->isClassOrModule() && !ownerData->isRewriterSynthesized()) {
            if (auto e =
                    ctx.state.beginError(fieldOrVariable.asgnLoc, core::errors::Namer::DynamicConstantAssignment)) {
                e.setHeader("Dynamic constant assignment");
            }
        }

        auto scope = contextClass(ctx.state, getSymbol(fieldOrVariable.owner));
        if (!scope.data(ctx)->isClassOrModule()) {
            if (auto e = ctx.state.beginError(fieldOrVariable.asgnLoc, core::errors::Namer::InvalidClassOwner)) {
                auto constLitName = fieldOrVariable.name.data(ctx)->show(ctx);
                auto scopeName = scope.data(ctx)->show(ctx);
                e.setHeader("Can't nest `{}` under `{}` because `{}` is not a class or module", constLitName, scopeName,
                            scopeName);
                e.addErrorLine(scope.data(ctx)->loc(), "`{}` defined here", scopeName);
            }
            // Mangle this one out of the way, and re-enter a symbol with this name as a class.
            auto scopeName = scope.data(ctx)->name;
            ctx.state.mangleRenameSymbol(scope, scopeName);
            scope = ctx.state.enterClassSymbol(fieldOrVariable.lhsLoc, scope.data(ctx)->owner, scopeName);
            scope.data(ctx)->singletonClass(ctx); // force singleton class into existance
        }

        auto sym = ctx.state.lookupStaticFieldSymbol(scope, fieldOrVariable.name);
        auto currSym = ctx.state.lookupSymbol(scope, fieldOrVariable.name);
        auto name = sym.exists() ? sym.data(ctx)->name : fieldOrVariable.name;
        if (!sym.exists() && currSym.exists()) {
            if (auto e = ctx.state.beginError(fieldOrVariable.asgnLoc, core::errors::Namer::ModuleKindRedefinition)) {
                e.setHeader("Redefining constant `{}`", fieldOrVariable.name.data(ctx)->show(ctx));
                e.addErrorLine(currSym.data(ctx)->loc(), "Previous definition");
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
                    e.addErrorLine(fieldOrVariable.asgnLoc, "Previous definition");
                }
            }
        }
        sym = ctx.state.enterStaticFieldSymbol(fieldOrVariable.lhsLoc, scope, name);

        if (fieldOrVariable.isTypeAlias && sym.data(ctx)->isStaticField()) {
            sym.data(ctx)->setTypeAlias();
        }

        ctx.state.tracer().debug("Defining static field symbol {}", sym.toString(ctx));
        return sym;
    }

    core::SymbolRef insertTypeMemberDefinition(core::MutableContext ctx, const FoundTypeMember &typeMember) {
        if (typeMember.tooManyArgs) {
            if (auto e = ctx.state.beginError(typeMember.sendLoc, core::errors::Namer::InvalidTypeDefinition)) {
                e.setHeader("Too many args in type definition");
            }
            FoundFieldOrVariable fieldOrVariable;
            fieldOrVariable.owner = typeMember.owner;
            fieldOrVariable.name = typeMember.name;
            fieldOrVariable.asgnLoc = typeMember.asgnLoc;
            fieldOrVariable.lhsLoc = typeMember.asgnLoc;
            fieldOrVariable.owner = typeMember.owner;
            fieldOrVariable.isTypeAlias = true;
            return insertFieldOrVariable(ctx, fieldOrVariable);
        }

        core::Variance variance = core::Variance::Invariant;
        const bool isTypeTemplate = typeMember.isTypeTemplete;

        ENFORCE(ctx.owner != core::Symbols::root());

        auto onSymbol = isTypeTemplate ? ctx.owner.data(ctx)->singletonClass(ctx) : ctx.owner;

        core::NameRef name = typeMember.varianceName;

        if (name == core::Names::covariant()) {
            variance = core::Variance::CoVariant;
        } else if (name == core::Names::contravariant()) {
            variance = core::Variance::ContraVariant;
        } else if (name == core::Names::invariant()) {
            variance = core::Variance::Invariant;
        }

        core::SymbolRef sym;
        auto existingTypeMember = ctx.state.lookupTypeMemberSymbol(onSymbol, typeMember.name);
        if (existingTypeMember.exists()) {
            // if we already have a type member but it was constructed in a different file from the one we're
            // looking at, then we need to raise an error
            if (existingTypeMember.data(ctx)->loc().file() != typeMember.asgnLoc.file()) {
                if (auto e = ctx.state.beginError(typeMember.asgnLoc, core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Duplicate type member `{}`", typeMember.name.data(ctx)->show(ctx));
                    e.addErrorLine(existingTypeMember.data(ctx)->loc(), "Also defined here");
                }
                if (auto e = ctx.state.beginError(existingTypeMember.data(ctx)->loc(),
                                                  core::errors::Namer::InvalidTypeDefinition)) {
                    e.setHeader("Duplicate type member `{}`", typeMember.name.data(ctx)->show(ctx));
                    e.addErrorLine(typeMember.asgnLoc, "Also defined here");
                }
            }

            // otherwise, we're looking at a type member defined in this class in the same file, which means all we
            // need to do is find out whether there was a redefinition the first time, and in that case display the
            // same error
            auto oldSym = ctx.state.findRenamedSymbol(onSymbol, existingTypeMember);
            if (oldSym.exists()) {
                if (auto e = ctx.state.beginError(typeMember.nameLoc, core::errors::Namer::ModuleKindRedefinition)) {
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
            auto oldSym = onSymbol.data(ctx)->findMemberNoDealias(ctx, typeMember.name);
            if (oldSym.exists()) {
                if (auto e = ctx.state.beginError(typeMember.nameLoc, core::errors::Namer::ModuleKindRedefinition)) {
                    e.setHeader("Redefining constant `{}`", oldSym.data(ctx)->show(ctx));
                    e.addErrorLine(oldSym.data(ctx)->loc(), "Previous definition");
                }
                ctx.state.mangleRenameSymbol(oldSym, oldSym.data(ctx)->name);
            }
            sym = ctx.state.enterTypeMember(typeMember.nameLoc, onSymbol, typeMember.name, variance);

            // The todo bounds will be fixed by the resolver in ResolveTypeParamsWalk.
            auto todo = core::make_type<core::ClassType>(core::Symbols::todo());
            sym.data(ctx)->resultType = core::make_type<core::LambdaParam>(sym, todo, todo);

            if (isTypeTemplate) {
                auto context = ctx.owner.data(ctx)->enclosingClass(ctx);
                oldSym = context.data(ctx)->findMemberNoDealias(ctx, typeMember.name);
                if (oldSym.exists() &&
                    !(oldSym.data(ctx)->loc() == typeMember.asgnLoc || oldSym.data(ctx)->loc().isTombStoned(ctx))) {
                    if (auto e =
                            ctx.state.beginError(typeMember.nameLoc, core::errors::Namer::ModuleKindRedefinition)) {
                        e.setHeader("Redefining constant `{}`", typeMember.name.data(ctx)->show(ctx));
                        e.addErrorLine(oldSym.data(ctx)->loc(), "Previous definition");
                    }
                    ctx.state.mangleRenameSymbol(oldSym, typeMember.name);
                }
                auto alias = ctx.state.enterStaticFieldSymbol(typeMember.asgnLoc, context, typeMember.name);
                alias.data(ctx)->resultType = core::make_type<core::AliasType>(sym);
            }

            ctx.state.tracer().debug("Defining type member {}", sym.toString(ctx));
        }

        if (typeMember.isFixed) {
            sym.data(ctx)->setFixed();
        }

        return sym;
    }

public:
    NameDefiner(unique_ptr<const FoundNames> foundNames) : foundNames(move(foundNames)) {}

    void run(core::MutableContext ctx) {
        for (auto &classRef : foundNames->klassRefs()) {
            definedClasses.push_back(defineClass(ctx.withOwner(getSymbol(classRef.owner)), classRef));
        }

        for (auto &klass : foundNames->klasses()) {
            insertClass(ctx.withOwner(getSymbol(klass.classRef).data(ctx)->owner), klass);
        }

        for (auto &method : foundNames->methods()) {
            definedMethods.push_back(insertMethod(ctx.withOwner(getSymbol(method.owner)), method));
        }

        for (auto &fieldOrVariable : foundNames->fieldOrVariables()) {
            insertFieldOrVariable(ctx.withOwner(getSymbol(fieldOrVariable.owner)), fieldOrVariable);
        }

        for (auto &typeMember : foundNames->typeMembers()) {
            insertTypeMemberDefinition(ctx.withOwner(getSymbol(typeMember.owner)), typeMember);
        }

        // TODO: Could these go on class/methods directly?
        for (const auto &modifier : foundNames->modifiers()) {
            const auto owner = getSymbol(modifier.owner);
            switch (modifier.kind) {
                case NameKind::Method:
                    modifyMethod(ctx.withOwner(owner), modifier);
                    break;
                case NameKind::Class:
                    modifyClass(ctx.withOwner(owner), modifier);
                    break;
                default:
                    Exception::raise("Found invalid modifier");
            }
        }
    }
};

/**
 * Inserts newly created names into a tree.
 */
class NameInserter {
    friend class Namer;

    core::SymbolRef squashNames(core::Context ctx, core::SymbolRef owner, unique_ptr<ast::Expression> &node) {
        auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(node.get());
        if (constLit == nullptr) {
            if (auto *id = ast::cast_tree<ast::ConstantLit>(node.get())) {
                return id->symbol.data(ctx)->dealias(ctx);
            }
            if (auto *uid = ast::cast_tree<ast::UnresolvedIdent>(node.get())) {
                if (uid->kind != ast::UnresolvedIdent::Kind::Class || uid->name != core::Names::singleton()) {
                    if (auto e = ctx.beginError(node->loc, core::errors::Namer::DynamicConstant)) {
                        e.setHeader("Unsupported constant scope");
                    }
                }
                // emitted via `class << self` blocks
            } else if (ast::isa_tree<ast::EmptyTree>(node.get())) {
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
        if (!existing.exists() && !newOwner.data(ctx)->isClassOrModule()) {
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
        // NameInserter should have created this symbol.
        ENFORCE(existing.exists());

        node.release();
        unique_ptr<ast::UnresolvedConstantLit> constTmp(constLit);
        node = make_unique<ast::ConstantLit>(constLit->loc, existing, std::move(constTmp));
        return existing;
    }

    unique_ptr<ast::Expression> arg2Symbol(core::Context ctx, int pos, ast::ParsedArg parsedArg,
                                           std::unique_ptr<ast::Expression> arg) {
        unique_ptr<ast::Reference> localExpr = make_unique<ast::Local>(parsedArg.loc, parsedArg.local);
        if (parsedArg.flags.isDefault) {
            localExpr =
                ast::MK::OptionalArg(parsedArg.loc, move(localExpr), ast::ArgParsing::getDefault(parsedArg, move(arg)));
        }
        return move(localExpr);
    }

    void addAncestor(core::Context ctx, unique_ptr<ast::ClassDef> &klass, const unique_ptr<ast::Expression> &node) {
        auto send = ast::cast_tree<ast::Send>(node.get());
        if (send == nullptr) {
            ENFORCE(node.get() != nullptr);
            return;
        }

        ast::ClassDef::ANCESTORS_store *dest;
        if (send->fun == core::Names::include()) {
            dest = &klass->ancestors;
        } else if (send->fun == core::Names::extend()) {
            dest = &klass->singletonAncestors;
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
            if (ast::isa_tree<ast::EmptyTree>(arg.get())) {
                continue;
            }
            if (arg->isSelfReference()) {
                dest->emplace_back(arg->deepCopy());
                continue;
            }
            if (isValidAncestor(arg.get())) {
                dest->emplace_back(arg->deepCopy());
            } else {
                if (auto e = ctx.beginError(arg->loc, core::errors::Namer::AncestorNotConstant)) {
                    e.setHeader("`{}` must only contain constant literals", send->fun.data(ctx)->show(ctx));
                }
                arg = ast::MK::EmptyTree();
            }
        }
    }

    core::SymbolRef methodOwner(core::Context ctx) {
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
    unique_ptr<ast::ClassDef> preTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> klass) {
        auto *ident = ast::cast_tree<ast::UnresolvedIdent>(klass->name.get());

        if ((ident != nullptr) && ident->name == core::Names::singleton()) {
            ENFORCE(ident->kind == ast::UnresolvedIdent::Kind::Class);
            klass->symbol = ctx.owner.data(ctx)->enclosingClass(ctx).data(ctx)->lookupSingletonClass(ctx);
            ENFORCE(klass->symbol.exists());
        } else {
            if (klass->symbol == core::Symbols::todo()) {
                klass->symbol = squashNames(ctx, ctx.owner.data(ctx)->enclosingClass(ctx), klass->name);
            } else {
                // Desugar populates a top-level root() ClassDef.
                // Nothing else should have been typeAlias by now.
                ENFORCE(klass->symbol == core::Symbols::root());
            }
            if (!klass->symbol.data(ctx)->isClassOrModule()) {
                auto klassSymbol =
                    ctx.state.lookupClassSymbol(klass->symbol.data(ctx)->owner, klass->symbol.data(ctx)->name);
                ENFORCE(klassSymbol.exists());
                klass->symbol = klassSymbol;
            }
        }
        return klass;
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

    unique_ptr<ast::Expression> postTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> klass) {
        // NameDefiner should have forced this class's singleton class into existence.
        ENFORCE(klass->symbol.data(ctx)->lookupSingletonClass(ctx).exists());

        for (auto &exp : klass->rhs) {
            addAncestor(ctx, klass, exp);
        }

        if (!klass->ancestors.empty()) {
            /* Superclass is typeAlias in parent scope, mixins are typeAlias in inner scope */
            for (auto &anc : klass->ancestors) {
                if (!isValidAncestor(anc.get())) {
                    if (auto e = ctx.beginError(anc->loc, core::errors::Namer::AncestorNotConstant)) {
                        e.setHeader("Superclasses must only contain constant literals");
                    }
                    anc = ast::MK::EmptyTree();
                }
            }
        }
        ast::InsSeq::STATS_store ideSeqs;
        if (ast::isa_tree<ast::ConstantLit>(klass->name.get())) {
            ideSeqs.emplace_back(ast::MK::KeepForIDE(klass->name->deepCopy()));
        }
        if (klass->kind == ast::ClassDef::Kind::Class && !klass->ancestors.empty() &&
            shouldLeaveAncestorForIDE(klass->ancestors.front())) {
            ideSeqs.emplace_back(ast::MK::KeepForIDE(klass->ancestors.front()->deepCopy()));
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

        ast::InsSeq::STATS_store retSeqs;
        auto loc = klass->declLoc;
        retSeqs.emplace_back(std::move(klass));
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
                auto localExpr = make_unique<ast::Local>(arg.loc, localVariable);
                args.emplace_back(move(localExpr));
            } else {
                ENFORCE(i < oldArgs.size());
                auto expr = arg2Symbol(ctx, i, move(arg), move(oldArgs[i]));
                args.emplace_back(move(expr));
            }
        }

        return args;
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> method) {
        core::SymbolRef owner = methodOwner(ctx);
        if (method->flags.isSelfMethod) {
            if (owner.data(ctx)->isClassOrModule()) {
                owner = owner.data(ctx)->lookupSingletonClass(ctx);
            }
        }
        ENFORCE(owner.exists());
        ENFORCE(owner.data(ctx)->isClassOrModule());

        auto parsedArgs = ast::ArgParsing::parseArgs(method->args);
        auto sym =
            ctx.state.lookupMethodSymbolWithHash(owner, method->name, ast::ArgParsing::hashArgs(ctx, parsedArgs));
        ENFORCE(sym.exists());
        method->symbol = sym;
        method->args = fillInArgs(ctx.withOwner(method->symbol), move(parsedArgs), std::move(method->args));
        return method;
    }

    unique_ptr<ast::MethodDef> postTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> method) {
        ENFORCE(method->args.size() == method->symbol.data(ctx)->arguments().size());
        ENFORCE(method->args.size() == method->symbol.data(ctx)->arguments().size(), "{}: {} != {}",
                method->name.showRaw(ctx), method->args.size(), method->symbol.data(ctx)->arguments().size());
        return method;
    }

    // Returns the SymbolRef corresponding to the class `self.class`, unless the
    // context is a class, in which case return it.
    core::SymbolRef contextClass(const core::GlobalState &gs, core::SymbolRef ofWhat) const {
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

    unique_ptr<ast::Assign> handleAssignment(core::Context ctx, unique_ptr<ast::Assign> asgn) {
        auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        ENFORCE(lhs);
        core::SymbolRef scope = squashNames(ctx, contextClass(ctx, ctx.owner), lhs->scope);
        if (!scope.data(ctx)->isClassOrModule()) {
            auto scopeName = scope.data(ctx)->name;
            scope = ctx.state.lookupClassSymbol(scope.data(ctx)->owner, scopeName);
        }

        core::SymbolRef cnst = ctx.state.lookupStaticFieldSymbol(scope, lhs->cnst);
        if (!cnst.exists()) {
            ctx.state.tracer().debug("cnst doesn't exist: {} on {}", lhs->cnst.toString(ctx),
                                     scope.data(ctx)->name.toString(ctx));
        }
        ENFORCE(cnst.exists());
        auto loc = lhs->loc;
        unique_ptr<ast::UnresolvedConstantLit> lhsU(lhs);
        asgn->lhs.release();
        asgn->lhs = make_unique<ast::ConstantLit>(loc, cnst, std::move(lhsU));
        return asgn;
    }

    unique_ptr<ast::Expression> handleTypeMemberDefinition(core::Context ctx, const ast::Send *send,
                                                           unique_ptr<ast::Assign> asgn,
                                                           const ast::UnresolvedConstantLit *typeName) {
        ENFORCE(asgn->lhs.get() == typeName &&
                asgn->rhs.get() == send); // this method assumes that `asgn` owns `send` and `typeName`
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
            auto send = ast::MK::Send0Block(asgn->loc, ast::MK::T(asgn->loc), core::Names::typeAlias(),
                                            ast::MK::Block0(asgn->loc, ast::MK::Untyped(asgn->loc)));

            return handleAssignment(ctx, make_unique<ast::Assign>(asgn->loc, std::move(asgn->lhs), std::move(send)));
        }

        bool isTypeTemplate = send->fun == core::Names::typeTemplate();
        auto onSymbol = isTypeTemplate ? ctx.owner.data(ctx)->lookupSingletonClass(ctx) : ctx.owner;
        ENFORCE(onSymbol.exists());
        if (send->args.size() > 2) {
            auto send =
                ast::MK::Send1(asgn->loc, ast::MK::T(asgn->loc), core::Names::typeAlias(), ast::MK::Untyped(asgn->loc));
            return handleAssignment(ctx, make_unique<ast::Assign>(asgn->loc, std::move(asgn->lhs), std::move(send)));
        }

        core::SymbolRef sym = ctx.state.lookupTypeMemberSymbol(onSymbol, typeName->cnst);
        ENFORCE(sym.exists());

        if (!send->args.empty()) {
            auto *hash = ast::cast_tree<ast::Hash>(send->args.back().get());
            if (hash) {
                bool fixed = false;
                bool bounded = false;

                for (auto &keyExpr : hash->keys) {
                    auto key = ast::cast_tree<ast::Literal>(keyExpr.get());
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
                    asgn->lhs = ast::MK::Constant(asgn->lhs->loc, sym);

                    // Leave it in the tree for the resolver to chew on.
                    return asgn;
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
        }

        return asgn;
    }

    unique_ptr<ast::Expression> postTransformAssign(core::Context ctx, unique_ptr<ast::Assign> asgn) {
        auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        if (lhs == nullptr) {
            return asgn;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn->rhs.get());
        if (send == nullptr) {
            return handleAssignment(ctx, std::move(asgn));
        }

        if (!send->recv->isSelfReference()) {
            return handleAssignment(ctx, std::move(asgn));
        }

        switch (send->fun._id) {
            case core::Names::typeTemplate()._id:
                return handleTypeMemberDefinition(ctx, send, std::move(asgn), lhs);
            case core::Names::typeMember()._id:
                return handleTypeMemberDefinition(ctx, send, std::move(asgn), lhs);
            default:
                return handleAssignment(ctx, std::move(asgn));
        }
    }

private:
    UnorderedMap<core::SymbolRef, core::Loc> classBehaviorLocs;
};

vector<NameFinderResult> findNames(const core::GlobalState &gs, vector<ast::ParsedFile> trees, WorkerPool &workers) {
    Timer timeit(gs.tracer(), "naming.findNames");
    auto resultq = make_shared<BlockingBoundedQueue<vector<NameFinderResult>>>(trees.size());
    auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
    vector<NameFinderResult> allFoundNames;
    allFoundNames.reserve(trees.size());
    for (auto &tree : trees) {
        fileq->push(move(tree), 1);
    }

    workers.multiplexJob("findNames", [gs, fileq, resultq]() {
        Timer timeit(gs.tracer(), "naming.findNamesWorker");
        NameFinder finder;
        vector<NameFinderResult> output;
        ast::ParsedFile job;
        for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
            if (result.gotItem()) {
                ctx.state.tracer().debug("findNames: {}", job.file.data(ctx).path());
                core::Context ctx(gs, core::Symbols::root(), job.file);
                job.tree = ast::TreeMap::apply(ctx, finder, std::move(job.tree));
                NameFinderResult jobOutput{move(job), finder.getAndClearFoundNames()};
                output.emplace_back(move(jobOutput));
            }
        }
        if (!output.empty()) {
            resultq->push(move(output), output.size());
        }
    });
    trees.clear();

    {
        vector<NameFinderResult> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), ctx.state.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), ctx.state.tracer())) {
            if (result.gotItem()) {
                allFoundNames.insert(allFoundNames.end(), make_move_iterator(threadResult.begin()),
                                     make_move_iterator(threadResult.end()));
            }
        }
    }
    fast_sort(allFoundNames,
              [](const auto &lhs, const auto &rhs) -> bool { return lhs.tree.file.id() < rhs.tree.file.id(); });
    return allFoundNames;
}

vector<ast::ParsedFile> defineNames(core::GlobalState &gs, vector<NameFinderResult> allFoundNames) {
    Timer timeit(gs.tracer(), "naming.defineNames");
    vector<ast::ParsedFile> output;
    output.reserve(allFoundNames.size());
    for (auto &fileFoundNames : allFoundNames) {
        ctx.state.tracer().debug("defineNames: {}", fileFoundNames.tree.file.data(ctx).path());
        core::MutableContext ctx(gs, core::Symbols::root(), fileFoundNames.tree.file);
        NameDefiner nameDefiner(move(fileFoundNames.names));
        output.push_back(move(fileFoundNames.tree));
        nameDefiner.run(ctx);
    }
    return output;
}

vector<ast::ParsedFile> insertNames(const core::GlobalStare &gs, vector<ast::ParsedFile> trees, WorkerPool &workers) {
    Timer timeit(gs.tracer(), "naming.insertNames");
    auto resultq = make_shared<BlockingBoundedQueue<vector<ast::ParsedFile>>>(trees.size());
    auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
    for (auto &tree : trees) {
        fileq->push(move(tree), 1);
    }

    workers.multiplexJob("insertNames", [gs, fileq, resultq]() {
        Timer timeit(ctx.state.tracer(), "naming.insertNamesWorker");
        NameInserter inserter;
        vector<ast::ParsedFile> output;
        ast::ParsedFile job;
        for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
            if (result.gotItem()) {
                ctx.state.tracer().debug("insertNames: {}", job.file.data(ctx).path());
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
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), ctx.state.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), ctx.state.tracer())) {
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

vector<ast::ParsedFile> Namer::run(core::GlobalState &gs, vector<ast::ParsedFile> trees, WorkerPool &workers) {
    auto foundNames = findNames(gs, move(trees), workers);
    trees = defineNames(gs, move(foundNames));
    trees = insertNames(gs, move(trees), workers);
    return trees;
}

}; // namespace sorbet::namer
