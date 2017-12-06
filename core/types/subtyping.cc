#include "common/common.h"
#include "core/Types.h"
#include <algorithm> // find_if

using namespace ruby_typer;
using namespace ruby_typer::core;
using namespace std;

shared_ptr<ruby_typer::core::Type> lubGround(core::Context ctx, shared_ptr<Type> &t1, shared_ptr<Type> &t2);
shared_ptr<ruby_typer::core::Type> ruby_typer::core::Types::lub(core::Context ctx, shared_ptr<Type> &t1,
                                                                shared_ptr<Type> &t2) {
    auto ret = _lub(ctx, t1, t2);
    DEBUG_ONLY(Error::check(Types::isSubType(ctx, t1, ret), ret->toString(ctx) + " is not a subtype of " +
                                                                t1->toString(ctx) + " was lubbing with " +
                                                                t2->toString(ctx)));
    DEBUG_ONLY(Error::check(Types::isSubType(ctx, t2, ret), ret->toString(ctx) + " is not a subtype of " +
                                                                t2->toString(ctx) + " was lubbing with " +
                                                                t1->toString(ctx)));
    return ret;
}

shared_ptr<ruby_typer::core::Type> ruby_typer::core::Types::_lub(core::Context ctx, shared_ptr<Type> &t1,
                                                                 shared_ptr<Type> &t2) {
    if (t1.get() == t2.get()) {
        categoryCounterInc("lub", "ref-eq");
        return t1;
    }

    if (ClassType *mayBeSpecial1 = dynamic_cast<ClassType *>(t1.get())) {
        if (mayBeSpecial1->symbol == core::GlobalState::defn_untyped()) {
            categoryCounterInc("lub", "<untyped");
            return t1;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_bottom()) {
            categoryCounterInc("lub", "<bottom");
            return t2;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_top()) {
            categoryCounterInc("lub", "<top");
            return t1;
        }
    }

    if (ClassType *mayBeSpecial2 = dynamic_cast<ClassType *>(t2.get())) {
        if (mayBeSpecial2->symbol == core::GlobalState::defn_untyped()) {
            categoryCounterInc("lub", "untyped>");
            return t2;
        }
        if (mayBeSpecial2->symbol == core::GlobalState::defn_bottom()) {
            categoryCounterInc("lub", "bottom>");
            return t1;
        }
        if (mayBeSpecial2->symbol == core::GlobalState::defn_top()) {
            categoryCounterInc("lub", "top>");
            return t2;
        }
    }

    if (ProxyType *p1 = dynamic_cast<ProxyType *>(t1.get())) {
        categoryCounterInc("lub", "<proxy");
        if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
            categoryCounterInc("lub", "proxy>");
            // both are proxy
            shared_ptr<ruby_typer::core::Type> result;
            ruby_typer::typecase(
                p1,
                [&](TupleType *a1) { // Warning: this implements COVARIANT arrays
                    if (TupleType *a2 = dynamic_cast<TupleType *>(p2)) {
                        if (a1->elems.size() == a2->elems.size()) { // lub arrays only if they have same element count
                            vector<shared_ptr<ruby_typer::core::Type>> elemLubs;
                            int i = 0;
                            for (auto &el2 : a2->elems) {
                                elemLubs.emplace_back(lub(ctx, a1->elems[i], el2));
                                ++i;
                            }
                            result = make_shared<TupleType>(elemLubs);
                        } else {
                            result = make_shared<ClassType>(core::GlobalState::defn_Array());
                        }
                    } else {
                        result = lubGround(ctx, p1->underlying, p2->underlying);
                    }
                },
                [&](ShapeType *h1) { // Warning: this implements COVARIANT hashes
                    if (ShapeType *h2 = dynamic_cast<ShapeType *>(p2)) {
                        if (h2->keys.size() == h1->keys.size()) {
                            // have enough keys.
                            int i = 0;
                            vector<shared_ptr<ruby_typer::core::LiteralType>> keys;
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
                                ++i;
                            }
                            result = make_shared<ShapeType>(keys, valueLubs);
                        } else {
                            result = make_shared<ClassType>(core::GlobalState::defn_Hash());
                        }
                    } else {
                        result = lubGround(ctx, p1->underlying, p2->underlying);
                    }
                },
                [&](LiteralType *l1) {
                    if (LiteralType *l2 = dynamic_cast<LiteralType *>(p2)) {
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
        categoryCounterInc("lub", "proxy>");
        // only 2nd is proxy
        shared_ptr<Type> &und = p2->underlying;
        return lubGround(ctx, t1, und);
    } else {
        // none is proxy
        return lubGround(ctx, t1, t2);
    }
}

shared_ptr<ruby_typer::core::Type> lubDistributeOr(core::Context ctx, shared_ptr<Type> t1, shared_ptr<Type> t2) {
    OrType *o1 = dynamic_cast<OrType *>(t1.get());
    Error::check(o1 != nullptr);
    shared_ptr<ruby_typer::core::Type> n1 = Types::lub(ctx, o1->left, t2);
    if (n1.get() == o1->left.get()) {
        categoryCounterInc("lubDistributeOr::outcome", "t1");
        return t1;
    }
    shared_ptr<ruby_typer::core::Type> n2 = Types::lub(ctx, o1->right, t2);
    if (n1.get() == t2.get()) {
        categoryCounterInc("lubDistributeOr::outcome", "n2'");
        return n2;
    }
    if (n2.get() == o1->right.get()) {
        categoryCounterInc("lubDistributeOr::outcome", "t1'");
        return t1;
    }
    if (n2.get() == t2.get()) {
        categoryCounterInc("lubDistributeOr::outcome", "n1'");
        return n1;
    }
    if (Types::isSubType(ctx, n1, n2)) {
        categoryCounterInc("lubDistributeOr::outcome", "n2''");
        return n2;
    } else if (Types::isSubType(ctx, n2, n1)) {
        categoryCounterInc("lubDistributeOr::outcome", "n1'''");
        return n1;
    }
    categoryCounterInc("lubDistributeOr::outcome", "worst");
    return make_shared<OrType>(t1, t2); // order matters for perf
}

shared_ptr<ruby_typer::core::Type> lubGround(core::Context ctx, shared_ptr<Type> &t1, shared_ptr<Type> &t2) {
    auto *g1 = dynamic_cast<GroundType *>(t1.get());
    auto *g2 = dynamic_cast<GroundType *>(t2.get());
    ruby_typer::Error::check(g1 != nullptr);
    ruby_typer::Error::check(g2 != nullptr);

    if (g1->kind() > g2->kind()) { // force the relation to be symmentric and half the implementation
        return lubGround(ctx, t2, t1);
    }
    /** this implementation makes a bet that types are small and very likely to be collapsable.
     * The more complex types we have, the more likely this bet is to be wrong.
     */
    if (t1.get() == t2.get()) {
        categoryCounterInc("lub", "ref-eq2");
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
        categoryCounterInc("lub", "or>");
        return lubDistributeOr(ctx, t2, t1);
    } else if (dynamic_cast<OrType *>(t1.get()) != nullptr) {
        categoryCounterInc("lub", "<or");
        Error::raise("should not happen");
    } else if (auto *a2 = dynamic_cast<AndType *>(t2.get())) { // 2, 4
        categoryCounterInc("lub", "and>");
        bool collapseInLeft = Types::isSubType(ctx, t1, a2->left);
        bool collapseInRight = Types::isSubType(ctx, t1, a2->right);
        if (collapseInLeft) {
            if (collapseInRight) {
                categoryCounterInc("lub::and>collapsed", "fully");
                return t2;
            }
            categoryCounterInc("lub::and>collapsed", "right");
            return make_shared<AndType>(t1, a2->right);
        } else if (collapseInRight) {
            categoryCounterInc("lub::and>collapsed", "left");
            return make_shared<AndType>(t1, a2->left);
        } else {
            categoryCounterInc("lub::and>collapsed", "none");
            return make_shared<AndType>(t1, t2);
        }
    }
    // 1 :-)
    ClassType *c1 = dynamic_cast<ClassType *>(t1.get());
    ClassType *c2 = dynamic_cast<ClassType *>(t2.get());
    categoryCounterInc("lub", "<class>");
    Error::check(c1 != nullptr && c2 != nullptr);

    core::SymbolRef sym1 = c1->symbol;
    core::SymbolRef sym2 = c2->symbol;
    if (sym1 == sym2 || sym1.info(ctx).derivesFrom(ctx, sym2)) {
        categoryCounterInc("lub::<class>::collapsed", "yes");
        return t2;
    } else if (sym2.info(ctx).derivesFrom(ctx, sym1)) {
        categoryCounterInc("lub::<class>::collapsed", "yes");
        return t1;
    } else {
        categoryCounterInc("lub::<class>::collapsed", "no");
        return make_shared<OrType>(t1, t2);
    }
}

shared_ptr<ruby_typer::core::Type> glbDistributeAnd(core::Context ctx, shared_ptr<Type> t1, shared_ptr<Type> t2) {
    AndType *a1 = dynamic_cast<AndType *>(t1.get());
    Error::check(t1 != nullptr);
    shared_ptr<ruby_typer::core::Type> n1 = Types::glb(ctx, a1->left, t2);
    if (n1.get() == a1->left.get()) {
        categoryCounterInc("glbDistributeAnd::outcome", "t1");
        return t1;
    }
    shared_ptr<ruby_typer::core::Type> n2 = Types::glb(ctx, a1->right, t2);
    if (n1.get() == t2.get()) {
        categoryCounterInc("glbDistributeAnd::outcome", "Zn2");
        return n2;
    }
    if (n2.get() == a1->right.get()) {
        categoryCounterInc("glbDistributeAnd::outcome", "Zt1");
        return t1;
    }
    if (n2.get() == t2.get()) {
        categoryCounterInc("glbDistributeAnd::outcome", "Zn1");
        return n1;
    }
    if (Types::isSubType(ctx, n1, n2)) {
        categoryCounterInc("glbDistributeAnd::outcome", "ZZn2");
        return n2;
    } else if (Types::isSubType(ctx, n2, n1)) {
        categoryCounterInc("glbDistributeAnd::outcome", "ZZZn1");
        return n1;
    }

    categoryCounterInc("glbDistributeAnd::outcome", "worst");
    return make_shared<AndType>(t1, t2);
}

shared_ptr<ruby_typer::core::Type> glbGround(core::Context ctx, shared_ptr<Type> &t1, shared_ptr<Type> &t2) {
    auto *g1 = dynamic_cast<GroundType *>(t1.get());
    auto *g2 = dynamic_cast<GroundType *>(t2.get());
    ruby_typer::Error::check(g1 != nullptr);
    ruby_typer::Error::check(g2 != nullptr);

    if (g1->kind() > g2->kind()) { // force the relation to be symmentric and half the implementation
        return glbGround(ctx, t2, t1);
    }
    /** this implementation makes a bet that types are small and very likely to be collapsable.
     * The more complex types we have, the more likely this bet is to be wrong.
     */
    if (t1.get() == t2.get()) {
        categoryCounterInc("glb", "ref-eq2");
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
        categoryCounterInc("glb", "<and");
        return glbDistributeAnd(ctx, t1, t2);
    } else if (auto *a2 = dynamic_cast<AndType *>(t2.get())) { // 2
        categoryCounterInc("glb", "and>");
        return glbDistributeAnd(ctx, t2, t1);
    } else if (auto *o2 = dynamic_cast<OrType *>(t2.get())) { // 3, 6
        bool collapseInLeft = Types::isSubType(ctx, t1, t2);
        if (collapseInLeft) {
            categoryCounterInc("glb", "Zor");
            return t1;
        }

        bool collapseInRight = Types::isSubType(ctx, t2, t1);
        if (collapseInRight) {
            categoryCounterInc("glb", "ZZor");
            return t2;
        }

        if (auto *c1 = dynamic_cast<ClassType *>(t1.get())) {
            auto lft = Types::glb(ctx, t1, o2->left);
            if (Types::isSubType(ctx, lft, o2->right)) {
                categoryCounterInc("glb", "ZZZorClass");
                return lft;
            }
            auto rght = Types::glb(ctx, t1, o2->right);
            if (Types::isSubType(ctx, rght, o2->left)) {
                categoryCounterInc("glb", "ZZZZorClass");
                return rght;
            }
        }

        if (auto *o1 = dynamic_cast<OrType *>(t1.get())) { // 6
            // try hard to collapse
            bool subt11 = Types::isSubType(ctx, o1->left, o2->left) || Types::isSubType(ctx, o1->left, o2->right);
            if (!subt11) { // left is not in right, we can drop it
                categoryCounterInc("glb", "ZorOr");
                return Types::glb(ctx, o1->right, t2);
            }
            bool subt12 = Types::isSubType(ctx, o1->right, o2->left) || Types::isSubType(ctx, o1->right, o2->right);
            if (!subt12) {
                categoryCounterInc("glb", "ZZorOr");
                return Types::glb(ctx, o1->left, t2);
            }
            bool subt21 = Types::isSubType(ctx, o2->left, o1->left) || Types::isSubType(ctx, o2->left, o1->right);
            if (!subt21) {
                categoryCounterInc("glb", "ZZZorOr");
                return Types::glb(ctx, o2->right, t1);
            }
            bool subt22 = Types::isSubType(ctx, o2->right, o1->left) || Types::isSubType(ctx, o2->right, o1->right);
            if (!subt22) {
                categoryCounterInc("glb", "ZZZZorOr");
                return Types::glb(ctx, o2->left, t1);
            }
            categoryCounterInc("glb", "ZZZZZorWorst");
            return make_shared<AndType>(t1, t2);
        } else {
            categoryCounterInc("glb::orcollapsed", "no");
            return make_shared<AndType>(t1, t2);
        }
    }
    // 1 :-)
    ClassType *c1 = dynamic_cast<ClassType *>(t1.get());
    ClassType *c2 = dynamic_cast<ClassType *>(t2.get());
    Error::check(c1 != nullptr && c2 != nullptr);
    categoryCounterInc("glb", "<class>");

    core::SymbolRef sym1 = c1->symbol;
    core::SymbolRef sym2 = c2->symbol;
    if (sym1 == sym2 || sym1.info(ctx).derivesFrom(ctx, sym2)) {
        categoryCounterInc("glb::<class>::collapsed", "yes");
        return t1;
    } else if (sym2.info(ctx).derivesFrom(ctx, sym1)) {
        categoryCounterInc("glb::<class>::collapsed", "yes");
        return t2;
    } else {
        if (sym1.info(ctx).isClass() && sym2.info(ctx).isClass()) {
            categoryCounterInc("glb.class.collapsed", "bottom");
            return Types::bottom();
        }
        categoryCounterInc("glb.class.ollapsed", "no");
        return make_shared<AndType>(t1, t2);
    }
}
shared_ptr<ruby_typer::core::Type> ruby_typer::core::Types::glb(core::Context ctx, shared_ptr<Type> &t1,
                                                                shared_ptr<Type> &t2) {
    auto ret = _glb(ctx, t1, t2);
    DEBUG_ONLY(Error::check(Types::isSubType(ctx, ret, t1), ret->toString(ctx) + " is not a supertype of " +
                                                                t1->toString(ctx) + " was glbbing with " +
                                                                t2->toString(ctx)));
    DEBUG_ONLY(Error::check(Types::isSubType(ctx, ret, t2), ret->toString(ctx) + " is not a supertype of " +
                                                                t2->toString(ctx) + " was glbbing with " +
                                                                t1->toString(ctx)));
    return ret;
}

shared_ptr<ruby_typer::core::Type> ruby_typer::core::Types::_glb(core::Context ctx, shared_ptr<Type> &t1,
                                                                 shared_ptr<Type> &t2) {
    if (t1.get() == t2.get()) {
        categoryCounterInc("glb", "ref-eq");
        return t1;
    }

    if (ClassType *mayBeSpecial1 = dynamic_cast<ClassType *>(t1.get())) {
        if (mayBeSpecial1->symbol == core::GlobalState::defn_untyped()) {
            categoryCounterInc("glb", "<untyped");
            return t1;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_bottom()) {
            categoryCounterInc("glb", "<bottom");
            return t1;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_top()) {
            categoryCounterInc("glb", "<top");
            return t2;
        }
    }

    if (ClassType *mayBeSpecial2 = dynamic_cast<ClassType *>(t2.get())) {
        if (mayBeSpecial2->symbol == core::GlobalState::defn_untyped()) {
            categoryCounterInc("glb", "untyped>");
            return t2;
        }
        if (mayBeSpecial2->symbol == core::GlobalState::defn_bottom()) {
            categoryCounterInc("glb", "bottom>");
            return t2;
        }
        if (mayBeSpecial2->symbol == core::GlobalState::defn_top()) {
            categoryCounterInc("glb", "top>");
            return t1;
        }
    }

    if (ProxyType *p1 = dynamic_cast<ProxyType *>(t1.get())) {
        if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
            if (typeid(*p1) != typeid(*p2)) {
                return Types::bottom();
            }
            shared_ptr<ruby_typer::core::Type> result;
            ruby_typer::typecase(
                p1,
                [&](TupleType *a1) { // Warning: this implements COVARIANT arrays
                    TupleType *a2 = dynamic_cast<TupleType *>(p2);
                    Error::check(a2);
                    if (a1->elems.size() == a2->elems.size()) { // lub arrays only if they have same element count
                        vector<shared_ptr<ruby_typer::core::Type>> elemGlbs;
                        int i = 0;
                        for (auto &el2 : a2->elems) {
                            auto glbe = glb(ctx, a1->elems[i], el2);
                            if (glbe->isBottom()) {
                                result = Types::bottom();
                                return;
                            }
                            elemGlbs.emplace_back(glbe);
                            ++i;
                        }
                        result = make_shared<TupleType>(elemGlbs);
                    } else {
                        result = Types::bottom();
                    }

                },
                [&](ShapeType *h1) { // Warning: this implements COVARIANT hashes
                    ShapeType *h2 = dynamic_cast<ShapeType *>(p2);
                    Error::check(h2);
                    if (h2->keys.size() == h1->keys.size()) {
                        // have enough keys.
                        int i = 0;
                        vector<shared_ptr<ruby_typer::core::LiteralType>> keys;
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
                                auto glbe = glb(ctx, h1->values[fnd - h1->keys.begin()], h2->values[i]);
                                if (glbe->isBottom()) {
                                    result = Types::bottom();
                                    return;
                                }
                                valueLubs.emplace_back(glbe);
                            } else {
                                result = Types::bottom();
                                return;
                            }
                            ++i;
                        }
                        result = make_shared<ShapeType>(keys, valueLubs);
                    } else {
                        result = Types::bottom();
                    }

                },
                [&](LiteralType *l1) {
                    LiteralType *l2 = dynamic_cast<LiteralType *>(p2);
                    Error::check(l2);
                    ClassType *u1 = dynamic_cast<ClassType *>(l1->underlying.get());
                    ClassType *u2 = dynamic_cast<ClassType *>(l2->underlying.get());
                    Error::check(u1 != nullptr && u2 != nullptr);
                    if (u1->symbol == u2->symbol) {
                        if (l1->value == l2->value) {
                            result = t1;
                        } else {
                            result = Types::bottom();
                        }
                    } else {
                        result = Types::bottom();
                    }

                });
            Error::check(result.get() != nullptr);
            return result;
        } else {
            // only 1st is proxy
            if (Types::isSubType(ctx, t1, t2)) {
                return t1;
            } else {
                return Types::bottom();
            }
        }
    } else if (ProxyType *p2 = dynamic_cast<ProxyType *>(t2.get())) {
        // only 1st is proxy
        if (Types::isSubType(ctx, t2, t1)) {
            return t2;
        } else {
            return Types::bottom();
        }
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
                                 [&](TupleType *a1) { // Warning: this implements COVARIANT arrays
                                     TupleType *a2 = dynamic_cast<TupleType *>(p2);
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
                                 [&](ShapeType *h1) { // Warning: this implements COVARIANT hashes
                                     ShapeType *h2 = dynamic_cast<ShapeType *>(p2);
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
                                 [&](LiteralType *l1) {
                                     LiteralType *l2 = dynamic_cast<LiteralType *>(p2);
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

bool ProxyType::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    return underlying->derivesFrom(ctx, klass);
}

bool ClassType::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    if (symbol == ctx.state.defn_untyped() || symbol == klass) {
        return true;
    }
    return symbol.info(ctx).derivesFrom(ctx, klass);
}

bool OrType::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    return left->derivesFrom(ctx, klass) && right->derivesFrom(ctx, klass);
}

bool AndType::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    return left->derivesFrom(ctx, klass) || right->derivesFrom(ctx, klass);
}

void AndType::_sanityCheck(core::Context ctx) {
    left->_sanityCheck(ctx);
    right->_sanityCheck(ctx);
    Error::check(!left->isDynamic());
    Error::check(!right->isDynamic());
}

void OrType::_sanityCheck(core::Context ctx) {
    left->_sanityCheck(ctx);
    right->_sanityCheck(ctx);
    Error::check(!left->isDynamic());
    Error::check(!right->isDynamic());
}

bool AliasType::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    Error::raise("AliasType::derivesFrom");
}

void AliasType::_sanityCheck(core::Context ctx) {
    Error::check(this->symbol.exists());
}
