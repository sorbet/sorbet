#include "core/Symbols.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "common/JSON.h"
#include "common/Levenstein.h"
#include "common/formatting.h"
#include "common/sort.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Types.h"
#include "core/errors/internal.h"
#include "core/hashing/hashing.h"
#include <string>

template class std::vector<sorbet::core::TypeAndOrigins>;
template class std::vector<std::pair<sorbet::core::NameRef, sorbet::core::SymbolRef>>;
template class std::vector<const sorbet::core::Symbol *>;

namespace sorbet::core {

using namespace std;

namespace {
constexpr string_view COLON_SEPARATOR = "::"sv;
constexpr string_view HASH_SEPARATOR = "#"sv;

string showInternal(const GlobalState &gs, core::SymbolRef owner, core::NameRef name, string_view separator) {
    if (!owner.exists() || owner == Symbols::root() || owner.data(gs)->name.isPackagerName(gs)) {
        return name.show(gs);
    }
    ENFORCE(owner != core::Symbols::PackageRegistry());
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

vector<TypePtr> Symbol::selfTypeArgs(const GlobalState &gs) const {
    ENFORCE(isClassOrModule()); // should be removed when we have generic methods
    vector<TypePtr> targs;
    for (auto tm : typeMembers()) {
        auto tmData = tm.data(gs);
        if (tmData->isFixed()) {
            auto *lambdaParam = cast_type<LambdaParam>(tmData->resultType);
            ENFORCE(lambdaParam != nullptr);
            targs.emplace_back(lambdaParam->upperBound);
        } else {
            targs.emplace_back(make_type<SelfTypeParam>(tm));
        }
    }
    return targs;
}
TypePtr Symbol::selfType(const GlobalState &gs) const {
    ENFORCE(isClassOrModule());
    // todo: in dotty it made sense to cache those.
    if (typeMembers().empty()) {
        return externalType();
    } else {
        return make_type<AppliedType>(ref(gs).asClassOrModuleRef(), selfTypeArgs(gs));
    }
}

TypePtr Symbol::externalType() const {
    ENFORCE_NO_TIMER(resultType);
    if (resultType == nullptr) {
        // Don't return nullptr in prod builds, which would cause a disruptive crash
        // Emit a metric and return untyped instead.
        prodCounterInc("symbol.externalType.nullptr");
        return Types::untypedUntracked();
    }
    return resultType;
}

TypePtr Symbol::unsafeComputeExternalType(GlobalState &gs) {
    ENFORCE_NO_TIMER(isClassOrModule());
    if (resultType != nullptr) {
        return resultType;
    }

    // note that sometimes resultType is set externally to not be a result of this computation
    // this happens e.g. in case this is a stub class
    auto ref = this->ref(gs).asClassOrModuleRef();
    if (typeMembers().empty()) {
        resultType = make_type<ClassType>(ref);
    } else {
        vector<TypePtr> targs;
        targs.reserve(typeMembers().size());

        // Special-case covariant stdlib generics to have their types
        // defaulted to `T.untyped`. This set *should not* grow over time.
        bool isStdlibGeneric = ref == core::Symbols::Hash() || ref == core::Symbols::Array() ||
                               ref == core::Symbols::Set() || ref == core::Symbols::Range() ||
                               ref == core::Symbols::Enumerable() || ref == core::Symbols::Enumerator();

        for (auto &tm : typeMembers()) {
            auto tmData = tm.data(gs);
            auto *lambdaParam = cast_type<LambdaParam>(tmData->resultType);
            ENFORCE(lambdaParam != nullptr);

            if (isStdlibGeneric) {
                // For backwards compatibility, instantiate stdlib generics
                // with T.untyped.
                targs.emplace_back(Types::untyped(gs, ref));
            } else if (tmData->isFixed() || tmData->isCovariant()) {
                // Default fixed or covariant parameters to their upper
                // bound.
                targs.emplace_back(lambdaParam->upperBound);
            } else if (tmData->isInvariant()) {
                // We instantiate Invariant type members as T.untyped as
                // this will behave a bit like a unification variable with
                // Types::glb.
                targs.emplace_back(Types::untyped(gs, ref));
            } else {
                // The remaining case is a contravariant parameter, which
                // gets defaulted to its lower bound.
                targs.emplace_back(lambdaParam->lowerBound);
            }
        }

        resultType = make_type<AppliedType>(ref, targs);
    }
    return resultType;
}

bool Symbol::derivesFrom(const GlobalState &gs, ClassOrModuleRef sym) const {
    if (isClassOrModuleLinearizationComputed()) {
        for (SymbolRef a : mixins()) {
            if (a == sym) {
                return true;
            }
        }
    } else {
        for (SymbolRef a : mixins()) {
            if (a == sym || a.data(gs)->derivesFrom(gs, sym)) {
                return true;
            }
        }
    }
    if (this->superClass().exists()) {
        return SymbolRef(sym) == this->superClass() || this->superClass().data(gs)->derivesFrom(gs, sym);
    }
    return false;
}

SymbolRef Symbol::ref(const GlobalState &gs) const {
    u4 distance = 0;
    auto type = SymbolRef::Kind::ClassOrModule;
    if (isClassOrModule()) {
        type = SymbolRef::Kind::ClassOrModule;
        distance = this - gs.classAndModules.data();
    } else if (isMethod()) {
        type = SymbolRef::Kind::Method;
        distance = this - gs.methods.data();
    } else if (isField() || isStaticField()) {
        type = SymbolRef::Kind::FieldOrStaticField;
        distance = this - gs.fields.data();
    } else if (isTypeMember()) {
        type = SymbolRef::Kind::TypeMember;
        distance = this - gs.typeMembers.data();
    } else if (isTypeArgument()) {
        type = SymbolRef::Kind::TypeArgument;
        distance = this - gs.typeArguments.data();
    } else {
        ENFORCE(false, "Invalid/unrecognized symbol type");
    }

    return SymbolRef(gs, type, distance);
}

bool SymbolRef::isField(const GlobalState &gs) const {
    return isFieldOrStaticField() && data(gs)->isField();
}
bool SymbolRef::isStaticField(const GlobalState &gs) const {
    return isFieldOrStaticField() && data(gs)->isStaticField();
}

SymbolData SymbolRef::data(GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    return dataAllowingNone(gs);
}

SymbolData SymbolRef::dataAllowingNone(GlobalState &gs) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            ENFORCE_NO_TIMER(classOrModuleIndex() < gs.classAndModules.size());
            return SymbolData(gs.classAndModules[classOrModuleIndex()], gs);
        case SymbolRef::Kind::Method:
            ENFORCE_NO_TIMER(methodIndex() < gs.methods.size());
            return SymbolData(gs.methods[methodIndex()], gs);
        case SymbolRef::Kind::FieldOrStaticField:
            ENFORCE_NO_TIMER(fieldIndex() < gs.fields.size());
            return SymbolData(gs.fields[fieldIndex()], gs);
        case SymbolRef::Kind::TypeArgument:
            ENFORCE_NO_TIMER(typeArgumentIndex() < gs.typeArguments.size());
            return SymbolData(gs.typeArguments[typeArgumentIndex()], gs);
        case SymbolRef::Kind::TypeMember:
            ENFORCE_NO_TIMER(typeMemberIndex() < gs.typeMembers.size());
            return SymbolData(gs.typeMembers[typeMemberIndex()], gs);
    }
}

ConstSymbolData SymbolRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    return dataAllowingNone(gs);
}

ConstSymbolData SymbolRef::dataAllowingNone(const GlobalState &gs) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            ENFORCE_NO_TIMER(classOrModuleIndex() < gs.classAndModules.size());
            return ConstSymbolData(gs.classAndModules[classOrModuleIndex()], gs);
        case SymbolRef::Kind::Method:
            ENFORCE_NO_TIMER(methodIndex() < gs.methods.size());
            return ConstSymbolData(gs.methods[methodIndex()], gs);
        case SymbolRef::Kind::FieldOrStaticField:
            ENFORCE_NO_TIMER(fieldIndex() < gs.fields.size());
            return ConstSymbolData(gs.fields[fieldIndex()], gs);
        case SymbolRef::Kind::TypeArgument:
            ENFORCE_NO_TIMER(typeArgumentIndex() < gs.typeArguments.size());
            return ConstSymbolData(gs.typeArguments[typeArgumentIndex()], gs);
        case SymbolRef::Kind::TypeMember:
            ENFORCE_NO_TIMER(typeMemberIndex() < gs.typeMembers.size());
            return ConstSymbolData(gs.typeMembers[typeMemberIndex()], gs);
    }
}

SymbolData ClassOrModuleRef::dataAllowingNone(GlobalState &gs) const {
    ENFORCE_NO_TIMER(_id < gs.classAndModulesUsed());
    return SymbolData(gs.classAndModules[_id], gs);
}

SymbolData ClassOrModuleRef::data(GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    return dataAllowingNone(gs);
}

ConstSymbolData ClassOrModuleRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    return dataAllowingNone(gs);
}

ConstSymbolData ClassOrModuleRef::dataAllowingNone(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(_id < gs.classAndModulesUsed());
    return ConstSymbolData(gs.classAndModules[_id], gs);
}

SymbolData MethodRef::data(GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.methodsUsed());
    return SymbolData(gs.methods[_id], gs);
}

ConstSymbolData MethodRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.methodsUsed());
    return ConstSymbolData(gs.methods[_id], gs);
}

SymbolData FieldRef::data(GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.fieldsUsed());
    return SymbolData(gs.fields[_id], gs);
}

ConstSymbolData FieldRef::dataAllowingNone(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(_id < gs.fieldsUsed());
    return ConstSymbolData(gs.fields[_id], gs);
}

ConstSymbolData FieldRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    return dataAllowingNone(gs);
}

SymbolData TypeMemberRef::data(GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.typeMembersUsed());
    return SymbolData(gs.typeMembers[_id], gs);
}

ConstSymbolData TypeMemberRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.typeMembersUsed());
    return ConstSymbolData(gs.typeMembers[_id], gs);
}

SymbolData TypeArgumentRef::data(GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.typeArgumentsUsed());
    return SymbolData(gs.typeArguments[_id], gs);
}

ConstSymbolData TypeArgumentRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(_id < gs.typeArgumentsUsed());
    return ConstSymbolData(gs.typeArguments[_id], gs);
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

SymbolRef::SymbolRef(const GlobalState &from, SymbolRef::Kind kind, u4 id)
    : _id((id << KIND_BITS) | static_cast<u4>(kind)) {
    // If this fails, the symbol table is too big :(
    ENFORCE_NO_TIMER(id <= MAX_ID);
}
SymbolRef::SymbolRef(GlobalState const *from, SymbolRef::Kind kind, u4 id)
    : _id((id << KIND_BITS) | static_cast<u4>(kind)) {
    // If this fails, the symbol table is too big :(
    ENFORCE_NO_TIMER(id <= MAX_ID);
}

SymbolRef::SymbolRef(ClassOrModuleRef kls) : SymbolRef(nullptr, SymbolRef::Kind::ClassOrModule, kls.id()) {}

SymbolRef::SymbolRef(MethodRef kls) : SymbolRef(nullptr, SymbolRef::Kind::Method, kls.id()) {}

SymbolRef::SymbolRef(FieldRef field) : SymbolRef(nullptr, SymbolRef::Kind::FieldOrStaticField, field.id()) {}

SymbolRef::SymbolRef(TypeMemberRef typeMember) : SymbolRef(nullptr, SymbolRef::Kind::TypeMember, typeMember.id()) {}

SymbolRef::SymbolRef(TypeArgumentRef typeArg) : SymbolRef(nullptr, SymbolRef::Kind::TypeArgument, typeArg.id()) {}

ClassOrModuleRef::ClassOrModuleRef(const GlobalState &from, u4 id) : _id(id) {}

MethodRef::MethodRef(const GlobalState &from, u4 id) : _id(id) {}

FieldRef::FieldRef(const GlobalState &from, u4 id) : _id(id) {}

TypeMemberRef::TypeMemberRef(const GlobalState &from, u4 id) : _id(id) {}

TypeArgumentRef::TypeArgumentRef(const GlobalState &from, u4 id) : _id(id) {}

string SymbolRef::show(const GlobalState &gs) const {
    switch (kind()) {
        case Kind::ClassOrModule:
            return asClassOrModuleRef().show(gs);
        case Kind::Method:
            return asMethodRef().show(gs);
        case Kind::FieldOrStaticField:
            return asFieldRef().show(gs);
        case Kind::TypeArgument:
            return asTypeArgumentRef().show(gs);
        case Kind::TypeMember:
            return asTypeMemberRef().show(gs);
    }
}

string ClassOrModuleRef::show(const GlobalState &gs) const {
    auto sym = dataAllowingNone(gs);
    if (sym->isSingletonClass(gs)) {
        auto attached = sym->attachedClass(gs);
        if (attached.exists()) {
            return fmt::format("T.class_of({})", attached.show(gs));
        }
    }

    // Make sure that we get nice error messages for things involving the proc sig builders.
    if (sym->name == core::Names::Constants::DeclBuilderForProcs()) {
        return "T.proc";
    }

    if (sym->owner == core::Symbols::PackageRegistry()) {
        // Pretty print package name (only happens when `--stripe-packages` is enabled)
        if (sym->name.isPackagerName(gs)) {
            auto nameStr = sym->name.shortName(gs);
            if (sym->name.isPackagerPrivateName(gs)) {
                // Foo_Bar_Package_Private => Foo::Bar
                // Remove _Package_Private before de-munging
                return absl::StrReplaceAll(nameStr.substr(0, nameStr.size() - core::PACKAGE_PRIVATE_SUFFIX.size()),
                                           {{"_", "::"}});
            } else {
                // Foo_Bar_Package => Foo::Bar
                // Remove _Package before de-munging
                return absl::StrReplaceAll(nameStr.substr(0, nameStr.size() - core::PACKAGE_SUFFIX.size()),
                                           {{"_", "::"}});
            }
        }
    }

    if (sym->name == core::Names::Constants::AttachedClass()) {
        auto attached = sym->owner.data(gs)->attachedClass(gs);
        ENFORCE(attached.exists());
        return fmt::format("T.attached_class (of {})", attached.show(gs));
    }

    return showInternal(gs, sym->owner, sym->name, COLON_SEPARATOR);
}

string MethodRef::show(const GlobalState &gs) const {
    auto sym = data(gs);
    if (sym->owner.isClassOrModule() && sym->owner.data(gs)->isSingletonClass(gs)) {
        return absl::StrCat(sym->owner.data(gs)->attachedClass(gs).show(gs), ".", sym->name.show(gs));
    }
    return showInternal(gs, sym->owner, sym->name, HASH_SEPARATOR);
}

string FieldRef::show(const GlobalState &gs) const {
    auto sym = dataAllowingNone(gs);
    return showInternal(gs, sym->owner, sym->name, sym->isStaticField() ? COLON_SEPARATOR : HASH_SEPARATOR);
}

string TypeArgumentRef::show(const GlobalState &gs) const {
    auto sym = data(gs);
    return showInternal(gs, sym->owner, sym->name, HASH_SEPARATOR);
}

string TypeMemberRef::show(const GlobalState &gs) const {
    auto sym = data(gs);
    if (sym->name == core::Names::Constants::AttachedClass()) {
        auto attached = sym->owner.data(gs)->attachedClass(gs);
        ENFORCE(attached.exists());
        return fmt::format("T.attached_class (of {})", attached.show(gs));
    }
    return showInternal(gs, sym->owner, sym->name, COLON_SEPARATOR);
}

TypePtr ArgInfo::argumentTypeAsSeenByImplementation(Context ctx, core::TypeConstraint &constr) const {
    auto owner = ctx.owner;
    auto klass = owner.enclosingClass(ctx);
    auto instantiated = Types::resultTypeAsSeenFrom(ctx, type, klass, klass, klass.data(ctx)->selfTypeArgs(ctx));
    if (instantiated == nullptr) {
        instantiated = core::Types::untyped(ctx, owner);
    }
    if (owner.data(ctx)->isGenericMethod()) {
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

bool Symbol::addMixin(const GlobalState &gs, ClassOrModuleRef sym, std::optional<u2> index) {
    ENFORCE(isClassOrModule());
    // Note: Symbols without an explicit declaration may not have class or module set. They default to modules in
    // GlobalPass.cc. We also do not complain if the mixin is BasicObject.
    bool isValidMixin = !sym.data(gs)->isClassModuleSet() || sym.data(gs)->isClassOrModuleModule() ||
                        sym == core::Symbols::BasicObject();

    if (!isClassOrModuleLinearizationComputed()) {
        // Symbol hasn't been linearized yet, so add symbol unconditionally (order matters, so dupes are OK and
        // semantically important!)
        // This is the 99% common case.
        if (index.has_value()) {
            auto i = index.value();
            ENFORCE(mixins_.size() > i);
            ENFORCE(!mixins_[i].exists());
            mixins_[i] = sym;
        } else {
            mixins_.emplace_back(sym);
        }
    } else {
        ENFORCE(!index.has_value());
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
                mixins_.emplace_back(sym);
                unsetClassOrModuleLinearizationComputed();
            }
        }
    }
    return isValidMixin;
}

u2 Symbol::addStubMixin(const GlobalState &gs) {
    ENFORCE(isClassOrModule());
    mixins_.emplace_back(ClassOrModuleRef());
    ENFORCE(mixins_.size() < numeric_limits<u2>::max());
    return mixins_.size() - 1;
}

SymbolRef Symbol::findMember(const GlobalState &gs, NameRef name) const {
    auto ret = findMemberNoDealias(gs, name);
    if (ret.exists()) {
        return ret.data(gs)->dealias(gs);
    }
    return ret;
}

SymbolRef Symbol::findMemberNoDealias(const GlobalState &gs, NameRef name) const {
    histogramInc("find_member_scope_size", members().size());
    auto fnd = members().find(name);
    if (fnd == members().end()) {
        return Symbols::noSymbol();
    }
    return fnd->second;
}

SymbolRef Symbol::findMemberTransitive(const GlobalState &gs, NameRef name) const {
    return findMemberTransitiveInternal(gs, name, Flags::NONE, Flags::NONE, 100);
}

SymbolRef Symbol::findConcreteMethodTransitive(const GlobalState &gs, NameRef name) const {
    return findMemberTransitiveInternal(gs, name, Flags::METHOD | Flags::METHOD_ABSTRACT, Flags::METHOD, 100);
}

SymbolRef Symbol::findMemberTransitiveInternal(const GlobalState &gs, NameRef name, u4 mask, u4 flags,
                                               int maxDepth) const {
    ENFORCE(this->isClassOrModule());
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

    SymbolRef result = findMember(gs, name);
    if (result.exists()) {
        if (mask == 0 || (result.data(gs)->flags & mask) == flags) {
            return result;
        }
    }
    if (isClassOrModuleLinearizationComputed()) {
        for (auto it = this->mixins().begin(); it != this->mixins().end(); ++it) {
            ENFORCE(it->exists());
            if (isClassOrModuleLinearizationComputed()) {
                result = it->data(gs)->findMember(gs, name);
                if (result.exists()) {
                    if (mask == 0 || (result.data(gs)->flags & mask) == flags) {
                        return result;
                    }
                }
                result = core::Symbols::noSymbol();
            }
        }
    } else {
        for (auto it = this->mixins().rbegin(); it != this->mixins().rend(); ++it) {
            ENFORCE(it->exists());
            result = it->data(gs)->findMemberTransitiveInternal(gs, name, mask, flags, maxDepth - 1);
            if (result.exists()) {
                return result;
            }
        }
    }
    if (this->superClass().exists()) {
        return this->superClass().data(gs)->findMemberTransitiveInternal(gs, name, mask, flags, maxDepth - 1);
    }
    return Symbols::noSymbol();
}

vector<Symbol::FuzzySearchResult> Symbol::findMemberFuzzyMatch(const GlobalState &gs, NameRef name,
                                                               int betterThan) const {
    vector<Symbol::FuzzySearchResult> res;
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
            vector<Symbol::FuzzySearchResult> constant_matches = findMemberFuzzyMatchConstant(gs, name, betterThan);
            res.insert(res.end(), constant_matches.begin(), constant_matches.end());
        }
    } else if (name.kind() == NameKind::CONSTANT) {
        res = findMemberFuzzyMatchConstant(gs, name, betterThan);
    }
    return res;
}

vector<Symbol::FuzzySearchResult> Symbol::findMemberFuzzyMatchConstant(const GlobalState &gs, NameRef name,
                                                                       int betterThan) const {
    // Performance of this method is bad, to say the least.
    // It's written under assumption that it's called rarely
    // and that it's worth spending a lot of time finding a good candidate in ALL scopes.
    // It may return multiple candidates:
    //   - best candidate per every outer scope if it's better than all the candidates in inner scope
    //   - globally best candidate in ALL scopes.
    vector<Symbol::FuzzySearchResult> result;
    FuzzySearchResult best;
    best.symbol = Symbols::noSymbol();
    best.name = NameRef::noName();
    best.distance = betterThan;
    auto currentName = name.shortName(gs);
    if (best.distance < 0) {
        best.distance = 1 + (currentName.size() / 2);
    }

    // Find the closest by following outer scopes
    {
        SymbolRef base = ref(gs);
        do {
            // follow outer scopes

            // find scopes that would be considered for search
            vector<ClassOrModuleRef> candidateScopes;
            vector<Symbol::FuzzySearchResult> scopeBest;
            candidateScopes.emplace_back(base.asClassOrModuleRef());
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
        vector<Symbol::FuzzySearchResult> globalBest;
        vector<ClassOrModuleRef> yetToGoDeeper;
        yetToGoDeeper.emplace_back(Symbols::root());
        while (!yetToGoDeeper.empty()) {
            const ClassOrModuleRef thisIter = yetToGoDeeper.back();
            yetToGoDeeper.pop_back();
            for (auto member : thisIter.data(gs)->members()) {
                if (member.second.exists() && member.first.exists() && member.first.kind() == NameKind::CONSTANT &&
                    (member.first.dataCnst(gs)->original.kind() == NameKind::UTF8 || member.first.isPackagerName(gs))) {
                    if (member.second.isClassOrModule() &&
                        member.second.data(gs)->derivesFrom(gs, core::Symbols::StubModule())) {
                        continue;
                    }
                    // Mangled packager names are not matched, but we do descend into them to search
                    // deeper.
                    if (member.first.dataCnst(gs)->original.kind() == NameKind::UTF8) {
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

Symbol::FuzzySearchResult Symbol::findMemberFuzzyMatchUTF8(const GlobalState &gs, NameRef name, int betterThan) const {
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
bool isHiddenFromPrinting(const GlobalState &gs, const Symbol &symbol) {
    if (symbol.ref(gs).isSynthetic()) {
        return true;
    }
    if (symbol.locs().empty()) {
        return true;
    }
    for (auto loc : symbol.locs()) {
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

string_view getVariance(ConstSymbolData &sym) {
    if (sym->isCovariant()) {
        return "(+)"sv;
    } else if (sym->isContravariant()) {
        return "(-)"sv;
    } else if (sym->isInvariant()) {
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
    return absl::StrCat(ownerStr, includeOwner ? separator : "", name.showRaw(gs)); // TODO(jez) includeOwner required?
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
    return showFullNameInternal(gs, sym->owner, sym->name, sym->isStaticField() ? COLON_SEPARATOR : HASH_SEPARATOR);
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
    return toStringFullNameInternal(gs, sym->owner, sym->name, sym->isStaticField() ? COLON_SEPARATOR : HASH_SEPARATOR);
}

string TypeArgumentRef::toStringFullName(const GlobalState &gs) const {
    auto sym = data(gs);
    return toStringFullNameInternal(gs, sym->owner, sym->name, HASH_SEPARATOR);
}

string TypeMemberRef::toStringFullName(const GlobalState &gs) const {
    auto sym = data(gs);
    return toStringFullNameInternal(gs, sym->owner, sym->name, COLON_SEPARATOR);
}

bool Symbol::isPrintable(const GlobalState &gs) const {
    if (!isHiddenFromPrinting(gs, *this)) {
        return true;
    }

    for (auto childPair : this->members()) {
        if (childPair.first == Names::singleton() || childPair.first == Names::attached() ||
            childPair.first == Names::mixedInClassMethods()) {
            continue;
        }

        if (childPair.second.data(gs)->isPrintable(gs)) {
            return true;
        }
    }

    return false;
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
    } else if (sym->isClassOrModuleClass()) {
        return "class"sv;
    } else {
        return "module"sv;
    }
}

string_view FieldRef::showKind(const GlobalState &gs) const {
    auto sym = dataAllowingNone(gs);
    if (sym->isStaticField()) {
        if (sym->isTypeAlias()) {
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

    auto typeMembers = sym->typeMembers();
    auto it =
        remove_if(typeMembers.begin(), typeMembers.end(), [&gs](auto &sym) -> bool { return sym.data(gs)->isFixed(); });
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

    if (sym->isClassOrModulePrivate()) {
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

        if (!showFull && !pair.second.data(gs)->isPrintable(gs)) {
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

    if (sym->isMethodPrivate()) {
        methodFlags.emplace_back("private");
    } else if (sym->isMethodProtected()) {
        methodFlags.emplace_back("protected");
    }

    if (sym->isAbstract()) {
        methodFlags.emplace_back("abstract");
    }
    if (sym->isOverridable()) {
        methodFlags.emplace_back("overridable");
    }
    if (sym->isOverride()) {
        methodFlags.emplace_back("override");
    }
    if (sym->isIncompatibleOverride()) {
        methodFlags.emplace_back("allow_incompatible");
    }
    if (sym->isFinalMethod()) {
        methodFlags.emplace_back("final");
    }

    if (!methodFlags.empty()) {
        fmt::format_to(std::back_inserter(buf), " : {}",
                       fmt::map_join(methodFlags, "|", [](const auto &flag) { return flag; }));
    }

    auto typeMembers = sym->typeArguments();
    auto it =
        remove_if(typeMembers.begin(), typeMembers.end(), [&gs](auto &sym) -> bool { return sym.data(gs)->isFixed(); });
    typeMembers.erase(it, typeMembers.end());
    if (!typeMembers.empty()) {
        fmt::format_to(std::back_inserter(buf), "[{}]", fmt::map_join(typeMembers, ", ", [&](auto symb) {
                           auto name = symb.data(gs)->name;
                           return showRaw ? name.showRaw(gs) : name.show(gs);
                       }));
    }
    fmt::format_to(std::back_inserter(buf), " ({})",
                   fmt::map_join(sym->arguments(), ", ", [&](const auto &symb) { return symb.argumentName(gs); }));

    printResultType(gs, buf, sym->resultType, tabs, showRaw);
    printLocs(gs, buf, sym->locs(), showRaw);

    if (sym->rebind().exists()) {
        fmt::format_to(std::back_inserter(buf), " rebindTo {}",
                       showRaw ? sym->rebind().toStringFullName(gs) : sym->rebind().showFullName(gs));
    }

    ENFORCE(!absl::c_any_of(to_string(buf), [](char c) { return c == '\n'; }));
    fmt::format_to(std::back_inserter(buf), "\n");
    for (auto pair : sym->membersStableOrderSlow(gs)) {
        ENFORCE_NO_TIMER(pair.second.exists());
        // These should only show up in classes.
        ENFORCE_NO_TIMER(pair.first != Names::singleton() && pair.first != Names::attached() &&
                         pair.first != Names::mixedInClassMethods());

        if (!showFull && !pair.second.data(gs)->isPrintable(gs)) {
            continue;
        }

        auto str = pair.second.toStringWithOptions(gs, tabs + 1, showFull, showRaw);
        ENFORCE(!str.empty());
        fmt::format_to(std::back_inserter(buf), "{}", move(str));
    }

    for (auto &arg : sym->arguments()) {
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
    if (sym->isStaticField() && sym->isStaticFieldPrivate()) {
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
    ENFORCE_NO_TIMER(sym->members().empty());

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
    ENFORCE_NO_TIMER(sym->members().empty());

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
    if (flags.isKeyword) {
        return name.shortName(gs);
    } else {
        // positional arg
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

bool Symbol::isSingletonClass(const GlobalState &gs) const {
    bool isSingleton = isClassOrModule() && (isSingletonName(gs, name) || isMangledSingletonName(gs, name));
    DEBUG_ONLY(if (ref(gs) != Symbols::untyped()) { // Symbol::untyped is attached to itself
        if (isSingleton) {
            ENFORCE(attachedClass(gs).exists());
        } else {
            ENFORCE(!attachedClass(gs).exists());
        }
    });
    return isSingleton;
}

ClassOrModuleRef Symbol::singletonClass(GlobalState &gs) {
    auto singleton = lookupSingletonClass(gs);
    if (singleton.exists()) {
        return singleton;
    }
    SymbolRef selfRef = this->ref(gs);

    // avoid using `this` after the call to gs.enterTypeMember
    auto selfLoc = this->loc();

    NameRef singletonName = gs.freshNameUnique(UniqueNameKind::Singleton, this->name, 1);
    singleton = gs.enterClassSymbol(this->loc(), this->owner.asClassOrModuleRef(), singletonName);
    SymbolData singletonInfo = singleton.data(gs);

    prodCounterInc("types.input.singleton_classes.total");
    singletonInfo->members()[Names::attached()] = selfRef;
    singletonInfo->setSuperClass(Symbols::todo());
    singletonInfo->setIsModule(false);

    auto tp = gs.enterTypeMember(selfLoc, singleton, Names::Constants::AttachedClass(), Variance::CoVariant);

    // Initialize the bounds of AttachedClass as todo, as they will be updated
    // to the externalType of the attached class for the upper bound, and bottom
    // for the lower bound in the ResolveSignaturesWalk pass of the resolver.
    auto todo = make_type<ClassType>(Symbols::todo());
    tp.data(gs)->resultType = make_type<LambdaParam>(tp, todo, todo);

    selfRef.data(gs)->members()[Names::singleton()] = singleton;
    return singleton;
}

ClassOrModuleRef Symbol::lookupSingletonClass(const GlobalState &gs) const {
    ENFORCE(this->isClassOrModule());
    ENFORCE(this->name.isClassName(gs));

    SymbolRef selfRef = this->ref(gs);
    if (selfRef == Symbols::untyped()) {
        return Symbols::untyped();
    }

    return findMember(gs, Names::singleton()).asClassOrModuleRef();
}

ClassOrModuleRef Symbol::attachedClass(const GlobalState &gs) const {
    ENFORCE(this->isClassOrModule());
    if (this->ref(gs) == Symbols::untyped()) {
        return Symbols::untyped();
    }

    SymbolRef singleton = findMember(gs, Names::attached());
    return singleton.asClassOrModuleRef();
}

ClassOrModuleRef Symbol::topAttachedClass(const GlobalState &gs) const {
    ClassOrModuleRef classSymbol = this->ref(gs).asClassOrModuleRef();

    while (true) {
        auto attachedClass = classSymbol.data(gs)->attachedClass(gs);
        if (!attachedClass.exists()) {
            break;
        }
        classSymbol = attachedClass;
    }

    return classSymbol;
}

void Symbol::recordSealedSubclass(MutableContext ctx, ClassOrModuleRef subclass) {
    ENFORCE(this->isClassOrModuleSealed(), "Class is not marked sealed: {}", ref(ctx).show(ctx));
    ENFORCE(subclass.exists(), "Can't record sealed subclass for {} when subclass doesn't exist", ref(ctx).show(ctx));

    // Avoid using a clobbered `this` pointer, as `singletonClass` can cause the symbol table to move.
    auto selfRef = this->ref(ctx);

    // We record sealed subclasses on a magical method called core::Names::sealedSubclasses(). This is so we don't
    // bloat the `sizeof class Symbol` with an extra field that most class sybmols will never use.
    // Note: We had hoped to ALSO implement this method in the runtime, but we couldn't think of a way to make it work
    // that didn't require running with the help of Stripe's autoloader, specifically because we might want to allow
    // subclassing a sealed class across multiple files, not just one file.
    auto classOfSubclass = subclass.data(ctx)->singletonClass(ctx);
    auto sealedSubclasses =
        selfRef.data(ctx)->lookupSingletonClass(ctx).data(ctx)->findMember(ctx, core::Names::sealedSubclasses());

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

const InlinedVector<Loc, 2> &Symbol::sealedLocs(const GlobalState &gs) const {
    ENFORCE(this->isClassOrModuleSealed(), "Class is not marked sealed: {}", ref(gs).show(gs));
    auto sealedSubclasses = this->lookupSingletonClass(gs).data(gs)->findMember(gs, core::Names::sealedSubclasses());
    auto &result = sealedSubclasses.data(gs)->locs();
    ENFORCE(result.size() > 0);
    return result;
}

TypePtr Symbol::sealedSubclassesToUnion(const GlobalState &gs) const {
    ENFORCE(this->isClassOrModuleSealed(), "Class is not marked sealed: {}", ref(gs).show(gs));

    auto sealedSubclasses = this->lookupSingletonClass(gs).data(gs)->findMember(gs, core::Names::sealedSubclasses());

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
        result = Types::any(gs, make_type<ClassType>(subclass), result);
        currentClasses = orType->left;
    }

    ENFORCE(isa_type<ClassType>(currentClasses), "Last element of sealedSubclasses must be ClassType");
    auto lastClassType = cast_type_nonnull<ClassType>(currentClasses);
    auto subclass = lastClassType.symbol.data(gs)->attachedClass(gs);
    ENFORCE(subclass.exists());
    result = Types::any(gs, make_type<ClassType>(subclass), result);

    return result;
}

bool Symbol::hasSingleSealedSubclass(const GlobalState &gs) const {
    ENFORCE(this->isClassOrModuleSealed(), "Class is not marked sealed: {}", ref(gs).show(gs));

    auto sealedSubclasses = this->lookupSingletonClass(gs).data(gs)->findMember(gs, core::Names::sealedSubclasses());

    // When the sealed type is a class, it must also be abstract for there to be a single subclass.
    if (this->isClassOrModuleClass() && !this->isClassOrModuleAbstract()) {
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
void Symbol::recordRequiredAncestorInternal(GlobalState &gs, Symbol::RequiredAncestor &ancestor, NameRef prop) {
    ENFORCE(this->isClassOrModule(), "Symbol is not a class or module: {}", ref(gs).show(gs));

    // We store the required ancestors into a fake property called `<required-ancestors>`
    auto ancestors = this->findMember(gs, prop);
    if (!ancestors.exists()) {
        ancestors = gs.enterMethodSymbol(ancestor.loc, this->ref(gs).asClassOrModuleRef(), prop);
        ancestors.data(gs)->locs_.clear(); // Remove the original location

        // Create the return type tuple to store RequiredAncestor.symbol
        vector<TypePtr> tsymbols;
        ancestors.data(gs)->resultType = make_type<TupleType>(move(tsymbols));

        // Create the first argument typed as a tuple to store RequiredAncestor.origin
        auto &arg = gs.enterMethodArgumentSymbol(core::Loc::none(), ancestors.asMethodRef(), core::Names::arg());
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
    (cast_type<TupleType>(ancestors.data(gs)->arguments()[0].type))->elems.emplace_back(tOrigin);

    // Store the RequiredAncestor.loc
    ancestors.data(gs)->locs_.emplace_back(ancestor.loc);
}

// Locally required ancestors by this class or module
vector<Symbol::RequiredAncestor> Symbol::readRequiredAncestorsInternal(const GlobalState &gs, NameRef prop) const {
    ENFORCE(this->isClassOrModule(), "Symbol is not a class or module: {}", ref(gs).show(gs));

    vector<RequiredAncestor> res;

    auto ancestors = this->findMember(gs, prop);
    if (!ancestors.exists()) {
        // No ancestor was recorded for this class or module
        return res;
    }

    auto data = ancestors.data(gs);
    auto tSymbols = cast_type<TupleType>(data->resultType);
    auto tOrigins = cast_type<TupleType>(data->arguments()[0].type);
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
void Symbol::recordRequiredAncestor(GlobalState &gs, ClassOrModuleRef ancestor, Loc loc) {
    RequiredAncestor req = {this->ref(gs).asClassOrModuleRef(), ancestor, loc};
    recordRequiredAncestorInternal(gs, req, Names::requiredAncestors());
}

// Locally required ancestors by this class or module
vector<Symbol::RequiredAncestor> Symbol::requiredAncestors(const GlobalState &gs) const {
    return readRequiredAncestorsInternal(gs, Names::requiredAncestors());
}

// All required ancestors by this class or module
std::vector<Symbol::RequiredAncestor> Symbol::requiredAncestorsTransitiveInternal(GlobalState &gs,
                                                                                  std::vector<ClassOrModuleRef> &seen) {
    if (absl::c_find(seen, this->ref(gs).asClassOrModuleRef()) != seen.end()) {
        return requiredAncestors(gs); // Break recursive loops if we already visited this ancestor
    }
    seen.emplace_back(this->ref(gs).asClassOrModuleRef());

    for (auto ancst : requiredAncestors(gs)) {
        recordRequiredAncestorInternal(gs, ancst, Names::requiredAncestorsLin());
        for (auto sancst : ancst.symbol.data(gs)->requiredAncestorsTransitiveInternal(gs, seen)) {
            if (sancst.symbol != this->ref(gs).asClassOrModuleRef()) {
                recordRequiredAncestorInternal(gs, sancst, Names::requiredAncestorsLin());
            }
        }
    }

    auto parent = superClass();
    if (parent.exists()) {
        for (auto ancst : parent.data(gs)->requiredAncestorsTransitiveInternal(gs, seen)) {
            if (ancst.symbol != this->ref(gs).asClassOrModuleRef()) {
                recordRequiredAncestorInternal(gs, ancst, Names::requiredAncestorsLin());
            }
        }
    }

    for (auto mixin : mixins()) {
        for (auto ancst : mixin.data(gs)->requiredAncestors(gs)) {
            if (ancst.symbol != this->ref(gs).asClassOrModuleRef()) {
                recordRequiredAncestorInternal(gs, ancst, Names::requiredAncestorsLin());
            }
        }
    }

    return requiredAncestorsTransitive(gs);
}

// All required ancestors by this class or module
vector<Symbol::RequiredAncestor> Symbol::requiredAncestorsTransitive(const GlobalState &gs) const {
    ENFORCE(gs.requiresAncestorEnabled);
    return readRequiredAncestorsInternal(gs, Names::requiredAncestorsLin());
}

void Symbol::computeRequiredAncestorLinearization(GlobalState &gs) {
    ENFORCE(gs.requiresAncestorEnabled);
    ENFORCE(this->isClassOrModule(), "Symbol is not a class or module: {}", ref(gs).show(gs));
    std::vector<ClassOrModuleRef> seen;
    requiredAncestorsTransitiveInternal(gs, seen);
}

SymbolRef Symbol::dealiasWithDefault(const GlobalState &gs, int depthLimit, SymbolRef def) const {
    if (isa_type<AliasType>(resultType)) {
        auto alias = cast_type_nonnull<AliasType>(resultType);
        if (depthLimit == 0) {
            if (auto e = gs.beginError(loc(), errors::Internal::CyclicReferenceError)) {
                e.setHeader("Too many alias expansions for symbol {}, the alias is either too long or infinite. Next "
                            "expansion would have been to {}",
                            ref(gs).showFullName(gs), alias.symbol.showFullName(gs));
            }
            return def;
        }
        return alias.symbol.data(gs)->dealiasWithDefault(gs, depthLimit - 1, def);
    }
    return this->ref(gs);
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

u1 ArgInfo::ArgFlags::toU1() const {
    u1 flags = 0;
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

void ArgInfo::ArgFlags::setFromU1(u1 flags) {
    isKeyword = flags & 1;
    isRepeated = flags & 2;
    isDefault = flags & 4;
    isShadow = flags & 8;
    isBlock = flags & 16;
}

Symbol Symbol::deepCopy(const GlobalState &to, bool keepGsId) const {
    Symbol result;
    result.owner = this->owner;
    result.flags = this->flags;
    result.mixins_ = this->mixins_;
    result.resultType = this->resultType;
    result.name = NameRef(to, this->name);
    result.locs_ = this->locs_;
    result.typeParams = this->typeParams;
    if (keepGsId) {
        result.members_ = this->members_;
    } else {
        result.members_.reserve(this->members().size());
        for (auto &mem : this->members_) {
            result.members_[NameRef(to, mem.first)] = mem.second;
        }
    }
    result.arguments_.reserve(this->arguments_.size());
    for (auto &mem : this->arguments_) {
        auto &store = result.arguments_.emplace_back(mem.deepCopy());
        store.name = NameRef(to, mem.name);
    }
    result.superClassOrRebind = this->superClassOrRebind;
    result.intrinsic = this->intrinsic;
    return result;
}

int Symbol::typeArity(const GlobalState &gs) const {
    ENFORCE(this->isClassOrModule());
    int arity = 0;
    for (auto &ty : this->typeMembers()) {
        if (!ty.data(gs)->isFixed()) {
            ++arity;
        }
    }
    return arity;
}

void Symbol::sanityCheck(const GlobalState &gs) const {
    if (!debug_mode) {
        return;
    }
    SymbolRef current = this->ref(gs);
    if (current != Symbols::root()) {
        SymbolRef current2;
        switch (current.kind()) {
            case SymbolRef::Kind::ClassOrModule:
                current2 = const_cast<GlobalState &>(gs).enterClassSymbol(this->loc(), this->owner.asClassOrModuleRef(),
                                                                          this->name);
                break;
            case SymbolRef::Kind::Method:
                current2 = const_cast<GlobalState &>(gs).enterMethodSymbol(
                    this->loc(), this->owner.asClassOrModuleRef(), this->name);
                break;
            case SymbolRef::Kind::FieldOrStaticField:
                if (isField()) {
                    current2 = const_cast<GlobalState &>(gs).enterFieldSymbol(
                        this->loc(), this->owner.asClassOrModuleRef(), this->name);
                } else {
                    current2 = const_cast<GlobalState &>(gs).enterStaticFieldSymbol(
                        this->loc(), this->owner.asClassOrModuleRef(), this->name);
                }
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
        for (auto &e : members()) {
            ENFORCE_NO_TIMER(e.first.exists(), name.toString(gs) + " has a member symbol without a name");
            ENFORCE_NO_TIMER(e.second.exists(), name.toString(gs) + "." + e.first.toString(gs) +
                                                    " corresponds to a core::Symbols::noSymbol()");
        }
    }
    if (this->isMethod()) {
        if (isa_type<AliasType>(this->resultType)) {
            // If we have an alias method, we should never look at it's arguments;
            // we should instead look at the arguments of whatever we're aliasing.
            ENFORCE_NO_TIMER(this->arguments().empty(), ref(gs).show(gs));
        } else {
            ENFORCE_NO_TIMER(!this->arguments().empty(), ref(gs).show(gs));
        }
    }
}

ClassOrModuleRef MethodRef::enclosingClass(const GlobalState &gs) const {
    // Methods can only be owned by classes or modules.
    return data(gs)->owner.asClassOrModuleRef();
}

ClassOrModuleRef SymbolRef::enclosingClass(const GlobalState &gs) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            return asClassOrModuleRef();
        case SymbolRef::Kind::Method:
            return asMethodRef().enclosingClass(gs);
        case SymbolRef::Kind::FieldOrStaticField:
            // Fields can only be owned by classes or modules.
            return asFieldRef().data(gs)->owner.asClassOrModuleRef();
        case SymbolRef::Kind::TypeArgument:
            // Typeargs are owned by methods.
            return asTypeArgumentRef().data(gs)->owner.asMethodRef().enclosingClass(gs);
        case SymbolRef::Kind::TypeMember:
            // TypeMembers are only owned by classes or modules.
            return asTypeMemberRef().data(gs)->owner.asClassOrModuleRef();
    }
}

u4 Symbol::hash(const GlobalState &gs) const {
    u4 result = _hash(name.shortName(gs));
    result = mix(result, !this->resultType ? 0 : this->resultType.hash(gs));
    result = mix(result, this->flags);
    result = mix(result, this->owner._id);
    result = mix(result, this->superClassOrRebind.id());
    // argumentsOrMixins, typeParams, typeAliases
    if (!members().empty()) {
        // Rather than use membersStableOrderSlow, which is... slow..., use an order dictated by symbol ref ID.
        // It's faster to sort, and SymbolRef IDs are stable during hashing.
        vector<core::SymbolRef> membersToHash;
        membersToHash.reserve(this->members().size());
        for (auto e : this->members()) {
            if (e.second.exists() && !e.second.data(gs)->ignoreInHashing(gs)) {
                membersToHash.emplace_back(e.second);
            }
        }
        fast_sort(membersToHash, [](const auto &a, const auto &b) -> bool { return a.rawId() < b.rawId(); });
        for (auto member : membersToHash) {
            result = mix(result, _hash(member.data(gs)->name.shortName(gs)));
        }
    }
    for (const auto &e : mixins_) {
        if (e.exists() && !e.data(gs)->ignoreInHashing(gs)) {
            result = mix(result, _hash(e.data(gs)->name.shortName(gs)));
        }
    }
    for (const auto &arg : arguments_) {
        // If an argument's resultType changes, then the sig has changed.
        auto type = arg.type;
        if (!type) {
            type = Types::untypedUntracked();
        }
        result = mix(result, type.hash(gs));
        result = mix(result, _hash(arg.name.shortName(gs)));
    }
    for (const auto &e : typeParams) {
        if (e.exists() && !e.data(gs)->ignoreInHashing(gs)) {
            result = mix(result, _hash(e.data(gs)->name.shortName(gs)));
        }
    }

    return result;
}

u4 Symbol::methodShapeHash(const GlobalState &gs) const {
    ENFORCE(isMethod());

    u4 result = _hash(name.shortName(gs));
    result = mix(result, this->flags);
    result = mix(result, this->owner._id);
    result = mix(result, this->superClassOrRebind.id());
    result = mix(result, this->hasSig());
    for (auto &arg : this->methodArgumentHash(gs)) {
        result = mix(result, arg);
    }

    if (name == core::Names::unresolvedAncestors()) {
        // This is a synthetic method that encodes the superclasses of its owning class in its return type.
        // If the return type changes, we must take the slow path.
        ENFORCE(resultType);
        result = mix(result, resultType.hash(gs));
    }

    return result;
}

vector<u4> Symbol::methodArgumentHash(const GlobalState &gs) const {
    vector<u4> result;
    result.reserve(arguments().size());
    for (const auto &e : arguments()) {
        u4 arg = 0;
        // Changing name of keyword arg is a shape change.
        if (e.flags.isKeyword) {
            arg = mix(arg, _hash(e.name.shortName(gs)));
        }
        // Changing an argument from e.g. keyword to position-based is a shape change.
        result.push_back(mix(arg, e.flags.toU1()));
    }
    return result;
}

bool Symbol::ignoreInHashing(const GlobalState &gs) const {
    if (isClassOrModule()) {
        return superClass() == core::Symbols::StubModule();
    } else if (isMethod()) {
        return name.kind() == NameKind::UNIQUE && name.dataUnique(gs)->original == core::Names::staticInit();
    }
    return false;
}
Loc Symbol::loc() const {
    if (!locs_.empty()) {
        return locs_.back();
    }
    return Loc::none();
}

const InlinedVector<Loc, 2> &Symbol::locs() const {
    return locs_;
}

void Symbol::addLoc(const core::GlobalState &gs, core::Loc loc) {
    if (!loc.file().exists()) {
        return;
    }

    // We shouldn't add locs for <root> or <PackageRegistry>, otherwise it'll end up with a massive loc list (O(number
    // of files)). Those locs aren't useful, either.
    ENFORCE(ref(gs) != Symbols::root());
    ENFORCE(ref(gs) != Symbols::PackageRegistry());
    // We allow one loc (during class creation) for packages under package registry.
    ENFORCE(locs_.empty() || owner != Symbols::PackageRegistry());

    for (auto &existing : locs_) {
        if (existing.file() == loc.file()) {
            existing = loc;
            return;
        }
    }

    if (locs_.empty() || (loc.file().data(gs).sourceType == core::File::Type::Normal && !loc.file().data(gs).isRBI())) {
        if (this->loc().exists() && loc.file().data(gs).strictLevel >= this->loc().file().data(gs).strictLevel) {
            // The new loc is stricter; make it the new canonical loc.
            locs_.emplace_back(loc);
        } else {
            locs_.insert(locs_.begin(), loc);
        }
    } else {
        // This is an RBI file; continue to use existing loc as the canonical loc.
        // Insert just before end.
        locs_.insert(locs_.end() - 1, loc);
    }
}

vector<std::pair<NameRef, SymbolRef>> Symbol::membersStableOrderSlow(const GlobalState &gs) const {
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
        const auto &rhsLocs = rhs.second.data(gs)->locs();
        for (const auto lhsLoc : lhs.second.data(gs)->locs()) {
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
        return 0;
    });
    return result;
}

SymbolData::SymbolData(Symbol &ref, GlobalState &gs) : DebugOnlyCheck(gs), symbol(ref) {}

ConstSymbolData::ConstSymbolData(const Symbol &ref, const GlobalState &gs) : DebugOnlyCheck(gs), symbol(ref) {}

SymbolDataDebugCheck::SymbolDataDebugCheck(const GlobalState &gs)
    : gs(gs), symbolCountAtCreation(gs.symbolsUsedTotal()) {}

void SymbolDataDebugCheck::check() const {
    ENFORCE_NO_TIMER(symbolCountAtCreation == gs.symbolsUsedTotal());
}

Symbol *SymbolData::operator->() {
    runDebugOnlyCheck();
    return &symbol;
};

const Symbol *SymbolData::operator->() const {
    runDebugOnlyCheck();
    return &symbol;
};

const Symbol *ConstSymbolData::operator->() const {
    runDebugOnlyCheck();
    return &symbol;
};

} // namespace sorbet::core
