#include "core/Symbols.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "common/JSON.h"
#include "common/Levenstein.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Types.h"
#include "core/errors/internal.h"
#include "core/hashing/hashing.h"
#include <string>

template class std::vector<sorbet::core::TypeAndOrigins>;
template class std::vector<std::pair<sorbet::core::NameRef, sorbet::core::SymbolRef>>;
template class std::vector<const sorbet::core::ClassOrModule *>;

namespace sorbet::core {

using namespace std;

const int Symbols::MAX_SYNTHETIC_CLASS_SYMBOLS = 212;
const int Symbols::MAX_SYNTHETIC_METHOD_SYMBOLS = 50;
const int Symbols::MAX_SYNTHETIC_FIELD_SYMBOLS = 4;
const int Symbols::MAX_SYNTHETIC_TYPEARGUMENT_SYMBOLS = 4;
const int Symbols::MAX_SYNTHETIC_TYPEMEMBER_SYMBOLS = 72;

namespace {
constexpr string_view COLON_SEPARATOR = "::"sv;
constexpr string_view HASH_SEPARATOR = "#"sv;

string showInternal(const GlobalState &gs, core::SymbolRef owner, core::NameRef name, string_view separator) {
    if (!owner.exists() || owner == Symbols::root() || owner == core::Symbols::PackageSpecRegistry()) {
        return name.show(gs);
    }
    return absl::StrCat(owner.show(gs), separator, name.show(gs));
}
} // namespace

bool SymbolRef::operator==(const SymbolRef &rhs) const {
    return _id == rhs._id;
}

bool SymbolRef::operator!=(const SymbolRef &rhs) const {
    return !(rhs == *this);
}

bool ClassOrModuleRef::operator==(const ClassOrModuleRef &rhs) const {
    return rhs._id == this->_id;
}

bool ClassOrModuleRef::operator!=(const ClassOrModuleRef &rhs) const {
    return rhs._id != this->_id;
}

bool MethodRef::operator==(const MethodRef &rhs) const {
    return rhs._id == this->_id;
}

bool MethodRef::operator!=(const MethodRef &rhs) const {
    return rhs._id != this->_id;
}

bool FieldRef::operator==(const FieldRef &rhs) const {
    return rhs._id == this->_id;
}

bool FieldRef::operator!=(const FieldRef &rhs) const {
    return rhs._id != this->_id;
}

bool TypeMemberRef::operator==(const TypeMemberRef &rhs) const {
    return rhs._id == this->_id;
}

bool TypeMemberRef::operator!=(const TypeMemberRef &rhs) const {
    return rhs._id != this->_id;
}

bool TypeArgumentRef::operator==(const TypeArgumentRef &rhs) const {
    return rhs._id == this->_id;
}

bool TypeArgumentRef::operator!=(const TypeArgumentRef &rhs) const {
    return rhs._id != this->_id;
}

vector<TypePtr> ClassOrModule::selfTypeArgs(const GlobalState &gs) const {
    vector<TypePtr> targs;
    for (auto tm : typeMembers()) {
        auto tmData = tm.data(gs);
        if (tmData->flags.isFixed) {
            auto *lambdaParam = cast_type<LambdaParam>(tmData->resultType);
            ENFORCE(lambdaParam != nullptr);
            targs.emplace_back(lambdaParam->upperBound);
        } else {
            auto selfType = core::SymbolRef(tm);
            targs.emplace_back(make_type<SelfTypeParam>(selfType));
        }
    }
    return targs;
}
TypePtr ClassOrModule::selfType(const GlobalState &gs) const {
    // todo: in dotty it made sense to cache those.
    if (typeMembers().empty()) {
        return externalType();
    } else {
        return make_type<AppliedType>(ref(gs), selfTypeArgs(gs));
    }
}

TypePtr ClassOrModule::externalType() const {
    ENFORCE_NO_TIMER(resultType);
    if (resultType == nullptr) {
        // Don't return nullptr in prod builds, which would cause a disruptive crash
        // Emit a metric and return untyped instead.
        prodCounterInc("symbol.externalType.nullptr");
        return Types::untypedUntracked();
    }
    return resultType;
}

TypePtr ClassOrModule::unsafeComputeExternalType(GlobalState &gs) {
    if (resultType != nullptr) {
        return resultType;
    }

    // note that sometimes resultType is set externally to not be a result of this computation
    // this happens e.g. in case this is a stub class
    auto ref = this->ref(gs);
    if (typeMembers().empty()) {
        resultType = make_type<ClassType>(ref);
    } else {
        vector<TypePtr> targs;
        targs.reserve(typeMembers().size());

        for (auto &tm : typeMembers()) {
            auto tmData = tm.data(gs);
            auto *lambdaParam = cast_type<LambdaParam>(tmData->resultType);
            ENFORCE(lambdaParam != nullptr);

            if (ref.isLegacyStdlibGeneric()) {
                // Instantiate certain covariant stdlib generics with T.untyped, instead of <top>
                targs.emplace_back(Types::untyped(gs, ref));
            } else if (tmData->flags.isFixed || tmData->flags.isCovariant) {
                // Default fixed or covariant parameters to their upper bound.
                targs.emplace_back(lambdaParam->upperBound);
            } else if (tmData->flags.isInvariant) {
                // We instantiate Invariant type members as T.untyped as this will behave a bit like
                // a unification variable with Types::glb.
                targs.emplace_back(Types::untyped(gs, ref));
            } else {
                // The remaining case is a contravariant parameter, which gets defaulted to its lower bound.
                targs.emplace_back(lambdaParam->lowerBound);
            }
        }

        resultType = make_type<AppliedType>(ref, std::move(targs));
    }
    return resultType;
}

bool ClassOrModule::derivesFrom(const GlobalState &gs, ClassOrModuleRef sym) const {
    if (flags.isLinearizationComputed) {
        for (ClassOrModuleRef a : mixins()) {
            if (a == sym) {
                return true;
            }
        }
    } else {
        for (ClassOrModuleRef a : mixins()) {
            if (a == sym || a.data(gs)->derivesFrom(gs, sym)) {
                return true;
            }
        }
    }
    if (this->superClass().exists()) {
        return sym == this->superClass() || this->superClass().data(gs)->derivesFrom(gs, sym);
    }
    return false;
}

ClassOrModuleRef ClassOrModule::ref(const GlobalState &gs) const {
    uint32_t distance = this - gs.classAndModules.data();
    return ClassOrModuleRef(gs, distance);
}

FieldRef Field::ref(const GlobalState &gs) const {
    uint32_t distance = this - gs.fields.data();
    return FieldRef(gs, distance);
}

MethodRef Method::ref(const GlobalState &gs) const {
    uint32_t distance = this - gs.methods.data();
    return MethodRef(gs, distance);
}

SymbolRef TypeParameter::ref(const GlobalState &gs) const {
    if (flags.isTypeArgument) {
        uint32_t distance = this - gs.typeArguments.data();
        return TypeArgumentRef(gs, distance);
    } else {
        ENFORCE_NO_TIMER(flags.isTypeMember);
        uint32_t distance = this - gs.typeMembers.data();
        return TypeMemberRef(gs, distance);
    }
}

bool SymbolRef::isTypeAlias(const GlobalState &gs) const {
    return isFieldOrStaticField() && asFieldRef().dataAllowingNone(gs)->flags.isStaticFieldTypeAlias;
}

bool SymbolRef::isField(const GlobalState &gs) const {
    return isFieldOrStaticField() && asFieldRef().dataAllowingNone(gs)->flags.isField;
}

bool SymbolRef::isStaticField(const GlobalState &gs) const {
    return isFieldOrStaticField() && asFieldRef().dataAllowingNone(gs)->flags.isStaticField;
}

bool SymbolRef::isClassAlias(const GlobalState &gs) const {
    if (!isFieldOrStaticField()) {
        return false;
    }

    const auto &data = asFieldRef().dataAllowingNone(gs);
    if (!data->flags.isStaticField) {
        return false;
    }

    return data->isClassAlias();
}

ClassOrModuleData ClassOrModuleRef::dataAllowingNone(GlobalState &gs) const {
    ENFORCE_NO_TIMER(_id < gs.classAndModulesUsed());
    return ClassOrModuleData(gs.classAndModules[_id], gs);
}

ClassOrModuleData ClassOrModuleRef::data(GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    return dataAllowingNone(gs);
}

ConstClassOrModuleData ClassOrModuleRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    return dataAllowingNone(gs);
}

ConstClassOrModuleData ClassOrModuleRef::dataAllowingNone(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(_id < gs.classAndModulesUsed());
    return ConstClassOrModuleData(gs.classAndModules[_id], gs);
}

MethodData MethodRef::data(GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.methodsUsed());
    return MethodData(gs.methods[_id], gs);
}

ConstMethodData MethodRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.methodsUsed());
    return ConstMethodData(gs.methods[_id], gs);
}

MethodData MethodRef::dataAllowingNone(GlobalState &gs) const {
    ENFORCE_NO_TIMER(_id < gs.methodsUsed());
    return MethodData(gs.methods[_id], gs);
}

FieldData FieldRef::data(GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.fieldsUsed());
    return FieldData(gs.fields[_id], gs);
}

ConstFieldData FieldRef::dataAllowingNone(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(_id < gs.fieldsUsed());
    return ConstFieldData(gs.fields[_id], gs);
}

ConstFieldData FieldRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    return dataAllowingNone(gs);
}

FieldData FieldRef::dataAllowingNone(GlobalState &gs) const {
    ENFORCE_NO_TIMER(_id < gs.fieldsUsed());
    return FieldData(gs.fields[_id], gs);
}

TypeParameterData TypeMemberRef::data(GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.typeMembersUsed());
    return TypeParameterData(gs.typeMembers[_id], gs);
}

ConstTypeParameterData TypeMemberRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.typeMembersUsed());
    return ConstTypeParameterData(gs.typeMembers[_id], gs);
}

TypeParameterData TypeMemberRef::dataAllowingNone(GlobalState &gs) const {
    ENFORCE_NO_TIMER(_id < gs.typeMembersUsed());
    return TypeParameterData(gs.typeMembers[_id], gs);
}

TypeParameterData TypeArgumentRef::data(GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.typeArgumentsUsed());
    return TypeParameterData(gs.typeArguments[_id], gs);
}

ConstTypeParameterData TypeArgumentRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.typeArgumentsUsed());
    return ConstTypeParameterData(gs.typeArguments[_id], gs);
}

TypeParameterData TypeArgumentRef::dataAllowingNone(GlobalState &gs) const {
    ENFORCE_NO_TIMER(_id < gs.typeArgumentsUsed());
    return TypeParameterData(gs.typeArguments[_id], gs);
}

bool SymbolRef::isSynthetic() const {
    switch (this->kind()) {
        case Kind::ClassOrModule:
            return classOrModuleIndex() < Symbols::MAX_SYNTHETIC_CLASS_SYMBOLS;
        case Kind::Method:
            return methodIndex() < Symbols::MAX_SYNTHETIC_METHOD_SYMBOLS;
        case Kind::FieldOrStaticField:
            return fieldIndex() < Symbols::MAX_SYNTHETIC_FIELD_SYMBOLS;
        case Kind::TypeArgument:
            return typeArgumentIndex() < Symbols::MAX_SYNTHETIC_TYPEARGUMENT_SYMBOLS;
        case Kind::TypeMember:
            return typeMemberIndex() < Symbols::MAX_SYNTHETIC_TYPEMEMBER_SYMBOLS;
    }
}

void printTabs(fmt::memory_buffer &to, int count) {
    string ident(count * 2, ' ');
    fmt::format_to(std::back_inserter(to), "{}", ident);
}

SymbolRef::SymbolRef(const GlobalState &from, SymbolRef::Kind kind, uint32_t id)
    : _id((id << KIND_BITS) | static_cast<uint32_t>(kind)) {
    // If this fails, the symbol table is too big :(
    ENFORCE_NO_TIMER(id <= MAX_ID);
}
SymbolRef::SymbolRef(GlobalState const *from, SymbolRef::Kind kind, uint32_t id)
    : _id((id << KIND_BITS) | static_cast<uint32_t>(kind)) {
    // If this fails, the symbol table is too big :(
    ENFORCE_NO_TIMER(id <= MAX_ID);
}

SymbolRef::SymbolRef(ClassOrModuleRef kls) : SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, kls.id()) {}

SymbolRef::SymbolRef(MethodRef kls) : SymbolRef(nullptr, SymbolRef::Kind::Method, kls.id()) {}

SymbolRef::SymbolRef(FieldRef field) : SymbolRef(nullptr, SymbolRef::Kind::FieldOrStaticField, field.id()) {}

SymbolRef::SymbolRef(TypeMemberRef typeMember) : SymbolRef(nullptr, SymbolRef::Kind::TypeMember, typeMember.id()) {}

SymbolRef::SymbolRef(TypeArgumentRef typeArg) : SymbolRef(nullptr, SymbolRef::Kind::TypeArgument, typeArg.id()) {}

ClassOrModuleRef::ClassOrModuleRef(const GlobalState &from, uint32_t id) : _id(id) {}

MethodRef::MethodRef(const GlobalState &from, uint32_t id) : _id(id) {}

FieldRef::FieldRef(const GlobalState &from, uint32_t id) : _id(id) {}

TypeMemberRef::TypeMemberRef(const GlobalState &from, uint32_t id) : _id(id) {}

TypeArgumentRef::TypeArgumentRef(const GlobalState &from, uint32_t id) : _id(id) {}

string SymbolRef::show(const GlobalState &gs, ShowOptions options) const {
    switch (kind()) {
        case Kind::ClassOrModule:
            return asClassOrModuleRef().show(gs, options);
        case Kind::Method:
            return asMethodRef().show(gs, options);
        case Kind::FieldOrStaticField:
            return asFieldRef().show(gs, options);
        case Kind::TypeArgument:
            return asTypeArgumentRef().show(gs, options);
        case Kind::TypeMember:
            return asTypeMemberRef().show(gs, options);
    }
}

string ClassOrModuleRef::show(const GlobalState &gs, ShowOptions options) const {
    auto sym = dataAllowingNone(gs);
    if (sym->isSingletonClass(gs)) {
        auto attached = sym->attachedClass(gs);
        if (attached.exists()) {
            return fmt::format("T.class_of({})", attached.show(gs, options));
        }
    }

    // Make sure that we get nice error messages for things involving the proc sig builders.
    if (sym->name == core::Names::Constants::DeclBuilderForProcs()) {
        return "T.proc";
    }

    return showInternal(gs, sym->owner, sym->name, COLON_SEPARATOR);
}

string MethodRef::show(const GlobalState &gs, ShowOptions options) const {
    auto sym = data(gs);
    if (sym->owner.data(gs)->isSingletonClass(gs)) {
        return absl::StrCat(sym->owner.data(gs)->attachedClass(gs).show(gs, options), ".", sym->name.show(gs));
    }
    return showInternal(gs, sym->owner, sym->name, HASH_SEPARATOR);
}

string FieldRef::show(const GlobalState &gs, ShowOptions options) const {
    auto sym = dataAllowingNone(gs);
    return showInternal(gs, sym->owner, sym->name, sym->flags.isStaticField ? COLON_SEPARATOR : HASH_SEPARATOR);
}

string TypeArgumentRef::show(const GlobalState &gs, ShowOptions options) const {
    auto sym = data(gs);
    if (options.showForRBI) {
        return fmt::format("T.type_parameter(:{})", sym->name.show(gs));
    } else {
        return fmt::format("T.type_parameter(:{}) (of {})", sym->name.show(gs), sym->owner.show(gs));
    }
}

string TypeMemberRef::show(const GlobalState &gs, ShowOptions options) const {
    auto sym = data(gs);
    if (sym->name == core::Names::Constants::AttachedClass()) {
        auto owner = sym->owner.asClassOrModuleRef();
        auto attached = owner.data(gs)->attachedClass(gs);
        if (options.showForRBI || !attached.exists()) {
            // Attached wont exist for a number of cases:
            // - owner is a module that uses has_attached_class!
            // - owner is ::Class
            return "T.attached_class";
        }
        return fmt::format("T.attached_class (of {})", attached.show(gs, options));
    }
    if (options.showForRBI) {
        return sym->name.show(gs);
    }
    return showInternal(gs, sym->owner, sym->name, COLON_SEPARATOR);
}

TypePtr ArgInfo::argumentTypeAsSeenByImplementation(Context ctx, core::TypeConstraint &constr) const {
    auto owner = ctx.owner.asMethodRef();
    auto klass = owner.enclosingClass(ctx);
    auto instantiated = Types::resultTypeAsSeenFrom(ctx, type, klass, klass, klass.data(ctx)->selfTypeArgs(ctx));
    if (instantiated == nullptr) {
        instantiated = core::Types::untyped(ctx, owner);
    }
    if (owner.data(ctx)->flags.isGenericMethod) {
        instantiated = core::Types::instantiate(ctx, instantiated, constr);
    } else {
        // You might expect us to instantiate with the constr to be null for a non-generic method,
        // but you might have the constraint that is used to guess return type of
        // this method. It's not solved and you shouldn't try to instantiate types against itt
    }

    if (!flags.isRepeated) {
        return instantiated;
    }
    if (flags.isKeyword) {
        return Types::hashOf(ctx, instantiated);
    }
    return Types::arrayOf(ctx, instantiated);
}

void ClassOrModule::addMixinAt(ClassOrModuleRef sym, std::optional<uint16_t> index) {
    if (index.has_value()) {
        auto i = index.value();
        ENFORCE(mixins_.size() > i);
        ENFORCE(mixins_[i] == core::Symbols::PlaceholderMixin());
        mixins_[i] = sym;
    } else {
        mixins_.emplace_back(sym);
    }
}

bool ClassOrModule::addMixin(const GlobalState &gs, ClassOrModuleRef sym, std::optional<uint16_t> index) {
    // Note: Symbols without an explicit declaration may not have class or module set. They default to modules in
    // GlobalPass.cc. We also do not complain if the mixin is BasicObject.
    bool isValidMixin =
        !sym.data(gs)->isClassModuleSet() || sym.data(gs)->isModule() || sym == core::Symbols::BasicObject();

    if (!flags.isLinearizationComputed) {
        // Symbol hasn't been linearized yet, so add symbol unconditionally (order matters, so dupes are OK and
        // semantically important!)
        // This is the 99% common case.
        addMixinAt(sym, index);
    } else {
        // Symbol has been linearized, but we are trying to add another mixin. This is bad behavior and we shouldn't
        // allow it, but we currently allow it for the following circumstances:
        // * To support mixing in items to classes defined in payload, which have already been linearized.
        // * To support no-op addMixin during incrementalResolver, which shouldn't be introducing new mixins.
        //   * In other words, we expect the mixin to already be present.
        //   * incrementalResolver contains an ENFORCE that verifies that symbols haven't received new mixins via
        //   checking the linearization bit.

        // Ignore superclass (as in GlobalPass.cc's `computeClassLinearization`)
        if (sym != superClass() && absl::c_find(mixins_, sym) == mixins_.end()) {
            auto parent = superClass();
            // Don't include as mixin if it derives from the parent class (as in GlobalPass.cc's `maybeAddMixin`)
            if (!parent.exists() || !parent.data(gs)->derivesFrom(gs, sym)) {
                addMixinAt(sym, index);
                unsetClassOrModuleLinearizationComputed();
            }
        }
    }
    return isValidMixin;
}

uint16_t ClassOrModule::addMixinPlaceholder(const GlobalState &gs) {
    ENFORCE(ref(gs) != core::Symbols::PlaceholderMixin(), "Created a cycle through PlaceholderMixin");
    mixins_.emplace_back(core::Symbols::PlaceholderMixin());
    ENFORCE(mixins_.size() < numeric_limits<uint16_t>::max());
    return mixins_.size() - 1;
}

SymbolRef ClassOrModule::findMember(const GlobalState &gs, NameRef name) const {
    auto ret = findMemberNoDealias(gs, name);
    if (ret.exists()) {
        return ret.dealias(gs);
    }
    return ret;
}

MethodRef ClassOrModule::findMethod(const GlobalState &gs, NameRef name) const {
    auto sym = findMember(gs, name);
    if (sym.exists() && sym.isMethod()) {
        return sym.asMethodRef();
    }
    return Symbols::noMethod();
}

SymbolRef ClassOrModule::findMemberNoDealias(const GlobalState &gs, NameRef name) const {
    histogramInc("find_member_scope_size", members().size());
    auto fnd = members().find(name);
    if (fnd == members().end()) {
        return Symbols::noSymbol();
    }
    return fnd->second;
}

MethodRef ClassOrModule::findMethodNoDealias(const GlobalState &gs, NameRef name) const {
    auto sym = findMemberNoDealias(gs, name);
    if (!sym.isMethod()) {
        return Symbols::noMethod();
    }
    return sym.asMethodRef();
}

SymbolRef ClassOrModule::findMemberTransitive(const GlobalState &gs, NameRef name) const {
    auto dealias = true;
    return findMemberTransitiveInternal(gs, name, 100, dealias);
}

SymbolRef ClassOrModule::findMemberTransitiveNoDealias(const GlobalState &gs, NameRef name) const {
    auto dealias = false;
    return findMemberTransitiveInternal(gs, name, 100, dealias);
}

MethodRef ClassOrModule::findMethodTransitive(const GlobalState &gs, NameRef name) const {
    auto sym = findMemberTransitive(gs, name);
    if (sym.exists() && sym.isMethod()) {
        return sym.asMethodRef();
    }
    return Symbols::noMethod();
}

bool singleFileDefinition(const GlobalState &gs, const core::SymbolRef::LOC_store &locs, core::FileRef file) {
    bool result = false;

    for (auto &loc : locs) {
        if (loc.file().data(gs).isRBI()) {
            continue;
        }

        if (loc.file() != file) {
            return false;
        }

        result = true;
    }

    return result;
}

// Returns true if the given symbol is only defined in a given file (not accounting for RBIs).
bool SymbolRef::isOnlyDefinedInFile(const GlobalState &gs, core::FileRef file) const {
    if (file.data(gs).isRBI()) {
        return false;
    }

    return singleFileDefinition(gs, locs(gs), file);
}

// Returns true if the given class/module is only defined in a given file (not accounting for RBIs).
bool ClassOrModuleRef::isOnlyDefinedInFile(const GlobalState &gs, core::FileRef file) const {
    if (file.data(gs).isRBI()) {
        return false;
    }

    return singleFileDefinition(gs, data(gs)->locs(), file);
}

// Documented in SymbolRef.h
bool ClassOrModuleRef::isPackageSpecSymbol(const GlobalState &gs) const {
    auto sym = *this;
    while (sym.exists() && sym != core::Symbols::root()) {
        if (sym == core::Symbols::PackageSpecRegistry()) {
            return true;
        }

        sym = sym.data(gs)->owner;
    }

    return false;
}

bool ClassOrModuleRef::isBuiltinGenericForwarder() const {
    return *this == Symbols::T_Hash() || *this == Symbols::T_Array() || *this == Symbols::T_Set() ||
           *this == Symbols::T_Range() || *this == Symbols::T_Class() || *this == Symbols::T_Enumerable() ||
           *this == Symbols::T_Enumerator() || *this == Symbols::T_Enumerator_Lazy() ||
           *this == Symbols::T_Enumerator_Chain();
}

ClassOrModuleRef ClassOrModuleRef::maybeUnwrapBuiltinGenericForwarder() const {
    if (*this == Symbols::T_Array()) {
        return Symbols::Array();
    } else if (*this == Symbols::T_Hash()) {
        return Symbols::Hash();
    } else if (*this == Symbols::T_Enumerable()) {
        return Symbols::Enumerable();
    } else if (*this == Symbols::T_Enumerator()) {
        return Symbols::Enumerator();
    } else if (*this == Symbols::T_Enumerator_Lazy()) {
        return Symbols::Enumerator_Lazy();
    } else if (*this == Symbols::T_Enumerator_Chain()) {
        return Symbols::Enumerator_Chain();
    } else if (*this == Symbols::T_Range()) {
        return Symbols::Range();
    } else if (*this == Symbols::T_Set()) {
        return Symbols::Set();
    } else if (*this == Symbols::T_Class()) {
        return Symbols::Class();
    } else {
        return *this;
    }
}

ClassOrModuleRef ClassOrModuleRef::forwarderForBuiltinGeneric() const {
    if (*this == Symbols::Array()) {
        return Symbols::T_Array();
    } else if (*this == Symbols::Hash()) {
        return Symbols::T_Hash();
    } else if (*this == Symbols::Enumerable()) {
        return Symbols::T_Enumerable();
    } else if (*this == Symbols::Enumerator()) {
        return Symbols::T_Enumerator();
    } else if (*this == Symbols::Enumerator_Lazy()) {
        return Symbols::T_Enumerator_Lazy();
    } else if (*this == Symbols::Enumerator_Chain()) {
        return Symbols::T_Enumerator_Chain();
    } else if (*this == Symbols::Range()) {
        return Symbols::T_Range();
    } else if (*this == Symbols::Set()) {
        return Symbols::T_Set();
    } else if (*this == Symbols::Class()) {
        return Symbols::T_Class();
    } else {
        return Symbols::noClassOrModule();
    }
}

// See the comment in the header.
// !! The set of stdlib classes receiving this special behavior should NOT grow over time !!
bool ClassOrModuleRef::isLegacyStdlibGeneric() const {
    return *this == Symbols::Hash() || *this == Symbols::Array() || *this == Symbols::Set() ||
           *this == Symbols::Range() || *this == Symbols::Enumerable() || *this == Symbols::Enumerator() ||
           *this == Symbols::Enumerator_Lazy() || *this == Symbols::Enumerator_Chain();
}

namespace {
MethodRef findConcreteMethodTransitiveInternal(const GlobalState &gs, ClassOrModuleRef owner, NameRef name,
                                               int maxDepth) {
    // We can support it before linearization but it's more code to do so.
    ENFORCE(owner.data(gs)->flags.isLinearizationComputed);

    if (maxDepth == 0) {
        if (auto e = gs.beginError(Loc::none(), errors::Internal::InternalError)) {
            e.setHeader("findConcreteMethodTransitive hit a loop while resolving `{}` in `{}`. Parents are: ",
                        name.show(gs), owner.showFullName(gs));
        }
        int i = -1;
        for (auto it = owner.data(gs)->mixins().rbegin(); it != owner.data(gs)->mixins().rend(); ++it) {
            i++;
            if (auto e = gs.beginError(Loc::none(), errors::Internal::InternalError)) {
                e.setHeader("`{}`:- `{}`", i, it->showFullName(gs));
            }
            int j = 0;
            for (auto it2 = it->data(gs)->mixins().rbegin(); it2 != it->data(gs)->mixins().rend(); ++it2) {
                if (auto e = gs.beginError(Loc::none(), errors::Internal::InternalError)) {
                    e.setHeader("`{}`:`{}` `{}`", i, j, it2->showFullName(gs));
                }
                j++;
            }
        }

        Exception::raise("findConcreteMethodTransitive hit a loop while resolving");
    }

    MethodRef result = owner.data(gs)->findMethod(gs, name);
    if (result.exists() && !result.data(gs)->flags.isAbstract) {
        return result;
    }

    for (auto it = owner.data(gs)->mixins().begin(); it != owner.data(gs)->mixins().end(); ++it) {
        ENFORCE(it->exists());
        result = it->data(gs)->findMethod(gs, name);
        if (result.exists() && !result.data(gs)->flags.isAbstract) {
            return result;
        }
    }

    if (owner.data(gs)->superClass().exists()) {
        return findConcreteMethodTransitiveInternal(gs, owner.data(gs)->superClass(), name, maxDepth - 1);
    }

    return Symbols::noMethod();
}
} // namespace

MethodRef ClassOrModule::findConcreteMethodTransitive(const GlobalState &gs, NameRef name) const {
    return findConcreteMethodTransitiveInternal(gs, this->ref(gs), name, 100);
}

SymbolRef ClassOrModule::findMemberTransitiveInternal(const GlobalState &gs, NameRef name, int maxDepth,
                                                      bool dealias) const {
    if (maxDepth == 0) {
        if (auto e = gs.beginError(Loc::none(), errors::Internal::InternalError)) {
            e.setHeader("findMemberTransitive hit a loop while resolving `{}` in `{}`. Parents are: ", name.show(gs),
                        ref(gs).showFullName(gs));
        }
        int i = -1;
        for (auto it = this->mixins().rbegin(); it != this->mixins().rend(); ++it) {
            i++;
            if (auto e = gs.beginError(Loc::none(), errors::Internal::InternalError)) {
                e.setHeader("`{}`:- `{}`", i, it->showFullName(gs));
            }
            int j = 0;
            for (auto it2 = it->data(gs)->mixins().rbegin(); it2 != it->data(gs)->mixins().rend(); ++it2) {
                if (auto e = gs.beginError(Loc::none(), errors::Internal::InternalError)) {
                    e.setHeader("`{}`:`{}` `{}`", i, j, it2->showFullName(gs));
                }
                j++;
            }
        }

        Exception::raise("findMemberTransitive hit a loop while resolving");
    }

    SymbolRef result = dealias ? findMember(gs, name) : findMemberNoDealias(gs, name);
    if (result.exists()) {
        return result;
    }
    if (flags.isLinearizationComputed) {
        for (auto it = this->mixins().begin(); it != this->mixins().end(); ++it) {
            ENFORCE(it->exists());
            result = dealias ? it->data(gs)->findMember(gs, name) : it->data(gs)->findMemberNoDealias(gs, name);
            if (result.exists()) {
                return result;
            }
            result = core::Symbols::noSymbol();
        }
    } else {
        for (auto it = this->mixins().rbegin(); it != this->mixins().rend(); ++it) {
            ENFORCE(it->exists());
            result = it->data(gs)->findMemberTransitiveInternal(gs, name, maxDepth - 1, dealias);
            if (result.exists()) {
                return result;
            }
        }
    }
    if (this->superClass().exists()) {
        return this->superClass().data(gs)->findMemberTransitiveInternal(gs, name, maxDepth - 1, dealias);
    }
    return Symbols::noSymbol();
}

vector<ClassOrModule::FuzzySearchResult> ClassOrModule::findMemberFuzzyMatch(const GlobalState &gs, NameRef name,
                                                                             int betterThan) const {
    vector<ClassOrModule::FuzzySearchResult> res;
    // Don't run under the fuzzer, as otherwise fuzzy match dominates runtime.
    // N.B.: There are benefits to running this method under the fuzzer; we have found bugs in this method before
    // via fuzzing (e.g. https://github.com/sorbet/sorbet/issues/128).
    if (fuzz_mode) {
        return res;
    }

    if (name.kind() == NameKind::UTF8) {
        auto sym = findMemberFuzzyMatchUTF8(gs, name, betterThan);
        if (sym.symbol.exists()) {
            res.emplace_back(sym);
        } else {
            // For the error when you use a instance method but wanted the
            // singleton one
            auto singleton = lookupSingletonClass(gs);
            if (singleton.exists()) {
                sym = singleton.data(gs)->findMemberFuzzyMatchUTF8(gs, name, betterThan);
                if (sym.symbol.exists()) {
                    res.emplace_back(sym);
                }
            } else {
                // For the error when you use a singleton method but wanted the
                // instance one
                auto attached = attachedClass(gs);
                sym = attached.data(gs)->findMemberFuzzyMatchUTF8(gs, name, betterThan);
                if (sym.symbol.exists()) {
                    res.emplace_back(sym);
                }
            }
        }
        auto shortName = name.shortName(gs);
        if (!shortName.empty() && std::isupper(shortName.front())) {
            vector<ClassOrModule::FuzzySearchResult> constant_matches =
                findMemberFuzzyMatchConstant(gs, name, betterThan);
            res.insert(res.end(), constant_matches.begin(), constant_matches.end());
        }
    } else if (name.kind() == NameKind::CONSTANT) {
        res = findMemberFuzzyMatchConstant(gs, name, betterThan);
    }
    return res;
}

bool SymbolRef::isUnderNamespace(const GlobalState &gs, ClassOrModuleRef otherClass) const {
    if (isClassOrModule() && otherClass == asClassOrModuleRef()) {
        return true;
    }

    // if we are checking for nesting under root itself, which is always true
    if (otherClass == core::Symbols::root()) {
        return true;
    }

    auto curOwner = owner(gs).asClassOrModuleRef();
    while (curOwner != core::Symbols::root()) {
        if (curOwner == otherClass) {
            return true;
        }

        curOwner = curOwner.data(gs)->owner;
    }

    return false;
}

vector<ClassOrModule::FuzzySearchResult>
ClassOrModule::findMemberFuzzyMatchConstant(const GlobalState &gs, NameRef name, int betterThan) const {
    // Performance of this method is bad, to say the least.
    // It's written under assumption that it's called rarely
    // and that it's worth spending a lot of time finding a good candidate in ALL scopes.
    // It may return multiple candidates:
    //   - best candidate per every outer scope if it's better than all the candidates in inner scope
    //   - globally best candidate in ALL scopes.
    vector<ClassOrModule::FuzzySearchResult> result;

    FuzzySearchResult best;
    best.symbol = Symbols::noSymbol();
    best.name = NameRef::noName();
    best.distance = betterThan;
    auto currentName = name.shortName(gs);
    if (best.distance < 0) {
        best.distance = 1 + (currentName.size() / 2);
    }

    bool onlySuggestPackageSpecs = ref(gs).isPackageSpecSymbol(gs);

    // Find the closest by following outer scopes
    {
        ClassOrModuleRef base = ref(gs);

        do {
            // follow outer scopes

            // find scopes that would be considered for search
            vector<ClassOrModuleRef> candidateScopes;
            vector<ClassOrModule::FuzzySearchResult> scopeBest;
            candidateScopes.emplace_back(base);
            int i = 0;
            // this is quadratic in number of scopes that we traverse, but YOLO, this should rarely run
            while (i < candidateScopes.size()) {
                const auto &sym = candidateScopes[i].data(gs);
                if (sym->superClass().exists()) {
                    if (!absl::c_linear_search(candidateScopes, sym->superClass())) {
                        candidateScopes.emplace_back(sym->superClass());
                    }
                }
                for (auto ancestor : sym->mixins()) {
                    if (!absl::c_linear_search(candidateScopes, ancestor)) {
                        candidateScopes.emplace_back(ancestor);
                    }
                }
                i++;
            }
            for (const auto scope : candidateScopes) {
                scopeBest.clear();
                for (auto member : scope.data(gs)->members()) {
                    if (member.first.kind() == NameKind::CONSTANT &&
                        member.first.dataCnst(gs)->original.kind() == NameKind::UTF8 && member.second.exists()) {
                        if (onlySuggestPackageSpecs) {
                            if (!member.second.isClassOrModule() ||
                                !member.second.asClassOrModuleRef().isPackageSpecSymbol(gs)) {
                                continue;
                            }
                        }

                        auto thisDistance = Levenstein::distance(
                            currentName, member.first.dataCnst(gs)->original.dataUtf8(gs)->utf8, best.distance);
                        if (thisDistance <= best.distance) {
                            if (thisDistance < best.distance) {
                                scopeBest.clear();
                            }
                            best.distance = thisDistance;
                            best.symbol = member.second;
                            best.name = member.first;
                            scopeBest.emplace_back(best);
                        }
                    }
                }
                if (!scopeBest.empty()) {
                    // NOTE: Iteration order over members is nondeterministic, so we use SymbolId to deterministically
                    // order the recommendations from this scope.
                    // We order in decreasing symbol ID order because `result` is later reversed and we want earlier
                    // ID'd symbols to be recommended first.
                    fast_sort(scopeBest, [&](const auto &lhs, const auto &rhs) -> bool {
                        return lhs.symbol._id > rhs.symbol._id;
                    });
                    for (auto &item : scopeBest) {
                        result.emplace_back(item);
                    }
                }
            }

            base = base.data(gs)->owner;
        } while (best.distance > 0 && base.data(gs)->owner.exists() && base != Symbols::root());
    }

    // At this point, `result` is in a deterministic order, and is ordered with _decreasing_ edit distance

    if (best.distance > 0) {
        // find the closest by global dfs.
        auto globalBestDistance = best.distance - 1;
        vector<ClassOrModule::FuzzySearchResult> globalBest;
        vector<ClassOrModuleRef> yetToGoDeeper;
        yetToGoDeeper.emplace_back(Symbols::root());
        while (!yetToGoDeeper.empty()) {
            const ClassOrModuleRef thisIter = yetToGoDeeper.back();
            yetToGoDeeper.pop_back();
            for (auto member : thisIter.data(gs)->members()) {
                if (member.second.exists() && member.first.exists() && member.first.kind() == NameKind::CONSTANT &&
                    member.first.dataCnst(gs)->original.kind() == NameKind::UTF8) {
                    if (member.second.isClassOrModule() &&
                        member.second.asClassOrModuleRef().data(gs)->derivesFrom(gs, core::Symbols::StubModule())) {
                        continue;
                    }
                    if (onlySuggestPackageSpecs) {
                        if (!member.second.isClassOrModule() ||
                            !member.second.asClassOrModuleRef().isPackageSpecSymbol(gs)) {
                            continue;
                        }
                    }

                    auto thisDistance = Levenstein::distance(
                        currentName, member.first.dataCnst(gs)->original.dataUtf8(gs)->utf8, best.distance);
                    if (thisDistance <= globalBestDistance) {
                        if (thisDistance < globalBestDistance) {
                            globalBest.clear();
                        }
                        globalBestDistance = thisDistance;
                        best.distance = thisDistance;
                        best.symbol = member.second;
                        best.name = member.first;
                        globalBest.emplace_back(best);
                    }
                    if (member.second.isClassOrModule()) {
                        yetToGoDeeper.emplace_back(member.second.asClassOrModuleRef());
                    }
                }
            }
        }
        // globalBest is nondeterministically ordered since iteration over `members()` is non deterministic.
        // Everything in globalBest has the same edit distance, so we just have to sort by symbol ID to get a
        // deterministic order.
        // We order in decreasing symbol ID order because `result` is later reversed and we want earlier
        // ID'd symbols to be recommended first.
        fast_sort(globalBest,
                  [&](const auto &lhs, const auto &rhs) -> bool { return lhs.symbol._id > rhs.symbol._id; });
        for (auto &e : globalBest) {
            result.emplace_back(e);
        }
    }

    // result is ordered in decreasing edit distance order. We want to flip the order so the items w/ the smallest edit
    // distance are first.
    absl::c_reverse(result);
    return result;
}

ClassOrModule::FuzzySearchResult ClassOrModule::findMemberFuzzyMatchUTF8(const GlobalState &gs, NameRef name,
                                                                         int betterThan) const {
    FuzzySearchResult result;
    result.symbol = Symbols::noSymbol();
    result.name = NameRef::noName();
    result.distance = betterThan;

    auto currentName = name.dataUtf8(gs)->utf8;
    if (result.distance < 0) {
        result.distance = 1 + (currentName.size() / 2);
    }

    for (auto pair : members()) {
        auto thisName = pair.first;
        if (thisName.kind() != NameKind::UTF8) {
            continue;
        }
        auto utf8 = thisName.dataUtf8(gs)->utf8;
        int thisDistance = Levenstein::distance(currentName, utf8, result.distance);
        if (thisDistance < result.distance ||
            (thisDistance == result.distance && result.symbol._id > pair.second._id)) {
            result.distance = thisDistance;
            result.name = thisName;
            result.symbol = pair.second;
        }
    }

    for (auto it = this->mixins().rbegin(); it != this->mixins().rend(); ++it) {
        ENFORCE(it->exists());

        auto subResult = it->data(gs)->findMemberFuzzyMatchUTF8(gs, name, result.distance);
        if (subResult.symbol.exists()) {
            ENFORCE(subResult.name.exists());
            ENFORCE(subResult.name.kind() == NameKind::UTF8);
            result = subResult;
        }
    }
    if (this->superClass().exists()) {
        auto subResult = this->superClass().data(gs)->findMemberFuzzyMatchUTF8(gs, name, result.distance);
        if (subResult.symbol.exists()) {
            ENFORCE(subResult.name.exists());
            ENFORCE(subResult.name.kind() == NameKind::UTF8);
            result = subResult;
        }
    }
    return result;
}

namespace {
bool isHiddenFromPrinting(const GlobalState &gs, SymbolRef symbol) {
    if (symbol.isSynthetic()) {
        return true;
    }
    if (symbol.locs(gs).empty()) {
        return true;
    }
    for (auto loc : symbol.locs(gs)) {
        if (loc.file().data(gs).sourceType == File::Type::Payload) {
            return true;
        }
    }
    return false;
}

void printLocs(const GlobalState &gs, fmt::memory_buffer &buf, const InlinedVector<Loc, 2> &locs, bool showRaw) {
    if (!locs.empty()) {
        fmt::format_to(std::back_inserter(buf), " @ ");
        if (locs.size() > 1) {
            fmt::format_to(std::back_inserter(buf), "(");
        }
        fmt::format_to(std::back_inserter(buf), "{}", fmt::map_join(locs, ", ", [&](auto loc) {
                           return showRaw ? loc.showRaw(gs) : loc.filePosToString(gs);
                       }));
        if (locs.size() > 1) {
            fmt::format_to(std::back_inserter(buf), ")");
        }
    }
}

string_view getVariance(ConstTypeParameterData &sym) {
    if (sym->flags.isCovariant) {
        return "(+)"sv;
    } else if (sym->flags.isContravariant) {
        return "(-)"sv;
    } else if (sym->flags.isInvariant) {
        return "(=)"sv;
    } else {
        Exception::raise("type without variance");
    }
}

void printResultType(const GlobalState &gs, fmt::memory_buffer &buf, const TypePtr &resultType, int tabs,
                     bool showRaw) {
    if (resultType) {
        string printed;
        if (showRaw) {
            printed = absl::StrReplaceAll(resultType.toStringWithTabs(gs, tabs), {{"\n", " "}});
        } else {
            printed = resultType.show(gs);
        }
        fmt::format_to(std::back_inserter(buf), " -> {}", printed);
    }
}

string showFullNameInternal(const GlobalState &gs, core::SymbolRef owner, core::NameRef name, string_view separator) {
    bool includeOwner = owner.exists() && owner != Symbols::root();
    string ownerStr = includeOwner ? owner.showFullName(gs) : "";
    return absl::StrCat(ownerStr, separator, name.show(gs));
}

string toStringFullNameInternal(const GlobalState &gs, core::SymbolRef owner, core::NameRef name,
                                string_view separator) {
    bool includeOwner = owner.exists() && owner != Symbols::root();
    string ownerStr = includeOwner ? owner.toStringFullName(gs) : "";
    return absl::StrCat(ownerStr, includeOwner ? separator : "", name.showRaw(gs));
}
} // namespace

string SymbolRef::showFullName(const GlobalState &gs) const {
    switch (kind()) {
        case Kind::ClassOrModule:
            return asClassOrModuleRef().showFullName(gs);
        case Kind::Method:
            return asMethodRef().showFullName(gs);
        case Kind::FieldOrStaticField:
            return asFieldRef().showFullName(gs);
        case Kind::TypeArgument:
            return asTypeArgumentRef().showFullName(gs);
        case Kind::TypeMember:
            return asTypeMemberRef().showFullName(gs);
    }
}

string ClassOrModuleRef::showFullName(const GlobalState &gs) const {
    auto sym = dataAllowingNone(gs);
    return showFullNameInternal(gs, sym->owner, sym->name, COLON_SEPARATOR);
}

string MethodRef::showFullName(const GlobalState &gs) const {
    auto sym = data(gs);
    return showFullNameInternal(gs, sym->owner, sym->name, HASH_SEPARATOR);
}

string FieldRef::showFullName(const GlobalState &gs) const {
    auto sym = data(gs);
    return showFullNameInternal(gs, sym->owner, sym->name, sym->flags.isStaticField ? COLON_SEPARATOR : HASH_SEPARATOR);
}

string TypeArgumentRef::showFullName(const GlobalState &gs) const {
    auto sym = data(gs);
    return showFullNameInternal(gs, sym->owner, sym->name, HASH_SEPARATOR);
}

string TypeMemberRef::showFullName(const GlobalState &gs) const {
    auto sym = data(gs);
    return showFullNameInternal(gs, sym->owner, sym->name, COLON_SEPARATOR);
}

string SymbolRef::toStringFullName(const GlobalState &gs) const {
    switch (kind()) {
        case Kind::ClassOrModule:
            return asClassOrModuleRef().toStringFullName(gs);
        case Kind::Method:
            return asMethodRef().toStringFullName(gs);
        case Kind::FieldOrStaticField:
            return asFieldRef().toStringFullName(gs);
        case Kind::TypeArgument:
            return asTypeArgumentRef().toStringFullName(gs);
        case Kind::TypeMember:
            return asTypeMemberRef().toStringFullName(gs);
    }
}

string ClassOrModuleRef::toStringFullName(const GlobalState &gs) const {
    auto sym = dataAllowingNone(gs);
    return toStringFullNameInternal(gs, sym->owner, sym->name, COLON_SEPARATOR);
}

string MethodRef::toStringFullName(const GlobalState &gs) const {
    auto sym = data(gs);
    return toStringFullNameInternal(gs, sym->owner, sym->name, HASH_SEPARATOR);
}

string FieldRef::toStringFullName(const GlobalState &gs) const {
    auto sym = data(gs);
    return toStringFullNameInternal(gs, sym->owner, sym->name,
                                    sym->flags.isStaticField ? COLON_SEPARATOR : HASH_SEPARATOR);
}

string TypeArgumentRef::toStringFullName(const GlobalState &gs) const {
    auto sym = data(gs);
    return toStringFullNameInternal(gs, sym->owner, sym->name, HASH_SEPARATOR);
}

string TypeMemberRef::toStringFullName(const GlobalState &gs) const {
    auto sym = data(gs);
    return toStringFullNameInternal(gs, sym->owner, sym->name, COLON_SEPARATOR);
}

bool ClassOrModule::isPrintable(const GlobalState &gs) const {
    if (!isHiddenFromPrinting(gs, this->ref(gs))) {
        return true;
    }

    for (auto childPair : this->members()) {
        if (childPair.first == Names::singleton() || childPair.first == Names::attached() ||
            childPair.first == Names::mixedInClassMethods()) {
            continue;
        }

        if (childPair.second.isPrintable(gs)) {
            return true;
        }
    }

    return false;
}

// TODO(jvilk): Hoist this to *Ref classes to remove use of ref().
bool Method::isPrintable(const GlobalState &gs) const {
    if (!isHiddenFromPrinting(gs, this->ref(gs))) {
        return true;
    }

    for (auto typeParam : this->typeArguments()) {
        if (typeParam.data(gs)->isPrintable(gs)) {
            return true;
        }
    }

    return false;
}

bool Field::isPrintable(const GlobalState &gs) const {
    return !isHiddenFromPrinting(gs, this->ref(gs));
}

bool TypeParameter::isPrintable(const GlobalState &gs) const {
    return !isHiddenFromPrinting(gs, this->ref(gs));
}

string_view SymbolRef::showKind(const GlobalState &gs) const {
    switch (this->kind()) {
        case Kind::ClassOrModule:
            return asClassOrModuleRef().showKind(gs);
        case Kind::FieldOrStaticField:
            return asFieldRef().showKind(gs);
        case Kind::Method:
            return asMethodRef().showKind(gs);
        case Kind::TypeArgument:
            return asTypeArgumentRef().showKind(gs);
        case Kind::TypeMember:
            return asTypeMemberRef().showKind(gs);
    }
}

string_view ClassOrModuleRef::showKind(const GlobalState &gs) const {
    auto sym = dataAllowingNone(gs);
    if (!sym->isClassModuleSet()) {
        return "class-or-module"sv;
    } else if (sym->isClass()) {
        return "class"sv;
    } else {
        return "module"sv;
    }
}

string_view FieldRef::showKind(const GlobalState &gs) const {
    auto sym = dataAllowingNone(gs);
    if (sym->flags.isStaticField) {
        if (sym->flags.isStaticFieldTypeAlias) {
            return "static-field-type-alias"sv;
        } else {
            return "static-field"sv;
        }
    }
    return "field"sv;
}

string_view MethodRef::showKind(const GlobalState &gs) const {
    return "method"sv;
}

string_view TypeMemberRef::showKind(const GlobalState &gs) const {
    return "type-member"sv;
}

string_view TypeArgumentRef::showKind(const GlobalState &gs) const {
    return "type-argument"sv;
}

string ClassOrModuleRef::toStringWithOptions(const GlobalState &gs, int tabs, bool showFull, bool showRaw) const {
    fmt::memory_buffer buf;

    printTabs(buf, tabs);

    auto sym = data(gs);

    fmt::format_to(std::back_inserter(buf), "{} {}", showKind(gs), showRaw ? toStringFullName(gs) : showFullName(gs));

    auto membersSpan = sym->typeMembers();
    InlinedVector<TypeMemberRef, 4> typeMembers;
    typeMembers.assign(membersSpan.begin(), membersSpan.end());
    auto it = remove_if(typeMembers.begin(), typeMembers.end(),
                        [&gs](auto &sym) -> bool { return sym.data(gs)->flags.isFixed; });
    typeMembers.erase(it, typeMembers.end());
    if (!typeMembers.empty()) {
        fmt::format_to(std::back_inserter(buf), "[{}]", fmt::map_join(typeMembers, ", ", [&](auto symb) {
                           auto name = symb.data(gs)->name;
                           return showRaw ? name.showRaw(gs) : name.show(gs);
                       }));
    }

    if (sym->superClass().exists()) {
        auto superClass = sym->superClass();
        fmt::format_to(std::back_inserter(buf), " < {}",
                       showRaw ? superClass.toStringFullName(gs) : superClass.showFullName(gs));
    }

    fmt::format_to(std::back_inserter(buf), " ({})", fmt::map_join(sym->mixins(), ", ", [&](auto symb) {
                       auto name = symb.data(gs)->name;
                       return showRaw ? name.showRaw(gs) : name.show(gs);
                   }));

    if (sym->flags.isPrivate) {
        fmt::format_to(std::back_inserter(buf), " : private");
    }
    // root should have no locs. We used to have special handling here to hide locs on root
    // when censorForSnapshotTests was passed, but it's not needed anymore.
    ENFORCE(!(*this == core::Symbols::root() && !sym->locs().empty()));

    printLocs(gs, buf, sym->locs(), showRaw);

    ENFORCE(!absl::c_any_of(to_string(buf), [](char c) { return c == '\n'; }));
    fmt::format_to(std::back_inserter(buf), "\n");
    for (auto pair : sym->membersStableOrderSlow(gs)) {
        if (!pair.second.exists()) {
            ENFORCE(*this == core::Symbols::root());
            continue;
        }

        if (pair.first == Names::singleton() || pair.first == Names::attached() ||
            pair.first == Names::mixedInClassMethods()) {
            continue;
        }

        if (!showFull && !pair.second.isPrintable(gs)) {
            continue;
        }

        auto str = pair.second.toStringWithOptions(gs, tabs + 1, showFull, showRaw);
        ENFORCE(!str.empty());
        fmt::format_to(std::back_inserter(buf), "{}", move(str));
    }

    return to_string(buf);
}

string MethodRef::toStringWithOptions(const GlobalState &gs, int tabs, bool showFull, bool showRaw) const {
    fmt::memory_buffer buf;

    printTabs(buf, tabs);

    auto sym = data(gs);

    fmt::format_to(std::back_inserter(buf), "{} {}", showKind(gs), showRaw ? toStringFullName(gs) : showFullName(gs));

    auto methodFlags = InlinedVector<string, 3>{};

    if (sym->flags.isPrivate) {
        methodFlags.emplace_back("private");
    } else if (sym->flags.isProtected) {
        methodFlags.emplace_back("protected");
    } else if (sym->flags.isPackagePrivate) {
        methodFlags.emplace_back("package_private");
    }

    if (sym->flags.isAbstract) {
        methodFlags.emplace_back("abstract");
    }
    if (sym->flags.isOverridable) {
        methodFlags.emplace_back("overridable");
    }
    if (sym->flags.isOverride) {
        methodFlags.emplace_back("override");
    }
    if (sym->flags.isIncompatibleOverride) {
        methodFlags.emplace_back("allow_incompatible");
    }
    if (sym->flags.isFinal) {
        methodFlags.emplace_back("final");
    }

    if (!methodFlags.empty()) {
        fmt::format_to(std::back_inserter(buf), " : {}",
                       fmt::map_join(methodFlags, "|", [](const auto &flag) { return flag; }));
    }

    InlinedVector<TypeArgumentRef, 4> typeMembers;
    typeMembers.assign(sym->typeArguments().begin(), sym->typeArguments().end());
    auto it = remove_if(typeMembers.begin(), typeMembers.end(),
                        [&gs](auto &sym) -> bool { return sym.data(gs)->flags.isFixed; });
    typeMembers.erase(it, typeMembers.end());
    if (!typeMembers.empty()) {
        fmt::format_to(std::back_inserter(buf), "[{}]", fmt::map_join(typeMembers, ", ", [&](auto symb) {
                           auto name = symb.data(gs)->name;
                           return showRaw ? name.showRaw(gs) : name.show(gs);
                       }));
    }
    fmt::format_to(std::back_inserter(buf), " ({})",
                   fmt::map_join(sym->arguments, ", ", [&](const auto &symb) { return symb.argumentName(gs); }));

    printResultType(gs, buf, sym->resultType, tabs, showRaw);
    printLocs(gs, buf, sym->locs(), showRaw);

    if (sym->rebind.exists()) {
        fmt::format_to(std::back_inserter(buf), " rebindTo {}",
                       showRaw ? sym->rebind.toStringFullName(gs) : sym->rebind.showFullName(gs));
    }

    ENFORCE(!absl::c_any_of(to_string(buf), [](char c) { return c == '\n'; }));
    fmt::format_to(std::back_inserter(buf), "\n");
    for (auto ta : sym->typeArguments()) {
        ENFORCE_NO_TIMER(ta.exists());

        if (!showFull && !ta.data(gs)->isPrintable(gs)) {
            continue;
        }

        auto str = ta.toStringWithOptions(gs, tabs + 1, showFull, showRaw);
        ENFORCE(!str.empty());
        fmt::format_to(std::back_inserter(buf), "{}", move(str));
    }

    for (auto &arg : sym->arguments) {
        auto str = arg.toString(gs);
        ENFORCE(!str.empty());
        printTabs(buf, tabs + 1);
        fmt::format_to(std::back_inserter(buf), "{}\n", move(str));
    }

    return to_string(buf);
}

string FieldRef::toStringWithOptions(const GlobalState &gs, int tabs, bool showFull, bool showRaw) const {
    fmt::memory_buffer buf;

    printTabs(buf, tabs);

    auto sym = data(gs);

    string_view type = showKind(gs);

    string_view access;
    if (sym->flags.isStaticField && sym->flags.isStaticFieldPrivate) {
        access = " : private"sv;
    }

    fmt::format_to(std::back_inserter(buf), "{} {}{}", type, showRaw ? toStringFullName(gs) : showFullName(gs), access);

    printResultType(gs, buf, sym->resultType, tabs, showRaw);
    printLocs(gs, buf, sym->locs(), showRaw);

    ENFORCE(!absl::c_any_of(to_string(buf), [](char c) { return c == '\n'; }));

    fmt::format_to(std::back_inserter(buf), "\n");

    return to_string(buf);
}

string TypeMemberRef::toStringWithOptions(const GlobalState &gs, int tabs, bool showFull, bool showRaw) const {
    fmt::memory_buffer buf;

    printTabs(buf, tabs);

    auto sym = data(gs);

    fmt::format_to(std::back_inserter(buf), "{}{} {}", showKind(gs), getVariance(sym),
                   showRaw ? toStringFullName(gs) : showFullName(gs));

    printResultType(gs, buf, sym->resultType, tabs, showRaw);
    printLocs(gs, buf, sym->locs(), showRaw);

    ENFORCE(!absl::c_any_of(to_string(buf), [](char c) { return c == '\n'; }));
    fmt::format_to(std::back_inserter(buf), "\n");

    return to_string(buf);
}

string TypeArgumentRef::toStringWithOptions(const GlobalState &gs, int tabs, bool showFull, bool showRaw) const {
    fmt::memory_buffer buf;

    printTabs(buf, tabs);

    auto sym = data(gs);

    fmt::format_to(std::back_inserter(buf), "{}{} {}", showKind(gs), getVariance(sym),
                   showRaw ? toStringFullName(gs) : showFullName(gs));

    printResultType(gs, buf, sym->resultType, tabs, showRaw);
    printLocs(gs, buf, sym->locs(), showRaw);

    ENFORCE(!absl::c_any_of(to_string(buf), [](char c) { return c == '\n'; }));
    fmt::format_to(std::back_inserter(buf), "\n");

    return to_string(buf);
}

string SymbolRef::toStringWithOptions(const GlobalState &gs, int tabs, bool showFull, bool showRaw) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            return asClassOrModuleRef().toStringWithOptions(gs, tabs, showFull, showRaw);
        case SymbolRef::Kind::Method:
            return asMethodRef().toStringWithOptions(gs, tabs, showFull, showRaw);
        case SymbolRef::Kind::FieldOrStaticField:
            return asFieldRef().toStringWithOptions(gs, tabs, showFull, showRaw);
        case SymbolRef::Kind::TypeArgument:
            return asTypeArgumentRef().toStringWithOptions(gs, tabs, showFull, showRaw);
        case SymbolRef::Kind::TypeMember:
            return asTypeMemberRef().toStringWithOptions(gs, tabs, showFull, showRaw);
    }
}

NameRef SymbolRef::name(const GlobalState &gs) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            return asClassOrModuleRef().data(gs)->name;
        case SymbolRef::Kind::Method:
            return asMethodRef().data(gs)->name;
        case SymbolRef::Kind::FieldOrStaticField:
            return asFieldRef().data(gs)->name;
        case SymbolRef::Kind::TypeArgument:
            return asTypeArgumentRef().data(gs)->name;
        case SymbolRef::Kind::TypeMember:
            return asTypeMemberRef().data(gs)->name;
    }
}

SymbolRef SymbolRef::owner(const GlobalState &gs) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            return asClassOrModuleRef().data(gs)->owner;
        case SymbolRef::Kind::Method:
            return asMethodRef().data(gs)->owner;
        case SymbolRef::Kind::FieldOrStaticField:
            return asFieldRef().data(gs)->owner;
        case SymbolRef::Kind::TypeArgument:
            return asTypeArgumentRef().data(gs)->owner;
        case SymbolRef::Kind::TypeMember:
            return asTypeMemberRef().data(gs)->owner;
    }
}

Loc SymbolRef::loc(const GlobalState &gs) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            return asClassOrModuleRef().data(gs)->loc();
        case SymbolRef::Kind::Method:
            return asMethodRef().data(gs)->loc();
        case SymbolRef::Kind::FieldOrStaticField:
            return asFieldRef().data(gs)->loc();
        case SymbolRef::Kind::TypeArgument:
            return asTypeArgumentRef().data(gs)->loc();
        case SymbolRef::Kind::TypeMember:
            return asTypeMemberRef().data(gs)->loc();
    }
}

bool SymbolRef::isPrintable(const GlobalState &gs) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            return asClassOrModuleRef().data(gs)->isPrintable(gs);
        case SymbolRef::Kind::Method:
            return asMethodRef().data(gs)->isPrintable(gs);
        case SymbolRef::Kind::FieldOrStaticField:
            return asFieldRef().data(gs)->isPrintable(gs);
        case SymbolRef::Kind::TypeArgument:
            return asTypeArgumentRef().data(gs)->isPrintable(gs);
        case SymbolRef::Kind::TypeMember:
            return asTypeMemberRef().data(gs)->isPrintable(gs);
    }
}

const InlinedVector<Loc, 2> &SymbolRef::locs(const GlobalState &gs) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            return asClassOrModuleRef().data(gs)->locs();
        case SymbolRef::Kind::Method:
            return asMethodRef().data(gs)->locs();
        case SymbolRef::Kind::FieldOrStaticField:
            return asFieldRef().data(gs)->locs();
        case SymbolRef::Kind::TypeArgument:
            return asTypeArgumentRef().data(gs)->locs();
        case SymbolRef::Kind::TypeMember:
            return asTypeMemberRef().data(gs)->locs();
    }
}

void SymbolRef::removeLocsForFile(GlobalState &gs, core::FileRef file) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            return asClassOrModuleRef().data(gs)->removeLocsForFile(file);
        case SymbolRef::Kind::Method:
            return asMethodRef().data(gs)->removeLocsForFile(file);
        case SymbolRef::Kind::FieldOrStaticField:
            return asFieldRef().data(gs)->removeLocsForFile(file);
        case SymbolRef::Kind::TypeArgument:
            return asTypeArgumentRef().data(gs)->removeLocsForFile(file);
        case SymbolRef::Kind::TypeMember:
            return asTypeMemberRef().data(gs)->removeLocsForFile(file);
    }
}

const TypePtr &SymbolRef::resultType(const GlobalState &gs) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            return asClassOrModuleRef().data(gs)->resultType;
        case SymbolRef::Kind::Method:
            return asMethodRef().data(gs)->resultType;
        case SymbolRef::Kind::FieldOrStaticField:
            return asFieldRef().data(gs)->resultType;
        case SymbolRef::Kind::TypeArgument:
            return asTypeArgumentRef().data(gs)->resultType;
        case SymbolRef::Kind::TypeMember:
            return asTypeMemberRef().data(gs)->resultType;
    }
}

void SymbolRef::setResultType(GlobalState &gs, const TypePtr &typePtr) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            asClassOrModuleRef().data(gs)->resultType = typePtr;
            return;
        case SymbolRef::Kind::Method:
            asMethodRef().data(gs)->resultType = typePtr;
            return;
        case SymbolRef::Kind::FieldOrStaticField:
            asFieldRef().data(gs)->resultType = typePtr;
            return;
        case SymbolRef::Kind::TypeArgument:
            asTypeArgumentRef().data(gs)->resultType = typePtr;
            return;
        case SymbolRef::Kind::TypeMember:
            asTypeMemberRef().data(gs)->resultType = typePtr;
            return;
    }
}

SymbolRef SymbolRef::dealias(const GlobalState &gs) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            return asClassOrModuleRef().data(gs)->dealias(gs);
        case SymbolRef::Kind::Method:
            return asMethodRef().data(gs)->dealiasMethod(gs);
        case SymbolRef::Kind::FieldOrStaticField:
            return asFieldRef().data(gs)->dealias(gs);
        case SymbolRef::Kind::TypeArgument:
            return asTypeArgumentRef().data(gs)->dealias(gs);
        case SymbolRef::Kind::TypeMember:
            return asTypeMemberRef().data(gs)->dealias(gs);
    }
}

string ArgInfo::show(const GlobalState &gs) const {
    return fmt::format("{}", this->argumentName(gs));
}

string ArgInfo::toString(const GlobalState &gs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "argument {}", show(gs));
    vector<string_view> flagTexts;
    if (flags.isDefault) {
        flagTexts.emplace_back("optional"sv);
    }
    if (flags.isKeyword) {
        flagTexts.emplace_back("keyword"sv);
    }
    if (flags.isRepeated) {
        flagTexts.emplace_back("repeated"sv);
    }
    if (flags.isBlock) {
        flagTexts.emplace_back("block"sv);
    }
    if (flags.isShadow) {
        flagTexts.emplace_back("shadow"sv);
    }
    fmt::format_to(std::back_inserter(buf), "<{}>", fmt::join(flagTexts, ", "));
    if (this->type) {
        fmt::format_to(std::back_inserter(buf), " -> {}", this->type.show(gs));
    }

    fmt::format_to(std::back_inserter(buf), " @ {}", loc.showRaw(gs));

    if (this->rebind.exists()) {
        fmt::format_to(std::back_inserter(buf), " rebindTo {}", this->rebind.showFullName(gs));
    }

    return to_string(buf);
}

string_view ArgInfo::argumentName(const GlobalState &gs) const {
    if (flags.isKeyword && !flags.isRepeated) {
        return name.shortName(gs);
    } else {
        if (auto source = loc.source(gs)) {
            return source.value();
        } else {
            return name.shortName(gs);
        }
    }
}

namespace {
bool isSingletonName(const GlobalState &gs, core::NameRef name) {
    return name.kind() == NameKind::UNIQUE && name.dataUnique(gs)->uniqueNameKind == UniqueNameKind::Singleton;
}

bool isMangledSingletonName(const GlobalState &gs, core::NameRef name) {
    return name.kind() == NameKind::UNIQUE && name.dataUnique(gs)->uniqueNameKind == UniqueNameKind::MangleRename &&
           isSingletonName(gs, name.dataUnique(gs)->original);
}
} // namespace

bool ClassOrModule::isSingletonClass(const GlobalState &gs) const {
    bool isSingleton = isSingletonName(gs, name) || isMangledSingletonName(gs, name);
    DEBUG_ONLY(if (ref(gs) != Symbols::untyped()) { // ClassOrModule::untyped is attached to itself
        if (isSingleton) {
            ENFORCE(attachedClass(gs).exists());
        } else {
            ENFORCE(!attachedClass(gs).exists());
        }
    });
    return isSingleton;
}

ClassOrModuleRef ClassOrModule::singletonClass(GlobalState &gs) {
    auto singleton = lookupSingletonClass(gs);
    if (singleton.exists()) {
        return singleton;
    }
    ClassOrModuleRef selfRef = this->ref(gs);

    NameRef singletonName = gs.freshNameUnique(UniqueNameKind::Singleton, this->name, 1);
    singleton = gs.enterClassSymbol(this->loc(), this->owner, singletonName);
    ClassOrModuleData singletonInfo = singleton.data(gs);

    // --------
    // Call to enterClassSymbol might have reallocated the memory that `*this` pointed to
    // It's not safe to use `this` anymore.
    // --------
    const auto &self = selfRef.data(gs);

    singletonInfo->members()[Names::attached()] = selfRef;
    singletonInfo->setSuperClass(Symbols::todo());
    singletonInfo->setIsModule(false);

    ENFORCE(self->isClassModuleSet(), "{}", selfRef.show(gs));
    if (self->isClass()) {
        auto tp = gs.enterTypeMember(self->loc(), singleton, Names::Constants::AttachedClass(), Variance::CoVariant);

        // Initialize the bounds of AttachedClass as todo, as they will be updated
        // to the externalType of the attached class for the upper bound, and bottom
        // for the lower bound in the ResolveSignaturesWalk pass of the resolver.
        auto todo = make_type<ClassType>(Symbols::todo());
        tp.data(gs)->resultType = make_type<LambdaParam>(tp, todo, todo);
    }

    selfRef.data(gs)->members()[Names::singleton()] = singleton;
    return singleton;
}

ClassOrModuleRef ClassOrModule::lookupSingletonClass(const GlobalState &gs) const {
    ENFORCE(this->name.isClassName(gs));

    SymbolRef selfRef = this->ref(gs);
    if (selfRef == Symbols::untyped()) {
        return Symbols::untyped();
    }

    return findMember(gs, Names::singleton()).asClassOrModuleRef();
}

ClassOrModuleRef ClassOrModule::attachedClass(const GlobalState &gs) const {
    if (this->ref(gs) == Symbols::untyped()) {
        return Symbols::untyped();
    }

    SymbolRef singleton = findMember(gs, Names::attached());
    return singleton.asClassOrModuleRef();
}

ClassOrModuleRef ClassOrModule::topAttachedClass(const GlobalState &gs) const {
    ClassOrModuleRef classSymbol = this->ref(gs);

    while (true) {
        auto attachedClass = classSymbol.data(gs)->attachedClass(gs);
        if (!attachedClass.exists()) {
            break;
        }
        classSymbol = attachedClass;
    }

    return classSymbol;
}

void ClassOrModule::recordSealedSubclass(MutableContext ctx, ClassOrModuleRef subclass) {
    ENFORCE(this->flags.isSealed, "Class is not marked sealed: {}", ref(ctx).show(ctx));
    ENFORCE(subclass.exists(), "Can't record sealed subclass for {} when subclass doesn't exist", ref(ctx).show(ctx));

    // Avoid using a clobbered `this` pointer, as `singletonClass` can cause the symbol table to move.
    ClassOrModuleRef selfRef = this->ref(ctx);

    // We record sealed subclasses on a magical method called core::Names::sealedSubclasses(). This is so we don't
    // bloat the `sizeof class Symbol` with an extra field that most class sybmols will never use.
    // Note: We had hoped to ALSO implement this method in the runtime, but we couldn't think of a way to make it work
    // that didn't require running with the help of Stripe's autoloader, specifically because we might want to allow
    // subclassing a sealed class across multiple files, not just one file.
    auto classOfSubclass = subclass.data(ctx)->singletonClass(ctx);
    auto sealedSubclasses =
        selfRef.data(ctx)->lookupSingletonClass(ctx).data(ctx)->findMethod(ctx, core::Names::sealedSubclasses());

    auto data = sealedSubclasses.data(ctx);
    ENFORCE(data->resultType != nullptr, "Should have been populated in namer");
    auto appliedType = cast_type<AppliedType>(data->resultType);
    ENFORCE(appliedType != nullptr, "sealedSubclasses should always be AppliedType");
    ENFORCE(appliedType->klass == core::Symbols::Set(), "sealedSubclasses should always be Set");
    auto currentClasses = appliedType->targs[0];

    const TypePtr *iter = &currentClasses;
    const OrType *orT = nullptr;
    while ((orT = cast_type<OrType>(*iter))) {
        auto right = cast_type_nonnull<ClassType>(orT->right);
        ENFORCE(left);
        if (right.symbol == classOfSubclass) {
            return;
        }
        iter = &orT->left;
    }
    if (cast_type_nonnull<ClassType>(*iter).symbol == classOfSubclass) {
        return;
    }
    if (currentClasses != core::Types::bottom()) {
        appliedType->targs[0] = OrType::make_shared(currentClasses, make_type<ClassType>(classOfSubclass));
    } else {
        appliedType->targs[0] = make_type<ClassType>(classOfSubclass);
    }
}

const InlinedVector<Loc, 2> &ClassOrModule::sealedLocs(const GlobalState &gs) const {
    ENFORCE(this->flags.isSealed, "Class is not marked sealed: {}", ref(gs).show(gs));
    auto sealedSubclasses = this->lookupSingletonClass(gs).data(gs)->findMethod(gs, core::Names::sealedSubclasses());
    auto &result = sealedSubclasses.data(gs)->locs();
    ENFORCE(result.size() > 0);
    return result;
}

TypePtr ClassOrModule::sealedSubclassesToUnion(const GlobalState &gs) const {
    ENFORCE(this->flags.isSealed, "Class is not marked sealed: {}", ref(gs).show(gs));

    auto sealedSubclasses = this->lookupSingletonClass(gs).data(gs)->findMethod(gs, core::Names::sealedSubclasses());

    auto data = sealedSubclasses.data(gs);
    ENFORCE(data->resultType != nullptr, "Should have been populated in namer");
    auto appliedType = cast_type<AppliedType>(data->resultType);
    ENFORCE(appliedType != nullptr, "sealedSubclasses should always be AppliedType");
    ENFORCE(appliedType->klass == core::Symbols::Set(), "sealedSubclasses should always be Set");

    auto currentClasses = appliedType->targs[0];
    if (currentClasses.isBottom()) {
        // Declared sealed parent class, but never saw any children.
        return Types::bottom();
    }

    auto result = Types::bottom();
    while (auto orType = cast_type<OrType>(currentClasses)) {
        ENFORCE(isa_type<ClassType>(orType->right), "Something in sealedSubclasses that's not a ClassType");
        auto classType = cast_type_nonnull<ClassType>(orType->right);
        auto subclass = classType.symbol.data(gs)->attachedClass(gs);
        ENFORCE(subclass.exists());
        result = Types::any(gs, subclass.data(gs)->externalType(), result);
        currentClasses = orType->left;
    }

    ENFORCE(isa_type<ClassType>(currentClasses), "Last element of sealedSubclasses must be ClassType");
    auto lastClassType = cast_type_nonnull<ClassType>(currentClasses);
    auto subclass = lastClassType.symbol.data(gs)->attachedClass(gs);
    ENFORCE(subclass.exists());
    result = Types::any(gs, subclass.data(gs)->externalType(), result);

    return result;
}

bool ClassOrModule::hasSingleSealedSubclass(const GlobalState &gs) const {
    ENFORCE(this->flags.isSealed, "Class is not marked sealed: {}", ref(gs).show(gs));

    auto sealedSubclasses = this->lookupSingletonClass(gs).data(gs)->findMethod(gs, core::Names::sealedSubclasses());

    // When the sealed type is a class, it must also be abstract for there to be a single subclass.
    if (this->isClass() && !this->flags.isAbstract) {
        return false;
    }

    auto data = sealedSubclasses.data(gs);
    ENFORCE(data->resultType != nullptr, "Should have been populated in namer");
    auto appliedType = cast_type<AppliedType>(data->resultType);
    ENFORCE(appliedType != nullptr, "sealedSubclasses should always be AppliedType");
    ENFORCE(appliedType->klass == core::Symbols::Set(), "sealedSubclasses should always be Set");

    // There is exactly one subclass when the sealedSubclasses field is a set with a single non-bottom type in it
    auto currentClasses = appliedType->targs[0];
    return !currentClasses.isBottom() && !isa_type<OrType>(currentClasses);
}

// Record a required ancestor for this class of module
//
// Each RequiredAncestor is stored into a magic method referenced by `prop` where:
// * RequiredAncestor.symbol goes into the return type tuple
// * RequiredAncestor.origin goes into the first argument type tuple
// * RequiredAncestor.loc goes into the symbol loc
// All fields for the same RequiredAncestor are stored at the same index.
void ClassOrModule::recordRequiredAncestorInternal(GlobalState &gs, ClassOrModule::RequiredAncestor &ancestor,
                                                   NameRef prop) {
    // We store the required ancestors into a fake property called `<required-ancestors>`
    auto ancestors = this->findMethod(gs, prop);
    if (!ancestors.exists()) {
        ancestors = gs.enterMethodSymbol(ancestor.loc, this->ref(gs), prop);
        ancestors.data(gs)->locs_.clear(); // Remove the original location

        // Create the return type tuple to store RequiredAncestor.symbol
        vector<TypePtr> tsymbols;
        ancestors.data(gs)->resultType = make_type<TupleType>(move(tsymbols));

        // Create the first argument typed as a tuple to store RequiredAncestor.origin
        auto &arg = gs.enterMethodArgumentSymbol(core::Loc::none(), ancestors, core::Names::arg());
        vector<TypePtr> torigins;
        arg.type = make_type<TupleType>(move(torigins));
    }

    // Do not require the same ancestor twice
    auto &elems = (cast_type<TupleType>(ancestors.data(gs)->resultType))->elems;
    bool alreadyRecorded = absl::c_any_of(elems, [ancestor](auto elem) {
        ENFORCE(isa_type<ClassType>(elem), "Something in requiredAncestors that's not a ClassType");
        return cast_type_nonnull<ClassType>(elem).symbol == ancestor.symbol;
    });
    if (alreadyRecorded) {
        return;
    }

    // Store the RequiredAncestor.symbol
    auto tSymbol = core::make_type<ClassType>(ancestor.symbol);
    (cast_type<TupleType>(ancestors.data(gs)->resultType))->elems.emplace_back(tSymbol);

    // Store the RequiredAncestor.origin
    auto tOrigin = core::make_type<ClassType>(ancestor.origin);
    (cast_type<TupleType>(ancestors.data(gs)->arguments[0].type))->elems.emplace_back(tOrigin);

    // Store the RequiredAncestor.loc
    ancestors.data(gs)->locs_.emplace_back(ancestor.loc);
}

// Locally required ancestors by this class or module
vector<ClassOrModule::RequiredAncestor> ClassOrModule::readRequiredAncestorsInternal(const GlobalState &gs,
                                                                                     NameRef prop) const {
    vector<RequiredAncestor> res;

    auto ancestors = this->findMethod(gs, prop);
    if (!ancestors.exists()) {
        // No ancestor was recorded for this class or module
        return res;
    }

    auto data = ancestors.data(gs);
    auto tSymbols = cast_type<TupleType>(data->resultType);
    auto tOrigins = cast_type<TupleType>(data->arguments[0].type);
    auto index = 0;
    for (auto elem : tSymbols->elems) {
        ENFORCE(isa_type<ClassType>(elem), "Something in requiredAncestors that's not a ClassType");
        ENFORCE(isa_type<ClassType>(tOrigins->elems[index]), "Bad origin in requiredAncestors");
        ENFORCE(index < data->locs().size(), "Missing loc in requiredAncestors");
        auto &origin = cast_type_nonnull<ClassType>(tOrigins->elems[index]).symbol;
        auto &symbol = cast_type_nonnull<ClassType>(elem).symbol;
        auto &loc = data->locs()[index];
        res.emplace_back(origin, symbol, loc);
        index++;
    }

    return res;
}

// Record a required ancestor for this class of module
void ClassOrModule::recordRequiredAncestor(GlobalState &gs, ClassOrModuleRef ancestor, Loc loc) {
    RequiredAncestor req = {this->ref(gs), ancestor, loc};
    recordRequiredAncestorInternal(gs, req, Names::requiredAncestors());
}

// Locally required ancestors by this class or module
vector<ClassOrModule::RequiredAncestor> ClassOrModule::requiredAncestors(const GlobalState &gs) const {
    return readRequiredAncestorsInternal(gs, Names::requiredAncestors());
}

// All required ancestors by this class or module
std::vector<ClassOrModule::RequiredAncestor>
ClassOrModule::requiredAncestorsTransitiveInternal(GlobalState &gs, std::vector<ClassOrModuleRef> &seen) {
    if (absl::c_find(seen, this->ref(gs)) != seen.end()) {
        return requiredAncestors(gs); // Break recursive loops if we already visited this ancestor
    }
    seen.emplace_back(this->ref(gs));

    for (auto ancst : requiredAncestors(gs)) {
        recordRequiredAncestorInternal(gs, ancst, Names::requiredAncestorsLin());
        for (auto sancst : ancst.symbol.data(gs)->requiredAncestorsTransitiveInternal(gs, seen)) {
            if (sancst.symbol != this->ref(gs)) {
                recordRequiredAncestorInternal(gs, sancst, Names::requiredAncestorsLin());
            }
        }
    }

    auto parent = superClass();
    if (parent.exists()) {
        for (auto ancst : parent.data(gs)->requiredAncestorsTransitiveInternal(gs, seen)) {
            if (ancst.symbol != this->ref(gs)) {
                recordRequiredAncestorInternal(gs, ancst, Names::requiredAncestorsLin());
            }
        }
    }

    for (auto mixin : mixins()) {
        for (auto ancst : mixin.data(gs)->requiredAncestors(gs)) {
            if (ancst.symbol != this->ref(gs)) {
                recordRequiredAncestorInternal(gs, ancst, Names::requiredAncestorsLin());
            }
        }
    }

    return requiredAncestorsTransitive(gs);
}

// All required ancestors by this class or module
vector<ClassOrModule::RequiredAncestor> ClassOrModule::requiredAncestorsTransitive(const GlobalState &gs) const {
    ENFORCE(gs.requiresAncestorEnabled);
    return readRequiredAncestorsInternal(gs, Names::requiredAncestorsLin());
}

void ClassOrModule::computeRequiredAncestorLinearization(GlobalState &gs) {
    ENFORCE(gs.requiresAncestorEnabled);
    std::vector<ClassOrModuleRef> seen;
    requiredAncestorsTransitiveInternal(gs, seen);
}

namespace {
SymbolRef dealiasWithDefault(const GlobalState &gs, core::SymbolRef symbol, int depthLimit, SymbolRef def) {
    const auto &resultType = symbol.resultType(gs);
    if (isa_type<AliasType>(resultType)) {
        auto alias = cast_type_nonnull<AliasType>(resultType);
        if (depthLimit == 0) {
            if (auto e = gs.beginError(symbol.loc(gs), errors::Internal::CyclicReferenceError)) {
                e.setHeader("Too many alias expansions for symbol {}, the alias is either too long or infinite. Next "
                            "expansion would have been to {}",
                            symbol.showFullName(gs), alias.symbol.showFullName(gs));
            }
            return def;
        }
        return dealiasWithDefault(gs, alias.symbol, depthLimit - 1, def);
    }
    return symbol;
}
} // namespace

// if dealiasing fails here, then we return Untyped instead
SymbolRef ClassOrModule::dealias(const GlobalState &gs, int depthLimit) const {
    return dealiasWithDefault(gs, this->ref(gs), depthLimit, Symbols::untyped());
}
// if dealiasing fails here, then we return a bad alias method stub instead
MethodRef Method::dealiasMethod(const GlobalState &gs, int depthLimit) const {
    return dealiasWithDefault(gs, this->ref(gs), depthLimit, core::Symbols::Sorbet_Private_Static_badAliasMethodStub())
        .asMethodRef();
}

SymbolRef Field::dealias(const GlobalState &gs, int depthLimit) const {
    return dealiasWithDefault(gs, this->ref(gs), depthLimit, Symbols::untyped());
}

SymbolRef TypeParameter::dealias(const GlobalState &gs, int depthLimit) const {
    return dealiasWithDefault(gs, this->ref(gs), depthLimit, Symbols::untyped());
}

bool ArgInfo::isSyntheticBlockArgument() const {
    // Every block argument that we synthesize in desugar or enter manually into global state uses Loc::none().
    return flags.isBlock && !loc.exists();
}

ArgInfo ArgInfo::deepCopy() const {
    ArgInfo result;
    result.flags = this->flags;
    result.type = this->type;
    result.loc = this->loc;
    result.name = this->name;
    result.rebind = this->rebind;
    return result;
}

uint8_t ArgInfo::ArgFlags::toU1() const {
    uint8_t flags = 0;
    if (isKeyword) {
        flags += 1;
    }
    if (isRepeated) {
        flags += 2;
    }
    if (isDefault) {
        flags += 4;
    }
    if (isShadow) {
        flags += 8;
    }
    if (isBlock) {
        flags += 16;
    }
    return flags;
}

void ArgInfo::ArgFlags::setFromU1(uint8_t flags) {
    isKeyword = flags & 1;
    isRepeated = flags & 2;
    isDefault = flags & 4;
    isShadow = flags & 8;
    isBlock = flags & 16;
}

ClassOrModule ClassOrModule::deepCopy(const GlobalState &to, bool keepGsId) const {
    ClassOrModule result;
    result.owner = this->owner;
    result.flags = this->flags;
    result.mixins_ = this->mixins_;
    result.resultType = this->resultType;
    result.name = NameRef(to, this->name);
    result.locs_ = this->locs_;
    if (this->typeParams) {
        result.typeParams = std::make_unique<InlinedVector<TypeMemberRef, 4>>(*this->typeParams);
    }
    if (keepGsId) {
        result.members_ = this->members_;
    } else {
        result.members_.reserve(this->members().size());
        for (auto &mem : this->members_) {
            result.members_[NameRef(to, mem.first)] = mem.second;
        }
    }

    result.superClass_ = this->superClass_;
    return result;
}

Method Method::deepCopy(const GlobalState &to) const {
    Method result;
    result.owner = this->owner;
    result.flags = this->flags;
    result.resultType = this->resultType;
    result.name = NameRef(to, this->name);
    result.locs_ = this->locs_;
    if (this->typeArgs) {
        result.typeArgs = std::make_unique<InlinedVector<TypeArgumentRef, 4>>(*this->typeArgs);
    }
    result.arguments.reserve(this->arguments.size());
    for (auto &mem : this->arguments) {
        auto &store = result.arguments.emplace_back(mem.deepCopy());
        store.name = NameRef(to, mem.name);
    }
    result.rebind = this->rebind;
    result.intrinsicOffset = this->intrinsicOffset;
    return result;
}

Field Field::deepCopy(const GlobalState &to) const {
    Field result;
    result.owner = this->owner;
    result.flags = this->flags;
    result.resultType = this->resultType;
    result.name = NameRef(to, this->name);
    result.locs_ = this->locs_;
    return result;
}

TypeParameter TypeParameter::deepCopy(const GlobalState &to) const {
    TypeParameter result;
    result.owner = this->owner;
    result.flags = this->flags;
    result.resultType = this->resultType;
    result.name = NameRef(to, this->name);
    result.locs_ = this->locs_;
    return result;
}

int ClassOrModule::typeArity(const GlobalState &gs) const {
    int arity = 0;
    for (auto &tm : this->typeMembers()) {
        if (!tm.data(gs)->flags.isFixed) {
            ++arity;
        }
    }
    return arity;
}

void ClassOrModule::sanityCheck(const GlobalState &gs) const {
    if (!debug_mode) {
        return;
    }
    ClassOrModuleRef current = this->ref(gs);
    if (current != Symbols::root()) {
        ClassOrModuleRef current2 =
            const_cast<GlobalState &>(gs).enterClassSymbol(this->loc(), this->owner, this->name);
        ENFORCE_NO_TIMER(current == current2);
        for (auto &e : members()) {
            ENFORCE_NO_TIMER(e.first.exists(), "{} has a member symbol without a name", name.toString(gs));
            ENFORCE_NO_TIMER(e.second.exists(), "{}.{} corresponds to a core::Symbols::noSymbol()", name.toString(gs),
                             e.first.toString(gs));
        }
    }
}

void Method::sanityCheck(const GlobalState &gs) const {
    if (!debug_mode) {
        return;
    }
    MethodRef current = this->ref(gs);
    MethodRef current2 = const_cast<GlobalState &>(gs).enterMethodSymbol(this->loc(), this->owner, this->name);

    ENFORCE_NO_TIMER(current == current2);
    for (auto &tp : typeArguments()) {
        ENFORCE_NO_TIMER(tp.data(gs)->name.exists(), name.toString(gs) + " has a member symbol without a name");
        ENFORCE_NO_TIMER(tp.exists(), name.toString(gs) + "." + tp.data(gs)->name.toString(gs) +
                                          " corresponds to a core::Symbols::noTypeArgument()");
    }

    // There should always either be a block argument at the end, or the method should be an alias
    ENFORCE_NO_TIMER(!this->arguments.empty(), ref(gs).show(gs));

    if (isa_type<AliasType>(this->resultType)) {
        // The arguments of an alias method don't mean anything. When calling a method alias,
        // we dealias the symbol and use those arguments.
        //
        // This leaves the alias method's arguments vector free for us to stash some information. See resolver.
        ENFORCE_NO_TIMER(absl::c_all_of(this->arguments, [](const auto &arg) { return arg.flags.isKeyword; }),
                         ref(gs).show(gs));
        ENFORCE_NO_TIMER(absl::c_all_of(this->arguments, [](const auto &arg) { return arg.flags.isKeyword; }),
                         ref(gs).show(gs));
    }
}

void Field::sanityCheck(const GlobalState &gs) const {
    if (!debug_mode) {
        return;
    }
    FieldRef current = this->ref(gs);
    FieldRef current2;
    if (flags.isField) {
        current2 = const_cast<GlobalState &>(gs).enterFieldSymbol(this->loc(), this->owner, this->name);
        ENFORCE_NO_TIMER(!flags.isStaticFieldTypeAlias);
        ENFORCE_NO_TIMER(!flags.isStaticField);
        ENFORCE_NO_TIMER(!flags.isStaticFieldPrivate);
    } else {
        current2 = const_cast<GlobalState &>(gs).enterStaticFieldSymbol(this->loc(), this->owner, this->name);
    }
    ENFORCE_NO_TIMER(current == current2);
}

void TypeParameter::sanityCheck(const GlobalState &gs) const {
    if (!debug_mode) {
        return;
    }
    SymbolRef current = this->ref(gs);
    if (current != Symbols::root()) {
        SymbolRef current2;
        switch (current.kind()) {
            case SymbolRef::Kind::ClassOrModule:
            case SymbolRef::Kind::Method:
            case SymbolRef::Kind::FieldOrStaticField:
                ENFORCE(false, "Should not happen");
                break;
            case SymbolRef::Kind::TypeArgument:
                current2 = const_cast<GlobalState &>(gs).enterTypeArgument(this->loc(), this->owner.asMethodRef(),
                                                                           this->name, this->variance());
                break;
            case SymbolRef::Kind::TypeMember:
                current2 = const_cast<GlobalState &>(gs).enterTypeMember(this->loc(), this->owner.asClassOrModuleRef(),
                                                                         this->name, this->variance());
                break;
        }

        ENFORCE_NO_TIMER(current == current2);
    }
}

ClassOrModuleRef MethodRef::enclosingClass(const GlobalState &gs) const {
    // Methods can only be owned by classes or modules.
    auto result = data(gs)->owner;
    ENFORCE(result != core::Symbols::todo(),
            "Namer hasn't populated the information required to provide an enclosing class yet");
    return result;
}

ClassOrModuleRef SymbolRef::enclosingClass(const GlobalState &gs) const {
    core::ClassOrModuleRef result;
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            result = asClassOrModuleRef();
            break;
        case SymbolRef::Kind::Method:
            result = asMethodRef().enclosingClass(gs);
            break;
        case SymbolRef::Kind::FieldOrStaticField:
            // Fields can only be owned by classes or modules.
            result = asFieldRef().data(gs)->owner;
            break;
        case SymbolRef::Kind::TypeArgument:
            // Typeargs are owned by methods.
            result = asTypeArgumentRef().data(gs)->owner.asMethodRef().enclosingClass(gs);
            break;
        case SymbolRef::Kind::TypeMember:
            // TypeMembers are only owned by classes or modules.
            result = asTypeMemberRef().data(gs)->owner.asClassOrModuleRef();
            break;
    }

    ENFORCE(result != core::Symbols::todo(),
            "Namer hasn't populated the information required to provide an enclosing class yet");
    return result;
}

uint32_t ClassOrModule::hash(const GlobalState &gs, bool skipTypeMemberNames) const {
    uint32_t result = _hash(name.shortName(gs));

    // Bit of a hack to ensure that the isBehaviorDefining flag is not serialized.
    // We do not want behavior changes to trigger the slow path.
    auto flagsCopy = this->flags;
    flagsCopy.isBehaviorDefining = false;
    result = mix(result, std::move(flagsCopy).serialize());

    result = mix(result, this->owner.id());
    result = mix(result, this->superClass_.id());
    // argumentsOrMixins, typeParams, typeAliases
    if (!members().empty()) {
        // Rather than use membersStableOrderSlow, which is... slow..., use an order dictated by symbol ref ID.
        // It's faster to sort, and SymbolRef IDs are stable during hashing.
        vector<core::SymbolRef> membersToHash;
        membersToHash.reserve(this->members().size());
        for (auto e : this->members()) {
            if (!e.second.exists()) {
                continue;
            }

            if (e.second.isMethod()) {
                continue;
            }

            if (e.second.isFieldOrStaticField()) {
                const auto &field = e.second.asFieldRef().data(gs);
                if (field->flags.isStaticField && !field->isClassAlias()) {
                    continue;
                } else if (field->flags.isField) {
                    continue;
                } else {
                    // Currently only static field class aliases must take the slow path
                    ENFORCE(field->flags.isStaticField && field->isClassAlias());

                    const auto &dealiased = field->dealias(gs);
                    if (dealiased.isTypeMember() && field->name == dealiased.name(gs) &&
                        dealiased.owner(gs) == this->lookupSingletonClass(gs)) {
                        // This is a static field class alias that forwards to a type_template on the singleton class
                        // (in service of constant literal resolution). Treat this as a type member (which we can
                        // handle on the fast path) and not like other class aliases.
                        continue;
                    }
                }
            }

            if (skipTypeMemberNames && e.second.isTypeMember()) {
                // skipTypeMemberNames is currently the difference between `hash` and `classOrModuleShapeHash`
                // (It felt wasteful to dupe this whole method.) Type member names have to be in the
                // full hash so that a change to a type member name causes the right downstream
                // files to retypecheck on the fast path (they might only mention the owner class,
                // not the type member names themselves). They don't have to be in the shape hash
                // because changes to type members can take the fast path.
                continue;
            }

            if (e.second.isClassOrModule() && e.second.asClassOrModuleRef().data(gs)->ignoreInHashing(gs)) {
                continue;
            }

            membersToHash.emplace_back(e.second);
        }
        fast_sort(membersToHash, [](const auto &a, const auto &b) -> bool { return a.rawId() < b.rawId(); });
        for (auto member : membersToHash) {
            if (member.isTypeMember()) {
                result = mix(result, member.asTypeMemberRef().data(gs)->hash(gs));
            } else {
                result = mix(result, _hash(member.name(gs).shortName(gs)));
            }
        }
    }
    for (const auto &e : mixins_) {
        if (e.exists() && !e.data(gs)->ignoreInHashing(gs)) {
            result = mix(result, _hash(e.data(gs)->name.shortName(gs)));
        }
    }

    return result;
}

// Tracks whether _anything_ changed about the method, but unlike the other things, this hash is not
// used for the fast path decision. This is just used to figure out the names of the method symbols
// that changed, so that _if_ we took the fast path (determined using methodShapeHash), we know
// which methods might have been updated on the fast path, so we can then figure out which files to
// type check after the fast path resolver finishes.
uint32_t Method::hash(const GlobalState &gs) const {
    uint32_t result = _hash(name.shortName(gs));
    result = mix(result, !this->resultType ? 0 : this->resultType.hash(gs));
    result = mix(result, this->flags.serialize());
    result = mix(result, this->owner.id());
    result = mix(result, this->rebind.id());
    result = mix(result, this->methodArityHash(gs)._hashValue);
    for (const auto &arg : arguments) {
        // If an argument's resultType changes, then the sig has changed.
        auto type = arg.type;
        if (!type) {
            type = Types::untypedUntracked();
        }
        result = mix(result, type.hash(gs));
    }
    for (const auto &e : typeArguments()) {
        if (e.exists()) {
            result = mix(result, _hash(e.data(gs)->name.shortName(gs)));
        }
    }

    return result;
}

bool Field::isClassAlias() const {
    ENFORCE(this->flags.isStaticField, "should never ask whether instance variable is a class alias");
    return isa_type<AliasType>(resultType);
}

uint32_t Field::hash(const GlobalState &gs) const {
    uint32_t result = _hash(name.shortName(gs));
    result = mix(result, !this->resultType ? 0 : this->resultType.hash(gs));
    result = mix(result, this->flags.serialize());
    result = mix(result, this->owner.id());
    return result;
}

uint32_t TypeParameter::hash(const GlobalState &gs) const {
    uint32_t result = _hash(name.shortName(gs));
    result = mix(result, !this->resultType ? 0 : this->resultType.hash(gs));
    result = mix(result, this->flags.serialize());
    result = mix(result, this->owner.rawId());
    return result;
}

uint32_t Method::methodShapeHash(const GlobalState &gs) const {
    uint32_t result = _hash(name.shortName(gs));
    result = mix(result, this->flags.serialize());
    result = mix(result, this->owner.id());
    result = mix(result, this->rebind.id());
    result = mix(result, this->hasSig());
    result = mix(result, this->methodArityHash(gs)._hashValue);

    if (name == Names::unresolvedAncestors() || name == Names::requiredAncestors() ||
        name == Names::requiredAncestorsLin()) {
        // These are synthetic methods that encode information about the class hierarchy in their types.
        // If the types change, ancestor information changed, and we must take the slow path.
        ENFORCE(resultType);
        result = mix(result, resultType.hash(gs));
    }

    return result;
}

uint32_t Field::fieldShapeHash(const GlobalState &gs) const {
    // Only consider static fields for the fast path at the moment.  It is probably
    // straightforward to take the fast path for changes to regular fields by changing
    // this and the corresponding code in GlobalState, but one step at a time.
    // Only normal static fields are ok (no type aliases, no class aliases).
    ENFORCE(!this->flags.isStaticField || (this->flags.isStaticField && !this->isClassAlias()));
    uint32_t result = _hash(name.shortName(gs));

    result = mix(result, 1 + (this->resultType != nullptr));
    result = mix(result, this->flags.serialize());
    result = mix(result, this->owner.id());
    return result;
}

// This has to match the implementation of ArgParsing::hashArgs
ArityHash Method::methodArityHash(const GlobalState &gs) const {
    uint32_t result = 0;
    result = mix(result, arguments.size());
    for (const auto &e : arguments) {
        // Changing name of keyword arg is a shape change.
        if (e.flags.isKeyword) {
            result = mix(result, _hash(e.name.shortName(gs)));
        }
        // Changing an argument from e.g. keyword to position-based is a shape change.
        result = mix(result, e.flags.toU1());
    }
    return ArityHash(result);
}

bool ClassOrModule::ignoreInHashing(const GlobalState &gs) const {
    return superClass() == core::Symbols::StubModule();
}

bool Method::ignoreInHashing(const GlobalState &gs) const {
    return name.kind() == NameKind::UNIQUE && name.dataUnique(gs)->original == core::Names::staticInit();
}

Loc Method::loc() const {
    if (!locs_.empty()) {
        return locs_.back();
    }
    return Loc::none();
}

Loc ClassOrModule::loc() const {
    if (!locs_.empty()) {
        return locs_.back();
    }
    return Loc::none();
}

Loc Field::loc() const {
    if (!locs_.empty()) {
        return locs_.back();
    }
    return Loc::none();
}

Loc TypeParameter::loc() const {
    if (!locs_.empty()) {
        return locs_.back();
    }
    return Loc::none();
}

const InlinedVector<Loc, 2> &Method::locs() const {
    return locs_;
}

const InlinedVector<Loc, 2> &ClassOrModule::locs() const {
    return locs_;
}

const InlinedVector<Loc, 2> &Field::locs() const {
    return locs_;
}

const InlinedVector<Loc, 2> &TypeParameter::locs() const {
    return locs_;
}

namespace {
void addLocInternal(const core::GlobalState &gs, core::Loc loc, core::Loc mainLoc, InlinedVector<Loc, 2> &locs) {
    for (auto &existing : locs) {
        if (existing.file() == loc.file()) {
            existing = loc;
            return;
        }
    }

    if (locs.empty()) {
        locs.emplace_back(loc);
    } else if (loc.file().data(gs).sourceType == core::File::Type::Normal && !loc.file().data(gs).isRBI()) {
        if (mainLoc.exists() && loc.file().data(gs).strictLevel >= mainLoc.file().data(gs).strictLevel) {
            // The new loc is stricter; make it the new canonical loc.
            locs.emplace_back(loc);
        } else {
            locs.insert(locs.begin(), loc);
        }
    } else {
        // This is an RBI file; continue to use existing loc as the canonical loc.
        // Insert just before end.
        locs.insert(locs.end() - 1, loc);
    }
}

void removeLocsForFileImpl(SymbolRef::LOC_store &locs, core::FileRef file) {
    auto it = remove_if(locs.begin(), locs.end(), [&](const auto loc) { return loc.file() == file; });
    locs.erase(it, locs.end());
}

} // namespace

void Method::addLoc(const core::GlobalState &gs, core::Loc loc) {
    if (!loc.file().exists()) {
        return;
    }

    addLocInternal(gs, loc, this->loc(), locs_);
}

void Method::removeLocsForFile(core::FileRef file) {
    removeLocsForFileImpl(this->locs_, file);
}

void Field::addLoc(const core::GlobalState &gs, core::Loc loc) {
    if (!loc.file().exists()) {
        return;
    }

    addLocInternal(gs, loc, this->loc(), locs_);
}

void Field::removeLocsForFile(core::FileRef file) {
    removeLocsForFileImpl(this->locs_, file);
}

void TypeParameter::addLoc(const core::GlobalState &gs, core::Loc loc) {
    if (!loc.file().exists()) {
        return;
    }

    addLocInternal(gs, loc, this->loc(), locs_);
}

void TypeParameter::removeLocsForFile(core::FileRef file) {
    removeLocsForFileImpl(this->locs_, file);
}

void ClassOrModule::addLoc(const core::GlobalState &gs, core::Loc loc) {
    if (!loc.file().exists()) {
        return;
    }

    // We shouldn't add locs for <root>, otherwise it'll end up with a massive loc list (O(number
    // of files)). Those locs aren't useful, either.
    ENFORCE(ref(gs) != Symbols::root());
    ENFORCE(ref(gs) != Symbols::PackageSpecRegistry());

    addLocInternal(gs, loc, this->loc(), locs_);
}

void ClassOrModule::removeLocsForFile(core::FileRef file) {
    removeLocsForFileImpl(this->locs_, file);
}

vector<std::pair<NameRef, SymbolRef>> ClassOrModule::membersStableOrderSlow(const GlobalState &gs) const {
    vector<pair<NameRef, SymbolRef>> result;
    result.reserve(members().size());
    for (const auto &e : members()) {
        result.emplace_back(e);
    }
    fast_sort(result, [&](auto const &lhs, auto const &rhs) -> bool {
        auto lhsShort = lhs.first.shortName(gs);
        auto rhsShort = rhs.first.shortName(gs);
        auto compareShort = lhsShort.compare(rhsShort);
        if (compareShort != 0) {
            return compareShort < 0;
        }
        auto lhsRaw = lhs.first.showRaw(gs);
        auto rhsRaw = rhs.first.showRaw(gs);
        auto compareRaw = lhsRaw.compare(rhsRaw);
        if (compareRaw != 0) {
            return compareRaw < 0;
        }
        int i = -1;
        const auto &rhsLocs = rhs.second.locs(gs);
        for (const auto lhsLoc : lhs.second.locs(gs)) {
            i++;
            if (i > rhsLocs.size()) {
                // more locs in lhs, so `lhs < rhs` is `false`
                return false;
            }
            auto rhsLoc = rhsLocs[i];
            auto compareLoc = lhsLoc.filePosToString(gs).compare(rhsLoc.filePosToString(gs));
            if (compareLoc != 0) {
                return compareLoc < 0;
            }
        }
        if (i < rhsLocs.size()) {
            // more locs in rhs, so `lhs < rhs` is true
            return true;
        }
        ENFORCE(false, "no stable sort");
        return false;
    });
    return result;
}

ClassOrModuleData::ClassOrModuleData(ClassOrModule &ref, GlobalState &gs) : DebugOnlyCheck(gs), symbol(ref) {}

ConstClassOrModuleData::ConstClassOrModuleData(const ClassOrModule &ref, const GlobalState &gs)
    : DebugOnlyCheck(gs), symbol(ref) {}

SymbolDataDebugCheck::SymbolDataDebugCheck(const GlobalState &gs)
    : gs(gs), symbolCountAtCreation(gs.symbolsUsedTotal()) {}

void SymbolDataDebugCheck::check() const {
    ENFORCE_NO_TIMER(symbolCountAtCreation == gs.symbolsUsedTotal());
}

ClassOrModule *ClassOrModuleData::operator->() {
    runDebugOnlyCheck();
    return &symbol;
};

const ClassOrModule *ClassOrModuleData::operator->() const {
    runDebugOnlyCheck();
    return &symbol;
};

const ClassOrModule *ConstClassOrModuleData::operator->() const {
    runDebugOnlyCheck();
    return &symbol;
};

MethodData::MethodData(Method &ref, GlobalState &gs) : DebugOnlyCheck(gs), method(ref) {}

ConstMethodData::ConstMethodData(const Method &ref, const GlobalState &gs) : DebugOnlyCheck(gs), method(ref) {}

Method *MethodData::operator->() {
    runDebugOnlyCheck();
    return &method;
};

const Method *MethodData::operator->() const {
    runDebugOnlyCheck();
    return &method;
};

const Method *ConstMethodData::operator->() const {
    runDebugOnlyCheck();
    return &method;
};

FieldData::FieldData(Field &ref, GlobalState &gs) : DebugOnlyCheck(gs), field(ref) {}

ConstFieldData::ConstFieldData(const Field &ref, const GlobalState &gs) : DebugOnlyCheck(gs), field(ref) {}

Field *FieldData::operator->() {
    runDebugOnlyCheck();
    return &field;
};

const Field *FieldData::operator->() const {
    runDebugOnlyCheck();
    return &field;
};

const Field *ConstFieldData::operator->() const {
    runDebugOnlyCheck();
    return &field;
};

TypeParameterData::TypeParameterData(TypeParameter &ref, GlobalState &gs) : DebugOnlyCheck(gs), typeParam(ref) {}

ConstTypeParameterData::ConstTypeParameterData(const TypeParameter &ref, const GlobalState &gs)
    : DebugOnlyCheck(gs), typeParam(ref) {}

TypeParameter *TypeParameterData::operator->() {
    runDebugOnlyCheck();
    return &typeParam;
};

const TypeParameter *TypeParameterData::operator->() const {
    runDebugOnlyCheck();
    return &typeParam;
};

const TypeParameter *ConstTypeParameterData::operator->() const {
    runDebugOnlyCheck();
    return &typeParam;
};

bool Method::hasIntrinsic() const {
    return this->intrinsicOffset != INVALID_INTRINSIC_OFFSET;
}

vector<NameRef> IntrinsicMethod::dispatchesTo() const {
    return {};
}

const IntrinsicMethod *Method::getIntrinsic() const {
    if (this->intrinsicOffset == INVALID_INTRINSIC_OFFSET) {
        return nullptr;
    }

    return intrinsicMethods()[this->intrinsicOffset - FIRST_VALID_INTRINSIC_OFFSET].impl;
}

} // namespace sorbet::core
