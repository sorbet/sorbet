#include "common/common.h"
#include "common/typecase.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"
#include <algorithm> // find_if
#include <utility>

namespace sorbet::core {

using namespace std;

namespace {
bool compositeTypeDeepRefEqual(const OrType &o1, const OrType &o2);
bool compositeTypeDeepRefEqual(const AndType &a1, const AndType &a2);
bool compositeTypeDeepRefEqualHelper(const TypePtr &t1, const TypePtr &t2) {
    if (t1 == t2) {
        return true;
    }
    if (t1.tag() != t2.tag()) {
        return false;
    }
    // t1 and t2 are the same kind of type.
    if (isa_type<OrType>(t1)) {
        return compositeTypeDeepRefEqual(cast_type_nonnull<OrType>(t1), cast_type_nonnull<OrType>(t2));
    }
    if (isa_type<AndType>(t1)) {
        return compositeTypeDeepRefEqual(cast_type_nonnull<AndType>(t1), cast_type_nonnull<AndType>(t2));
    }
    return false;
}

// Returns 'true' if the tree of types stemming from this AndType are referentially equal.
bool compositeTypeDeepRefEqual(const AndType &a1, const AndType &a2) {
    return compositeTypeDeepRefEqualHelper(a1.left, a2.left) && compositeTypeDeepRefEqualHelper(a1.right, a2.right);
}

// Returns 'true' if the tree of types stemming from this OrType are referentially equal.
bool compositeTypeDeepRefEqual(const OrType &o1, const OrType &o2) {
    return compositeTypeDeepRefEqualHelper(o1.left, o2.left) && compositeTypeDeepRefEqualHelper(o1.right, o2.right);
}
} // namespace

TypePtr lubGround(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);

TypePtr Types::any(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    auto ret = lub(gs, t1, t2);
    SLOW_ENFORCE(Types::isSubType(gs, t1, ret), "\n{}\nis not a super type of\n{}\nwas lubbing with {}",
                 ret.toString(gs), t1.toString(gs), t2.toString(gs));
    SLOW_ENFORCE(Types::isSubType(gs, t2, ret), "\n{}\nis not a super type of\n{}\nwas lubbing with {}",
                 ret.toString(gs), t2.toString(gs), t1.toString(gs));

    //  TODO: @dmitry, reenable
    //    ENFORCE(t1->hasUntyped() || t2->hasUntyped() || ret->hasUntyped() || // check if this test makes sense
    //                !Types::isSubTypeUnderConstraint(gs, t2, t1) || ret == t1 || ret->isUntyped(),
    //            "we do pointer comparisons in order to see if one is subtype of another. " + t1->toString(gs) +
    //
    //                " was lubbing with " + t2->toString(gs) + " got " + ret->toString(gs));
    //
    //    ENFORCE(t1->hasUntyped() || t2->hasUntyped() || ret->hasUntyped() || // check if this test makes sense!
    //                !Types::isSubTypeUnderConstraint(gs, t1, t2) || ret == t2 || ret->isUntyped() || ret == t1 ||
    //                Types::isSubTypeUnderConstraint(gs, t2, t1),
    //            "we do pointer comparisons in order to see if one is subtype of another " + t1->toString(gs) +
    //                " was lubbing with " + t2->toString(gs) + " got " + ret->toString(gs));

    ret.sanityCheck(gs);

    return ret;
}

const TypePtr underlying(const GlobalState &gs, const TypePtr &t1) {
    if (is_proxy_type(t1)) {
        return t1.underlying(gs);
    }
    return t1;
}

void fillInOrComponents(InlinedVector<TypePtr, 4> &orComponents, const TypePtr &type) {
    auto *o = cast_type<OrType>(type);
    if (o == nullptr) {
        orComponents.emplace_back(type);
    } else {
        fillInOrComponents(orComponents, o->left);
        fillInOrComponents(orComponents, o->right);
    }
}

TypePtr filterOrComponents(const TypePtr &originalType, const InlinedVector<TypePtr, 4> &typeFilter) {
    auto *o = cast_type<OrType>(originalType);
    if (o == nullptr) {
        if (absl::c_linear_search(typeFilter, originalType)) {
            return nullptr;
        }
        return originalType;
    } else {
        auto left = filterOrComponents(o->left, typeFilter);
        auto right = filterOrComponents(o->right, typeFilter);
        if (left == nullptr) {
            return right;
        }
        if (right == nullptr) {
            return left;
        }
        if (left == o->left && right == o->right) {
            return originalType;
        }
        return OrType::make_shared(move(left), move(right));
    }
}

TypePtr lubDistributeOr(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    InlinedVector<TypePtr, 4> originalOrComponents;
    InlinedVector<TypePtr, 4> typesConsumed;
    auto *o1 = cast_type<OrType>(t1);
    ENFORCE(o1 != nullptr);
    fillInOrComponents(originalOrComponents, o1->left);
    fillInOrComponents(originalOrComponents, o1->right);

    for (auto &component : originalOrComponents) {
        auto lubbed = Types::any(gs, component, t2);
        if (lubbed == component) {
            // lubbed == component, so t2 <: component and t2 <: t1
            categoryCounterInc("lubDistributeOr.outcome", "t1");
            return t1;
        } else if (lubbed == t2) {
            // lubbed == t2, so component <: t2
            // Thus, we don't need to include component in the final OrType; it's subsumed by t2.
            typesConsumed.emplace_back(component);
        }
    }
    if (typesConsumed.empty()) {
        // t1 has no components that overlap with t2
        categoryCounterInc("lubDistributeOr.outcome", "worst");
        return OrType::make_shared(t1, underlying(gs, t2));
    }
    // lub back everything except typesConsumed
    auto remainingTypes = filterOrComponents(t1, typesConsumed);
    if (remainingTypes == nullptr) {
        categoryCounterInc("lubDistributeOr.outcome", "t2");
        // t1 <: t2
        return t2;
    }
    categoryCounterInc("lubDistributeOr.outcome", "consumedComponent");
    return OrType::make_shared(move(remainingTypes), underlying(gs, t2));
}

TypePtr glbDistributeAnd(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    auto *a1 = cast_type<AndType>(t1);
    ENFORCE(t1 != nullptr);
    TypePtr n1 = Types::all(gs, a1->left, t2);
    if (n1 == a1->left) {
        categoryCounterInc("lubDistributeOr.outcome", "t1");
        return t1;
    }
    TypePtr n2 = Types::all(gs, a1->right, t2);
    if (n1 == t2) {
        categoryCounterInc("glbDistributeAnd.outcome", "Zn2");
        return n2;
    }
    if (n2 == a1->right) {
        categoryCounterInc("glbDistributeAnd.outcome", "Zt1");
        return t1;
    }
    if (n2 == t2) {
        categoryCounterInc("glbDistributeAnd.outcome", "Zn1");
        return n1;
    }
    if (Types::isSubType(gs, n1, n2)) {
        categoryCounterInc("glbDistributeAnd.outcome", "ZZn1");
        return n1;
    } else if (Types::isSubType(gs, n2, n1)) {
        categoryCounterInc("glbDistributeAnd.outcome", "ZZZn2");
        return n2;
    }

    categoryCounterInc("glbDistributeAnd.outcome", "worst");
    return AndType::make_shared(t1, t2);
}

// only keep knowledge in t1 that is not already present in t2. Return the same reference if unchaged
TypePtr dropLubComponents(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    if (auto *a1 = cast_type<AndType>(t1)) {
        auto a1a = dropLubComponents(gs, a1->left, t2);
        auto a1b = dropLubComponents(gs, a1->right, t2);
        auto subl = Types::isSubType(gs, a1a, t2);
        auto subr = Types::isSubType(gs, a1b, t2);
        if (subl || subr) {
            return Types::bottom();
        }
        if (a1a != a1->left || a1b != a1->right) {
            return Types::all(gs, a1a, a1b);
        }
    } else if (auto *o1 = cast_type<OrType>(t1)) {
        auto subl = Types::isSubType(gs, o1->left, t2);
        auto subr = Types::isSubType(gs, o1->right, t2);
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

TypePtr Types::lub(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    if (t1 == t2) {
        categoryCounterInc("lub", "ref-eq");
        return t1;
    }

    if (t1.kind() > t2.kind()) { // force the relation to be symmentric and half the implementation
        return lub(gs, t2, t1);
    }

    if (isa_type<ClassType>(t1)) {
        auto mayBeSpecial1 = cast_type_nonnull<ClassType>(t1);
        if (mayBeSpecial1.symbol == Symbols::untyped()) {
            categoryCounterInc("lub", "<untyped");
            return t1;
        }
        if (mayBeSpecial1.symbol == Symbols::bottom()) {
            categoryCounterInc("lub", "<bottom");
            return t2;
        }
        if (mayBeSpecial1.symbol == Symbols::top()) {
            categoryCounterInc("lub", "<top");
            return t1;
        }
    }

    if (isa_type<ClassType>(t2)) {
        auto mayBeSpecial2 = cast_type_nonnull<ClassType>(t2);
        if (mayBeSpecial2.symbol == Symbols::untyped()) {
            categoryCounterInc("lub", "untyped>");
            return t2;
        }
        if (mayBeSpecial2.symbol == Symbols::bottom()) {
            categoryCounterInc("lub", "bottom>");
            return t1;
        }
        if (mayBeSpecial2.symbol == Symbols::top()) {
            categoryCounterInc("lub", "top>");
            return t2;
        }
    }

    if (auto *o2 = cast_type<OrType>(t2)) { // 3, 5, 6
        categoryCounterInc("lub", "or>");
        return lubDistributeOr(gs, t2, t1);
    } else if (auto *a2 = cast_type<AndType>(t2)) { // 2, 4
        if (auto *a1 = cast_type<AndType>(t1)) {
            // Check if the members of a1 and a2 are referentially equivalent. This helps simplify T.all types created
            // during type inference.
            if (compositeTypeDeepRefEqual(*a1, *a2)) {
                categoryCounterInc("lub", "<and>");
                return t2;
            }
        }

        categoryCounterInc("lub", "and>");
        auto t1d = underlying(gs, t1);
        auto t2filtered = dropLubComponents(gs, t2, t1d);
        if (t2filtered != t2) {
            return lub(gs, t1d, t2filtered);
        }
        if (isa_type<OrType>(t1)) {
            return lubDistributeOr(gs, t1, t2);
        }
        return OrType::make_shared(t1, t2filtered);
    } else if (isa_type<OrType>(t1)) {
        categoryCounterInc("lub", "<or");
        return lubDistributeOr(gs, t1, t2);
    }

    if (auto *a1 = cast_type<AppliedType>(t1)) {
        auto *a2 = cast_type<AppliedType>(t2);
        if (a2 == nullptr) {
            if (isSubType(gs, t2, t1)) {
                return t1;
            }
            if (isSubType(gs, t1, t2)) {
                return t2;
            }
            return OrType::make_shared(t1, t2);
        }

        bool rtl = a1->klass == a2->klass || a1->klass.data(gs)->derivesFrom(gs, a2->klass);
        bool ltr = !rtl && a2->klass.data(gs)->derivesFrom(gs, a1->klass);
        if (!rtl && !ltr) {
            return OrType::make_shared(t1, t2);
        }
        if (ltr) {
            swap(a1, a2);
        }
        const TypePtr &t1s = ltr ? t2 : t1;
        const TypePtr &t2s = ltr ? t1 : t2;
        // now a1 <: a2

        InlinedVector<TypeMemberRef, 4> indexes = Types::alignBaseTypeArgs(gs, a1->klass, a1->targs, a2->klass);
        vector<TypePtr> newTargs;
        newTargs.reserve(indexes.size());
        // code below inverts permutation of type params
        int j = 0;
        bool changedFromT2 = false;
        // If klasses are equal, then it's possible that t1s <: t2s.
        bool changedFromT1 = a1->klass != a2->klass;
        for (SymbolRef idx : a2->klass.data(gs)->typeMembers()) {
            TypeMemberRef idxTypeMember = idx.asTypeMemberRef();
            int i = 0;
            while (indexes[j] != a1->klass.data(gs)->typeMembers()[i]) {
                i++;
            }
            ENFORCE(i < a1->klass.data(gs)->typeMembers().size());
            if (idxTypeMember.data(gs)->flags.isCovariant) {
                newTargs.emplace_back(Types::any(gs, a1->targs[i], a2->targs[j]));
            } else if (idxTypeMember.data(gs)->flags.isInvariant) {
                if (!Types::equiv(gs, a1->targs[i], a2->targs[j])) {
                    return OrType::make_shared(t1s, t2s);
                }
                // We don't need to check the idxTypeMember upper/lower bounds like the corresponding case in glb
                // because it's a2->targs[j] is already within the bounds of idxTypeMember (or an error was reported
                // already), and a1->targs[i] is untyped so it trivially matches all bounds.
                if (a1->targs[i].isUntyped()) {
                    newTargs.emplace_back(a1->targs[i]);
                } else {
                    newTargs.emplace_back(a2->targs[j]);
                }

            } else if (idxTypeMember.data(gs)->flags.isContravariant) {
                newTargs.emplace_back(Types::all(gs, a1->targs[i], a2->targs[j]));
            }
            changedFromT2 = changedFromT2 || newTargs.back() != a2->targs[j];
            changedFromT1 = changedFromT1 || newTargs.back() != a1->targs[i];
            j++;
        }
        if (!changedFromT2) {
            return t2s;
        } else if (!changedFromT1) {
            return t1s;
        } else {
            return make_type<AppliedType>(a2->klass, move(newTargs));
        }
    }

    if (is_proxy_type(t1)) {
        categoryCounterInc("lub", "<proxy");
        if (is_proxy_type(t2)) {
            categoryCounterInc("lub", "proxy>");
            // both are proxy
            TypePtr result;
            typecase(
                t1,
                [&](const TupleType &a1) { // Warning: this implements COVARIANT arrays
                    if (auto *a2 = cast_type<TupleType>(t2)) {
                        if (a1.elems.size() == a2->elems.size()) { // lub arrays only if they have same element count
                            vector<TypePtr> elemLubs;
                            int i = -1;
                            bool differ1 = false;
                            bool differ2 = false;
                            for (auto &el2 : a2->elems) {
                                ++i;
                                auto &inserted = elemLubs.emplace_back(lub(gs, a1.elems[i], el2));
                                differ1 = differ1 || inserted != a1.elems[i];
                                differ2 = differ2 || inserted != el2;
                            }
                            if (!differ1) {
                                result = t1;
                            } else if (!differ2) {
                                result = t2;
                            } else {
                                result = make_type<TupleType>(move(elemLubs));
                            }
                        } else {
                            result = Types::arrayOfUntyped(Symbols::Magic_UntypedSource_tupleLub());
                        }
                    } else {
                        result = lub(gs, a1.underlying(gs), t2.underlying(gs));
                    }
                },
                [&](const ShapeType &h1) { // Warning: this implements COVARIANT hashes
                    if (auto *h2 = cast_type<ShapeType>(t2)) {
                        if (h2->keys.size() == h1.keys.size()) {
                            // have enough keys.
                            int i = -1;
                            vector<TypePtr> valueLubs;
                            valueLubs.reserve(h2->keys.size());
                            bool differ1 = false;
                            bool differ2 = false;
                            for (auto &el2 : h2->keys) {
                                ++i;
                                auto optind = h1.indexForKey(el2);
                                if (!optind.has_value()) {
                                    result = Types::hashOfUntyped(Symbols::Magic_UntypedSource_shapeLub());
                                    return;
                                }
                                auto &inserted =
                                    valueLubs.emplace_back(lub(gs, h1.values[optind.value()], h2->values[i]));
                                differ1 = differ1 || inserted != h1.values[optind.value()];
                                differ2 = differ2 || inserted != h2->values[i];
                            }
                            if (!differ1) {
                                result = t1;
                            } else if (!differ2) {
                                result = t2;
                            } else {
                                result = make_type<ShapeType>(h2->keys, move(valueLubs));
                            }
                        } else {
                            result = Types::hashOfUntyped(Symbols::Magic_UntypedSource_shapeLub());
                        }
                    } else {
                        bool allowProxyInLub = isa_type<TupleType>(t2);
                        if (allowProxyInLub) {
                            result = OrType::make_shared(t1, t2);
                        } else {
                            result = lub(gs, h1.underlying(gs), t2.underlying(gs));
                        }
                    }
                },
                [&](const NamedLiteralType &l1) {
                    if (isa_type<NamedLiteralType>(t2)) {
                        auto l2 = cast_type_nonnull<NamedLiteralType>(t2);
                        auto underlyingL1 = l1.underlying(gs);
                        auto underlyingL2 = l2.underlying(gs);
                        auto class1 = cast_type_nonnull<ClassType>(underlyingL1);
                        auto class2 = cast_type_nonnull<ClassType>(underlyingL2);
                        if (class1.symbol == class2.symbol) {
                            if (l1.equals(l2)) {
                                result = t1;
                            } else {
                                result = l1.underlying(gs);
                            }
                        } else {
                            result = lubGround(gs, l1.underlying(gs), l2.underlying(gs));
                        }
                    } else {
                        result = lub(gs, l1.underlying(gs), t2.underlying(gs));
                    }
                },
                [&](const IntegerLiteralType &l1) {
                    if (isa_type<IntegerLiteralType>(t2)) {
                        auto &l2 = cast_type_nonnull<IntegerLiteralType>(t2);
                        if (l1.equals(l2)) {
                            result = t1;
                        } else {
                            result = l1.underlying(gs);
                        }
                    } else {
                        result = lub(gs, l1.underlying(gs), t2.underlying(gs));
                    }
                },
                [&](const FloatLiteralType &l1) {
                    if (isa_type<FloatLiteralType>(t2)) {
                        auto &l2 = cast_type_nonnull<FloatLiteralType>(t2);
                        if (l1.equals(l2)) {
                            result = t1;
                        } else {
                            result = l1.underlying(gs);
                        }
                    } else {
                        result = lub(gs, l1.underlying(gs), t2.underlying(gs));
                    }
                },
                [&](const MetaType &m1) {
                    if (auto *m2 = cast_type<MetaType>(t2)) {
                        if (Types::equiv(gs, m1.wrapped, m2->wrapped)) {
                            result = t1;
                            return;
                        }
                    }
                    result = lub(gs, m1.underlying(gs), t2.underlying(gs));
                });
            ENFORCE(result != nullptr);
            return result;
        } else {
            bool allowProxyInLub = isa_type<TupleType>(t1) || isa_type<ShapeType>(t1);
            // only 1st is proxy
            TypePtr und = t1.underlying(gs);
            if (isSubType(gs, und, t2)) {
                return t2;
            } else if (allowProxyInLub) {
                return OrType::make_shared(t1, t2);
            } else {
                return lub(gs, t2, und);
            }
        }
    } else if (is_proxy_type(t2)) {
        // only 2nd is proxy
        bool allowProxyInLub = isa_type<TupleType>(t2) || isa_type<ShapeType>(t2);
        // only 1st is proxy
        TypePtr und = t2.underlying(gs);
        if (isSubType(gs, und, t1)) {
            return t1;
        } else if (allowProxyInLub) {
            return OrType::make_shared(t1, t2);
        } else {
            return lub(gs, t1, und);
        }
    }

    {
        if (isa_type<LambdaParam>(t1) || isa_type<LambdaParam>(t2)) {
            return OrType::make_shared(t1, t2);
        }
    }

    {
        if (isa_type<TypeVar>(t1) || isa_type<TypeVar>(t2)) {
            return OrType::make_shared(t1, t2);
        }
    }

    {
        auto isSelfTypeT1 = isa_type<SelfTypeParam>(t1);
        auto isSelfTypeT2 = isa_type<SelfTypeParam>(t2);

        if (isSelfTypeT1 || isSelfTypeT2) {
            // NOTE: SelfTypeParam is an inlined type, so TypePtr equality is type equality.
            if (t1 != t2) {
                return OrType::make_shared(t1, t2);
            } else {
                return t1;
            }
        }
    }

    {
        if (isa_type<SelfType>(t1) || isa_type<SelfType>(t2)) {
            // NOTE: SelfType is an inlined type, so TypePtr equality is type equality.
            if (t1 != t2) {
                return OrType::make_shared(t1, t2);
            } else {
                return t1;
            }
        }
    }

    // none is proxy
    return lubGround(gs, t1, t2);
}

TypePtr lubGround(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    ENFORCE(is_ground_type(t1));
    ENFORCE(is_ground_type(t2));

    //    if (g1->kind() > g2->kind()) { // force the relation to be symmentric and half the implementation
    //        return lubGround(gs, t2, t1);
    //    }
    /** this implementation makes a bet that types are small and very likely to be collapsable.
     * The more complex types we have, the more likely this bet is to be wrong.
     */
    if (t1 == t2) {
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
    auto c1 = cast_type_nonnull<ClassType>(t1);
    auto c2 = cast_type_nonnull<ClassType>(t2);
    categoryCounterInc("lub", "<class>");

    auto sym1 = c1.symbol;
    auto sym2 = c2.symbol;
    if (sym1 == sym2 || sym2.data(gs)->derivesFrom(gs, sym1)) {
        categoryCounterInc("lub.<class>.collapsed", "yes");
        return t1;
    } else if (sym1.data(gs)->derivesFrom(gs, sym2)) {
        categoryCounterInc("lub.<class>.collapsed", "yes");
        return t2;
    } else {
        categoryCounterInc("lub.<class>.collapsed", "no");
        return OrType::make_shared(t1, t2);
    }
}

TypePtr glbGround(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    ENFORCE(is_ground_type(t1));
    ENFORCE(is_ground_type(t2));

    if (t1.kind() > t1.kind()) { // force the relation to be symmentric and half the implementation
        return glbGround(gs, t2, t1);
    }
    /** this implementation makes a bet that types are small and very likely to be collapsable.
     * The more complex types we have, the more likely this bet is to be wrong.
     */
    if (t1 == t2) {
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
    auto c1 = cast_type_nonnull<ClassType>(t1);
    auto c2 = cast_type_nonnull<ClassType>(t2);
    categoryCounterInc("glb", "<class>");

    auto sym1 = c1.symbol;
    auto sym2 = c2.symbol;
    if (sym1 == sym2 || sym1.data(gs)->derivesFrom(gs, sym2)) {
        categoryCounterInc("glb.<class>.collapsed", "yes");
        return t1;
    } else if (sym2.data(gs)->derivesFrom(gs, sym1)) {
        categoryCounterInc("glb.<class>.collapsed", "yes");
        return t2;
    } else {
        if (sym1.data(gs)->isClass() && sym2.data(gs)->isClass()) {
            categoryCounterInc("glb.<class>.collapsed", "bottom");
            return Types::bottom();
        } else if (sym1.data(gs)->flags.isFinal || sym2.data(gs)->flags.isFinal) {
            // If at least one of them is a module, the only way this type could ever be inhabited
            // is if a descendant of one of the symbols later includes the module symbol. This can't
            // happen if either one is final.
            return Types::bottom();
        }
        categoryCounterInc("glb.<class>.collapsed", "no");
        return AndType::make_shared(t1, t2);
    }
}
TypePtr Types::all(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    auto ret = glb(gs, t1, t2);
    ret.sanityCheck(gs);

    SLOW_ENFORCE(Types::isSubType(gs, ret, t1), "\n{}\nis not a subtype of\n{}\nwas glbbing with\n{}", ret.toString(gs),
                 t1.toString(gs), t2.toString(gs));

    SLOW_ENFORCE(Types::isSubType(gs, ret, t2), "\n{}\n is not a subtype of\n{}\nwas glbbing with\n{}",
                 ret.toString(gs), t2.toString(gs), t1.toString(gs));
    //  TODO: @dmitry, reenable
    //    ENFORCE(t1->hasUntyped() || t2->hasUntyped() || ret->hasUntyped() || // check if this test makes sense
    //                !Types::isSubTypeUnderConstraint(gs, t1, t2) || ret == t1 || ret->isUntyped(),
    //            "we do pointer comparisons in order to see if one is subtype of another. " + t1->toString(gs) +
    //
    //                " was glbbing with " + t2->toString(gs) + " got " + ret->toString(gs));
    //
    //    ENFORCE(t1->hasUntyped() || t2->hasUntyped() || ret->hasUntyped() || // check if this test makes sense
    //                !Types::isSubTypeUnderConstraint(gs, t2, t1) || ret == t2 || ret->isUntyped() || ret == t1 ||
    //                Types::isSubTypeUnderConstraint(gs, t1, t2),
    //            "we do pointer comparisons in order to see if one is subtype of another " + t1->toString(gs) +
    //                " was glbbing with " + t2->toString(gs) + " got " + ret->toString(gs));

    return ret;
}

TypePtr Types::glb(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    if (t1 == t2) {
        categoryCounterInc("glb", "ref-eq");
        return t1;
    }

    if (isa_type<ClassType>(t1)) {
        auto mayBeSpecial1 = cast_type_nonnull<ClassType>(t1);
        if (mayBeSpecial1.symbol == Symbols::top()) {
            categoryCounterInc("glb", "<top");
            return t2;
        }
        if (mayBeSpecial1.symbol == Symbols::untyped()) {
            // This case is to prefer `T.untyped` to top, so that
            // `glb(T.untyped,<any>)` will reduce to `T.untyped`.
            if (isa_type<ClassType>(t2)) {
                auto mayBeSpecial2 = cast_type_nonnull<ClassType>(t2);
                if (mayBeSpecial2.symbol == Symbols::top()) {
                    categoryCounterInc("glb", "top>");
                    return t1;
                }
            }
            categoryCounterInc("glb", "<untyped");
            return t2;
        }
        if (mayBeSpecial1.symbol == Symbols::bottom()) {
            categoryCounterInc("glb", "<bottom");
            return t1;
        }
    }

    if (isa_type<ClassType>(t2)) {
        auto mayBeSpecial2 = cast_type_nonnull<ClassType>(t2);
        if (mayBeSpecial2.symbol == Symbols::top()) {
            categoryCounterInc("glb", "top>");
            return t1;
        }
        if (mayBeSpecial2.symbol == Symbols::untyped()) {
            categoryCounterInc("glb", "untyped>");
            return t1;
        }
        if (mayBeSpecial2.symbol == Symbols::bottom()) {
            categoryCounterInc("glb", "bottom>");
            return t2;
        }
    }

    if (t1.kind() > t2.kind()) { // force the relation to be symmentric and half the implementation
        return glb(gs, t2, t1);
    }
    if (isa_type<AndType>(t1)) { // 4, 5
        categoryCounterInc("glb", "<and");
        return glbDistributeAnd(gs, t1, t2);
    } else if (isa_type<AndType>(t2)) { // 2
        categoryCounterInc("glb", "and>");
        return glbDistributeAnd(gs, t2, t1);
    }

    if (is_proxy_type(t1)) {
        if (is_proxy_type(t2)) {
            if (t1.tag() != t2.tag()) {
                return Types::bottom();
            }
            TypePtr result;
            typecase(
                t1,
                [&](const TupleType &a1) { // Warning: this implements COVARIANT arrays
                    auto &a2 = cast_type_nonnull<TupleType>(t2);
                    if (a1.elems.size() == a2.elems.size()) { // lub arrays only if they have same element count
                        vector<TypePtr> elemGlbs;
                        elemGlbs.reserve(a2.elems.size());

                        int i = -1;
                        for (auto &el2 : a2.elems) {
                            ++i;
                            auto glbe = glb(gs, a1.elems[i], el2);
                            if (glbe.isBottom()) {
                                result = Types::bottom();
                                return;
                            }
                            elemGlbs.emplace_back(glbe);
                        }
                        if (absl::c_equal(a1.elems, elemGlbs)) {
                            result = t1;
                        } else if (absl::c_equal(a2.elems, elemGlbs)) {
                            result = t2;
                        } else {
                            result = make_type<TupleType>(move(elemGlbs));
                        }
                    } else {
                        result = Types::bottom();
                    }

                },
                [&](const ShapeType &h1) { // Warning: this implements COVARIANT hashes
                    auto &h2 = cast_type_nonnull<ShapeType>(t2);
                    if (h2.keys.size() == h1.keys.size()) {
                        // have enough keys.
                        int i = -1;
                        vector<TypePtr> valueLubs;
                        valueLubs.reserve(h2.keys.size());
                        bool canReuseT1 = true;
                        bool canReuseT2 = true;
                        for (auto &el2 : h2.keys) {
                            ++i;
                            auto optind = h1.indexForKey(el2);
                            if (!optind.has_value()) {
                                result = Types::bottom();
                                return;
                            }
                            auto &left = h1.values[optind.value()];
                            auto &right = h2.values[i];
                            auto glbe = glb(gs, left, right);
                            if (glbe.isBottom()) {
                                result = Types::bottom();
                                return;
                            }
                            canReuseT1 &= glbe == left;
                            canReuseT2 &= glbe == right;
                            valueLubs.emplace_back(glbe);
                        }
                        if (canReuseT1) {
                            result = t1;
                        } else if (canReuseT2) {
                            result = t2;
                        } else {
                            result = make_type<ShapeType>(h2.keys, move(valueLubs));
                        }
                    } else {
                        result = Types::bottom();
                    }

                },
                [&](const NamedLiteralType &l1) {
                    auto l2 = cast_type_nonnull<NamedLiteralType>(t2);
                    auto underlyingL1 = l1.underlying(gs);
                    auto underlyingL2 = l2.underlying(gs);
                    auto class1 = cast_type_nonnull<ClassType>(underlyingL1);
                    auto class2 = cast_type_nonnull<ClassType>(underlyingL2);
                    if (class1.symbol == class2.symbol) {
                        if (l1.equals(l2)) {
                            result = t1;
                        } else {
                            result = Types::bottom();
                        }
                    } else {
                        result = Types::bottom();
                    }
                },
                [&](const IntegerLiteralType &l1) {
                    auto &l2 = cast_type_nonnull<IntegerLiteralType>(t2);
                    if (l1.equals(l2)) {
                        result = t1;
                    } else {
                        result = Types::bottom();
                    }
                },
                [&](const FloatLiteralType &l1) {
                    auto &l2 = cast_type_nonnull<FloatLiteralType>(t2);
                    if (l1.equals(l2)) {
                        result = t1;
                    } else {
                        result = Types::bottom();
                    }
                },
                [&](const MetaType &m1) {
                    auto *m2 = cast_type<MetaType>(t2);
                    ENFORCE(m2 != nullptr);
                    if (Types::equiv(gs, m1.wrapped, m2->wrapped)) {
                        result = t1;
                    } else {
                        result = Types::bottom();
                    }
                });
            ENFORCE(result != nullptr);
            return result;
        } else {
            // only 1st is proxy
            if (Types::isSubType(gs, t1, t2)) {
                return t1;
            } else {
                return Types::bottom();
            }
        }
    } else if (is_proxy_type(t2)) {
        // only 1st is proxy
        if (Types::isSubType(gs, t2, t1)) {
            return t2;
        } else {
            return Types::bottom();
        }
    }

    if (auto *o2 = cast_type<OrType>(t2)) { // 3, 6
        bool collapseInLeft = Types::isAsSpecificAs(gs, t1, t2);
        if (collapseInLeft) {
            categoryCounterInc("glb", "Zor");
            return t1;
        }

        bool collapseInRight = Types::isAsSpecificAs(gs, t2, t1);
        if (collapseInRight) {
            categoryCounterInc("glb", "ZZor");
            return t2;
        }

        if (isa_type<ClassType>(t1) || isa_type<AppliedType>(t1)) {
            auto lft = Types::all(gs, t1, o2->left);
            if (Types::isAsSpecificAs(gs, lft, o2->right) && !lft.isBottom()) {
                categoryCounterInc("glb", "ZZZorClass");
                return lft;
            }
            auto rght = Types::all(gs, t1, o2->right);
            if (Types::isAsSpecificAs(gs, rght, o2->left) && !rght.isBottom()) {
                categoryCounterInc("glb", "ZZZZorClass");
                return rght;
            }
            if (lft.isBottom()) {
                return rght;
            }
            if (rght.isBottom()) {
                return lft;
            }
        }

        if (auto *o1 = cast_type<OrType>(t1)) { // 6
            auto t11 = Types::all(gs, o1->left, o2->left);
            auto t12 = Types::all(gs, o1->left, o2->right);
            auto t21 = Types::all(gs, o1->right, o2->left);
            auto t22 = Types::all(gs, o1->right, o2->right);

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
            if (t11.isBottom() || t12.isBottom() || t21.isBottom() || t22.isBottom()) {
                score++;
            }

            if (score > 0) {
                return Types::any(gs, Types::any(gs, t11, t12), Types::any(gs, t21, t22));
            }
        }
        categoryCounterInc("glb.orcollapsed", "no");
        return AndType::make_shared(t1, t2);
    }

    if (auto *a1 = cast_type<AppliedType>(t1)) {
        auto *a2 = cast_type<AppliedType>(t2);
        if (a2 == nullptr) {
            if (a1->klass.data(gs)->isModule() || !isa_type<ClassType>(t2)) {
                return AndType::make_shared(t1, t2);
            }
            auto c2 = cast_type_nonnull<ClassType>(t2);
            if (a1->klass.data(gs)->derivesFrom(gs, c2.symbol)) {
                return t1;
            }
            if (c2.symbol.data(gs)->isModule()) {
                return AndType::make_shared(t1, t2);
            }
            return Types::bottom();
        }
        bool rtl = a1->klass == a2->klass || a1->klass.data(gs)->derivesFrom(gs, a2->klass);
        bool ltr = !rtl && a2->klass.data(gs)->derivesFrom(gs, a1->klass);
        if (!rtl && !ltr) {
            if (a1->klass.data(gs)->isClass() && a2->klass.data(gs)->isClass()) {
                // At this point, the two types are both classes, and unrelated
                // to each other. Because ruby does not support multiple
                // inheritance, this type is empty.
                return Types::bottom();
            } else if (a1->klass.data(gs)->flags.isFinal || a2->klass.data(gs)->flags.isFinal) {
                // If at least one of them is a module, the only way this type could ever be
                // inhabited is if a descendant of one of the symbols later includes the module
                // symbol. This can't happen if either one is final.
                return Types::bottom();
            } else {
                return AndType::make_shared(t1, t2); // we can as well return nothing here?
            }
        }
        if (ltr) { // swap
            swap(a1, a2);
        }
        // a1 <:< a2

        InlinedVector<TypeMemberRef, 4> indexes = Types::alignBaseTypeArgs(gs, a2->klass, a2->targs, a1->klass);

        // code below inverts permutation of type params

        vector<TypePtr> newTargs;
        newTargs.reserve(a1->klass.data(gs)->typeMembers().size());
        int j = 0;
        for (auto idx : a1->klass.data(gs)->typeMembers()) {
            int i = 0;
            if (j >= indexes.size()) {
                i = INT_MAX;
            }
            while (i < a2->klass.data(gs)->typeMembers().size() && indexes[j] != a2->klass.data(gs)->typeMembers()[i]) {
                i++;
            }
            if (i >= a2->klass.data(gs)->typeMembers().size()) { // a1 has more tparams, this is fine, it's a child
                newTargs.emplace_back(a1->targs[j]);
            } else {
                auto a2TypeMember = a2->klass.data(gs)->typeMembers()[i];
                if (a2TypeMember.data(gs)->flags.isCovariant) {
                    newTargs.emplace_back(Types::all(gs, a1->targs[j], a2->targs[i]));
                } else if (a2TypeMember.data(gs)->flags.isInvariant) {
                    if (!Types::equiv(gs, a1->targs[j], a2->targs[i])) {
                        return AndType::make_shared(t1, t2);
                    }
                    const auto &lambdaParam = cast_type<LambdaParam>(idx.data(gs)->resultType);
                    if (a1->targs[j].isUntyped() && Types::isSubType(gs, lambdaParam->lowerBound, a2->targs[i]) &&
                        Types::isSubType(gs, a2->targs[i], lambdaParam->upperBound)) {
                        newTargs.emplace_back(a2->targs[i]);
                    } else {
                        newTargs.emplace_back(a1->targs[j]);
                    }
                } else if (a2TypeMember.data(gs)->flags.isContravariant) {
                    newTargs.emplace_back(Types::any(gs, a1->targs[j], a2->targs[i]));
                }
            }
            j++;
        }
        if (absl::c_equal(a1->targs, newTargs)) {
            return ltr ? t2 : t1;
        } else if (absl::c_equal(a2->targs, newTargs) && a1->klass == a2->klass) {
            return ltr ? t1 : t2;
        } else {
            return make_type<AppliedType>(a1->klass, move(newTargs));
        }
    }
    {
        if (isa_type<TypeVar>(t1) || isa_type<TypeVar>(t2)) {
            return AndType::make_shared(t1, t2);
        }
    }
    {
        auto isSelfTypeT1 = isa_type<SelfTypeParam>(t1);
        auto isSelfTypeT2 = isa_type<SelfTypeParam>(t2);

        if (isSelfTypeT1 || isSelfTypeT2) {
            // NOTE: SelfTypeParam is an inlined type, so TypePtr equality is type equality.
            if (t1 != t2) {
                return AndType::make_shared(t1, t2);
            } else {
                return t1;
            }
        }
    }

    {
        if (isa_type<SelfType>(t1) || isa_type<SelfType>(t2)) {
            // NOTE: SelfType is an ilined type, so TypePtr equality is type equality.
            if (t1 != t2) {
                return AndType::make_shared(t1, t2);
            } else {
                return t1;
            }
        }
    }
    return glbGround(gs, t1, t2);
}

bool classSymbolIsAsGoodAs(const GlobalState &gs, ClassOrModuleRef c1, ClassOrModuleRef c2) {
    return c1 == c2 || c1.data(gs)->derivesFrom(gs, c2);
}

void compareToUntyped(const GlobalState &gs, TypeConstraint &constr, const TypePtr &ty, const TypePtr &blame) {
    ENFORCE(blame.isUntyped());
    if (is_proxy_type(ty)) {
        compareToUntyped(gs, constr, ty.underlying(gs), blame);
    }

    if (auto *t = cast_type<AppliedType>(ty)) {
        for (auto &targ : t->targs) {
            compareToUntyped(gs, constr, targ, blame);
        }
    } else if (auto *t = cast_type<ShapeType>(ty)) {
        for (auto &val : t->values) {
            compareToUntyped(gs, constr, val, blame);
        }
    } else if (auto *t = cast_type<TupleType>(ty)) {
        for (auto &val : t->elems) {
            compareToUntyped(gs, constr, val, blame);
        }
    } else if (auto *t = cast_type<OrType>(ty)) {
        compareToUntyped(gs, constr, t->left, blame);
        compareToUntyped(gs, constr, t->right, blame);
    } else if (auto *t = cast_type<AndType>(ty)) {
        compareToUntyped(gs, constr, t->left, blame);
        compareToUntyped(gs, constr, t->right, blame);
    } else if (auto *t = cast_type<TypeVar>(ty)) {
        constr.rememberIsSubtype(gs, ty, blame);
        constr.rememberIsSubtype(gs, blame, ty);
    }
}

// "Single" means "ClassType or ProxyType"; since ProxyTypes are constrained to
// be proxies over class types, this means "class or class-like"
bool isSubTypeUnderConstraintSingle(const GlobalState &gs, TypeConstraint &constr, UntypedMode mode, const TypePtr &t1,
                                    const TypePtr &t2) {
    ENFORCE(t1 != nullptr);
    ENFORCE(t2 != nullptr);

    if (t1 == t2) {
        return true;
    }

    if (isa_type<TypeVar>(t1) || isa_type<TypeVar>(t2)) {
        if (constr.isSolved()) {
            return constr.isAlreadyASubType(gs, t1, t2);
        } else {
            return constr.rememberIsSubtype(gs, Types::dropLiteral(gs, t1), Types::dropLiteral(gs, t2));
        }
    }

    if (isa_type<ClassType>(t1)) {
        auto mayBeSpecial1 = cast_type_nonnull<ClassType>(t1);
        if (mayBeSpecial1.symbol == Symbols::untyped()) {
            if (!constr.isSolved()) {
                compareToUntyped(gs, constr, t2, t1);
            }
            return mode == UntypedMode::AlwaysCompatible;
        }
        if (mayBeSpecial1.symbol == Symbols::bottom()) {
            return true;
        }
        if (mayBeSpecial1.symbol == Symbols::top()) {
            if (isa_type<ClassType>(t2)) {
                auto mayBeSpecial2 = cast_type_nonnull<ClassType>(t2);
                return mayBeSpecial2.symbol == Symbols::top() || mayBeSpecial2.symbol == Symbols::untyped();
            } else {
                return false;
            }
        }
    }

    if (isa_type<ClassType>(t2)) {
        auto mayBeSpecial2 = cast_type_nonnull<ClassType>(t2);
        if (mayBeSpecial2.symbol == Symbols::untyped()) {
            if (!constr.isSolved()) {
                compareToUntyped(gs, constr, t1, t2);
            }
            return mode == UntypedMode::AlwaysCompatible;
        }
        if (mayBeSpecial2.symbol == Symbols::bottom()) {
            return false; // (bot, bot) is handled above.
        }
        if (mayBeSpecial2.symbol == Symbols::top()) {
            return true;
        }
    }
    //    ENFORCE(!isa_type<LambdaParam>(t1.get())); // sandly, this is false in Resolver, as we build
    //    original signatures using lub ENFORCE(cast_type<LambdaParam>(t2) == nullptr);

    {
        auto *lambda1 = cast_type<LambdaParam>(t1);
        auto *lambda2 = cast_type<LambdaParam>(t2);
        if (lambda1 != nullptr || lambda2 != nullptr) {
            // This should only be reachable in resolver.
            if (lambda1 == nullptr || lambda2 == nullptr) {
                return false;
            }
            return lambda1->definition == lambda2->definition;
        }
    }

    {
        auto isSelfTypeT1 = isa_type<SelfTypeParam>(t1);
        auto isSelfTypeT2 = isa_type<SelfTypeParam>(t2);
        if (isSelfTypeT1 || isSelfTypeT2) {
            // NOTE: SelfTypeParam is used both with LambdaParam and TypeVar, so
            // we can only check bounds when a LambdaParam is present.
            if (!isSelfTypeT1) {
                auto self2 = cast_type_nonnull<SelfTypeParam>(t2);
                if (auto *lambdaParam = cast_type<LambdaParam>(self2.definition.resultType(gs))) {
                    return Types::isSubTypeUnderConstraint(gs, constr, t1, lambdaParam->lowerBound, mode);
                } else {
                    return false;
                }
            } else if (!isSelfTypeT2) {
                auto self1 = cast_type_nonnull<SelfTypeParam>(t1);
                if (auto *lambdaParam = cast_type<LambdaParam>(self1.definition.resultType(gs))) {
                    return Types::isSubTypeUnderConstraint(gs, constr, lambdaParam->upperBound, t2, mode);
                } else {
                    return false;
                }
            } else {
                auto self1 = cast_type_nonnull<SelfTypeParam>(t1);
                auto self2 = cast_type_nonnull<SelfTypeParam>(t2);
                if (self1.definition == self2.definition) {
                    return true;
                }

                auto *lambda1 = cast_type<LambdaParam>(self1.definition.resultType(gs));
                auto *lambda2 = cast_type<LambdaParam>(self2.definition.resultType(gs));
                return lambda1 && lambda2 &&
                       Types::isSubTypeUnderConstraint(gs, constr, lambda1->upperBound, lambda2->lowerBound, mode);
            }
        }
    }

    {
        if (isa_type<SelfType>(t1) || isa_type<SelfType>(t2)) {
            // NOTE: SelfType is an inlined type, so TypePtr equality is type equality.
            if (t1 != t2) {
                return false;
            }
            return true;
        }
    }

    if (auto *a1 = cast_type<AppliedType>(t1)) {
        auto *a2 = cast_type<AppliedType>(t2);
        bool result;
        if (a2 == nullptr) {
            if (isa_type<ClassType>(t2)) {
                auto c2 = cast_type_nonnull<ClassType>(t2);
                return classSymbolIsAsGoodAs(gs, a1->klass, c2.symbol);
            }
            return false;
        } else {
            result = classSymbolIsAsGoodAs(gs, a1->klass, a2->klass);
        }
        if (result) {
            InlinedVector<TypeMemberRef, 4> indexes = Types::alignBaseTypeArgs(gs, a1->klass, a1->targs, a2->klass);
            // code below inverts permutation of type params
            int j = 0;
            for (SymbolRef idx : a2->klass.data(gs)->typeMembers()) {
                TypeMemberRef idxTypeMember = idx.asTypeMemberRef();
                int i = 0;
                while (indexes[j] != a1->klass.data(gs)->typeMembers()[i]) {
                    i++;
                    if (i >= a1->klass.data(gs)->typeMembers().size()) {
                        return result;
                    }
                }

                ENFORCE(i < a1->klass.data(gs)->typeMembers().size());

                if (idxTypeMember.data(gs)->flags.isCovariant) {
                    result = Types::isSubTypeUnderConstraint(gs, constr, a1->targs[i], a2->targs[j], mode);
                } else if (idxTypeMember.data(gs)->flags.isInvariant) {
                    auto &a = a1->targs[i];
                    auto &b = a2->targs[j];
                    if (mode == UntypedMode::AlwaysCompatible) {
                        result = Types::equivUnderConstraint(gs, constr, a, b);
                    } else {
                        // At the time of writing, we never set mode == UntypedMode::AlwaysIncompatible
                        // except when `constr` is EmptyFrozenConstraint, so there's no observable
                        // difference whether we use equivNoUntyped or equivNoUntypedUnderConstraint here.
                        // May as well do it for symmetry though.
                        result = Types::equivNoUntypedUnderConstraint(gs, constr, a, b);
                    }
                } else if (idxTypeMember.data(gs)->flags.isContravariant) {
                    result = Types::isSubTypeUnderConstraint(gs, constr, a2->targs[j], a1->targs[i], mode);
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
    if (isa_type<AppliedType>(t2)) {
        if (is_proxy_type(t1)) {
            return Types::isSubTypeUnderConstraint(gs, constr, t1.underlying(gs), t2, mode);
        }
        return false;
    }

    if (is_proxy_type(t1)) {
        if (is_proxy_type(t2)) {
            bool result;
            // TODO: simply compare as memory regions
            typecase(
                t1,
                [&](const TupleType &a1) { // Warning: this implements COVARIANT arrays
                    auto *a2 = cast_type<TupleType>(t2);
                    result = a2 != nullptr && a1.elems.size() >= a2->elems.size();
                    if (result) {
                        int i = -1;
                        for (auto &el2 : a2->elems) {
                            ++i;
                            result = Types::isSubTypeUnderConstraint(gs, constr, a1.elems[i], el2, mode);
                            if (!result) {
                                break;
                            }
                        }
                    }
                },
                [&](const ShapeType &h1) { // Warning: this implements COVARIANT hashes
                    auto *h2 = cast_type<ShapeType>(t2);
                    result = h2 != nullptr && h2->keys.size() <= h1.keys.size();
                    if (!result) {
                        return;
                    }
                    // have enough keys.
                    int i = -1;
                    for (auto &el2 : h2->keys) {
                        ++i;
                        auto optind = h1.indexForKey(el2);
                        if (!optind.has_value()) {
                            result = false;
                            return;
                        }
                        if (!Types::isSubTypeUnderConstraint(gs, constr, h1.values[optind.value()], h2->values[i],
                                                             mode)) {
                            result = false;
                            return;
                        }
                    }
                },
                [&](const NamedLiteralType &l1) {
                    if (!isa_type<NamedLiteralType>(t2)) {
                        // is a literal a subtype of a different kind of proxy
                        result = false;
                        return;
                    }

                    auto l2 = cast_type_nonnull<NamedLiteralType>(t2);
                    result = l1.equals(l2);
                },
                [&](const IntegerLiteralType &l1) {
                    if (!isa_type<IntegerLiteralType>(t2)) {
                        // is a literal a subtype of a different kind of proxy
                        result = false;
                        return;
                    }

                    auto &l2 = cast_type_nonnull<IntegerLiteralType>(t2);
                    result = l1.equals(l2);
                },
                [&](const FloatLiteralType &l1) {
                    if (!isa_type<FloatLiteralType>(t2)) {
                        // is a literal a subtype of a different kind of proxy
                        result = false;
                        return;
                    }

                    auto &l2 = cast_type_nonnull<FloatLiteralType>(t2);
                    result = l1.equals(l2);
                },
                [&](const MetaType &m1) {
                    auto *m2 = cast_type<MetaType>(t2);
                    if (m2 == nullptr) {
                        // is a literal a subtype of a different kind of proxy
                        result = false;
                        return;
                    }

                    result = Types::equiv(gs, m1.wrapped, m2->wrapped);
                });
            return result;
            // both are proxy
        } else {
            // only 1st is proxy
            TypePtr und = t1.underlying(gs);
            return isSubTypeUnderConstraintSingle(gs, constr, mode, und, t2);
        }
    } else if (is_proxy_type(t2)) {
        // non-proxies are never subtypes of proxies.
        return false;
    } else {
        if (isa_type<ClassType>(t1)) {
            if (isa_type<ClassType>(t2)) {
                auto c1 = cast_type_nonnull<ClassType>(t1);
                auto c2 = cast_type_nonnull<ClassType>(t2);
                return classSymbolIsAsGoodAs(gs, c1.symbol, c2.symbol);
            }
        }
        Exception::raise("isSubTypeUnderConstraint({}, {}): unreachable", t1.typeName(), t2.typeName());
    }
}

bool Types::isAsSpecificAs(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    return isSubTypeUnderConstraint(gs, TypeConstraint::EmptyFrozenConstraint, t1, t2, UntypedMode::AlwaysIncompatible);
}

bool Types::isSubType(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    return isSubTypeUnderConstraint(gs, TypeConstraint::EmptyFrozenConstraint, t1, t2, UntypedMode::AlwaysCompatible);
}

bool Types::isSubTypeUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                     const TypePtr &t2, UntypedMode mode) {
    if (t1 == t2) {
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

    // Note: order of cases here matters! We can't loose "and" information in t1 early and we can't
    // loose "or" information in t2 early.
    if (auto *o1 = cast_type<OrType>(t1)) { // 7, 8, 9
        return Types::isSubTypeUnderConstraint(gs, constr, o1->left, t2, mode) &&
               Types::isSubTypeUnderConstraint(gs, constr, o1->right, t2, mode);
    }

    if (auto *a2 = cast_type<AndType>(t2)) { // 2, 5
        return Types::isSubTypeUnderConstraint(gs, constr, t1, a2->left, mode) &&
               Types::isSubTypeUnderConstraint(gs, constr, t1, a2->right, mode);
    }

    auto *a1 = cast_type<AndType>(t1);
    auto *o2 = cast_type<OrType>(t2);

    if (a1 != nullptr) {
        // If the left is an And of an Or, then we can reorder it to be an Or of
        // an And, which lets us recurse on smaller types
        auto l = a1->left;
        auto r = a1->right;
        if (isa_type<OrType>(r)) {
            swap(r, l);
        }
        auto *a2o = cast_type<OrType>(l);
        if (a2o != nullptr) {
            // This handles `(A | B) & C` -> `(A & C) | (B & C)`

            // this could be using glb, but we _know_ that we alredy tried to collapse it(prior
            // construction of types did). Thus we use AndType::make_shared instead
            return Types::isSubTypeUnderConstraint(gs, constr, AndType::make_shared(a2o->left, r), t2, mode) &&
                   Types::isSubTypeUnderConstraint(gs, constr, AndType::make_shared(a2o->right, r), t2, mode);
        }
    }
    if (o2 != nullptr) {
        // Simiarly to above, if the right is an Or of an And, then we can reorder it to be an And of
        // an Or, which lets us recurse on smaller types
        auto l = o2->left;
        auto r = o2->right;
        if (isa_type<AndType>(r)) {
            swap(r, l);
        }
        auto *o2a = cast_type<AndType>(l);
        if (o2a != nullptr) {
            // This handles `(A & B) | C` -> `(A | C) & (B | C)`

            // this could be using lub, but we _know_ that we alredy tried to collapse it(prior
            // construction of types did). Thus we use OrType::make_shared instead
            return Types::isSubTypeUnderConstraint(gs, constr, t1, OrType::make_shared(o2a->left, r), mode) &&
                   Types::isSubTypeUnderConstraint(gs, constr, t1, OrType::make_shared(o2a->right, r), mode);
        }
    }

    // For these two cases, we have special cases if we are determining subtyping
    // with respect to a TypeVar.  The rationale is that the non-TypeVar type has
    // some structure that we wish to record: it's not correct to record each member
    // individually in a recursive call to isSubTypeUnderConstraint since a) the
    // recursive calls to isSubTypeUnderConstraint on each member would short-circuit
    // and therefore not examine some members and b) we lose the structure of the
    // original type.  But we only want to special-case TypeVars when recording type
    // constraints prior to solving; once we have solved the type constraints, we
    // want to look at each member individually and short-circuit as appropriate.

    // This order matters
    if (o2 != nullptr) { // 3
        if (isa_type<TypeVar>(t1) && !constr.isSolved()) {
            return constr.rememberIsSubtype(gs, t1, t2);
        }

        // This is a hack. isSubTypeUnderConstraint is trying to do double duty as constraint generation and constraint
        // solving. It essentially implements a greedy algorithm despite no greedy algorithm being correct.
        // There are a handful of places where we try to work around those hacks with more hacks, and this is one of
        // them.
        //
        // Concretely, it's still possible to come up with cases where this heuristic isn't good enough.
        // For more, see the comment in `no_short_circuit_type_constraint.rb`
        auto leftIsSubType = Types::isSubTypeUnderConstraint(gs, constr, t1, o2->left, mode);
        auto stillNeedToCheckRight = t1.isUntyped() && o2->left.isFullyDefined() && !o2->right.isFullyDefined();
        if (leftIsSubType && !stillNeedToCheckRight) {
            // Short circuit to save time
            return true;
        } else {
            return Types::isSubTypeUnderConstraint(gs, constr, t1, o2->right, mode);
        }
    }
    if (a1 != nullptr) { // 4
        if (isa_type<TypeVar>(t2) && !constr.isSolved()) {
            return constr.rememberIsSubtype(gs, t1, t2);
        }
        // See explanation in "// 3"
        auto leftIsSubType = Types::isSubTypeUnderConstraint(gs, constr, a1->left, t2, mode);
        auto stillNeedToCheckRight = t2.isUntyped() && a1->left.isFullyDefined() && !a1->right.isFullyDefined();
        if (leftIsSubType && !stillNeedToCheckRight) {
            // Short circuit to save time
            return true;
        } else {
            return Types::isSubTypeUnderConstraint(gs, constr, a1->right, t2, mode);
        }
    }

    return isSubTypeUnderConstraintSingle(gs, constr, mode, t1, t2); // 1
}

bool Types::equiv(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    return isSubType(gs, t1, t2) && isSubType(gs, t2, t1);
}

bool Types::equivUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1, const TypePtr &t2) {
    auto mode = UntypedMode::AlwaysCompatible;
    return isSubTypeUnderConstraint(gs, constr, t1, t2, mode) && isSubTypeUnderConstraint(gs, constr, t2, t1, mode);
}

bool Types::equivNoUntyped(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    return isAsSpecificAs(gs, t1, t2) && isAsSpecificAs(gs, t2, t1);
}

bool Types::equivNoUntypedUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                          const TypePtr &t2) {
    auto mode = UntypedMode::AlwaysIncompatible;
    return isSubTypeUnderConstraint(gs, constr, t1, t2, mode) && isSubTypeUnderConstraint(gs, constr, t2, t1, mode);
}

} // namespace sorbet::core
