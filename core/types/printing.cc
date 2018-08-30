#include "absl/base/casts.h"
#include "absl/strings/escaping.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"

using namespace std;

namespace sorbet {
namespace core {

namespace {

void printTabs(stringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}
} // namespace

string ClassType::toString(const GlobalState &gs, int tabs) const {
    return this->symbol.data(gs).show(gs);
}

string ClassType::show(const GlobalState &gs) const {
    return this->symbol.data(gs).show(gs);
}

string ClassType::typeName() const {
    return "ClassType";
}

string LiteralType::typeName() const {
    return "LiteralType";
}

string LiteralType::toString(const GlobalState &gs, int tabs) const {
    return this->underlying()->toString(gs, tabs) + "(" + showValue(gs) + ")";
}

string LiteralType::show(const GlobalState &gs) const {
    return this->underlying()->show(gs) + "(" + showValue(gs) + ")";
}

string LiteralType::showValue(const GlobalState &gs) const {
    string value;
    SymbolRef undSymbol = cast_type<ClassType>(this->underlying().get())->symbol;
    if (undSymbol == Symbols::String()) {
        value = "\"" + NameRef(gs, this->value).toString(gs) + "\"";
    } else if (undSymbol == Symbols::Symbol()) {
        value = ":\"" + NameRef(gs, this->value).toString(gs) + "\"";
    } else if (undSymbol == Symbols::Integer()) {
        value = to_string(this->value);
    } else if (undSymbol == Symbols::Float()) {
        value = to_string(absl::bit_cast<double>(this->value));
    } else if (undSymbol == Symbols::TrueClass()) {
        value = "true";
    } else if (undSymbol == Symbols::FalseClass()) {
        value = "false";
    } else {
        Error::raise("should not be reachable");
    }
    return value;
}

string TupleType::typeName() const {
    return "TupleType";
}

string ShapeType::typeName() const {
    return "ShapeType";
}

string AliasType::typeName() const {
    return "AliasType";
}

string AndType::typeName() const {
    return "AndType";
}

string OrType::typeName() const {
    return "OrType";
}

string TupleType::toString(const GlobalState &gs, int tabs) const {
    stringstream buf;
    buf << "TupleType {" << '\n';
    int i = -1;
    for (auto &el : this->elems) {
        i++;
        printTabs(buf, tabs + 1);
        buf << i << " = " << el->toString(gs, tabs + 3) << '\n';
    }
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string TupleType::show(const GlobalState &gs) const {
    stringstream buf;
    buf << "[";
    bool first = true;
    for (auto &el : this->elems) {
        if (first) {
            first = false;
        } else {
            buf << ", ";
        }
        buf << el->show(gs);
    }
    buf << "]";
    return buf.str();
}

string ShapeType::toString(const GlobalState &gs, int tabs) const {
    stringstream buf;
    buf << "ShapeType {" << '\n';
    auto valueIterator = this->values.begin();
    for (auto &el : this->keys) {
        printTabs(buf, tabs + 1);
        buf << el->toString(gs, tabs + 2) << " => " << (*valueIterator)->toString(gs, tabs + 2) << '\n';
        ++valueIterator;
    }
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string ShapeType::show(const GlobalState &gs) const {
    stringstream buf;
    buf << "{";
    auto valueIterator = this->values.begin();
    bool first = true;
    for (auto &key : this->keys) {
        if (first) {
            first = false;
        } else {
            buf << ", ";
        }
        SymbolRef undSymbol = cast_type<ClassType>(key->underlying().get())->symbol;
        if (undSymbol == Symbols::Symbol()) {
            buf << NameRef(gs, key->value).toString(gs) << ": ";
        } else {
            buf << key->show(gs) << " => ";
        }
        buf << (*valueIterator)->show(gs);
        ++valueIterator;
    }
    buf << "}";
    return buf.str();
}

string AliasType::toString(const GlobalState &gs, int tabs) const {
    stringstream buf;
    buf << "AliasType { symbol = " << this->symbol.data(gs).fullName(gs) << " }";
    return buf.str();
}

string AliasType::show(const GlobalState &gs) const {
    stringstream buf;
    buf << "<Alias:" << this->symbol.data(gs).fullName(gs) << ">";
    return buf.str();
}

string AndType::toString(const GlobalState &gs, int tabs) const {
    stringstream buf;
    bool leftBrace = isa_type<OrType>(this->left.get());
    bool rightBrace = isa_type<OrType>(this->right.get());

    if (leftBrace) {
        buf << "(";
    }
    buf << this->left->toString(gs, tabs + 2);
    if (leftBrace) {
        buf << ")";
    }
    buf << " & ";
    if (rightBrace) {
        buf << "(";
    }
    buf << this->right->toString(gs, tabs + 2);
    if (rightBrace) {
        buf << ")";
    }
    return buf.str();
}

string AndType::show(const GlobalState &gs) const {
    stringstream buf;

    buf << "T.all(";
    buf << this->left->show(gs);
    buf << ", ";
    buf << this->right->show(gs);
    buf << ")";
    return buf.str();
}

string OrType::toString(const GlobalState &gs, int tabs) const {
    stringstream buf;
    bool leftBrace = isa_type<AndType>(this->left.get());
    bool rightBrace = isa_type<AndType>(this->right.get());

    if (leftBrace) {
        buf << "(";
    }
    buf << this->left->toString(gs, tabs + 2);
    if (leftBrace) {
        buf << ")";
    }
    buf << " | ";
    if (rightBrace) {
        buf << "(";
    }
    buf << this->right->toString(gs, tabs + 2);
    if (rightBrace) {
        buf << ")";
    }
    return buf.str();
}

string showOrs(const GlobalState &gs, shared_ptr<Type> left, shared_ptr<Type> right) {
    stringstream buf;
    auto *lt = cast_type<OrType>(left.get());
    if (lt != nullptr) {
        buf << showOrs(gs, lt->left, lt->right);
    } else {
        buf << left->show(gs);
    }
    buf << ", ";

    auto *rt = cast_type<OrType>(right.get());
    if (rt != nullptr) {
        buf << showOrs(gs, rt->left, rt->right);
    } else {
        buf << right->show(gs);
    }
    return buf.str();
}

string showOrSpecialCase(const GlobalState &gs, shared_ptr<Type> type, shared_ptr<Type> rest) {
    auto *ct = cast_type<ClassType>(type.get());
    if (ct != nullptr && ct->symbol == Symbols::NilClass()) {
        stringstream buf;
        buf << "T.nilable(";
        buf << rest->show(gs);
        buf << ")";
        return buf.str();
    }
    return "";
}

string OrType::show(const GlobalState &gs) const {
    stringstream buf;

    string ret;
    if (!(ret = showOrSpecialCase(gs, this->left, this->right)).empty()) {
        return ret;
    }
    if (!(ret = showOrSpecialCase(gs, this->right, this->left)).empty()) {
        return ret;
    }

    buf << "T.any(";
    buf << showOrs(gs, this->left, this->right);
    buf << ")";
    return buf.str();
}

string TypeVar::toString(const GlobalState &gs, int tabs) const {
    return "TypeVar(" + sym.data(gs).name.toString(gs) + ")";
}

string TypeVar::show(const GlobalState &gs) const {
    return sym.data(gs).name.toString(gs);
}

string TypeVar::typeName() const {
    return "TypeVar";
}

string AppliedType::toString(const GlobalState &gs, int tabs) const {
    stringstream buf;
    buf << "AppliedType {" << '\n';
    printTabs(buf, tabs + 1);
    buf << "klass = " << this->klass.data(gs).fullName(gs) << '\n';

    printTabs(buf, tabs + 1);
    buf << "targs = [" << '\n';
    int i = -1;
    for (auto &targ : this->targs) {
        ++i;
        printTabs(buf, tabs + 2);
        if (i < this->klass.data(gs).typeMembers().size()) {
            auto tyMem = this->klass.data(gs).typeMembers()[i];
            buf << tyMem.data(gs).name.toString(gs) << " = " << targ->toString(gs, tabs + 3) << '\n';
        } else {
            // this happens if we try to print type before resolver has processed stdlib
            buf << "EARLY_TYPE_MEMBER\n";
        }
    }
    printTabs(buf, tabs + 1);
    buf << "]" << '\n';

    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string AppliedType::show(const GlobalState &gs) const {
    stringstream buf;
    if (this->klass == Symbols::Array()) {
        buf << "T::Array";
    } else if (this->klass == Symbols::Hash()) {
        buf << "T::Hash";
    } else {
        buf << this->klass.data(gs).show(gs);
    }
    buf << "[";

    bool first = true;
    for (auto &targ : this->targs) {
        if (this->klass == Symbols::Hash() && &targ == &this->targs.back()) {
            break;
        }
        if (first) {
            first = false;
        } else {
            buf << ", ";
        }
        buf << targ->show(gs);
    }

    buf << "]";
    return buf.str();
}

string AppliedType::typeName() const {
    return "AppliedType";
}

string LambdaParam::toString(const GlobalState &gs, int tabs) const {
    return "LambdaParam(" + this->definition.data(gs).fullName(gs) + ")";
}

string LambdaParam::show(const GlobalState &gs) const {
    return this->definition.data(gs).show(gs);
}

string SelfTypeParam::toString(const GlobalState &gs, int tabs) const {
    return "SelfTypeParam(" + this->definition.data(gs).fullName(gs) + ")";
}

string SelfTypeParam::show(const GlobalState &gs) const {
    return this->definition.data(gs).show(gs);
}

string LambdaParam::typeName() const {
    return "LambdaParam";
}

string SelfTypeParam::typeName() const {
    return "SelfTypeParam";
}

string SelfType::toString(const GlobalState &gs, int tabs) const {
    return show(gs);
}

string SelfType::show(const GlobalState &gs) const {
    return "T.self_type()";
}

string SelfType::showValue(const GlobalState &gs) const {
    return show(gs);
}
} // namespace core
} // namespace sorbet