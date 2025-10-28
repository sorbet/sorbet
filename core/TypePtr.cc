#include "core/TypePtr.h"
#include "common/has_member.h"
#include "core/Symbols.h"
#include "core/Types.h"
#include "core/hashing/hashing.h"

using namespace std;

#define CASE_STATEMENT(CASE_BODY, T) \
    case TypePtr::Tag::T: {          \
        CASE_BODY(T)                 \
        break;                       \
    }

#define GENERATE_TAG_SWITCH(tag, CASE_BODY)              \
    switch (tag) {                                       \
        CASE_STATEMENT(CASE_BODY, ClassType)             \
        CASE_STATEMENT(CASE_BODY, LambdaParam)           \
        CASE_STATEMENT(CASE_BODY, SelfTypeParam)         \
        CASE_STATEMENT(CASE_BODY, AliasType)             \
        CASE_STATEMENT(CASE_BODY, SelfType)              \
        CASE_STATEMENT(CASE_BODY, NamedLiteralType)      \
        CASE_STATEMENT(CASE_BODY, IntegerLiteralType)    \
        CASE_STATEMENT(CASE_BODY, FloatLiteralType)      \
        CASE_STATEMENT(CASE_BODY, TypeVar)               \
        CASE_STATEMENT(CASE_BODY, OrType)                \
        CASE_STATEMENT(CASE_BODY, AndType)               \
        CASE_STATEMENT(CASE_BODY, ShapeType)             \
        CASE_STATEMENT(CASE_BODY, TupleType)             \
        CASE_STATEMENT(CASE_BODY, AppliedType)           \
        CASE_STATEMENT(CASE_BODY, MetaType)              \
        CASE_STATEMENT(CASE_BODY, BlamedUntyped)         \
        CASE_STATEMENT(CASE_BODY, UnresolvedClassType)   \
        CASE_STATEMENT(CASE_BODY, UnresolvedAppliedType) \
    }

namespace sorbet::core {

namespace {
GENERATE_CALL_MEMBER(showWithMoreInfo, return self.show(std::forward<Args>(args)...),
                     std::declval<const GlobalState &>())

GENERATE_CALL_MEMBER(dispatchCall,
                     Exception::raise("should never happen: dispatchCall on {}",
                                      TypePtr::tagToString(TypePtr::TypeToTag<typename remove_const<T>::type>::value));
                     return DispatchResult{};, std::declval<const GlobalState &>(), std::declval<DispatchArgs>())

GENERATE_CALL_MEMBER(_instantiateTypeVars, return nullptr, std::declval<const GlobalState &>(),
                     std::declval<const TypeConstraint &>())

GENERATE_CALL_MEMBER(_replaceSelfType, return nullptr, declval<const GlobalState &>(), declval<const TypePtr &>())

GENERATE_CALL_MEMBER(_approximateTypeVars, return nullptr, declval<const GlobalState &>(),
                     declval<const TypeConstraint &>(), declval<core::Polarity>())

GENERATE_CALL_MEMBER(underlying,
                     Exception::raise("TypePtr::underlying called on non-proxy-type `{}`",
                                      TypePtr::tagToString(TypePtr::TypeToTag<typename remove_const<T>::type>::value));
                     return nullptr, std::declval<const GlobalState &>());

} // namespace

void TypePtr::deleteTagged(Tag tag, void *ptr) noexcept {
    ENFORCE_NO_TIMER(ptr != nullptr);

#define DELETE_TYPE(T) delete reinterpret_cast<T *>(ptr);

    GENERATE_TAG_SWITCH(tag, DELETE_TYPE)

#undef DELETE_TYPE
}

bool TypePtr::isUntyped() const {
    return isa_type<ClassType>(*this) && cast_type_nonnull<ClassType>(*this).symbol == Symbols::untyped();
}

bool TypePtr::isNilClass() const {
    return isa_type<ClassType>(*this) && cast_type_nonnull<ClassType>(*this).symbol == Symbols::NilClass();
}

bool TypePtr::isBottom() const {
    return isa_type<ClassType>(*this) && cast_type_nonnull<ClassType>(*this).symbol == Symbols::bottom();
}
bool TypePtr::isTop() const {
    return isa_type<ClassType>(*this) && cast_type_nonnull<ClassType>(*this).symbol == Symbols::top();
}

int TypePtr::kind() const {
    // This order is load bearing for the purpose of subtyping, because our type constraint
    // algorithm is greedy.
    //
    // For `glb` and `lub`, we sort the arguments based on their `kind`, so the first argument to
    // these functions effectively always has a lesser `kind`. This matters for the sake of ensuring
    // that `T.type_parameter` types show up at the right spots during the subtyping checks.
    switch (tag()) {
        case Tag::AppliedType:
            return 1;
        case Tag::BlamedUntyped:
        case Tag::UnresolvedAppliedType:
        case Tag::UnresolvedClassType:
        case Tag::ClassType:
            return 2;
        case Tag::IntegerLiteralType:
            return 3;
        case Tag::FloatLiteralType:
            return 4;
        case Tag::NamedLiteralType:
            return 5;
        case Tag::ShapeType:
            return 6;
        case Tag::TupleType:
            return 7;
        case Tag::LambdaParam:
        case Tag::SelfTypeParam:
            return 8;
        case Tag::MetaType:
            return 9;
        case Tag::TypeVar:
            return 10;
        case Tag::AliasType:
            return 11;
        case Tag::OrType:
            return 12;
        case Tag::AndType:
            return 13;
        case Tag::SelfType:
            return 14;
    }
}

string TypePtr::tagToString(Tag tag) {
#define TYPE_TO_STRING(T) return #T;
    GENERATE_TAG_SWITCH(tag, TYPE_TO_STRING)
#undef TYPE_TO_STRING
}

string TypePtr::typeName() const {
    return TypePtr::tagToString(tag());
}

bool TypePtr::isFullyDefined() const {
    switch (tag()) {
        // Base cases.
        case Tag::UnresolvedAppliedType:
        case Tag::UnresolvedClassType:
        case Tag::BlamedUntyped:
        case Tag::ClassType:
        case Tag::NamedLiteralType:
        case Tag::IntegerLiteralType:
        case Tag::FloatLiteralType:
        case Tag::AliasType:
        case Tag::SelfTypeParam:
        case Tag::MetaType: // MetaType: this is kinda true but kinda false. it's false for subtyping but true for
                            // inferencer.
            return true;

        case Tag::TypeVar:
        case Tag::LambdaParam:
        case Tag::SelfType:
            return false;

        // Composite types
        case Tag::ShapeType: {
            auto &shape = cast_type_nonnull<ShapeType>(*this);
            return absl::c_all_of(shape.values, [](const TypePtr &t) { return t.isFullyDefined(); });
        }
        case Tag::TupleType: {
            auto &tuple = cast_type_nonnull<TupleType>(*this);
            return absl::c_all_of(tuple.elems, [](const TypePtr &t) { return t.isFullyDefined(); });
        }
        case Tag::AndType: {
            auto &andType = cast_type_nonnull<AndType>(*this);
            return andType.left.isFullyDefined() && andType.right.isFullyDefined();
        }
        case Tag::OrType: {
            auto &orType = cast_type_nonnull<OrType>(*this);
            return orType.left.isFullyDefined() && orType.right.isFullyDefined();
        }
        case Tag::AppliedType: {
            auto &app = cast_type_nonnull<AppliedType>(*this);
            return absl::c_all_of(app.targs, [](const TypePtr &t) { return t.isFullyDefined(); });
        }
    }
}

bool TypePtr::hasUntyped() const {
    switch (tag()) {
        case Tag::TypeVar:
        case Tag::NamedLiteralType:
        case Tag::IntegerLiteralType:
        case Tag::FloatLiteralType:
        case Tag::SelfType:
        case Tag::AliasType:
        case Tag::SelfTypeParam:
        case Tag::LambdaParam:
        case Tag::MetaType:
            // These cannot have untyped.
            return false;

        case Tag::BlamedUntyped:
        case Tag::UnresolvedAppliedType:
        case Tag::UnresolvedClassType:
        case Tag::ClassType: {
            auto c = cast_type_nonnull<ClassType>(*this);
            return c.symbol == Symbols::untyped();
        }
        case Tag::OrType: {
            auto &o = cast_type_nonnull<OrType>(*this);
            return o.left.hasUntyped() || o.right.hasUntyped();
        }
        case Tag::AndType: {
            auto &a = cast_type_nonnull<AndType>(*this);
            return a.left.hasUntyped() || a.right.hasUntyped();
        }
        case Tag::AppliedType: {
            auto &app = cast_type_nonnull<AppliedType>(*this);
            return absl::c_any_of(app.targs, [](const TypePtr &t) { return t.hasUntyped(); });
        }
        case Tag::TupleType: {
            auto &tuple = cast_type_nonnull<TupleType>(*this);
            return absl::c_any_of(tuple.elems, [](const TypePtr &t) { return t.hasUntyped(); });
        }
        case Tag::ShapeType: {
            auto &shape = cast_type_nonnull<ShapeType>(*this);
            return absl::c_any_of(shape.values, [](const TypePtr &t) { return t.hasUntyped(); });
        }
    }
}

bool TypePtr::hasTopLevelVoid() const {
    switch (tag()) {
        case Tag::TypeVar:
        case Tag::NamedLiteralType:
        case Tag::IntegerLiteralType:
        case Tag::FloatLiteralType:
        case Tag::SelfType:
        case Tag::AliasType:
        case Tag::SelfTypeParam:
        case Tag::LambdaParam:
        case Tag::MetaType:
        case Tag::BlamedUntyped:
        case Tag::UnresolvedAppliedType:
        case Tag::UnresolvedClassType:
            // These cannot have void.
            return false;

        case Tag::AppliedType:
        case Tag::TupleType:
        case Tag::ShapeType:
            // Unlike hasUntyped, we do not look in type args for this method.
            return false;

        case Tag::ClassType: {
            auto c = cast_type_nonnull<ClassType>(*this);
            return c.symbol == Symbols::void_();
        }
        case Tag::OrType: {
            auto &o = cast_type_nonnull<OrType>(*this);
            return o.left.hasTopLevelVoid() || o.right.hasTopLevelVoid();
        }
        case Tag::AndType: {
            auto &a = cast_type_nonnull<AndType>(*this);
            return a.left.hasTopLevelVoid() || a.right.hasTopLevelVoid();
        }
    }
}

core::SymbolRef TypePtr::untypedBlame() const {
    ENFORCE(hasUntyped());
    if (isa_type<BlamedUntyped>(*this)) {
        return cast_type_nonnull<BlamedUntyped>(*this).blame;
    }
    return Symbols::noSymbol();
}

// Converts a type like this:
//   T.proc.params(arg0: Integer, arg1: Integer).void
// into this:
//   [Integer, Integer]
// for use with LoadYieldParams
//
// Currently, this method is never called with `name` set to anything other than `Names::call()`,
// which is where the `getCallArguments` comes from.
TypePtr TypePtr::getCallArguments(const GlobalState &gs, NameRef name) const {
    switch (tag()) {
        case Tag::MetaType:
        case Tag::TupleType:
        case Tag::ShapeType:
        case Tag::NamedLiteralType:
        case Tag::IntegerLiteralType:
        case Tag::FloatLiteralType: {
            return this->underlying(gs).getCallArguments(gs, name);
        }
        case Tag::OrType: {
            auto &orType = cast_type_nonnull<OrType>(*this);
            return orType.getCallArguments(gs, name);
        }
        case Tag::AndType: {
            auto &andType = cast_type_nonnull<AndType>(*this);
            return andType.getCallArguments(gs, name);
        }
        case Tag::BlamedUntyped: {
            auto c = cast_type_nonnull<BlamedUntyped>(*this);
            return c.getCallArguments(gs, name);
        }
        case Tag::UnresolvedClassType:
        case Tag::UnresolvedAppliedType:
        case Tag::ClassType: {
            auto c = cast_type_nonnull<ClassType>(*this);
            return c.getCallArguments(gs, name);
        }
        case Tag::AppliedType: {
            auto &app = cast_type_nonnull<AppliedType>(*this);
            return app.getCallArguments(gs, name);
        }
        case Tag::SelfType:
        case Tag::SelfTypeParam:
        case Tag::LambdaParam:
        case Tag::TypeVar:
        case Tag::AliasType: {
            Exception::raise("should never happen: getCallArguments on `{}`", typeName());
        }
    }
}

TypePtr TypePtr::_approximateTypeVars(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const {
#define _APPROXIMATE(T) \
    return CALL_MEMBER__approximateTypeVars<const T>::call(cast_type_nonnull<T>(*this), gs, tc, polarity);
    GENERATE_TAG_SWITCH(tag(), _APPROXIMATE)
#undef _APPROXIMATE
}

TypePtr TypePtr::_replaceSelfType(const GlobalState &gs, const TypePtr &receiver) const {
#define _REPLACE_SELF_TYPE(T) \
    return CALL_MEMBER__replaceSelfType<const T>::call(cast_type_nonnull<T>(*this), gs, receiver);
    GENERATE_TAG_SWITCH(tag(), _REPLACE_SELF_TYPE)
#undef _REPLACE_SELF_TYPE
}

TypePtr TypePtr::_instantiateTypeVars(const GlobalState &gs, const TypeConstraint &tc) const {
#define _INSTANTIATE(T) return CALL_MEMBER__instantiateTypeVars<const T>::call(cast_type_nonnull<T>(*this), gs, tc);
    GENERATE_TAG_SWITCH(tag(), _INSTANTIATE)
#undef _INSTANTIATE
}

void TypePtr::InstantiationContext::computeSelfTypeArgs(const GlobalState &gs) {
    ENFORCE_NO_TIMER(this->originalOwner.exists() && this->inWhat == this->originalOwner && !this->targs.has_value() &&
                     this->currentAlignment.empty());

    this->targsOwned = this->originalOwner.data(gs)->selfTypeArgs(gs);
    this->targs = absl::MakeSpan(this->targsOwned);
}

void TypePtr::InstantiationContext::computeAlignment(const GlobalState &gs) {
    ENFORCE_NO_TIMER(this->targs.has_value());
    this->currentAlignment = Types::alignBaseTypeArgs(gs, originalOwner, this->targs.value(), inWhat);
}

// Returns nullptr to indicate no change.
TypePtr TypePtr::_instantiateLambdaParams(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                                          const vector<TypePtr> &targs) const {
    switch (tag()) {
        case Tag::BlamedUntyped:
        case Tag::UnresolvedAppliedType:
        case Tag::UnresolvedClassType:
        case Tag::ClassType:
        case Tag::TypeVar:
        case Tag::NamedLiteralType:
        case Tag::IntegerLiteralType:
        case Tag::FloatLiteralType:
        case Tag::SelfTypeParam:
        case Tag::SelfType:
            // nullptr is a special value meaning that nothing changed (e.g., we didn't find a
            // LambdaParam that needed to be instantiated), so as an optimization the caller doesn't
            // have to allocate another type.
            return nullptr;

        case Tag::TupleType:
            return cast_type_nonnull<TupleType>(*this)._instantiateLambdaParams(gs, params, targs);
        case Tag::ShapeType:
            return cast_type_nonnull<ShapeType>(*this)._instantiateLambdaParams(gs, params, targs);
        case Tag::OrType:
            return cast_type_nonnull<OrType>(*this)._instantiateLambdaParams(gs, params, targs);
        case Tag::AndType:
            return cast_type_nonnull<AndType>(*this)._instantiateLambdaParams(gs, params, targs);
        case Tag::AppliedType:
            return cast_type_nonnull<AppliedType>(*this)._instantiateLambdaParams(gs, params, targs);
        case Tag::LambdaParam:
            return cast_type_nonnull<LambdaParam>(*this)._instantiateLambdaParams(gs, params, targs);

        case Tag::MetaType:
        case Tag::AliasType:
            Exception::raise("should never happen: _instantiateLambdaParams on `{}`", typeName());
    }
}

void TypePtr::_sanityCheck(const GlobalState &gs) const {
    // Not really a dynamic check, but this is a convenient place to put this to
    // ensure that all relevant types get checked.
#define SANITY_CHECK(T)                                                                   \
    static_assert(TypePtr::TypeToIsInlined<T>::value || !std::is_copy_constructible_v<T>, \
                  "non-inline types must not be copy-constructible");                     \
    break;
    GENERATE_TAG_SWITCH(tag(), SANITY_CHECK)
#undef SANITY_CHECK
#define SANITY_CHECK(T) return cast_type_nonnull<T>(*this)._sanityCheck(gs);
    GENERATE_TAG_SWITCH(tag(), SANITY_CHECK)
#undef SANITY_CHECK
}

string TypePtr::toStringWithTabs(const GlobalState &gs, int tabs) const {
#define TO_STRING_WITH_TABS(T) return cast_type_nonnull<T>(*this).toStringWithTabs(gs, tabs);
    GENERATE_TAG_SWITCH(tag(), TO_STRING_WITH_TABS)
#undef TO_STRING_WITH_TABS
}

uint32_t TypePtr::hash(const GlobalState &gs) const {
#define HASH(T) return cast_type_nonnull<T>(*this).hash(gs);
    GENERATE_TAG_SWITCH(tag(), HASH)
#undef HASH
}

string TypePtr::show(const GlobalState &gs, ShowOptions options) const {
#define SHOW(T) return cast_type_nonnull<T>(*this).show(gs, options);
    GENERATE_TAG_SWITCH(tag(), SHOW)
#undef SHOW
}

string TypePtr::showWithMoreInfo(const GlobalState &gs) const {
#define SHOW_WITH_MORE_INFO(T) return CALL_MEMBER_showWithMoreInfo<const T>::call(cast_type_nonnull<T>(*this), gs);
    GENERATE_TAG_SWITCH(tag(), SHOW_WITH_MORE_INFO)
#undef SHOW_WITH_MORE_INFO
}

bool TypePtr::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
#define DERIVES_FROM(T) return cast_type_nonnull<T>(*this).derivesFrom(gs, klass);
    GENERATE_TAG_SWITCH(tag(), DERIVES_FROM)
#undef DERIVES_FROM
}

DispatchResult TypePtr::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
#define DISPATCH_CALL(T) return CALL_MEMBER_dispatchCall<const T>::call(cast_type_nonnull<T>(*this), gs, args);
    GENERATE_TAG_SWITCH(tag(), DISPATCH_CALL)
#undef DISPATCH_CALL
}

TypePtr TypePtr::underlying(const GlobalState &gs) const {
    ENFORCE(is_proxy_type(*this), "underlying() only makes sense on a proxy type");
#define UNDERLYING(T) return CALL_MEMBER_underlying<const T>::call(cast_type_nonnull<T>(*this), gs);
    GENERATE_TAG_SWITCH(tag(), UNDERLYING)
#undef UNDERLYING
}

} // namespace sorbet::core
