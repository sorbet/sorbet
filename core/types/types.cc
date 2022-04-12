#include "core/Types.h"
#include "absl/base/casts.h"
#include "absl/strings/match.h"
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

TypePtr Types::dispatchCallWithoutBlock(const GlobalState &gs, const TypePtr &recv, const DispatchArgs &args) {
    auto dispatched = recv.dispatchCall(gs, args);
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

TypePtr Types::todo() {
    static auto res = make_type<ClassType>(Symbols::todo());
    return res;
}

TypePtr Types::dropSubtypesOf(const GlobalState &gs, const TypePtr &from, ClassOrModuleRef klass) {
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
            if (c.symbol == core::Symbols::untyped()) {
                result = from;
            } else if (c.symbol == klass || c.derivesFrom(gs, klass)) {
                result = Types::bottom();
            } else if (c.symbol.data(gs)->isClass() && klass.data(gs)->isClass() &&
                       !klass.data(gs)->derivesFrom(gs, c.symbol)) {
                // We have two classes (not modules), and if the class we're
                // removing doesn't derive from `c`, there's nothing to do,
                // because of ruby having single inheretance.
                result = from;
            } else if (cdata->flags.isSealed && (cdata->flags.isAbstract || cdata->isModule())) {
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
        [&](const TypePtr &) {
            if (is_proxy_type(from) && dropSubtypesOf(gs, from.underlying(gs), klass).isBottom()) {
                result = Types::bottom();
            } else {
                result = from;
            }
        });
    SLOW_ENFORCE(Types::isSubType(gs, result, from),
                 "dropSubtypesOf({}, {}) returned {}, which is not a subtype of the input", from.toString(gs),
                 klass.showFullName(gs), result.toString(gs));
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

TypePtr Types::dropLiteral(const GlobalState &gs, const TypePtr &tp) {
    if (isa_type<LiteralType>(tp)) {
        auto a = cast_type_nonnull<LiteralType>(tp);
        return a.underlying(gs);
    }
    return tp;
}

TypePtr Types::lubAll(const GlobalState &gs, const vector<TypePtr> &elements) {
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
    vector<TypePtr> targs{Types::Symbol(), elem, make_type<TupleType>(move(tupleArgs))};
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

ClassType::ClassType(ClassOrModuleRef symbol) : symbol(symbol) {
    categoryCounterInc("types.allocated", "classtype");
    ENFORCE(symbol.exists());
}

namespace {
void sanityCheckProxyType(const GlobalState &gs, TypePtr underlying) {
    ENFORCE(isa_type<ClassType>(underlying) || isa_type<AppliedType>(underlying));
    underlying.sanityCheck(gs);
}
} // namespace

LiteralType::LiteralType(int64_t val) : value(val), literalKind(LiteralTypeKind::Integer) {
    categoryCounterInc("types.allocated", "literaltype");
}

LiteralType::LiteralType(double val) : floatval(val), literalKind(LiteralTypeKind::Float) {
    categoryCounterInc("types.allocated", "literaltype");
}

LiteralType::LiteralType(ClassOrModuleRef klass, NameRef val)
    : nameId(val.rawId()), literalKind(klass == Symbols::String() ? LiteralTypeKind::String : LiteralTypeKind::Symbol) {
    categoryCounterInc("types.allocated", "literaltype");
    ENFORCE(klass == Symbols::String() || klass == Symbols::Symbol());
}

int64_t LiteralType::asInteger() const {
    ENFORCE_NO_TIMER(literalKind == LiteralTypeKind::Integer);
    return value;
}

double LiteralType::asFloat() const {
    ENFORCE_NO_TIMER(literalKind == LiteralTypeKind::Float);
    return floatval;
}

core::NameRef LiteralType::asName(const core::GlobalState &gs) const {
    ENFORCE_NO_TIMER(literalKind == LiteralTypeKind::Symbol || literalKind == LiteralTypeKind::String);
    return NameRef::fromRaw(gs, nameId);
}

core::NameRef LiteralType::unsafeAsName() const {
    ENFORCE_NO_TIMER(literalKind == LiteralTypeKind::Symbol || literalKind == LiteralTypeKind::String);
    return NameRef::fromRawUnchecked(nameId);
}

TypePtr LiteralType::underlying(const GlobalState &gs) const {
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

TupleType::TupleType(vector<TypePtr> elements) : elems(move(elements)) {
    categoryCounterInc("types.allocated", "tupletype");
}

AndType::AndType(const TypePtr &left, const TypePtr &right) : left(move(left)), right(move(right)) {
    categoryCounterInc("types.allocated", "andtype");
}

void LiteralType::_sanityCheck(const GlobalState &gs) const {
    sanityCheckProxyType(gs, underlying(gs));
}

bool LiteralType::equals(const LiteralType &rhs) const {
    if (this->literalKind != rhs.literalKind) {
        return false;
    }
    switch (this->literalKind) {
        case LiteralTypeKind::Float:
            return this->floatval == rhs.floatval;
        case LiteralTypeKind::Integer:
            return this->value == rhs.value;
        case LiteralTypeKind::Symbol:
        case LiteralTypeKind::String:
            return this->nameId == rhs.nameId;
    }
}

OrType::OrType(const TypePtr &left, const TypePtr &right) : left(move(left)), right(move(right)) {
    categoryCounterInc("types.allocated", "ortype");
}

void TupleType::_sanityCheck(const GlobalState &gs) const {
    sanityCheckProxyType(gs, underlying(gs));
    auto underlying = this->underlying(gs);
    auto *applied = cast_type<AppliedType>(underlying);
    ENFORCE(applied);
    ENFORCE(applied->klass == Symbols::Array());
}

ShapeType::ShapeType(vector<TypePtr> keys, vector<TypePtr> values) : keys(move(keys)), values(move(values)) {
    DEBUG_ONLY(for (auto &k : this->keys) { ENFORCE(isa_type<LiteralType>(k)); };);
    categoryCounterInc("types.allocated", "shapetype");
}

TypePtr ShapeType::underlying(const GlobalState &gs) const {
    return Types::hashOfUntyped();
}

TypePtr TupleType::underlying(const GlobalState &gs) const {
    if (this->elems.empty()) {
        return Types::arrayOfUntyped();
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
    categoryCounterInc("types.allocated", "aliastype");
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
                                                         const vector<TypePtr> &targs, ClassOrModuleRef asIf) {
    ENFORCE(what == asIf || what.data(gs)->derivesFrom(gs, asIf) || asIf.data(gs)->derivesFrom(gs, what),
            "what={} asIf={}", what.data(gs)->name.showRaw(gs), asIf.data(gs)->name.showRaw(gs));
    InlinedVector<TypeMemberRef, 4> currentAlignment;
    if (targs.empty()) {
        return currentAlignment;
    }

    if (what == asIf || (asIf.data(gs)->isClass() && what.data(gs)->isClass() &&
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
                currentAlignment.emplace_back(Symbols::noTypeMember());
            }
        }
    }
    return currentAlignment;
}

/**
 * fromWhat - where the generic type was written
 * inWhat   - where the generic type is observed
 */
TypePtr Types::resultTypeAsSeenFrom(const GlobalState &gs, const TypePtr &what, ClassOrModuleRef fromWhat,
                                    ClassOrModuleRef inWhat, const vector<TypePtr> &targs) {
    auto originalOwner = fromWhat;

    // TODO: the ENFORCE below should be above this conditional, but there is
    // currently a problem with the handling of `module_function` that causes it
    // to fail reliably. https://github.com/sorbet/sorbet/issues/904
    if (originalOwner.data(gs)->typeMembers().empty() || (what == nullptr)) {
        return what;
    }

    ENFORCE(inWhat == fromWhat || inWhat.data(gs)->derivesFrom(gs, fromWhat) ||
                fromWhat.data(gs)->derivesFrom(gs, inWhat),
            "\n{}\nis unrelated to\n\n{}", fromWhat.toString(gs), inWhat.toString(gs));

    auto currentAlignment = alignBaseTypeArgs(gs, originalOwner, targs, inWhat);

    return instantiate(gs, what, currentAlignment, targs);
}

TypePtr Types::getProcReturnType(const GlobalState &gs, const TypePtr &procType) {
    if (!procType.derivesFrom(gs, Symbols::Proc())) {
        return Types::untypedUntracked();
    }
    auto *applied = cast_type<AppliedType>(procType);
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
    Exception::raise("should never happen. You're missing a call to either Types::approximate or Types::instantiate");
}

MetaType::MetaType(const TypePtr &wrapped) : wrapped(move(wrapped)) {
    categoryCounterInc("types.allocated", "metattype");
}

bool MetaType::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    return false;
}

TypePtr MetaType::_approximate(const GlobalState &gs, const TypeConstraint &tc) const {
    // dispatchCall is invoked on them in resolver
    return nullptr;
}

TypePtr MetaType::underlying(const GlobalState &gs) const {
    return Types::Object();
}

TypeVar::TypeVar(TypeArgumentRef sym) : sym(sym) {
    categoryCounterInc("types.allocated", "typevar");
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
    categoryCounterInc("types.allocated", "lambdatypeparam");
}

SelfTypeParam::SelfTypeParam(const SymbolRef definition) : definition(definition) {
    categoryCounterInc("types.allocated", "selftypeparam");
}

bool LambdaParam::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    Exception::raise(
        "LambdaParam::derivesFrom not implemented, not clear what it should do. Let's see this fire first.");
}

bool SelfTypeParam::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    return false;
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

SendAndBlockLink::SendAndBlockLink(NameRef fun, vector<ArgInfo::ArgFlags> &&argFlags, int rubyRegionId)
    : argFlags(move(argFlags)), fun(fun), rubyRegionId(rubyRegionId) {}

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

TypePtr TupleType::elementType(const GlobalState &gs) const {
    auto underlying = this->underlying(gs);
    auto *ap = cast_type<AppliedType>(underlying);
    ENFORCE(ap);
    ENFORCE(ap->klass == Symbols::Array());
    ENFORCE(ap->targs.size() == 1);
    return ap->targs.front();
}

SelfType::SelfType() {
    categoryCounterInc("types.allocated", "selftype");
};
AppliedType::AppliedType(ClassOrModuleRef klass, vector<TypePtr> targs) : klass(klass), targs(std::move(targs)) {
    categoryCounterInc("types.allocated", "appliedtype");
}

bool SelfType::derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const {
    Exception::raise("should never happen");
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
    std::vector<TypePtr> unwrapped;
    unwrapped.reserve(elems.size());
    for (auto &e : elems) {
        unwrapped.emplace_back(Types::unwrapSelfTypeParam(ctx, e));
    }
    return unwrapped;
}
} // namespace

TypePtr Types::unwrapSelfTypeParam(Context ctx, const TypePtr &type) {
    ENFORCE(type != nullptr);

    TypePtr ret;
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
        [&](const TypePtr &tp) {
            ENFORCE(false, "Unhandled case: {}", type.toString(ctx));
            Exception::notImplemented();
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
        auto *at = cast_type<AppliedType>(ty);
        if (at == nullptr) {
            return core::Symbols::noClassOrModule();
        }

        singleton = at->klass;
    }
    return singleton.data(gs)->attachedClass(gs);
}

Loc DispatchArgs::blockLoc(const GlobalState &gs) const {
    ENFORCE(this->block != nullptr);
    auto blockLoc = core::Loc(locs.file, argsLoc().endPos(), callLoc().endPos());
    auto blockLocSource = blockLoc.source(gs);
    if (!blockLocSource.has_value()) {
        return callLoc();
    }

    if (absl::StartsWith(blockLocSource.value(), ")") || absl::StartsWith(blockLocSource.value(), "]")) {
        return blockLoc.adjust(gs, 1, 0);
    }

    return blockLoc;
}

DispatchArgs DispatchArgs::withSelfRef(const TypePtr &newSelfRef) const {
    return DispatchArgs{
        name,        locs,          numPosArgs, args, newSelfRef, fullType, newSelfRef, block, originForUninitialized,
        isPrivateOk, suppressErrors};
}

DispatchArgs DispatchArgs::withThisRef(const TypePtr &newThisRef) const {
    return DispatchArgs{
        name,        locs,          numPosArgs, args, selfType, fullType, newThisRef, block, originForUninitialized,
        isPrivateOk, suppressErrors};
}

DispatchArgs DispatchArgs::withErrorsSuppressed() const {
    return DispatchArgs{
        name, locs, numPosArgs, args, selfType, fullType, thisType, block, originForUninitialized, isPrivateOk, true};
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
