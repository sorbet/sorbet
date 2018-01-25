#include "Symbols.h"
#include "Context.h"
#include "Types.h"
#include "core/Names/core.h"
#include <sstream>
#include <string>

template class std::vector<ruby_typer::core::TypeAndOrigins>;
template class std::vector<ruby_typer::core::LocalVariable>;

namespace ruby_typer {
namespace core {

using namespace std;

bool SymbolRef::operator==(const SymbolRef &rhs) const {
    return _id == rhs._id;
}

bool SymbolRef::operator!=(const SymbolRef &rhs) const {
    return !(rhs == *this);
}

bool SymbolRef::isPrimitive() const {
    Error::notImplemented();
}
bool Symbol::isConstructor(GlobalState &gs) const {
    return this->name._id == 1;
}

std::vector<std::shared_ptr<core::Type>> Symbol::selfTypeArgs(GlobalState &gs) {
    ENFORCE(isClass()); // should be removed when we have generic methods
    std::vector<shared_ptr<core::Type>> targs;
    for (auto tm : typeMembers()) {
        targs.emplace_back(make_shared<core::SelfTypeParam>(tm));
    }
    return targs;
}
std::shared_ptr<core::Type> Symbol::selfType(GlobalState &gs) {
    ENFORCE(isClass());
    // todo: in dotty it made sense to cache those.
    if (typeMembers().empty()) {
        return make_shared<core::ClassType>(ref(gs));
    } else {
        return make_shared<core::AppliedType>(ref(gs), selfTypeArgs(gs));
    }
}

bool Symbol::derivesFrom(GlobalState &gs, SymbolRef sym) {
    // TODO: add baseClassSet
    for (SymbolRef a : argumentsOrMixins) {
        if (a == sym || a.info(gs).derivesFrom(gs, sym)) {
            return true;
        }
    }
    if (this->superClass.exists()) {
        return sym == this->superClass || this->superClass.info(gs).derivesFrom(gs, sym);
    }
    return false;
}

SymbolRef Symbol::ref(const GlobalState &gs) const {
    auto distance = this - gs.symbols.data();
    return SymbolRef(gs, distance);
}

Symbol &SymbolRef::info(GlobalState &gs, bool allowNone) const {
    ENFORCE(_id < gs.symbols.size());
    if (!allowNone) {
        ENFORCE(this->exists());
    }

    return gs.symbols[this->_id];
}

bool SymbolRef::isSynthetic() const {
    return this->_id < GlobalState::MAX_SYNTHETIC_SYMBOLS;
}

bool SymbolRef::isHiddenFromPrinting(GlobalState &gs) const {
    if (isSynthetic()) {
        return true;
    }
    auto loc = info(gs).definitionLoc;
    return loc.file.id() < 0 || loc.file.file(gs).source_type == File::Payload;
}

void printTabs(ostringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

string SymbolRef::toString(GlobalState &gs, int tabs, bool showHidden) const {
    ostringstream os;
    Symbol &myInfo = info(gs, true);
    string name = myInfo.name.toString(gs);
    auto &members = myInfo.members;

    vector<string> children;
    children.reserve(members.size());
    for (auto pair : members) {
        if (pair.first == Names::singletonClass() || pair.first == Names::attachedClass()) {
            continue;
        }

        auto str = pair.second.toString(gs, tabs + 1, showHidden);
        if (!str.empty()) {
            children.push_back(str);
        }
    }

    if (!showHidden && this->isHiddenFromPrinting(gs) && children.empty()) {
        return "";
    }

    printTabs(os, tabs);

    string type = "unknown";
    if (myInfo.isClass()) {
        type = "class";
    } else if (myInfo.isStaticField()) {
        type = "static-field";
    } else if (myInfo.isField()) {
        type = "field";
    } else if (myInfo.isMethod()) {
        type = "method";
    } else if (myInfo.isMethodArgument()) {
        type = "argument";
    } else if (myInfo.isTypeMember()) {
        type = "typeMember";
    } else if (myInfo.isTypeArgument()) {
        type = "type argument";
    }

    if (myInfo.isTypeArgument() || myInfo.isTypeMember()) {
        char variance;
        if (myInfo.isCovariant()) {
            variance = '+';
        } else if (myInfo.isContravariant()) {
            variance = '-';
        } else if (myInfo.isInvariant()) {
            variance = '=';
        } else {
            Error::raise("type without variance");
        }
        type = type + "(" + variance + ")";
    }

    os << type << " " << name;
    if (myInfo.isClass() || myInfo.isMethod()) {
        if (myInfo.isMethod()) {
            if (myInfo.isPrivate()) {
                os << " : private";
            } else if (myInfo.isProtected()) {
                os << " : protected";
            }
        }

        auto typeMembers = myInfo.isClass() ? myInfo.typeMembers() : myInfo.typeArguments();
        if (!typeMembers.empty()) {
            os << "[";
            bool first = true;
            for (SymbolRef thing : typeMembers) {
                if (first) {
                    first = false;
                } else {
                    os << ", ";
                }
                os << thing.info(gs).name.toString(gs);
            }
            os << "]";
        }

        if (myInfo.superClass.exists()) {
            os << " < " << myInfo.superClass.info(gs).fullName(gs);
        }
        os << " (";
        bool first = true;
        for (SymbolRef thing : myInfo.argumentsOrMixins) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << thing.info(gs).name.toString(gs);
        }
        os << ")";
    }
    if (myInfo.isMethodArgument()) {
        vector<pair<int, const char *>> methodFlags = {
            {Symbol::Flags::ARGUMENT_OPTIONAL, "optional"},
            {Symbol::Flags::ARGUMENT_KEYWORD, "keyword"},
            {Symbol::Flags::ARGUMENT_REPEATED, "repeated"},
            {Symbol::Flags::ARGUMENT_BLOCK, "block"},
        };
        os << "<";
        bool first = true;
        for (auto &flag : methodFlags) {
            if ((myInfo.flags & flag.first) != 0) {
                if (first) {
                    first = false;
                } else {
                    os << ", ";
                }
                os << flag.second;
            }
        }
        os << ">";
    }
    if (myInfo.resultType) {
        os << " -> " << myInfo.resultType->toString(Context(gs, gs.defn_root()), tabs);
    }
    os << endl;

    sort(children.begin(), children.end());
    for (auto row : children) {
        os << row;
    }
    return os.str();
}

SymbolRef::SymbolRef(const GlobalState &from, u4 _id) : _id(_id) {}
SymbolRef::SymbolRef(GlobalState const *from, u4 _id) : _id(_id) {}

SymbolRef Symbol::findMember(GlobalState &gs, NameRef name) {
    histogramInc("find_member_scope_size", members.size());
    for (auto &member : members) {
        if (member.first == name) {
            return member.second.info(gs).dealias(gs);
        }
    }
    return SymbolRef();
}

SymbolRef Symbol::findMemberTransitive(GlobalState &gs, NameRef name, int maxDepth) {
    ENFORCE(this->isClass());
    if (maxDepth == 0) {
        gs.logger.critical("findMemberTransitive hit a loop while resolving {} in {}. Parents are: ", name.toString(gs),
                           this->fullName(gs));
        int i = -1;
        for (auto it = this->argumentsOrMixins.rbegin(); it != this->argumentsOrMixins.rend(); ++it) {
            i++;
            gs.logger.critical("{}:- {}", i, it->info(gs).fullName(gs));
            int j = 0;
            for (auto it2 = it->info(gs).argumentsOrMixins.rbegin(); it2 != it->info(gs).argumentsOrMixins.rend();
                 ++it2) {
                gs.logger.critical("{}:{} {}", i, j, it2->info(gs).fullName(gs));
                j++;
            }
        }

        Error::raise("findMemberTransitive hit a loop while resolving ");
    }

    SymbolRef result = findMember(gs, name);
    if (result.exists()) {
        return result;
    }
    for (auto it = this->argumentsOrMixins.rbegin(); it != this->argumentsOrMixins.rend(); ++it) {
        ENFORCE(it->exists());
        result = it->info(gs).findMemberTransitive(gs, name, maxDepth - 1);
        if (result.exists()) {
            return result;
        }
    }
    if (this->superClass.exists()) {
        return this->superClass.info(gs).findMemberTransitive(gs, name, maxDepth - 1);
    }
    return SymbolRef();
}

string Symbol::fullName(GlobalState &gs) const {
    string owner_str;
    if (this->owner.exists() && this->owner != gs.defn_root()) {
        owner_str = this->owner.info(gs).fullName(gs);
    }

    if (this->isClass()) {
        return owner_str + "::" + this->name.toString(gs);
    } else {
        return owner_str + "#" + this->name.toString(gs);
    }
}

SymbolRef Symbol::singletonClass(GlobalState &gs) {
    ENFORCE(this->isClass());
    ENFORCE(this->name.name(gs).isClassName(gs));

    SymbolRef selfRef = this->ref(gs);
    if (selfRef == GlobalState::defn_untyped()) {
        return GlobalState::defn_untyped();
    }

    SymbolRef singleton = findMember(gs, Names::singletonClass());
    if (singleton.exists()) {
        return singleton;
    }

    NameRef singletonName = gs.freshNameUnique(UniqueNameKind::Singleton, this->name, 1);
    singleton = gs.enterClassSymbol(this->definitionLoc, this->owner, singletonName);
    Symbol &singletonInfo = singleton.info(gs);

    counterInc("singleton_classes");
    singletonInfo.members.push_back(make_pair(Names::attachedClass(), selfRef));
    singletonInfo.superClass = core::GlobalState::defn_todo();
    singletonInfo.setIsModule(false);

    selfRef.info(gs).members.push_back(make_pair(Names::singletonClass(), singleton));
    return singleton;
}

SymbolRef Symbol::attachedClass(GlobalState &gs) {
    ENFORCE(this->isClass());
    if (this->ref(gs) == GlobalState::defn_untyped()) {
        return GlobalState::defn_untyped();
    }

    SymbolRef singleton = findMember(gs, Names::attachedClass());
    return singleton;
}

SymbolRef Symbol::dealias(GlobalState &gs) {
    if (auto alias = cast_type<AliasType>(resultType.get())) {
        return alias->symbol.info(gs).dealias(gs);
    }
    return this->ref(gs);
}

bool Symbol::isBlockSymbol(GlobalState &gs) const {
    core::Name &nm = name.name(gs);
    return nm.kind == NameKind::UNIQUE && nm.unique.original == Names::blockTemp();
}

SymbolRef SymbolRef::dealiasAt(GlobalState &gs, core::SymbolRef klass) {
    ENFORCE(info(gs).isTypeMember());
    if (info(gs).owner == klass) {
        return *this;
    } else {
        SymbolRef cursor;
        if (info(gs).owner.info(gs).derivesFrom(gs, klass)) {
            cursor = info(gs).owner;
        } else if (klass.info(gs).derivesFrom(gs, info(gs).owner)) {
            cursor = klass;
        }
        while (true) {
            if (!cursor.exists()) {
                return cursor;
            }
            for (auto aliasPair : cursor.info(gs).typeAliases) {
                if (aliasPair.first == *this) {
                    return aliasPair.second.dealiasAt(gs, klass);
                }
            }
            cursor = cursor.info(gs).superClass;
        }
    }
}

Symbol Symbol::deepCopy(const GlobalState &to) const {
    Symbol result;
    result.owner = this->owner;
    result.flags = this->flags;
    result.argumentsOrMixins = this->argumentsOrMixins;
    result.resultType = this->resultType;
    result.name = NameRef(to, this->name.id());
    result.definitionLoc = this->definitionLoc;
    result.definitionLoc.file = FileRef(to, this->definitionLoc.file.id());

    result.members.reserve(this->members.size());
    for (auto &mem : this->members) {
        result.members.emplace_back(NameRef(to, mem.first.id()), mem.second);
    }
    result.superClass = this->superClass;
    result.uniqueCounter = this->uniqueCounter;
    return result;
}

void Symbol::sanityCheck(const GlobalState &gs) const {
    if (!debug_mode) {
        return;
    }
    SymbolRef current = this->ref(gs);
    if (current != GlobalState::defn_root()) {
        SymbolRef current2 =
            const_cast<GlobalState &>(gs).enterSymbol(this->definitionLoc, this->owner, this->name, this->flags);
        ENFORCE(current == current2);
    }
}

LocalVariable::LocalVariable(NameRef name) : name(name) {}
LocalVariable::LocalVariable() : name() {}
bool LocalVariable::exists() {
    return name._id > 0;
}

bool LocalVariable::isSyntheticTemporary(GlobalState &gs) const {
    return name.name(gs).kind == NameKind::UNIQUE;
}

bool LocalVariable::isAliasForGlobal(GlobalState &gs) const {
    return name.name(gs).kind == NameKind::UNIQUE && name.name(gs).unique.uniqueNameKind == UniqueNameKind::CFGAlias;
}

bool LocalVariable::operator==(const LocalVariable &rhs) const {
    return this->name == rhs.name;
}

bool LocalVariable::operator!=(const LocalVariable &rhs) const {
    return this->name != rhs.name;
}

} // namespace core
} // namespace ruby_typer
