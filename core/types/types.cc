#include "core/Types.h"
#include "absl/base/casts.h"
#include "common/common.h"
#include "common/typecase.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include <utility>

#include "core/Types.h"

// improve debugging.
template class std::shared_ptr<sorbet::core::Type>;
template class std::shared_ptr<sorbet::core::TypeConstraint>;
template class std::shared_ptr<sorbet::core::SendAndBlockLink>;
template class std::vector<sorbet::core::Loc>;

namespace sorbet::core {

using namespace std;

TypePtr Types::dispatchCallWithoutBlock(const GlobalState &gs, const TypePtr &recv, DispatchArgs args) {
    auto dispatched = recv->dispatchCall(gs, move(args));
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
    static auto res = make_type<ClassType>(Symbols::top());
    return res;
}

TypePtr Types::bottom() {
    static auto res = make_type<ClassType>(Symbols::bottom());
    return res;
}

TypePtr Types::nilClass() {
    static auto res = make_type<ClassType>(Symbols::NilClass());
    return res;
}

TypePtr Types::untypedUntracked() {
    static auto res = make_type<ClassType>(Symbols::untyped());
    return res;
}

TypePtr Types::untyped(const sorbet::core::GlobalState &gs, sorbet::core::SymbolRef blame) {
    if (sorbet::debug_mode) {
        return make_type<BlamedUntyped>(blame);
    } else {
        return untypedUntracked();
    }
}

TypePtr Types::void_() {
    static auto res = make_type<ClassType>(Symbols::void_());
    return res;
}

TypePtr Types::trueClass() {
    static auto res = make_type<ClassType>(Symbols::TrueClass());
    return res;
}

TypePtr Types::falseClass() {
    static auto res = make_type<ClassType>(Symbols::FalseClass());
    return res;
}

TypePtr Types::Boolean() {
    static auto res = OrType::make_shared(trueClass(), falseClass());
    return res;
}

TypePtr Types::Integer() {
    static auto res = make_type<ClassType>(Symbols::Integer());
    return res;
}

TypePtr Types::Float() {
    static auto res = make_type<ClassType>(Symbols::Float());
    return res;
}

TypePtr Types::arrayOfUntyped() {
    static vector<TypePtr> targs{Types::untypedUntracked()};
    static auto res = make_type<AppliedType>(Symbols::Array(), move(targs));
    return res;
}

TypePtr Types::rangeOfUntyped() {
    static vector<TypePtr> targs{Types::untypedUntracked()};
    static auto res = make_type<AppliedType>(Symbols::Range(), move(targs));
    return res;
}

TypePtr Types::hashOfUntyped() {
    static vector<TypePtr> targs{Types::untypedUntracked(), Types::untypedUntracked(), Types::untypedUntracked()};
    static auto res = make_type<AppliedType>(Symbols::Hash(), move(targs));
    return res;
}

TypePtr Types::procClass() {
    static auto res = make_type<ClassType>(Symbols::Proc());
    return res;
}

TypePtr Types::classClass() {
    static auto res = make_type<ClassType>(Symbols::Class());
    return res;
}

TypePtr Types::declBuilderForProcsSingletonClass() {
    static auto res = make_type<ClassType>(Symbols::DeclBuilderForProcsSingleton());
    return res;
}

TypePtr Types::String() {
    static auto res = make_type<ClassType>(Symbols::String());
    return res;
}

TypePtr Types::Symbol() {
    static auto res = make_type<ClassType>(Symbols::Symbol());
    return res;
}

TypePtr Types::Object() {
    static auto res = make_type<ClassType>(Symbols::Object());
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

    typecase(
        from,
        [&](const OrType &o) {
            auto lhs = dropSubtypesOf(gs, o.left, klass);
            auto rhs = dropSubtypesOf(gs, o.right, klass);
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
            auto lhs = dropSubtypesOf(gs, a.left, klass);
            auto rhs = dropSubtypesOf(gs, a.right, klass);
            if (lhs != a.left || rhs != a.right) {
                result = Types::all(gs, lhs, rhs);
            } else {
                result = from;
            }
        },
        [&](const ClassType &c) {
            auto cdata = c.symbol.data(gs);
            if (c.hasUntyped()) {
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
        },
        [&](const AppliedType &c) {
            if (c.klass == klass || c.derivesFrom(gs, klass)) {
                result = Types::bottom();
            } else {
                result = from;
            }
        },
        [&](const ProxyType &c) {
            if (dropSubtypesOf(gs, c.underlying(), klass).isBottom()) {
                result = Types::bottom();
            } else {
                result = from;
            }
        },
        [&](const TypePtr &) { result = from; });
    SLOW_ENFORCE(Types::isSubType(gs, result, from),
                 "dropSubtypesOf({}, {}) returned {}, which is not a subtype of the input", from->toString(gs),
                 klass.data(gs)->showFullName(gs), result->toString(gs));
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
        [&](const ProxyType &c) { isTruthy = canBeTruthy(gs, c.underlying()); },
        [&](const TypePtr &) { isTruthy = true; });

    return isTruthy;
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
    TypePtr result;
    typecase(
        what, [&](const ClassType &c) { result = Types::dropSubtypesOf(gs, from, c.symbol); },
        [&](const AppliedType &c) { result = Types::dropSubtypesOf(gs, from, c.klass); },
        [&](const OrType &o) {
            result = Types::approximateSubtract(gs, Types::approximateSubtract(gs, from, o.left), o.right);
        },
        [&](const TypePtr &) { result = from; });
    return result;
}

TypePtr Types::dropLiteral(const TypePtr &tp) {
    if (auto *a = cast_type<LiteralType>(tp)) {
        return a->underlying();
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
    return make_type<AppliedType>(Symbols::Array(), move(targs));
}

TypePtr Types::rangeOf(const GlobalState &gs, const TypePtr &elem) {
    vector<TypePtr> targs{move(elem)};
    return make_type<AppliedType>(Symbols::Range(), move(targs));
}

TypePtr Types::hashOf(const GlobalState &gs, const TypePtr &elem) {
    vector<TypePtr> tupleArgs{Types::Symbol(), elem};
    vector<TypePtr> targs{Types::Symbol(), elem, TupleType::build(gs, move(tupleArgs))};
    return make_type<AppliedType>(Symbols::Hash(), move(targs));
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

void ProxyType::_sanityCheck(const GlobalState &gs) {
    ENFORCE(isa_type<ClassType>(this->underlying()) || isa_type<AppliedType>(this->underlying()));
    this->underlying()->sanityCheck(gs);
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
    auto *lklass = cast_type<ClassType>(this->underlying());
    auto *rklass = cast_type<ClassType>(rhs.underlying());
    if (!lklass || !rklass) {
        return false;
    }
    return lklass->symbol == rklass->symbol;
}

OrType::OrType(const TypePtr &left, const TypePtr &right) : left(move(left)), right(move(right)) {
    categoryCounterInc("types.allocated", "ortype");
}

void TupleType::_sanityCheck(const GlobalState &gs) {
    ProxyType::_sanityCheck(gs);
    auto *applied = cast_type<AppliedType>(this->underlying());
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

void ShapeType::_sanityCheck(const GlobalState &gs) {
    ProxyType::_sanityCheck(gs);
    ENFORCE(this->values.size() == this->keys.size());
    for (auto &v : this->keys) {
        v->_sanityCheck(gs);
    }
    for (auto &e : this->values) {
        e->_sanityCheck(gs);
    }
}

AliasType::AliasType(SymbolRef other) : symbol(other) {
    categoryCounterInc("types.allocated", "aliastype");
}

void AndType::_sanityCheck(const GlobalState &gs) {
    left->_sanityCheck(gs);
    right->_sanityCheck(gs);
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

void OrType::_sanityCheck(const GlobalState &gs) {
    left->_sanityCheck(gs);
    right->_sanityCheck(gs);
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

void ClassType::_sanityCheck(const GlobalState &gs) {
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
    if (!procType->derivesFrom(gs, Symbols::Proc())) {
        return Types::untypedUntracked();
    }
    auto *applied = cast_type<AppliedType>(procType);
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

void TypeVar::_sanityCheck(const GlobalState &gs) {
    ENFORCE(this->sym.exists());
}

void AppliedType::_sanityCheck(const GlobalState &gs) {
    ENFORCE(this->klass.data(gs)->isClassOrModule());
    ENFORCE(this->klass != Symbols::untyped());

    ENFORCE(this->klass.data(gs)->typeMembers().size() == this->targs.size() ||
                (this->klass == Symbols::Array() && (this->targs.size() == 1)) ||
                (this->klass == Symbols::Hash() && (this->targs.size() == 3)) ||
                this->klass.classOrModuleIndex() >= Symbols::Proc0().classOrModuleIndex() &&
                    this->klass.classOrModuleIndex() <= Symbols::last_proc().classOrModuleIndex(),
            this->klass.data(gs)->name.showRaw(gs));
    for (auto &targ : this->targs) {
        targ->sanityCheck(gs);
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

DispatchResult LambdaParam::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    Exception::raise(
        "LambdaParam::dispatchCall not implemented, not clear what it should do. Let's see this fire first.");
}

DispatchResult SelfTypeParam::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    auto untypedUntracked = Types::untypedUntracked();
    return untypedUntracked->dispatchCall(gs, args.withThisRef(untypedUntracked));
}

void LambdaParam::_sanityCheck(const GlobalState &gs) {}
void SelfTypeParam::_sanityCheck(const GlobalState &gs) {}

bool ClassType::hasUntyped() const {
    return this->symbol == Symbols::untyped();
}

bool OrType::hasUntyped() const {
    return left.hasUntyped() || right.hasUntyped();
}

TypePtr OrType::make_shared(const TypePtr &left, const TypePtr &right) {
    TypePtr res(TypePtr::Tag::OrType, new OrType(left, right));
    return res;
}

bool AndType::hasUntyped() const {
    return left.hasUntyped() || right.hasUntyped();
}

TypePtr AndType::make_shared(const TypePtr &left, const TypePtr &right) {
    TypePtr res(TypePtr::Tag::AndType, new AndType(left, right));
    return res;
}

bool AppliedType::hasUntyped() const {
    for (auto &arg : this->targs) {
        if (arg.hasUntyped()) {
            return true;
        }
    }
    return false;
}

bool TupleType::hasUntyped() const {
    for (auto &arg : this->elems) {
        if (arg.hasUntyped()) {
            return true;
        }
    }
    return false;
}

bool ShapeType::hasUntyped() const {
    for (auto &arg : this->values) {
        if (arg.hasUntyped()) {
            return true;
        }
    }
    return false;
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
    auto *ap = cast_type<AppliedType>(this->underlying());
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

DispatchResult SelfType::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    Exception::raise("should never happen");
}

void SelfType::_sanityCheck(const GlobalState &gs) {}

TypePtr Types::widen(const GlobalState &gs, const TypePtr &type) {
    ENFORCE(type != nullptr);
    TypePtr ret;
    typecase(
        type, [&](const AndType &andType) { ret = all(gs, widen(gs, andType.left), widen(gs, andType.right)); },
        [&](const OrType &orType) { ret = any(gs, widen(gs, orType.left), widen(gs, orType.right)); },
        [&](const ProxyType &proxy) { ret = Types::widen(gs, proxy.underlying()); },
        [&](const AppliedType &appliedType) {
            vector<TypePtr> newTargs;
            newTargs.reserve(appliedType.targs.size());
            for (const auto &t : appliedType.targs) {
                newTargs.emplace_back(widen(gs, t));
            }
            ret = make_type<AppliedType>(appliedType.klass, move(newTargs));
        },
        [&](const TypePtr &tp) { ret = type; });
    ENFORCE(ret);
    return ret;
}

TypePtr Types::unwrapSelfTypeParam(Context ctx, const TypePtr &type) {
    ENFORCE(type != nullptr);

    TypePtr ret;

    auto unwrapTypeVector = [&](const std::vector<TypePtr> &elems) -> std::vector<TypePtr> {
        std::vector<TypePtr> unwrapped;
        unwrapped.reserve(elems.size());
        for (auto &e : elems) {
            unwrapped.emplace_back(unwrapSelfTypeParam(ctx, e));
        }
        return unwrapped;
    };

    typecase(
        type, [&](const ClassType &klass) { ret = type; }, [&](const TypeVar &tv) { ret = type; },
        [&](const LambdaParam &tv) { ret = type; }, [&](const SelfType &self) { ret = type; },
        [&](const LiteralType &lit) { ret = type; },
        [&](const AndType &andType) {
            ret = AndType::make_shared(unwrapSelfTypeParam(ctx, andType.left), unwrapSelfTypeParam(ctx, andType.right));
        },
        [&](const OrType &orType) {
            ret = OrType::make_shared(unwrapSelfTypeParam(ctx, orType.left), unwrapSelfTypeParam(ctx, orType.right));
        },
        [&](const ShapeType &shape) {
            ret = make_type<ShapeType>(unwrapSelfTypeParam(ctx, shape.underlying_), shape.keys,
                                       unwrapTypeVector(shape.values));
        },
        [&](const TupleType &tuple) {
            ret = make_type<TupleType>(unwrapSelfTypeParam(ctx, tuple.underlying_), unwrapTypeVector(tuple.elems));
        },
        [&](const MetaType &meta) { ret = make_type<MetaType>(unwrapSelfTypeParam(ctx, meta.wrapped)); },
        [&](const AppliedType &appliedType) {
            ret = make_type<AppliedType>(appliedType.klass, unwrapTypeVector(appliedType.targs));
        },
        [&](const SelfTypeParam &param) {
            auto sym = param.definition;
            if (sym.data(ctx)->owner == ctx.owner) {
                ENFORCE(isa_type<LambdaParam>(sym.data(ctx)->resultType));
                ret = sym.data(ctx)->resultType;
            } else {
                ret = type;
            }
        },
        [&](const TypePtr &tp) {
            ENFORCE(false, "Unhandled case: {}", type->toString(ctx));
            Exception::notImplemented();
        });

    ENFORCE(ret != nullptr);

    return ret;
}

core::SymbolRef Types::getRepresentedClass(const GlobalState &gs, const TypePtr &ty) {
    if (!ty->derivesFrom(gs, core::Symbols::Module())) {
        return core::Symbols::noSymbol();
    }
    core::SymbolRef singleton;
    auto *s = cast_type<ClassType>(ty);
    if (s != nullptr) {
        singleton = s->symbol;
    } else {
        auto *at = cast_type<AppliedType>(ty);
        if (at == nullptr) {
            return core::Symbols::noSymbol();
        }

        singleton = at->klass;
    }
    return singleton.data(gs)->attachedClass(gs);
}

DispatchArgs DispatchArgs::withSelfRef(const TypePtr &newSelfRef) {
    return DispatchArgs{name, locs, numPosArgs, args, newSelfRef, fullType, newSelfRef, block, originForUninitialized};
}

DispatchArgs DispatchArgs::withThisRef(const TypePtr &newThisRef) {
    return DispatchArgs{name, locs, numPosArgs, args, selfType, fullType, newThisRef, block, originForUninitialized};
}
} // namespace sorbet::core
