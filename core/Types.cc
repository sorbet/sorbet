#include "Types.h"
#include "../common/common.h"
#include <algorithm> // find_if

using namespace ruby_typer;
using namespace ruby_typer::core;
using namespace std;

// improve debugging.
template class std::shared_ptr<ruby_typer::core::Type>;

shared_ptr<ruby_typer::core::Type> lubGround(core::Context ctx, shared_ptr<Type> &t1, shared_ptr<Type> &t2);
shared_ptr<ruby_typer::core::Type> ruby_typer::core::Types::lub(core::Context ctx, shared_ptr<Type> &t1,
                                                                shared_ptr<Type> &t2) {
    if (t1.get() == t2.get()) {
        return t1;
    }

    if (ClassType *mayBeSpecial1 = dynamic_cast<ClassType *>(t1.get())) {
        if (mayBeSpecial1->symbol == core::GlobalState::defn_untyped()) {
            return t1;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_bottom()) {
            return t2;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_top()) {
            return t1;
        }
    }

    if (ClassType *mayBeSpecial2 = dynamic_cast<ClassType *>(t2.get())) {
        if (mayBeSpecial2->symbol == core::GlobalState::defn_untyped()) {
            return t2;
        }
        if (mayBeSpecial2->symbol == core::GlobalState::defn_bottom()) {
            return t1;
        }
        if (mayBeSpecial2->symbol == core::GlobalState::defn_top()) {
            return t2;
        }
    }

    if (ProxyType *p1 = dynamic_cast<ProxyType *>(t1.get())) {
        if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
            // both are proxy
            shared_ptr<ruby_typer::core::Type> result;
            ruby_typer::typecase(
                p1,
                [&](ArrayType *a1) { // Warning: this implements COVARIANT arrays
                    if (ArrayType *a2 = dynamic_cast<ArrayType *>(p2)) {
                        if (a1->elems.size() == a2->elems.size()) { // lub arrays only if they have same element count
                            vector<shared_ptr<ruby_typer::core::Type>> elemLubs;
                            int i = 0;
                            for (auto &el2 : a2->elems) {
                                elemLubs.emplace_back(lub(ctx, a1->elems[i], el2));
                                ++i;
                            }
                            result = make_shared<ArrayType>(elemLubs);
                        } else {
                            result = make_shared<ClassType>(core::GlobalState::defn_Array());
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
                            vector<shared_ptr<ruby_typer::core::Literal>> keys;
                            vector<shared_ptr<ruby_typer::core::Type>> valueLubs;
                            for (auto &el2 : h2->keys) {
                                ClassType *u2 = dynamic_cast<ClassType *>(el2->underlying.get());
                                Error::check(u2 != nullptr);
                                auto fnd = find_if(h1->keys.begin(), h1->keys.end(), [&](auto &candidate) -> bool {
                                    ClassType *u1 = dynamic_cast<ClassType *>(candidate->underlying.get());
                                    return candidate->value == el2->value && u1 == u2; // from lambda
                                });
                                if (fnd != h1->keys.end()) {
                                    keys.emplace_back(el2);
                                    valueLubs.emplace_back(lub(ctx, h1->values[fnd - h1->keys.begin()], h2->values[i]));
                                } else {
                                    result = make_shared<ClassType>(core::GlobalState::defn_Hash());
                                    return;
                                }
                                result = make_shared<HashType>(keys, valueLubs);
                                ++i;
                            }
                        } else {
                            result = make_shared<ClassType>(core::GlobalState::defn_Hash());
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
            shared_ptr<Type> &und = p1->underlying;
            return lubGround(ctx, und, t2);
        }
    } else if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
        // only 2nd is proxy
        shared_ptr<Type> &und = p2->underlying;
        return lubGround(ctx, t1, und);
    } else {
        // none is proxy
        return lubGround(ctx, t1, t2);
    }
}

shared_ptr<ruby_typer::core::Type> lubDistributeOr(core::Context ctx, OrType *t1, shared_ptr<Type> t2) {
    shared_ptr<ruby_typer::core::Type> n1 = Types::lub(ctx, t1->left, t2);
    shared_ptr<ruby_typer::core::Type> n2 = Types::lub(ctx, t1->right, t2);
    if (Types::isSubType(ctx, n1, n2)) {
        return n2;
    } else if (Types::isSubType(ctx, n2, n1)) {
        return n1;
    }
    return make_shared<OrType>(n1, n2);
}

shared_ptr<ruby_typer::core::Type> lubGround(core::Context ctx, shared_ptr<Type> &t1, shared_ptr<Type> &t2) {
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

    shared_ptr<ruby_typer::core::Type> result;

    if (auto *o2 = dynamic_cast<OrType *>(t2.get())) { // 3, 5, 6
        return lubDistributeOr(ctx, o2, t1);
    } else if (dynamic_cast<OrType *>(t1.get()) != nullptr) {
        Error::raise("should not happen");
    } else if (auto *a2 = dynamic_cast<AndType *>(t2.get())) { // 2, 4
        bool collapseInLeft = Types::isSubType(ctx, t1, a2->left);
        bool collapseInRight = Types::isSubType(ctx, t1, a2->right);
        if (collapseInLeft) {
            if (collapseInRight) {
                return t2;
            }
            return make_shared<AndType>(t1, a2->right);
        } else if (collapseInRight) {
            return make_shared<AndType>(t1, a2->left);
        } else {
            return make_shared<AndType>(t1, t2);
        }
    }
    // 1 :-)
    ClassType *c1 = dynamic_cast<ClassType *>(t1.get());
    ClassType *c2 = dynamic_cast<ClassType *>(t2.get());
    Error::check(c1 != nullptr && c2 != nullptr);

    core::SymbolRef sym1 = c1->symbol;
    core::SymbolRef sym2 = c2->symbol;
    if (sym1 == sym2 || sym1.info(ctx).derivesFrom(ctx, sym2)) {
        return t2;
    } else if (sym2.info(ctx).derivesFrom(ctx, sym1)) {
        return t1;
    } else {
        return make_shared<OrType>(t1, t2);
    }
}

shared_ptr<ruby_typer::core::Type> glbDistributeAnd(core::Context ctx, AndType *t1, shared_ptr<Type> t2) {
    shared_ptr<ruby_typer::core::Type> n1 = Types::glb(ctx, t1->left, t2);
    shared_ptr<ruby_typer::core::Type> n2 = Types::glb(ctx, t1->right, t2);
    if (Types::isSubType(ctx, n1, n2)) {
        return n2;
    } else if (Types::isSubType(ctx, n2, n1)) {
        return n1;
    }
    return make_shared<AndType>(n1, n2);
}

shared_ptr<ruby_typer::core::Type> glbGround(core::Context ctx, shared_ptr<Type> &t1, shared_ptr<Type> &t2) {
    auto *g1 = dynamic_cast<GroundType *>(t1.get());
    auto *g2 = dynamic_cast<GroundType *>(t2.get());
    ruby_typer::Error::check(g1 != nullptr);
    ruby_typer::Error::check(g2 != nullptr);

    if (g1->kind() > g2->kind()) // force the relation to be symmentric and half the implementation
        return glbGround(ctx, t2, t1);
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

    shared_ptr<ruby_typer::core::Type> result;

    if (auto *a1 = dynamic_cast<AndType *>(t1.get())) { // 4, 5
        return glbDistributeAnd(ctx, a1, t2);
    } else if (auto *a2 = dynamic_cast<AndType *>(t2.get())) { // 2
        return glbDistributeAnd(ctx, a2, t1);
    } else if (auto *o2 = dynamic_cast<OrType *>(t2.get())) { // 3, 6
        bool collapseInLeft = Types::isSubType(ctx, t1, o2->left);
        bool collapseInRight = Types::isSubType(ctx, t1, o2->right);
        if (collapseInLeft || collapseInRight) {
            return t2;
        } else if (auto *o1 = dynamic_cast<OrType *>(t1.get())) { // 6
            bool collapseInLeft1 = Types::isSubType(ctx, t2, o1->left);
            bool collapseInRight1 = Types::isSubType(ctx, t1, o1->right);
            if (collapseInLeft1 || collapseInRight1) {
                return t1;
            }
        } else {
            return make_shared<AndType>(t1, t2);
        }
    }
    // 1 :-)
    ClassType *c1 = dynamic_cast<ClassType *>(t1.get());
    ClassType *c2 = dynamic_cast<ClassType *>(t2.get());
    Error::check(c1 != nullptr && c2 != nullptr);

    core::SymbolRef sym1 = c1->symbol;
    core::SymbolRef sym2 = c2->symbol;
    if (sym1 == sym2 || sym1.info(ctx).derivesFrom(ctx, sym2)) {
        return t1;
    } else if (sym2.info(ctx).derivesFrom(ctx, sym1)) {
        return t2;
    } else {
        return make_shared<AndType>(t1, t2);
    }
}

shared_ptr<ruby_typer::core::Type> ruby_typer::core::Types::glb(core::Context ctx, shared_ptr<Type> &t1,
                                                                shared_ptr<Type> &t2) {
    if (t1.get() == t2.get()) {
        return t1;
    }

    if (ClassType *mayBeSpecial1 = dynamic_cast<ClassType *>(t1.get())) {
        if (mayBeSpecial1->symbol == core::GlobalState::defn_dynamic()) {
            return t1;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_bottom()) {
            return t1;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_top()) {
            return t2;
        }
    }

    if (ClassType *mayBeSpecial2 = dynamic_cast<ClassType *>(t2.get())) {
        if (mayBeSpecial2->symbol == core::GlobalState::defn_dynamic()) {
            return t2;
        }
        if (mayBeSpecial2->symbol == core::GlobalState::defn_bottom()) {
            return t2;
        }
        if (mayBeSpecial2->symbol == core::GlobalState::defn_top()) {
            return t1;
        }
    }

    if (ProxyType *p1 = dynamic_cast<ProxyType *>(t1.get())) {
        if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
            Error::notImplemented();
            // Users can't write it yet. Not clear what this means.
            // Should we return bottom early?
            // what is an intersection between [1] and [:foo]?
        } else {
            // only 1st is proxy
            return t1;
        }
    } else if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
        return t2;
    } else {
        // none is proxy
        return glbGround(ctx, t1, t2);
    }
}

bool isSubTypeGround(core::Context ctx, shared_ptr<Type> &t1, shared_ptr<Type> &t2) {
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

bool ruby_typer::core::Types::equiv(core::Context ctx, shared_ptr<Type> &t1, shared_ptr<Type> &t2) {
    return isSubType(ctx, t1, t2) && isSubType(ctx, t2, t1);
}

bool ruby_typer::core::Types::isSubType(core::Context ctx, shared_ptr<Type> &t1, shared_ptr<Type> &t2) {
    if (t1.get() == t2.get()) {
        return true;
    }
    if (ClassType *mayBeSpecial1 = dynamic_cast<ClassType *>(t1.get())) {
        if (mayBeSpecial1->symbol == core::GlobalState::defn_untyped()) {
            return true;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_bottom()) {
            return true;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_top()) {
            if (ClassType *mayBeSpecial2 = dynamic_cast<ClassType *>(t2.get())) {
                return mayBeSpecial2->symbol == core::GlobalState::defn_top();
            } else {
                return false;
            }
        }
    }

    if (ClassType *mayBeSpecial2 = dynamic_cast<ClassType *>(t2.get())) {
        if (mayBeSpecial2->symbol == core::GlobalState::defn_untyped()) {
            return true;
        }
        if (mayBeSpecial2->symbol == core::GlobalState::defn_bottom()) {
            return false; // (bot, bot) is handled above.
        }
        if (mayBeSpecial2->symbol == core::GlobalState::defn_top()) {
            return true;
        }
    }

    if (ProxyType *p1 = dynamic_cast<ProxyType *>(t1.get())) {
        if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
            bool result;
            // TODO: simply compare as memory regions
            ruby_typer::typecase(p1,
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
                                         auto fnd =
                                             find_if(h1->keys.begin(), h1->keys.end(), [&](auto &candidate) -> bool {
                                                 ClassType *u1 = dynamic_cast<ClassType *>(candidate->underlying.get());
                                                 return candidate->value == el2->value && u1 == u2; // from lambda
                                             });
                                         result = fnd != h1->keys.end() &&
                                                  isSubType(ctx, h1->values[fnd - h1->keys.begin()], h2->values[i]);
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
            shared_ptr<Type> &und = p1->underlying;
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

string ruby_typer::core::ClassType::toString(ruby_typer::core::Context ctx, int tabs) {
    return this->symbol.info(ctx).name.toString(ctx);
}

string ruby_typer::core::ClassType::typeName() {
    return "ClassType";
}

ruby_typer::core::ProxyType::ProxyType(shared_ptr<ruby_typer::core::Type> underlying) : underlying(move(underlying)) {}

shared_ptr<Type> ProxyType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> fullType) {
    return underlying->dispatchCall(ctx, name, callLoc, args, fullType);
}

shared_ptr<Type> ProxyType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return underlying->getCallArgumentType(ctx, name, i);
}

shared_ptr<Type> OrType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                      vector<TypeAndOrigins> &args, shared_ptr<Type> fullType) {
    return Types::lub(ctx, left->dispatchCall(ctx, name, callLoc, args, fullType),
                      right->dispatchCall(ctx, name, callLoc, args, fullType));
}

shared_ptr<Type> OrType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return left->getCallArgumentType(ctx, name, i); // TODO: should glb with right
}

shared_ptr<Type> AndType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                       vector<TypeAndOrigins> &args, shared_ptr<Type> fullType) {
    Error::notImplemented();
}

shared_ptr<Type> AndType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return Types::lub(ctx, left->getCallArgumentType(ctx, name, i), right->getCallArgumentType(ctx, name, i));
}

shared_ptr<Type> ClassType::dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> fullType) {
    if (isDynamic()) {
        return Types::dynamic();
    }
    core::SymbolRef method = this->symbol.info(ctx).findMember(fun);

    if (!method.exists()) {
        string maybeComponent;
        if (fullType.get() != this) {
            maybeComponent = " component of " + fullType->toString(ctx);
        }
        ctx.state.errors.error(callLoc, core::ErrorClass::UnknownMethod,
                               "Method {} does not exist on {}" + maybeComponent, fun.name(ctx).toString(ctx),
                               this->toString(ctx));
        return Types::dynamic();
    }
    core::Symbol &info = method.info(ctx);

    if (info.arguments().size() != args.size()) { // todo: this should become actual argument matching
        ctx.state.errors.error(callLoc, core::ErrorClass::MethodArgumentCountMismatch,
                               "Wrong number of arguments for method {}.\n Expected: {}, found: {}", fun.toString(ctx),
                               info.arguments().size(),
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
            ctx.state.errors.error(core::Reporter::ComplexError(
                callLoc, core::ErrorClass::MethodArgumentMismatch,
                "Argument number " + to_string(i + 1) + " does not match expected type.",
                {core::Reporter::ErrorSection(
                     "Expected " + expectedType->toString(ctx),
                     {
                         core::Reporter::ErrorLine::from(
                             info.arguments()[i].info(ctx).definitionLoc,
                             "Method {} has specified type of argument {} as {}", info.name.toString(ctx),
                             info.arguments()[i].info(ctx).name.toString(ctx), expectedType->toString(ctx)),
                     }),
                 core::Reporter::ErrorSection("Got " + argTpe.type->toString(ctx) + " originating from:",
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

shared_ptr<Type> ClassType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    if (isDynamic()) {
        return Types::dynamic();
    }
    core::SymbolRef method = this->symbol.info(ctx).findMember(name);

    if (method.exists()) {
        core::Symbol &info = method.info(ctx);

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
ruby_typer::core::Literal::Literal(int64_t val)
    : ProxyType(make_shared<ClassType>(core::GlobalState::defn_Integer())), value(val) {}

ruby_typer::core::Literal::Literal(float val)
    : ProxyType(make_shared<ClassType>(core::GlobalState::defn_Float())), value(*reinterpret_cast<u4 *>(&val)) {}

ruby_typer::core::Literal::Literal(core::NameRef val)
    : ProxyType(make_shared<ClassType>(core::GlobalState::defn_String())), value(val._id) {}

ruby_typer::core::Literal::Literal(bool val)
    : ProxyType(
          make_shared<ClassType>(val ? core::GlobalState::defn_TrueClass() : core::GlobalState::defn_FalseClass())),
      value(val ? 1 : 0) {}

string Literal::typeName() {
    return "Literal";
}

string Literal::toString(core::Context ctx, int tabs) {
    string value;
    SymbolRef undSymbol = dynamic_cast<ClassType *>(this->underlying.get())->symbol;
    switch (undSymbol._id) {
        case GlobalState::defn_String()._id:
            value = "\"" + NameRef(this->value).toString(ctx) + "\"";
            break;
        case GlobalState::defn_Integer()._id:
            value = to_string(this->value);
            break;
        case GlobalState::defn_Float()._id:
            value = to_string(*reinterpret_cast<float *>(&(this->value)));
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

ruby_typer::core::HashType::HashType(vector<shared_ptr<Literal>> &keys, vector<shared_ptr<Type>> &values)
    : ProxyType(make_shared<ClassType>(core::GlobalState::defn_Hash())), keys(move(keys)), values(move(values)) {}

string HashType::toString(core::Context ctx, int tabs) {
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

bool Type::isDynamic() {
    auto *t = dynamic_cast<ClassType *>(this);
    return t != nullptr && t->symbol == core::GlobalState::defn_untyped();
}
