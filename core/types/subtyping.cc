#include "../Types.h"
#include "common/common.h"
#include "core/Types.h"
#include <algorithm> // find_if
#include <unordered_map>

namespace ruby_typer {
namespace core {

using namespace std;

shared_ptr<ruby_typer::core::Type> lubGround(core::Context ctx, shared_ptr<Type> t1, shared_ptr<Type> t2);

shared_ptr<ruby_typer::core::Type> ruby_typer::core::Types::lub(core::Context ctx, shared_ptr<Type> t1,
                                                                shared_ptr<Type> t2) {
    auto ret = _lub(ctx, t1, t2);
    ret->sanityCheck(ctx);
    ENFORCE(Types::isSubType(ctx, t1, ret), ret->toString(ctx) + " is not a super type of " + t1->toString(ctx) +
                                                " was lubbing with " + t2->toString(ctx));
    ENFORCE(Types::isSubType(ctx, t2, ret), ret->toString(ctx) + " is not a super type of " + t2->toString(ctx) +
                                                " was lubbing with " + t1->toString(ctx));
    return ret;
}

shared_ptr<ruby_typer::core::Type> underlying(shared_ptr<Type> t1) {
    if (auto *f = dynamic_cast<ProxyType *>(t1.get())) {
        return f->underlying;
    }
    return t1;
}

shared_ptr<ruby_typer::core::Type> lubDistributeOr(core::Context ctx, shared_ptr<Type> t1, shared_ptr<Type> t2) {
    OrType *o1 = dynamic_cast<OrType *>(t1.get());
    ENFORCE(o1 != nullptr);
    shared_ptr<ruby_typer::core::Type> n1 = Types::lub(ctx, o1->left, t2);
    if (n1.get() == o1->left.get()) {
        categoryCounterInc("lub_distribute_or.outcome", "t1");
        return t1;
    }
    shared_ptr<ruby_typer::core::Type> n2 = Types::lub(ctx, o1->right, t2);
    if (n1.get() == t2.get()) {
        categoryCounterInc("lub_distribute_or.outcome", "n2'");
        return n2;
    }
    if (n2.get() == o1->right.get()) {
        categoryCounterInc("lub_distribute_or.outcome", "t1'");
        return t1;
    }
    if (n2.get() == t2.get()) {
        categoryCounterInc("lub_distribute_or.outcome", "n1'");
        return n1;
    }
    if (Types::isSubTypeWhenFrozen(ctx, n1, n2)) {
        categoryCounterInc("lub_distribute_or.outcome", "n2''");
        return n2;
    } else if (Types::isSubTypeWhenFrozen(ctx, n2, n1)) {
        categoryCounterInc("lub_distribute_or.outcome", "n1'''");
        return n1;
    }
    categoryCounterInc("lub_distribute_or.outcome", "worst");
    return OrType::make_shared(t1, underlying(t2)); // order matters for perf
}

shared_ptr<ruby_typer::core::Type> glbDistributeAnd(core::Context ctx, shared_ptr<Type> t1, shared_ptr<Type> t2) {
    AndType *a1 = dynamic_cast<AndType *>(t1.get());
    ENFORCE(t1 != nullptr);
    shared_ptr<ruby_typer::core::Type> n1 = Types::glb(ctx, a1->left, t2);
    if (n1.get() == a1->left.get()) {
        categoryCounterInc("lub_distribute_or.outcome", "t1");
        return t1;
    }
    shared_ptr<ruby_typer::core::Type> n2 = Types::glb(ctx, a1->right, t2);
    if (n1.get() == t2.get()) {
        categoryCounterInc("glbDistributeAnd.outcome", "Zn2");
        return n2;
    }
    if (n2.get() == a1->right.get()) {
        categoryCounterInc("glbDistributeAnd.outcome", "Zt1");
        return t1;
    }
    if (n2.get() == t2.get()) {
        categoryCounterInc("glbDistributeAnd.outcome", "Zn1");
        return n1;
    }
    if (Types::isSubTypeWhenFrozen(ctx, n1, n2)) {
        categoryCounterInc("glbDistributeAnd.outcome", "ZZn2");
        return n2;
    } else if (Types::isSubTypeWhenFrozen(ctx, n2, n1)) {
        categoryCounterInc("glbDistributeAnd.outcome", "ZZZn1");
        return n1;
    }

    categoryCounterInc("glbDistributeAnd.outcome", "worst");
    return AndType::make_shared(t1, t2);
}

shared_ptr<ruby_typer::core::Type> ruby_typer::core::Types::_lub(core::Context ctx, shared_ptr<Type> t1,
                                                                 shared_ptr<Type> t2) {
    if (t1.get() == t2.get()) {
        categoryCounterInc("lub", "ref-eq");
        return t1;
    }

    if (t1->kind() > t2->kind()) { // force the relation to be symmentric and half the implementation
        return _lub(ctx, t2, t1);
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

    if (ClassType *mayBeSpecial2 = cast_type<ClassType>(t2.get())) {
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

    if (auto *o2 = dynamic_cast<OrType *>(t2.get())) { // 3, 5, 6
        categoryCounterInc("lub", "or>");
        return lubDistributeOr(ctx, t2, t1);
    } else if (dynamic_cast<OrType *>(t1.get()) != nullptr) {
        categoryCounterInc("lub", "<or");
        return lubDistributeOr(ctx, t1, t2);
    } else if (auto *a2 = dynamic_cast<AndType *>(t2.get())) { // 2, 4
        categoryCounterInc("lub", "and>");
        auto t1d = underlying(t1);

        bool returnT1 = Types::isSubTypeWhenFrozen(ctx, t2, t1);
        if (returnT1) {
            categoryCounterInc("lub.and.t1", "f");
            return t1;
        }
        bool returnT2L = Types::isSubTypeWhenFrozen(ctx, t1d, a2->left);
        bool returnT2R = Types::isSubTypeWhenFrozen(ctx, t1d, a2->right);
        if (returnT2L) {
            categoryCounterInc("lub.and.t2", "l");
            if (returnT2R) {
                return t2;
            }
            // why recourse?
            // `A & (B | C) lub C` is not   `A & (B | C)` because A is not a supertype of C.
            return lub(ctx, a2->left, a2->right);
        }

        if (returnT2R) {
            categoryCounterInc("lub.and.t2", "r");
            // see comment in returnT2L branch
            return lub(ctx, a2->left, a2->right);
        }

        categoryCounterInc("lub.and>collapsed", "none");
        return OrType::make_shared(t1, t2);
    }

    if (AppliedType *a1 = dynamic_cast<AppliedType *>(t1.get())) {
        AppliedType *a2 = dynamic_cast<AppliedType *>(t2.get());
        if (a2 == nullptr) {
            if (isSubType(ctx, t2, t1)) {
                return t1;
            }
            return OrType::make_shared(t1, t2);
        }
        bool rtl = a1->klass == a2->klass || a1->klass.info(ctx).derivesFrom(ctx, a2->klass);
        bool ltr = !rtl && a2->klass.info(ctx).derivesFrom(ctx, a1->klass);
        if (!rtl && !ltr) {
            return OrType::make_shared(t1, t2);
        }
        if (ltr) { // swap
            auto tmp = a1;
            a1 = a2;
            a2 = tmp;
        }

        vector<SymbolRef> indexes = Types::alignBaseTypeArgs(ctx, a1->klass, a1->targs, a2->klass);
        vector<shared_ptr<Type>> newTargs;
        newTargs.reserve(indexes.size());
        int i = 0;
        for (SymbolRef idx : indexes) {
            int j = 0;
            while (a2->klass.info(ctx).typeMembers()[i] != idx) {
                j++;
            }
            if (idx.info(ctx).isCovariant()) {
                newTargs.push_back(Types::lub(ctx, a1->targs[i], a2->targs[j]));
            } else if (idx.info(ctx).isInvariant()) {
                if (!Types::equiv(ctx, a1->targs[i], a2->targs[j])) {
                    return OrType::make_shared(t1, t2);
                }
                newTargs.push_back(a1->targs[i]);
            } else if (idx.info(ctx).isContravariant()) {
                newTargs.push_back(Types::glb(ctx, a1->targs[i], a2->targs[j]));
            }
            i++;
        }
        return make_shared<AppliedType>(a2->klass, newTargs);
    }

    if (ProxyType *p1 = cast_type<ProxyType>(t1.get())) {
        categoryCounterInc("lub", "<proxy");
        if (ProxyType *p2 = cast_type<ProxyType>(t2.get())) {
            categoryCounterInc("lub", "proxy>");
            // both are proxy
            shared_ptr<ruby_typer::core::Type> result;
            ruby_typer::typecase(
                p1,
                [&](TupleType *a1) { // Warning: this implements COVARIANT arrays
                    if (TupleType *a2 = cast_type<TupleType>(p2)) {
                        if (a1->elems.size() == a2->elems.size()) { // lub arrays only if they have same element count
                            vector<shared_ptr<ruby_typer::core::Type>> elemLubs;
                            int i = -1;
                            for (auto &el2 : a2->elems) {
                                ++i;
                                elemLubs.emplace_back(lub(ctx, a1->elems[i], el2));
                            }
                            result = make_shared<TupleType>(elemLubs);
                        } else {
                            result = core::Types::arrayOfUntyped();
                        }
                    } else {
                        result = lub(ctx, p1->underlying, p2->underlying);
                    }
                },
                [&](ShapeType *h1) { // Warning: this implements COVARIANT hashes
                    if (ShapeType *h2 = cast_type<ShapeType>(p2)) {
                        if (h2->keys.size() == h1->keys.size()) {
                            // have enough keys.
                            int i = -1;
                            vector<shared_ptr<ruby_typer::core::LiteralType>> keys;
                            vector<shared_ptr<ruby_typer::core::Type>> valueLubs;
                            for (auto &el2 : h2->keys) {
                                ++i;
                                ClassType *u2 = cast_type<ClassType>(el2->underlying.get());
                                ENFORCE(u2 != nullptr);
                                auto fnd = find_if(h1->keys.begin(), h1->keys.end(), [&](auto &candidate) -> bool {
                                    ClassType *u1 = cast_type<ClassType>(candidate->underlying.get());
                                    return candidate->value == el2->value && u1 == u2; // from lambda
                                });
                                if (fnd != h1->keys.end()) {
                                    keys.emplace_back(el2);
                                    valueLubs.emplace_back(lub(ctx, h1->values[fnd - h1->keys.begin()], h2->values[i]));
                                } else {
                                    result = core::Types::hashOfUntyped();
                                    return;
                                }
                            }
                            result = make_shared<ShapeType>(keys, valueLubs);
                        } else {
                            result = core::Types::hashOfUntyped();
                        }
                    } else {
                        result = lub(ctx, p1->underlying, p2->underlying);
                    }
                },
                [&](LiteralType *l1) {
                    if (LiteralType *l2 = cast_type<LiteralType>(p2)) {
                        ClassType *u1 = cast_type<ClassType>(l1->underlying.get());
                        ClassType *u2 = cast_type<ClassType>(l2->underlying.get());
                        ENFORCE(u1 != nullptr && u2 != nullptr);
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
                        result = lub(ctx, p1->underlying, p2->underlying);
                    }

                });
            ENFORCE(result.get() != nullptr);
            return result;
        } else {
            // only 1st is proxy
            shared_ptr<Type> und = p1->underlying;
            return lub(ctx, und, t2);
        }
    } else if (ProxyType *p2 = cast_type<ProxyType>(t2.get())) {
        categoryCounterInc("lub", "proxy>");
        // only 2nd is proxy
        shared_ptr<Type> und = p2->underlying;
        return lub(ctx, t1, und);
    } else {
        // none is proxy
        return lubGround(ctx, t1, t2);
    }
}

shared_ptr<ruby_typer::core::Type> lubGround(core::Context ctx, shared_ptr<Type> t1, shared_ptr<Type> t2) {
    auto *g1 = cast_type<GroundType>(t1.get());
    auto *g2 = cast_type<GroundType>(t2.get());
    ENFORCE(g1 != nullptr);
    ENFORCE(g2 != nullptr);

    //    if (g1->kind() > g2->kind()) { // force the relation to be symmentric and half the implementation
    //        return lubGround(ctx, t2, t1);
    //    }
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

    // 1 :-)
    ClassType *c1 = cast_type<ClassType>(t1.get());
    ClassType *c2 = cast_type<ClassType>(t2.get());
    categoryCounterInc("lub", "<class>");
    ENFORCE(c1 != nullptr && c2 != nullptr);

    core::SymbolRef sym1 = c1->symbol;
    core::SymbolRef sym2 = c2->symbol;
    if (sym1 == sym2 || sym1.info(ctx).derivesFrom(ctx, sym2)) {
        categoryCounterInc("lub.<class>.collapsed", "yes");
        return t2;
    } else if (sym2.info(ctx).derivesFrom(ctx, sym1)) {
        categoryCounterInc("lub.<class>.collapsed", "yes");
        return t1;
    } else {
        categoryCounterInc("lub.class>.collapsed", "no");
        return OrType::make_shared(t1, t2);
    }
}

shared_ptr<ruby_typer::core::Type> glbGround(core::Context ctx, shared_ptr<Type> t1, shared_ptr<Type> t2) {
    auto *g1 = cast_type<GroundType>(t1.get());
    auto *g2 = cast_type<GroundType>(t2.get());
    ENFORCE(g1 != nullptr);
    ENFORCE(g2 != nullptr);

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
    // 1 :-)
    ClassType *c1 = cast_type<ClassType>(t1.get());
    ClassType *c2 = cast_type<ClassType>(t2.get());
    ENFORCE(c1 != nullptr && c2 != nullptr);
    categoryCounterInc("glb", "<class>");

    core::SymbolRef sym1 = c1->symbol;
    core::SymbolRef sym2 = c2->symbol;
    if (sym1 == sym2 || sym1.info(ctx).derivesFrom(ctx, sym2)) {
        categoryCounterInc("glb.<class>.collapsed", "yes");
        return t1;
    } else if (sym2.info(ctx).derivesFrom(ctx, sym1)) {
        categoryCounterInc("glb.<class>.collapsed", "yes");
        return t2;
    } else {
        if (sym1.info(ctx).isClass() && sym2.info(ctx).isClass()) {
            categoryCounterInc("glb.<class>.collapsed", "bottom");
            return Types::bottom();
        }
        categoryCounterInc("glb.<class>.collapsed", "no");
        return AndType::make_shared(t1, t2);
    }
}
shared_ptr<ruby_typer::core::Type> ruby_typer::core::Types::glb(core::Context ctx, shared_ptr<Type> t1,
                                                                shared_ptr<Type> t2) {
    auto ret = _glb(ctx, t1, t2);
    ret->sanityCheck(ctx);

    ENFORCE(Types::isSubType(ctx, ret, t1), ret->toString(ctx) + " is not a subtype of " + t1->toString(ctx) +
                                                " was glbbing with " + t2->toString(ctx));

    ENFORCE(Types::isSubType(ctx, ret, t2), ret->toString(ctx) + " is not a subtype of " + t2->toString(ctx) +
                                                " was glbbing with " + t1->toString(ctx));

    return ret;
}

shared_ptr<ruby_typer::core::Type> ruby_typer::core::Types::_glb(core::Context ctx, shared_ptr<Type> t1,
                                                                 shared_ptr<Type> t2) {
    if (t1.get() == t2.get()) {
        categoryCounterInc("glb", "ref-eq");
        return t1;
    }

    if (ClassType *mayBeSpecial1 = cast_type<ClassType>(t1.get())) {
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

    if (ClassType *mayBeSpecial2 = cast_type<ClassType>(t2.get())) {
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

    if (t1->kind() > t2->kind()) { // force the relation to be symmentric and half the implementation
        return _glb(ctx, t2, t1);
    }
    if (auto *a1 = dynamic_cast<AndType *>(t1.get())) { // 4, 5
        categoryCounterInc("glb", "<and");
        return glbDistributeAnd(ctx, t1, t2);
    } else if (auto *a2 = dynamic_cast<AndType *>(t2.get())) { // 2
        categoryCounterInc("glb", "and>");
        return glbDistributeAnd(ctx, t2, t1);
    }

    if (ProxyType *p1 = cast_type<ProxyType>(t1.get())) {
        if (ProxyType *p2 = cast_type<ProxyType>(t2.get())) {
            if (typeid(*p1) != typeid(*p2)) {
                return Types::bottom();
            }
            shared_ptr<ruby_typer::core::Type> result;
            ruby_typer::typecase(
                p1,
                [&](TupleType *a1) { // Warning: this implements COVARIANT arrays
                    TupleType *a2 = cast_type<TupleType>(p2);
                    ENFORCE(a2 != nullptr);
                    if (a1->elems.size() == a2->elems.size()) { // lub arrays only if they have same element count
                        vector<shared_ptr<ruby_typer::core::Type>> elemGlbs;
                        int i = -1;
                        for (auto &el2 : a2->elems) {
                            ++i;
                            auto glbe = glb(ctx, a1->elems[i], el2);
                            if (glbe->isBottom()) {
                                result = Types::bottom();
                                return;
                            }
                            elemGlbs.emplace_back(glbe);
                        }
                        result = make_shared<TupleType>(elemGlbs);
                    } else {
                        result = Types::bottom();
                    }

                },
                [&](ShapeType *h1) { // Warning: this implements COVARIANT hashes
                    ShapeType *h2 = cast_type<ShapeType>(p2);
                    ENFORCE(h2 != nullptr);
                    if (h2->keys.size() == h1->keys.size()) {
                        // have enough keys.
                        int i = -1;
                        vector<shared_ptr<ruby_typer::core::LiteralType>> keys;
                        vector<shared_ptr<ruby_typer::core::Type>> valueLubs;
                        for (auto &el2 : h2->keys) {
                            ++i;
                            ClassType *u2 = cast_type<ClassType>(el2->underlying.get());
                            ENFORCE(u2 != nullptr);
                            auto fnd = find_if(h1->keys.begin(), h1->keys.end(), [&](auto &candidate) -> bool {
                                ClassType *u1 = cast_type<ClassType>(candidate->underlying.get());
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
                        }
                        result = make_shared<ShapeType>(keys, valueLubs);
                    } else {
                        result = Types::bottom();
                    }

                },
                [&](LiteralType *l1) {
                    LiteralType *l2 = cast_type<LiteralType>(p2);
                    ENFORCE(l2 != nullptr);
                    ClassType *u1 = cast_type<ClassType>(l1->underlying.get());
                    ClassType *u2 = cast_type<ClassType>(l2->underlying.get());
                    ENFORCE(u1 != nullptr && u2 != nullptr);
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
            ENFORCE(result.get() != nullptr);
            return result;
        } else {
            // only 1st is proxy
            if (Types::isSubTypeWhenFrozen(ctx, t1, t2)) {
                return t1;
            } else {
                return Types::bottom();
            }
        }
    } else if (ProxyType *p2 = cast_type<ProxyType>(t2.get())) {
        // only 1st is proxy
        if (Types::isSubTypeWhenFrozen(ctx, t2, t1)) {
            return t2;
        } else {
            return Types::bottom();
        }
    }

    if (auto *o2 = dynamic_cast<OrType *>(t2.get())) { // 3, 6
        bool collapseInLeft = Types::isSubTypeWhenFrozen(ctx, t1, t2);
        if (collapseInLeft) {
            categoryCounterInc("glb", "Zor");
            return t1;
        }

        bool collapseInRight = Types::isSubTypeWhenFrozen(ctx, t2, t1);
        if (collapseInRight) {
            categoryCounterInc("glb", "ZZor");
            return t2;
        }

        if (auto *c1 = dynamic_cast<ClassType *>(t1.get())) {
            auto lft = Types::glb(ctx, t1, o2->left);
            if (Types::isSubTypeWhenFrozen(ctx, lft, o2->right)) {
                categoryCounterInc("glb", "ZZZorClass");
                return lft;
            }
            auto rght = Types::glb(ctx, t1, o2->right);
            if (Types::isSubTypeWhenFrozen(ctx, rght, o2->left)) {
                categoryCounterInc("glb", "ZZZZorClass");
                return rght;
            }
            if (lft->isBottom()) {
                return rght;
            }
            if (rght->isBottom()) {
                return lft;
            }
        }

        if (auto *o1 = dynamic_cast<OrType *>(t1.get())) { // 6
            // try hard to collapse
            bool subt11 = Types::isSubTypeWhenFrozen(ctx, o1->left, o2->left) ||
                          Types::isSubTypeWhenFrozen(ctx, o1->left, o2->right);
            if (!subt11) { // left is not in right, we can drop it
                categoryCounterInc("glb", "ZorOr");
                return Types::glb(ctx, o1->right, t2);
            }
            bool subt12 = Types::isSubTypeWhenFrozen(ctx, o1->right, o2->left) ||
                          Types::isSubTypeWhenFrozen(ctx, o1->right, o2->right);
            if (!subt12) {
                categoryCounterInc("glb", "ZZorOr");
                return Types::glb(ctx, o1->left, t2);
            }
            bool subt21 = Types::isSubTypeWhenFrozen(ctx, o2->left, o1->left) ||
                          Types::isSubTypeWhenFrozen(ctx, o2->left, o1->right);
            if (!subt21) {
                categoryCounterInc("glb", "ZZZorOr");
                return Types::glb(ctx, o2->right, t1);
            }
            bool subt22 = Types::isSubTypeWhenFrozen(ctx, o2->right, o1->left) ||
                          Types::isSubTypeWhenFrozen(ctx, o2->right, o1->right);
            if (!subt22) {
                categoryCounterInc("glb", "ZZZZorOr");
                return Types::glb(ctx, o2->left, t1);
            }
            categoryCounterInc("glb", "ZZZZZorWorst");
            return AndType::make_shared(t1, t2);
        } else {
            categoryCounterInc("glb.orcollapsed", "no");
            return AndType::make_shared(t1, t2);
        }
    }

    if (AppliedType *a1 = dynamic_cast<AppliedType *>(t1.get())) {
        AppliedType *a2 = dynamic_cast<AppliedType *>(t2.get());
        if (a2 == nullptr) {
            return AndType::make_shared(t1, t2);
        }
        bool rtl = a1->klass == a2->klass || a1->klass.info(ctx).derivesFrom(ctx, a2->klass);
        bool ltr = !rtl && a2->klass.info(ctx).derivesFrom(ctx, a1->klass);
        if (!rtl && !ltr) {
            return AndType::make_shared(t1, t2); // we can as well return nothing here?
        }
        if (ltr) { // swap
            auto tmp = a1;
            a1 = a2;
            a2 = tmp;
        }

        vector<SymbolRef> indexes = Types::alignBaseTypeArgs(ctx, a1->klass, a1->targs, a2->klass);
        vector<shared_ptr<Type>> newTargs;
        newTargs.reserve(indexes.size());
        int i = 0;
        for (SymbolRef idx : indexes) {
            int j = 0;
            while (a2->klass.info(ctx).typeMembers()[i] != idx) {
                j++;
            }
            if (idx.info(ctx).isCovariant()) {
                newTargs.push_back(Types::glb(ctx, a1->targs[i], a2->targs[j]));
            } else if (idx.info(ctx).isInvariant()) {
                if (!Types::equiv(ctx, a1->targs[i], a2->targs[j])) {
                    return AndType::make_shared(t1, t2); // we can as well return nothing here?
                }
                newTargs.push_back(a1->targs[i]);
            } else if (idx.info(ctx).isContravariant()) {
                newTargs.push_back(Types::lub(ctx, a1->targs[i], a2->targs[j]));
            }
            i++;
        }
        return make_shared<AppliedType>(a2->klass, newTargs);
    }

    return glbGround(ctx, t1, t2);
}

bool classSymbolIsAsGoodAs(core::Context ctx, core::SymbolRef c1, core::SymbolRef c2) {
    ENFORCE(c1.info(ctx).isClass());
    ENFORCE(c2.info(ctx).isClass());
    return c1 == c2 || c1.info(ctx).derivesFrom(ctx, c2);
}

// "Single" means "ClassType or ProxyType"; since ProxyTypes are constrained to
// be proxies over class types, this means "class or class-like"
bool isSubTypeSingle(core::Context ctx, shared_ptr<Type> t1, shared_ptr<Type> t2) {
    if (t1.get() == t2.get()) {
        return true;
    }
    if (ClassType *mayBeSpecial1 = cast_type<ClassType>(t1.get())) {
        if (mayBeSpecial1->symbol == core::GlobalState::defn_untyped()) {
            return true;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_bottom()) {
            return true;
        }
        if (mayBeSpecial1->symbol == core::GlobalState::defn_top()) {
            if (ClassType *mayBeSpecial2 = cast_type<ClassType>(t2.get())) {
                return mayBeSpecial2->symbol == core::GlobalState::defn_top();
            } else {
                return false;
            }
        }
    }

    if (ClassType *mayBeSpecial2 = cast_type<ClassType>(t2.get())) {
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

    if (TypeVar *tv1 = dynamic_cast<TypeVar *>(t1.get())) {
        if (tv1->isInstantiated) {
            return Types::isSubType(ctx, tv1->instantiation, t2);
        }
        bool already = false;
        for (auto &upperB : tv1->upperConstraints) {
            already = already || Types::isSubTypeWhenFrozen(ctx, upperB, t2);
        }
        if (already) {
            return true;
        }
        if (ctx.frozenConstraint) {
            return false;
        }
        tv1->upperConstraints.push_back(t2);
        // TODO: check upper constraint for validity?

        return true;
    }

    if (TypeVar *tv2 = dynamic_cast<TypeVar *>(t2.get())) {
        if (tv2->isInstantiated) {
            return Types::isSubType(ctx, t1, tv2->instantiation);
        }
        bool already = false;
        for (auto &lowerB : tv2->lowerConstraints) {
            already = already || Types::isSubTypeWhenFrozen(ctx, t1, lowerB);
        }
        if (already) {
            return true;
        }
        if (ctx.frozenConstraint) {
            return false;
        }

        tv2->lowerConstraints.push_back(t1);
        return true;
    }
    if (AppliedType *a1 = dynamic_cast<AppliedType *>(t1.get())) {
        AppliedType *a2 = dynamic_cast<AppliedType *>(t2.get());
        bool result;
        if (a2 == nullptr) {
            if (ClassType *c2 = dynamic_cast<ClassType *>(t2.get())) {
                return classSymbolIsAsGoodAs(ctx, a1->klass, c2->symbol);
            }
            return false;
        } else {
            result = classSymbolIsAsGoodAs(ctx, a1->klass, a2->klass);
        }
        if (result) {
            vector<SymbolRef> indexes = Types::alignBaseTypeArgs(ctx, a1->klass, a1->targs, a2->klass);
            int i = 0;
            for (SymbolRef idx : indexes) {
                int j = 0;
                while (a2->klass.info(ctx).typeMembers()[i] != idx) {
                    j++;
                }
                if (idx.info(ctx).isCovariant()) {
                    result = Types::isSubType(ctx, a1->targs[i], a2->targs[j]);
                } else if (idx.info(ctx).isInvariant()) {
                    result = Types::equiv(ctx, a1->targs[i], a2->targs[j]);
                } else if (idx.info(ctx).isContravariant()) {
                    result = Types::isSubType(ctx, a2->targs[j], a1->targs[i]);
                }
                if (!result) {
                    break;
                }
                i++;
            }
            // alight type params.
        }
        return result;
    }
    if (AppliedType *a2 = dynamic_cast<AppliedType *>(t2.get())) {
        if (ProxyType *pt = dynamic_cast<ProxyType *>(t1.get())) {
            return Types::isSubType(ctx, pt->underlying, t2);
        }
        return false;
    }

    if (ProxyType *p1 = cast_type<ProxyType>(t1.get())) {
        if (ProxyType *p2 = cast_type<ProxyType>(t2.get())) {
            bool result;
            // TODO: simply compare as memory regions
            ruby_typer::typecase(p1,
                                 [&](TupleType *a1) { // Warning: this implements COVARIANT arrays
                                     TupleType *a2 = cast_type<TupleType>(p2);
                                     result = a2 != nullptr && a1->elems.size() >= a2->elems.size();
                                     if (result) {
                                         int i = -1;
                                         for (auto &el2 : a2->elems) {
                                             ++i;
                                             result = Types::isSubType(ctx, a1->elems[i], el2);
                                             if (!result) {
                                                 break;
                                             }
                                         }
                                     }
                                 },
                                 [&](ShapeType *h1) { // Warning: this implements COVARIANT hashes
                                     ShapeType *h2 = cast_type<ShapeType>(p2);
                                     result = h2 != nullptr && h2->keys.size() <= h1->keys.size();
                                     if (!result) {
                                         return;
                                     }
                                     // have enough keys.
                                     int i = -1;
                                     for (auto &el2 : h2->keys) {
                                         ++i;
                                         ClassType *u2 = cast_type<ClassType>(el2->underlying.get());
                                         ENFORCE(u2 != nullptr);
                                         auto fnd =
                                             find_if(h1->keys.begin(), h1->keys.end(), [&](auto &candidate) -> bool {
                                                 ClassType *u1 = cast_type<ClassType>(candidate->underlying.get());
                                                 return candidate->value == el2->value && u1 == u2; // from lambda
                                             });
                                         result =
                                             fnd != h1->keys.end() &&
                                             Types::isSubType(ctx, h1->values[fnd - h1->keys.begin()], h2->values[i]);
                                         if (!result) {
                                             return;
                                         }
                                     }
                                 },
                                 [&](LiteralType *l1) {
                                     LiteralType *l2 = cast_type<LiteralType>(p2);
                                     ClassType *u1 = cast_type<ClassType>(l1->underlying.get());
                                     ClassType *u2 = cast_type<ClassType>(l2->underlying.get());
                                     ENFORCE(u1 != nullptr && u2 != nullptr);
                                     result = l2 != nullptr && u1->symbol == u2->symbol && l1->value == l2->value;
                                 });
            return result;
            // both are proxy
        } else {
            // only 1st is proxy
            shared_ptr<Type> und = p1->underlying;
            return isSubTypeSingle(ctx, und, t2);
        }
    } else if (ProxyType *p2 = cast_type<ProxyType>(t2.get())) {
        // non-proxies are never subtypes of proxies.
        return false;
    } else {
        if (auto *c1 = cast_type<ClassType>(t1.get())) {
            if (auto *c2 = cast_type<ClassType>(t2.get())) {
                return classSymbolIsAsGoodAs(ctx, c1->symbol, c2->symbol);
            }
        }
        Error::raise("isSubType(", t1->typeName(), ", ", t2->typeName(), "): unreachable");
    }
}

bool ruby_typer::core::Types::isSubType(core::Context ctx, shared_ptr<Type> t1, shared_ptr<Type> t2) {
    if (t1.get() == t2.get()) {
        return true;
    }

    // pairs to cover: 1  (_, _)
    //                 2  (_, And)
    //                 3  (_, Or)
    //                 4  (And, _)
    //                 5  (And, And)
    //                 6  (And, Or)
    //                 7 (Or, _)
    //                 8 (Or, And)
    //                 9 (Or, Or)
    // _ wildcards are ClassType or ProxyType(ClassType)

    // Note: order of cases here matters!
    if (auto *a1 = cast_type<OrType>(t1.get())) { // 7, 8, 9
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, a1->left, t2) && Types::isSubType(ctx, a1->right, t2);
    }

    if (auto *a2 = cast_type<AndType>(t2.get())) { // 2, 5
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, t1, a2->left) && Types::isSubType(ctx, t1, a2->right);
    }

    if (auto *a1 = cast_type<AndType>(t1.get())) { // 4, 6
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, a1->left, t2) || Types::isSubType(ctx, a1->right, t2);
    }

    if (auto *a2 = cast_type<OrType>(t2.get())) { // 3
        // this will be incorrect if\when we have Type members
        return Types::isSubType(ctx, t1, a2->left) || Types::isSubType(ctx, t1, a2->right);
    }

    return isSubTypeSingle(ctx, t1, t2); // 1
}

bool ruby_typer::core::Types::equiv(core::Context ctx, shared_ptr<Type> t1, shared_ptr<Type> t2) {
    return isSubType(ctx, t1, t2) && isSubType(ctx, t2, t1);
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

bool AliasType::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    Error::raise("AliasType.derivesfrom");
}

void AliasType::_sanityCheck(core::Context ctx) {
    ENFORCE(this->symbol.exists());
}

std::shared_ptr<Type> AliasType::instantiate(std::vector<SymbolRef> params, std::vector<std::shared_ptr<Type>> &targs) {
    Error::raise("should never happen");
}

std::string TypeConstructor::toString(GlobalState &gs, int tabs) {
    return "TypeConstructor";
}

std::string TypeConstructor::typeName() {
    return "TypeConstructor";
}

void TypeConstructor::_sanityCheck(core::Context ctx) {}

bool TypeConstructor::isFullyDefined() {
    return false;
}
std::shared_ptr<Type> TypeConstructor::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                                    std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                                    bool hasBlock) {
    Error::raise("should never happen");
}

std::shared_ptr<Type> TypeConstructor::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    Error::raise("should never happen");
}

bool TypeConstructor::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    return false;
}

std::shared_ptr<Type> TypeConstructor::instantiate(std::vector<SymbolRef> params,
                                                   std::vector<std::shared_ptr<Type>> &targs) {
    Error::raise("should never happen");
}

TypeConstructor::TypeConstructor(core::SymbolRef protoType, const std::vector<std::shared_ptr<Type>> &targs)
    : protoType(protoType), targs(targs) {}
} // namespace core
} // namespace ruby_typer
