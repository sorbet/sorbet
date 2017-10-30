#include "Types.h"
#include "../common/common.h"
#include <algorithm> // find_if

using namespace ruby_typer;
using namespace ruby_typer::infer;
using namespace std;

// improve debugging.
template class std::shared_ptr<ruby_typer::infer::Type>;

std::shared_ptr<ruby_typer::infer::Type> lubGround(ast::Context ctx, std::shared_ptr<Type> &t1,
                                                   std::shared_ptr<Type> &t2);
std::shared_ptr<ruby_typer::infer::Type> ruby_typer::infer::Types::lub(ast::Context ctx, std::shared_ptr<Type> &t1,
                                                                       std::shared_ptr<Type> &t2) {
    if (t1.get() == t2.get()) {
        return t1;
    }
    if (ProxyType *p1 = dynamic_cast<ProxyType *>(t1.get())) {
        if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
            Error::notImplemented();
            // both are proxy
        } else {
            // only 1st is proxy
            std::shared_ptr<Type> &und = p1->underlying;
            return lubGround(ctx, und, t2);
        }
    } else if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
        // only 2nd is proxy
        std::shared_ptr<Type> &und = p2->underlying;
        return lubGround(ctx, t1, und);
    } else {
        // none is proxy
        return lubGround(ctx, t1, t2);
    }
}

std::shared_ptr<ruby_typer::infer::Type> distributeOr(ast::Context ctx, OrType *t1, std::shared_ptr<Type> t2) {
    std::shared_ptr<ruby_typer::infer::Type> n1 = Types::lub(ctx, t2, t1->left);
    std::shared_ptr<ruby_typer::infer::Type> n2 = Types::lub(ctx, t2, t1->right);
    if (Types::isSubType(ctx, n1, n2)) {
        return n2;
    } else if (Types::isSubType(ctx, n2, n1)) {
        return n1;
    }
    return std::make_shared<OrType>(n1, n2);
}

std::shared_ptr<ruby_typer::infer::Type> lubGround(ast::Context ctx, std::shared_ptr<Type> &t1,
                                                   std::shared_ptr<Type> &t2) {
    auto *g1 = dynamic_cast<GroundType *>(t1.get());
    auto *g2 = dynamic_cast<GroundType *>(t2.get());
    if (g1->kind() > g2->kind()) // force the relation to be symmentric and half the implementation
        return lubGround(ctx, t2, t1);
    /** this implementation makes a bet that types are small and very likely to be collapsable.
     * The more complex types we have, the more likely this bet is to be wrong.
     */
    ruby_typer::Error::check(g1 != nullptr && g2 != nullptr);
    if (t1.get() == t2.get()) {
        return t1;
    }

    // Prereq: t1.kind <= t2.kind
    // pairs to cover: 1  (Class, Class)
    //                 2  (Class, And)
    //                 3  (Class, Or)
    //                 4  (Class, Method)
    //                 5  (And, And)
    //                 6  (And, Or)
    //                 7  (And, Method)
    //                 8  (Or, Or)
    //                 9  (Or, Method)
    //                 10 (Method, Method)

    std::shared_ptr<ruby_typer::infer::Type> result;

    if (auto *o2 = dynamic_cast<OrType *>(t2.get())) { // 3, 6, 8
        return distributeOr(ctx, o2, t1);
    } else if (dynamic_cast<MethodType *>(t1.get()) != nullptr ||
               dynamic_cast<OrType *>(t2.get()) != nullptr) { // 4, 7, 9, 10
        Error::raise("should not happen");
    } else if (auto *a2 = dynamic_cast<AndType *>(t2.get())) { // 2, 5
        if (Types::isSubType(ctx, t1, a2->left)) {
            return std::make_shared<AndType>(t1, a2->right);
        } else if (Types::isSubType(ctx, t1, a2->left) || Types::isSubType(ctx, t1, a2->right)) {
            return t2;
        } else if (Types::isSubType(ctx, t1, a2->right)) {
            return std::make_shared<AndType>(t1, a2->left);
        } else {
            return std::make_shared<AndType>(t1, t2);
        }
    }
    // 1 :-)
    ClassType *c1 = dynamic_cast<ClassType *>(t1.get());
    ClassType *c2 = dynamic_cast<ClassType *>(t2.get());
    Error::check(c1 != nullptr && c2 != nullptr);

    ast::SymbolRef sym1 = c1->symbol;
    ast::SymbolRef sym2 = c2->symbol;
    if (sym1 == sym2 || sym1.info(ctx).derivesFrom(ctx, sym2)) {
        result = t1;
    } else if (sym1.info(ctx).derivesFrom(ctx, sym2)) {
        result = t2;
    } else {
        result = std::make_shared<AndType>(t1, t2);
    }
}

std::shared_ptr<ruby_typer::infer::Type> ruby_typer::infer::Types::glb(ast::Context ctx, std::shared_ptr<Type> &t1,
                                                                       std::shared_ptr<Type> &t2) {
    Error::notImplemented();
}

bool isSubTypeGround(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2) {

    auto *g1 = dynamic_cast<GroundType *>(t1.get());
    auto *g2 = dynamic_cast<GroundType *>(t2.get());

    ruby_typer::Error::check(g1 != nullptr && g2 != nullptr);
    if (t1.get() == t2.get()) {
        return true;
    }

    // Prereq: t1.kind <= t2.kind
    // pairs to cover: 1  (Class, Class)
    //                 2  (Class, And)
    //                 3  (Class, Or)
    //                 4  (Class, Method)
    //                 5  (And, Class)
    //                 6  (And, And)
    //                 7  (And, Or)
    //                 8  (And, Method)
    //                 9 (Or, Class)
    //                 10 (Or, And)
    //                 11 (Or, Or)
    //                 12 (Or, Method)
    //                 13 (Method, Class)
    //                 14 (Method, And)
    //                 15 (Method, Or)
    //                 16 (Method, Method)

    if (dynamic_cast<MethodType *>(t1.get()) || dynamic_cast<MethodType *>(t2.get())) { // 4, 8, 12, 13, 14, 15, 16
        Error::notImplemented();
    }

    if (auto *a2 = dynamic_cast<AndType *>(t2.get())) { // 2, 6, 10, 14
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, t1, a2->left) && Types::isSubType(ctx, t1, a2->right);
    }

    if (auto *a2 = dynamic_cast<OrType *>(t2.get())) { // 3, 7, 11, 15
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, t1, a2->left) || Types::isSubType(ctx, t1, a2->right);
    }

    if (auto *a1 = dynamic_cast<AndType *>(t1.get())) { // 5, 7, 8
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, a1->left, t2) || Types::isSubType(ctx, a1->right, t2);
    }

    if (auto *a1 = dynamic_cast<OrType *>(t1.get())) { // 9, 11, 12
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, a1->left, t2) && Types::isSubType(ctx, a1->right, t2);
    }

    if (auto *c1 = dynamic_cast<ClassType *>(t1.get())) { // 1
        if (auto *c2 = dynamic_cast<ClassType *>(t2.get())) {
            return c1->symbol == c2->symbol || c1->symbol.info(ctx).derivesFrom(ctx, c2->symbol);
        }
    }
    Error::raise("should never ber reachable");
}

bool ruby_typer::infer::Types::equiv(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2) {
    return isSubType(ctx, t1, t2) && isSubType(ctx, t2, t1);
}

bool ruby_typer::infer::Types::isSubType(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2) {
    if (t1.get() == t2.get()) {
        return true;
    }
    if (ProxyType *p1 = dynamic_cast<ProxyType *>(t1.get())) {
        if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
            bool result;
            // TODO: simply compare as memory regions
            ruby_typer::typecase(
                p1,
                [&](ArrayType *a1) { // Warning: this implements COVARIANT arrays
                    ArrayType *a2 = dynamic_cast<ArrayType *>(p2);
                    result = a2 != nullptr && a1->elems.size() >= a2->elems.size();
                    if (result) {
                        int i = 0;
                        for (auto &el2 : a2->elems) {
                            result = isSubType(ctx, a1->elems[i], el2);
                            if (!result) {
                                break;
                            }
                            ++i;
                        }
                    }
                },
                [&](HashType *h1) { // Warning: this implements COVARIANT hashes
                    HashType *h2 = dynamic_cast<HashType *>(p2);
                    result = h2 != nullptr && h2->keys.size() <= h1->keys.size();
                    if (!result) {
                        return;
                    }
                    // have enough keys.
                    int i = 0;
                    for (auto &el2 : h2->keys) {
                        ClassType *u2 = dynamic_cast<ClassType *>(el2->underlying.get());
                        Error::check(u2 != nullptr);
                        auto fnd = std::find_if(h1->keys.begin(), h1->keys.end(), [&](auto &candidate) -> bool {
                            ClassType *u1 = dynamic_cast<ClassType *>(candidate->underlying.get());
                            return candidate->value == el2->value && u1 == u2; // from lambda
                        });
                        result =
                            fnd != h1->keys.end() && isSubType(ctx, h1->values[fnd - h1->keys.begin()], h2->values[i]);
                        if (!result) {
                            return;
                        }
                        ++i;
                    }
                },
                [&](Literal *l1) {
                    Literal *l2 = dynamic_cast<Literal *>(p2);
                    ClassType *u1 = dynamic_cast<ClassType *>(l1->underlying.get());
                    ClassType *u2 = dynamic_cast<ClassType *>(l2->underlying.get());
                    Error::check(u1 != nullptr && u2 != nullptr);
                    result = l2 != nullptr && u1->symbol == u2->symbol && l1->value == l2->value;
                });
            return result;
            // both are proxy
        } else {
            // only 1st is proxy
            std::shared_ptr<Type> &und = p1->underlying;
            return isSubTypeGround(ctx, und, t2);
        }
    } else if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
        // non-proxies are never subtypes of proxies.
        return false;
    } else {
        // none is proxy
        return isSubTypeGround(ctx, t1, t2);
    }
}

ruby_typer::infer::ClassType::ClassType(ruby_typer::ast::SymbolRef symbol) : symbol(symbol) {}

std::string ruby_typer::infer::ClassType::toString(ruby_typer::ast::Context ctx, int tabs) {
    return this->symbol.toString(ctx);
}

std::string ruby_typer::infer::ClassType::typeName() {
    return "ClassType";
}

ruby_typer::infer::ProxyType::ProxyType(std::shared_ptr<ruby_typer::infer::Type> underlying)
    : underlying(std::move(underlying)) {}

ruby_typer::infer::MethodType::MethodType(ruby_typer::ast::SymbolRef symbol) : symbol(symbol) {}

// TODO: somehow reuse existing references instead of allocating new ones.
ruby_typer::infer::Literal::Literal(int val)
    : ProxyType(std::make_shared<ClassType>(ast::GlobalState::defn_Integer())), value(val) {}

ruby_typer::infer::Literal::Literal(float val)
    : ProxyType(std::make_shared<ClassType>(ast::GlobalState::defn_Float())), value(*reinterpret_cast<int *>(&val)) {}

ruby_typer::infer::Literal::Literal(ast::NameRef val)
    : ProxyType(std::make_shared<ClassType>(ast::GlobalState::defn_String())), value(val._id) {}

ruby_typer::infer::Literal::Literal(bool val)
    : ProxyType(
          std::make_shared<ClassType>(val ? ast::GlobalState::defn_TrueClass() : ast::GlobalState::defn_FalseClass())),
      value(val ? 1 : 0) {}

std::string Literal::typeName() {
    return "Literal";
}

std::string Literal::toString(ast::Context ctx, int tabs) {
    return "Literal[" + this->underlying->toString(ctx, tabs) + "]{" + to_string(value) + "}";
}

ruby_typer::infer::ArrayType::ArrayType(std::vector<std::shared_ptr<Type>> elements)
    : ProxyType(std::make_shared<ClassType>(ast::GlobalState::defn_Array())), elems(elements) {}

std::string ArrayType::typeName() {
    return "ArrayType";
}

std::string HashType::typeName() {
    return "HashType";
}

std::string AndType::typeName() {
    return "AndType";
}

AndType::AndType(std::shared_ptr<Type> left, std::shared_ptr<Type> right) : left(left), right(right) {}

OrType::OrType(std::shared_ptr<Type> left, std::shared_ptr<Type> right) : left(left), right(right) {}

std::string OrType::typeName() {
    return "OrType";
}

void printTabs(stringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

std::string ArrayType::toString(ast::Context ctx, int tabs) {
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

ruby_typer::infer::HashType::HashType(std::vector<std::shared_ptr<Literal>> keys,
                                      std::vector<std::shared_ptr<Type>> values)
    : ProxyType(std::make_shared<ClassType>(ast::GlobalState::defn_Hash())), keys(keys), values(values) {}

std::string HashType::toString(ast::Context ctx, int tabs) {
    stringstream buf;
    buf << "HashType {" << endl;
    auto valueIterator = this->values.begin();
    for (auto &el : this->keys) {
        printTabs(buf, tabs + 1);
        buf << (*valueIterator)->toString(ctx, tabs + 2) << " -> " << el->toString(ctx, tabs + 2) << endl;
        ++valueIterator;
    }
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

int ClassType::kind() {
    return 1;
}

int AndType::kind() {
    return 2;
}

std::string AndType::toString(ast::Context ctx, int tabs) {
    stringstream buf;
    buf << "AndType {" << endl;
    printTabs(buf, tabs + 1);
    buf << "left"
        << " = " << this->left->toString(ctx, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "right"
        << " = " << this->right->toString(ctx, tabs + 2) << endl;

    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

int OrType::kind() {
    return 3;
}

std::string OrType::toString(ast::Context ctx, int tabs) {
    stringstream buf;
    buf << "OrType {" << endl;
    printTabs(buf, tabs + 1);
    buf << "left"
        << " = " << this->left->toString(ctx, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "right"
        << " = " << this->right->toString(ctx, tabs + 2) << endl;

    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

int MethodType::kind() {
    return 4;
}

std::string MethodType::typeName() {
    return "MethodType";
}

std::string MethodType::toString(ast::Context ctx, int tabs) {
    Error::notImplemented();
}
