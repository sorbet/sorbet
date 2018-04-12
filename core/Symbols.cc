#include "Symbols.h"
#include "Context.h"
#include "Types.h"
#include "common/JSON.h"
#include "core/Names/core.h"
#include "core/errors/internal.h"
#include <sstream>
#include <string>

template class std::vector<ruby_typer::core::TypeAndOrigins>;
template class std::vector<ruby_typer::core::LocalVariable>;
template class std::vector<std::pair<ruby_typer::core::NameRef, ruby_typer::core::SymbolRef>>;

namespace ruby_typer {
namespace core {

using namespace std;

bool SymbolRef::operator==(const SymbolRef &rhs) const {
    return _id == rhs._id;
}

bool SymbolRef::operator!=(const SymbolRef &rhs) const {
    return !(rhs == *this);
}

std::vector<std::shared_ptr<core::Type>> Symbol::selfTypeArgs(const GlobalState &gs) const {
    ENFORCE(isClass()); // should be removed when we have generic methods
    std::vector<shared_ptr<core::Type>> targs;
    for (auto tm : typeMembers()) {
        if (tm.data(gs).isFixed()) {
            targs.emplace_back(tm.data(gs).resultType);
        } else {
            targs.emplace_back(make_shared<core::SelfTypeParam>(tm));
        }
    }
    return targs;
}
std::shared_ptr<core::Type> Symbol::selfType(const GlobalState &gs) const {
    ENFORCE(isClass());
    // todo: in dotty it made sense to cache those.
    if (typeMembers().empty()) {
        return make_shared<core::ClassType>(ref(gs));
    } else {
        return make_shared<core::AppliedType>(ref(gs), selfTypeArgs(gs));
    }
}

std::shared_ptr<core::Type> Symbol::externalType(const GlobalState &gs) const {
    ENFORCE(isClass());
    // todo: also cache these?
    if (typeMembers().empty()) {
        return make_shared<ClassType>(ref(gs));
    } else {
        std::vector<shared_ptr<Type>> targs;
        for (auto tm : typeMembers()) {
            if (tm.data(gs).isFixed()) {
                targs.emplace_back(tm.data(gs).resultType);
            } else {
                targs.emplace_back(Types::dynamic());
            }
        }
        return make_shared<AppliedType>(ref(gs), targs);
    }
}

bool Symbol::derivesFrom(const GlobalState &gs, SymbolRef sym) const {
    // TODO: add baseClassSet
    for (SymbolRef a : argumentsOrMixins) {
        if (a == sym || a.data(gs).derivesFrom(gs, sym)) {
            return true;
        }
    }
    if (this->superClass.exists()) {
        return sym == this->superClass || this->superClass.data(gs).derivesFrom(gs, sym);
    }
    return false;
}

SymbolRef Symbol::ref(const GlobalState &gs) const {
    auto distance = this - gs.symbols.data();
    return SymbolRef(gs, distance);
}

Symbol &SymbolRef::data(GlobalState &gs, bool allowNone) const {
    ENFORCE(_id < gs.symbols.size());
    if (!allowNone) {
        ENFORCE(this->exists());
    }

    return gs.symbols[this->_id];
}

const Symbol &SymbolRef::data(const GlobalState &gs, bool allowNone) const {
    ENFORCE(_id < gs.symbols.size());
    if (!allowNone) {
        ENFORCE(this->exists());
    }

    return gs.symbols[this->_id];
}

bool SymbolRef::isSynthetic() const {
    return this->_id < Symbols::MAX_SYNTHETIC_SYMBOLS;
}

void printTabs(ostringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

SymbolRef::SymbolRef(const GlobalState &from, u4 _id) : _id(_id) {}
SymbolRef::SymbolRef(GlobalState const *from, u4 _id) : _id(_id) {}

string SymbolRef::toString(const GlobalState &gs, int tabs, bool showHidden) const {
    return data(gs, true).toString(gs, tabs, showHidden);
}
string SymbolRef::show(const GlobalState &gs) const {
    return data(gs, true).show(gs);
}

SymbolRef Symbol::findMember(const GlobalState &gs, NameRef name) const {
    core::histogramInc("find_member_scope_size", members.size());
    for (auto &member : members) {
        if (member.first == name) {
            return member.second.data(gs).dealias(gs);
        }
    }
    return Symbols::noSymbol();
}

SymbolRef Symbol::findMemberTransitive(const GlobalState &gs, NameRef name, int maxDepth) const {
    ENFORCE(this->isClass());
    if (maxDepth == 0) {
        if (auto e = gs.beginError(core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("findMemberTransitive hit a loop while resolving `{}` in `{}`. Parents are: ",
                        name.toString(gs), this->fullName(gs));
        }
        int i = -1;
        for (auto it = this->argumentsOrMixins.rbegin(); it != this->argumentsOrMixins.rend(); ++it) {
            i++;
            if (auto e = gs.beginError(core::Loc::none(), core::errors::Internal::InternalError)) {
                e.setHeader("`{}`:- `{}`", i, it->data(gs).fullName(gs));
            }
            int j = 0;
            for (auto it2 = it->data(gs).argumentsOrMixins.rbegin(); it2 != it->data(gs).argumentsOrMixins.rend();
                 ++it2) {
                if (auto e = gs.beginError(core::Loc::none(), core::errors::Internal::InternalError)) {
                    e.setHeader("`{}`:`{}` `{}`", i, j, it2->data(gs).fullName(gs));
                }
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
        result = it->data(gs).findMemberTransitive(gs, name, maxDepth - 1);
        if (result.exists()) {
            return result;
        }
    }
    if (this->superClass.exists()) {
        return this->superClass.data(gs).findMemberTransitive(gs, name, maxDepth - 1);
    }
    return Symbols::noSymbol();
}

Symbol::FuzzySearchResult Symbol::findMemberFuzzyMatch(const GlobalState &gs, NameRef name, int betterThan) const {
    FuzzySearchResult result;
    result.symbol = Symbols::noSymbol();
    result.name = NameRef::noName();
    result.distance = betterThan;
    if (name.data(gs).kind != NameKind::UTF8) {
        return result;
    }

    auto currentName = name.data(gs).raw.utf8;
    if (result.distance < 0) {
        result.distance = 1 + (currentName.size() / 2);
    }

    for (auto pair : members) {
        auto thisName = pair.first;
        if (thisName.data(gs).kind != NameKind::UTF8) {
            continue;
        }
        auto utf8 = thisName.data(gs).raw.utf8;
        int thisDistance = Levenstein::distance(currentName, utf8, result.distance);
        if (thisDistance < result.distance) {
            result.distance = thisDistance;
            result.name = thisName;
            result.symbol = pair.second;
        }
    }

    for (auto it = this->argumentsOrMixins.rbegin(); it != this->argumentsOrMixins.rend(); ++it) {
        ENFORCE(it->exists());

        auto subResult = it->data(gs).findMemberFuzzyMatch(gs, name, result.distance);
        if (subResult.symbol.exists()) {
            ENFORCE(subResult.name.exists());
            ENFORCE(subResult.name.data(gs).kind == NameKind::UTF8);
            result = subResult;
        }
    }
    if (this->superClass.exists()) {
        auto subResult = this->superClass.data(gs).findMemberFuzzyMatch(gs, name, result.distance);
        if (subResult.symbol.exists()) {
            ENFORCE(subResult.name.exists());
            ENFORCE(subResult.name.data(gs).kind == NameKind::UTF8);
            result = subResult;
        }
    }
    return result;
}

string Symbol::fullName(const GlobalState &gs) const {
    string owner_str;
    if (this->owner.exists() && this->owner != core::Symbols::root()) {
        owner_str = this->owner.data(gs).fullName(gs);
    }

    if (this->isClass()) {
        return owner_str + "::" + this->name.show(gs);
    } else {
        return owner_str + "#" + this->name.show(gs);
    }
}

bool Symbol::isHiddenFromPrinting(const GlobalState &gs) const {
    if (ref(gs).isSynthetic()) {
        return true;
    }
    return definitionLoc.file.id() < 0 || definitionLoc.file.data(gs).source_type == File::Payload;
}

string Symbol::toString(const GlobalState &gs, int tabs, bool showHidden) const {
    ostringstream os;
    string name = this->name.toString(gs);
    auto &members = this->members;

    vector<string> children;
    children.reserve(members.size());
    for (auto pair : members) {
        if (pair.first == Names::singleton() || pair.first == Names::attached() ||
            pair.first == Names::classMethods()) {
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
    if (this->isClass()) {
        type = "class";
    } else if (this->isStaticField()) {
        type = "static-field";
    } else if (this->isField()) {
        type = "field";
    } else if (this->isMethod()) {
        type = "method";
    } else if (this->isMethodArgument()) {
        type = "argument";
    } else if (this->isTypeMember()) {
        type = "type-member";
    } else if (this->isTypeArgument()) {
        type = "type-argument";
    }

    if (this->isTypeArgument() || this->isTypeMember()) {
        char variance;
        if (this->isCovariant()) {
            variance = '+';
        } else if (this->isContravariant()) {
            variance = '-';
        } else if (this->isInvariant()) {
            variance = '=';
        } else {
            Error::raise("type without variance");
        }
        type = type + "(" + variance + ")";
    }

    os << type << " " << name;
    if (this->isClass() || this->isMethod()) {
        if (this->isMethod()) {
            if (this->isPrivate()) {
                os << " : private";
            } else if (this->isProtected()) {
                os << " : protected";
            }
        }

        auto typeMembers = this->isClass() ? this->typeMembers() : this->typeArguments();
        if (!typeMembers.empty()) {
            os << "[";
            bool first = true;
            for (SymbolRef thing : typeMembers) {
                if (first) {
                    first = false;
                } else {
                    os << ", ";
                }
                os << thing.data(gs).name.toString(gs);
            }
            os << "]";
        }

        if (this->superClass.exists()) {
            os << " < " << this->superClass.data(gs).fullName(gs);
        }
        os << " (";
        bool first = true;
        auto list = this->isClass() ? this->mixins() : this->arguments();
        for (SymbolRef thing : list) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << thing.data(gs).name.toString(gs);
        }
        os << ")";
    }
    if (this->isMethodArgument()) {
        vector<pair<int, const char *>> methodFlags = {
            {Symbol::Flags::ARGUMENT_OPTIONAL, "optional"},
            {Symbol::Flags::ARGUMENT_KEYWORD, "keyword"},
            {Symbol::Flags::ARGUMENT_REPEATED, "repeated"},
            {Symbol::Flags::ARGUMENT_BLOCK, "block"},
        };
        os << "<";
        bool first = true;
        for (auto &flag : methodFlags) {
            if ((this->flags & flag.first) != 0) {
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
    if (this->resultType) {
        os << " -> " << this->resultType->toString(gs, tabs);
    }
    os << " @ " << this->definitionLoc.filePosToString(gs);
    os << '\n';

    sort(children.begin(), children.end());
    for (auto row : children) {
        os << row;
    }
    return os.str();
}

string Symbol::toJSON(const GlobalState &gs, int tabs, bool showHidden) const {
    ostringstream os;
    auto &members = this->members;

    vector<string> children;
    children.reserve(members.size());
    for (auto pair : members) {
        if (pair.first == Names::singleton() || pair.first == Names::attached() ||
            pair.first == Names::classMethods()) {
            continue;
        }

        auto str = pair.second.data(gs, true).toJSON(gs, tabs + 1, showHidden);
        if (!str.empty()) {
            children.push_back(str);
        }
    }

    if (!showHidden && this->isHiddenFromPrinting(gs) && children.empty()) {
        return "";
    }

    os << "{" << endl;
    printTabs(os, tabs + 1);

    string name = this->name.show(gs);
    os << "\"name\" : \"" << core::JSON::escape(name) << "\"," << endl;
    printTabs(os, tabs + 1);

    string type = "unknown";
    if (this->isClass()) {
        type = "class";
    } else if (this->isStaticField()) {
        type = "static-field";
    } else if (this->isField()) {
        type = "field";
    } else if (this->isMethod()) {
        type = "method";
    } else if (this->isMethodArgument()) {
        type = "argument";
    } else if (this->isTypeMember()) {
        type = "type-member";
    } else if (this->isTypeArgument()) {
        type = "type-argument";
    }
    os << "\"type\" : \"" << type << "\"," << endl;
    printTabs(os, tabs + 1);

    if (this->isTypeArgument() || this->isTypeMember()) {
        char variance;
        if (this->isCovariant()) {
            variance = '+';
        } else if (this->isContravariant()) {
            variance = '-';
        } else if (this->isInvariant()) {
            variance = '=';
        } else {
            Error::raise("type without variance");
        }
        os << "\"variance\" : \"" << variance << "\"," << endl;
        printTabs(os, tabs + 1);
    }

    if (this->isClass() || this->isMethod()) {
        if (this->isMethod()) {
            string visibility = "public";
            if (this->isPrivate()) {
                visibility = "private";
            } else if (this->isProtected()) {
                visibility = "protected";
            }
            os << "\"visibility\" : \"" << visibility << "\"," << endl;
            printTabs(os, tabs + 1);
        }

        auto typeMembers = this->isClass() ? this->typeMembers() : this->typeArguments();
        if (!typeMembers.empty()) {
            os << "\"typeMembers\" : [" << endl;
            printTabs(os, tabs + 1);
            bool first = true;
            for (SymbolRef thing : typeMembers) {
                if (first) {
                    first = false;
                } else {
                    os << ", ";
                }
                printTabs(os, 1);
                os << "\"" << core::JSON::escape(thing.data(gs).name.show(gs)) << "\"" << endl;
                printTabs(os, tabs + 1);
            }
            os << "]," << endl;
            printTabs(os, tabs + 1);
        }

        string superclass = "null";
        if (this->superClass.exists()) {
            superclass = "\"" + core::JSON::escape(this->superClass.data(gs).fullName(gs)) + "\"";
        }
        os << "\"superclass\" : " << superclass << "," << endl;
        printTabs(os, tabs + 1);

        bool first = true;
        auto list = this->isClass() ? this->mixins() : this->arguments();
        auto key = this->isClass() ? "mixins" : "arguments";
        os << "\"" << key << "\" : [" << endl;
        printTabs(os, tabs + 1);
        for (SymbolRef thing : list) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            printTabs(os, 1);
            auto name = this->isClass() ? thing.data(gs).show(gs) : thing.data(gs).name.show(gs);
            os << "\"" << core::JSON::escape(name) << "\"" << endl;
            printTabs(os, tabs + 1);
        }
        os << "]," << endl;
        printTabs(os, tabs + 1);
    }
    if (this->isMethodArgument()) {
        vector<pair<int, const char *>> methodFlags = {
            {Symbol::Flags::ARGUMENT_OPTIONAL, "optional"},
            {Symbol::Flags::ARGUMENT_KEYWORD, "keyword"},
            {Symbol::Flags::ARGUMENT_REPEATED, "repeated"},
            {Symbol::Flags::ARGUMENT_BLOCK, "block"},
        };
        os << "\"flags\" : [" << endl;
        printTabs(os, tabs + 1);
        bool first = true;
        for (auto &flag : methodFlags) {
            if ((this->flags & flag.first) != 0) {
                if (first) {
                    first = false;
                } else {
                    os << ", ";
                }
                printTabs(os, 1);
                os << "\"" << flag.second << "\"";
                printTabs(os, tabs + 1);
            }
        }
        os << "]," << endl;
        printTabs(os, tabs + 1);
    }
    if (this->resultType) {
        string resultType = core::JSON::escape(this->resultType->show(gs));
        os << "\"resultType\" : \"" << resultType << "\"," << endl;
        printTabs(os, tabs + 1);
    }
    string loc = this->definitionLoc.toJSON(gs, tabs + 1);
    os << "\"loc\" : " << loc << "," << endl;
    printTabs(os, tabs + 1);

    sort(children.begin(), children.end());
    os << "\"children\" : [";
    bool first = true;
    for (auto row : children) {
        if (first) {
            first = false;
        } else {
            os << ", ";
        }
        os << row;
    }
    os << "]" << endl;
    printTabs(os, tabs);
    os << "}";
    return os.str();
}

string Symbol::show(const GlobalState &gs) const {
    string owner_str;

    if (this->isClass() && this->name.data(gs).kind == UNIQUE &&
        this->name.data(gs).unique.uniqueNameKind == UniqueNameKind::Singleton) {
        auto attached = this->attachedClass(gs);
        if (attached.exists()) {
            return "<Class:" + attached.data(gs).show(gs) + ">";
        }
    }

    if (this->owner.exists() && this->owner != core::Symbols::root()) {
        owner_str = this->owner.data(gs).show(gs);
        if (this->isClass()) {
            owner_str = owner_str + "::";
        } else {
            owner_str = owner_str + "#";
        }
    }

    return owner_str + this->name.data(gs).show(gs);
}

SymbolRef Symbol::singletonClass(GlobalState &gs) {
    auto singleton = lookupSingletonClass(gs);
    if (singleton.exists()) {
        return singleton;
    }
    SymbolRef selfRef = this->ref(gs);

    NameRef singletonName = gs.freshNameUnique(UniqueNameKind::Singleton, this->name, 1);
    singleton = gs.enterClassSymbol(this->definitionLoc, this->owner, singletonName);
    Symbol &singletonInfo = singleton.data(gs);

    core::counterInc("singleton_classes");
    singletonInfo.members.push_back(make_pair(Names::attached(), selfRef));
    singletonInfo.superClass = core::Symbols::todo();
    singletonInfo.setIsModule(false);

    selfRef.data(gs).members.push_back(make_pair(Names::singleton(), singleton));
    return singleton;
}

SymbolRef Symbol::lookupSingletonClass(const GlobalState &gs) const {
    ENFORCE(this->isClass());
    ENFORCE(this->name.data(gs).isClassName(gs));

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

SymbolRef Symbol::dealias(const GlobalState &gs) const {
    if (auto alias = cast_type<AliasType>(resultType.get())) {
        return alias->symbol.data(gs).dealias(gs);
    }
    return this->ref(gs);
}

bool Symbol::isBlockSymbol(const GlobalState &gs) const {
    const core::Name &nm = name.data(gs);
    return nm.kind == NameKind::UNIQUE && nm.unique.original == Names::blockTemp();
}

SymbolRef SymbolRef::dealiasAt(GlobalState &gs, core::SymbolRef klass) const {
    ENFORCE(data(gs).isTypeMember());
    if (data(gs).owner == klass) {
        return *this;
    } else {
        SymbolRef cursor;
        if (data(gs).owner.data(gs).derivesFrom(gs, klass)) {
            cursor = data(gs).owner;
        } else if (klass.data(gs).derivesFrom(gs, data(gs).owner)) {
            cursor = klass;
        }
        while (true) {
            if (!cursor.exists()) {
                return cursor;
            }
            for (auto aliasPair : cursor.data(gs).typeAliases) {
                if (aliasPair.first == *this) {
                    return aliasPair.second.dealiasAt(gs, klass);
                }
            }
            cursor = cursor.data(gs).superClass;
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
    result.typeParams = this->typeParams;
    result.typeAliases = this->typeAliases;

    result.members.reserve(this->members.size());
    for (auto &mem : this->members) {
        result.members.emplace_back(NameRef(to, mem.first.id()), mem.second);
    }
    result.superClass = this->superClass;
    result.uniqueCounter = this->uniqueCounter;
    return result;
}

int Symbol::typeArity(const GlobalState &gs) const {
    ENFORCE(this->isClass());
    int arity = 0;
    for (auto &ty : this->typeMembers()) {
        if (!ty.data(gs).isFixed()) {
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
            const_cast<GlobalState &>(gs).enterSymbol(this->definitionLoc, this->owner, this->name, this->flags);
        ENFORCE(current == current2);
    }
}

SymbolRef Symbol::enclosingMethod(const GlobalState &gs) const {
    if (isMethod()) {
        return ref(gs);
    }
    SymbolRef owner = this->owner;
    while (owner != Symbols::root() && !owner.data(gs, false).isMethod()) {
        ENFORCE(owner.exists(), "non-existing owner in enclosingMethod");
        owner = owner.data(gs).owner;
    }
    return owner;
}

SymbolRef Symbol::enclosingClass(const GlobalState &gs) const {
    SymbolRef owner = ref(gs);
    while (!owner.data(gs, false).isClass()) {
        ENFORCE(owner.exists(), "non-existing owner in enclosingClass");
        owner = owner.data(gs).owner;
    }
    return owner;
}

LocalVariable::LocalVariable(NameRef name, u4 unique) : _name(name), unique(unique) {}
LocalVariable::LocalVariable() : _name() {}
bool LocalVariable::exists() const {
    return _name._id > 0;
}

bool LocalVariable::isSyntheticTemporary(const GlobalState &gs) const {
    if (_name.data(gs).kind == NameKind::UNIQUE) {
        return true;
    }
    if (unique == 0) {
        return false;
    }
    return _name == core::Names::whileTemp() || _name == core::Names::ifTemp() || _name == core::Names::returnTemp() ||
           _name == core::Names::statTemp() || _name == core::Names::assignTemp() ||
           _name == core::Names::returnMethodTemp() || _name == core::Names::blockReturnTemp() ||
           _name == core::Names::selfMethodTemp() || _name == core::Names::hashTemp() ||
           _name == core::Names::arrayTemp() || _name == core::Names::rescueTemp() ||
           _name == core::Names::castTemp() || _name == core::Names::finalReturn();
}

bool LocalVariable::isAliasForGlobal(const GlobalState &gs) const {
    return _name == core::Names::cfgAlias();
}

bool LocalVariable::operator==(const LocalVariable &rhs) const {
    return this->_name == rhs._name && this->unique == rhs.unique;
}

bool LocalVariable::operator!=(const LocalVariable &rhs) const {
    return !this->operator==(rhs);
}
std::string LocalVariable::toString(const core::GlobalState &gs) const {
    if (unique == 0) {
        return this->_name.toString(gs);
    }
    return this->_name.toString(gs) + "$" + std::to_string(this->unique);
}

} // namespace core
} // namespace ruby_typer
