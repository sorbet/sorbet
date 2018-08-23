#include "core/Types.h"
#include "absl/base/casts.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/TypeConstraint.h"
#include <utility>

#include "core/Types.h"

// improve debugging.
template class std::shared_ptr<sorbet::core::Type>;
template class std::shared_ptr<sorbet::core::TypeConstraint>;
template class std::shared_ptr<sorbet::core::SendAndBlockLink>;
template class std::vector<sorbet::core::Loc>;

namespace sorbet {
namespace core {

using namespace std;

shared_ptr<Type> Types::top() {
    static auto res = make_shared<ClassType>(Symbols::top());
    return res;
}

shared_ptr<Type> Types::bottom() {
    static auto res = make_shared<ClassType>(Symbols::bottom());
    return res;
}

shared_ptr<Type> Types::nilClass() {
    static auto res = make_shared<ClassType>(Symbols::NilClass());
    return res;
}

shared_ptr<Type> Types::untyped() {
    static auto res = make_shared<ClassType>(Symbols::untyped());
    return res;
}

shared_ptr<Type> Types::void_() {
    static auto res = make_shared<ClassType>(Symbols::void_());
    return res;
}

shared_ptr<Type> Types::trueClass() {
    static auto res = make_shared<ClassType>(Symbols::TrueClass());
    return res;
}

shared_ptr<Type> Types::falseClass() {
    static auto res = make_shared<ClassType>(Symbols::FalseClass());
    return res;
}

shared_ptr<Type> Types::Boolean() {
    static auto res = OrType::make_shared(trueClass(), falseClass());
    return res;
}

shared_ptr<Type> Types::Integer() {
    static auto res = make_shared<ClassType>(Symbols::Integer());
    return res;
}

shared_ptr<Type> Types::Float() {
    static auto res = make_shared<ClassType>(Symbols::Float());
    return res;
}

shared_ptr<Type> Types::arrayClass() {
    static auto res = make_shared<ClassType>(Symbols::Array());
    return res;
}

shared_ptr<Type> Types::hashClass() {
    static auto res = make_shared<ClassType>(Symbols::Hash());
    return res;
}

shared_ptr<Type> Types::arrayOfUntyped() {
    static vector<shared_ptr<Type>> targs{Types::untyped()};
    static auto res = make_shared<AppliedType>(Symbols::Array(), targs);
    return res;
}

shared_ptr<Type> Types::hashOfUntyped() {
    static vector<shared_ptr<Type>> targs{Types::untyped(), Types::untyped(), Types::untyped()};
    static auto res = make_shared<AppliedType>(Symbols::Hash(), targs);
    return res;
}

shared_ptr<Type> Types::procClass() {
    static auto res = make_shared<ClassType>(Symbols::Proc());
    return res;
}

shared_ptr<Type> Types::classClass() {
    static auto res = make_shared<ClassType>(Symbols::Class());
    return res;
}

shared_ptr<Type> Types::String() {
    static auto res = make_shared<ClassType>(Symbols::String());
    return res;
}

shared_ptr<Type> Types::Symbol() {
    static auto res = make_shared<ClassType>(Symbols::Symbol());
    return res;
}

shared_ptr<Type> Types::Object() {
    static auto res = make_shared<ClassType>(Symbols::Object());
    return res;
}

shared_ptr<Type> Types::falsyTypes() {
    static auto res = OrType::make_shared(Types::nilClass(), Types::falseClass());
    return res;
}

shared_ptr<Type> Types::dropSubtypesOf(Context ctx, const shared_ptr<Type> &from, SymbolRef klass) {
    shared_ptr<Type> result;

    if (from->isUntyped()) {
        return from;
    }

    typecase(from.get(),
             [&](OrType *o) {
                 auto lhs = dropSubtypesOf(ctx, o->left, klass);
                 auto rhs = dropSubtypesOf(ctx, o->right, klass);
                 if (lhs != o->left || rhs != o->right) {
                     result = Types::any(ctx, lhs, rhs);
                 } else {
                     result = from;
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
                 if (c->isUntyped()) {
                     result = from;
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
                 if (dropSubtypesOf(ctx, c->underlying, klass)->isBottom()) {
                     result = Types::bottom();
                 } else {
                     result = from;
                 }
             },
             [&](Type *) { result = from; });
    ENFORCE(Types::isSubType(ctx, result, from), "dropSubtypesOf(" + from->toString(ctx) + "," +
                                                     klass.data(ctx).fullName(ctx) + ") returned " +
                                                     result->toString(ctx) + ", which is not a subtype of the input");
    return result;
}

bool Types::canBeTruthy(Context ctx, const shared_ptr<Type> &what) {
    if (what->isUntyped()) {
        return true;
    }
    auto truthyPart =
        Types::dropSubtypesOf(ctx, Types::dropSubtypesOf(ctx, what, Symbols::NilClass()), Symbols::FalseClass());
    return !truthyPart->isBottom(); // check if truthyPart is empty
}

bool Types::canBeFalsy(Context ctx, const shared_ptr<Type> &what) {
    if (what->isUntyped()) {
        return true;
    }
    return Types::isSubType(ctx, Types::falseClass(), what) ||
           Types::isSubType(ctx, Types::nilClass(),
                            what); // check if inhabited by falsy values
}

shared_ptr<Type> Types::approximateSubtract(Context ctx, const shared_ptr<Type> &from, const shared_ptr<Type> &what) {
    shared_ptr<Type> result;
    typecase(what.get(), [&](ClassType *c) { result = Types::dropSubtypesOf(ctx, from, c->symbol); },
             [&](AppliedType *c) { result = Types::dropSubtypesOf(ctx, from, c->klass); },
             [&](OrType *o) {
                 result = Types::approximateSubtract(ctx, Types::approximateSubtract(ctx, from, o->left), o->right);
             },
             [&](Type *) { result = from; });
    return result;
}

shared_ptr<Type> Types::dropLiteral(const shared_ptr<Type> &tp) {
    if (auto *a = cast_type<LiteralType>(tp.get())) {
        return a->underlying;
    }
    return tp;
}

shared_ptr<Type> Types::lubAll(Context ctx, vector<shared_ptr<Type>> &elements) {
    shared_ptr<Type> acc = Types::bottom();
    for (auto &el : elements) {
        acc = Types::lub(ctx, acc, el);
    }
    return acc;
}

shared_ptr<Type> Types::arrayOf(Context ctx, const shared_ptr<Type> &elem) {
    vector<shared_ptr<Type>> targs{move(elem)};
    return make_shared<AppliedType>(Symbols::Array(), targs);
}

ClassType::ClassType(SymbolRef symbol) : symbol(symbol) {
    core::categoryCounterInc("types.allocated", "classtype");
    ENFORCE(symbol.exists());
}

ProxyType::ProxyType(const shared_ptr<Type> &underlying) : underlying(move(underlying)) {}

void ProxyType::_sanityCheck(Context ctx) {
    ENFORCE(cast_type<ClassType>(this->underlying.get()) != nullptr ||
            cast_type<AppliedType>(this->underlying.get()) != nullptr);
    this->underlying->sanityCheck(ctx);
}

bool Type::isUntyped() {
    auto *t = cast_type<ClassType>(this);
    return t != nullptr && t->symbol == Symbols::untyped();
}

bool Type::isTop() {
    auto *t = cast_type<ClassType>(this);
    return t != nullptr && t->symbol == Symbols::top();
}

bool Type::isBottom() {
    auto *t = cast_type<ClassType>(this);
    return t != nullptr && t->symbol == Symbols::bottom();
}

LiteralType::LiteralType(int64_t val) : ProxyType(Types::Integer()), value(val) {
    core::categoryCounterInc("types.allocated", "literaltype");
}

LiteralType::LiteralType(double val) : ProxyType(Types::Float()), floatval(val) {
    core::categoryCounterInc("types.allocated", "literaltype");
}

LiteralType::LiteralType(SymbolRef klass, NameRef val)
    : ProxyType(klass == Symbols::String() ? Types::String() : Types::Symbol()), value(val._id) {
    core::categoryCounterInc("types.allocated", "literaltype");
    ENFORCE(klass == Symbols::String() || klass == Symbols::Symbol());
}

LiteralType::LiteralType(bool val) : ProxyType(val ? Types::trueClass() : Types::falseClass()), value(val ? 1 : 0) {
    core::categoryCounterInc("types.allocated", "literaltype");
}

TupleType::TupleType(const shared_ptr<Type> &underlying, vector<shared_ptr<Type>> elements)
    : ProxyType(move(underlying)), elems(move(elements)) {
    core::categoryCounterInc("types.allocated", "tupletype");
}

shared_ptr<Type> TupleType::build(Context ctx, vector<shared_ptr<Type>> elements) {
    shared_ptr<Type> underlying = Types::arrayOf(ctx, Types::dropLiteral(Types::lubAll(ctx, elements)));
    return make_shared<TupleType>(move(underlying), move(elements));
}

AndType::AndType(const shared_ptr<Type> &left, const shared_ptr<Type> &right) : left(move(left)), right(move(right)) {
    core::categoryCounterInc("types.allocated", "andtype");
}

bool LiteralType::equals(const shared_ptr<LiteralType> &rhs) const {
    if (this->value != rhs->value) {
        return false;
    }
    auto *lklass = cast_type<ClassType>(this->underlying.get());
    auto *rklass = cast_type<ClassType>(rhs->underlying.get());
    if (!lklass || !rklass) {
        return false;
    }
    return lklass->symbol == rklass->symbol;
}

OrType::OrType(const shared_ptr<Type> &left, const shared_ptr<Type> &right) : left(move(left)), right(move(right)) {
    core::categoryCounterInc("types.allocated", "ortype");
}

void TupleType::_sanityCheck(Context ctx) {
    ProxyType::_sanityCheck(ctx);
    auto *applied = cast_type<AppliedType>(this->underlying.get());
    ENFORCE(applied);
    ENFORCE(applied->klass == Symbols::Array());
}

ShapeType::ShapeType() : ProxyType(Types::hashOfUntyped()) {
    core::categoryCounterInc("types.allocated", "shapetype");
}

ShapeType::ShapeType(vector<shared_ptr<LiteralType>> keys, vector<shared_ptr<Type>> values)
    : ProxyType(Types::hashOfUntyped()), keys(move(keys)), values(move(values)) {
    core::categoryCounterInc("types.allocated", "shapetype");
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
    core::categoryCounterInc("types.allocated", "aliastype");
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
    return true; // might not be true if we support uninstantiated types inside hashes. For now, we don't
}

bool TupleType::isFullyDefined() {
    return true; // might not be true if we support uninstantiated types inside tuples. For now, we don't
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
InlinedVector<SymbolRef, 4> Types::alignBaseTypeArgs(Context ctx, SymbolRef what, const vector<shared_ptr<Type>> &targs,
                                                     SymbolRef asIf) {
    ENFORCE(asIf.data(ctx).isClass());
    ENFORCE(what.data(ctx).isClass());
    ENFORCE(what == asIf || what.data(ctx).derivesFrom(ctx, asIf) || asIf.data(ctx).derivesFrom(ctx, what),
            what.data(ctx).name.toString(ctx), asIf.data(ctx).name.toString(ctx));
    InlinedVector<SymbolRef, 4> currentAlignment;
    if (targs.empty()) {
        return currentAlignment;
    }

    if (what == asIf || (asIf.data(ctx).isClassClass() && what.data(ctx).isClassClass())) {
        currentAlignment = what.data(ctx).typeMembers();
    } else {
        for (auto originalTp : asIf.data(ctx).typeMembers()) {
            auto name = originalTp.data(ctx).name;
            SymbolRef align;
            int i = 0;
            for (auto x : what.data(ctx).typeMembers()) {
                if (x.data(ctx).name == name) {
                    align = x;
                    currentAlignment.push_back(x);
                    break;
                }
                i++;
            }
            if (!align.exists()) {
                currentAlignment.push_back(Symbols::noSymbol());
            }
        }
    }
    return currentAlignment;
}

shared_ptr<Type> Types::resultTypeAsSeenFrom(Context ctx, SymbolRef what, SymbolRef inWhat,
                                             const vector<shared_ptr<Type>> &targs) {
    const sorbet::core::Symbol &original = what.data(ctx);
    SymbolRef originalOwner = what.data(ctx).enclosingClass(ctx);

    if (originalOwner.data(ctx).typeMembers().empty() || (original.resultType == nullptr)) {
        return original.resultType;
    }

    auto currentAlignment = alignBaseTypeArgs(ctx, originalOwner, targs, inWhat);

    return instantiate(ctx, original.resultType, currentAlignment, targs);
}

shared_ptr<Type> Types::getProcReturnType(Context ctx, const shared_ptr<Type> &procType) {
    if (!procType->derivesFrom(ctx, Symbols::Proc())) {
        return Types::untyped();
    }
    auto *applied = cast_type<AppliedType>(procType.get());
    if (applied == nullptr || applied->targs.empty()) {
        return Types::untyped();
    }
    // Proc types have their return type as the first targ
    return applied->targs.front();
}

bool Types::isSubType(Context ctx, const shared_ptr<Type> &t1, const shared_ptr<Type> &t2) {
    return isSubTypeUnderConstraint(ctx, TypeConstraint::EmptyFrozenConstraint, t1, t2);
}

bool TypeVar::isFullyDefined() {
    return false;
}

shared_ptr<Type> TypeVar::getCallArgumentType(Context ctx, NameRef name, int i) {
    Error::raise("should never happen");
}

bool TypeVar::derivesFrom(const GlobalState &gs, SymbolRef klass) {
    Error::raise("should never happen");
}

TypeVar::TypeVar(SymbolRef sym) : sym(sym) {
    core::categoryCounterInc("types.allocated", "typevar");
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
    ENFORCE(this->klass.data(ctx).isClass());
    ENFORCE(this->klass != Symbols::untyped());

    ENFORCE(this->klass.data(ctx).typeMembers().size() == this->targs.size() ||
                (this->klass == Symbols::Array() && (this->targs.size() == 1)) ||
                (this->klass == Symbols::Hash() && (this->targs.size() == 3)) ||
                this->klass._id >= Symbols::Proc0()._id && this->klass._id <= Symbols::last_proc()._id,
            this->klass.data(ctx).name.toString(ctx));
    for (auto &targ : this->targs) {
        targ->sanityCheck(ctx);
    }
}

bool AppliedType::derivesFrom(const GlobalState &gs, SymbolRef klass) {
    ClassType und(this->klass);
    return und.derivesFrom(gs, klass);
}

LambdaParam::LambdaParam(const SymbolRef definition) : definition(definition) {
    core::categoryCounterInc("types.allocated", "lambdatypeparam");
}

SelfTypeParam::SelfTypeParam(const SymbolRef definition) : definition(definition) {
    core::categoryCounterInc("types.allocated", "selftypeparam");
}

bool LambdaParam::derivesFrom(const GlobalState &gs, SymbolRef klass) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

bool SelfTypeParam::derivesFrom(const GlobalState &gs, SymbolRef klass) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

shared_ptr<Type> LambdaParam::getCallArgumentType(Context ctx, NameRef name, int i) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

shared_ptr<Type> SelfTypeParam::getCallArgumentType(Context ctx, NameRef name, int i) {
    return Types::untyped()->getCallArgumentType(ctx, name, i);
}

DispatchResult LambdaParam::dispatchCall(Context ctx, NameRef name, Loc callLoc, Loc receiverLoc,
                                         vector<TypeAndOrigins> &args, vector<Loc> &argLocs,
                                         const shared_ptr<Type> &selfType, const shared_ptr<Type> &fullType,
                                         const shared_ptr<SendAndBlockLink> &block) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

DispatchResult SelfTypeParam::dispatchCall(Context ctx, NameRef name, Loc callLoc, Loc receiverLoc,
                                           vector<TypeAndOrigins> &args, vector<Loc> &argLocs,
                                           const shared_ptr<Type> &selfType, const shared_ptr<Type> &fullType,
                                           const shared_ptr<SendAndBlockLink> &block) {
    return Types::untyped()->dispatchCall(ctx, name, callLoc, receiverLoc, args, argLocs, selfType, fullType, block);
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

bool AndType::hasUntyped() {
    return left->hasUntyped() || right->hasUntyped();
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
SendAndBlockLink::SendAndBlockLink(SymbolRef block, NameRef fun)
    : block(block), fun(fun), constr(make_shared<TypeConstraint>()) {}

shared_ptr<Type> TupleType::elementType() const {
    auto *ap = cast_type<AppliedType>(this->underlying.get());
    ENFORCE(ap);
    ENFORCE(ap->klass == Symbols::Array());
    ENFORCE(ap->targs.size() == 1);
    return ap->targs.front();
}

SelfType::SelfType() {
    core::categoryCounterInc("types.allocated", "selftype");
};
AppliedType::AppliedType(SymbolRef klass, vector<shared_ptr<Type>> targs) : klass(klass), targs(targs) {
    core::categoryCounterInc("types.allocated", "appliedtype");
}

string SelfType::typeName() const {
    return "SelfType";
}

bool SelfType::isFullyDefined() {
    return false;
}

shared_ptr<Type> SelfType::getCallArgumentType(Context ctx, NameRef name, int i) {
    Error::raise("should never happen");
}

bool SelfType::derivesFrom(const GlobalState &gs, SymbolRef klass) {
    Error::raise("should never happen");
}

DispatchResult SelfType::dispatchCall(Context ctx, NameRef name, Loc callLoc, Loc receiverLoc,
                                      vector<TypeAndOrigins> &args, vector<Loc> &argLocs,
                                      const shared_ptr<Type> &selfRef, const shared_ptr<Type> &fullType,
                                      const shared_ptr<SendAndBlockLink> &link) {
    Error::raise("should never happen");
}

void SelfType::_sanityCheck(Context ctx) {}

} // namespace core
} // namespace sorbet
