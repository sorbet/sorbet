#include "Symbols.h"
#include "Context.h"
#include "Types.h"
#include <sstream>
#include <string>

namespace ruby_typer {
namespace ast {

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

bool Symbol::derivesFrom(GlobalState &gs, SymbolRef sym) {
    // TODO: add baseClassSet
    for (SymbolRef a : argumentsOrMixins) {
        if (a == sym || a.info(gs).derivesFrom(gs, sym))
            return true;
    }
    return false;
}

SymbolRef Symbol::ref(GlobalState &gs) const {
    auto distance = this - gs.symbols.data();
    return SymbolRef(distance);
}

Symbol &SymbolRef::info(GlobalState &gs, bool allowNone) const {
    Error::check(_id < gs.symbols.size());
    if (!allowNone)
        Error::check(this->exists());

    return gs.symbols[this->_id];
}

bool SymbolRef::isSynthetic() const {
    return this->_id <= GlobalState::defn_last_synthetic_sym()._id;
}

bool SymbolRef::isHiddenFromPrinting() const {
    return isSynthetic() && _id != GlobalState::defn_Opus()._id;
}

void printTabs(ostringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

string SymbolRef::toString(GlobalState &gs, int tabs) const {
    ostringstream os;
    Symbol &myInfo = info(gs, true);
    string name = myInfo.name.toString(gs);
    auto members = myInfo.members;

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
    }
    os << type << " " << name;
    if (!myInfo.isField() && !myInfo.isStaticField()) {
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
    if (myInfo.resultType) {
        os << " -> " << myInfo.resultType->toString(Context(gs, gs.defn_root()), tabs + 2);
    }
    os << endl;

    vector<string> children;
    for (auto pair : members) {
        if (pair.first == Names::singletonClass() || pair.first == Names::attachedClass())
            continue;
        if (pair.second.isHiddenFromPrinting())
            continue;

        children.push_back(pair.second.toString(gs, tabs + 1));
    }
    sort(children.begin(), children.end());
    for (auto row : children) {
        os << row;
    }
    return os.str();
}

SymbolRef Symbol::findMember(NameRef name) {
    for (auto &member : members) {
        if (member.first == name)
            return member.second;
    }
    return SymbolRef(0);
}

SymbolRef Symbol::findMemberTransitive(GlobalState &gs, NameRef name) {
    Error::check(this->isClass());

    SymbolRef result = findMember(name);
    if (result.exists())
        return result;
    for (auto it = this->argumentsOrMixins.rbegin(); it != this->argumentsOrMixins.rend(); ++it) {
        Error::check(it->exists());
        result = it->info(gs).findMemberTransitive(gs, name);
        if (result.exists())
            return result;
    }
    return SymbolRef(0);
}

string Symbol::fullName(GlobalState &gs) const {
    string owner_str;
    if (this->owner.exists() && this->owner != gs.defn_root())
        owner_str = this->owner.info(gs).fullName(gs);

    if (this->isClass())
        return owner_str + "::" + this->name.toString(gs);
    else
        return owner_str + "#" + this->name.toString(gs);
}

bool Symbol::isSyntheticTemporary(GlobalState &gs) const {
    return name.name(gs).kind == NameKind::UNIQUE;
}

SymbolRef Symbol::singletonClass(GlobalState &gs) {
    Error::check(this->isClass());

    SymbolRef singleton = findMember(Names::singletonClass());
    if (singleton.exists())
        return singleton;

    NameRef singletonName = gs.freshNameUnique(UniqueNameKind::Singleton, this->name);
    singleton = gs.enterClassSymbol(this->owner, singletonName);
    Symbol &singletonInfo = singleton.info(gs);

    singletonInfo.members.push_back(make_pair(Names::attachedClass(), this->ref(gs)));
    singletonInfo.argumentsOrMixins.push_back(gs.defn_Class());

    this->members.push_back(make_pair(Names::singletonClass(), singleton));
    return singleton;
}

SymbolRef Symbol::attachedClass(GlobalState &gs) {
    Error::check(this->isClass());

    SymbolRef singleton = findMember(Names::attachedClass());
    return singleton;
}

} // namespace ast
} // namespace ruby_typer
