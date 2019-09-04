#include "core/Symbols.h"
#include "absl/strings/match.h"
#include "common/JSON.h"
#include "common/Levenstein.h"
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
    ENFORCE(isClass()); // should be removed when we have generic methods
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
    ENFORCE(isClass());
    // todo: in dotty it made sense to cache those.
    if (typeMembers().empty()) {
        return externalType(gs);
    } else {
        return make_type<AppliedType>(ref(gs), selfTypeArgs(gs));
    }
}

TypePtr Symbol::externalType(const GlobalState &gs) const {
    ENFORCE(isClass());
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

            for (auto &tm : typeMembers()) {
                auto tmData = tm.data(gs);
                auto *lambdaParam = cast_type<LambdaParam>(tmData->resultType.get());
                ENFORCE(lambdaParam != nullptr);

                if (tmData->isFixed() || tmData->isCovariant()) {
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
    if (isClassLinearizationComputed()) {
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
    auto distance = this - gs.symbols.data();
    return SymbolRef(gs, distance);
}

SymbolData SymbolRef::data(GlobalState &gs) const {
    ENFORCE(this->exists());
    return dataAllowingNone(gs);
}

SymbolData SymbolRef::dataAllowingNone(GlobalState &gs) const {
    ENFORCE(_id < gs.symbols.size());
    return SymbolData(gs.symbols[this->_id], gs);
}

const SymbolData SymbolRef::data(const GlobalState &gs) const {
    ENFORCE(this->exists());
    return dataAllowingNone(gs);
}

const SymbolData SymbolRef::dataAllowingNone(const GlobalState &gs) const {
    ENFORCE(_id < gs.symbols.size());
    return SymbolData(const_cast<Symbol &>(gs.symbols[this->_id]), gs);
}

bool SymbolRef::isSynthetic() const {
    return this->_id < Symbols::MAX_SYNTHETIC_SYMBOLS;
}

void printTabs(fmt::memory_buffer &to, int count) {
    string ident(count * 2, ' ');
    fmt::format_to(to, "{}", ident);
}

SymbolRef::SymbolRef(const GlobalState &from, u4 _id) : _id(_id) {}
SymbolRef::SymbolRef(GlobalState const *from, u4 _id) : _id(_id) {}

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
    ENFORCE(klass.data(ctx)->isClass());
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
    ENFORCE(this->isClass());
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
    if (isClassLinearizationComputed()) {
        for (auto it = this->mixins().begin(); it != this->mixins().end(); ++it) {
            ENFORCE(it->exists());
            if (isClassLinearizationComputed()) {
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
            ENFORCE(thisIter.data(gs)->isClass());
            for (auto member : thisIter.data(gs)->membersStableOrderSlow(gs)) {
                if (member.second.exists() && member.first.exists() &&
                    member.first.data(gs)->kind == NameKind::CONSTANT &&
                    member.first.data(gs)->cnst.original.data(gs)->kind == NameKind::UTF8) {
                    if (member.second.data(gs)->isClass() &&
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
                    if (member.second.data(gs)->isClass()) {
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
    bool includeOwner = this->owner.exists() && this->owner != Symbols::root();
    string owner = includeOwner ? this->owner.data(gs)->showFullName(gs) : "";

    bool needsColonColon = this->isClass() || this->isStaticField() || this->isTypeMember();
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
        if (loc.file().data(gs).sourceType == File::Payload) {
            return true;
        }
    }
    return false;
}

string Symbol::toStringWithOptions(const GlobalState &gs, int tabs, bool showFull, bool showRaw) const {
    fmt::memory_buffer buf;

    printTabs(buf, tabs);

    string_view type = "unknown"sv;
    if (this->isClass()) {
        type = "class"sv;
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

    if (this->isClass() || this->isMethod()) {
        if (this->isMethod()) {
            if (this->isPrivate()) {
                fmt::format_to(buf, " : private");
            } else if (this->isProtected()) {
                fmt::format_to(buf, " : protected");
            }
        }

        auto typeMembers = this->isClass() ? this->typeMembers() : this->typeArguments();
        auto it = remove_if(typeMembers.begin(), typeMembers.end(),
                            [&gs](auto &sym) -> bool { return sym.data(gs)->isFixed(); });
        typeMembers.erase(it, typeMembers.end());
        if (!typeMembers.empty()) {
            fmt::format_to(buf, "[{}]", fmt::map_join(typeMembers, ", ", [&](auto symb) {
                               auto name = symb.data(gs)->name;
                               return showRaw ? name.showRaw(gs) : name.show(gs);
                           }));
        }

        if (this->isClass() && this->superClass().exists()) {
            auto superClass = this->superClass().data(gs);
            fmt::format_to(buf, " < {}", showRaw ? superClass->toStringFullName(gs) : superClass->showFullName(gs));
        }

        if (this->isClass()) {
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
    if (this->resultType && !isClass()) {
        fmt::format_to(buf, " -> {}",
                       showRaw ? this->resultType->toStringWithTabs(gs, tabs) : this->resultType->show(gs));
    }
    if (!locs_.empty()) {
        fmt::format_to(buf, " @ ");
        if (locs_.size() > 1) {
            if (ref(gs) == core::Symbols::root() && gs.censorForSnapshotTests) {
                const auto payloadPathPrefix = "https://github.com/sorbet/sorbet/tree/master/rbi/";
                bool hasPayloadLoc = absl::c_any_of(locs_, [&](const auto loc) {
                    return absl::StartsWith(loc.file().data(gs).path(), payloadPathPrefix);
                });

                fmt::format_to(buf, "(");
                if (hasPayloadLoc) {
                    fmt::format_to(buf, "... removed core rbi locs ...");
                }

                bool first = true;
                for (const auto loc : locs_) {
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

    fmt::format_to(buf, "\n");
    if (!isMethod()) {
        for (auto pair : membersStableOrderSlow(gs)) {
            if (!pair.second.exists()) {
                ENFORCE(ref(gs) == core::Symbols::root());
                continue;
            }

            if (pair.first == Names::singleton() || pair.first == Names::attached() ||
                pair.first == Names::classMethods()) {
                continue;
            }

            if (!showFull && pair.second.data(gs)->isHiddenFromPrinting(gs)) {
                bool hadPrintableChild = false;
                for (auto childPair : pair.second.data(gs)->members()) {
                    if (!childPair.second.data(gs)->isHiddenFromPrinting(gs)) {
                        hadPrintableChild = true;
                        break;
                    }
                }
                if (!hadPrintableChild) {
                    continue;
                }
            }

            auto str = pair.second.data(gs)->toStringWithOptions(gs, tabs + 1, showFull, showRaw);
            ENFORCE(!str.empty());
            fmt::format_to(buf, "{}", move(str));
        }
    } else {
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
    if (isClass() && isSingletonClass(gs)) {
        auto attached = this->attachedClass(gs);
        if (attached.exists()) {
            return fmt::format("T.class_of({})", attached.data(gs)->show(gs));
        }
    }

    if (!this->owner.exists() || this->owner == Symbols::root()) {
        return this->name.data(gs)->show(gs);
    }

    if (this->isMethod() && this->owner.data(gs)->isClass() && this->owner.data(gs)->isSingletonClass(gs)) {
        return fmt::format("{}.{}", this->owner.data(gs)->attachedClass(gs).data(gs)->show(gs),
                           this->name.data(gs)->show(gs));
    }

    auto needsColonColon = this->isClass() || this->isStaticField() || this->isTypeMember();

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
    return name.data(gs)->kind == UNIQUE && name.data(gs)->unique.uniqueNameKind == UniqueNameKind::Singleton;
}

bool isMangledSingletonName(const GlobalState &gs, core::NameRef name) {
    return name.data(gs)->kind == UNIQUE && name.data(gs)->unique.uniqueNameKind == UniqueNameKind::MangleRename &&
           isSingletonName(gs, name.data(gs)->unique.original);
}
} // namespace

bool Symbol::isSingletonClass(const GlobalState &gs) const {
    bool isSingleton = isClass() && (isSingletonName(gs, name) || isMangledSingletonName(gs, name));
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

    NameRef singletonName = gs.freshNameUnique(UniqueNameKind::Singleton, this->name, 1);
    singleton = gs.enterClassSymbol(this->loc(), this->owner, singletonName);
    SymbolData singletonInfo = singleton.data(gs);

    counterInc("singleton_classes");
    singletonInfo->members()[Names::attached()] = selfRef;
    singletonInfo->setSuperClass(Symbols::todo());
    singletonInfo->setIsModule(false);

    selfRef.data(gs)->members()[Names::singleton()] = singleton;
    return singleton;
}

SymbolRef Symbol::lookupSingletonClass(const GlobalState &gs) const {
    ENFORCE(this->isClass());
    ENFORCE(this->name.data(gs)->isClassName(gs));

    SymbolRef selfRef = this->ref(gs);
    if (selfRef == Symbols::untyped()) {
        return Symbols::untyped();
    }

    return findMember(gs, Names::singleton());
}

SymbolRef Symbol::attachedClass(const GlobalState &gs) const {
    ENFORCE(this->isClass());
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
    ENFORCE(this->isClassSealed(), "Class is not marked sealed: {}", this->show(ctx));
    ENFORCE(subclass.exists(), "Can't record sealed subclass for {} when subclass doesn't exist", this->show(ctx));
    ENFORCE(subclass.data(ctx)->isClass(), "Sealed subclass {} must be class", subclass.show(ctx));

    auto classOfSubclass = subclass.data(ctx)->singletonClass(ctx);
    auto sealedSubclasses = this->lookupSingletonClass(ctx).data(ctx)->findMember(ctx, core::Names::sealedSubclasses());

    auto data = sealedSubclasses.data(ctx);
    ENFORCE(data->resultType != nullptr, "Should have been populated in namer");
    auto appliedType = cast_type<AppliedType>(data->resultType.get());
    ENFORCE(appliedType != nullptr, "sealedSubclasses should always be AppliedType");
    ENFORCE(appliedType->klass == core::Symbols::Array(), "sealedSubclasses should always be Array");
    auto currentClasses = appliedType->targs[0];
    // Abusing T.any to be list cons, with T.noreturn as the empty list.
    // (Except it's not, because Types::lub is too smart, and drops the T.noreturn to prevent allocating a T.any)
    appliedType->targs[0] = Types::any(ctx, currentClasses, make_type<ClassType>(classOfSubclass));
}

const InlinedVector<Loc, 2> &Symbol::sealedLocs(const GlobalState &gs) const {
    ENFORCE(this->isClassSealed(), "Class is not marked sealed: {}", this->show(gs));
    auto sealedSubclasses = this->lookupSingletonClass(gs).data(gs)->findMember(gs, core::Names::sealedSubclasses());
    auto &result = sealedSubclasses.data(gs)->locs();
    ENFORCE(result.size() > 0);
    return result;
}

TypePtr Symbol::sealedSubclassesToUnion(const Context ctx) const {
    ENFORCE(this->isClassSealed(), "Class is not marked sealed: {}", this->show(ctx));

    auto sealedSubclasses = this->lookupSingletonClass(ctx).data(ctx)->findMember(ctx, core::Names::sealedSubclasses());

    auto data = sealedSubclasses.data(ctx);
    ENFORCE(data->resultType != nullptr, "Should have been populated in namer");
    auto appliedType = cast_type<AppliedType>(data->resultType.get());
    ENFORCE(appliedType != nullptr, "sealedSubclasses should always be AppliedType");
    ENFORCE(appliedType->klass == core::Symbols::Array(), "sealedSubclasses should always be Array");

    auto currentClasses = appliedType->targs[0];
    if (currentClasses->isBottom()) {
        // Declared sealed parent class, but never saw any children.
        return make_type<ClassType>(this->ref(ctx));
    }

    auto result = Types::bottom();
    while (auto orType = cast_type<OrType>(currentClasses.get())) {
        auto classType = cast_type<ClassType>(orType->right.get());
        ENFORCE(classType != nullptr, "Something in sealedSubclasses that's not a ClassType");
        auto subclass = classType->symbol.data(ctx)->attachedClass(ctx);
        ENFORCE(subclass.exists());
        result = Types::any(ctx, make_type<ClassType>(subclass), result);
        currentClasses = orType->left;
    }
    auto lastClassType = cast_type<ClassType>(currentClasses.get());
    ENFORCE(lastClassType != nullptr, "Last element of sealedSubclasses must be ClassType");
    auto subclass = lastClassType->symbol.data(ctx)->attachedClass(ctx);
    ENFORCE(subclass.exists());
    result = Types::any(ctx, make_type<ClassType>(subclass), result);

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
    result.uniqueCounter = this->uniqueCounter;
    result.intrinsic = this->intrinsic;
    return result;
}

int Symbol::typeArity(const GlobalState &gs) const {
    ENFORCE(this->isClass());
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
        SymbolRef current2 =
            const_cast<GlobalState &>(gs).enterSymbol(this->loc(), this->owner, this->name, this->flags);
        ENFORCE(current == current2);
        for (auto &e : members()) {
            ENFORCE(e.first.exists(), "symbol without a name in scope");
            ENFORCE(e.second.exists(), "name corresponding to a <none> in scope");
        }
    }
    if (this->isMethod()) {
        if (isa_type<AliasType>(this->resultType.get())) {
            // If we have an alias method, we should never look at it's arguments;
            // we should instead look at the arguments of whatever we're aliasing.
            ENFORCE(this->arguments().empty(), this->show(gs));
        } else {
            ENFORCE(!this->arguments().empty(), this->show(gs));
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
    while (!owner.data(gs)->isClass()) {
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
        result = mix(result, type);
        result = mix(result, _hash(arg.name.data(gs)->shortName(gs)));
    }
    for (const auto &e : typeParams) {
        if (e.exists() && !e.data(gs)->ignoreInHashing(gs)) {
            result = mix(result, _hash(e.data(gs)->name.data(gs)->shortName(gs)));
        }
    }

    return result;
}

// Bitmask for all method flags ignored when calculating the shape hash of a method symbol.
constexpr u4 METHOD_FLAGS_IGNORED_IN_SHAPE_HASH = Symbol::Flags::METHOD_GENERATED_SIG;

u4 Symbol::methodShapeHash(const GlobalState &gs) const {
    ENFORCE(isMethod());

    u4 result = _hash(name.data(gs)->shortName(gs));
    // Mark ignored flags to ON for shape hash.
    result = mix(result, this->flags | METHOD_FLAGS_IGNORED_IN_SHAPE_HASH);
    result = mix(result, this->owner._id);
    result = mix(result, this->superClassOrRebind._id);
    result = mix(result, this->hasSig());

    for (const auto &e : arguments()) {
        // Changing name of keyword arg is a shape change.
        // (N.B.: Is always <arg>/<blk> for positional/block args; renaming one of those is not a shape change.)
        result = mix(result, _hash(e.name.data(gs)->shortName(gs)));
        // Changing an argument from e.g. keyword to position-based is a shape change.
        result = mix(result, e.flags.toU1());
    }

    return result;
}

bool Symbol::ignoreInHashing(const GlobalState &gs) const {
    if (isClass()) {
        return superClass() == core::Symbols::StubModule();
    } else if (isMethod()) {
        return name.data(gs)->kind == NameKind::UNIQUE && name.data(gs)->unique.original == core::Names::staticInit();
    }
    return false;
}
Loc Symbol::loc() const {
    if (!locs_.empty()) {
        return locs_[0];
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
    for (auto &existing : locs_) {
        if (existing.file() == loc.file()) {
            existing = loc;
            return;
        }
    }

    if (loc.file().data(gs).sourceType == core::File::Type::Normal && !loc.file().data(gs).isRBI()) {
        locs_.insert(locs_.begin(), loc);
    } else {
        locs_.emplace_back(loc);
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
        return lhsShort < rhsShort ||
               (lhsShort == rhsShort && lhs.first.data(gs)->showRaw(gs) < rhs.first.data(gs)->showRaw(gs));
    });
    return result;
}

SymbolData::SymbolData(Symbol &ref, const GlobalState &gs) : DebugOnlyCheck(gs), symbol(ref) {}

SymbolDataDebugCheck::SymbolDataDebugCheck(const GlobalState &gs) : gs(gs), symbolCountAtCreation(gs.symbolsUsed()) {}

void SymbolDataDebugCheck::check() const {
    ENFORCE(symbolCountAtCreation == gs.symbolsUsed());
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
