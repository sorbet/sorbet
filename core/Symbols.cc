#include "core/Symbols.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "common/JSON.h"
#include "common/Levenstein.h"
#include "common/formatting.h"
#include "common/sort.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/Hashing.h"
#include "core/Names.h"
#include "core/Types.h"
#include "core/errors/internal.h"
#include <string>

template class std::vector<sorbet::core::TypeAndOrigins>;
template class std::vector<std::pair<sorbet::core::NameRef, sorbet::core::SymbolRef>>;
template class std::vector<const sorbet::core::Symbol *>;

namespace sorbet::core {

using namespace std;

bool SymbolRef::operator==(const SymbolRef &rhs) const {
    return _id == rhs._id;
}

bool SymbolRef::operator!=(const SymbolRef &rhs) const {
    return !(rhs == *this);
}

vector<TypePtr> Symbol::selfTypeArgs(const GlobalState &gs) const {
    ENFORCE(isClassOrModule()); // should be removed when we have generic methods
    vector<TypePtr> targs;
    for (auto tm : typeMembers()) {
        auto tmData = tm.data(gs);
        if (tmData->isFixed()) {
            auto *lambdaParam = cast_type<LambdaParam>(tmData->resultType.get());
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
        return externalType(gs);
    } else {
        return make_type<AppliedType>(ref(gs), selfTypeArgs(gs));
    }
}

TypePtr Symbol::externalType(const GlobalState &gs) const {
    ENFORCE(isClassOrModule());
    if (!resultType) {
        // note that sometimes resultType is set externally to not be a result of this computation
        // this happens e.g. in case this is a stub class
        TypePtr newResultType;
        auto ref = this->ref(gs);
        if (typeMembers().empty()) {
            newResultType = make_type<ClassType>(ref);
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
                auto *lambdaParam = cast_type<LambdaParam>(tmData->resultType.get());
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

            newResultType = make_type<AppliedType>(ref, targs);
        }
        {
            // this method is supposed to be idempotent. The lines below implement "safe publication" of a value that is
            // safe to be used in presence of multiple threads running this tion concurrently
            auto mutableThis = const_cast<Symbol *>(this);
            shared_ptr<core::Type> current(nullptr);
            atomic_compare_exchange_weak(&mutableThis->resultType.store, &current, newResultType.store);
        }
        return externalType(gs);
    }
    return resultType;
}

bool Symbol::derivesFrom(const GlobalState &gs, SymbolRef sym) const {
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
        return sym == this->superClass() || this->superClass().data(gs)->derivesFrom(gs, sym);
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
        type = SymbolRef::Kind::Field;
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
        case SymbolRef::Kind::Field:
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

const SymbolData SymbolRef::data(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(this->exists());
    return dataAllowingNone(gs);
}

const SymbolData SymbolRef::dataAllowingNone(const GlobalState &gs) const {
    switch (kind()) {
        case SymbolRef::Kind::ClassOrModule:
            ENFORCE_NO_TIMER(classOrModuleIndex() < gs.classAndModules.size());
            return SymbolData(const_cast<Symbol &>(gs.classAndModules[classOrModuleIndex()]), gs);
        case SymbolRef::Kind::Method:
            ENFORCE_NO_TIMER(methodIndex() < gs.methods.size());
            return SymbolData(const_cast<Symbol &>(gs.methods[methodIndex()]), gs);
        case SymbolRef::Kind::Field:
            ENFORCE_NO_TIMER(fieldIndex() < gs.fields.size());
            return SymbolData(const_cast<Symbol &>(gs.fields[fieldIndex()]), gs);
        case SymbolRef::Kind::TypeArgument:
            ENFORCE_NO_TIMER(typeArgumentIndex() < gs.typeArguments.size());
            return SymbolData(const_cast<Symbol &>(gs.typeArguments[typeArgumentIndex()]), gs);
        case SymbolRef::Kind::TypeMember:
            ENFORCE_NO_TIMER(typeMemberIndex() < gs.typeMembers.size());
            return SymbolData(const_cast<Symbol &>(gs.typeMembers[typeMemberIndex()]), gs);
    }
}

bool SymbolRef::isSynthetic() const {
    switch (this->kind()) {
        case Kind::ClassOrModule:
            return classOrModuleIndex() < Symbols::MAX_SYNTHETIC_CLASS_SYMBOLS;
        case Kind::Method:
            return methodIndex() < Symbols::MAX_SYNTHETIC_METHOD_SYMBOLS;
        case Kind::Field:
            return fieldIndex() < Symbols::MAX_SYNTHETIC_FIELD_SYMBOLS;
        case Kind::TypeArgument:
            return typeArgumentIndex() < Symbols::MAX_SYNTHETIC_TYPEARGUMENT_SYMBOLS;
        case Kind::TypeMember:
            return typeMemberIndex() < Symbols::MAX_SYNTHETIC_TYPEMEMBER_SYMBOLS;
    }
}

void printTabs(fmt::memory_buffer &to, int count) {
    string ident(count * 2, ' ');
    fmt::format_to(to, "{}", ident);
}

SymbolRef::SymbolRef(const GlobalState &from, SymbolRef::Kind kind, u4 id)
    : _id(id | (static_cast<u4>(kind) << ID_BITS)) {
    // If this fails, the symbol table is too big :(
    ENFORCE_NO_TIMER(id <= ID_MASK);
}
SymbolRef::SymbolRef(GlobalState const *from, SymbolRef::Kind kind, u4 id)
    : _id(id | (static_cast<u4>(kind) << ID_BITS)) {
    // If this fails, the symbol table is too big :(
    ENFORCE_NO_TIMER(id <= ID_MASK);
}

string SymbolRef::showRaw(const GlobalState &gs) const {
    return dataAllowingNone(gs)->showRaw(gs);
}
string SymbolRef::toString(const GlobalState &gs) const {
    return dataAllowingNone(gs)->toString(gs);
}
string SymbolRef::show(const GlobalState &gs) const {
    return dataAllowingNone(gs)->show(gs);
}

TypePtr ArgInfo::argumentTypeAsSeenByImplementation(Context ctx, core::TypeConstraint &constr) const {
    auto owner = ctx.owner;
    auto klass = owner.data(ctx)->enclosingClass(ctx);
    ENFORCE(klass.data(ctx)->isClassOrModule());
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
                        this->showFullName(gs));
        }
        int i = -1;
        for (auto it = this->mixins().rbegin(); it != this->mixins().rend(); ++it) {
            i++;
            if (auto e = gs.beginError(Loc::none(), errors::Internal::InternalError)) {
                e.setHeader("`{}`:- `{}`", i, it->data(gs)->showFullName(gs));
            }
            int j = 0;
            for (auto it2 = it->data(gs)->mixins().rbegin(); it2 != it->data(gs)->mixins().rend(); ++it2) {
                if (auto e = gs.beginError(Loc::none(), errors::Internal::InternalError)) {
                    e.setHeader("`{}`:`{}` `{}`", i, j, it2->data(gs)->showFullName(gs));
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

    if (name.data(gs)->kind == NameKind::UTF8) {
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
        auto shortName = name.data(gs)->shortName(gs);
        if (!shortName.empty() && std::isupper(shortName.front())) {
            vector<Symbol::FuzzySearchResult> constant_matches = findMemberFuzzyMatchConstant(gs, name, betterThan);
            res.insert(res.end(), constant_matches.begin(), constant_matches.end());
        }
    } else if (name.data(gs)->kind == NameKind::CONSTANT) {
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
    auto currentName = name.data(gs)->shortName(gs);
    if (best.distance < 0) {
        best.distance = 1 + (currentName.size() / 2);
    }

    // Find the closest by following outer scopes
    {
        SymbolRef base = ref(gs);
        do {
            // follow outer scopes

            // find scopes that would be considered for search
            vector<SymbolRef> candidateScopes;
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
                for (auto member : scope.data(gs)->membersStableOrderSlow(gs)) {
                    if (member.first.data(gs)->kind == NameKind::CONSTANT &&
                        member.first.data(gs)->cnst.original.data(gs)->kind == NameKind::UTF8 &&
                        member.second.exists()) {
                        auto thisDistance = Levenstein::distance(
                            currentName, member.first.data(gs)->cnst.original.data(gs)->raw.utf8, best.distance);
                        if (thisDistance <= best.distance) {
                            best.distance = thisDistance;
                            best.symbol = member.second;
                            best.name = member.first;
                            result.emplace_back(best);
                        }
                    }
                }
            }

            base = base.data(gs)->owner;
        } while (best.distance > 0 && base.data(gs)->owner.exists() && base != Symbols::root());
    }

    // make sure we have a stable order
    fast_sort(result, [&](auto lhs, auto rhs) -> bool {
        return lhs.distance < rhs.distance || (lhs.distance == rhs.distance && lhs.symbol._id < rhs.symbol._id);
    });

    if (best.distance > 0) {
        // find the closest by global dfs.
        auto globalBestDistance = best.distance - 1;
        vector<Symbol::FuzzySearchResult> globalBest;
        vector<SymbolRef> yetToGoDeeper;
        yetToGoDeeper.emplace_back(Symbols::root());
        while (!yetToGoDeeper.empty()) {
            const SymbolRef thisIter = yetToGoDeeper.back();
            yetToGoDeeper.pop_back();
            ENFORCE(thisIter.data(gs)->isClassOrModule());
            for (auto member : thisIter.data(gs)->membersStableOrderSlow(gs)) {
                if (member.second.exists() && member.first.exists() &&
                    member.first.data(gs)->kind == NameKind::CONSTANT &&
                    member.first.data(gs)->cnst.original.data(gs)->kind == NameKind::UTF8) {
                    if (member.second.data(gs)->isClassOrModule() &&
                        member.second.data(gs)->derivesFrom(gs, core::Symbols::StubModule())) {
                        continue;
                    }
                    auto thisDistance = Levenstein::distance(
                        currentName, member.first.data(gs)->cnst.original.data(gs)->raw.utf8, best.distance);
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
                    if (member.second.data(gs)->isClassOrModule()) {
                        yetToGoDeeper.emplace_back(member.second);
                    }
                }
            }
        }
        for (auto e : globalBest) {
            result.emplace_back(e);
        }
    }
    absl::c_reverse(result);
    return result;
}

Symbol::FuzzySearchResult Symbol::findMemberFuzzyMatchUTF8(const GlobalState &gs, NameRef name, int betterThan) const {
    FuzzySearchResult result;
    result.symbol = Symbols::noSymbol();
    result.name = NameRef::noName();
    result.distance = betterThan;
    ENFORCE(name.data(gs)->kind == NameKind::UTF8);

    auto currentName = name.data(gs)->raw.utf8;
    if (result.distance < 0) {
        result.distance = 1 + (currentName.size() / 2);
    }

    for (auto pair : members()) {
        auto thisName = pair.first;
        if (thisName.data(gs)->kind != NameKind::UTF8) {
            continue;
        }
        auto utf8 = thisName.data(gs)->raw.utf8;
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
            ENFORCE(subResult.name.data(gs)->kind == NameKind::UTF8);
            result = subResult;
        }
    }
    if (this->superClass().exists()) {
        auto subResult = this->superClass().data(gs)->findMemberFuzzyMatchUTF8(gs, name, result.distance);
        if (subResult.symbol.exists()) {
            ENFORCE(subResult.name.exists());
            ENFORCE(subResult.name.data(gs)->kind == NameKind::UTF8);
            result = subResult;
        }
    }
    return result;
}

string Symbol::toStringFullName(const GlobalState &gs) const {
    bool includeOwner = this->owner.exists() && this->owner != Symbols::root();
    string owner = includeOwner ? this->owner.data(gs)->toStringFullName(gs) : "";

    return fmt::format("{}{}", owner, this->name.showRaw(gs));
}

string Symbol::showFullName(const GlobalState &gs) const {
    if (this->owner == core::Symbols::PackageRegistry()) {
        // Pretty print package name (only happens when `--stripe-packages` is enabled)
        auto nameStr = this->name.show(gs);
        constexpr string_view packageNameSuffix = "_Package"sv;
        if (absl::EndsWith(nameStr, packageNameSuffix)) {
            // Foo_Bar_Package => Foo::Bar
            return absl::StrReplaceAll(nameStr.substr(0, nameStr.size() - packageNameSuffix.size()), {{"_", "::"}});
        }
    }
    bool includeOwner = this->owner.exists() && this->owner != Symbols::root();
    string owner = includeOwner ? this->owner.data(gs)->showFullName(gs) : "";

    bool needsColonColon = this->isClassOrModule() || this->isStaticField() || this->isTypeMember();
    string separator = needsColonColon ? "::" : "#";

    return fmt::format("{}{}{}", owner, separator, this->name.show(gs));
}

bool Symbol::isHiddenFromPrinting(const GlobalState &gs) const {
    if (ref(gs).isSynthetic()) {
        return true;
    }
    if (locs_.empty()) {
        return true;
    }
    for (auto loc : locs_) {
        if (loc.file().data(gs).sourceType == File::Type::Payload) {
            return true;
        }
    }
    return false;
}

bool Symbol::isHiddenFromPrintingRecursive(const GlobalState &gs) const {
    for (auto childPair : this->members()) {
        if (childPair.first == Names::singleton() || childPair.first == Names::attached() ||
            childPair.first == Names::classMethods()) {
            continue;
        }
        if (!childPair.second.data(gs)->isHiddenFromPrinting(gs)) {
            return true;
        }

        if (childPair.second.data(gs)->isHiddenFromPrintingRecursive(gs)) {
            return true;
        }
    }

    return false;
}

string Symbol::toStringWithOptions(const GlobalState &gs, int tabs, bool showFull, bool showRaw) const {
    fmt::memory_buffer buf;

    printTabs(buf, tabs);

    string_view type = "unknown"sv;
    if (this->isClassOrModule()) {
        if (this->isClassOrModuleClass()) {
            type = "class"sv;
        } else {
            type = "module"sv;
        }
    } else if (this->isStaticField()) {
        if (this->isTypeAlias()) {
            type = "static-field-type-alias"sv;
        } else {
            type = "static-field"sv;
        }
    } else if (this->isField()) {
        type = "field"sv;
    } else if (this->isMethod()) {
        type = "method"sv;
    } else if (this->isTypeMember()) {
        type = "type-member"sv;
    } else if (this->isTypeArgument()) {
        type = "type-argument"sv;
    }

    string_view variance = ""sv;

    if (this->isTypeArgument() || this->isTypeMember()) {
        if (this->isCovariant()) {
            variance = "(+)"sv;
        } else if (this->isContravariant()) {
            variance = "(-)"sv;
        } else if (this->isInvariant()) {
            variance = "(=)"sv;
        } else {
            Exception::raise("type without variance");
        }
    }

    fmt::format_to(buf, "{}{} {}", type, variance, showRaw ? this->toStringFullName(gs) : this->showFullName(gs));

    if (this->isClassOrModule() || this->isMethod()) {
        if (this->isMethod()) {
            if (this->isPrivate()) {
                fmt::format_to(buf, " : private");
            } else if (this->isProtected()) {
                fmt::format_to(buf, " : protected");
            }
        }

        auto typeMembers = this->isClassOrModule() ? this->typeMembers() : this->typeArguments();
        auto it = remove_if(typeMembers.begin(), typeMembers.end(),
                            [&gs](auto &sym) -> bool { return sym.data(gs)->isFixed(); });
        typeMembers.erase(it, typeMembers.end());
        if (!typeMembers.empty()) {
            fmt::format_to(buf, "[{}]", fmt::map_join(typeMembers, ", ", [&](auto symb) {
                               auto name = symb.data(gs)->name;
                               return showRaw ? name.showRaw(gs) : name.show(gs);
                           }));
        }

        if (this->isClassOrModule() && this->superClass().exists()) {
            auto superClass = this->superClass().data(gs);
            fmt::format_to(buf, " < {}", showRaw ? superClass->toStringFullName(gs) : superClass->showFullName(gs));
        }

        if (this->isClassOrModule()) {
            fmt::format_to(buf, " ({})", fmt::map_join(this->mixins(), ", ", [&](auto symb) {
                               auto name = symb.data(gs)->name;
                               return showRaw ? name.showRaw(gs) : name.show(gs);
                           }));

        } else {
            fmt::format_to(buf, " ({})", fmt::map_join(this->arguments(), ", ", [&](const auto &symb) {
                               return symb.argumentName(gs);
                           }));
        }
    }
    if (this->resultType && !isClassOrModule()) {
        string resultType;
        if (showRaw) {
            resultType = absl::StrReplaceAll(this->resultType->toStringWithTabs(gs, tabs), {{"\n", " "}});
        } else {
            resultType = this->resultType->show(gs);
        }
        fmt::format_to(buf, " -> {}", resultType);
    }
    if (!locs_.empty()) {
        fmt::format_to(buf, " @ ");
        if (locs_.size() > 1) {
            if (ref(gs) == core::Symbols::root() && gs.censorForSnapshotTests) {
                const auto payloadPathPrefix = "https://github.com/sorbet/sorbet/tree/master/";
                bool hasPayloadLoc = absl::c_any_of(locs_, [&](const auto loc) {
                    return absl::StartsWith(loc.file().data(gs).path(), payloadPathPrefix);
                });

                fmt::format_to(buf, "(");
                if (hasPayloadLoc) {
                    fmt::format_to(buf, "... removed core rbi locs ...");
                }

                bool first = true;
                vector<Loc> sortedLocs;
                sortedLocs.reserve(locs_.size());
                for (const auto loc : locs_) {
                    sortedLocs.emplace_back(loc);
                }
                fast_sort(sortedLocs,
                          [&](const Loc &lhs, const Loc &rhs) { return lhs.showRaw(gs) < rhs.showRaw(gs); });
                for (const auto loc : sortedLocs) {
                    if (absl::StartsWith(loc.file().data(gs).path(), payloadPathPrefix)) {
                        continue;
                    }

                    if (first) {
                        first = false;
                        if (hasPayloadLoc) {
                            fmt::format_to(buf, ", ");
                        }
                    } else {
                        fmt::format_to(buf, ", ");
                    }
                    fmt::format_to(buf, "{}", showRaw ? loc.showRaw(gs) : loc.filePosToString(gs));
                }
                fmt::format_to(buf, ")");
            } else {
                fmt::format_to(buf, "({})", fmt::map_join(locs_, ", ", [&](auto loc) {
                                   return showRaw ? loc.showRaw(gs) : loc.filePosToString(gs);
                               }));
            }
        } else {
            fmt::format_to(buf, "{}", showRaw ? locs_[0].showRaw(gs) : locs_[0].filePosToString(gs));
        }
    }

    if (this->isMethod()) {
        if (this->rebind().exists()) {
            fmt::format_to(buf, " rebindTo {}",
                           showRaw ? this->rebind().data(gs)->toStringFullName(gs)
                                   : this->rebind().data(gs)->showFullName(gs));
        }
    }

    ENFORCE(!absl::c_any_of(to_string(buf), [](char c) { return c == '\n'; }));

    fmt::format_to(buf, "\n");
    for (auto pair : membersStableOrderSlow(gs)) {
        if (!pair.second.exists()) {
            ENFORCE(ref(gs) == core::Symbols::root());
            continue;
        }

        if (pair.first == Names::singleton() || pair.first == Names::attached() ||
            pair.first == Names::classMethods()) {
            continue;
        }

        if (!showFull && pair.second.data(gs)->isHiddenFromPrinting(gs) &&
            !pair.second.data(gs)->isHiddenFromPrintingRecursive(gs)) {
            continue;
        }

        auto str = pair.second.data(gs)->toStringWithOptions(gs, tabs + 1, showFull, showRaw);
        ENFORCE(!str.empty());
        fmt::format_to(buf, "{}", move(str));
    }
    if (isMethod()) {
        for (auto &arg : arguments()) {
            auto str = arg.toString(gs);
            ENFORCE(!str.empty());
            printTabs(buf, tabs + 1);
            fmt::format_to(buf, "{}\n", move(str));
        }
    }

    return to_string(buf);
}

string ArgInfo::show(const GlobalState &gs) const {
    return fmt::format("{}", this->argumentName(gs));
}

string ArgInfo::toString(const GlobalState &gs) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "argument {}", show(gs));
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
    fmt::format_to(buf, "<{}>", fmt::join(flagTexts, ", "));
    if (this->type) {
        fmt::format_to(buf, " -> {}", this->type->show(gs));
    }

    fmt::format_to(buf, " @ {}", loc.showRaw(gs));

    if (this->rebind.exists()) {
        fmt::format_to(buf, " rebindTo {}", this->rebind.data(gs)->showFullName(gs));
    }

    return to_string(buf);
}

string Symbol::show(const GlobalState &gs) const {
    if (isClassOrModule() && isSingletonClass(gs)) {
        auto attached = this->attachedClass(gs);
        if (attached.exists()) {
            return fmt::format("T.class_of({})", attached.data(gs)->show(gs));
        }
    }

    if (!this->owner.exists() || this->owner == Symbols::root() ||
        this->owner.data(gs)->owner == Symbols::PackageRegistry()) {
        // <PackageRegistry> is an internal detail of --stripe-packages. It only owns synthetic modules that encapsulate
        // package namespaces; they should not be shown to the user. Another way to think about this:
        // if --stripe-packages hadn't been specified, these constants would have been owned by <root>.
        return this->name.data(gs)->show(gs);
    }

    if (this->name == core::Names::Constants::AttachedClass()) {
        auto attached = this->owner.data(gs)->attachedClass(gs);
        ENFORCE(attached.exists());
        return fmt::format("T.attached_class (of {})", attached.data(gs)->show(gs));
    }

    if (this->isMethod() && this->owner.data(gs)->isClassOrModule() && this->owner.data(gs)->isSingletonClass(gs)) {
        return fmt::format("{}.{}", this->owner.data(gs)->attachedClass(gs).data(gs)->show(gs),
                           this->name.data(gs)->show(gs));
    }

    auto needsColonColon = this->isClassOrModule() || this->isStaticField() || this->isTypeMember();

    return fmt::format("{}{}{}", this->owner.data(gs)->show(gs), needsColonColon ? "::" : "#",
                       this->name.data(gs)->show(gs));
}

string ArgInfo::argumentName(const GlobalState &gs) const {
    if (flags.isKeyword) {
        return (string)name.data(gs)->shortName(gs);
    } else {
        // positional arg
        if (loc.exists()) {
            return loc.source(gs);
        } else {
            return (string)name.data(gs)->shortName(gs);
        }
    }
}

namespace {
bool isSingletonName(const GlobalState &gs, core::NameRef name) {
    return name.data(gs)->kind == NameKind::UNIQUE && name.data(gs)->unique.uniqueNameKind == UniqueNameKind::Singleton;
}

bool isMangledSingletonName(const GlobalState &gs, core::NameRef name) {
    return name.data(gs)->kind == NameKind::UNIQUE &&
           name.data(gs)->unique.uniqueNameKind == UniqueNameKind::MangleRename &&
           isSingletonName(gs, name.data(gs)->unique.original);
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

SymbolRef Symbol::singletonClass(GlobalState &gs) {
    auto singleton = lookupSingletonClass(gs);
    if (singleton.exists()) {
        return singleton;
    }
    SymbolRef selfRef = this->ref(gs);

    // avoid using `this` after the call to gs.enterTypeMember
    auto selfLoc = this->loc();

    NameRef singletonName = gs.freshNameUnique(UniqueNameKind::Singleton, this->name, 1);
    singleton = gs.enterClassSymbol(this->loc(), this->owner, singletonName);
    SymbolData singletonInfo = singleton.data(gs);

    counterInc("singleton_classes");
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

SymbolRef Symbol::lookupSingletonClass(const GlobalState &gs) const {
    ENFORCE(this->isClassOrModule());
    ENFORCE(this->name.data(gs)->isClassName(gs));

    SymbolRef selfRef = this->ref(gs);
    if (selfRef == Symbols::untyped()) {
        return Symbols::untyped();
    }

    return findMember(gs, Names::singleton());
}

SymbolRef Symbol::attachedClass(const GlobalState &gs) const {
    ENFORCE(this->isClassOrModule());
    if (this->ref(gs) == Symbols::untyped()) {
        return Symbols::untyped();
    }

    SymbolRef singleton = findMember(gs, Names::attached());
    return singleton;
}

SymbolRef Symbol::topAttachedClass(const GlobalState &gs) const {
    auto classSymbol = this->ref(gs);

    while (true) {
        auto attachedClass = classSymbol.data(gs)->attachedClass(gs);
        if (!attachedClass.exists()) {
            break;
        }
        classSymbol = attachedClass;
    }

    return classSymbol;
}

void Symbol::recordSealedSubclass(MutableContext ctx, SymbolRef subclass) {
    ENFORCE(this->isClassOrModuleSealed(), "Class is not marked sealed: {}", this->show(ctx));
    ENFORCE(subclass.exists(), "Can't record sealed subclass for {} when subclass doesn't exist", this->show(ctx));
    ENFORCE(subclass.data(ctx)->isClassOrModule(), "Sealed subclass {} must be class", subclass.show(ctx));

    // We record sealed subclasses on a magical method called core::Names::sealedSubclasses(). This is so we don't
    // bloat the `sizeof class Symbol` with an extra field that most class sybmols will never use.
    // Note: We had hoped to ALSO implement this method in the runtime, but we couldn't think of a way to make it work
    // that didn't require running with the help of Stripe's autoloader, specifically because we might want to allow
    // subclassing a sealed class across multiple files, not just one file.
    auto classOfSubclass = subclass.data(ctx)->singletonClass(ctx);
    auto sealedSubclasses = this->lookupSingletonClass(ctx).data(ctx)->findMember(ctx, core::Names::sealedSubclasses());

    auto data = sealedSubclasses.data(ctx);
    ENFORCE(data->resultType != nullptr, "Should have been populated in namer");
    auto appliedType = cast_type<AppliedType>(data->resultType.get());
    ENFORCE(appliedType != nullptr, "sealedSubclasses should always be AppliedType");
    ENFORCE(appliedType->klass == core::Symbols::Array(), "sealedSubclasses should always be Array");
    auto currentClasses = appliedType->targs[0];

    auto iter = currentClasses.get();
    OrType *orT = nullptr;
    while ((orT = cast_type<OrType>(iter))) {
        auto right = cast_type<ClassType>(orT->right.get());
        ENFORCE(left);
        if (right->symbol == classOfSubclass) {
            return;
        }
        iter = orT->left.get();
    }
    ENFORCE(isa_type<ClassType>(iter));
    if (cast_type<ClassType>(iter)->symbol == classOfSubclass) {
        return;
    }
    if (currentClasses != core::Types::bottom()) {
        appliedType->targs[0] = OrType::make_shared(currentClasses, make_type<ClassType>(classOfSubclass));
    } else {
        appliedType->targs[0] = make_type<ClassType>(classOfSubclass);
    }
}

const InlinedVector<Loc, 2> &Symbol::sealedLocs(const GlobalState &gs) const {
    ENFORCE(this->isClassOrModuleSealed(), "Class is not marked sealed: {}", this->show(gs));
    auto sealedSubclasses = this->lookupSingletonClass(gs).data(gs)->findMember(gs, core::Names::sealedSubclasses());
    auto &result = sealedSubclasses.data(gs)->locs();
    ENFORCE(result.size() > 0);
    return result;
}

TypePtr Symbol::sealedSubclassesToUnion(const GlobalState &gs) const {
    ENFORCE(this->isClassOrModuleSealed(), "Class is not marked sealed: {}", this->show(gs));

    auto sealedSubclasses = this->lookupSingletonClass(gs).data(gs)->findMember(gs, core::Names::sealedSubclasses());

    auto data = sealedSubclasses.data(gs);
    ENFORCE(data->resultType != nullptr, "Should have been populated in namer");
    auto appliedType = cast_type<AppliedType>(data->resultType.get());
    ENFORCE(appliedType != nullptr, "sealedSubclasses should always be AppliedType");
    ENFORCE(appliedType->klass == core::Symbols::Array(), "sealedSubclasses should always be Array");

    auto currentClasses = appliedType->targs[0];
    if (currentClasses->isBottom()) {
        // Declared sealed parent class, but never saw any children.
        return Types::bottom();
    }

    auto result = Types::bottom();
    while (auto orType = cast_type<OrType>(currentClasses.get())) {
        auto classType = cast_type<ClassType>(orType->right.get());
        ENFORCE(classType != nullptr, "Something in sealedSubclasses that's not a ClassType");
        auto subclass = classType->symbol.data(gs)->attachedClass(gs);
        ENFORCE(subclass.exists());
        result = Types::any(gs, make_type<ClassType>(subclass), result);
        currentClasses = orType->left;
    }
    auto lastClassType = cast_type<ClassType>(currentClasses.get());
    ENFORCE(lastClassType != nullptr, "Last element of sealedSubclasses must be ClassType");
    auto subclass = lastClassType->symbol.data(gs)->attachedClass(gs);
    ENFORCE(subclass.exists());
    result = Types::any(gs, make_type<ClassType>(subclass), result);

    return result;
}

SymbolRef Symbol::dealiasWithDefault(const GlobalState &gs, int depthLimit, SymbolRef def) const {
    if (auto alias = cast_type<AliasType>(resultType.get())) {
        if (depthLimit == 0) {
            if (auto e = gs.beginError(loc(), errors::Internal::CyclicReferenceError)) {
                e.setHeader("Too many alias expansions for symbol {}, the alias is either too long or infinite. Next "
                            "expansion would have been to {}",
                            showFullName(gs), alias->symbol.data(gs)->showFullName(gs));
            }
            return def;
        }
        return alias->symbol.data(gs)->dealiasWithDefault(gs, depthLimit - 1, def);
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
    result.name = NameRef(to, this->name.id());
    result.locs_ = this->locs_;
    result.typeParams = this->typeParams;
    if (keepGsId) {
        result.members_ = this->members_;
    } else {
        result.members_.reserve(this->members().size());
        for (auto &mem : this->members_) {
            result.members_[NameRef(to, mem.first.id())] = mem.second;
        }
    }
    result.arguments_.reserve(this->arguments_.size());
    for (auto &mem : this->arguments_) {
        auto &store = result.arguments_.emplace_back(mem.deepCopy());
        store.name = NameRef(to, mem.name.id());
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
                current2 = const_cast<GlobalState &>(gs).enterClassSymbol(this->loc(), this->owner, this->name);
                break;
            case SymbolRef::Kind::Method:
                current2 = const_cast<GlobalState &>(gs).enterMethodSymbol(this->loc(), this->owner, this->name);
                break;
            case SymbolRef::Kind::Field:
                if (isField()) {
                    current2 = const_cast<GlobalState &>(gs).enterFieldSymbol(this->loc(), this->owner, this->name);
                } else {
                    current2 =
                        const_cast<GlobalState &>(gs).enterStaticFieldSymbol(this->loc(), this->owner, this->name);
                }
                break;
            case SymbolRef::Kind::TypeArgument:
                current2 = const_cast<GlobalState &>(gs).enterTypeArgument(this->loc(), this->owner, this->name,
                                                                           this->variance());
                break;
            case SymbolRef::Kind::TypeMember:
                current2 = const_cast<GlobalState &>(gs).enterTypeMember(this->loc(), this->owner, this->name,
                                                                         this->variance());
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
        if (isa_type<AliasType>(this->resultType.get())) {
            // If we have an alias method, we should never look at it's arguments;
            // we should instead look at the arguments of whatever we're aliasing.
            ENFORCE_NO_TIMER(this->arguments().empty(), this->show(gs));
        } else {
            ENFORCE_NO_TIMER(!this->arguments().empty(), this->show(gs));
        }
    }
}

SymbolRef Symbol::enclosingMethod(const GlobalState &gs) const {
    if (isMethod()) {
        return ref(gs);
    }
    SymbolRef owner = this->owner;
    while (owner != Symbols::root() && !owner.data(gs)->isMethod()) {
        ENFORCE(owner.exists(), "non-existing owner in enclosingMethod");
        owner = owner.data(gs)->owner;
    }
    return owner;
}

SymbolRef Symbol::enclosingClass(const GlobalState &gs) const {
    SymbolRef owner = ref(gs);
    while (!owner.data(gs)->isClassOrModule()) {
        ENFORCE(owner.exists(), "non-existing owner in enclosingClass");
        owner = owner.data(gs)->owner;
    }
    return owner;
}

u4 Symbol::hash(const GlobalState &gs) const {
    u4 result = _hash(name.data(gs)->shortName(gs));
    result = mix(result, !this->resultType ? 0 : this->resultType->hash(gs));
    result = mix(result, this->flags);
    result = mix(result, this->owner._id);
    result = mix(result, this->superClassOrRebind._id);
    // argumentsOrMixins, typeParams, typeAliases
    for (auto e : membersStableOrderSlow(gs)) {
        if (e.second.exists() && !e.second.data(gs)->ignoreInHashing(gs)) {
            result = mix(result, _hash(e.second.data(gs)->name.data(gs)->shortName(gs)));
        }
    }
    for (const auto &e : mixins_) {
        if (e.exists() && !e.data(gs)->ignoreInHashing(gs)) {
            result = mix(result, _hash(e.data(gs)->name.data(gs)->shortName(gs)));
        }
    }
    for (const auto &arg : arguments_) {
        // If an argument's resultType changes, then the sig has changed.
        auto type = arg.type;
        if (!type) {
            type = Types::untypedUntracked();
        }
        result = mix(result, type->hash(gs));
        result = mix(result, _hash(arg.name.data(gs)->shortName(gs)));
    }
    for (const auto &e : typeParams) {
        if (e.exists() && !e.data(gs)->ignoreInHashing(gs)) {
            result = mix(result, _hash(e.data(gs)->name.data(gs)->shortName(gs)));
        }
    }

    return result;
}

u4 Symbol::methodShapeHash(const GlobalState &gs) const {
    ENFORCE(isMethod());

    u4 result = _hash(name.data(gs)->shortName(gs));
    result = mix(result, this->flags);
    result = mix(result, this->owner._id);
    result = mix(result, this->superClassOrRebind._id);
    result = mix(result, this->hasSig());
    for (auto &arg : this->methodArgumentHash(gs)) {
        result = mix(result, arg);
    }

    if (name == core::Names::unresolvedAncestors()) {
        // This is a synthetic method that encodes the superclasses of its owning class in its return type.
        // If the return type changes, we must take the slow path.
        ENFORCE(resultType);
        result = mix(result, resultType->hash(gs));
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
            arg = mix(arg, _hash(e.name.data(gs)->shortName(gs)));
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
        return name.data(gs)->kind == NameKind::UNIQUE && name.data(gs)->unique.original == core::Names::staticInit();
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
        // Make this the new canonical loc.
        locs_.emplace_back(loc);
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
        auto lhsShort = lhs.first.data(gs)->shortName(gs);
        auto rhsShort = rhs.first.data(gs)->shortName(gs);
        auto compareShort = lhsShort.compare(rhsShort);
        if (compareShort != 0) {
            return compareShort < 0;
        }
        auto lhsRaw = lhs.first.data(gs)->showRaw(gs);
        auto rhsRaw = rhs.first.data(gs)->showRaw(gs);
        auto compareRaw = lhsRaw.compare(rhsRaw);
        if (compareRaw != 0) {
            return compareRaw < 0;
        }
        auto lhsSym = lhs.second.data(gs)->showRaw(gs);
        auto rhsSym = rhs.second.data(gs)->showRaw(gs);
        auto compareSym = lhsSym.compare(rhsSym);
        if (compareSym != 0) {
            return compareSym < 0;
        }
        ENFORCE(false, "no stable sort");
        return 0;
    });
    return result;
}

SymbolData::SymbolData(Symbol &ref, const GlobalState &gs) : DebugOnlyCheck(gs), symbol(ref) {}

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

} // namespace sorbet::core
