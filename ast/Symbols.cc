#include "Symbols.h"
#include "Context.h"
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
bool Symbol::isConstructor(GlobalState &ctx) const {
    return this->name._id == 1;
}
Symbol &SymbolRef::info(GlobalState &ctx, bool allowNone) const {
    Error::check(_id < ctx.symbols.size());
    if (!allowNone)
        Error::check(this->exists());

    return ctx.symbols[this->_id];
}

bool SymbolRef::isSynthetic() const {
    return this->_id <= GlobalState::defn_last_synthetic_sym()._id;
}

bool SymbolRef::isPlaceHolder() const {
    return this->_id >= GlobalState::defn_todo()._id && this->_id <= GlobalState::defn_cvar_todo()._id;
}

void printTabs(std::ostringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

std::string SymbolRef::toString(GlobalState &ctx, int tabs) const {
    std::ostringstream os;
    Symbol &myInfo = info(ctx, true);
    std::string name = myInfo.name.toString(ctx);
    auto members = myInfo.members;

    printTabs(os, tabs);

    std::string type = "unknown";
    if (myInfo.isClass()) {
        type = "class";
    } else if (myInfo.isArray()) {
        type = "array";
    } else if (myInfo.isField()) {
        type = "field";
    } else if (myInfo.isMethod()) {
        type = "method";
    }
    os << type << " " << name;
    os << " (";
    bool first = true;
    for (SymbolRef thing : myInfo.argumentsOrMixins) {
        if (first) {
            first = false;
        } else {
            os << ", ";
        }
        os << thing.info(ctx).name.toString(ctx);
    }
    os << ")";
    os << std::endl;

    std::vector<std::string> children;
    for (auto pair : members) {
        children.push_back(pair.second.toString(ctx, tabs + 1));
    }
    std::sort(children.begin(), children.end());
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

string Symbol::fullName(GlobalState &ctx) const {
    string owner_str;
    if (this->owner.exists() && this->owner != ctx.defn_root())
        owner_str = this->owner.info(ctx).fullName(ctx);

    if (this->isClass())
        return owner_str + "::" + this->name.toString(ctx);
    else
        return owner_str + "#" + this->name.toString(ctx);
}

} // namespace ast
} // namespace ruby_typer
