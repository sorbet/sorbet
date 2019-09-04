#include "common/common.h"
#include "common/typecase.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"
#include <algorithm> // find_if
#include <utility>

namespace sorbet::core {

using namespace std;

TypePtr lubGround(Context ctx, const TypePtr &t1, const TypePtr &t2);

TypePtr Types::any(Context ctx, const TypePtr &t1, const TypePtr &t2) {
    auto ret = lub(ctx, t1, t2);
    ENFORCE(Types::isSubType(ctx, t1, ret), "\n{}\nis not a super type of\n{}\nwas lubbing with {}", ret->toString(ctx),
            t1->toString(ctx), t2->toString(ctx));
    ENFORCE(Types::isSubType(ctx, t2, ret), "\n{}\nis not a super type of\n{}\nwas lubbing with {}", ret->toString(ctx),
            t2->toString(ctx), t1->toString(ctx));

    //  TODO: @dmitry, reenable
    //    ENFORCE(t1->hasUntyped() || t2->hasUntyped() || ret->hasUntyped() || // check if this test makes sense
    //                !Types::isSubTypeUnderConstraint(ctx, t2, t1) || ret == t1 || ret->isUntyped(),
    //            "we do pointer comparisons in order to see if one is subtype of another. " + t1->toString(ctx) +
    //
    //                " was lubbing with " + t2->toString(ctx) + " got " + ret->toString(ctx));
    //
    //    ENFORCE(t1->hasUntyped() || t2->hasUntyped() || ret->hasUntyped() || // check if this test makes sense!
    //                !Types::isSubTypeUnderConstraint(ctx, t1, t2) || ret == t2 || ret->isUntyped() || ret == t1 ||
    //                Types::isSubTypeUnderConstraint(ctx, t2, t1),
    //            "we do pointer comparisons in order to see if one is subtype of another " + t1->toString(ctx) +
    //                " was lubbing with " + t2->toString(ctx) + " got " + ret->toString(ctx));

    ret->sanityCheck(ctx);

    return ret;
}

const TypePtr underlying(const TypePtr &t1) {
    if (auto *f = cast_type<ProxyType>(t1.get())) {
        return f->underlying();
    }
    return t1;
}

TypePtr lubDistributeOr(Context ctx, const TypePtr &t1, const TypePtr &t2) {
    auto *o1 = cast_type<OrType>(t1.get());
    ENFORCE(o1 != nullptr);
    TypePtr n1 = Types::any(ctx, o1->left, t2);
    if (n1.get() == o1->left.get()) {
        categoryCounterInc("lubDistributeOr.outcome", "t1");
        return t1;
    }
    TypePtr n2 = Types::any(ctx, o1->right, t2);
    if (n1.get() == t2.get()) {
        categoryCounterInc("lubDistributeOr.outcome", "n2'");
        return n2;
    }
    if (n2.get() == o1->right.get()) {
        categoryCounterInc("lubDistributeOr.outcome", "t1'");
        return t1;
    }
    if (n2.get() == t2.get()) {
        categoryCounterInc("lubDistributeOr.outcome", "n1'");
        return n1;
    }
    if (Types::isSubType(ctx, n1, n2)) {
        categoryCounterInc("lubDistributeOr.outcome", "n2''");
        return n2;
    } else if (Types::isSubType(ctx, n2, n1)) {
        categoryCounterInc("lubDistributeOr.outcome", "n1'''");
        return n1;
    }
    categoryCounterInc("lubDistributeOr.outcome", "worst");
    return OrType::make_shared(t1, underlying(t2)); // order matters for perf
}

TypePtr glbDistributeAnd(Context ctx, const TypePtr &t1, const TypePtr &t2) {
    auto *a1 = cast_type<AndType>(t1.get());
    ENFORCE(t1 != nullptr);
    TypePtr n1 = Types::all(ctx, a1->left, t2);
    if (n1.get() == a1->left.get()) {
        categoryCounterInc("lubDistributeOr.outcome", "t1");
        return t1;
    }
    TypePtr n2 = Types::all(ctx, a1->right, t2);
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
    if (Types::isSubType(ctx, n1, n2)) {
        categoryCounterInc("glbDistributeAnd.outcome", "ZZn1");
        return n1;
    } else if (Types::isSubType(ctx, n2, n1)) {
        categoryCounterInc("glbDistributeAnd.outcome", "ZZZn2");
        return n2;
    }

    categoryCounterInc("glbDistributeAnd.outcome", "worst");
    return AndType::make_shared(t1, t2);
}

// only keep knowledge in t1 that is not already present in t2. Return the same reference if unchaged
TypePtr dropLubComponents(Context ctx, const TypePtr &t1, const TypePtr &t2) {
    if (auto *a1 = cast_type<AndType>(t1.get())) {
        auto a1a = dropLubComponents(ctx, a1->left, t2);
        auto a1b = dropLubComponents(ctx, a1->right, t2);
        auto subl = Types::isSubType(ctx, a1a, t2);
        auto subr = Types::isSubType(ctx, a1b, t2);
        if (subl || subr) {
            return Types::bottom();
        }
        if (a1a != a1->left || a1b != a1->right) {
            return Types::all(ctx, a1a, a1b);
        }
    } else if (auto *o1 = cast_type<OrType>(t1.get())) {
        auto subl = Types::isSubType(ctx, o1->left, t2);
        auto subr = Types::isSubType(ctx, o1->right, t2);
        if (subl && subr) {
            return Types::bottom();
        } else if (subl) {
            return o1->right;
        } else if (subr) {
            return o1->left;
        }
    }
    return t1;
}

TypePtr Types::lub(Context ctx, const TypePtr &t1, const TypePtr &t2) {
    if (t1.get() == t2.get()) {
        categoryCounterInc("lub", "ref-eq");
        return t1;
    }

    if (t1->kind() > t2->kind()) { // force the relation to be symmentric and half the implementation
        return lub(ctx, t2, t1);
    }

    if (auto *mayBeSpecial1 = cast_type<ClassType>(t1.get())) {
        if (mayBeSpecial1->symbol == Symbols::untyped()) {
            categoryCounterInc("lub", "<untyped");
            return t1;
        }
        if (mayBeSpecial1->symbol == Symbols::bottom()) {
            categoryCounterInc("lub", "<bottom");
            return t2;
        }
        if (mayBeSpecial1->symbol == Symbols::top()) {
            categoryCounterInc("lub", "<top");
            return t1;
        }
    }

    if (auto *mayBeSpecial2 = cast_type<ClassType>(t2.get())) {
        if (mayBeSpecial2->symbol == Symbols::untyped()) {
            categoryCounterInc("lub", "untyped>");
            return t2;
        }
        if (mayBeSpecial2->symbol == Symbols::bottom()) {
            categoryCounterInc("lub", "bottom>");
            return t1;
        }
        if (mayBeSpecial2->symbol == Symbols::top()) {
            categoryCounterInc("lub", "top>");
            return t2;
        }
    }

    if (auto *o2 = cast_type<OrType>(t2.get())) { // 3, 5, 6
        categoryCounterInc("lub", "or>");
        return lubDistributeOr(ctx, t2, t1);
    } else if (auto *a2 = cast_type<AndType>(t2.get())) { // 2, 4
        categoryCounterInc("lub", "and>");
        auto t1d = underlying(t1);
        auto t2filtered = dropLubComponents(ctx, t2, t1d);
        if (t2filtered != t2) {
            return lub(ctx, t1, t2filtered);
        }
        return OrType::make_shared(t1, t2filtered);
    } else if (isa_type<OrType>(t1.get())) {
        categoryCounterInc("lub", "<or");
        return lubDistributeOr(ctx, t1, t2);
    }

    if (auto *a1 = cast_type<AppliedType>(t1.get())) {
        auto *a2 = cast_type<AppliedType>(t2.get());
        if (a2 == nullptr) {
            if (isSubType(ctx, t2, t1)) {
                return t1;
            }
            if (isSubType(ctx, t1, t2)) {
                return t2;
            }
            return OrType::make_shared(t1, t2);
        }

        bool ltr = a1->klass == a2->klass || a2->klass.data(ctx)->derivesFrom(ctx, a1->klass);
        bool rtl = !ltr && a1->klass.data(ctx)->derivesFrom(ctx, a2->klass);
        if (!rtl && !ltr) {
            return OrType::make_shared(t1, t2);
        }
        if (ltr) {
            swap(a1, a2);
        }
        const TypePtr &t1s = ltr ? t2 : t1;
        const TypePtr &t2s = ltr ? t1 : t2;
        // now a1 <: a2

        InlinedVector<SymbolRef, 4> indexes = Types::alignBaseTypeArgs(ctx, a1->klass, a1->targs, a2->klass);
        vector<TypePtr> newTargs;
        newTargs.reserve(indexes.size());
        // code below inverts permutation of type params
        int j = 0;
        bool changed = false;
        for (SymbolRef idx : a2->klass.data(ctx)->typeMembers()) {
            int i = 0;
            while (indexes[j] != a1->klass.data(ctx)->typeMembers()[i]) {
                i++;
            }
            ENFORCE(i < a1->klass.data(ctx)->typeMembers().size());
            if (idx.data(ctx)->isCovariant()) {
                newTargs.emplace_back(Types::any(ctx, a1->targs[i], a2->targs[j]));
            } else if (idx.data(ctx)->isInvariant()) {
                if (!Types::equiv(ctx, a1->targs[i], a2->targs[j])) {
                    return OrType::make_shared(t1s, t2s);
                }
                if (a1->targs[i]->isUntyped()) {
                    newTargs.emplace_back(a1->targs[i]);
                } else {
                    newTargs.emplace_back(a2->targs[j]);
                }

            } else if (idx.data(ctx)->isContravariant()) {
                newTargs.emplace_back(Types::all(ctx, a1->targs[i], a2->targs[j]));
            }
            changed = changed || newTargs.back() != a2->targs[j];
            j++;
        }
        if (changed) {
            return make_type<AppliedType>(a2->klass, newTargs);
        } else {
            return t2s;
        }
    }

    if (auto *p1 = cast_type<ProxyType>(t1.get())) {
        categoryCounterInc("lub", "<proxy");
        if (auto *p2 = cast_type<ProxyType>(t2.get())) {
            categoryCounterInc("lub", "proxy>");
            // both are proxy
            TypePtr result;
            typecase(
                p1,
                [&](TupleType *a1) { // Warning: this implements COVARIANT arrays
                    if (auto *a2 = cast_type<TupleType>(p2)) {
                        if (a1->elems.size() == a2->elems.size()) { // lub arrays only if they have same element count
                            vector<TypePtr> elemLubs;
                            int i = -1;
                            bool differ1 = false;
                            bool differ2 = false;
                            for (auto &el2 : a2->elems) {
                                ++i;
                                auto &inserted = elemLubs.emplace_back(lub(ctx, a1->elems[i], el2));
                                differ1 = differ1 || inserted != a1->elems[i];
                                differ2 = differ2 || inserted != el2;
                            }
                            if (!differ1) {
                                result = t1;
                            } else if (!differ2) {
                                result = t2;
                            } else {
                                result = TupleType::build(ctx, elemLubs);
                            }
                        } else {
                            result = Types::arrayOfUntyped();
                        }
                    } else {
                        result = lub(ctx, p1->underlying(), p2->underlying());
                    }
                },
                [&](ShapeType *h1) { // Warning: this implements COVARIANT hashes
                    if (auto *h2 = cast_type<ShapeType>(p2)) {
                        if (h2->keys.size() == h1->keys.size()) {
                            // have enough keys.
                            int i = -1;
                            vector<TypePtr> valueLubs;
                            valueLubs.reserve(h2->keys.size());
                            bool differ1 = false;
                            bool differ2 = false;
                            for (auto &el2 : h2->keys) {
                                ++i;
                                auto el2l = cast_type<LiteralType>(el2.get());
                                auto *u2 = cast_type<ClassType>(el2l->underlying().get());
                                ENFORCE(u2 != nullptr);
                                auto fnd = absl::c_find_if(h1->keys, [&](auto &candidate) -> bool {
                                    auto el1l = cast_type<LiteralType>(candidate.get());
                                    ClassType *u1 = cast_type<ClassType>(el1l->underlying().get());
                                    return el1l->value == el2l->value && u1->symbol == u2->symbol; // from lambda
                                });
                                if (fnd != h1->keys.end()) {
                                    auto &inserted = valueLubs.emplace_back(
                                        lub(ctx, h1->values[fnd - h1->keys.begin()], h2->values[i]));
                                    differ1 = differ1 || inserted != h1->values[fnd - h1->keys.begin()];
                                    differ2 = differ2 || inserted != h2->values[i];
                                } else {
                                    result = Types::hashOfUntyped();
                                    return;
                                }
                            }
                            if (!differ1) {
                                result = t1;
                            } else if (!differ2) {
                                result = t2;
                            } else {
                                result = make_type<ShapeType>(Types::hashOfUntyped(), h2->keys, valueLubs);
                            }
                        } else {
                            result = Types::hashOfUntyped();
                        }
                    } else {
                        result = lub(ctx, p1->underlying(), p2->underlying());
                    }
                },
                [&](LiteralType *l1) {
                    if (auto *l2 = cast_type<LiteralType>(p2)) {
                        auto *u1 = cast_type<ClassType>(l1->underlying().get());
                        auto *u2 = cast_type<ClassType>(l2->underlying().get());
                        ENFORCE(u1 != nullptr && u2 != nullptr);
                        if (u1->symbol == u2->symbol) {
                            if (l1->value == l2->value) {
                                result = t1;
                            } else {
                                result = l1->underlying();
                            }
                        } else {
                            result = lubGround(ctx, l1->underlying(), l2->underlying());
                        }
                    } else {
                        result = lub(ctx, p1->underlying(), p2->underlying());
                    }
                },
                [&](MetaType *m1) {
                    if (auto *m2 = cast_type<MetaType>(p2)) {
                        if (Types::equiv(ctx, m1->wrapped, m2->wrapped)) {
                            result = t1;
                            return;
                        }
                    }
                    result = lub(ctx, p1->underlying(), p2->underlying());
                });
            ENFORCE(result.get() != nullptr);
            return result;
        } else {
            bool allowProxyInLub = isa_type<TupleType>(p1) || isa_type<ShapeType>(p1);
            // only 1st is proxy
            TypePtr und = p1->underlying();
            if (isSubType(ctx, und, t2)) {
                return t2;
            } else if (allowProxyInLub) {
                return OrType::make_shared(t1, t2);
            } else {
                return lub(ctx, t2, und);
            }
        }
    } else if (auto *p2 = cast_type<ProxyType>(t2.get())) {
        // only 2nd is proxy
        bool allowProxyInLub = isa_type<TupleType>(p2) || isa_type<ShapeType>(p2);
        // only 1st is proxy
        TypePtr und = p2->underlying();
        if (isSubType(ctx, und, t1)) {
            return t1;
        } else if (allowProxyInLub) {
            return OrType::make_shared(t1, t2);
        } else {
            return lub(ctx, t1, und);
        }
    }

    {
        if (isa_type<LambdaParam>(t1.get()) || isa_type<LambdaParam>(t2.get())) {
            return OrType::make_shared(t1, t2);
        }
    }

    {
        if (isa_type<TypeVar>(t1.get()) || isa_type<TypeVar>(t2.get())) {
            return OrType::make_shared(t1, t2);
        }
    }

    {
        auto *s1 = cast_type<SelfTypeParam>(t1.get());
        auto *s2 = cast_type<SelfTypeParam>(t2.get());

        if (s1 != nullptr || s2 != nullptr) {
            if (s1 == nullptr || s2 == nullptr || s2->definition != s1->definition) {
                return OrType::make_shared(t1, t2);
            } else {
                return t1;
            }
        }
    }

    {
        auto *self1 = cast_type<SelfType>(t1.get());
        auto *self2 = cast_type<SelfType>(t2.get());

        if (self1 != nullptr || self2 != nullptr) {
            if (self1 == nullptr || self2 == nullptr) {
                return OrType::make_shared(t1, t2);
            } else {
                return t1;
            }
        }
    }

    // none is proxy
    return lubGround(ctx, t1, t2);
}

TypePtr lubGround(Context ctx, const TypePtr &t1, const TypePtr &t2) {
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

    TypePtr result;

    // 1 :-)
    auto *c1 = cast_type<ClassType>(t1.get());
    auto *c2 = cast_type<ClassType>(t2.get());
    categoryCounterInc("lub", "<class>");
    ENFORCE(c1 != nullptr && c2 != nullptr);

    SymbolRef sym1 = c1->symbol;
    SymbolRef sym2 = c2->symbol;
    if (sym1 == sym2 || sym2.data(ctx)->derivesFrom(ctx, sym1)) {
        categoryCounterInc("lub.<class>.collapsed", "yes");
        return t1;
    } else if (sym1.data(ctx)->derivesFrom(ctx, sym2)) {
        categoryCounterInc("lub.<class>.collapsed", "yes");
        return t2;
    } else {
        categoryCounterInc("lub.<class>.collapsed", "no");
        return OrType::make_shared(t1, t2);
    }
}

TypePtr glbGround(Context ctx, const TypePtr &t1, const TypePtr &t2) {
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

    TypePtr result;
    // 1 :-)
    auto *c1 = cast_type<ClassType>(t1.get());
    auto *c2 = cast_type<ClassType>(t2.get());
    ENFORCE(c1 != nullptr && c2 != nullptr);
    categoryCounterInc("glb", "<class>");

    SymbolRef sym1 = c1->symbol;
    SymbolRef sym2 = c2->symbol;
    if (sym1 == sym2 || sym1.data(ctx)->derivesFrom(ctx, sym2)) {
        categoryCounterInc("glb.<class>.collapsed", "yes");
        return t1;
    } else if (sym2.data(ctx)->derivesFrom(ctx, sym1)) {
        categoryCounterInc("glb.<class>.collapsed", "yes");
        return t2;
    } else {
        if (sym1.data(ctx)->isClassClass() && sym2.data(ctx)->isClassClass()) {
            categoryCounterInc("glb.<class>.collapsed", "bottom");
            return Types::bottom();
        }
        categoryCounterInc("glb.<class>.collapsed", "no");
        return AndType::make_shared(t1, t2);
    }
}
TypePtr Types::all(Context ctx, const TypePtr &t1, const TypePtr &t2) {
    auto ret = glb(ctx, t1, t2);
    ret->sanityCheck(ctx);

    ENFORCE(Types::isSubType(ctx, ret, t1), "\n{}\nis not a subtype of\n{}\nwas glbbing with\n{}", ret->toString(ctx),
            t1->toString(ctx), t2->toString(ctx));

    ENFORCE(Types::isSubType(ctx, ret, t2), "\n{}\n is not a subtype of\n{}\nwas glbbing with\n{}", ret->toString(ctx),
            t2->toString(ctx), t1->toString(ctx));
    //  TODO: @dmitry, reenable
    //    ENFORCE(t1->hasUntyped() || t2->hasUntyped() || ret->hasUntyped() || // check if this test makes sense
    //                !Types::isSubTypeUnderConstraint(ctx, t1, t2) || ret == t1 || ret->isUntyped(),
    //            "we do pointer comparisons in order to see if one is subtype of another. " + t1->toString(ctx) +
    //
    //                " was glbbing with " + t2->toString(ctx) + " got " + ret->toString(ctx));
    //
    //    ENFORCE(t1->hasUntyped() || t2->hasUntyped() || ret->hasUntyped() || // check if this test makes sense
    //                !Types::isSubTypeUnderConstraint(ctx, t2, t1) || ret == t2 || ret->isUntyped() || ret == t1 ||
    //                Types::isSubTypeUnderConstraint(ctx, t1, t2),
    //            "we do pointer comparisons in order to see if one is subtype of another " + t1->toString(ctx) +
    //                " was glbbing with " + t2->toString(ctx) + " got " + ret->toString(ctx));

    return ret;
}

TypePtr Types::glb(Context ctx, const TypePtr &t1, const TypePtr &t2) {
    if (t1.get() == t2.get()) {
        categoryCounterInc("glb", "ref-eq");
        return t1;
    }

    if (auto *mayBeSpecial1 = cast_type<ClassType>(t1.get())) {
        if (mayBeSpecial1->symbol == Symbols::untyped()) {
            categoryCounterInc("glb", "<untyped");
            return t2;
        }
        if (mayBeSpecial1->symbol == Symbols::bottom()) {
            categoryCounterInc("glb", "<bottom");
            return t1;
        }
        if (mayBeSpecial1->symbol == Symbols::top()) {
            categoryCounterInc("glb", "<top");
            return t2;
        }
    }

    if (auto *mayBeSpecial2 = cast_type<ClassType>(t2.get())) {
        if (mayBeSpecial2->symbol == Symbols::untyped()) {
            categoryCounterInc("glb", "untyped>");
            return t1;
        }
        if (mayBeSpecial2->symbol == Symbols::bottom()) {
            categoryCounterInc("glb", "bottom>");
            return t2;
        }
        if (mayBeSpecial2->symbol == Symbols::top()) {
            categoryCounterInc("glb", "top>");
            return t1;
        }
    }

    if (t1->kind() > t2->kind()) { // force the relation to be symmentric and half the implementation
        return glb(ctx, t2, t1);
    }
    if (isa_type<AndType>(t1.get())) { // 4, 5
        categoryCounterInc("glb", "<and");
        return glbDistributeAnd(ctx, t1, t2);
    } else if (isa_type<AndType>(t2.get())) { // 2
        categoryCounterInc("glb", "and>");
        return glbDistributeAnd(ctx, t2, t1);
    }

    if (auto *p1 = cast_type<ProxyType>(t1.get())) {
        if (auto *p2 = cast_type<ProxyType>(t2.get())) {
            if (typeid(*p1) != typeid(*p2)) {
                return Types::bottom();
            }
            TypePtr result;
            typecase(
                p1,
                [&](TupleType *a1) { // Warning: this implements COVARIANT arrays
                    auto *a2 = cast_type<TupleType>(p2);
                    ENFORCE(a2 != nullptr);
                    if (a1->elems.size() == a2->elems.size()) { // lub arrays only if they have same element count
                        vector<TypePtr> elemGlbs;
                        elemGlbs.reserve(a2->elems.size());

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
                        if (absl::c_equal(a1->elems, elemGlbs)) {
                            result = t1;
                        } else if (absl::c_equal(a2->elems, elemGlbs)) {
                            result = t2;
                        } else {
                            result = TupleType::build(ctx, elemGlbs);
                        }
                    } else {
                        result = Types::bottom();
                    }

                },
                [&](ShapeType *h1) { // Warning: this implements COVARIANT hashes
                    auto *h2 = cast_type<ShapeType>(p2);
                    ENFORCE(h2 != nullptr);
                    if (h2->keys.size() == h1->keys.size()) {
                        // have enough keys.
                        int i = -1;
                        vector<TypePtr> valueLubs;
                        valueLubs.reserve(h2->keys.size());
                        bool canReuseT1 = true;
                        bool canReuseT2 = true;
                        for (auto &el2 : h2->keys) {
                            ++i;
                            auto el2l = cast_type<LiteralType>(el2.get());
                            auto *u2 = cast_type<ClassType>(el2l->underlying().get());
                            ENFORCE(u2 != nullptr);
                            auto fnd = absl::c_find_if(h1->keys, [&](auto &candidate) -> bool {
                                auto el1l = cast_type<LiteralType>(candidate.get());
                                ClassType *u1 = cast_type<ClassType>(el1l->underlying().get());
                                return el1l->value == el2l->value && u1->symbol == u2->symbol; // from lambda
                            });
                            if (fnd != h1->keys.end()) {
                                auto left = h1->values[fnd - h1->keys.begin()];
                                auto right = h2->values[i];
                                auto glbe = glb(ctx, left, right);
                                if (glbe->isBottom()) {
                                    result = Types::bottom();
                                    return;
                                }
                                canReuseT1 &= glbe == left;
                                canReuseT2 &= glbe == right;
                                valueLubs.emplace_back(glbe);
                            } else {
                                result = Types::bottom();
                                return;
                            }
                        }
                        if (canReuseT1) {
                            result = t1;
                        } else if (canReuseT2) {
                            result = t2;
                        } else {
                            result = make_type<ShapeType>(Types::hashOfUntyped(), h2->keys, valueLubs);
                        }
                    } else {
                        result = Types::bottom();
                    }

                },
                [&](LiteralType *l1) {
                    auto *l2 = cast_type<LiteralType>(p2);
                    ENFORCE(l2 != nullptr);
                    auto *u1 = cast_type<ClassType>(l1->underlying().get());
                    auto *u2 = cast_type<ClassType>(l2->underlying().get());
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
                },
                [&](MetaType *m1) {
                    auto *m2 = cast_type<MetaType>(p2);
                    ENFORCE(m2 != nullptr);
                    if (Types::equiv(ctx, m1->wrapped, m2->wrapped)) {
                        result = t1;
                    } else {
                        result = Types::bottom();
                    }
                });
            ENFORCE(result.get() != nullptr);
            return result;
        } else {
            // only 1st is proxy
            if (Types::isSubType(ctx, t1, t2)) {
                return t1;
            } else {
                return Types::bottom();
            }
        }
    } else if (isa_type<ProxyType>(t2.get())) {
        // only 1st is proxy
        if (Types::isSubType(ctx, t2, t1)) {
            return t2;
        } else {
            return Types::bottom();
        }
    }

    if (auto *o2 = cast_type<OrType>(t2.get())) { // 3, 6
        bool collapseInLeft = Types::isAsSpecificAs(ctx, t1, t2);
        if (collapseInLeft) {
            categoryCounterInc("glb", "Zor");
            return t1;
        }

        bool collapseInRight = Types::isAsSpecificAs(ctx, t2, t1);
        if (collapseInRight) {
            categoryCounterInc("glb", "ZZor");
            return t2;
        }

        if (isa_type<ClassType>(t1.get()) || isa_type<AppliedType>(t1.get())) {
            auto lft = Types::all(ctx, t1, o2->left);
            if (Types::isAsSpecificAs(ctx, lft, o2->right) && !lft->isBottom()) {
                categoryCounterInc("glb", "ZZZorClass");
                return lft;
            }
            auto rght = Types::all(ctx, t1, o2->right);
            if (Types::isAsSpecificAs(ctx, rght, o2->left) && !rght->isBottom()) {
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

        if (auto *o1 = cast_type<OrType>(t1.get())) { // 6
            auto t11 = Types::all(ctx, o1->left, o2->left);
            auto t12 = Types::all(ctx, o1->left, o2->right);
            auto t21 = Types::all(ctx, o1->right, o2->left);
            auto t22 = Types::all(ctx, o1->right, o2->right);

            // This is a heuristic to try and eagerly make a smaller type. For
            // now we are choosing that if any type collapses then we should use
            // an Or otherwise use an And.
            auto score = 0;
            if (t11 == o1->left || t11 == o2->left) {
                score++;
            }
            if (t12 == o1->left || t12 == o2->right) {
                score++;
            }
            if (t21 == o1->right || t21 == o2->left) {
                score++;
            }
            if (t22 == o1->right || t22 == o2->right) {
                score++;
            }
            if (t11->isBottom() || t12->isBottom() || t21->isBottom() || t22->isBottom()) {
                score++;
            }

            if (score > 0) {
                return Types::any(ctx, Types::any(ctx, t11, t12), Types::any(ctx, t21, t22));
            }
        }
        categoryCounterInc("glb.orcollapsed", "no");
        return AndType::make_shared(t1, t2);
    }

    if (auto *a1 = cast_type<AppliedType>(t1.get())) {
        auto *a2 = cast_type<AppliedType>(t2.get());
        if (a2 == nullptr) {
            auto *c2 = cast_type<ClassType>(t2.get());
            if (a1->klass.data(ctx)->isClassModule() || c2 == nullptr) {
                return AndType::make_shared(t1, t2);
            }
            if (a1->klass.data(ctx)->derivesFrom(ctx, c2->symbol)) {
                return t1;
            }
            if (c2->symbol.data(ctx)->isClassModule()) {
                return AndType::make_shared(t1, t2);
            }
            return Types::bottom();
        }
        bool rtl = a1->klass == a2->klass || a1->klass.data(ctx)->derivesFrom(ctx, a2->klass);
        bool ltr = !rtl && a2->klass.data(ctx)->derivesFrom(ctx, a1->klass);
        if (!rtl && !ltr) {
            if (a1->klass.data(ctx)->isClass() && a2->klass.data(ctx)->isClass()) {
                return Types::bottom();
            } else {
                return AndType::make_shared(t1, t2); // we can as well return nothing here?
            }
        }
        if (ltr) { // swap
            swap(a1, a2);
        }
        // a1 <:< a2

        InlinedVector<SymbolRef, 4> indexes = Types::alignBaseTypeArgs(ctx, a2->klass, a2->targs, a1->klass);

        // code below inverts permutation of type params

        vector<TypePtr> newTargs;
        newTargs.reserve(a1->klass.data(ctx)->typeMembers().size());
        int j = 0;
        for (SymbolRef idx : a1->klass.data(ctx)->typeMembers()) {
            int i = 0;
            if (j >= indexes.size()) {
                i = INT_MAX;
            }
            while (i < a2->klass.data(ctx)->typeMembers().size() &&
                   indexes[j] != a2->klass.data(ctx)->typeMembers()[i]) {
                i++;
            }
            if (i >= a2->klass.data(ctx)->typeMembers().size()) { // a1 has more tparams, this is fine, it's a child
                newTargs.emplace_back(a1->targs[j]);
            } else {
                if (idx.data(ctx)->isCovariant()) {
                    newTargs.emplace_back(Types::all(ctx, a1->targs[j], a2->targs[i]));
                } else if (idx.data(ctx)->isInvariant()) {
                    if (!Types::equiv(ctx, a1->targs[j], a2->targs[i])) {
                        return AndType::make_shared(t1, t2);
                    }
                    if (a1->targs[j]->isUntyped()) {
                        newTargs.emplace_back(a2->targs[i]);
                    } else {
                        newTargs.emplace_back(a1->targs[j]);
                    }
                } else if (idx.data(ctx)->isContravariant()) {
                    newTargs.emplace_back(Types::any(ctx, a1->targs[j], a2->targs[i]));
                }
            }
            j++;
        }
        if (absl::c_equal(a1->targs, newTargs)) {
            return ltr ? t2 : t1;
        } else if (absl::c_equal(a2->targs, newTargs) && a1->klass == a2->klass) {
            return ltr ? t1 : t2;
        } else {
            return make_type<AppliedType>(a1->klass, newTargs);
        }
    }
    {
        if (isa_type<TypeVar>(t1.get()) || isa_type<TypeVar>(t2.get())) {
            return AndType::make_shared(t1, t2);
        }
    }
    {
        auto *s1 = cast_type<SelfTypeParam>(t1.get());
        auto *s2 = cast_type<SelfTypeParam>(t2.get());

        if (s1 != nullptr || s2 != nullptr) {
            if (s1 == nullptr || s2 == nullptr || s2->definition != s1->definition) {
                return AndType::make_shared(t1, t2);
            } else {
                return t1;
            }
        }
    }

    {
        auto *self1 = cast_type<SelfType>(t1.get());
        auto *self2 = cast_type<SelfType>(t2.get());

        if (self1 != nullptr || self2 != nullptr) {
            if (self1 == nullptr || self2 == nullptr) {
                return AndType::make_shared(t1, t2);
            } else {
                return t1;
            }
        }
    }
    return glbGround(ctx, t1, t2);
}

bool classSymbolIsAsGoodAs(Context ctx, SymbolRef c1, SymbolRef c2) {
    ENFORCE(c1.data(ctx)->isClass());
    ENFORCE(c2.data(ctx)->isClass());
    return c1 == c2 || c1.data(ctx)->derivesFrom(ctx, c2);
}

void compareToUntyped(Context ctx, TypeConstraint &constr, const TypePtr &ty, const TypePtr &blame) {
    ENFORCE(blame->isUntyped());
    if (auto *p = cast_type<ProxyType>(ty.get())) {
        compareToUntyped(ctx, constr, p->underlying(), blame);
    }

    if (auto *t = cast_type<AppliedType>(ty.get())) {
        for (auto &targ : t->targs) {
            compareToUntyped(ctx, constr, targ, blame);
        }
    } else if (auto *t = cast_type<ShapeType>(ty.get())) {
        for (auto &val : t->values) {
            compareToUntyped(ctx, constr, val, blame);
        }
    } else if (auto *t = cast_type<TupleType>(ty.get())) {
        for (auto &val : t->elems) {
            compareToUntyped(ctx, constr, val, blame);
        }
    } else if (auto *t = cast_type<OrType>(ty.get())) {
        compareToUntyped(ctx, constr, t->left, blame);
        compareToUntyped(ctx, constr, t->right, blame);
    } else if (auto *t = cast_type<AndType>(ty.get())) {
        compareToUntyped(ctx, constr, t->left, blame);
        compareToUntyped(ctx, constr, t->right, blame);
    } else if (auto *t = cast_type<TypeVar>(ty.get())) {
        constr.rememberIsSubtype(ctx, ty, blame);
    }
}

// "Single" means "ClassType or ProxyType"; since ProxyTypes are constrained to
// be proxies over class types, this means "class or class-like"
bool isSubTypeUnderConstraintSingle(Context ctx, TypeConstraint &constr, UntypedMode mode, const TypePtr &t1,
                                    const TypePtr &t2) {
    ENFORCE(t1 != nullptr);
    ENFORCE(t2 != nullptr);

    if (t1.get() == t2.get()) {
        return true;
    }

    if (isa_type<TypeVar>(t1.get()) || isa_type<TypeVar>(t2.get())) {
        if (constr.isSolved()) {
            return constr.isAlreadyASubType(ctx, t1, t2);
        } else {
            return constr.rememberIsSubtype(ctx, t1, t2);
        }
    }

    if (auto *mayBeSpecial1 = cast_type<ClassType>(t1.get())) {
        if (mayBeSpecial1->symbol == Symbols::untyped()) {
            if (!constr.isSolved()) {
                compareToUntyped(ctx, constr, t2, t1);
            }
            return mode == UntypedMode::AlwaysCompatible;
        }
        if (mayBeSpecial1->symbol == Symbols::bottom()) {
            return true;
        }
        if (mayBeSpecial1->symbol == Symbols::top()) {
            if (auto *mayBeSpecial2 = cast_type<ClassType>(t2.get())) {
                return mayBeSpecial2->symbol == Symbols::top() || mayBeSpecial2->symbol == Symbols::untyped();
            } else {
                return false;
            }
        }
    }

    if (auto *mayBeSpecial2 = cast_type<ClassType>(t2.get())) {
        if (mayBeSpecial2->symbol == Symbols::untyped()) {
            if (!constr.isSolved()) {
                compareToUntyped(ctx, constr, t1, t2);
            }
            return mode == UntypedMode::AlwaysCompatible;
        }
        if (mayBeSpecial2->symbol == Symbols::bottom()) {
            return false; // (bot, bot) is handled above.
        }
        if (mayBeSpecial2->symbol == Symbols::top()) {
            return true;
        }
    }
    //    ENFORCE(!isa_type<LambdaParam>(t1.get())); // sandly, this is false in Resolver, as we build
    //    original signatures using lub ENFORCE(cast_type<LambdaParam>(t2.get()) == nullptr);

    {
        auto *lambda1 = cast_type<LambdaParam>(t1.get());
        auto *lambda2 = cast_type<LambdaParam>(t2.get());
        if (lambda1 != nullptr || lambda2 != nullptr) {
            // This should only be reachable in resolver.
            if (lambda1 == nullptr || lambda2 == nullptr) {
                return false;
            }
            return lambda1->definition == lambda2->definition;
        }
    }

    {
        auto *self1 = cast_type<SelfTypeParam>(t1.get());
        auto *self2 = cast_type<SelfTypeParam>(t2.get());
        if (self1 != nullptr || self2 != nullptr) {
            if (self1 == nullptr || self2 == nullptr) {
                return false;
            }
            return self1->definition == self2->definition;
        }
    }

    {
        auto *self1 = cast_type<SelfType>(t1.get());
        auto *self2 = cast_type<SelfType>(t2.get());
        if (self1 != nullptr || self2 != nullptr) {
            if (self1 == nullptr || self2 == nullptr) {
                return false;
            }
            return true;
        }
    }

    if (auto *a1 = cast_type<AppliedType>(t1.get())) {
        auto *a2 = cast_type<AppliedType>(t2.get());
        bool result;
        if (a2 == nullptr) {
            if (auto *c2 = cast_type<ClassType>(t2.get())) {
                return classSymbolIsAsGoodAs(ctx, a1->klass, c2->symbol);
            }
            return false;
        } else {
            result = classSymbolIsAsGoodAs(ctx, a1->klass, a2->klass);
        }
        if (result) {
            InlinedVector<SymbolRef, 4> indexes = Types::alignBaseTypeArgs(ctx, a1->klass, a1->targs, a2->klass);
            // code below inverts permutation of type params
            int j = 0;
            for (SymbolRef idx : a2->klass.data(ctx)->typeMembers()) {
                int i = 0;
                while (indexes[j] != a1->klass.data(ctx)->typeMembers()[i]) {
                    i++;
                    if (i >= a1->klass.data(ctx)->typeMembers().size()) {
                        return result;
                    }
                }

                ENFORCE(i < a1->klass.data(ctx)->typeMembers().size());

                if (idx.data(ctx)->isCovariant()) {
                    result = Types::isSubTypeUnderConstraint(ctx, constr, mode, a1->targs[i], a2->targs[j]);
                } else if (idx.data(ctx)->isInvariant()) {
                    auto &a = a1->targs[i];
                    auto &b = a2->targs[j];
                    if (mode == UntypedMode::AlwaysCompatible) {
                        result = Types::equiv(ctx, a, b);
                    } else {
                        result = Types::equivNoUntyped(ctx, a, b);
                    }
                } else if (idx.data(ctx)->isContravariant()) {
                    result = Types::isSubTypeUnderConstraint(ctx, constr, mode, a2->targs[j], a1->targs[i]);
                }
                if (!result) {
                    break;
                }
                j++;
            }
            // alight type params.
        }
        return result;
    }
    if (isa_type<AppliedType>(t2.get())) {
        if (auto *pt = cast_type<ProxyType>(t1.get())) {
            return Types::isSubTypeUnderConstraint(ctx, constr, mode, pt->underlying(), t2);
        }
        return false;
    }

    if (auto *p1 = cast_type<ProxyType>(t1.get())) {
        if (auto *p2 = cast_type<ProxyType>(t2.get())) {
            bool result;
            // TODO: simply compare as memory regions
            typecase(
                p1,
                [&](TupleType *a1) { // Warning: this implements COVARIANT arrays
                    auto *a2 = cast_type<TupleType>(p2);
                    result = a2 != nullptr && a1->elems.size() >= a2->elems.size();
                    if (result) {
                        int i = -1;
                        for (auto &el2 : a2->elems) {
                            ++i;
                            result = Types::isSubTypeUnderConstraint(ctx, constr, mode, a1->elems[i], el2);
                            if (!result) {
                                break;
                            }
                        }
                    }
                },
                [&](ShapeType *h1) { // Warning: this implements COVARIANT hashes
                    auto *h2 = cast_type<ShapeType>(p2);
                    result = h2 != nullptr && h2->keys.size() <= h1->keys.size();
                    if (!result) {
                        return;
                    }
                    // have enough keys.
                    int i = -1;
                    for (auto &el2 : h2->keys) {
                        ++i;
                        auto el2l = cast_type<LiteralType>(el2.get());
                        auto *u2 = cast_type<ClassType>(el2l->underlying().get());
                        ENFORCE(u2 != nullptr);
                        auto fnd = absl::c_find_if(h1->keys, [&](auto &candidate) -> bool {
                            auto el1l = cast_type<LiteralType>(candidate.get());
                            ClassType *u1 = cast_type<ClassType>(el1l->underlying().get());
                            return el1l->value == el2l->value && u1->symbol == u2->symbol; // from lambda
                        });
                        result = fnd != h1->keys.end() &&
                                 Types::isSubTypeUnderConstraint(ctx, constr, mode, h1->values[fnd - h1->keys.begin()],
                                                                 h2->values[i]);
                        if (!result) {
                            return;
                        }
                    }
                },
                [&](LiteralType *l1) {
                    auto *l2 = cast_type<LiteralType>(p2);
                    if (l2 == nullptr) {
                        // is a literal a subtype of a different kind of proxy
                        result = false;
                        return;
                    }
                    auto *u1 = cast_type<ClassType>(l1->underlying().get());
                    auto *u2 = cast_type<ClassType>(l2->underlying().get());
                    ENFORCE(u1 != nullptr && u2 != nullptr);
                    result = l2 != nullptr && u1->symbol == u2->symbol && l1->value == l2->value;
                },
                [&](MetaType *m1) {
                    auto *m2 = cast_type<MetaType>(p2);
                    if (m2 == nullptr) {
                        // is a literal a subtype of a different kind of proxy
                        result = false;
                        return;
                    }

                    result = Types::equiv(ctx, m1->wrapped, m2->wrapped);
                });
            return result;
            // both are proxy
        } else {
            // only 1st is proxy
            TypePtr und = p1->underlying();
            return isSubTypeUnderConstraintSingle(ctx, constr, mode, und, t2);
        }
    } else if (isa_type<ProxyType>(t2.get())) {
        // non-proxies are never subtypes of proxies.
        return false;
    } else {
        if (auto *c1 = cast_type<ClassType>(t1.get())) {
            if (auto *c2 = cast_type<ClassType>(t2.get())) {
                return classSymbolIsAsGoodAs(ctx, c1->symbol, c2->symbol);
            }
        }
        Exception::raise("isSubTypeUnderConstraint({}, {}): unreachable", t1->typeName(), t2->typeName());
    }
}

bool Types::isSubTypeUnderConstraint(Context ctx, TypeConstraint &constr, UntypedMode mode, const TypePtr &t1,
                                     const TypePtr &t2) {
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
    if (auto *o1 = cast_type<OrType>(t1.get())) { // 7, 8, 9
        return Types::isSubTypeUnderConstraint(ctx, constr, mode, o1->left, t2) &&
               Types::isSubTypeUnderConstraint(ctx, constr, mode, o1->right, t2);
    }

    if (auto *a2 = cast_type<AndType>(t2.get())) { // 2, 5
        return Types::isSubTypeUnderConstraint(ctx, constr, mode, t1, a2->left) &&
               Types::isSubTypeUnderConstraint(ctx, constr, mode, t1, a2->right);
    }

    auto *a1 = cast_type<AndType>(t1.get());
    auto *o2 = cast_type<OrType>(t2.get());

    if (a1 != nullptr) {
        // If the left is an And of an Or, then we can reorder it to be an Or of
        // an And, which lets us recurse on smaller types
        auto l = a1->left;
        auto r = a1->right;
        if (isa_type<OrType>(r.get())) {
            swap(r, l);
        }
        auto *a2o = cast_type<OrType>(l.get());
        if (a2o != nullptr) {
            // This handles `(A | B) & C` -> `(A & C) | (B & C)`
            return Types::isSubTypeUnderConstraint(ctx, constr, mode, glb(ctx, a2o->left, r), t2) &&
                   Types::isSubTypeUnderConstraint(ctx, constr, mode, glb(ctx, a2o->right, r), t2);
        }
    }
    if (o2 != nullptr) {
        // Simiarly to above, if the right is an Or of an And, then we can reorder it to be an And of
        // an Or, which lets us recurse on smaller types
        auto l = o2->left;
        auto r = o2->right;
        if (isa_type<AndType>(r.get())) {
            swap(r, l);
        }
        auto *o2a = cast_type<AndType>(l.get());
        if (o2a != nullptr) {
            // This handles `(A & B) | C` -> `(A | C) & (B | C)`
            return Types::isSubTypeUnderConstraint(ctx, constr, mode, t1, lub(ctx, o2a->left, r)) &&
                   Types::isSubTypeUnderConstraint(ctx, constr, mode, t1, lub(ctx, o2a->right, r));
        }
    }

    // This order matters
    if (o2 != nullptr) { // 3
        return Types::isSubTypeUnderConstraint(ctx, constr, mode, t1, o2->left) ||
               Types::isSubTypeUnderConstraint(ctx, constr, mode, t1, o2->right);
    }
    if (a1 != nullptr) { // 4
        return Types::isSubTypeUnderConstraint(ctx, constr, mode, a1->left, t2) ||
               Types::isSubTypeUnderConstraint(ctx, constr, mode, a1->right, t2);
    }

    return isSubTypeUnderConstraintSingle(ctx, constr, mode, t1, t2); // 1
}

bool Types::equiv(Context ctx, const TypePtr &t1, const TypePtr &t2) {
    return isSubType(ctx, t1, t2) && isSubType(ctx, t2, t1);
}

bool Types::equivNoUntyped(Context ctx, const TypePtr &t1, const TypePtr &t2) {
    return isAsSpecificAs(ctx, t1, t2) && isAsSpecificAs(ctx, t2, t1);
}

bool ProxyType::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    return underlying()->derivesFrom(gs, klass);
}

bool ClassType::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    if (symbol == Symbols::untyped() || symbol == klass) {
        return true;
    }
    return symbol.data(gs)->derivesFrom(gs, klass);
}

bool OrType::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    return left->derivesFrom(gs, klass) && right->derivesFrom(gs, klass);
}

bool AndType::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    return left->derivesFrom(gs, klass) || right->derivesFrom(gs, klass);
}

bool AliasType::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    Exception::raise("AliasType.derivesfrom");
}

void AliasType::_sanityCheck(Context ctx) {
    ENFORCE(this->symbol.exists());
}

TypePtr AliasType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params, const vector<TypePtr> &targs) {
    Exception::raise("should never happen");
}

string MetaType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return "MetaType";
}

string MetaType::show(const GlobalState &gs) const {
    return "<Type: " + wrapped->show(gs) + ">";
}

string MetaType::typeName() const {
    return "MetaType";
}

void MetaType::_sanityCheck(Context ctx) {
    ENFORCE(!core::isa_type<MetaType>(wrapped.get()));
    this->wrapped->sanityCheck(ctx);
}

bool MetaType::isFullyDefined() {
    return true; // this is kinda true but kinda false. it's false for subtyping but true for inferencer.
}

bool MetaType::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    return false;
}

TypePtr MetaType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params, const vector<TypePtr> &targs) {
    Exception::raise("should never happen");
}

MetaType::MetaType(const TypePtr &wrapped) : wrapped(move(wrapped)) {
    categoryCounterInc("types.allocated", "metattype");
}

TypePtr MetaType::_approximate(Context ctx, const TypeConstraint &tc) {
    // dispatchCall is invoked on them in resolver
    return nullptr;
}

TypePtr MetaType::underlying() const {
    return Types::Object();
}

} // namespace sorbet::core
