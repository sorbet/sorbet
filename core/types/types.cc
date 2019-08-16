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

TypePtr Types::dispatchCallWithoutBlock(Context ctx, const TypePtr &recv, DispatchArgs args) {
    auto dispatched = recv->dispatchCall(ctx, move(args));
    auto link = &dispatched;
    while (link != nullptr) {
        for (auto &err : link->main.errors) {
            ctx.state._error(move(err));
        }
        link = link->secondary.get();
    }
    return move(dispatched.returnType);
}

TypePtr::TypePtr(shared_ptr<Type> &&store) : store(move(store)){};

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
    static auto res = make_type<AppliedType>(Symbols::Array(), targs);
    return res;
}

TypePtr Types::hashOfUntyped() {
    static vector<TypePtr> targs{Types::untypedUntracked(), Types::untypedUntracked(), Types::untypedUntracked()};
    static auto res = make_type<AppliedType>(Symbols::Hash(), targs);
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

TypePtr Types::dropSubtypesOf(Context ctx, const TypePtr &from, SymbolRef klass) {
    TypePtr result;

    if (from->isUntyped()) {
        return from;
    }

    typecase(
        from.get(),
        [&](OrType *o) {
            auto lhs = dropSubtypesOf(ctx, o->left, klass);
            auto rhs = dropSubtypesOf(ctx, o->right, klass);
            if (lhs == o->left && rhs == o->right) {
                result = from;
            } else if (lhs->isBottom()) {
                result = rhs;
            } else if (rhs->isBottom()) {
                result = lhs;
            } else {
                result = OrType::make_shared(lhs, rhs);
            }
        },
        [&](AndType *a) {
            auto lhs = dropSubtypesOf(ctx, a->left, klass);
            auto rhs = dropSubtypesOf(ctx, a->right, klass);
            if (lhs != a->left || rhs != a->right) {
                result = Types::all(ctx, lhs, rhs);
            } else {
                result = from;
            }
        },
        [&](ClassType *c) {
            auto cdata = c->symbol.data(ctx);
            if (c->isUntyped()) {
                result = from;
            } else if (cdata->isClassSealed() && (cdata->isClassAbstract() || cdata->isClassModule())) {
                result = dropSubtypesOf(ctx, cdata->sealedSubclassesToUnion(ctx), klass);
            } else if (c->symbol == klass || c->derivesFrom(ctx, klass)) {
                result = Types::bottom();
            } else {
                result = from;
            }
        },
        [&](AppliedType *c) {
            if (c->klass == klass || c->derivesFrom(ctx, klass)) {
                result = Types::bottom();
            } else {
                result = from;
            }
        },
        [&](ProxyType *c) {
            if (dropSubtypesOf(ctx, c->underlying(), klass)->isBottom()) {
                result = Types::bottom();
            } else {
                result = from;
            }
        },
        [&](Type *) { result = from; });
    ENFORCE(Types::isSubType(ctx, result, from),
            "dropSubtypesOf({}, {}) returned {}, which is not a subtype of the input", from->toString(ctx),
            klass.data(ctx)->showFullName(ctx), result->toString(ctx));
    return result;
}

bool Types::canBeTruthy(Context ctx, const TypePtr &what) {
    if (what->isUntyped()) {
        return true;
    }
    auto truthyPart =
        Types::dropSubtypesOf(ctx, Types::dropSubtypesOf(ctx, what, Symbols::NilClass()), Symbols::FalseClass());
    return !truthyPart->isBottom(); // check if truthyPart is empty
}

bool Types::canBeFalsy(Context ctx, const TypePtr &what) {
    if (what->isUntyped()) {
        return true;
    }
    return Types::isSubType(ctx, Types::falseClass(), what) ||
           Types::isSubType(ctx, Types::nilClass(),
                            what); // check if inhabited by falsy values
}

TypePtr Types::approximateSubtract(Context ctx, const TypePtr &from, const TypePtr &what) {
    TypePtr result;
    typecase(
        what.get(), [&](ClassType *c) { result = Types::dropSubtypesOf(ctx, from, c->symbol); },
        [&](AppliedType *c) { result = Types::dropSubtypesOf(ctx, from, c->klass); },
        [&](OrType *o) {
            result = Types::approximateSubtract(ctx, Types::approximateSubtract(ctx, from, o->left), o->right);
        },
        [&](Type *) { result = from; });
    return result;
}

TypePtr Types::dropLiteral(const TypePtr &tp) {
    if (auto *a = cast_type<LiteralType>(tp.get())) {
        return a->underlying();
    }
    return tp;
}

TypePtr Types::lubAll(Context ctx, vector<TypePtr> &elements) {
    TypePtr acc = Types::bottom();
    for (auto &el : elements) {
        acc = Types::lub(ctx, acc, el);
    }
    return acc;
}

TypePtr Types::arrayOf(Context ctx, const TypePtr &elem) {
    vector<TypePtr> targs{move(elem)};
    return make_type<AppliedType>(Symbols::Array(), targs);
}

TypePtr Types::hashOf(Context ctx, const TypePtr &elem) {
    vector<TypePtr> tupleArgs{Types::Symbol(), elem};
    vector<TypePtr> targs{Types::Symbol(), elem, TupleType::build(ctx, tupleArgs)};
    return make_type<AppliedType>(Symbols::Hash(), targs);
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

void ProxyType::_sanityCheck(Context ctx) {
    ENFORCE(cast_type<ClassType>(this->underlying().get()) != nullptr ||
            cast_type<AppliedType>(this->underlying().get()) != nullptr);
    this->underlying()->sanityCheck(ctx);
}

bool Type::isUntyped() const {
    auto *t = cast_type<ClassType>(this);
    return t != nullptr && t->symbol == Symbols::untyped();
}

core::SymbolRef Type::untypedBlame() const {
    ENFORCE(isUntyped());
    auto *t = cast_type<BlamedUntyped>(this);
    if (t == nullptr) {
        return Symbols::noSymbol();
    }
    return t->blame;
}

bool Type::isBottom() const {
    auto *t = cast_type<ClassType>(this);
    return t != nullptr && t->symbol == Symbols::bottom();
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

TypePtr TupleType::build(Context ctx, vector<TypePtr> elements) {
    TypePtr underlying = Types::arrayOf(ctx, Types::dropLiteral(Types::lubAll(ctx, elements)));
    return make_type<TupleType>(move(underlying), move(elements));
}

AndType::AndType(const TypePtr &left, const TypePtr &right) : left(move(left)), right(move(right)) {
    categoryCounterInc("types.allocated", "andtype");
}

bool LiteralType::equals(const LiteralType &rhs) const {
    if (this->value != rhs.value) {
        return false;
    }
    auto *lklass = cast_type<ClassType>(this->underlying().get());
    auto *rklass = cast_type<ClassType>(rhs.underlying().get());
    if (!lklass || !rklass) {
        return false;
    }
    return lklass->symbol == rklass->symbol;
}

OrType::OrType(const TypePtr &left, const TypePtr &right) : left(move(left)), right(move(right)) {
    categoryCounterInc("types.allocated", "ortype");
}

void TupleType::_sanityCheck(Context ctx) {
    ProxyType::_sanityCheck(ctx);
    auto *applied = cast_type<AppliedType>(this->underlying().get());
    ENFORCE(applied);
    ENFORCE(applied->klass == Symbols::Array());
}

ShapeType::ShapeType() : underlying_(Types::hashOfUntyped()) {
    categoryCounterInc("types.allocated", "shapetype");
}

ShapeType::ShapeType(TypePtr underlying, vector<TypePtr> keys, vector<TypePtr> values)
    : keys(move(keys)), values(move(values)), underlying_(std::move(underlying)) {
    DEBUG_ONLY(for (auto &k : this->keys) { ENFORCE(cast_type<LiteralType>(k.get()) != nullptr); };);
    categoryCounterInc("types.allocated", "shapetype");
}

TypePtr ShapeType::underlying() const {
    return this->underlying_;
}

TypePtr TupleType::underlying() const {
    return this->underlying_;
}

void ShapeType::_sanityCheck(Context ctx) {
    ProxyType::_sanityCheck(ctx);
    ENFORCE(this->values.size() == this->keys.size());
    for (auto &v : this->keys) {
        v->_sanityCheck(ctx);
    }
    for (auto &e : this->values) {
        e->_sanityCheck(ctx);
    }
}

AliasType::AliasType(SymbolRef other) : symbol(other) {
    categoryCounterInc("types.allocated", "aliastype");
}

void AndType::_sanityCheck(Context ctx) {
    left->_sanityCheck(ctx);
    right->_sanityCheck(ctx);
    /*
     * This is no longer true. Now we can construct types such as:
     * ShapeType(1 => 1), AppliedType{Array, Integer}
       ENFORCE(!isa_type<ProxyType>(left.get()));
       ENFORCE(!isa_type<ProxyType>(right.get()));

       */

    ENFORCE(!left->isUntyped());
    ENFORCE(!right->isUntyped());
    // TODO: reenable
    //    ENFORCE(!Types::isSubType(ctx, left, right),
    //            this->toString(ctx) + " should have collapsed: " + left->toString(ctx) + " <: " +
    //            right->toString(ctx));
    //    ENFORCE(!Types::isSubType(ctx, right, left),
    //            this->toString(ctx) + " should have collapsed: " + right->toString(ctx) + " <: " +
    //            left->toString(ctx));
}

void OrType::_sanityCheck(Context ctx) {
    left->_sanityCheck(ctx);
    right->_sanityCheck(ctx);
    //    ENFORCE(!isa_type<ProxyType>(left.get()));
    //    ENFORCE(!isa_type<ProxyType>(right.get()));
    ENFORCE(!left->isUntyped());
    ENFORCE(!right->isUntyped());
    //  TODO: @dmitry, reenable
    //    ENFORCE(!Types::isSubType(ctx, left, right),
    //            this->toString(ctx) + " should have collapsed: " + left->toString(ctx) + " <: " +
    //            right->toString(ctx));
    //    ENFORCE(!Types::isSubType(ctx, right, left),
    //            this->toString(ctx) + " should have collapsed: " + right->toString(ctx) + " <: " +
    //            left->toString(ctx));
}

void ClassType::_sanityCheck(Context ctx) {
    ENFORCE(this->symbol.exists());
}

int AppliedType::kind() {
    return 1;
}

int ClassType::kind() {
    return 2;
}

int LiteralType::kind() {
    return 3;
}

int ShapeType::kind() {
    return 4;
}

int TupleType::kind() {
    return 5;
}

int LambdaParam::kind() {
    return 6;
}

int SelfTypeParam::kind() {
    return 6;
}

int MetaType::kind() {
    return 7;
}

int TypeVar::kind() {
    return 8;
}

int AliasType::kind() {
    return 9;
}

int OrType::kind() {
    return 10;
}

int AndType::kind() {
    return 11;
}

int SelfType::kind() {
    return 12;
}

bool ClassType::isFullyDefined() {
    return true;
}

bool LiteralType::isFullyDefined() {
    return true;
}

bool ShapeType::isFullyDefined() {
    return absl::c_all_of(values, [](TypePtr t) { return t->isFullyDefined(); });
}

bool TupleType::isFullyDefined() {
    return absl::c_all_of(elems, [](TypePtr t) { return t->isFullyDefined(); });
}

bool AliasType::isFullyDefined() {
    return true;
}

bool AndType::isFullyDefined() {
    return this->left->isFullyDefined() && this->right->isFullyDefined();
}

bool OrType::isFullyDefined() {
    return this->left->isFullyDefined() && this->right->isFullyDefined();
}

/** Returns type parameters of what reordered in the order of type parameters of asIf
 * If some typeArgs are not present, return NoSymbol
 * */
InlinedVector<SymbolRef, 4> Types::alignBaseTypeArgs(Context ctx, SymbolRef what, const vector<TypePtr> &targs,
                                                     SymbolRef asIf) {
    ENFORCE(asIf.data(ctx)->isClass());
    ENFORCE(what.data(ctx)->isClass());
    ENFORCE(what == asIf || what.data(ctx)->derivesFrom(ctx, asIf) || asIf.data(ctx)->derivesFrom(ctx, what),
            what.data(ctx)->name.showRaw(ctx), asIf.data(ctx)->name.showRaw(ctx));
    InlinedVector<SymbolRef, 4> currentAlignment;
    if (targs.empty()) {
        return currentAlignment;
    }

    if (what == asIf || (asIf.data(ctx)->isClassClass() && what.data(ctx)->isClassClass() &&
                         asIf.data(ctx)->typeMembers().size() == what.data(ctx)->typeMembers().size())) {
        currentAlignment = what.data(ctx)->typeMembers();
    } else {
        currentAlignment.reserve(asIf.data(ctx)->typeMembers().size());
        for (auto originalTp : asIf.data(ctx)->typeMembers()) {
            auto name = originalTp.data(ctx)->name;
            SymbolRef align;
            int i = 0;
            for (auto x : what.data(ctx)->typeMembers()) {
                if (x.data(ctx)->name == name) {
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
TypePtr Types::resultTypeAsSeenFrom(Context ctx, TypePtr what, SymbolRef fromWhat, SymbolRef inWhat,
                                    const vector<TypePtr> &targs) {
    SymbolRef originalOwner = fromWhat;
    ENFORCE(fromWhat.data(ctx)->isClass());
    ENFORCE(inWhat.data(ctx)->isClass());

    // TODO: the ENFORCE below should be above this conditional, but there is
    // currently a problem with the handling of `module_function` that causes it
    // to fail reliably. https://github.com/sorbet/sorbet/issues/904
    if (originalOwner.data(ctx)->typeMembers().empty() || (what == nullptr)) {
        return what;
    }

    ENFORCE(inWhat == fromWhat || inWhat.data(ctx)->derivesFrom(ctx, fromWhat) ||
                fromWhat.data(ctx)->derivesFrom(ctx, inWhat),
            "\n{}\nis unrelated to\n\n{}", fromWhat.data(ctx)->toString(ctx), inWhat.data(ctx)->toString(ctx));

    auto currentAlignment = alignBaseTypeArgs(ctx, originalOwner, targs, inWhat);

    return instantiate(ctx, what, currentAlignment, targs);
}

TypePtr Types::getProcReturnType(Context ctx, const TypePtr &procType) {
    if (!procType->derivesFrom(ctx, Symbols::Proc())) {
        return Types::untypedUntracked();
    }
    auto *applied = cast_type<AppliedType>(procType.get());
    if (applied == nullptr || applied->targs.empty()) {
        return Types::untypedUntracked();
    }
    // Proc types have their return type as the first targ
    return applied->targs.front();
}

bool Types::isAsSpecificAs(Context ctx, const TypePtr &t1, const TypePtr &t2) {
    return isSubTypeUnderConstraint(ctx, TypeConstraint::EmptyFrozenConstraint, false, t1, t2);
}

bool Types::isSubType(Context ctx, const TypePtr &t1, const TypePtr &t2) {
    return isSubTypeUnderConstraint(ctx, TypeConstraint::EmptyFrozenConstraint, true, t1, t2);
}

bool TypeVar::isFullyDefined() {
    return false;
}

TypePtr TypeVar::getCallArguments(Context ctx, NameRef name) {
    Exception::raise("should never happen");
}

bool TypeVar::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    Exception::raise("should never happen. You're missing a call to either Types::approximate or Types::instantiate");
}

TypeVar::TypeVar(SymbolRef sym) : sym(sym) {
    categoryCounterInc("types.allocated", "typevar");
}

void TypeVar::_sanityCheck(Context ctx) {
    ENFORCE(this->sym.exists());
}

bool AppliedType::isFullyDefined() {
    for (auto &targ : this->targs) {
        if (!targ->isFullyDefined()) {
            return false;
        }
    }
    return true;
}

void AppliedType::_sanityCheck(Context ctx) {
    ENFORCE(this->klass.data(ctx)->isClass());
    ENFORCE(this->klass != Symbols::untyped());

    ENFORCE(this->klass.data(ctx)->typeMembers().size() == this->targs.size() ||
                (this->klass == Symbols::Array() && (this->targs.size() == 1)) ||
                (this->klass == Symbols::Hash() && (this->targs.size() == 3)) ||
                this->klass._id >= Symbols::Proc0()._id && this->klass._id <= Symbols::last_proc()._id,
            this->klass.data(ctx)->name.showRaw(ctx));
    for (auto &targ : this->targs) {
        targ->sanityCheck(ctx);
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

TypePtr LambdaParam::getCallArguments(Context ctx, NameRef name) {
    Exception::raise(
        "LambdaParam::getCallArguments not implemented, not clear what it should do. Let's see this fire first.");
}

TypePtr SelfTypeParam::getCallArguments(Context ctx, NameRef name) {
    Exception::raise(
        "SelfTypeParam::getCallArguments not implemented, not clear what it should do. Let's see this fire first.");
}

DispatchResult LambdaParam::dispatchCall(Context ctx, DispatchArgs args) {
    Exception::raise(
        "LambdaParam::dispatchCall not implemented, not clear what it should do. Let's see this fire first.");
}

DispatchResult SelfTypeParam::dispatchCall(Context ctx, DispatchArgs args) {
    return Types::untypedUntracked()->dispatchCall(ctx, args);
}

void LambdaParam::_sanityCheck(Context ctx) {}
void SelfTypeParam::_sanityCheck(Context ctx) {}

bool LambdaParam::isFullyDefined() {
    return false;
}

bool SelfTypeParam::isFullyDefined() {
    return true;
}

bool Type::hasUntyped() {
    return false;
}

bool ClassType::hasUntyped() {
    return isUntyped();
}

bool OrType::hasUntyped() {
    return left->hasUntyped() || right->hasUntyped();
}

TypePtr OrType::make_shared(const TypePtr &left, const TypePtr &right) {
    TypePtr res(new OrType(left, right));
    return res;
}

bool AndType::hasUntyped() {
    return left->hasUntyped() || right->hasUntyped();
}

TypePtr AndType::make_shared(const TypePtr &left, const TypePtr &right) {
    TypePtr res(new AndType(left, right));
    return res;
}

bool AppliedType::hasUntyped() {
    for (auto &arg : this->targs) {
        if (arg->hasUntyped()) {
            return true;
        }
    }
    return false;
}

bool TupleType::hasUntyped() {
    for (auto &arg : this->elems) {
        if (arg->hasUntyped()) {
            return true;
        }
    }
    return false;
}

bool ShapeType::hasUntyped() {
    for (auto &arg : this->values) {
        if (arg->hasUntyped()) {
            return true;
        }
    }
    return false;
};
SendAndBlockLink::SendAndBlockLink(NameRef fun, vector<ArgInfo::ArgFlags> &&argFlags) : argFlags(argFlags), fun(fun) {}

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
    auto *ap = cast_type<AppliedType>(this->underlying().get());
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

string SelfType::typeName() const {
    return "SelfType";
}

bool SelfType::isFullyDefined() {
    return false;
}

TypePtr SelfType::getCallArguments(Context ctx, NameRef name) {
    Exception::raise("should never happen");
}

bool SelfType::derivesFrom(const GlobalState &gs, SymbolRef klass) const {
    Exception::raise("should never happen");
}

DispatchResult SelfType::dispatchCall(Context ctx, DispatchArgs args) {
    Exception::raise("should never happen");
}

void SelfType::_sanityCheck(Context ctx) {}

TypePtr Types::widen(Context ctx, const TypePtr &type) {
    ENFORCE(type != nullptr);
    TypePtr ret;
    typecase(
        type.get(), [&](AndType *andType) { ret = all(ctx, widen(ctx, andType->left), widen(ctx, andType->right)); },
        [&](OrType *orType) { ret = any(ctx, widen(ctx, orType->left), widen(ctx, orType->right)); },
        [&](ProxyType *proxy) { ret = Types::widen(ctx, proxy->underlying()); },
        [&](AppliedType *appliedType) {
            vector<TypePtr> newTargs;
            newTargs.reserve(appliedType->targs.size());
            for (const auto &t : appliedType->targs) {
                newTargs.emplace_back(widen(ctx, t));
            }
            ret = make_type<AppliedType>(appliedType->klass, newTargs);
        },
        [&](Type *tp) { ret = type; });
    ENFORCE(ret);
    return ret;
}

core::SymbolRef Types::getRepresentedClass(core::Context ctx, const core::Type *ty) {
    if (!ty->derivesFrom(ctx, core::Symbols::Module())) {
        return core::Symbols::noSymbol();
    }
    core::SymbolRef singleton;
    auto *s = core::cast_type<core::ClassType>(ty);
    if (s != nullptr) {
        singleton = s->symbol;
    } else {
        auto *at = core::cast_type<core::AppliedType>(ty);
        if (at == nullptr) {
            return core::Symbols::noSymbol();
        }
        // If this class is a "real" generic, leave it alone. We're not quite
        // sure yet what it means to move between the "attached" and "singleton"
        // levels for generic singletons. However, if the singleton has kind `*`
        // due to all type members being fixed:, we're OK to treat the
        // AppliedType and the Symbol as interchangeable here.
        if (at->klass.data(ctx)->typeArity(ctx) != 0) {
            return core::Symbols::noSymbol();
        }
        singleton = at->klass;
    }
    return singleton.data(ctx)->attachedClass(ctx);
}

DispatchArgs DispatchArgs::withSelfRef(const TypePtr &newSelfRef) {
    return DispatchArgs{name, locs, args, newSelfRef, fullType, block};
}
} // namespace sorbet::core
