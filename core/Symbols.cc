#include "core/Symbols.h"
#include "common/JSON.h"
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
        if (tm.data(gs)->isFixed()) {
            targs.emplace_back(tm.data(gs)->resultType);
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
        return make_type<ClassType>(ref(gs));
    } else {
        return make_type<AppliedType>(ref(gs), selfTypeArgs(gs));
    }
}

TypePtr Symbol::externalType(const GlobalState &gs) const {
    ENFORCE(isClass());
    auto ref = this->ref(gs);
    // todo: also cache these?
    if (typeMembers().empty()) {
        return make_type<ClassType>(ref);
    } else {
        vector<TypePtr> targs;
        for (auto tm : typeMembers()) {
            if (tm.data(gs)->isFixed()) {
                targs.emplace_back(tm.data(gs)->resultType);
            } else {
                targs.emplace_back(Types::untyped(gs, ref));
            }
        }
        return make_type<AppliedType>(ref, targs);
    }
}

bool Symbol::derivesFrom(const GlobalState &gs, SymbolRef sym) const {
    if (isClassLinearizationComputed()) {
        for (SymbolRef a : argumentsOrMixins) {
            if (a == sym) {
                return true;
            }
        }
    } else {
        for (SymbolRef a : argumentsOrMixins) {
            if (a == sym || a.data(gs)->derivesFrom(gs, sym)) {
                return true;
            }
        }
    }
    if (this->superClass.exists()) {
        return sym == this->superClass || this->superClass.data(gs)->derivesFrom(gs, sym);
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

string SymbolRef::toStringWithTabs(const GlobalState &gs, int tabs, bool showHidden, bool useToString) const {
    return dataAllowingNone(gs)->toStringWithTabs(gs, tabs, showHidden, useToString);
}
string SymbolRef::show(const GlobalState &gs) const {
    return dataAllowingNone(gs)->show(gs);
}

TypePtr Symbol::argumentTypeAsSeenByImplementation(Context ctx, core::TypeConstraint &constr) const {
    ENFORCE(isMethodArgument());
    auto klass = owner.data(ctx)->owner;
    ENFORCE(klass.data(ctx)->isClass());
    auto instantiated = Types::resultTypeAsSeenFrom(ctx, ref(ctx), klass, klass.data(ctx)->selfTypeArgs(ctx));
    if (instantiated == nullptr) {
        instantiated = core::Types::untyped(ctx, this->owner);
    }
    if (owner.data(ctx)->isGenericMethod()) {
        instantiated = core::Types::instantiate(ctx, instantiated, constr);
    } else {
        // You might expect us to instantiate with the constr to be null for a non-generic method,
        // but you might have the constraint that is used to guess return type of
        // this method. It's not solved and you shouldn't try to instantiate types against itt
    }

    if (!isRepeated()) {
        return instantiated;
    }
    if (isKeyword()) {
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
    histogramInc("find_member_scope_size", members.size());
    auto fnd = members.find(name);
    if (fnd == members.end()) {
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
        for (auto it = this->argumentsOrMixins.rbegin(); it != this->argumentsOrMixins.rend(); ++it) {
            i++;
            if (auto e = gs.beginError(Loc::none(), errors::Internal::InternalError)) {
                e.setHeader("`{}`:- `{}`", i, it->data(gs)->showFullName(gs));
            }
            int j = 0;
            for (auto it2 = it->data(gs)->argumentsOrMixins.rbegin(); it2 != it->data(gs)->argumentsOrMixins.rend();
                 ++it2) {
                if (auto e = gs.beginError(Loc::none(), errors::Internal::InternalError)) {
                    e.setHeader("`{}`:`{}` `{}`", i, j, it2->data(gs)->showFullName(gs));
                }
                j++;
            }
        }

        Exception::raise("findMemberTransitive hit a loop while resolving ");
    }

    SymbolRef result = findMember(gs, name);
    if (result.exists()) {
        if (mask == 0 || (result.data(gs)->flags & mask) == flags) {
            return result;
        }
    }
    if (isClassLinearizationComputed()) {
        for (auto it = this->argumentsOrMixins.begin(); it != this->argumentsOrMixins.end(); ++it) {
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
        for (auto it = this->argumentsOrMixins.rbegin(); it != this->argumentsOrMixins.rend(); ++it) {
            ENFORCE(it->exists());
            result = it->data(gs)->findMemberTransitiveInternal(gs, name, mask, flags, maxDepth - 1);
            if (result.exists()) {
                return result;
            }
        }
    }
    if (this->superClass.exists()) {
        return this->superClass.data(gs)->findMemberTransitiveInternal(gs, name, mask, flags, maxDepth - 1);
    }
    return Symbols::noSymbol();
}

vector<Symbol::FuzzySearchResult> Symbol::findMemberFuzzyMatch(const GlobalState &gs, NameRef name,
                                                               int betterThan) const {
    vector<Symbol::FuzzySearchResult> res;
    if (name.data(gs)->kind == NameKind::UTF8) {
        auto sym = findMemberFuzzyMatchUTF8(gs, name, betterThan);
        if (sym.symbol.exists()) {
            res.emplace_back(sym);
        } else {
            auto singleton = lookupSingletonClass(gs);
            if (singleton.exists()) {
                sym = singleton.data(gs)->findMemberFuzzyMatchUTF8(gs, name, betterThan);
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
                if (sym->superClass.exists()) {
                    if (!absl::c_linear_search(candidateScopes, sym->superClass)) {
                        candidateScopes.emplace_back(sym->superClass);
                    }
                }
                for (auto ancestor : sym->argumentsOrMixins) {
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

    for (auto pair : members) {
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

    for (auto it = this->argumentsOrMixins.rbegin(); it != this->argumentsOrMixins.rend(); ++it) {
        ENFORCE(it->exists());

        auto subResult = it->data(gs)->findMemberFuzzyMatchUTF8(gs, name, result.distance);
        if (subResult.symbol.exists()) {
            ENFORCE(subResult.name.exists());
            ENFORCE(subResult.name.data(gs)->kind == NameKind::UTF8);
            result = subResult;
        }
    }
    if (this->superClass.exists()) {
        auto subResult = this->superClass.data(gs)->findMemberFuzzyMatchUTF8(gs, name, result.distance);
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

    if (this->isMethodArgument()) {
        return fmt::format("{}{}", owner, this->argumentName(gs));
    }

    return fmt::format("{}{}", owner, this->name.toString(gs));
}

string Symbol::showFullName(const GlobalState &gs) const {
    bool includeOwner = this->owner.exists() && this->owner != Symbols::root();
    string owner = includeOwner ? this->owner.data(gs)->showFullName(gs) : "";

    bool needsColonColon = this->isClass() || this->isStaticField() || this->isTypeMember();
    string separator = needsColonColon ? "::" : "#";
    if (this->isMethodArgument()) {
        return fmt::format("{}{}{}", owner, separator, this->argumentName(gs));
    }

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

string Symbol::toStringWithTabs(const GlobalState &gs, int tabs, bool showHidden, bool useToString) const {
    fmt::memory_buffer buf;

    printTabs(buf, tabs);

    string_view type = "unknown"sv;
    if (this->isClass()) {
        type = "class"sv;
    } else if (this->isStaticField()) {
        if (this->isStaticTypeAlias()) {
            type = "static-field-type-alias"sv;
        } else {
            type = "static-field"sv;
        }
    } else if (this->isField()) {
        type = "field"sv;
    } else if (this->isMethod()) {
        type = "method"sv;
    } else if (this->isMethodArgument()) {
        type = "argument"sv;
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

    fmt::format_to(buf, "{}{} {}", type, variance, useToString ? this->toStringFullName(gs) : this->showFullName(gs));

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
                               return useToString ? name.toString(gs) : name.show(gs);
                           }));
        }

        if (this->superClass.exists()) {
            auto superClass = this->superClass.data(gs);
            fmt::format_to(buf, " < {}", useToString ? superClass->toStringFullName(gs) : superClass->showFullName(gs));
        }

        if (this->isClass()) {
            fmt::format_to(buf, " ({})", fmt::map_join(this->mixins(), ", ", [&](auto symb) {
                               auto name = symb.data(gs)->name;
                               return useToString ? name.toString(gs) : name.show(gs);
                           }));

        } else {
            fmt::format_to(buf, " ({})", fmt::map_join(this->arguments(), ", ", [&](auto symb) {
                               return symb.data(gs)->argumentName(gs);
                           }));
        }
    }
    if (this->isMethodArgument()) {
        vector<pair<u4, string_view>> methodFlags = {
            {Symbol::Flags::ARGUMENT_OPTIONAL, "optional"sv},
            {Symbol::Flags::ARGUMENT_KEYWORD, "keyword"sv},
            {Symbol::Flags::ARGUMENT_REPEATED, "repeated"sv},
            {Symbol::Flags::ARGUMENT_BLOCK, "block"sv},
        };
        fmt::format_to(buf, "<");
        bool first = true;
        for (auto &flag : methodFlags) {
            if ((this->flags & flag.first) != 0) {
                if (first) {
                    first = false;
                } else {
                    fmt::format_to(buf, ", ");
                }
                fmt::format_to(buf, "{}", flag.second);
            }
        }
        fmt::format_to(buf, ">");
    }
    if (this->resultType) {
        fmt::format_to(buf, " -> {}",
                       useToString ? this->resultType->toStringWithTabs(gs, tabs) : this->resultType->show(gs));
    }
    if (!locs_.empty()) {
        fmt::format_to(buf, " @ ");
        if (locs_.size() > 1) {
            fmt::format_to(buf, "({})", fmt::map_join(locs_, ", ", [&](auto loc) {
                               return useToString ? loc.showRaw(gs) : loc.filePosToString(gs);
                           }));
        } else {
            fmt::format_to(buf, "{}", useToString ? locs_[0].showRaw(gs) : locs_[0].filePosToString(gs));
        }
    }

    fmt::format_to(buf, "\n");
    bool hadPrintableChild = false;
    for (auto pair : membersStableOrderSlow(gs)) {
        if (pair.first == Names::singleton() || pair.first == Names::attached() ||
            pair.first == Names::classMethods()) {
            continue;
        }

        auto str = pair.second.toStringWithTabs(gs, tabs + 1, showHidden, useToString);
        if (!str.empty()) {
            hadPrintableChild = true;
            fmt::format_to(buf, "{}", move(str));
        }
    }

    if (!showHidden && this->isHiddenFromPrinting(gs) && !hadPrintableChild) {
        return "";
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

    if (this->isMethodArgument()) {
        return fmt::format("{}{}{}", this->owner.data(gs)->show(gs), needsColonColon ? "::" : "#",
                           this->argumentName(gs));
    }
    return fmt::format("{}{}{}", this->owner.data(gs)->show(gs), needsColonColon ? "::" : "#",
                       this->name.data(gs)->show(gs));
}

string Symbol::argumentName(const GlobalState &gs) const {
    ENFORCE(isMethodArgument());
    if (isKeyword()) {
        return (string)name.data(gs)->shortName(gs);
    } else {
        // positional arg
        if (loc().exists()) {
            return loc().source(gs);
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
    return name.data(gs)->kind == UNIQUE && name.data(gs)->unique.uniqueNameKind == UniqueNameKind::Namer &&
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
    singletonInfo->members[Names::attached()] = selfRef;
    singletonInfo->superClass = Symbols::todo();
    singletonInfo->setIsModule(false);

    selfRef.data(gs)->members[Names::singleton()] = singleton;
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

SymbolRef Symbol::dealias(const GlobalState &gs, int depthLimit) const {
    if (auto alias = cast_type<AliasType>(resultType.get())) {
        if (depthLimit == 0) {
            if (auto e = gs.beginError(loc(), errors::Internal::CyclicReferenceError)) {
                e.setHeader("Too many alias expansions for symbol {}, the alias is either too long or infinite. Next "
                            "expansion would have been to {}",
                            showFullName(gs), alias->symbol.data(gs)->showFullName(gs));
            }
            return alias->symbol;
        }
        return alias->symbol.data(gs)->dealias(gs, depthLimit - 1);
    }
    return this->ref(gs);
}

bool Symbol::isBlockSymbol(const GlobalState &gs) const {
    const auto &nm = name.data(gs);
    return nm->kind == NameKind::UNIQUE && nm->unique.original == Names::blockTemp();
}

Symbol Symbol::deepCopy(const GlobalState &to, bool keepGsId) const {
    Symbol result;
    result.owner = this->owner;
    result.flags = this->flags;
    result.argumentsOrMixins = this->argumentsOrMixins;
    result.resultType = this->resultType;
    result.name = NameRef(to, this->name.id());
    result.locs_ = this->locs_;
    result.typeParams = this->typeParams;
    if (keepGsId) {
        result.members = this->members;
    } else {
        result.members.reserve(this->members.size());
        for (auto &mem : this->members) {
            result.members[NameRef(to, mem.first.id())] = mem.second;
        }
    }
    result.superClass = this->superClass;
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

unsigned int Symbol::hash(const GlobalState &gs) const {
    unsigned int result = _hash(name.data(gs)->shortName(gs));
    result = mix(result, !this->resultType ? 0 : this->resultType->hash(gs));
    result = mix(result, this->flags);
    result = mix(result, this->owner._id);
    result = mix(result, this->superClass._id);
    // argumentsOrMixins, typeParams, typeAliases
    for (auto e : membersStableOrderSlow(gs)) {
        if (e.second.exists() && !e.second.data(gs)->ignoreInHashing(gs)) {
            result = mix(result, _hash(e.second.data(gs)->name.data(gs)->shortName(gs)));
        }
    }
    for (const auto &e : argumentsOrMixins) {
        if (e.exists() && !e.data(gs)->ignoreInHashing(gs)) {
            result = mix(result, _hash(e.data(gs)->name.data(gs)->shortName(gs)));
        }
    }
    for (const auto &e : typeParams) {
        if (e.exists() && !e.data(gs)->ignoreInHashing(gs)) {
            result = mix(result, _hash(e.data(gs)->name.data(gs)->shortName(gs)));
        }
    }

    return result;
}

bool Symbol::ignoreInHashing(const GlobalState &gs) const {
    if (isClass()) {
        return superClass == core::Symbols::StubClass();
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
    result.reserve(members.size());
    for (const auto &e : members) {
        result.emplace_back(e);
    }
    fast_sort(result, [&](auto const &lhs, auto const &rhs) -> bool {
        auto lhsShort = lhs.first.data(gs)->shortName(gs);
        auto rhsShort = rhs.first.data(gs)->shortName(gs);
        return lhsShort < rhsShort ||
               (lhsShort == rhsShort && lhs.first.data(gs)->toString(gs) < rhs.first.data(gs)->toString(gs));
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
