#include "core/Types.h"
#include "common/common.h"

using namespace ruby_typer;
using namespace ruby_typer::core;
using namespace std;

// improve debugging.
template class std::shared_ptr<ruby_typer::core::Type>;
template class std::vector<core::Loc>;

shared_ptr<Type> Types::top() {
    return make_shared<ClassType>(core::GlobalState::defn_top());
}

shared_ptr<Type> Types::bottom() {
    return make_shared<ClassType>(core::GlobalState::defn_bottom());
}

shared_ptr<Type> Types::nil() {
    return make_shared<ClassType>(core::GlobalState::defn_NilClass());
}

shared_ptr<Type> Types::dynamic() {
    return make_shared<ClassType>(core::GlobalState::defn_untyped());
}

ruby_typer::core::ClassType::ClassType(ruby_typer::core::SymbolRef symbol) : symbol(symbol) {}

namespace {
string classNameToString(core::Context ctx, core::NameRef nm) {
    core::Name &name = nm.name(ctx);
    if (name.kind == core::CONSTANT) {
        return name.cnst.original.toString(ctx);
    } else {
        Error::check(name.kind == core::UNIQUE);
        Error::check(name.unique.uniqueNameKind == core::Singleton);
        return "<class:" + classNameToString(ctx, name.unique.original) + ">";
    }
}
}; // namespace

string ruby_typer::core::ClassType::toString(ruby_typer::core::Context ctx, int tabs) {
    return classNameToString(ctx, this->symbol.info(ctx).name);
}

string ruby_typer::core::ClassType::typeName() {
    return "ClassType";
}

ruby_typer::core::ProxyType::ProxyType(shared_ptr<ruby_typer::core::Type> underlying) : underlying(move(underlying)) {}

bool Type::isDynamic() {
    auto *t = dynamic_cast<ClassType *>(this);
    return t != nullptr && t->symbol == core::GlobalState::defn_untyped();
}

// TODO: somehow reuse existing references instead of allocating new ones.
ruby_typer::core::LiteralType::LiteralType(int64_t val)
    : ProxyType(make_shared<ClassType>(core::GlobalState::defn_Integer())), value(val) {}

ruby_typer::core::LiteralType::LiteralType(double val)
    : ProxyType(make_shared<ClassType>(core::GlobalState::defn_Float())), value(*reinterpret_cast<u8 *>(&val)) {}

ruby_typer::core::LiteralType::LiteralType(core::SymbolRef klass, core::NameRef val)
    : ProxyType(make_shared<ClassType>(klass)), value(val._id) {
    Error::check(klass == core::GlobalState::defn_String() || klass == core::GlobalState::defn_Symbol());
}

ruby_typer::core::LiteralType::LiteralType(bool val)
    : ProxyType(
          make_shared<ClassType>(val ? core::GlobalState::defn_TrueClass() : core::GlobalState::defn_FalseClass())),
      value(val ? 1 : 0) {}

string LiteralType::typeName() {
    return "LiteralType";
}

string LiteralType::toString(core::Context ctx, int tabs) {
    string value;
    SymbolRef undSymbol = dynamic_cast<ClassType *>(this->underlying.get())->symbol;
    switch (undSymbol._id) {
        case GlobalState::defn_String()._id:
            value = "\"" + NameRef(this->value).toString(ctx) + "\"";
            break;
        case GlobalState::defn_Symbol()._id:
            value = ":\"" + NameRef(this->value).toString(ctx) + "\"";
            break;
        case GlobalState::defn_Integer()._id:
            value = to_string(this->value);
            break;
        case GlobalState::defn_Float()._id:
            value = to_string(*reinterpret_cast<double *>(&(this->value)));
            break;
        case GlobalState::defn_TrueClass()._id:
            value = "true";
            break;
        case GlobalState::defn_FalseClass()._id:
            value = "false";
            break;
        default:
            Error::raise("should not be reachable");
    }
    return this->underlying->toString(ctx, tabs) + "(" + value + ")";
}

ruby_typer::core::ArrayType::ArrayType(vector<shared_ptr<Type>> &elements)
    : ProxyType(make_shared<ClassType>(core::GlobalState::defn_Array())), elems(move(elements)) {}

string ArrayType::typeName() {
    return "ArrayType";
}

string HashType::typeName() {
    return "HashType";
}

string MagicType::typeName() {
    return "MagicType";
}

string AndType::typeName() {
    return "AndType";
}

AndType::AndType(shared_ptr<Type> left, shared_ptr<Type> right) : left(left), right(right) {}

OrType::OrType(shared_ptr<Type> left, shared_ptr<Type> right) : left(left), right(right) {}

string OrType::typeName() {
    return "OrType";
}

void printTabs(stringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

string ArrayType::toString(core::Context ctx, int tabs) {
    stringstream buf;
    buf << "ArrayType {" << endl;
    int i = 0;
    for (auto &el : this->elems) {
        printTabs(buf, tabs + 1);
        buf << i << " = " << el->toString(ctx, tabs + 2) << endl;
    }
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

ruby_typer::core::HashType::HashType() : ProxyType(make_shared<ClassType>(core::GlobalState::defn_Hash())) {}

ruby_typer::core::HashType::HashType(vector<shared_ptr<LiteralType>> &keys, vector<shared_ptr<Type>> &values)
    : ProxyType(make_shared<ClassType>(core::GlobalState::defn_Hash())), keys(move(keys)), values(move(values)) {}

string HashType::toString(core::Context ctx, int tabs) {
    stringstream buf;
    buf << "HashType {" << endl;
    auto valueIterator = this->values.begin();
    for (auto &el : this->keys) {
        printTabs(buf, tabs + 1);
        buf << el->toString(ctx, tabs + 2) << " => " << (*valueIterator)->toString(ctx, tabs + 2) << endl;
        ++valueIterator;
    }
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

MagicType::MagicType() : ProxyType(make_shared<ClassType>(core::GlobalState::defn_Magic())) {}

string MagicType::toString(core::Context ctx, int tabs) {
    return underlying->toString(ctx, tabs);
}

int ClassType::kind() {
    return 1;
}

int AndType::kind() {
    return 2;
}

string AndType::toString(core::Context ctx, int tabs) {
    stringstream buf;
    buf << this->left->toString(ctx, tabs + 2) << " && " << this->right->toString(ctx, tabs + 2);
    return buf.str();
}

int OrType::kind() {
    return 3;
}

string OrType::toString(core::Context ctx, int tabs) {
    stringstream buf;
    buf << this->left->toString(ctx, tabs + 2) << " | " << this->right->toString(ctx, tabs + 2);
    return buf.str();
}
