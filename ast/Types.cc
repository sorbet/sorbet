#include "Types.h"
#include "../common/common.h"
#include <algorithm> // find_if

using namespace ruby_typer;
using namespace ruby_typer::ast;
using namespace std;

// improve debugging.
template class std::shared_ptr<ruby_typer::ast::Type>;

std::shared_ptr<ruby_typer::ast::Type> lubGround(ast::Context ctx, std::shared_ptr<Type> &t1,
                                                 std::shared_ptr<Type> &t2);
std::shared_ptr<ruby_typer::ast::Type> ruby_typer::ast::Types::lub(ast::Context ctx, std::shared_ptr<Type> &t1,
                                                                   std::shared_ptr<Type> &t2) {
    if (t1.get() == t2.get()) {
        return t1;
    }

    if (ClassType *mayBeSpecial1 = dynamic_cast<ClassType *>(t1.get())) {
        if (mayBeSpecial1->symbol == ast::GlobalState::defn_dynamic()) {
            return t1;
        }
        if (mayBeSpecial1->symbol == ast::GlobalState::defn_bottom()) {
            return t2;
        }
        if (mayBeSpecial1->symbol == ast::GlobalState::defn_top()) {
            return t1;
        }
    }

    if (ClassType *mayBeSpecial2 = dynamic_cast<ClassType *>(t2.get())) {
        if (mayBeSpecial2->symbol == ast::GlobalState::defn_dynamic()) {
            return t2;
        }
        if (mayBeSpecial2->symbol == ast::GlobalState::defn_bottom()) {
            return t1;
        }
        if (mayBeSpecial2->symbol == ast::GlobalState::defn_top()) {
            return t2;
        }
    }

    if (ProxyType *p1 = dynamic_cast<ProxyType *>(t1.get())) {
        if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
            // both are proxy
            std::shared_ptr<ruby_typer::ast::Type> result;
            ruby_typer::typecase(
                p1,
                [&](ArrayType *a1) { // Warning: this implements COVARIANT arrays
                    if (ArrayType *a2 = dynamic_cast<ArrayType *>(p2)) {
                        if (a1->elems.size() == a2->elems.size()) { // lub arrays only if they have same element count
                            std::vector<std::shared_ptr<ruby_typer::ast::Type>> elemLubs;
                            int i = 0;
                            for (auto &el2 : a2->elems) {
                                elemLubs.emplace_back(lub(ctx, a1->elems[i], el2));
                                ++i;
                            }
                            result = std::make_shared<ArrayType>(elemLubs);
                        } else {
                            result = std::make_shared<ClassType>(ast::GlobalState::defn_Array());
                        }
                    } else {
                        result = lubGround(ctx, p1->underlying, p2->underlying);
                    }
                },
                [&](HashType *h1) { // Warning: this implements COVARIANT hashes
                    if (HashType *h2 = dynamic_cast<HashType *>(p2)) {
                        if (h2->keys.size() == h1->keys.size()) {
                            // have enough keys.
                            int i = 0;
                            std::vector<std::shared_ptr<ruby_typer::ast::Literal>> keys;
                            std::vector<std::shared_ptr<ruby_typer::ast::Type>> valueLubs;
                            for (auto &el2 : h2->keys) {
                                ClassType *u2 = dynamic_cast<ClassType *>(el2->underlying.get());
                                Error::check(u2 != nullptr);
                                auto fnd = std::find_if(h1->keys.begin(), h1->keys.end(), [&](auto &candidate) -> bool {
                                    ClassType *u1 = dynamic_cast<ClassType *>(candidate->underlying.get());
                                    return candidate->value == el2->value && u1 == u2; // from lambda
                                });
                                if (fnd != h1->keys.end()) {
                                    keys.emplace_back(el2);
                                    valueLubs.emplace_back(lub(ctx, h1->values[fnd - h1->keys.begin()], h2->values[i]));
                                } else {
                                    result = std::make_shared<ClassType>(ast::GlobalState::defn_Hash());
                                    return;
                                }
                                result = std::make_shared<HashType>(keys, valueLubs);
                                ++i;
                            }
                        } else {
                            result = std::make_shared<ClassType>(ast::GlobalState::defn_Hash());
                        }
                    } else {
                        result = lubGround(ctx, p1->underlying, p2->underlying);
                    }
                },
                [&](Literal *l1) {
                    if (Literal *l2 = dynamic_cast<Literal *>(p2)) {
                        ClassType *u1 = dynamic_cast<ClassType *>(l1->underlying.get());
                        ClassType *u2 = dynamic_cast<ClassType *>(l2->underlying.get());
                        Error::check(u1 != nullptr && u2 != nullptr);
                        if (u1->symbol == u2->symbol) {
                            if (l1->value == l2->value) {
                                result = t1;
                            } else {
                                result = l1->underlying;
                            }
                        } else {
                            result = lubGround(ctx, l1->underlying, l2->underlying);
                        }
                    } else {
                        result = lubGround(ctx, p1->underlying, p2->underlying);
                    }

                });
            Error::check(result.get() != nullptr);
            return result;
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

std::shared_ptr<ruby_typer::ast::Type> distributeOr(ast::Context ctx, OrType *t1, std::shared_ptr<Type> t2) {
    std::shared_ptr<ruby_typer::ast::Type> n1 = Types::lub(ctx, t1->left, t2);
    std::shared_ptr<ruby_typer::ast::Type> n2 = Types::lub(ctx, t1->right, t2);
    if (Types::isSubType(ctx, n1, n2)) {
        return n2;
    } else if (Types::isSubType(ctx, n2, n1)) {
        return n1;
    }
    return std::make_shared<OrType>(n1, n2);
}

std::shared_ptr<ruby_typer::ast::Type> lubGround(ast::Context ctx, std::shared_ptr<Type> &t1,
                                                 std::shared_ptr<Type> &t2) {
    auto *g1 = dynamic_cast<GroundType *>(t1.get());
    auto *g2 = dynamic_cast<GroundType *>(t2.get());
    ruby_typer::Error::check(g1 != nullptr);
    ruby_typer::Error::check(g2 != nullptr);

    if (g1->kind() > g2->kind()) // force the relation to be symmentric and half the implementation
        return lubGround(ctx, t2, t1);
    /** this implementation makes a bet that types are small and very likely to be collapsable.
     * The more complex types we have, the more likely this bet is to be wrong.
     */
    if (t1.get() == t2.get()) {
        return t1;
    }

    // Prereq: t1.kind <= t2.kind
    // pairs to cover: 1  (Class, Class)
    //                 2  (Class, And)
    //                 3  (Class, Or)
    //                 4  (And, And)
    //                 5  (And, Or)
    //                 6  (Or, Or)

    std::shared_ptr<ruby_typer::ast::Type> result;

    if (auto *o2 = dynamic_cast<OrType *>(t2.get())) { // 3, 5, 6
        return distributeOr(ctx, o2, t1);
    } else if (dynamic_cast<OrType *>(t1.get()) != nullptr) {
        Error::raise("should not happen");
    } else if (auto *a2 = dynamic_cast<AndType *>(t2.get())) { // 2, 4
        bool collapseInLeft = Types::isSubType(ctx, t1, a2->left);
        bool collapseInRight = Types::isSubType(ctx, t1, a2->right);
        if (collapseInLeft) {
            if (collapseInRight) {
                return t2;
            }
            return std::make_shared<AndType>(t1, a2->right);
        } else if (collapseInRight) {
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
        return t2;
    } else if (sym2.info(ctx).derivesFrom(ctx, sym1)) {
        return t1;
    } else {
        return std::make_shared<OrType>(t1, t2);
    }
}

std::shared_ptr<ruby_typer::ast::Type> ruby_typer::ast::Types::glb(ast::Context ctx, std::shared_ptr<Type> &t1,
                                                                   std::shared_ptr<Type> &t2) {
    Error::notImplemented();
}

bool isSubTypeGround(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2) {

    auto *g1 = dynamic_cast<GroundType *>(t1.get());
    auto *g2 = dynamic_cast<GroundType *>(t2.get());

    ruby_typer::Error::check(g1 != nullptr);
    ruby_typer::Error::check(g2 != nullptr);
    if (t1.get() == t2.get()) {
        return true;
    }

    // Prereq: t1.kind <= t2.kind
    // pairs to cover: 1  (Class, Class)
    //                 2  (Class, And)
    //                 3  (Class, Or)
    //                 4  (And, Class)
    //                 5  (And, And)
    //                 6  (And, Or)
    //                 7 (Or, Class)
    //                 8 (Or, And)
    //                 9 (Or, Or)

    // Note: order of cases here matters!
    if (auto *a1 = dynamic_cast<OrType *>(t1.get())) { // 7, 8, 9
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, a1->left, t2) && Types::isSubType(ctx, a1->right, t2);
    }

    if (auto *a2 = dynamic_cast<AndType *>(t2.get())) { // 2, 5
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, t1, a2->left) && Types::isSubType(ctx, t1, a2->right);
    }

    if (auto *a2 = dynamic_cast<OrType *>(t2.get())) { // 3, 6
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, t1, a2->left) || Types::isSubType(ctx, t1, a2->right);
    }

    if (auto *a1 = dynamic_cast<AndType *>(t1.get())) { // 4
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, a1->left, t2) || Types::isSubType(ctx, a1->right, t2);
    }

    if (auto *c1 = dynamic_cast<ClassType *>(t1.get())) { // 1
        if (auto *c2 = dynamic_cast<ClassType *>(t2.get())) {
            return c1->symbol == c2->symbol || c1->symbol.info(ctx).derivesFrom(ctx, c2->symbol);
        }
    }
    Error::raise("should never ber reachable");
}

bool ruby_typer::ast::Types::equiv(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2) {
    return isSubType(ctx, t1, t2) && isSubType(ctx, t2, t1);
}

bool ruby_typer::ast::Types::isSubType(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2) {
    if (t1.get() == t2.get()) {
        return true;
    }
    if (ClassType *mayBeSpecial1 = dynamic_cast<ClassType *>(t1.get())) {
        if (mayBeSpecial1->symbol == ast::GlobalState::defn_dynamic()) {
            return true;
        }
        if (mayBeSpecial1->symbol == ast::GlobalState::defn_bottom()) {
            return true;
        }
        if (mayBeSpecial1->symbol == ast::GlobalState::defn_top()) {
            if (ClassType *mayBeSpecial2 = dynamic_cast<ClassType *>(t2.get())) {
                return mayBeSpecial2->symbol == ast::GlobalState::defn_top();
            } else {
                return false;
            }
        }
    }

    if (ClassType *mayBeSpecial2 = dynamic_cast<ClassType *>(t2.get())) {
        if (mayBeSpecial2->symbol == ast::GlobalState::defn_dynamic()) {
            return true;
        }
        if (mayBeSpecial2->symbol == ast::GlobalState::defn_bottom()) {
            return false; // (bot, bot) is handled above.
        }
        if (mayBeSpecial2->symbol == ast::GlobalState::defn_top()) {
            return true;
        }
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

std::shared_ptr<Type> Types::top() {
    return make_shared<ClassType>(ast::GlobalState::defn_top());
}

std::shared_ptr<Type> Types::bottom() {
    return make_shared<ClassType>(ast::GlobalState::defn_bottom());
}

std::shared_ptr<Type> Types::nil() {
    return make_shared<ClassType>(ast::GlobalState::defn_NilClass());
}

std::shared_ptr<Type> Types::dynamic() {
    return make_shared<ClassType>(ast::GlobalState::defn_dynamic());
}

ruby_typer::ast::ClassType::ClassType(ruby_typer::ast::SymbolRef symbol) : symbol(symbol) {}

std::string ruby_typer::ast::ClassType::toString(ruby_typer::ast::Context ctx, int tabs) {
    return this->symbol.info(ctx).name.toString(ctx);
}

std::string ruby_typer::ast::ClassType::typeName() {
    return "ClassType";
}

ruby_typer::ast::ProxyType::ProxyType(std::shared_ptr<ruby_typer::ast::Type> underlying)
    : underlying(std::move(underlying)) {}

std::shared_ptr<Type> ProxyType::dispatchCall(ast::Context ctx, ast::NameRef name, ast::Loc callLoc,
                                              std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType) {
    return underlying->dispatchCall(ctx, name, callLoc, args, fullType);
}

std::shared_ptr<Type> ProxyType::getCallArgumentType(ast::Context ctx, ast::NameRef name, int i) {
    return underlying->getCallArgumentType(ctx, name, i);
}

std::shared_ptr<Type> OrType::dispatchCall(ast::Context ctx, ast::NameRef name, ast::Loc callLoc,
                                           std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType) {
    return Types::lub(ctx, left->dispatchCall(ctx, name, callLoc, args, fullType),
                      right->dispatchCall(ctx, name, callLoc, args, fullType));
}

std::shared_ptr<Type> OrType::getCallArgumentType(ast::Context ctx, ast::NameRef name, int i) {
    return left->getCallArgumentType(ctx, name, i); // TODO: should glb with right
}

std::shared_ptr<Type> AndType::dispatchCall(ast::Context ctx, ast::NameRef name, ast::Loc callLoc,
                                            std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType) {

    Error::notImplemented();
}

std::shared_ptr<Type> AndType::getCallArgumentType(ast::Context ctx, ast::NameRef name, int i) {
    return Types::lub(ctx, left->getCallArgumentType(ctx, name, i), right->getCallArgumentType(ctx, name, i));
}

std::shared_ptr<Type> ClassType::dispatchCall(ast::Context ctx, ast::NameRef fun, ast::Loc callLoc,
                                              std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType) {
    if (isDynamic()) {
        return Types::dynamic();
    }
    ast::SymbolRef method = this->symbol.info(ctx).findMember(fun);

    if (!method.exists()) {
        ctx.state.errors.error(
            ast::Loc::none(0), ast::ErrorClass::UnknownMethod, "Method not found, {} is not a member of {}",
            fun.toString(ctx),
            this->toString(ctx)); // TODO: should use position and print the source tree, not the cfg one.
        return Types::dynamic();
    }
    ast::Symbol &info = method.info(ctx);

    if (info.arguments().size() != args.size()) { // todo: this should become actual argument matching
        ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::UnknownMethod,
                               "Wrong number of arguments for method {}.\n Expected: {}, found: {}",
                               info.arguments().size(), args.size(),
                               args.size()); // TODO: should use position and print the source tree, not the cfg one.
        return Types::dynamic();
    }
    int i = 0;
    for (TypeAndOrigins &argTpe : args) {
        shared_ptr<Type> expectedType = info.arguments()[i].info(ctx).resultType;
        if (!expectedType) {
            expectedType = Types::dynamic();
        }

        if (!Types::isSubType(ctx, argTpe.type, expectedType)) {
            ctx.state.errors.error(ast::Reporter::ComplexError(
                ast::ErrorClass::MethodArgumentMismatch,
                "Argument  №" + std::to_string(i + 1) + " does not match expected type.",
                {ast::Reporter::ErrorSection(
                     "Expected " + expectedType->toString(ctx),
                     {
                         ast::Reporter::ErrorLine::from(
                             info.arguments()[i].info(ctx).definitionLoc,
                             "Method {} has specified type of argument {} as {}", info.name.toString(ctx),
                             info.arguments()[i].info(ctx).name.toString(ctx), expectedType->toString(ctx)),
                     }),
                 ast::Reporter::ErrorSection("Got " + argTpe.type->toString(ctx) + " originating from:",
                                             argTpe.origins2Explanations(ctx))}));
        }

        i++;
    }
    shared_ptr<Type> resultType = method.info(ctx).resultType;
    if (!resultType) {
        resultType = Types::dynamic();
    }
    return resultType;
}

std::shared_ptr<Type> ClassType::getCallArgumentType(ast::Context ctx, ast::NameRef name, int i) {
    if (isDynamic()) {
        return Types::dynamic();
    }
    ast::SymbolRef method = this->symbol.info(ctx).findMember(name);

    if (method.exists()) {
        ast::Symbol &info = method.info(ctx);

        if (info.arguments().size() > i) { // todo: this should become actual argument matching
            shared_ptr<Type> resultType = info.arguments()[i].info(ctx).resultType;
            if (!resultType) {
                resultType = Types::dynamic();
            }
            return resultType;
        } else {
            return Types::dynamic();
        }
    } else {
        return Types::dynamic();
    }
}

// TODO: somehow reuse existing references instead of allocating new ones.
ruby_typer::ast::Literal::Literal(int val)
    : ProxyType(std::make_shared<ClassType>(ast::GlobalState::defn_Integer())), value(val) {}

ruby_typer::ast::Literal::Literal(float val)
    : ProxyType(std::make_shared<ClassType>(ast::GlobalState::defn_Float())), value(*reinterpret_cast<int *>(&val)) {}

ruby_typer::ast::Literal::Literal(ast::NameRef val)
    : ProxyType(std::make_shared<ClassType>(ast::GlobalState::defn_String())), value(val._id) {}

ruby_typer::ast::Literal::Literal(bool val)
    : ProxyType(
          std::make_shared<ClassType>(val ? ast::GlobalState::defn_TrueClass() : ast::GlobalState::defn_FalseClass())),
      value(val ? 1 : 0) {}

std::string Literal::typeName() {
    return "Literal";
}

std::string Literal::toString(ast::Context ctx, int tabs) {
    string value;
    SymbolRef undSymbol = dynamic_cast<ClassType *>(this->underlying.get())->symbol;
    switch (undSymbol._id) {
        case GlobalState::defn_String()._id:
            value = "\"" + NameRef(this->value).toString(ctx) + "\"";
            break;
        case GlobalState::defn_Integer()._id:
            value = std::to_string(this->value);
            break;
        case GlobalState::defn_Float()._id:
            value = std::to_string(*reinterpret_cast<float *>(&(this->value)));
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

ruby_typer::ast::ArrayType::ArrayType(std::vector<std::shared_ptr<Type>> &elements)
    : ProxyType(std::make_shared<ClassType>(ast::GlobalState::defn_Array())), elems(std::move(elements)) {}

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

ruby_typer::ast::HashType::HashType(std::vector<std::shared_ptr<Literal>> &keys,
                                    std::vector<std::shared_ptr<Type>> &values)
    : ProxyType(std::make_shared<ClassType>(ast::GlobalState::defn_Hash())), keys(std::move(keys)),
      values(std::move(values)) {}

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
    buf << this->left->toString(ctx, tabs + 2) << " && " << this->right->toString(ctx, tabs + 2);
    return buf.str();
}

int OrType::kind() {
    return 3;
}

std::string OrType::toString(ast::Context ctx, int tabs) {
    stringstream buf;
    buf << this->left->toString(ctx, tabs + 2) << " | " << this->right->toString(ctx, tabs + 2);
    return buf.str();
}

bool Type::isDynamic() {
    auto *t = dynamic_cast<ClassType *>(this);
    return t != nullptr && t->symbol == ast::GlobalState::defn_dynamic();
}
