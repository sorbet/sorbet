#include "core/Types.h"
#include "absl/base/casts.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include <utility>

#include "core/Types.h"

// improve debugging.
template class std::shared_ptr<sorbet::core::TypeConstraint>;
template class std::shared_ptr<sorbet::core::SendAndBlockLink>;
template class std::vector<sorbet::core::Loc>;

namespace sorbet::core {

using namespace std;

TypePtr Types::dispatchCallWithoutBlock(const GlobalState &gs, const TypePtr &recv, DispatchArgs args) {
    auto dispatched = recv.dispatchCall(gs, move(args));
    auto link = &dispatched;
    while (link != nullptr) {
        for (auto &err : link->main.errors) {
            gs._error(move(err));
        }
        link = link->secondary.get();
    }
    return move(dispatched.returnType);
}

TypePtr Types::top() {
    static auto res = make_inline_type<ClassType>(Symbols::top());
    return res;
}

TypePtr Types::bottom() {
    static auto res = make_inline_type<ClassType>(Symbols::bottom());
    return res;
}

TypePtr Types::nilClass() {
    static auto res = make_inline_type<ClassType>(Symbols::NilClass());
    return res;
}

TypePtr Types::untypedUntracked() {
    static auto res = make_inline_type<ClassType>(Symbols::untyped());
    return res;
}

TypePtr Types::untyped(const sorbet::core::GlobalState &gs, sorbet::core::SymbolRef blame) {
    if (sorbet::debug_mode) {
        return make_inline_type<BlamedUntyped>(blame);
    } else {
        return untypedUntracked();
    }
}

TypePtr Types::void_() {
    static auto res = make_inline_type<ClassType>(Symbols::void_());
    return res;
}

TypePtr Types::trueClass() {
    static auto res = make_inline_type<ClassType>(Symbols::TrueClass());
    return res;
}

TypePtr Types::falseClass() {
    static auto res = make_inline_type<ClassType>(Symbols::FalseClass());
    return res;
}

TypePtr Types::Boolean() {
    static auto res = OrType::make_shared(trueClass(), falseClass());
    return res;
}

TypePtr Types::Integer() {
    static auto res = make_inline_type<ClassType>(Symbols::Integer());
    return res;
}

TypePtr Types::Float() {
    static auto res = make_inline_type<ClassType>(Symbols::Float());
    return res;
}

TypePtr Types::arrayOfUntyped() {
    static vector<TypePtr> targs{Types::untypedUntracked()};
    static auto res = make_type<AppliedType>(Symbols::Array(), targs);
    return res;
}

TypePtr Types::rangeOfUntyped() {
    static vector<TypePtr> targs{Types::untypedUntracked()};
    static auto res = make_type<AppliedType>(Symbols::Range(), targs);
    return res;
}

TypePtr Types::hashOfUntyped() {
    static vector<TypePtr> targs{Types::untypedUntracked(), Types::untypedUntracked(), Types::untypedUntracked()};
    static auto res = make_type<AppliedType>(Symbols::Hash(), targs);
    return res;
}

TypePtr Types::procClass() {
    static auto res = make_inline_type<ClassType>(Symbols::Proc());
    return res;
}

TypePtr Types::classClass() {
    static auto res = make_inline_type<ClassType>(Symbols::Class());
    return res;
}

TypePtr Types::declBuilderForProcsSingletonClass() {
    static auto res = make_inline_type<ClassType>(Symbols::DeclBuilderForProcsSingleton());
    return res;
}

TypePtr Types::String() {
    static auto res = make_inline_type<ClassType>(Symbols::String());
    return res;
}

TypePtr Types::Symbol() {
    static auto res = make_inline_type<ClassType>(Symbols::Symbol());
    return res;
}

TypePtr Types::Object() {
    static auto res = make_inline_type<ClassType>(Symbols::Object());
    return res;
}

TypePtr Types::falsyTypes() {
    static auto res = OrType::make_shared(Types::nilClass(), Types::falseClass());
    return res;
}

TypePtr Types::dropSubtypesOf(const GlobalState &gs, const TypePtr &from, SymbolRef klass) {
    TypePtr result;

    if (from.isUntyped()) {
        return from;
    }

    if (auto *o = cast_type_const<OrType>(from)) {
        auto lhs = dropSubtypesOf(gs, o->left, klass);
        auto rhs = dropSubtypesOf(gs, o->right, klass);
        if (lhs == o->left && rhs == o->right) {
            result = from;
        } else if (lhs.isBottom()) {
            result = rhs;
        } else if (rhs.isBottom()) {
            result = lhs;
        } else {
            result = OrType::make_shared(lhs, rhs);
        }
    } else if (auto *a = cast_type_const<AndType>(from)) {
        auto lhs = dropSubtypesOf(gs, a->left, klass);
        auto rhs = dropSubtypesOf(gs, a->right, klass);
        if (lhs != a->left || rhs != a->right) {
            result = Types::all(gs, lhs, rhs);
        } else {
            result = from;
        }
    } else if (isa_type<ClassType>(from)) {
        auto c = cast_inline_type_nonnull<ClassType>(from);
        auto cdata = c.symbol.data(gs);
        if (from.hasUntyped()) {
            result = from;
        } else if (c.symbol == klass || c.derivesFrom(gs, klass)) {
            result = Types::bottom();
        } else if (c.symbol.data(gs)->isClassOrModuleClass() && klass.data(gs)->isClassOrModuleClass() &&
                   !klass.data(gs)->derivesFrom(gs, c.symbol)) {
            // We have two classes (not modules), and if the class we're
            // removing doesn't derive from `c`, there's nothing to do,
            // because of ruby having single inheretance.
            result = from;
        } else if (cdata->isClassOrModuleSealed() &&
                   (cdata->isClassOrModuleAbstract() || cdata->isClassOrModuleModule())) {
            auto subclasses = cdata->sealedSubclassesToUnion(gs);
            ENFORCE(!Types::equiv(gs, subclasses, from), "sealedSubclassesToUnion about to cause infinte loop");
            result = dropSubtypesOf(gs, subclasses, klass);
        } else {
            result = from;
        }
    } else if (auto *c = cast_type_const<AppliedType>(from)) {
        if (c->klass == klass || c->derivesFrom(gs, klass)) {
            result = Types::bottom();
        } else {
            result = from;
        }
    } else if (is_proxy_type(from)) {
        if (dropSubtypesOf(gs, from.underlying(), klass).isBottom()) {
            result = Types::bottom();
        } else {
            result = from;
        }
    } else {
        result = from;
    }
    SLOW_ENFORCE(Types::isSubType(gs, result, from),
                 "dropSubtypesOf({}, {}) returned {}, which is not a subtype of the input", from.toString(gs),
                 klass.data(gs)->showFullName(gs), result.toString(gs));
    return result;
}

bool Types::canBeTruthy(const GlobalState &gs, const TypePtr &what) {
    if (auto *o = cast_type_const<OrType>(what)) {
        return canBeTruthy(gs, o->left) || canBeTruthy(gs, o->right);
    } else if (auto *a = cast_type_const<AndType>(what)) {
        return canBeTruthy(gs, a->left) && canBeTruthy(gs, a->right);
    } else if (isa_type<ClassType>(what)) {
        auto c = cast_inline_type_nonnull<ClassType>(what);
        auto sym = c.symbol;
        return sym == core::Symbols::untyped() ||
               (sym != core::Symbols::FalseClass() && sym != core::Symbols::NilClass());
    } else if (auto *c = cast_type_const<AppliedType>(what)) {
        auto sym = c->klass;
        return sym == core::Symbols::untyped() ||
               (sym != core::Symbols::FalseClass() && sym != core::Symbols::NilClass());
    } else if (is_proxy_type(what)) {
        return canBeTruthy(gs, what.underlying());
    } else {
        return true;
    }
}

bool Types::canBeFalsy(const GlobalState &gs, const TypePtr &what) {
    if (what.isUntyped()) {
        return true;
    }
    return Types::isSubType(gs, Types::falseClass(), what) ||
           Types::isSubType(gs, Types::nilClass(),
                            what); // check if inhabited by falsy values
}

TypePtr Types::approximateSubtract(const GlobalState &gs, const TypePtr &from, const TypePtr &what) {
    if (isa_type<ClassType>(what)) {
        auto c = cast_inline_type_nonnull<ClassType>(what);
        return Types::dropSubtypesOf(gs, from, c.symbol);
    } else if (auto *c = cast_type_const<AppliedType>(what)) {
        return Types::dropSubtypesOf(gs, from, c->klass);
    } else if (auto *o = cast_type_const<OrType>(what)) {
        return Types::approximateSubtract(gs, Types::approximateSubtract(gs, from, o->left), o->right);
    } else {
        return from;
    }
}

TypePtr Types::dropLiteral(const TypePtr &tp) {
    if (isa_type<LiteralType>(tp)) {
        auto a = cast_inline_type_nonnull<LiteralType>(tp);
        return a.underlying();
    }
    return tp;
}

TypePtr Types::lubAll(const GlobalState &gs, vector<TypePtr> &elements) {
    TypePtr acc = Types::bottom();
    for (auto &el : elements) {
        acc = Types::lub(gs, acc, el);
    }
    return acc;
}

TypePtr Types::arrayOf(const GlobalState &gs, const TypePtr &elem) {
    vector<TypePtr> targs{move(elem)};
    return make_type<AppliedType>(Symbols::Array(), targs);
}

TypePtr Types::rangeOf(const GlobalState &gs, const TypePtr &elem) {
    vector<TypePtr> targs{move(elem)};
    return make_type<AppliedType>(Symbols::Range(), targs);
}

TypePtr Types::hashOf(const GlobalState &gs, const TypePtr &elem) {
    vector<TypePtr> tupleArgs{Types::Symbol(), elem};
    vector<TypePtr> targs{Types::Symbol(), elem, TupleType::build(gs, tupleArgs)};
    return make_type<AppliedType>(Symbols::Hash(), targs);
}

TypePtr Types::dropNil(const GlobalState &gs, const TypePtr &from) {
    return Types::dropSubtypesOf(gs, from, Symbols::NilClass());
}

std::optional<int> Types::getProcArity(const AppliedType &type) {
    for (int i = 0; i <= Symbols::MAX_PROC_ARITY; i++) {
        if (type.klass == Symbols::Proc(i)) {
            return i;
        }
    }
    return std::nullopt;
}

ClassType::ClassType(SymbolRef symbol) : symbol(symbol) {
    categoryCounterInc("types.allocated", "classtype");
    ENFORCE(symbol.exists());
}

void ProxyType::_sanityCheck(const GlobalState &gs, const TypePtr &underlying) {
    ENFORCE(isa_type<ClassType>(underlying) || isa_type<AppliedType>(underlying));
    underlying.sanityCheck(gs);
}

LiteralType::LiteralType(int64_t val) : value(val), literalKind(LiteralTypeKind::Integer) {
    categoryCounterInc("types.allocated", "literaltype");
}

LiteralType::LiteralType(double val) : floatval(val), literalKind(LiteralTypeKind::Float) {
    categoryCounterInc("types.allocated", "literaltype");
}

LiteralType::LiteralType(SymbolRef klass, NameRef val)
    : value(val._id), literalKind(klass == Symbols::String() ? LiteralTypeKind::String : LiteralTypeKind::Symbol) {
    categoryCounterInc("types.allocated", "literaltype");
    ENFORCE(klass == Symbols::String() || klass == Symbols::Symbol());
}

LiteralType::LiteralType(bool val)
    : value(val ? 1 : 0), literalKind(val ? LiteralTypeKind::True : LiteralTypeKind::False) {
    categoryCounterInc("types.allocated", "literaltype");
}

TypePtr LiteralType::underlying() const {
    switch (literalKind) {
        case LiteralTypeKind::Integer:
            return Types::Integer();
        case LiteralTypeKind::Float:
            return Types::Float();
        case LiteralTypeKind::String:
            return Types::String();
        case LiteralTypeKind::Symbol:
            return Types::Symbol();
        case LiteralTypeKind::True:
            return Types::trueClass();
        case LiteralTypeKind::False:
            return Types::falseClass();
    }
    Exception::raise("should never be reached");
}

TupleType::TupleType(TypePtr underlying, vector<TypePtr> elements)
    : elems(move(elements)), underlying_(std::move(underlying)) {
    categoryCounterInc("types.allocated", "tupletype");
}

TypePtr TupleType::build(const GlobalState &gs, vector<TypePtr> elements) {
    TypePtr underlying = Types::arrayOf(gs, Types::dropLiteral(Types::lubAll(gs, elements)));
    return make_type<TupleType>(move(underlying), move(elements));
}

AndType::AndType(const TypePtr &left, const TypePtr &right) : left(move(left)), right(move(right)) {
    categoryCounterInc("types.allocated", "andtype");
}

bool LiteralType::equals(const LiteralType &rhs) const {
    if (this->value != rhs.value) {
        return false;
    }
    if (!isa_type<ClassType>(this->underlying()) || !isa_type<ClassType>(rhs.underlying())) {
        return false;
    }
    auto lklass = cast_inline_type_nonnull<ClassType>(this->underlying());
    auto rklass = cast_inline_type_nonnull<ClassType>(rhs.underlying());
    return lklass.symbol == rklass.symbol;
}

OrType::OrType(const TypePtr &left, const TypePtr &right) : left(move(left)), right(move(right)) {
    categoryCounterInc("types.allocated", "ortype");
}

void TupleType::_sanityCheck(const GlobalState &gs) const {
    ProxyType::_sanityCheck(gs, underlying());
    auto *applied = cast_type_const<AppliedType>(this->underlying());
    ENFORCE(applied);
    ENFORCE(applied->klass == Symbols::Array());
}

ShapeType::ShapeType() : underlying_(Types::hashOfUntyped()) {
    categoryCounterInc("types.allocated", "shapetype");
}

ShapeType::ShapeType(TypePtr underlying, vector<TypePtr> keys, vector<TypePtr> values)
    : keys(move(keys)), values(move(values)), underlying_(std::move(underlying)) {
    DEBUG_ONLY(for (auto &k : this->keys) { ENFORCE(isa_type<LiteralType>(k)); };);
    categoryCounterInc("types.allocated", "shapetype");
}

TypePtr ShapeType::underlying() const {
    return this->underlying_;
}

TypePtr TupleType::underlying() const {
    return this->underlying_;
}

void ShapeType::_sanityCheck(const GlobalState &gs) const {
    ProxyType::_sanityCheck(gs, underlying());
    ENFORCE(this->values.size() == this->keys.size());
    for (auto &v : this->keys) {
        v.sanityCheck(gs);
    }
    for (auto &e : this->values) {
        e.sanityCheck(gs);
    }
}

AliasType::AliasType(SymbolRef other) : symbol(other) {
    categoryCounterInc("types.allocated", "aliastype");
}

void AndType::_sanityCheck(const GlobalState &gs) const {
    left.sanityCheck(gs);
    right.sanityCheck(gs);
    /*
     * This is no longer true. Now we can construct types such as:
     * ShapeType(1 => 1), AppliedType{Array, Integer}
       ENFORCE(!is_proxy_type(left.get()));
       ENFORCE(!is_proxy_type(right.get()));

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
    //    ENFORCE(!is_proxy_type(left.get()));
    //    ENFORCE(!is_proxy_type(right.get()));
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

/** Returns type parameters of what reordered in the order of type parameters of asIf
 * If some typeArgs are not present, return NoSymbol
 * */
InlinedVector<SymbolRef, 4> Types::alignBaseTypeArgs(const GlobalState &gs, SymbolRef what,
                                                     const vector<TypePtr> &targs, SymbolRef asIf) {
    ENFORCE(asIf.data(gs)->isClassOrModule());
    ENFORCE(what.data(gs)->isClassOrModule());
    ENFORCE(what == asIf || what.data(gs)->derivesFrom(gs, asIf) || asIf.data(gs)->derivesFrom(gs, what),
            what.data(gs)->name.showRaw(gs), asIf.data(gs)->name.showRaw(gs));
    InlinedVector<SymbolRef, 4> currentAlignment;
    if (targs.empty()) {
        return currentAlignment;
    }

    if (what == asIf || (asIf.data(gs)->isClassOrModuleClass() && what.data(gs)->isClassOrModuleClass() &&
                         asIf.data(gs)->typeMembers().size() == what.data(gs)->typeMembers().size())) {
        currentAlignment = what.data(gs)->typeMembers();
    } else {
        currentAlignment.reserve(asIf.data(gs)->typeMembers().size());
        for (auto originalTp : asIf.data(gs)->typeMembers()) {
            auto name = originalTp.data(gs)->name;
            SymbolRef align;
            int i = 0;
            for (auto x : what.data(gs)->typeMembers()) {
                if (x.data(gs)->name == name) {
                    align = x;
                    currentAlignment.emplace_back(x);
                    break;
                }
                i++;
            }
            if (!align.exists()) {
                currentAlignment.emplace_back(Symbols::noSymbol());
            }
        }
    }
    return currentAlignment;
}

/**
 * fromWhat - where the generic type was written
 * inWhat   - where the generic type is observed
 */
TypePtr Types::resultTypeAsSeenFrom(const GlobalState &gs, const TypePtr &what, SymbolRef fromWhat, SymbolRef inWhat,
                                    const vector<TypePtr> &targs) {
    SymbolRef originalOwner = fromWhat;
    ENFORCE(fromWhat.data(gs)->isClassOrModule());
    ENFORCE(inWhat.data(gs)->isClassOrModule());

    // TODO: the ENFORCE below should be above this conditional, but there is
    // currently a problem with the handling of `module_function` that causes it
    // to fail reliably. https://github.com/sorbet/sorbet/issues/904
    if (originalOwner.data(gs)->typeMembers().empty() || (what == nullptr)) {
        return what;
    }

    ENFORCE(inWhat == fromWhat || inWhat.data(gs)->derivesFrom(gs, fromWhat) ||
                fromWhat.data(gs)->derivesFrom(gs, inWhat),
            "\n{}\nis unrelated to\n\n{}", fromWhat.data(gs)->toString(gs), inWhat.data(gs)->toString(gs));

    auto currentAlignment = alignBaseTypeArgs(gs, originalOwner, targs, inWhat);

    return instantiate(gs, what, currentAlignment, targs);
}

TypePtr Types::getProcReturnType(const GlobalState &gs, const TypePtr &procType) {
    if (!procType.derivesFrom(gs, Symbols::Proc())) {
        return Types::untypedUntracked();
    }
    auto *applied = cast_type_const<AppliedType>(procType);
    if (applied == nullptr || applied->targs.empty()) {
        return Types::untypedUntracked();
    }
    // Proc types have their return type as the first targ
    return applied->targs.front();
}

bool Types::isAsSpecificAs(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    return isSubTypeUnderConstraint(gs, TypeConstraint::EmptyFrozenConstraint, t1, t2, UntypedMode::AlwaysIncompatible);
}

bool Types::isSubType(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    return isSubTypeUnderConstraint(gs, TypeConstraint::EmptyFrozenConstraint, t1, t2, UntypedMode::AlwaysCompatible);
}

bool TypeVar::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    Exception::raise("should never happen. You're missing a call to either Types::approximate or Types::instantiate");
}

TypeVar::TypeVar(SymbolRef sym) : sym(sym) {
    categoryCounterInc("types.allocated", "typevar");
}

void TypeVar::_sanityCheck(const GlobalState &gs) const {
    ENFORCE(this->sym.exists());
}

void AppliedType::_sanityCheck(const GlobalState &gs) const {
    ENFORCE(this->klass.data(gs)->isClassOrModule());
    ENFORCE(this->klass != Symbols::untyped());

    ENFORCE(this->klass.data(gs)->typeMembers().size() == this->targs.size() ||
                (this->klass == Symbols::Array() && (this->targs.size() == 1)) ||
                (this->klass == Symbols::Hash() && (this->targs.size() == 3)) ||
                this->klass.classOrModuleIndex() >= Symbols::Proc0().classOrModuleIndex() &&
                    this->klass.classOrModuleIndex() <= Symbols::last_proc().classOrModuleIndex(),
            this->klass.data(gs)->name.showRaw(gs));
    for (auto &targ : this->targs) {
        targ.sanityCheck(gs);
    }
}

bool AppliedType::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    ClassType und(this->klass);
    return und.derivesFrom(gs, klass);
}

LambdaParam::LambdaParam(const SymbolRef definition, TypePtr lowerBound, TypePtr upperBound)
    : definition(definition), lowerBound(lowerBound), upperBound(upperBound) {
    categoryCounterInc("types.allocated", "lambdatypeparam");
}

SelfTypeParam::SelfTypeParam(const SymbolRef definition) : definition(definition) {
    categoryCounterInc("types.allocated", "selftypeparam");
}

bool LambdaParam::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    Exception::raise(
        "LambdaParam::derivesFrom not implemented, not clear what it should do. Let's see this fire first.");
}

bool SelfTypeParam::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    return false;
}

DispatchResult SelfTypeParam::dispatchCall(const GlobalState &gs, DispatchArgs args) const {
    auto untypedUntracked = Types::untypedUntracked();
    return untypedUntracked.dispatchCall(gs, args.withThisRef(untypedUntracked));
}

void LambdaParam::_sanityCheck(const GlobalState &gs) const {}
void SelfTypeParam::_sanityCheck(const GlobalState &gs) const {}

TypePtr OrType::make_shared(const TypePtr &left, const TypePtr &right) {
    TypePtr res(TypePtr::Tag::OrType, new OrType(left, right));
    return res;
}

TypePtr AndType::make_shared(const TypePtr &left, const TypePtr &right) {
    TypePtr res(TypePtr::Tag::AndType, new AndType(left, right));
    return res;
}

SendAndBlockLink::SendAndBlockLink(NameRef fun, vector<ArgInfo::ArgFlags> &&argFlags, int rubyBlockId)
    : argFlags(move(argFlags)), fun(fun), rubyBlockId(rubyBlockId) {}

shared_ptr<SendAndBlockLink> SendAndBlockLink::duplicate() {
    auto copy = *this;
    return make_shared<SendAndBlockLink>(move(copy));
}

optional<int> SendAndBlockLink::fixedArity() const {
    optional<int> arity = 0;
    for (auto &arg : argFlags) {
        if (arg.isKeyword || arg.isDefault || arg.isRepeated) {
            arity = std::nullopt;
            break;
        }
        arity = *arity + 1;
    }
    return arity;
}

TypePtr TupleType::elementType() const {
    auto *ap = cast_type_const<AppliedType>(this->underlying());
    ENFORCE(ap);
    ENFORCE(ap->klass == Symbols::Array());
    ENFORCE(ap->targs.size() == 1);
    return ap->targs.front();
}

SelfType::SelfType() {
    categoryCounterInc("types.allocated", "selftype");
};
AppliedType::AppliedType(SymbolRef klass, vector<TypePtr> targs) : klass(klass), targs(std::move(targs)) {
    categoryCounterInc("types.allocated", "appliedtype");
}

bool SelfType::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    Exception::raise("should never happen");
}

void SelfType::_sanityCheck(const GlobalState &gs) const {}

TypePtr Types::widen(const GlobalState &gs, const TypePtr &type) {
    ENFORCE(type != nullptr);
    TypePtr ret;
    if (auto *andType = cast_type_const<AndType>(type)) {
        ret = all(gs, widen(gs, andType->left), widen(gs, andType->right));
    } else if (auto *orType = cast_type_const<OrType>(type)) {
        ret = any(gs, widen(gs, orType->left), widen(gs, orType->right));
    } else if (is_proxy_type(type)) {
        ret = Types::widen(gs, type.underlying());
    } else if (auto *appliedType = cast_type_const<AppliedType>(type)) {
        vector<TypePtr> newTargs;
        newTargs.reserve(appliedType->targs.size());
        for (const auto &t : appliedType->targs) {
            newTargs.emplace_back(widen(gs, t));
        }
        ret = make_type<AppliedType>(appliedType->klass, newTargs);
    } else {
        ret = type;
    }
    ENFORCE(ret);
    return ret;
}

TypePtr Types::unwrapSelfTypeParam(Context ctx, const TypePtr &type) {
    ENFORCE(type != nullptr);

    TypePtr ret;

    switch (type.tag()) {
        case TypePtr::Tag::BlamedUntyped:
        case TypePtr::Tag::UnresolvedClassType:
        case TypePtr::Tag::UnresolvedAppliedType:
        case TypePtr::Tag::ClassType:
        case TypePtr::Tag::TypeVar:
        case TypePtr::Tag::LambdaParam:
        case TypePtr::Tag::SelfType:
        case TypePtr::Tag::LiteralType:
            return type;
        case TypePtr::Tag::AndType: {
            auto *andType = cast_type_const<AndType>(type);
            return AndType::make_shared(unwrapSelfTypeParam(ctx, andType->left),
                                        unwrapSelfTypeParam(ctx, andType->right));
        }
        case TypePtr::Tag::OrType: {
            auto *orType = cast_type_const<OrType>(type);
            return OrType::make_shared(unwrapSelfTypeParam(ctx, orType->left), unwrapSelfTypeParam(ctx, orType->right));
        }
        case TypePtr::Tag::ShapeType: {
            auto *shape = cast_type_const<ShapeType>(type);
            std::vector<TypePtr> values;
            values.reserve(shape->values.size());

            for (auto &value : shape->values) {
                values.emplace_back(unwrapSelfTypeParam(ctx, value));
            }

            return make_type<ShapeType>(unwrapSelfTypeParam(ctx, shape->underlying_), shape->keys, std::move(values));
        }
        case TypePtr::Tag::TupleType: {
            auto *tuple = cast_type_const<TupleType>(type);
            std::vector<TypePtr> elems;
            elems.reserve(tuple->elems.size());

            for (auto &value : tuple->elems) {
                elems.emplace_back(unwrapSelfTypeParam(ctx, value));
            }

            return make_type<TupleType>(unwrapSelfTypeParam(ctx, tuple->underlying_), std::move(elems));
        }
        case TypePtr::Tag::MetaType: {
            auto *meta = cast_type_const<MetaType>(type);
            return make_type<MetaType>(unwrapSelfTypeParam(ctx, meta->wrapped));
        }
        case TypePtr::Tag::AppliedType: {
            auto *appliedType = cast_type_const<AppliedType>(type);
            vector<TypePtr> newTargs;
            newTargs.reserve(appliedType->targs.size());
            for (const auto &t : appliedType->targs) {
                newTargs.emplace_back(unwrapSelfTypeParam(ctx, t));
            }
            return make_type<AppliedType>(appliedType->klass, newTargs);
        }
        case TypePtr::Tag::SelfTypeParam: {
            auto param = cast_inline_type_nonnull<SelfTypeParam>(type);
            auto sym = param.definition;
            if (sym.data(ctx)->owner == ctx.owner) {
                ENFORCE(isa_type<LambdaParam>(sym.data(ctx)->resultType));
                return sym.data(ctx)->resultType;
            } else {
                return type;
            }
        }
        case TypePtr::Tag::AliasType: {
            ENFORCE(false, "Unhandled case: {}", type.toString(ctx));
            Exception::notImplemented();
        }
    }
}

core::SymbolRef Types::getRepresentedClass(const GlobalState &gs, const TypePtr &ty) {
    if (!ty.derivesFrom(gs, core::Symbols::Module())) {
        return core::Symbols::noSymbol();
    }
    core::SymbolRef singleton;
    if (isa_type<ClassType>(ty)) {
        auto s = cast_inline_type_nonnull<ClassType>(ty);
        singleton = s.symbol;
    } else {
        auto *at = cast_type_const<AppliedType>(ty);
        if (at == nullptr) {
            return core::Symbols::noSymbol();
        }

        singleton = at->klass;
    }
    return singleton.data(gs)->attachedClass(gs);
}

DispatchArgs DispatchArgs::withSelfRef(const TypePtr &newSelfRef) {
    return DispatchArgs{name, locs, args, newSelfRef, fullType, newSelfRef, block};
}

DispatchArgs DispatchArgs::withThisRef(const TypePtr &newThisRef) {
    return DispatchArgs{name, locs, args, selfType, fullType, newThisRef, block};
}
} // namespace sorbet::core
