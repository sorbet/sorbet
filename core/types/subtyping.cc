#include "common/common.h"
#include "common/sort/sort.h"
#include "common/typecase.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"
#include <algorithm> // find_if
#include <utility>

namespace sorbet::core {

using namespace std;

namespace {
// A class or module symbol other than the special `T.untyped`, `T.anything` and `T.noreturn` ones, which subtyping,
// `lub` and `glb` all handle before looking at the symbols involved.
bool isOrdinaryClassOrModule(ClassOrModuleRef symbol) {
    return symbol != Symbols::untyped() && symbol != Symbols::top() && symbol != Symbols::bottom();
}

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
    SLOW_ENFORCE(Types::isSubType(gs, t1, ret), "\n{}\nis not a supertype of\n{}\nwas lubbing with {}",
                 ret.toString(gs), t1.toString(gs), t2.toString(gs));
    SLOW_ENFORCE(Types::isSubType(gs, t2, ret), "\n{}\nis not a supertype of\n{}\nwas lubbing with {}",
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
    auto o = cast_type<OrType>(type);
    if (o == nullptr) {
        orComponents.emplace_back(type);
    } else {
        fillInOrComponents(orComponents, o->left);
        fillInOrComponents(orComponents, o->right);
    }
}

// The identities (see `TypePtr::identity`) of the components of `type`, a leaf or a tree of `OrType`s.
void fillInOrComponentIdentities(InlinedVector<TypePtr::tagged_storage, 4> &identities, const TypePtr &type) {
    auto o = cast_type<OrType>(type);
    if (o == nullptr) {
        identities.emplace_back(type.identity());
    } else {
        fillInOrComponentIdentities(identities, o->left);
        fillInOrComponentIdentities(identities, o->right);
    }
}

// `typeFilter` holds sorted identities.
TypePtr filterOrComponents(const TypePtr &originalType, absl::Span<const TypePtr::tagged_storage> typeFilter) {
    auto o = cast_type<OrType>(originalType);
    if (o == nullptr) {
        if (absl::c_binary_search(typeFilter, originalType.identity())) {
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

// Whether `lub(component, t2)` for a union `t2` is decided by distributing over `t2`'s components alone, so that it is
// `t2` itself as soon as `component` is one of them. `T.all` and `T.self_type` components are lubbed the other way
// around, and the special classes collapse with everything.
bool lubsByDistributingOverUnion(const TypePtr &component) {
    if (isa_type<AndType>(component) || isa_type<SelfType>(component)) {
        return false;
    }
    if (isa_type<ClassType>(component)) {
        return isOrdinaryClassOrModule(cast_type_nonnull<ClassType>(component).symbol);
    }
    return true;
}

TypePtr lubDistributeOr(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    InlinedVector<TypePtr, 4> originalOrComponents;
    InlinedVector<TypePtr::tagged_storage, 4> typesConsumed;
    auto o1 = cast_type<OrType>(t1);
    ENFORCE(o1 != nullptr);
    fillInOrComponents(originalOrComponents, o1->left);
    fillInOrComponents(originalOrComponents, o1->right);

    // A component of t1 that is also a component of a union t2 is subsumed by t2 without lubbing it against every
    // component of t2 in turn, which made this quadratic for the unions of hundreds of classes that sealed classes and
    // enums expand to.
    InlinedVector<TypePtr::tagged_storage, 4> t2Components;
    if (isa_type<OrType>(t2)) {
        fillInOrComponentIdentities(t2Components, t2);
        fast_sort(t2Components);
    }

    for (auto &component : originalOrComponents) {
        if (!t2Components.empty() && lubsByDistributingOverUnion(component) &&
            absl::c_binary_search(t2Components, component.identity())) {
            categoryCounterInc("lubDistributeOr.component", "shared");
            typesConsumed.emplace_back(component.identity());
            continue;
        }
        if (isa_type<ClassType>(component) && isa_type<ClassType>(t2)) {
            // `lub` of two ordinary classes collapses to one of them or is their plain union; decide which without
            // building (and then dropping) that union, which is what folding a class into a large union spends its
            // time on. Distinct classes with the same superclass cannot derive from each other.
            auto c1 = cast_type_nonnull<ClassType>(component).symbol;
            auto c2 = cast_type_nonnull<ClassType>(t2).symbol;
            if (isOrdinaryClassOrModule(c1) && isOrdinaryClassOrModule(c2)) {
                auto d1 = c1.data(gs);
                auto d2 = c2.data(gs);
                if (c1 == c2) {
                    categoryCounterInc("lubDistributeOr.outcome", "t1");
                    return t1;
                }
                auto siblings =
                    d1->isClass() && d2->isClass() && d1->superClass().exists() && d1->superClass() == d2->superClass();
                if (siblings) {
                    continue;
                }
                if (d2->derivesFrom(gs, c1)) {
                    categoryCounterInc("lubDistributeOr.outcome", "t1");
                    return t1;
                }
                if (d1->derivesFrom(gs, c2)) {
                    typesConsumed.emplace_back(component.identity());
                }
                continue;
            }
        }
        auto lubbed = Types::any(gs, component, t2);
        if (lubbed == component) {
            // lubbed == component, so t2 <: component and t2 <: t1
            categoryCounterInc("lubDistributeOr.outcome", "t1");
            return t1;
        } else if (lubbed == t2) {
            // lubbed == t2, so component <: t2
            // Thus, we don't need to include component in the final OrType; it's subsumed by t2.
            typesConsumed.emplace_back(component.identity());
        }
    }
    if (typesConsumed.empty()) {
        // t1 has no components that overlap with t2
        categoryCounterInc("lubDistributeOr.outcome", "worst");
        return OrType::make_shared(t1, underlying(gs, t2));
    }
    // lub back everything except typesConsumed
    fast_sort(typesConsumed);
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
    auto a1 = cast_type<AndType>(t1);
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
    } else if (!isa_type<AndType>(n1)) {
        categoryCounterInc("glbDistributeAnd.outcome", "n1a1r");
        return AndType::make_shared(n1, a1->right);
    } else if (!isa_type<AndType>(n2)) {
        categoryCounterInc("glbDistributeAnd.outcome", "a1ln2");
        return AndType::make_shared(a1->left, n2);
    }

    categoryCounterInc("glbDistributeAnd.outcome", "worst");
    return AndType::make_shared(t1, t2);
}

namespace {
// Collects the components of `type` (a leaf, or the leaves of an `OrType` tree) that can witness a non-empty glb by
// reference equality: an ordinary class or module, or an applied type.
void collectSharedComponentCandidates(InlinedVector<const TypePtr *, 8> &out, const TypePtr &type) {
    if (auto o = cast_type<OrType>(type)) {
        collectSharedComponentCandidates(out, o->left);
        collectSharedComponentCandidates(out, o->right);
    } else if (isa_type<AppliedType>(type)) {
        out.emplace_back(&type);
    } else if (isa_type<ClassType>(type) && isOrdinaryClassOrModule(cast_type_nonnull<ClassType>(type).symbol)) {
        out.emplace_back(&type);
    }
}
} // namespace

bool Types::glbIsKnownNonEmpty(const TypePtr &t1, const TypePtr &t2) {
    if (!isa_type<OrType>(t1) && !isa_type<OrType>(t2)) {
        // `glb` itself handles the non-union cases cheaply.
        return false;
    }
    // `glb` only produces `T.noreturn` for a union when it does for every pair of components (the union cases
    // distribute, and the class/applied cases collapse to a component they share), so one shared component decides.
    InlinedVector<const TypePtr *, 8> components1;
    InlinedVector<const TypePtr *, 8> components2;
    collectSharedComponentCandidates(components1, t1);
    collectSharedComponentCandidates(components2, t2);
    for (auto *c1 : components1) {
        for (auto *c2 : components2) {
            if (*c1 == *c2) {
                return true;
            }
        }
    }
    return false;
}

// only keep knowledge in t1 that is not already present in t2. Return the same reference if unchanged
TypePtr dropLubComponents(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    if (auto a1 = cast_type<AndType>(t1)) {
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
    } else if (auto o1 = cast_type<OrType>(t1)) {
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

    if (isa_type<OrType>(t2)) { // 3, 5, 6
        categoryCounterInc("lub", "or>");
        return lubDistributeOr(gs, t2, t1);
    } else if (auto a2 = cast_type<AndType>(t2)) { // 2, 4
        if (auto a1 = cast_type<AndType>(t1)) {
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

    if (auto a1 = cast_type<AppliedType>(t1)) {
        auto a2 = cast_type<AppliedType>(t2);
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
                    if (auto a2 = cast_type<TupleType>(t2)) {
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
                    if (auto h2 = cast_type<ShapeType>(t2)) {
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
        if (isa_type<MetaType>(t1) || isa_type<MetaType>(t2)) {
            auto m1 = cast_type<MetaType>(t1);
            auto m2 = cast_type<MetaType>(t2);
            if (m1 != nullptr && m2 != nullptr && Types::equiv(gs, m1->wrapped, m2->wrapped)) {
                return t1;
            }

            // This is weird. We used to treat the "underlying" of a MetaType as `Object`.
            // We should probably _not_ treat it like it has an underlying, to catch mistakes where
            // people treat runtime types as values, but that's a battle for another day.
            // We should at least treat it like T::Types::Base, not Object, but again: another day.
            auto m1underlying = m1 == nullptr ? t1 : Types::Object();
            auto m2underlying = m2 == nullptr ? t2 : Types::Object();
            return lub(gs, m1underlying, m2underlying);
        }
    }

    ENFORCE(!isa_type<LambdaParam>(t1) && !isa_type<LambdaParam>(t2),
            "Are you forgetting a call to resultTypeAsSeenFrom?");

    {
        if (isa_type<TypeVar>(t1) || isa_type<TypeVar>(t2)) {
            return OrType::make_shared(t1, t2);
        }
    }

    {
        auto isSelfTypeParamT1 = isa_type<SelfTypeParam>(t1);
        auto isSelfTypeParamT2 = isa_type<SelfTypeParam>(t2);

        // NOTE: SelfTypeParam is an inlined type, so TypePtr equality is type equality.
        if (isSelfTypeParamT1 && isSelfTypeParamT2) {
            if (t1 != t2) {
                return OrType::make_shared(t1, t2);
            } else {
                return t1;
            }
        } else if (isSelfTypeParamT2) {
            auto selfTypeT2 = cast_type_nonnull<SelfTypeParam>(t2);
            // NOTE: SelfTypeParam is used both with TypeMember and TypeParameter--only TypeMembers have bounds today
            if (const auto lambdaParam = cast_type<LambdaParam>(selfTypeT2.definition.resultType(gs))) {
                if (!lambdaParam->lowerBound.isUntyped() && isSubType(gs, t1, lambdaParam->lowerBound)) {
                    return t2;
                }
            }
            return OrType::make_shared(t1, t2);
        } else if (isSelfTypeParamT1) {
            return OrType::make_shared(t1, t2);
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
    /** this implementation makes a bet that types are small and very likely to be collapsible.
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
    /** this implementation makes a bet that types are small and very likely to be collapsible.
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

namespace {
// An ordinary class or module (see `isOrdinaryClassOrModule`) that is a class, and not `void` either.
bool isOrdinaryClass(const GlobalState &gs, ClassOrModuleRef symbol) {
    return isOrdinaryClassOrModule(symbol) && symbol != Symbols::void_() && symbol.data(gs)->isClass();
}

// Whether every component of the union `type` is an ordinary class that neither derives from nor is derived from
// `klass`, itself an ordinary class.
bool unionOfClassesUnrelatedTo(const GlobalState &gs, ClassOrModuleRef klass, const TypePtr &type) {
    if (auto o = cast_type<OrType>(type)) {
        return unionOfClassesUnrelatedTo(gs, klass, o->left) && unionOfClassesUnrelatedTo(gs, klass, o->right);
    }
    if (!isa_type<ClassType>(type)) {
        return false;
    }
    auto symbol = cast_type_nonnull<ClassType>(type).symbol;
    return symbol != klass && isOrdinaryClass(gs, symbol) && !symbol.data(gs)->derivesFrom(gs, klass) &&
           !klass.data(gs)->derivesFrom(gs, symbol);
}

// Whether `leaf` is, by reference, a component of the union `type`.
bool unionContains(const TypePtr &type, const TypePtr &leaf) {
    if (auto o = cast_type<OrType>(type)) {
        return unionContains(o->left, leaf) || unionContains(o->right, leaf);
    }
    return type == leaf;
}

// Whether `glb(classType, orType)` is `T.noreturn` because the class is unrelated to every class in the union: the
// recursive glb finds no collapse at any level and every leaf glb is empty (Ruby has single inheritance), so this
// decides in one pass what the recursion re-checks at each of its levels.
bool glbOfClassAndUnrelatedClassesIsEmpty(const GlobalState &gs, const TypePtr &classType, const TypePtr &orType) {
    auto klass = cast_type_nonnull<ClassType>(classType).symbol;
    return isOrdinaryClass(gs, klass) && unionOfClassesUnrelatedTo(gs, klass, orType);
}
} // namespace

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

    {
        if (isa_type<MetaType>(t1) || isa_type<MetaType>(t2)) {
            auto m1 = cast_type<MetaType>(t1);
            auto m2 = cast_type<MetaType>(t2);
            if (m1 != nullptr && m2 != nullptr && Types::equiv(gs, m1->wrapped, m2->wrapped)) {
                return t1;
            }

            return Types::bottom();
        }
    }

    if (auto o2 = cast_type<OrType>(t2)) { // 3, 6
        if (isa_type<ClassType>(t1) || isa_type<AppliedType>(t1)) {
            if (unionContains(t2, t1)) {
                // `t1` is one of the components, so it is as specific as the union (the check below would find so
                // after comparing it with every component up to itself).
                categoryCounterInc("glb", "Zor");
                return t1;
            }
            if (isa_type<ClassType>(t1) && glbOfClassAndUnrelatedClassesIsEmpty(gs, t1, t2)) {
                categoryCounterInc("glb", "unrelatedClasses");
                return Types::bottom();
            }
        }
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

        if (isa_type<ClassType>(t1) || isa_type<AppliedType>(t1) || isa_type<SelfTypeParam>(t1)) {
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

        if (auto o1 = cast_type<OrType>(t1)) { // 6
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

    if (auto a1 = cast_type<AppliedType>(t1)) {
        auto a2 = cast_type<AppliedType>(t2);
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

    ENFORCE(!isa_type<LambdaParam>(t1) && !isa_type<LambdaParam>(t2),
            "Are you forgetting a call to resultTypeAsSeenFrom?");

    {
        if (isa_type<TypeVar>(t1) || isa_type<TypeVar>(t2)) {
            return AndType::make_shared(t1, t2);
        }
    }
    {
        auto isSelfTypeParamT1 = isa_type<SelfTypeParam>(t1);
        auto isSelfTypeParamT2 = isa_type<SelfTypeParam>(t2);

        // NOTE: SelfTypeParam is an inlined type, so TypePtr equality is type equality.
        if (isSelfTypeParamT1 && isSelfTypeParamT2) {
            if (t1 != t2) {
                return AndType::make_shared(t1, t2);
            } else {
                return t1;
            }
        } else if (isSelfTypeParamT2) {
            auto selfTypeT2 = cast_type_nonnull<SelfTypeParam>(t2);
            // NOTE: SelfTypeParam is used both with TypeMember and TypeParameter--only TypeMembers have bounds today
            if (const auto lambdaParam = cast_type<LambdaParam>(selfTypeT2.definition.resultType(gs))) {
                if (!lambdaParam->upperBound.isUntyped() && isSubType(gs, lambdaParam->upperBound, t1)) {
                    return t2;
                } else if (glb(gs, t1, lambdaParam->upperBound).isBottom()) {
                    return bottom();
                }
            }
            return AndType::make_shared(t1, t2);
        } else if (isSelfTypeParamT1) {
            return AndType::make_shared(t1, t2);
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

bool isModuleSingletonClass(const GlobalState &gs, ClassOrModuleRef sym) {
    auto maybeAttachedClass = sym.data(gs)->attachedClass(gs);
    return maybeAttachedClass.exists() && maybeAttachedClass.data(gs)->isModule();
}

string moduleSingletonError(string_view tp) {
    return ErrorColors::format(
        "`{}` represents a module singleton class type, which is a `{}`, not a `{}`. See the `{}` docs.", tp, "Module",
        "Class", "T.class_of");
}

void doesNotDeriveFrom(const GlobalState &gs, ErrorSection::Collector &errorDetailsCollector, ClassOrModuleRef left,
                       ClassOrModuleRef right) {
    auto subCollector = errorDetailsCollector.newCollector();
    auto message = right == Symbols::Class() && isModuleSingletonClass(gs, left)
                       ? moduleSingletonError(left.show(gs))
                       : ErrorColors::format("`{}` does not derive from `{}`", left.show(gs), right.show(gs));
    subCollector.message = message;
    errorDetailsCollector.addErrorDetails(move(subCollector));
}

void checkForAttachedClassHint(const GlobalState &gs, ErrorSection::Collector &errorDetailsCollector,
                               const ClassType left, const SelfTypeParam right) {
    if (right.definition.name(gs) != Names::Constants::AttachedClass()) {
        return;
    }

    auto attachedClass = left.symbol.data(gs)->lookupSingletonClass(gs);
    if (!attachedClass.exists()) {
        return;
    }

    if (attachedClass != right.definition.owner(gs).asClassOrModuleRef()) {
        return;
    }

    auto gotStr = left.show(gs);
    auto expectedStr = right.show(gs);
    auto subCollector = errorDetailsCollector.newCollector();
    auto message = ErrorColors::format(
        "`{}` is incompatible with `{}` because when this method is called on a subclass `{}` will represent a more "
        "specific subclass, meaning `{}` will not be specific enough. See https://sorbet.org/docs/attached-class for "
        "more.",
        gotStr, expectedStr, expectedStr, gotStr);
    subCollector.message = message;
    errorDetailsCollector.addErrorDetails(move(subCollector));
}

void compareToUntyped(const GlobalState &gs, TypeConstraint &constr, const TypePtr &ty, const TypePtr &blame) {
    ENFORCE(blame.isUntyped());
    if (is_proxy_type(ty)) {
        compareToUntyped(gs, constr, ty.underlying(gs), blame);
    }

    if (auto t = cast_type<AppliedType>(ty)) {
        for (auto &targ : t->targs) {
            compareToUntyped(gs, constr, targ, blame);
        }
    } else if (auto t = cast_type<ShapeType>(ty)) {
        for (auto &val : t->values) {
            compareToUntyped(gs, constr, val, blame);
        }
    } else if (auto t = cast_type<TupleType>(ty)) {
        for (auto &val : t->elems) {
            compareToUntyped(gs, constr, val, blame);
        }
    } else if (auto t = cast_type<OrType>(ty)) {
        compareToUntyped(gs, constr, t->left, blame);
        compareToUntyped(gs, constr, t->right, blame);
    } else if (auto t = cast_type<AndType>(ty)) {
        compareToUntyped(gs, constr, t->left, blame);
        compareToUntyped(gs, constr, t->right, blame);
    } else if (isa_type<TypeVar>(ty)) {
        constr.rememberIsSubtype(gs, ty, blame);
        constr.rememberIsSubtype(gs, blame, ty);
    }
}

namespace {

// Whether recording a lower bound for `sym` would start from nothing: `rememberIsSubtype` treats a missing or empty
// bound as the first recorded type, and `lub(T.noreturn, t)` is `t`.
bool lowerBoundIsUnset(const TypeConstraint &constr, TypeParameterRef sym) {
    if (!constr.hasLowerBound(sym)) {
        return true;
    }
    auto bound = constr.findLowerBound(sym);
    return bound == nullptr || bound.isBottom();
}

// Collects the leaves of the `OrType` tree `type` in order, and reports whether the tree leans left (every right child
// is a leaf), the shape that folding `lub` over the leaves produces.
bool collectOrLeaves(const TypePtr &type, InlinedVector<TypePtr, 8> &leaves) {
    auto o = cast_type<OrType>(type);
    if (o == nullptr) {
        leaves.emplace_back(type);
        return true;
    }
    auto leansLeft = collectOrLeaves(o->left, leaves);
    auto rightIsLeaf = !isa_type<OrType>(o->right);
    collectOrLeaves(o->right, leaves);
    return leansLeft && rightIsLeaf;
}

// A class type other than the special `T.untyped`, `T.anything`, `T.noreturn` and `void` ones: one whose subtyping
// against another such type is decided by `classSymbolIsAsGoodAs` alone, without side effects.
bool isPlainClassType(const TypePtr &type) {
    if (!isa_type<ClassType>(type)) {
        return false;
    }
    auto symbol = cast_type_nonnull<ClassType>(type).symbol;
    return isOrdinaryClassOrModule(symbol) && symbol != Symbols::void_();
}

// Collects the leaves of the union `type` in order into `leaves` if they are all plain class types.
bool collectPlainClassLeaves(const TypePtr &type, InlinedVector<TypePtr, 8> &leaves) {
    if (auto o = cast_type<OrType>(type)) {
        return collectPlainClassLeaves(o->left, leaves) && collectPlainClassLeaves(o->right, leaves);
    }
    if (!isPlainClassType(type)) {
        return false;
    }
    leaves.emplace_back(type);
    return true;
}

// Whether the plain class `c1` is a subtype of the union `o2`, decided in one pass over the leaves when they are all
// plain class types: the recursion over the union checks the leaves in the same order and stops at the first that `c1`
// is as good as, with no side effects along the way. `nullopt` when some leaf is not a plain class type.
optional<bool> plainClassIsSubTypeOfUnion(const GlobalState &gs, ClassOrModuleRef c1, const OrType &o2) {
    InlinedVector<TypePtr, 8> leaves;
    if (!collectPlainClassLeaves(o2.left, leaves) || !collectPlainClassLeaves(o2.right, leaves)) {
        return nullopt;
    }
    for (auto &leaf : leaves) {
        if (classSymbolIsAsGoodAs(gs, c1, cast_type_nonnull<ClassType>(leaf).symbol)) {
            return true;
        }
    }
    return false;
}

// Whether the union `o1` is a subtype of the union `o2` when both are unions of plain class types: every class of `o1`
// is a class of `o2`, or derives from one. Checking each class of `o1` against `o2` in turn scans `o2` for each of
// them, quadratic for the unions of hundreds of classes that `T::Enum`s and sealed classes expand to; a class that
// `o2` contains outright needs no scan. `nullopt` when some leaf is not a plain class type.
optional<bool> unionOfPlainClassesIsSubTypeOfUnion(const GlobalState &gs, const OrType &o1, const OrType &o2) {
    InlinedVector<TypePtr, 8> leaves1;
    InlinedVector<TypePtr, 8> leaves2;
    if (!collectPlainClassLeaves(o1.left, leaves1) || !collectPlainClassLeaves(o1.right, leaves1) ||
        !collectPlainClassLeaves(o2.left, leaves2) || !collectPlainClassLeaves(o2.right, leaves2)) {
        return nullopt;
    }
    InlinedVector<TypePtr::tagged_storage, 8> identities2;
    identities2.reserve(leaves2.size());
    for (auto &leaf : leaves2) {
        identities2.emplace_back(leaf.identity());
    }
    fast_sort(identities2);
    for (auto &leaf1 : leaves1) {
        if (absl::c_binary_search(identities2, leaf1.identity())) {
            continue;
        }
        auto c1 = cast_type_nonnull<ClassType>(leaf1).symbol;
        if (!absl::c_any_of(leaves2, [&](const TypePtr &leaf2) {
                return classSymbolIsAsGoodAs(gs, c1, cast_type_nonnull<ClassType>(leaf2).symbol);
            })) {
            return false;
        }
    }
    return true;
}

} // namespace

// Whether `leaves` are classes that all share the same superclass, so that no two different ones are related and `lub`
// of any two of them is their plain union, or the class itself when they are the same class.
bool areSiblingClasses(const GlobalState &gs, absl::Span<const TypePtr> leaves) {
    auto superClass = Symbols::noClassOrModule();
    for (auto &leaf : leaves) {
        if (!isa_type<ClassType>(leaf)) {
            return false;
        }
        auto symbol = cast_type_nonnull<ClassType>(leaf).symbol;
        if (!isOrdinaryClass(gs, symbol) || !symbol.data(gs)->superClass().exists()) {
            return false;
        }
        if (!superClass.exists()) {
            superClass = symbol.data(gs)->superClass();
        } else if (symbol.data(gs)->superClass() != superClass) {
            return false;
        }
    }
    return true;
}

// Whether some leaf repeats an earlier one. Class types are stored inline in a `TypePtr`, so equal identities are the
// same class.
bool hasRepeatedLeaves(absl::Span<const TypePtr> leaves) {
    InlinedVector<TypePtr::tagged_storage, 8> identities;
    identities.reserve(leaves.size());
    for (auto &leaf : leaves) {
        identities.emplace_back(leaf.identity());
    }
    fast_sort(identities);
    return absl::c_adjacent_find(identities) != identities.end();
}

// Whether `leaves` are distinct classes that all share the same superclass, so that no two are related and `lub` of
// any two of them is their plain union.
bool areDistinctSiblingClasses(const GlobalState &gs, absl::Span<const TypePtr> leaves) {
    return areSiblingClasses(gs, leaves) && !hasRepeatedLeaves(leaves);
}

// The left-leaning chain that folding `lub` over `leaves`, distinct sibling classes, builds.
TypePtr chainOfSiblingClasses(absl::Span<const TypePtr> leaves) {
    TypePtr chain = leaves[0];
    for (size_t i = 1; i < leaves.size(); i++) {
        chain = OrType::make_shared(chain, leaves[i]);
    }
    return chain;
}

// If `type` is a union of distinct sibling classes, the left-leaning chain of its components that folding `lub` over
// them builds: `type` itself when it already has that shape. Otherwise `nullptr`.
TypePtr leftLeaningChainOfSiblingClasses(const GlobalState &gs, const TypePtr &type) {
    InlinedVector<TypePtr, 8> leaves;
    auto leansLeft = collectOrLeaves(type, leaves);
    if (!areDistinctSiblingClasses(gs, leaves)) {
        return nullptr;
    }
    return leansLeft ? type : chainOfSiblingClasses(leaves);
}

TypePtr Types::lubAll(const GlobalState &gs, const vector<TypePtr> &elements) {
    // Folding `lub` over N sibling classes (the elements of a large array literal of enum values, say) costs N^2 class
    // comparisons and builds the union one element at a time; its result is known directly: the union of the distinct
    // classes, each where it first occurs.
    if (elements.size() >= 2 && areSiblingClasses(gs, elements)) {
        InlinedVector<TypePtr, 8> distinct;
        UnorderedSet<TypePtr::tagged_storage> seen;
        for (auto &element : elements) {
            if (seen.insert(element.identity()).second) {
                distinct.emplace_back(element);
            }
        }
        return chainOfSiblingClasses(distinct);
    }
    TypePtr acc = Types::bottom();
    for (auto &el : elements) {
        // The only time that `Types::lub` produces a proxy_type is if the two proxy types are
        // equivalent: `:foo | :foo`. If they're not equivalent, we widen. There are no
        // `:foo | :bar` types produced by `lub`, so `widen` is unnecessary.
        //
        // Which means that to remove all literals, it's sufficient to do a single `dropLiteral`
        // at the call `lubAll` call site.
        acc = Types::lub(gs, acc, el);
    }
    return acc;
}

// "Single" means "ClassType or ProxyType"; since ProxyTypes are constrained to
// be proxies over class types, this means "class or class-like"
template <class T>
bool isSubTypeUnderConstraintSingle(const GlobalState &gs, TypeConstraint &constr, UntypedMode mode, const TypePtr &t1,
                                    const TypePtr &t2, T &errorDetailsCollector) {
    constexpr auto shouldAddErrorDetails = std::is_same_v<T, ErrorSection::Collector>;

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
                return mayBeSpecial2.symbol == Symbols::top() || mayBeSpecial2.symbol == Symbols::void_() ||
                       mayBeSpecial2.symbol == Symbols::untyped();
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
        if (mayBeSpecial2.symbol == Symbols::top() || mayBeSpecial2.symbol == Symbols::void_()) {
            return true;
        }
    }

    ENFORCE(!isa_type<LambdaParam>(t1) && !isa_type<LambdaParam>(t2),
            "Are you forgetting a call to resultTypeAsSeenFrom?");

    {
        auto isSelfTypeT1 = isa_type<SelfTypeParam>(t1);
        auto isSelfTypeT2 = isa_type<SelfTypeParam>(t2);
        if (isSelfTypeT1 || isSelfTypeT2) {
            // NOTE: SelfTypeParam is used both with LambdaParam and TypeVar, so
            // we can only check bounds when a LambdaParam is present.
            if (!isSelfTypeT1) {
                auto self2 = cast_type_nonnull<SelfTypeParam>(t2);
                if (auto lambdaParam = cast_type<LambdaParam>(self2.definition.resultType(gs))) {
                    auto result = Types::isSubTypeUnderConstraint(gs, constr, t1, lambdaParam->lowerBound, mode,
                                                                  errorDetailsCollector);
                    if constexpr (shouldAddErrorDetails) {
                        if (!result && isa_type<ClassType>(t1)) {
                            checkForAttachedClassHint(gs, errorDetailsCollector, cast_type_nonnull<ClassType>(t1),
                                                      self2);
                        }
                    }
                    return result;
                } else {
                    return false;
                }
            } else if (!isSelfTypeT2) {
                auto self1 = cast_type_nonnull<SelfTypeParam>(t1);
                if (auto lambdaParam = cast_type<LambdaParam>(self1.definition.resultType(gs))) {
                    return Types::isSubTypeUnderConstraint(gs, constr, lambdaParam->upperBound, t2, mode,
                                                           errorDetailsCollector);
                } else {
                    return false;
                }
            } else {
                auto self1 = cast_type_nonnull<SelfTypeParam>(t1);
                auto self2 = cast_type_nonnull<SelfTypeParam>(t2);
                if (self1.definition == self2.definition) {
                    return true;
                }

                auto lambda1 = cast_type<LambdaParam>(self1.definition.resultType(gs));
                auto lambda2 = cast_type<LambdaParam>(self2.definition.resultType(gs));
                return lambda1 && lambda2 &&
                       Types::isSubTypeUnderConstraint(gs, constr, lambda1->upperBound, lambda2->lowerBound, mode,
                                                       errorDetailsCollector);
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

    {
        if (isa_type<MetaType>(t1) || isa_type<MetaType>(t2)) {
            auto m1 = cast_type<MetaType>(t1);
            auto m2 = cast_type<MetaType>(t2);
            if (m1 != nullptr && m2 != nullptr) {
                // TODO(jez) Should this actually run under EmptyFrozenConstraint? Leaving for backwards
                // compatibility, but maybe we should do this under the `constr` that's in scope.
                return Types::equivUnderConstraint(gs, TypeConstraint::EmptyFrozenConstraint, m1->wrapped, m2->wrapped,
                                                   errorDetailsCollector);
            }

            if (m2 == nullptr) {
                auto res = isSubTypeUnderConstraintSingle(gs, constr, mode, Types::Object(), t2, errorDetailsCollector);

                if constexpr (shouldAddErrorDetails) {
                    auto subCollectorLine1 = errorDetailsCollector.newCollector();
                    subCollectorLine1.message = ErrorColors::format(
                        "It looks like you're using Sorbet type syntax in a runtime value position.");
                    errorDetailsCollector.addErrorDetails(move(subCollectorLine1));
                    auto subCollectorLine2 = errorDetailsCollector.newCollector();
                    subCollectorLine2.message =
                        ErrorColors::format("If you really mean to use types as values, use `{}` "
                                            "to hide the type syntax from the type checker.",
                                            "T::Utils.coerce");
                    errorDetailsCollector.addErrorDetails(move(subCollectorLine2));
                    auto subCollectorLine3 = errorDetailsCollector.newCollector();
                    subCollectorLine3.message = ErrorColors::format(
                        "Otherwise, you're likely using the type system in a way it wasn't meant to be used.");
                    errorDetailsCollector.addErrorDetails(move(subCollectorLine3));
                }

                return res;
            }

            return false;
        }
    }

    if (auto a1 = cast_type<AppliedType>(t1)) {
        auto a2 = cast_type<AppliedType>(t2);
        bool result;
        if (a2 == nullptr) {
            if (isa_type<ClassType>(t2)) {
                auto c2 = cast_type_nonnull<ClassType>(t2);
                result = classSymbolIsAsGoodAs(gs, a1->klass, c2.symbol);
                if constexpr (shouldAddErrorDetails) {
                    if (!result) {
                        doesNotDeriveFrom(gs, errorDetailsCollector, a1->klass, c2.symbol);
                    }
                }
                return result;
            }
            return false;
        } else {
            result = classSymbolIsAsGoodAs(gs, a1->klass, a2->klass);
        }
        if (!result) {
            if constexpr (shouldAddErrorDetails) {
                doesNotDeriveFrom(gs, errorDetailsCollector, a1->klass, a2->klass);
            }
            return result;
        }
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

            auto &a1i = a1->targs[i];
            auto &a2j = a2->targs[j];
            bool doesMemberMatch = true;
            auto subCollector = errorDetailsCollector.newCollector();
            if (idxTypeMember.data(gs)->flags.isCovariant) {
                doesMemberMatch = Types::isSubTypeUnderConstraint(gs, constr, a1i, a2j, mode, subCollector);
            } else if (idxTypeMember.data(gs)->flags.isInvariant) {
                if (mode == UntypedMode::AlwaysCompatible) {
                    doesMemberMatch = Types::equivUnderConstraint(gs, constr, a1i, a2j, subCollector);
                } else {
                    // At the time of writing, we never set mode == UntypedMode::AlwaysIncompatible
                    // except when `constr` is EmptyFrozenConstraint, so there's no observable
                    // difference whether we use equivNoUntyped or equivNoUntypedUnderConstraint here.
                    // May as well do it for symmetry though.
                    doesMemberMatch = Types::equivNoUntypedUnderConstraint(gs, constr, a1i, a2j, subCollector);
                }
            } else if (idxTypeMember.data(gs)->flags.isContravariant) {
                doesMemberMatch = Types::isSubTypeUnderConstraint(gs, constr, a2j, a1i, mode, subCollector);
            }
            if (!doesMemberMatch) {
                result = false;
                if constexpr (shouldAddErrorDetails) {
                    if (!(a2->klass == Symbols::Hash() &&
                          idxTypeMember.data(gs)->name == core::Names::Constants::Elem())) {
                        string variance;
                        string joiningText;
                        switch (idxTypeMember.data(gs)->variance()) {
                            case Variance::CoVariant: {
                                variance = "covariant";
                                joiningText = "a subtype of";
                                break;
                            }
                            case Variance::Invariant: {
                                variance = "invariant";
                                joiningText = "equivalent to";
                                break;
                            }
                            case Variance::ContraVariant: {
                                variance = "contravariant";
                                joiningText = "a supertype of";
                                break;
                            }
                        }
                        auto message = ErrorColors::format("`{}` is not {} `{}` for {} type member `{}`", a1i.show(gs),
                                                           joiningText, a2j.show(gs), variance, idxTypeMember.show(gs));
                        subCollector.message = message;
                        errorDetailsCollector.addErrorDetails(std::move(subCollector));
                    }
                } else {
                    break;
                }
            }
            j++;
        }
        // alight type params.
        return result;
    }
    if (auto a2 = cast_type<AppliedType>(t2)) {
        if (is_proxy_type(t1)) {
            return Types::isSubTypeUnderConstraint(gs, constr, t1.underlying(gs), t2, mode, errorDetailsCollector);
        }

        if constexpr (shouldAddErrorDetails) {
            if (a2->klass != Symbols::Class() || !isa_type<ClassType>(t1)) {
                return false;
            }
            const auto &c1 = cast_type_nonnull<ClassType>(t1);
            if (!isModuleSingletonClass(gs, c1.symbol)) {
                return false;
            }

            auto subCollector = errorDetailsCollector.newCollector();
            subCollector.message = moduleSingletonError(t1.show(gs));
            errorDetailsCollector.addErrorDetails(move(subCollector));
        }

        return false;
    }

    if (is_proxy_type(t1)) {
        if (is_proxy_type(t2)) {
            // both are proxy
            bool result;
            // TODO: simply compare as memory regions
            typecase(
                t1,
                [&](const TupleType &a1) { // Warning: this implements COVARIANT arrays
                    auto a2 = cast_type<TupleType>(t2);
                    result = a2 != nullptr && a1.elems.size() >= a2->elems.size();
                    if (result) {
                        int i = -1;
                        for (auto &el2 : a2->elems) {
                            ++i;
                            auto subCollector = errorDetailsCollector.newCollector();
                            if (!Types::isSubTypeUnderConstraint(gs, constr, a1.elems[i], el2, mode, subCollector)) {
                                result = false;
                                if constexpr (shouldAddErrorDetails) {
                                    auto message = ErrorColors::format(
                                        "`{}` is not a subtype of `{}` for index `{}` of `{}`-tuple",
                                        a1.elems[i].show(gs), el2.show(gs), i, a2->elems.size());
                                    subCollector.message = message;
                                    errorDetailsCollector.addErrorDetails(std::move(subCollector));
                                } else {
                                    break;
                                }
                            }
                        }
                    } else {
                        return;
                    }
                },
                [&](const ShapeType &h1) { // Warning: this implements COVARIANT hashes
                    auto h2 = cast_type<ShapeType>(t2);
                    result = h2 != nullptr && h2->keys.size() <= h1.keys.size();
                    if constexpr (shouldAddErrorDetails) {
                        if (h2 == nullptr) {
                            return;
                        }
                        // If we're using this subtyping call to report an error, we should loop through all the items
                        // even if there aren't enough keys, so we can report all the missing keys to the user, and
                        // report an error for the incorrect keys.
                    } else {
                        if (!result) {
                            return;
                        }
                    }
                    // have enough keys (or we want to keep going for rich error reporting).
                    int h2index = -1;
                    for (auto &el2 : h2->keys) {
                        ++h2index;
                        auto opth1index = h1.indexForKey(el2);
                        auto subCollector = errorDetailsCollector.newCollector();
                        if (!opth1index.has_value()) {
                            result = false;
                            if constexpr (!shouldAddErrorDetails) {
                                return;
                            }
                        } else if (!Types::isSubTypeUnderConstraint(gs, constr, h1.values[opth1index.value()],
                                                                    h2->values[h2index], mode, subCollector)) {
                            result = false;
                            if constexpr (shouldAddErrorDetails) {
                                auto message = ErrorColors::format("`{}` is not a subtype of `{}` for key `{}`",
                                                                   h1.values[opth1index.value()].show(gs),
                                                                   h2->values[h2index].show(gs), el2.show(gs));
                                subCollector.message = message;
                                errorDetailsCollector.addErrorDetails(std::move(subCollector));
                            } else {
                                return;
                            }
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
                });
            return result;
        } else {
            // only 1st is proxy
            TypePtr und = t1.underlying(gs);
            return isSubTypeUnderConstraintSingle(gs, constr, mode, und, t2, errorDetailsCollector);
        }
    } else if (is_proxy_type(t2)) {
        // only 2nd is proxy
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
    return isSubTypeUnderConstraint(gs, TypeConstraint::EmptyFrozenConstraint, t1, t2, UntypedMode::AlwaysIncompatible,
                                    ErrorSection::Collector::NO_OP);
}

template <class T>
bool Types::isSubType(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2, T &errorDetailsCollector) {
    return isSubTypeUnderConstraint(gs, TypeConstraint::EmptyFrozenConstraint, t1, t2, UntypedMode::AlwaysCompatible,
                                    errorDetailsCollector);
}

template <class T>
bool Types::isSubTypeUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                     const TypePtr &t2, UntypedMode mode, T &errorDetailsCollector) {
    constexpr auto shouldAddErrorDetails = std::is_same_v<T, ErrorSection::Collector>;

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

    // Note: order of cases here matters! We can't lose "and" information in t1 early and we can't
    // lose "or" information in t2 early.
    if (auto o1 = cast_type<OrType>(t1)) { // 7, 8, 9
        if (!constr.isSolved() && isa_type<TypeVar>(t2) &&
            lowerBoundIsUnset(constr, cast_type_nonnull<TypeVar>(t2).sym)) {
            // Recording the components one at a time lubs each into the growing lower bound, quadratic work for the
            // unions of hundreds of values that `T::Enum`s expand to. When they are distinct sibling classes, that
            // fold only rebuilds the union as a left-leaning chain, so record that chain in one step.
            if (auto chain = leftLeaningChainOfSiblingClasses(gs, t1)) {
                return constr.rememberIsSubtype(gs, chain, t2);
            }
        }
        if (auto o2 = cast_type<OrType>(t2)) {
            // Success has no side effects to reproduce; failure takes the general path below, which explains it.
            if (unionOfPlainClassesIsSubTypeOfUnion(gs, *o1, *o2) == true) {
                return true;
            }
        }
        auto subCollectorLeft = errorDetailsCollector.newCollector();
        auto isSubTypeOfLeft = Types::isSubTypeUnderConstraint(gs, constr, o1->left, t2, mode, subCollectorLeft);
        if (!isSubTypeOfLeft) {
            if constexpr (shouldAddErrorDetails) {
                // This if is to handle `T.nilable(X) < Y`; if we've already told the user that T.nilable(X) is not a
                // subtype of Y, it's not useful to also tell the user that X is not a subtype of Y or that nil is not a
                // subtype Y
                if (!o1->left.isNilClass() && !o1->right.isNilClass()) {
                    auto message = ErrorColors::format("`{}` (the left side of the `{}`) is not a subtype of `{}`",
                                                       o1->left.show(gs), "T.any", t2.show(gs));
                    subCollectorLeft.message = message;
                    errorDetailsCollector.addErrorDetails(move(subCollectorLeft));
                } else {
                    for (auto &c : subCollectorLeft.children) {
                        errorDetailsCollector.addErrorDetails(move(c));
                    }
                }
            }
            return isSubTypeOfLeft;
        }
        auto subCollectorRight = errorDetailsCollector.newCollector();
        auto isSubTypeOfRight = Types::isSubTypeUnderConstraint(gs, constr, o1->right, t2, mode, subCollectorRight);
        if (!isSubTypeOfRight) {
            if constexpr (shouldAddErrorDetails) {
                // This if is to handle `T.nilable(X) < Y`; if we've already told the user that T.nilable(X) is not a
                // subtype of Y, it's not useful to also tell the user that X is not a subtype of Y or that nil is not a
                // subtype Y
                if (!o1->left.isNilClass() && !o1->right.isNilClass()) {
                    auto message = ErrorColors::format("`{}` (the right side of the `{}`) is not a subtype of `{}`",
                                                       o1->right.show(gs), "T.any", t2.show(gs));
                    subCollectorRight.message = message;
                    errorDetailsCollector.addErrorDetails(move(subCollectorRight));
                } else {
                    for (auto &c : subCollectorRight.children) {
                        errorDetailsCollector.addErrorDetails(move(c));
                    }
                }
            }
        }
        return isSubTypeOfRight;
    }

    if (auto a2 = cast_type<AndType>(t2)) { // 2, 5
        auto subCollectorLeft = errorDetailsCollector.newCollector();
        auto isSubTypeOfLeft = Types::isSubTypeUnderConstraint(gs, constr, t1, a2->left, mode, subCollectorLeft);
        if (!isSubTypeOfLeft) {
            if constexpr (shouldAddErrorDetails) {
                auto message = ErrorColors::format("`{}` is not a subtype of `{}` (the left side of the `{}`)",
                                                   t1.show(gs), a2->left.show(gs), "T.all");
                subCollectorLeft.message = message;
                errorDetailsCollector.addErrorDetails(move(subCollectorLeft));
            }
            return isSubTypeOfLeft;
        }

        auto subCollectorRight = errorDetailsCollector.newCollector();
        auto isSubTypeOfRight = Types::isSubTypeUnderConstraint(gs, constr, t1, a2->right, mode, subCollectorRight);
        if constexpr (shouldAddErrorDetails) {
            if (!isSubTypeOfRight) {
                auto message = ErrorColors::format("`{}` is not a subtype of `{}` (the right side of the `{}`)",
                                                   t1.show(gs), a2->right.show(gs), "T.all");
                subCollectorRight.message = message;
                errorDetailsCollector.addErrorDetails(move(subCollectorRight));
            }
        }
        return isSubTypeOfRight;
    }

    auto a1 = cast_type<AndType>(t1);
    auto o2 = cast_type<OrType>(t2);

    if (a1 != nullptr) {
        // If the left is an And of an Or, then we can reorder it to be an Or of
        // an And, which lets us recurse on smaller types
        const auto *l = &a1->left;
        const auto *r = &a1->right;
        if (isa_type<OrType>(*r)) {
            swap(r, l);
        }
        auto a1o = cast_type<OrType>(*l);
        if (a1o != nullptr) {
            // This handles `(A | B) & C` -> `(A & C) | (B & C)`

            // this could be using glb, but we _know_ that we already tried to collapse it (prior
            // construction of types did). Thus we use AndType::make_shared instead
            return Types::isSubTypeUnderConstraint(gs, constr, AndType::make_shared(a1o->left, *r), t2, mode,
                                                   errorDetailsCollector) &&
                   Types::isSubTypeUnderConstraint(gs, constr, AndType::make_shared(a1o->right, *r), t2, mode,
                                                   errorDetailsCollector);
        }
    }
    if (o2 != nullptr) {
        // Similarly to above, if the right is an Or of an And, then we can reorder it to be an And of
        // an Or, which lets us recurse on smaller types
        const auto *l = &o2->left;
        const auto *r = &o2->right;
        if (isa_type<AndType>(*r)) {
            swap(r, l);
        }
        auto o2a = cast_type<AndType>(*l);
        if (o2a != nullptr) {
            // This handles `(A & B) | C` -> `(A | C) & (B | C)`

            // this could be using lub, but we _know_ that we already tried to collapse it (prior
            // construction of types did). Thus we use OrType::make_shared instead
            return Types::isSubTypeUnderConstraint(gs, constr, t1, OrType::make_shared(o2a->left, *r), mode,
                                                   errorDetailsCollector) &&
                   Types::isSubTypeUnderConstraint(gs, constr, t1, OrType::make_shared(o2a->right, *r), mode,
                                                   errorDetailsCollector);
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
        if (isPlainClassType(t1)) {
            // Neither outcome has side effects or error details to reproduce.
            if (auto result = plainClassIsSubTypeOfUnion(gs, cast_type_nonnull<ClassType>(t1).symbol, *o2)) {
                return *result;
            }
        }

        // This is a hack. isSubTypeUnderConstraint is trying to do double duty as constraint generation and constraint
        // solving. It essentially implements a greedy algorithm despite no greedy algorithm being correct.
        // There are a handful of places where we try to work around those hacks with more hacks, and this is one of
        // them.
        //
        // Concretely, it's still possible to come up with cases where this heuristic isn't good enough.
        // For more, see the comment in `no_short_circuit_type_constraint.rb`
        auto leftIsSubType = Types::isSubTypeUnderConstraint(gs, constr, t1, o2->left, mode, errorDetailsCollector);
        auto stillNeedToCheckRight = t1.isUntyped() && o2->left.isFullyDefined() && !o2->right.isFullyDefined();
        if (leftIsSubType && !stillNeedToCheckRight) {
            // Short circuit to save time
            return true;
        } else if (Types::isSubTypeUnderConstraint(gs, constr, t1, o2->right, mode, errorDetailsCollector)) {
            return true;
        } else if (isa_type<SelfTypeParam>(t1)) {
            auto selfTypeParam1 = cast_type_nonnull<SelfTypeParam>(t1);
            if (const auto lambdaParam = cast_type<LambdaParam>(selfTypeParam1.definition.resultType(gs))) {
                if (isa_type<OrType>(lambdaParam->upperBound) || isa_type<AndType>(lambdaParam->upperBound)) {
                    return Types::isSubTypeUnderConstraint(gs, constr, lambdaParam->upperBound, t2, mode,
                                                           errorDetailsCollector);
                }
            }
            return false;
        } else if (a1 == nullptr) {
            // If neither t1 <: o2->left nor t1 <: o2->right, it might mean that we tried to split
            // up an OrType when we weren't meant to. It could be that t1 is an AndType of an OrType
            //
            // Note: This is deliberately a case where the code to handle an OrType is not
            // symmetric (nor even anti-symmetric) with the code to handle an AndType. (There is no
            // corresponding logic in the `a1 != nullptr` condition below.)
            return false;
        }
    }
    if (a1 != nullptr) { // 4
        if (isa_type<TypeVar>(t2) && !constr.isSolved()) {
            return constr.rememberIsSubtype(gs, t1, t2);
        }
        // See explanation in "// 3"
        auto leftIsSubType = Types::isSubTypeUnderConstraint(gs, constr, a1->left, t2, mode, errorDetailsCollector);
        auto stillNeedToCheckRight = t2.isUntyped() && a1->left.isFullyDefined() && !a1->right.isFullyDefined();
        if (leftIsSubType && !stillNeedToCheckRight) {
            // Short circuit to save time
            return true;
        } else if (Types::isSubTypeUnderConstraint(gs, constr, a1->right, t2, mode, errorDetailsCollector)) {
            return true;
        } else if (isa_type<SelfTypeParam>(t2)) {
            auto selfTypeParam2 = cast_type_nonnull<SelfTypeParam>(t2);
            if (const auto lambdaParam = cast_type<LambdaParam>(selfTypeParam2.definition.resultType(gs))) {
                if (isa_type<OrType>(lambdaParam->lowerBound) || isa_type<AndType>(lambdaParam->lowerBound)) {
                    return Types::isSubTypeUnderConstraint(gs, constr, t1, lambdaParam->lowerBound, mode,
                                                           errorDetailsCollector);
                }
            }
            return false;
        } else {
            return false;
        }
    }

    return isSubTypeUnderConstraintSingle(gs, constr, mode, t1, t2, errorDetailsCollector); // 1
}

bool Types::equiv(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    return isSubType(gs, t1, t2) && isSubType(gs, t2, t1);
}

template <class T>
bool Types::equivUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1, const TypePtr &t2,
                                 T &errorDetailsCollector) {
    auto mode = UntypedMode::AlwaysCompatible;
    auto leftSubRight = isSubTypeUnderConstraint(gs, constr, t1, t2, mode, errorDetailsCollector);
    if (!leftSubRight) {
        return leftSubRight;
    }

    auto subCollector = errorDetailsCollector.newCollector();
    auto rightSubLeft = isSubTypeUnderConstraint(gs, constr, t2, t1, mode, subCollector);
    if constexpr (std::is_same_v<T, ErrorSection::Collector>) {
        if (!rightSubLeft) {
            auto message = ErrorColors::format(
                "`{}` is a subtype of `{}` but not the reverse, so they are not equivalent", t1.show(gs), t2.show(gs));
            subCollector.message = message;
            errorDetailsCollector.addErrorDetails(move(subCollector));
        }
    }

    return rightSubLeft;
}

bool Types::equivNoUntyped(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    return isAsSpecificAs(gs, t1, t2) && isAsSpecificAs(gs, t2, t1);
}

template <class T>
bool Types::equivNoUntypedUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                          const TypePtr &t2, T &errorDetailsCollector) {
    auto mode = UntypedMode::AlwaysIncompatible;
    return isSubTypeUnderConstraint(gs, constr, t1, t2, mode, errorDetailsCollector) &&
           isSubTypeUnderConstraint(gs, constr, t2, t1, mode, errorDetailsCollector);
}

template bool isSubTypeUnderConstraintSingle(const GlobalState &gs, TypeConstraint &constr, UntypedMode mode,
                                             const TypePtr &t1, const TypePtr &t2,
                                             core::ErrorSection::Collector &errorDetailsCollector);
template bool isSubTypeUnderConstraintSingle(const GlobalState &gs, TypeConstraint &constr, UntypedMode mode,
                                             const TypePtr &t1, const TypePtr &t2,
                                             core::ErrorSection::NoOpCollector const &errorDetailsCollector);

template bool Types::isSubTypeUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                              const TypePtr &t2, UntypedMode mode,
                                              core::ErrorSection::Collector &errorDetailsCollector);
template bool Types::isSubTypeUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                              const TypePtr &t2, UntypedMode mode,
                                              core::ErrorSection::NoOpCollector const &errorDetailsCollector);

template bool Types::isSubType(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2,
                               core::ErrorSection::Collector &errorDetailsCollector);
template bool Types::isSubType(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2,
                               core::ErrorSection::NoOpCollector const &errorDetailsCollector);

template bool Types::equivUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                          const TypePtr &t2, core::ErrorSection::Collector &errorDetailsCollector);
template bool Types::equivUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                          const TypePtr &t2,
                                          core::ErrorSection::NoOpCollector const &errorDetailsCollector);

template bool Types::equivNoUntypedUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                                   const TypePtr &t2,
                                                   core::ErrorSection::Collector &errorDetailsCollector);
template bool Types::equivNoUntypedUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                                   const TypePtr &t2,
                                                   core::ErrorSection::NoOpCollector const &errorDetailsCollector);

} // namespace sorbet::core
