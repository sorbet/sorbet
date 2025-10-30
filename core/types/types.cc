#include "core/Types.h"
#include "absl/base/casts.h"
#include "absl/strings/match.h"
#include "common/common.h"
#include "common/strings/formatting.h"
#include "common/typecase.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/errors/infer.h"
#include "core/errors/resolver.h"
#include <utility>

#include "core/Types.h"

// improve debugging.
template class std::shared_ptr<sorbet::core::TypeConstraint>;
template class std::shared_ptr<sorbet::core::SendAndBlockLink>;
template class std::vector<sorbet::core::Loc>;

namespace sorbet::core {

using namespace std;

namespace {
// We create types pretty freely during inference, and even small programs can create a
// large amount of types; see test/testdata/infer/is_subtype_timeout.rb for an example.
// Recording counters for these types can take up a substantial amount of time for such
// tests, and so we disable counting them altogether even in debug builds.
//
// If you want to compute a histogram of types, flip this to true and rebuild.
constexpr bool countTypeAllocations = false;
void recordAllocatedType(ConstExprStr kind) {
    if constexpr (countTypeAllocations) {
        categoryCounterInc("types.allocated", kind);
    }
}
} // namespace

TypePtr Types::top() {
    return make_type<ClassType>(Symbols::top());
}

TypePtr Types::bottom() {
    return make_type<ClassType>(Symbols::bottom());
}

TypePtr Types::nilClass() {
    return make_type<ClassType>(Symbols::NilClass());
}

TypePtr Types::untypedUntracked() {
    return make_type<ClassType>(Symbols::untyped());
}

TypePtr Types::untyped(sorbet::core::SymbolRef blame) {
    if constexpr (!sorbet::track_untyped_blame_mode && !sorbet::debug_mode) {
        return untypedUntracked();
    }

    if (blame.exists()) {
        return make_type<BlamedUntyped>(blame);
    } else {
        return untypedUntracked();
    }
}

TypePtr Types::void_() {
    return make_type<ClassType>(Symbols::void_());
}

TypePtr Types::trueClass() {
    return make_type<ClassType>(Symbols::TrueClass());
}

TypePtr Types::falseClass() {
    return make_type<ClassType>(Symbols::FalseClass());
}

TypePtr Types::Boolean() {
    static auto res = OrType::make_shared(trueClass(), falseClass());
    return res;
}

TypePtr Types::Integer() {
    return make_type<ClassType>(Symbols::Integer());
}

TypePtr Types::Float() {
    return make_type<ClassType>(Symbols::Float());
}

TypePtr Types::arrayOfUntyped(sorbet::core::SymbolRef blame) {
    if constexpr (!sorbet::track_untyped_blame_mode && !sorbet::debug_mode) {
        static vector<TypePtr> targs{Types::untypedUntracked()};
        static auto res = make_type<AppliedType>(Symbols::Array(), move(targs));
        return res;
    } else {
        vector<TypePtr> targs{Types::untyped(blame)};
        auto res = make_type<AppliedType>(Symbols::Array(), move(targs));
        return res;
    }
}

TypePtr Types::rangeOfUntyped(sorbet::core::SymbolRef blame) {
    if constexpr (!sorbet::track_untyped_blame_mode && !sorbet::debug_mode) {
        static vector<TypePtr> targs{Types::untypedUntracked()};
        static auto res = make_type<AppliedType>(Symbols::Range(), move(targs));
        return res;
    } else {
        vector<TypePtr> targs{Types::untyped(blame)};
        auto res = make_type<AppliedType>(Symbols::Range(), move(targs));
        return res;
    }
}

TypePtr Types::hashOfUntyped() {
    static vector<TypePtr> targs{Types::untypedUntracked(), Types::untypedUntracked(), Types::untypedUntracked()};
    static auto res = make_type<AppliedType>(Symbols::Hash(), move(targs));
    return res;
}

TypePtr Types::hashOfUntyped(sorbet::core::SymbolRef blame) {
    if constexpr (!sorbet::track_untyped_blame_mode && !sorbet::debug_mode) {
        return Types::hashOfUntyped();
    } else {
        auto untypedWithBlame = Types::untyped(blame);
        vector<TypePtr> targs{untypedWithBlame, untypedWithBlame, untypedWithBlame};
        auto res = make_type<AppliedType>(Symbols::Hash(), move(targs));
        return res;
    }
}

TypePtr Types::procClass() {
    return make_type<ClassType>(Symbols::Proc());
}

TypePtr Types::nilableProcClass() {
    static auto res = OrType::make_shared(nilClass(), procClass());
    return res;
}

TypePtr Types::declBuilderForProcsSingletonClass() {
    return make_type<ClassType>(Symbols::DeclBuilderForProcsSingleton());
}

TypePtr Types::String() {
    return make_type<ClassType>(Symbols::String());
}

TypePtr Types::Symbol() {
    return make_type<ClassType>(Symbols::Symbol());
}

TypePtr Types::Object() {
    return make_type<ClassType>(Symbols::Object());
}

TypePtr Types::BasicObject() {
    return make_type<ClassType>(Symbols::BasicObject());
}

TypePtr Types::falsyTypes() {
    static auto res = OrType::make_shared(Types::nilClass(), Types::falseClass());
    return res;
}

absl::Span<const ClassOrModuleRef> Types::falsySymbols() {
    static InlinedVector<ClassOrModuleRef, 2> res{Symbols::NilClass(), Symbols::FalseClass()};
    return res;
}

TypePtr Types::todo() {
    return make_type<ClassType>(Symbols::todo());
}

TypePtr Types::dropSubtypesOf(const GlobalState &gs, const TypePtr &from, absl::Span<const ClassOrModuleRef> klasses) {
    TypePtr result;

    if (from.isUntyped()) {
        return from;
    }

    typecase(
        from,
        [&](const OrType &o) {
            auto lhs = dropSubtypesOf(gs, o.left, klasses);
            auto rhs = dropSubtypesOf(gs, o.right, klasses);
            if (lhs == o.left && rhs == o.right) {
                result = from;
            } else if (lhs.isBottom()) {
                result = rhs;
            } else if (rhs.isBottom()) {
                result = lhs;
            } else {
                result = OrType::make_shared(lhs, rhs);
            }
        },
        [&](const AndType &a) {
            auto lhs = dropSubtypesOf(gs, a.left, klasses);
            auto rhs = dropSubtypesOf(gs, a.right, klasses);
            if (lhs != a.left || rhs != a.right) {
                result = Types::all(gs, lhs, rhs);
            } else {
                result = from;
            }
        },
        [&](const ClassType &c) {
            auto cdata = c.symbol.data(gs);
            if (c.symbol == core::Symbols::untyped() || c.symbol == core::Symbols::top()) {
                result = from;
            } else if (absl::c_any_of(klasses,
                                      [&](auto klass) { return c.symbol == klass || c.derivesFrom(gs, klass); })) {
                result = Types::bottom();
            } else if (c.symbol.data(gs)->isClass() && absl::c_all_of(klasses, [&](auto klass) {
                           return klass.data(gs)->isClass() && !klass.data(gs)->derivesFrom(gs, c.symbol);
                       })) {
                // We have two classes (not modules), and if all of the the classes we're removing
                // don't derive from `c`, there's nothing to do because Ruby has single inheritance.
                result = from;
            } else if (cdata->flags.isSealed && (cdata->flags.isAbstract || cdata->isModule())) {
                auto subclasses = cdata->sealedSubclassesToUnion(gs);
                ENFORCE(!Types::equiv(gs, subclasses, from), "sealedSubclassesToUnion about to cause infinite loop");
                result = dropSubtypesOf(gs, subclasses, klasses);
            } else {
                result = from;
            }
        },
        [&](const AppliedType &a) {
            auto adata = a.klass.data(gs);
            if (absl::c_any_of(klasses, [&](auto klass) { return a.klass == klass || a.derivesFrom(gs, klass); })) {
                result = Types::bottom();
            } else if (a.klass.data(gs)->isClass() && absl::c_all_of(klasses, [&](auto klass) {
                           return klass.data(gs)->isClass() && !klass.data(gs)->derivesFrom(gs, a.klass);
                       })) {
                // We have two classes (not modules), and if all of the the classes we're removing
                // don't derive from `c`, there's nothing to do because Ruby has single inheritance.
                result = from;
            } else if (adata->flags.isSealed && (adata->flags.isAbstract || adata->isModule())) {
                auto subclasses = adata->sealedSubclassesToUnion(gs);
                ENFORCE(!Types::equiv(gs, subclasses, from), "sealedSubclassesToUnion about to cause infinite loop");
                result = dropSubtypesOf(gs, subclasses, klasses);
                result = Types::all(gs, from, result);
            } else {
                result = from;
            }
        },
        [&](const SelfTypeParam &p) {
            if (!p.definition.isTypeMember()) {
                // Only type members have upper bounds--type parameters do not
                result = from;
                return;
            }

            auto &lambdaParam = cast_type_nonnull<LambdaParam>(p.definition.asTypeMemberRef().data(gs)->resultType);
            auto droppedFromUpper = dropSubtypesOf(gs, lambdaParam.upperBound, klasses);
            if (droppedFromUpper.isBottom()) {
                result = Types::bottom();
            } else if (droppedFromUpper == lambdaParam.upperBound) {
                result = from;
            } else if (droppedFromUpper.kind() <= from.kind()) {
                result = AndType::make_shared(droppedFromUpper, from);
            } else {
                result = AndType::make_shared(from, droppedFromUpper);
            }
        },
        [&](const TypePtr &) {
            if (is_proxy_type(from) && dropSubtypesOf(gs, from.underlying(gs), klasses).isBottom()) {
                result = Types::bottom();
            } else {
                result = from;
            }
        });
    SLOW_ENFORCE(Types::isSubType(gs, result, from),
                 "dropSubtypesOf({}, [{}]) returned {}, which is not a subtype of the input", from.toString(gs),
                 fmt::map_join(klasses, ", ", [&](auto klass) { return klass.showFullName(gs); }), result.toString(gs));
    return result;
}

bool Types::canBeTruthy(const GlobalState &gs, const TypePtr &what) {
    bool isTruthy = true;
    typecase(
        what, [&](const OrType &o) { isTruthy = canBeTruthy(gs, o.left) || canBeTruthy(gs, o.right); },
        [&](const AndType &a) { isTruthy = canBeTruthy(gs, a.left) && canBeTruthy(gs, a.right); },
        [&](const ClassType &c) {
            auto sym = c.symbol;
            isTruthy = sym == core::Symbols::untyped() ||
                       (sym != core::Symbols::FalseClass() && sym != core::Symbols::NilClass());
        },
        [&](const AppliedType &c) {
            auto sym = c.klass;
            isTruthy = sym == core::Symbols::untyped() ||
                       (sym != core::Symbols::FalseClass() && sym != core::Symbols::NilClass());
        },
        [&](const TypePtr &) {
            if (is_proxy_type(what)) {
                isTruthy = canBeTruthy(gs, what.underlying(gs));
            } else {
                isTruthy = true;
            }
        });

    return isTruthy;
}

bool Types::canBeFalsy(const GlobalState &gs, const TypePtr &what) {
    if (what.isUntyped()) {
        return true;
    }
    return Types::isSubType(gs, Types::falseClass(), what) ||
           Types::isSubType(gs, Types::nilClass(), what); // check if inhabited by falsy values
}

TypePtr Types::approximateSubtract(const GlobalState &gs, const TypePtr &from, const TypePtr &what) {
    TypePtr result;
    typecase(
        what, [&](const ClassType &c) { result = Types::dropSubtypesOf(gs, from, absl::MakeSpan(&c.symbol, 1)); },
        [&](const AppliedType &c) { result = Types::dropSubtypesOf(gs, from, absl::MakeSpan(&c.klass, 1)); },
        [&](const OrType &o) {
            result = Types::approximateSubtract(gs, Types::approximateSubtract(gs, from, o.left), o.right);
        },
        [&](const TypePtr &) { result = from; });
    return result;
}

TypePtr Types::dropLiteral(const GlobalState &gs, const TypePtr &tp) {
    if (isa_type<NamedLiteralType>(tp)) {
        auto a = cast_type_nonnull<NamedLiteralType>(tp);
        return a.underlying(gs);
    }
    if (isa_type<IntegerLiteralType>(tp)) {
        auto &i = cast_type_nonnull<IntegerLiteralType>(tp);
        return i.underlying(gs);
    }
    if (isa_type<FloatLiteralType>(tp)) {
        auto &f = cast_type_nonnull<FloatLiteralType>(tp);
        return f.underlying(gs);
    }
    return tp;
}

TypePtr Types::lubAll(const GlobalState &gs, const vector<TypePtr> &elements) {
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

TypePtr Types::arrayOf(const GlobalState &gs, const TypePtr &elem) {
    vector<TypePtr> targs{move(elem)};
    return make_type<AppliedType>(Symbols::Array(), move(targs));
}

TypePtr Types::rangeOf(const GlobalState &gs, const TypePtr &elem) {
    vector<TypePtr> targs{move(elem)};
    return make_type<AppliedType>(Symbols::Range(), move(targs));
}

TypePtr Types::hashOf(const GlobalState &gs, const TypePtr &elem) {
    vector<TypePtr> tupleArgs{Types::Symbol(), elem};
    vector<TypePtr> targs{Types::Symbol(), elem, make_type<TupleType>(move(tupleArgs))};
    return make_type<AppliedType>(Symbols::Hash(), move(targs));
}

TypePtr Types::hashOf(const GlobalState &gs, const TypePtr &key, const TypePtr &val) {
    vector<TypePtr> tupleArgs{key, val};
    vector<TypePtr> targs{key, val, make_type<TupleType>(move(tupleArgs))};
    return make_type<AppliedType>(Symbols::Hash(), move(targs));
}

TypePtr Types::setOf(const TypePtr &elem) {
    vector<TypePtr> targs{elem};
    return make_type<AppliedType>(Symbols::Set(), move(targs));
}

TypePtr Types::tClass(const TypePtr &attachedClass) {
    vector<TypePtr> targs;
    targs.emplace_back(attachedClass);
    return make_type<AppliedType>(Symbols::Class(), move(targs));
}

TypePtr Types::tModule(const TypePtr &attachedClass) {
    vector<TypePtr> targs;
    targs.emplace_back(attachedClass);
    return make_type<AppliedType>(Symbols::Module(), move(targs));
}

TypePtr Types::dropNil(const GlobalState &gs, const TypePtr &from) {
    static auto nilClass = core::Symbols::NilClass();
    static auto toDrop = absl::MakeSpan(&nilClass, 1);
    return Types::dropSubtypesOf(gs, from, toDrop);
}

optional<int> Types::getProcArity(const AppliedType &type) {
    for (int i = 0; i <= Symbols::MAX_PROC_ARITY; i++) {
        if (type.klass == Symbols::Proc(i)) {
            return i;
        }
    }
    return nullopt;
}

ClassType::ClassType(ClassOrModuleRef symbol) : symbol(symbol) {
    recordAllocatedType("classtype");
    ENFORCE(symbol.exists());
}

namespace {
void sanityCheckProxyType(const GlobalState &gs, TypePtr underlying) {
    ENFORCE(isa_type<ClassType>(underlying) || isa_type<AppliedType>(underlying));
    underlying.sanityCheck(gs);
}
} // namespace

NamedLiteralType::NamedLiteralType(ClassOrModuleRef klass, NameRef val)
    : name(val), kind(klass == Symbols::String() ? Kind::String : Kind::Symbol) {
    if (klass == Symbols::String()) {
        recordAllocatedType("literaltype.string");
    } else {
        recordAllocatedType("literaltype.symbol");
    }
    ENFORCE(klass == Symbols::String() || klass == Symbols::Symbol());
}

TypePtr NamedLiteralType::underlying(const GlobalState &gs) const {
    switch (kind) {
        case Kind::String:
            return Types::String();
        case Kind::Symbol:
            return Types::Symbol();
    }
}

IntegerLiteralType::IntegerLiteralType(int64_t val) : value(val) {
    recordAllocatedType("literalintegertype");
}

TypePtr IntegerLiteralType::underlying(const GlobalState &gs) const {
    return Types::Integer();
}

FloatLiteralType::FloatLiteralType(double val) : value(val) {
    recordAllocatedType("floatliteraltype");
}

TypePtr FloatLiteralType::underlying(const GlobalState &gs) const {
    return Types::Float();
}

TupleType::TupleType(vector<TypePtr> elements) : elems(move(elements)) {
    recordAllocatedType("tupletype");
    histogramInc("tupletype.elems", this->elems.size());
}

AndType::AndType(const TypePtr &left, const TypePtr &right) : left(move(left)), right(move(right)) {
    recordAllocatedType("andtype");
}

void NamedLiteralType::_sanityCheck(const GlobalState &gs) const {
    sanityCheckProxyType(gs, underlying(gs));
}

bool NamedLiteralType::equals(const NamedLiteralType &rhs) const {
    if (this->kind != rhs.kind) {
        return false;
    }

    return this->name == rhs.name;
}

void IntegerLiteralType::_sanityCheck(const GlobalState &gs) const {
    sanityCheckProxyType(gs, underlying(gs));
}

bool IntegerLiteralType::equals(const IntegerLiteralType &rhs) const {
    return this->value == rhs.value;
}

void FloatLiteralType::_sanityCheck(const GlobalState &gs) const {
    sanityCheckProxyType(gs, underlying(gs));
}

bool FloatLiteralType::equals(const FloatLiteralType &rhs) const {
    return this->value == rhs.value;
}

OrType::OrType(const TypePtr &left, const TypePtr &right) : left(move(left)), right(move(right)) {
    recordAllocatedType("ortype");
}

void TupleType::_sanityCheck(const GlobalState &gs) const {
    sanityCheckProxyType(gs, underlying(gs));
    auto underlying = this->underlying(gs);
    auto applied = cast_type<AppliedType>(underlying);
    ENFORCE(applied);
    ENFORCE(applied->klass == Symbols::Array());
}

ShapeType::ShapeType(vector<TypePtr> keys, vector<TypePtr> values) : keys(move(keys)), values(move(values)) {
    DEBUG_ONLY(for (auto &k
                    : this->keys) {
        ENFORCE(isa_type<NamedLiteralType>(k) || isa_type<IntegerLiteralType>(k) || isa_type<FloatLiteralType>(k));
    };);
    recordAllocatedType("shapetype");
    histogramInc("shapetype.keys", this->keys.size());
}

TypePtr ShapeType::underlying(const GlobalState &gs) const {
    return Types::hashOfUntyped(Symbols::Magic_UntypedSource_shapeUnderlying());
}

optional<size_t> ShapeType::indexForKey(const TypePtr &t) const {
    if (isa_type<NamedLiteralType>(t)) {
        const auto &lit = cast_type_nonnull<NamedLiteralType>(t);
        return this->indexForKey(lit);
    }
    if (isa_type<IntegerLiteralType>(t)) {
        auto &lit = cast_type_nonnull<IntegerLiteralType>(t);
        return this->indexForKey(lit);
    }
    if (isa_type<FloatLiteralType>(t)) {
        auto &lit = cast_type_nonnull<FloatLiteralType>(t);
        return this->indexForKey(lit);
    }
    return nullopt;
}

optional<size_t> ShapeType::indexForKey(const NamedLiteralType &lit) const {
    auto fnd = absl::c_find_if(this->keys, [&lit](const auto &candidate) -> bool {
        if (!isa_type<NamedLiteralType>(candidate)) {
            return false;
        }
        const auto &candlit = cast_type_nonnull<NamedLiteralType>(candidate);
        return candlit.equals(lit);
    });
    if (fnd == this->keys.end()) {
        return nullopt;
    }
    return std::distance(this->keys.begin(), fnd);
}

optional<size_t> ShapeType::indexForKey(const IntegerLiteralType &lit) const {
    auto fnd = absl::c_find_if(keys, [&](auto &candidate) -> bool {
        if (!isa_type<IntegerLiteralType>(candidate)) {
            return false;
        }
        const auto &candlit = cast_type_nonnull<IntegerLiteralType>(candidate);
        return candlit.equals(lit);
    });
    if (fnd == this->keys.end()) {
        return nullopt;
    }
    return std::distance(this->keys.begin(), fnd);
}

optional<size_t> ShapeType::indexForKey(const FloatLiteralType &lit) const {
    auto fnd = absl::c_find_if(keys, [&](auto &candidate) -> bool {
        if (!isa_type<FloatLiteralType>(candidate)) {
            return false;
        }
        const auto &candlit = cast_type_nonnull<FloatLiteralType>(candidate);
        return candlit.equals(lit);
    });
    if (fnd == this->keys.end()) {
        return nullopt;
    }
    return std::distance(this->keys.begin(), fnd);
}

TypePtr TupleType::underlying(const GlobalState &gs) const {
    if (this->elems.empty()) {
        return Types::arrayOfUntyped(Symbols::Magic_UntypedSource_tupleUnderlying());
    } else {
        return Types::arrayOf(gs, Types::dropLiteral(gs, Types::lubAll(gs, this->elems)));
    }
}

void ShapeType::_sanityCheck(const GlobalState &gs) const {
    sanityCheckProxyType(gs, underlying(gs));
    ENFORCE(this->values.size() == this->keys.size());
    for (auto &v : this->keys) {
        v.sanityCheck(gs);
    }
    for (auto &e : this->values) {
        e.sanityCheck(gs);
    }
}

AliasType::AliasType(SymbolRef other) : symbol(other) {
    recordAllocatedType("aliastype");
}

void AndType::_sanityCheck(const GlobalState &gs) const {
    left.sanityCheck(gs);
    right.sanityCheck(gs);
    /*
     * This is no longer true. Now we can construct types such as:
     * ShapeType(1 => 1), AppliedType{Array, Integer}
       ENFORCE(!isa_type<ProxyType>(left.get()));
       ENFORCE(!isa_type<ProxyType>(right.get()));

       */

    ENFORCE(!left.isUntyped());
    ENFORCE(!right.isUntyped());
    // TODO: reenable
    //    ENFORCE(!Types::isSubType(gs, left, right),
    //            this->toString(gs) + " should have collapsed: " + left->toString(gs) + " <: " +
    //            right->toString(gs));
    //    ENFORCE(!Types::isSubType(gs, right, left),
    //            this->toString(gs) + " should have collapsed: " + right->toString(gs) + " <: " +
    //            left->toString(gs));
}

void OrType::_sanityCheck(const GlobalState &gs) const {
    left.sanityCheck(gs);
    right.sanityCheck(gs);
    //    ENFORCE(!isa_type<ProxyType>(left.get()));
    //    ENFORCE(!isa_type<ProxyType>(right.get()));
    ENFORCE(!left.isUntyped());
    ENFORCE(!right.isUntyped());
    //  TODO: @dmitry, reenable
    //    ENFORCE(!Types::isSubType(gs, left, right),
    //            this->toString(gs) + " should have collapsed: " + left->toString(gs) + " <: " +
    //            right->toString(gs));
    //    ENFORCE(!Types::isSubType(gs, right, left),
    //            this->toString(gs) + " should have collapsed: " + right->toString(gs) + " <: " +
    //            left->toString(gs));
}

void ClassType::_sanityCheck(const GlobalState &gs) const {
    ENFORCE(this->symbol.exists());
}

void AliasType::_sanityCheck(const GlobalState &gs) const {
    ENFORCE(this->symbol.exists());
}

void MetaType::_sanityCheck(const GlobalState &gs) const {
    ENFORCE(!core::isa_type<MetaType>(wrapped));
    this->wrapped.sanityCheck(gs);
}

/** Returns type parameters of what reordered in the order of type parameters of asIf
 * If some typeArgs are not present, return NoSymbol
 * */
InlinedVector<TypeMemberRef, 4> Types::alignBaseTypeArgs(const GlobalState &gs, ClassOrModuleRef what,
                                                         absl::Span<const TypePtr> targs, ClassOrModuleRef asIf) {
    ENFORCE(what == asIf || what.data(gs)->derivesFrom(gs, asIf) || asIf.data(gs)->derivesFrom(gs, what),
            "what={} asIf={}", what.data(gs)->name.showRaw(gs), asIf.data(gs)->name.showRaw(gs));
    InlinedVector<TypeMemberRef, 4> currentAlignment;
    if (targs.empty()) {
        return currentAlignment;
    }

    if (what == asIf || (asIf.data(gs)->isClass() && what.data(gs)->isClass() &&
                         asIf.data(gs)->typeMembers().size() == what.data(gs)->typeMembers().size())) {
        auto members = what.data(gs)->typeMembers();
        currentAlignment.assign(members.begin(), members.end());
    } else {
        currentAlignment.reserve(asIf.data(gs)->typeMembers().size());
        for (auto originalTp : asIf.data(gs)->typeMembers()) {
            auto name = originalTp.data(gs)->name;
            SymbolRef align;
            for (auto x : what.data(gs)->typeMembers()) {
                if (x.data(gs)->name == name) {
                    align = x;
                    currentAlignment.emplace_back(x);
                    break;
                }
            }
            if (!align.exists()) {
                currentAlignment.emplace_back(Symbols::noTypeMember());
            }
        }
    }
    return currentAlignment;
}

/**
 * fromWhat - where the generic type was written
 * inWhat   - where the generic type is observed
 *
 * selfType - Every class has an "implicit" type member corresponding to itself.
 *            This parameter is the recursion: we pass ourself down once we know what ourself is.
 */
TypePtr Types::resultTypeAsSeenFrom(const GlobalState &gs, const TypePtr &what, ClassOrModuleRef fromWhat,
                                    ClassOrModuleRef inWhat, const vector<TypePtr> &targs, TypePtr selfType) {
    // requires_ancestor leads to dispatches that target some other symbol entirely.
    // There is no guarantee that we have redeclared the other method's type members.
    // This is yet another case where requires ancestor is just broken.
    ENFORCE(inWhat == fromWhat || inWhat.data(gs)->derivesFrom(gs, fromWhat) ||
                fromWhat.data(gs)->derivesFrom(gs, inWhat) || gs.cacheSensitiveOptions.requiresAncestorEnabled,
            "\n{}\nis unrelated to\n\n{}", fromWhat.toString(gs), inWhat.toString(gs));

    auto originalOwner = fromWhat;

    if (what == nullptr) {
        return what;
    }

    InstantiationContext ictx(originalOwner, inWhat, targs, selfType);
    auto result = what._instantiateLambdaParams(gs, ictx);
    if (result != nullptr) {
        return result;
    }
    return what;
}

TypePtr Types::resultTypeAsSeenFromSelf(const GlobalState &gs, const TypePtr &what, ClassOrModuleRef self) {
    if (what == nullptr) {
        return what;
    }

    InstantiationContext ictx(self, self);
    auto result = what._instantiateLambdaParams(gs, ictx);
    if (result != nullptr) {
        return result;
    }
    return what;
}

TypePtr Types::getProcReturnType(const GlobalState &gs, const TypePtr &procType) {
    if (!procType.derivesFrom(gs, Symbols::Proc())) {
        return Types::untypedUntracked();
    }
    auto applied = cast_type<AppliedType>(procType);
    if (applied == nullptr || applied->targs.empty()) {
        return Types::untypedUntracked();
    }
    // Proc types have their return type as the first targ
    return applied->targs.front();
}

bool ClassType::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    if (symbol == Symbols::untyped() || symbol == klass) {
        return true;
    }
    return symbol.data(gs)->derivesFrom(gs, klass);
}

bool OrType::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    return left.derivesFrom(gs, klass) && right.derivesFrom(gs, klass);
}

bool AndType::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    return left.derivesFrom(gs, klass) || right.derivesFrom(gs, klass);
}

bool AliasType::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    Exception::raise("AliasType.derivesfrom");
}

bool TypeVar::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    Exception::raise("should never happen. You're missing a call to either Types::approximateTypeVars or "
                     "Types::instantiateTypeVars");
}

MetaType::MetaType(const TypePtr &wrapped) : wrapped(move(wrapped)) {
    recordAllocatedType("metattype");
}

bool MetaType::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    return false;
}

TypeVar::TypeVar(TypeParameterRef sym) : sym(sym) {
    recordAllocatedType("typevar");
}

void TypeVar::_sanityCheck(const GlobalState &gs) const {
    ENFORCE(this->sym.exists());
}

void AppliedType::_sanityCheck(const GlobalState &gs) const {
    ENFORCE(this->klass != Symbols::untyped());

    ENFORCE(this->klass.data(gs)->typeMembers().size() == this->targs.size() ||
                (this->klass == Symbols::Array() && (this->targs.size() == 1)) ||
                (this->klass == Symbols::Hash() && (this->targs.size() == 3)) ||
                this->klass.id() >= Symbols::Proc0().id() && this->klass.id() <= Symbols::last_proc().id(),
            "{}", this->klass.data(gs)->name.showRaw(gs));
    for (auto &targ : this->targs) {
        targ.sanityCheck(gs);
    }
}

bool AppliedType::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    ClassType und(this->klass);
    return und.derivesFrom(gs, klass);
}

LambdaParam::LambdaParam(const TypeMemberRef definition, TypePtr lowerBound, TypePtr upperBound)
    : definition(definition), lowerBound(lowerBound), upperBound(upperBound) {
    recordAllocatedType("lambdatypeparam");
}

SelfTypeParam::SelfTypeParam(const SymbolRef definition) : definition(definition) {
    recordAllocatedType("selftypeparam");
}

NewSelfType::NewSelfType(const TypePtr &upperBound) : upperBound(move(upperBound)) {
    recordAllocatedType("selftype");
}

bool LambdaParam::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    Exception::raise(
        "LambdaParam::derivesFrom not implemented, not clear what it should do. Let's see this fire first.");
}

bool SelfTypeParam::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    if (auto lambdaParam = cast_type<LambdaParam>(this->definition.resultType(gs))) {
        return lambdaParam->upperBound.derivesFrom(gs, klass);
    } else {
        return false;
    }
}

bool NewSelfType::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    return false;
}

void LambdaParam::_sanityCheck(const GlobalState &gs) const {}
void SelfTypeParam::_sanityCheck(const GlobalState &gs) const {
    ENFORCE(definition.isTypeMember() || definition.isTypeParameter());
}

void NewSelfType::_sanityCheck(const GlobalState &gs) const {
    ENFORCE(this->upperBound != nullptr);
    ENFORCE(!this->upperBound.isUntyped());
    ENFORCE(this->upperBound.isFullyDefined());
}

TypePtr OrType::make_shared(const TypePtr &left, const TypePtr &right) {
    TypePtr res(TypePtr::Tag::OrType, new OrType(left, right));
    return res;
}

TypePtr AndType::make_shared(const TypePtr &left, const TypePtr &right) {
    TypePtr res(TypePtr::Tag::AndType, new AndType(left, right));
    return res;
}

SendAndBlockLink::SendAndBlockLink(NameRef fun, LocOffsets loc, vector<ParamInfo::Flags> &&paramFlags)
    : paramFlags(move(paramFlags)), loc(loc), fun(fun) {}

optional<int> SendAndBlockLink::fixedArity() const {
    optional<int> arity = 0;
    for (auto &arg : paramFlags) {
        if (arg.isKeyword || arg.isDefault || arg.isRepeated) {
            arity = nullopt;
            break;
        }
        arity = *arity + 1;
    }
    return arity;
}

TypePtr TupleType::elementType(const GlobalState &gs) const {
    auto underlying = this->underlying(gs);
    auto ap = cast_type<AppliedType>(underlying);
    ENFORCE(ap);
    ENFORCE(ap->klass == Symbols::Array());
    ENFORCE(ap->targs.size() == 1);
    return ap->targs.front();
}

SelfType::SelfType() {
    recordAllocatedType("selftype");
};

AppliedType::AppliedType(ClassOrModuleRef klass, vector<TypePtr> targs) : klass(klass), targs(std::move(targs)) {
    recordAllocatedType("appliedtype");
    histogramInc("appliedtype.targs", this->targs.size());
}

bool SelfType::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    Exception::raise("Should never call `derivesFrom` on a SelfType");
}

void SelfType::_sanityCheck(const GlobalState &gs) const {}

TypePtr Types::widen(const GlobalState &gs, const TypePtr &type) {
    ENFORCE(type != nullptr);
    TypePtr ret;
    typecase(
        type, [&](const AndType &andType) { ret = all(gs, widen(gs, andType.left), widen(gs, andType.right)); },
        [&](const OrType &orType) { ret = any(gs, widen(gs, orType.left), widen(gs, orType.right)); },
        [&](const AppliedType &appliedType) {
            vector<TypePtr> newTargs;
            newTargs.reserve(appliedType.targs.size());
            for (const auto &t : appliedType.targs) {
                newTargs.emplace_back(widen(gs, t));
            }
            ret = make_type<AppliedType>(appliedType.klass, move(newTargs));
        },
        [&](const TypePtr &) {
            if (is_proxy_type(type)) {
                ret = Types::widen(gs, type.underlying(gs));
            } else {
                ret = type;
            }
        });
    ENFORCE(ret);
    return ret;
}

namespace {
vector<TypePtr> unwrapTypeVector(Context ctx, const vector<TypePtr> &elems) {
    vector<TypePtr> unwrapped;
    unwrapped.reserve(elems.size());
    for (auto &e : elems) {
        unwrapped.emplace_back(Types::unwrapSelfTypeParam(ctx, e));
    }
    return unwrapped;
}
} // namespace

// This helper is only used in type_syntax.cc after wrapping up types.
//
// (Type syntax parsing wraps up what would normally be LambdaParam with SelfTypeParam so that it
// can use `Types::any` and `Types::all` to minimize types during parsing, but that requires
// fully-defined types.)
TypePtr Types::unwrapSelfTypeParam(Context ctx, const TypePtr &type) {
    ENFORCE(type != nullptr);

    TypePtr ret;
    typecase(
        type, [&](const ClassType &klass) { ret = type; }, [&](const TypeVar &tv) { ret = type; },
        [&](const LambdaParam &tv) { ret = type; }, [&](const NamedLiteralType &lit) { ret = type; },
        [&](const IntegerLiteralType &i) { ret = type; }, [&](const FloatLiteralType &i) { ret = type; },
        [&](const AndType &andType) {
            ret = AndType::make_shared(unwrapSelfTypeParam(ctx, andType.left), unwrapSelfTypeParam(ctx, andType.right));
        },
        [&](const OrType &orType) {
            ret = OrType::make_shared(unwrapSelfTypeParam(ctx, orType.left), unwrapSelfTypeParam(ctx, orType.right));
        },
        [&](const ShapeType &shape) { ret = make_type<ShapeType>(shape.keys, unwrapTypeVector(ctx, shape.values)); },
        [&](const TupleType &tuple) { ret = make_type<TupleType>(unwrapTypeVector(ctx, tuple.elems)); },
        [&](const MetaType &meta) { ret = make_type<MetaType>(unwrapSelfTypeParam(ctx, meta.wrapped)); },
        [&](const AppliedType &appliedType) {
            ret = make_type<AppliedType>(appliedType.klass, unwrapTypeVector(ctx, appliedType.targs));
        },
        [&](const SelfTypeParam &param) {
            auto sym = param.definition;
            if (sym.owner(ctx) == ctx.owner) {
                ENFORCE(isa_type<LambdaParam>(sym.resultType(ctx)));
                ret = sym.resultType(ctx);
            } else {
                ret = type;
            }
        },
        [&](const NewSelfType &self) { ret = core::Symbols::T_SelfType().data(ctx)->resultType; },
        [&](const TypePtr &tp) {
            if (type != nullptr) {
                Exception::raise("unwrapSelfTypeParam: unhandled case type={}", type.toString(ctx));
            } else {
                Exception::raise("unwrapSelfTypeParam: unhandled case type=nullptr");
            }
        });

    ENFORCE(ret != nullptr);

    return ret;
}

core::ClassOrModuleRef Types::getRepresentedClass(const GlobalState &gs, const TypePtr &ty) {
    if (!ty.derivesFrom(gs, core::Symbols::Module())) {
        return core::Symbols::noClassOrModule();
    }
    core::ClassOrModuleRef singleton;
    if (isa_type<ClassType>(ty)) {
        auto s = cast_type_nonnull<ClassType>(ty);
        singleton = s.symbol;
    } else {
        auto at = cast_type<AppliedType>(ty);
        if (at == nullptr) {
            return core::Symbols::noClassOrModule();
        }

        singleton = at->klass;
    }
    return singleton.data(gs)->attachedClass(gs);
}

TypePtr Types::unwrapType(const GlobalState &gs, Loc loc, const TypePtr &tp) {
    if (auto metaType = cast_type<MetaType>(tp)) {
        return metaType->wrapped;
    }

    if (isa_type<ClassType>(tp)) {
        auto classType = cast_type_nonnull<ClassType>(tp);
        if (classType.symbol.data(gs)->derivesFrom(gs, core::Symbols::T_Enum())) {
            // T::Enum instances are allowed to stand for themselves in type syntax positions.
            // See the note in type_syntax.cc regarding T::Enum.
            return tp;
        }

        auto attachedClass = classType.symbol.data(gs)->attachedClass(gs);
        if (!attachedClass.exists()) {
            if (auto e = gs.beginError(loc, errors::Infer::BareTypeUsage)) {
                e.setHeader("Unexpected bare `{}` value found in type position", tp.show(gs));
                if (classType.symbol == core::Symbols::T_Types_Base() ||
                    classType.symbol.data(gs)->derivesFrom(gs, core::Symbols::T_Types_Base())) {
                    // T::Types::Base is the parent class for runtime type objects.
                    // Give a more helpful error message
                    e.addErrorNote("Sorbet only allows statically-analyzable types in type positions.\n"
                                   "    To compute new runtime types, you must explicitly wrap with `{}`",
                                   "T.unsafe");
                    auto locSource = loc.source(gs);
                    if (locSource.has_value()) {
                        e.replaceWith("Wrap in `T.unsafe`", loc, "T.unsafe({})", locSource.value());
                    }
                }
            }

            return Types::untypedUntracked();
        }

        return attachedClass.data(gs)->externalType();
    }

    if (auto appType = cast_type<AppliedType>(tp)) {
        ClassOrModuleRef attachedClass = appType->klass.data(gs)->attachedClass(gs);
        if (!attachedClass.exists()) {
            if (auto e = gs.beginError(loc, errors::Infer::BareTypeUsage)) {
                e.setHeader("Unexpected bare `{}` value found in type position", tp.show(gs));
            }
            return Types::untypedUntracked();
        }

        return attachedClass.data(gs)->externalType();
    }

    if (auto shapeType = cast_type<ShapeType>(tp)) {
        vector<TypePtr> unwrappedValues;
        unwrappedValues.reserve(shapeType->values.size());
        for (auto &value : shapeType->values) {
            unwrappedValues.emplace_back(unwrapType(gs, loc, value));
        }
        return make_type<ShapeType>(shapeType->keys, move(unwrappedValues));
    } else if (auto tupleType = cast_type<TupleType>(tp)) {
        vector<TypePtr> unwrappedElems;
        unwrappedElems.reserve(tupleType->elems.size());
        for (auto &elem : tupleType->elems) {
            unwrappedElems.emplace_back(unwrapType(gs, loc, elem));
        }
        return make_type<TupleType>(move(unwrappedElems));
    }

    if (auto e = gs.beginError(loc, errors::Infer::BareTypeUsage)) {
        e.setHeader("Unexpected bare `{}` value found in type position", tp.show(gs));
    }
    return Types::untypedUntracked();
}

// This method is actually special: not only is it called from dispatchCall in calls.cc, it's
// also called directly by type_syntax parsing in resolver (because this method checks some
// invariants of generics that we want to hold even in `typed: false` files).
//
// Unfortunately, this means that some errors are double reported (once by resolver, and then
// again by infer).

TypePtr Types::applyTypeArguments(const GlobalState &gs, const CallLocs &locs, uint16_t numPosArgs,
                                  const InlinedVector<const TypeAndOrigins *, 2> &args, ClassOrModuleRef genericClass,
                                  ErrorClass genericArgumentCountMismatchError,
                                  ErrorClass genericArgumentKeywordArgsError) {
    genericClass = genericClass.maybeUnwrapBuiltinGenericForwarder();

    int arity;
    if (genericClass == Symbols::Hash()) {
        arity = 2;
    } else {
        arity = genericClass.data(gs)->typeArity(gs);
    }

    // This is something like Generic[T1,...,foo: bar...]
    auto numKwArgs = args.size() - numPosArgs;
    if (numKwArgs > 0) {
        auto begin = locs.args[numPosArgs].beginPos();
        auto end = locs.args.back().endPos();
        core::Loc kwargsLoc{locs.file, begin, end};

        if (auto e = gs.beginError(kwargsLoc, genericArgumentKeywordArgsError)) {
            e.setHeader("Keyword arguments given to `{}`", genericClass.show(gs));
            // offer an autocorrect to turn the keyword args into a hash if there is no double-splat
            if (numKwArgs % 2 == 0 && kwargsLoc.exists()) {
                e.replaceWith("Wrap with braces", kwargsLoc, "{{{}}}", kwargsLoc.source(gs).value());
            }
        }
    }

    if (numPosArgs != arity || arity == 0) {
        auto squareBracketsLoc = core::Loc(locs.file, locs.fun.endPos(), locs.call.endPos());
        auto errLoc =
            !locs.args.empty() ? core::Loc(locs.file, locs.args.front().join(locs.args.back())) : squareBracketsLoc;
        if (auto e = gs.beginError(errLoc, genericArgumentCountMismatchError)) {
            if (arity == 0) {
                if (genericClass.data(gs)->typeMembers().empty()) {
                    e.setHeader("`{}` is not a generic class, but was given type parameters", genericClass.show(gs));
                } else {
                    e.setHeader("All type parameters for `{}` have already been fixed", genericClass.show(gs));
                }
                e.replaceWith("Remove square brackets", squareBracketsLoc, "");
            } else {
                e.setHeader("Wrong number of type parameters for `{}`. Expected: `{}`, got: `{}`",
                            genericClass.show(gs), arity, numPosArgs);
            }
        }
    }

    if (genericClass.data(gs)->typeMembers().empty()) {
        // TODO(neil): this should actually blame to an error recovery magic symbol
        return Types::untypedUntracked();
    }

    vector<TypePtr> targs;
    auto it = args.begin();
    int i = -1;
    targs.reserve(genericClass.data(gs)->typeMembers().size());
    for (auto mem : genericClass.data(gs)->typeMembers()) {
        ++i;

        auto memData = mem.data(gs);

        auto memType = cast_type<LambdaParam>(memData->resultType);
        ENFORCE(memType != nullptr);

        if (memData->flags.isFixed) {
            // Fixed args are implicitly applied, and won't consume type
            // arguments from the list that's supplied.
            targs.emplace_back(memType->upperBound);
        } else if (it != args.end()) {
            auto loc = core::Loc(locs.file, locs.args[it - args.begin()]);
            auto argType = unwrapType(gs, loc, (*it)->type);
            bool validBounds = true;

            // Validate type parameter bounds.
            ErrorSection::Collector errorDetailsCollector;
            if (!Types::isSubType(gs, argType, memType->upperBound, errorDetailsCollector)) {
                validBounds = false;
                if (auto e = gs.beginError(loc, errors::Resolver::GenericTypeParamBoundMismatch)) {
                    auto argStr = argType.show(gs);
                    e.setHeader("`{}` is not a subtype of upper bound of type member `{}`", argStr,
                                mem.showFullName(gs));
                    e.addErrorLine(memData->loc(), "`{}` is `{}` bounded by `{}` here", mem.showFullName(gs), "upper",
                                   memType->upperBound.show(gs));
                    e.addErrorSections(move(errorDetailsCollector));
                }
            }

            ErrorSection::Collector errorDetailsCollector2;
            if (!Types::isSubType(gs, memType->lowerBound, argType, errorDetailsCollector2)) {
                validBounds = false;

                if (auto e = gs.beginError(loc, errors::Resolver::GenericTypeParamBoundMismatch)) {
                    auto argStr = argType.show(gs);
                    e.setHeader("`{}` is not a supertype of lower bound of type member `{}`", argStr,
                                mem.showFullName(gs));
                    e.addErrorLine(memData->loc(), "`{}` is `{}` bounded by `{}` here", mem.showFullName(gs), "lower",
                                   memType->lowerBound.show(gs));
                    e.addErrorSections(move(errorDetailsCollector2));
                }
            }

            if (validBounds) {
                targs.emplace_back(argType);
            } else {
                // TODO(neil): this should actually blame to an error recovery magic symbol
                targs.emplace_back(Types::untypedUntracked());
            }

            ++it;
        } else if (genericClass == Symbols::Hash() && i == 2) {
            auto tupleArgs = targs;
            targs.emplace_back(make_type<TupleType>(tupleArgs));
        } else {
            // TODO(neil): this should actually blame to an error recovery magic symbol
            targs.emplace_back(Types::untypedUntracked());
        }
    }

    return make_type<MetaType>(make_type<AppliedType>(genericClass, move(targs)));
}

DispatchArgs::DispatchArgs(NameRef name, const CallLocs &locs, uint16_t numPosArgs,
                           InlinedVector<const TypeAndOrigins *, 2> &args, const TypePtr &selfType,
                           const TypeAndOrigins &fullType, const TypePtr &thisType, const SendAndBlockLink *block,
                           Loc originForUninitialized, bool isPrivateOk, bool suppressErrors,
                           NameRef enclosingMethodForSuper)
    : name(name), locs(locs), numPosArgs(numPosArgs), args(args), selfType(selfType), fullType(fullType),
      thisType(thisType), block(block), originForUninitialized(originForUninitialized), isPrivateOk(isPrivateOk),
      suppressErrors(suppressErrors), enclosingMethodForSuper(enclosingMethodForSuper) {}

Loc DispatchArgs::argsLoc(const GlobalState &gs) const {
    if (!locs.args.empty()) {
        // Note: there are some limitations here when the args themselves are parenthesized
        return core::Loc(locs.file, locs.args.front().join(locs.args.back()));
    }

    Loc result;
    if (this->block != nullptr) {
        if (!this->block->loc.exists()) {
            result = callLoc().copyEndWithZeroLength();
        } else if (!locs.fun.exists()) {
            result = core::Loc(locs.file, this->block->loc.copyWithZeroLength());
        } else {
            result = core::Loc(locs.file, funLoc().endPos(), this->block->loc.beginPos());
            if (result.copyEndWithZeroLength().adjust(gs, -1, 0).source(gs) == " ") {
                result = result.adjust(gs, 0, -1);
            }
        }
    } else {
        if (locs.fun.exists()) {
            result = core::Loc(locs.file, locs.fun.endPos(), locs.call.endPos());
        } else {
            result = callLoc().copyEndWithZeroLength();
        }
    }

    auto source = result.source(gs);
    if (!source.has_value()) {
        return result;
    }

    auto sourceView = source.value();
    if (sourceView.size() < 2) {
        return result;
    }

    if ((sourceView.front() == '(' || sourceView.front() == '[') &&
        (sourceView.back() == ')' || sourceView.back() == ']')) {
        result = result.adjust(gs, 1, -1);
    }

    return result;
}

DispatchArgs DispatchArgs::withSelfAndThisRef(const TypePtr &newSelfRef) const {
    return DispatchArgs{name,        locs,           numPosArgs,
                        args,        newSelfRef,     fullType,
                        newSelfRef,  block,          originForUninitialized,
                        isPrivateOk, suppressErrors, enclosingMethodForSuper};
}

DispatchArgs DispatchArgs::withThisRef(const TypePtr &newThisRef) const {
    return DispatchArgs{name,        locs,           numPosArgs,
                        args,        selfType,       fullType,
                        newThisRef,  block,          originForUninitialized,
                        isPrivateOk, suppressErrors, enclosingMethodForSuper};
}

DispatchResult DispatchResult::merge(const GlobalState &gs, DispatchResult::Combinator kind, DispatchResult &&left,
                                     DispatchResult &&right) {
    DispatchResult res;

    switch (kind) {
        case DispatchResult::Combinator::OR:
            res.returnType = Types::any(gs, left.returnType, right.returnType);
            break;

        case DispatchResult::Combinator::AND:
            res.returnType = Types::all(gs, left.returnType, right.returnType);
            break;
    }

    res.main = std::move(left.main);
    res.secondary = std::move(left.secondary);
    res.secondaryKind = kind;

    auto *it = &res;
    while (it->secondary != nullptr) {
        it = it->secondary.get();
    }

    it->secondary = make_unique<DispatchResult>(std::move(right));

    return res;
}

} // namespace sorbet::core
