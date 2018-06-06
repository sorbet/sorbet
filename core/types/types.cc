#include "core/Types.h"
#include "absl/base/casts.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/TypeConstraint.h"
#include <utility>

#include "core/Types.h"

using namespace ruby_typer;
using namespace core;
using namespace std;

// improve debugging.
template class std::shared_ptr<core::Type>;
template class std::shared_ptr<core::TypeConstraint>;
template class std::shared_ptr<core::SendAndBlockLink>;
template class std::vector<core::Loc>;

shared_ptr<Type> Types::top() {
    static auto res = make_shared<ClassType>(core::Symbols::top());
    return res;
}

shared_ptr<Type> Types::bottom() {
    static auto res = make_shared<ClassType>(core::Symbols::bottom());
    return res;
}

shared_ptr<Type> Types::nilClass() {
    static auto res = make_shared<ClassType>(core::Symbols::NilClass());
    return res;
}

shared_ptr<Type> Types::dynamic() {
    static auto res = make_shared<ClassType>(core::Symbols::untyped());
    return res;
}

shared_ptr<Type> Types::void_() {
    static auto res = make_shared<ClassType>(core::Symbols::void_());
    return res;
}

std::shared_ptr<Type> Types::trueClass() {
    static auto res = make_shared<ClassType>(core::Symbols::TrueClass());
    return res;
}

std::shared_ptr<Type> Types::falseClass() {
    static auto res = make_shared<ClassType>(core::Symbols::FalseClass());
    return res;
}

std::shared_ptr<Type> Types::Boolean() {
    static auto res = OrType::make_shared(trueClass(), falseClass());
    return res;
}

std::shared_ptr<Type> Types::Integer() {
    static auto res = make_shared<ClassType>(core::Symbols::Integer());
    return res;
}

std::shared_ptr<Type> Types::Float() {
    static auto res = make_shared<ClassType>(core::Symbols::Float());
    return res;
}

std::shared_ptr<Type> Types::arrayClass() {
    static auto res = make_shared<ClassType>(core::Symbols::Array());
    return res;
}

std::shared_ptr<Type> Types::hashClass() {
    static auto res = make_shared<ClassType>(core::Symbols::Hash());
    return res;
}

std::shared_ptr<Type> Types::arrayOfUntyped() {
    static vector<shared_ptr<Type>> targs{core::Types::dynamic()};
    static auto res = make_shared<core::AppliedType>(core::Symbols::Array(), targs);
    return res;
}

std::shared_ptr<Type> Types::hashOfUntyped() {
    static vector<shared_ptr<Type>> targs{core::Types::dynamic(), core::Types::dynamic(), core::Types::dynamic()};
    static auto res = make_shared<core::AppliedType>(core::Symbols::Hash(), targs);
    return res;
}

std::shared_ptr<Type> Types::procClass() {
    static auto res = make_shared<ClassType>(core::Symbols::Proc());
    return res;
}

std::shared_ptr<Type> Types::classClass() {
    static auto res = make_shared<ClassType>(core::Symbols::Class());
    return res;
}

std::shared_ptr<Type> Types::String() {
    static auto res = make_shared<ClassType>(core::Symbols::String());
    return res;
}

std::shared_ptr<Type> Types::Symbol() {
    static auto res = make_shared<ClassType>(core::Symbols::Symbol());
    return res;
}

std::shared_ptr<Type> Types::Object() {
    static auto res = make_shared<ClassType>(core::Symbols::Object());
    return res;
}

std::shared_ptr<Type> Types::falsyTypes() {
    static auto res = OrType::make_shared(Types::nilClass(), Types::falseClass());
    return res;
}

std::shared_ptr<Type> Types::dropSubtypesOf(core::Context ctx, std::shared_ptr<Type> from, core::SymbolRef klass) {
    std::shared_ptr<Type> result;

    if (from->isDynamic()) {
        return from;
    }

    typecase(from.get(),
             [&](OrType *o) {
                 if (o->left->derivesFrom(ctx, klass)) {
                     result = Types::dropSubtypesOf(ctx, o->right, klass);
                 } else if (o->right->derivesFrom(ctx, klass)) {
                     result = Types::dropSubtypesOf(ctx, o->left, klass);
                 } else {
                     result = from;
                 }
             },
             [&](AndType *a) {
                 if (a->left->derivesFrom(ctx, klass) || a->right->derivesFrom(ctx, klass)) {
                     result = Types::bottom();
                 } else {
                     result = from;
                 }
             },
             [&](ClassType *c) {
                 if (c->isDynamic()) {
                     result = from;
                 } else if (c->symbol == klass || c->derivesFrom(ctx, klass)) {
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

bool Types::canBeTruthy(core::Context ctx, std::shared_ptr<Type> what) {
    if (what->isDynamic()) {
        return true;
    }
    auto truthyPart =
        Types::dropSubtypesOf(ctx, Types::dropSubtypesOf(ctx, what, Symbols::NilClass()), Symbols::FalseClass());
    return !truthyPart->isBottom(); // check if truthyPart is empty
}

bool Types::canBeFalsy(core::Context ctx, std::shared_ptr<Type> what) {
    if (what->isDynamic()) {
        return true;
    }
    return Types::isSubType(ctx, Types::falseClass(), what) ||
           Types::isSubType(ctx, Types::nilClass(),
                            what); // check if inhabited by falsy values
}

std::shared_ptr<Type> Types::approximateSubtract(core::Context ctx, std::shared_ptr<Type> from,
                                                 std::shared_ptr<Type> what) {
    std::shared_ptr<Type> result;
    typecase(what.get(), [&](ClassType *c) { result = Types::dropSubtypesOf(ctx, from, c->symbol); },
             [&](OrType *o) {
                 result = Types::approximateSubtract(ctx, Types::approximateSubtract(ctx, from, o->left), o->right);
             },
             [&](Type *) { result = from; });
    return result;
}

shared_ptr<Type> Types::lubAll(core::Context ctx, vector<shared_ptr<Type>> &elements) {
    shared_ptr<Type> acc = Types::bottom();
    for (auto &el : elements) {
        acc = Types::lub(ctx, acc, el);
    }
    return acc;
}

shared_ptr<Type> Types::arrayOf(core::Context ctx, shared_ptr<Type> elem) {
    vector<shared_ptr<Type>> targs{move(elem)};
    return make_shared<AppliedType>(core::Symbols::Array(), targs);
}

core::ClassType::ClassType(core::SymbolRef symbol) : symbol(symbol) {
    ENFORCE(symbol.exists());
}

core::ProxyType::ProxyType(shared_ptr<core::Type> underlying) : underlying(move(underlying)) {}

void ProxyType::_sanityCheck(core::Context ctx) {
    ENFORCE(cast_type<ClassType>(this->underlying.get()) != nullptr ||
            cast_type<AppliedType>(this->underlying.get()) != nullptr);
    this->underlying->sanityCheck(ctx);
}

bool Type::isDynamic() {
    auto *t = cast_type<ClassType>(this);
    return t != nullptr && t->symbol == core::Symbols::untyped();
}

bool Type::isTop() {
    auto *t = cast_type<ClassType>(this);
    return t != nullptr && t->symbol == core::Symbols::top();
}

bool Type::isBottom() {
    auto *t = cast_type<ClassType>(this);
    return t != nullptr && t->symbol == core::Symbols::bottom();
}

core::LiteralType::LiteralType(int64_t val) : ProxyType(Types::Integer()), value(val) {}

core::LiteralType::LiteralType(double val) : ProxyType(Types::Float()), floatval(val) {}

core::LiteralType::LiteralType(core::SymbolRef klass, core::NameRef val)
    : ProxyType(klass == core::Symbols::String() ? Types::String() : Types::Symbol()), value(val._id) {
    ENFORCE(klass == core::Symbols::String() || klass == core::Symbols::Symbol());
}

core::LiteralType::LiteralType(bool val)
    : ProxyType(val ? Types::trueClass() : Types::falseClass()), value(val ? 1 : 0) {}

core::TupleType::TupleType(shared_ptr<Type> underlying, vector<shared_ptr<Type>> elements)
    : ProxyType(move(underlying)), elems(move(elements)) {}

shared_ptr<Type> core::TupleType::build(core::Context ctx, vector<shared_ptr<Type>> elements) {
    shared_ptr<Type> underlying = Types::arrayOf(ctx, Types::lubAll(ctx, elements));
    return make_shared<TupleType>(move(underlying), move(elements));
}

AndType::AndType(shared_ptr<Type> left, shared_ptr<Type> right) : left(std::move(left)), right(std::move(right)) {}

bool LiteralType::equalsLiteral(const GlobalState &gs, std::shared_ptr<LiteralType> rhs) {
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

OrType::OrType(shared_ptr<Type> left, shared_ptr<Type> right) : left(std::move(left)), right(std::move(right)) {}

void TupleType::_sanityCheck(core::Context ctx) {
    ProxyType::_sanityCheck(ctx);
}

core::ShapeType::ShapeType() : ProxyType(core::Types::hashOfUntyped()) {}

core::ShapeType::ShapeType(vector<shared_ptr<LiteralType>> keys, vector<shared_ptr<Type>> values)
    : ProxyType(core::Types::hashOfUntyped()), keys(move(keys)), values(move(values)) {}

void ShapeType::_sanityCheck(core::Context ctx) {
    ProxyType::_sanityCheck(ctx);
    ENFORCE(this->values.size() == this->keys.size());
    for (auto &v : this->keys) {
        v->_sanityCheck(ctx);
    }
    for (auto &e : this->values) {
        e->_sanityCheck(ctx);
    }
}

MagicType::MagicType() : ProxyType(make_shared<ClassType>(core::Symbols::Magic())) {}

void MagicType::_sanityCheck(core::Context ctx) {
    ProxyType::_sanityCheck(ctx);
}

AliasType::AliasType(core::SymbolRef other) : symbol(other) {}

void AndType::_sanityCheck(core::Context ctx) {
    left->_sanityCheck(ctx);
    right->_sanityCheck(ctx);
    /*
     * This is no longer true. Now we can construct types such as:
     * ShapeType(1 => 1), AppliedType{Array, Integer}
       ENFORCE(!isa_type<ProxyType>(left.get()));
       ENFORCE(!isa_type<ProxyType>(right.get()));

       */

    ENFORCE(!left->isDynamic());
    ENFORCE(!right->isDynamic());
    // TODO: reenable
    //    ENFORCE(!Types::isSubType(ctx, left, right),
    //            this->toString(ctx) + " should have collapsed: " + left->toString(ctx) + " <: " +
    //            right->toString(ctx));
    //    ENFORCE(!Types::isSubType(ctx, right, left),
    //            this->toString(ctx) + " should have collapsed: " + right->toString(ctx) + " <: " +
    //            left->toString(ctx));
}

void OrType::_sanityCheck(core::Context ctx) {
    left->_sanityCheck(ctx);
    right->_sanityCheck(ctx);
    //    ENFORCE(!isa_type<ProxyType>(left.get()));
    //    ENFORCE(!isa_type<ProxyType>(right.get()));
    ENFORCE(!left->isDynamic());
    ENFORCE(!right->isDynamic());
    //  TODO: @dmitry, reenable
    //    ENFORCE(!Types::isSubType(ctx, left, right),
    //            this->toString(ctx) + " should have collapsed: " + left->toString(ctx) + " <: " +
    //            right->toString(ctx));
    //    ENFORCE(!Types::isSubType(ctx, right, left),
    //            this->toString(ctx) + " should have collapsed: " + right->toString(ctx) + " <: " +
    //            left->toString(ctx));
}

void ClassType::_sanityCheck(core::Context ctx) {
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

int MagicType::kind() {
    return 10;
}

int OrType::kind() {
    return 11;
}

int AndType::kind() {
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

bool MagicType::isFullyDefined() {
    return true;
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
std::vector<core::SymbolRef> Types::alignBaseTypeArgs(core::Context ctx, core::SymbolRef what,
                                                      const std::vector<std::shared_ptr<Type>> &targs,
                                                      core::SymbolRef asIf) {
    ENFORCE(asIf.data(ctx).isClass());
    ENFORCE(what.data(ctx).isClass());
    ENFORCE(what == asIf || what.data(ctx).derivesFrom(ctx, asIf) || asIf.data(ctx).derivesFrom(ctx, what),
            what.data(ctx).name.toString(ctx), asIf.data(ctx).name.toString(ctx));
    std::vector<core::SymbolRef> currentAlignment;
    if (targs.empty()) {
        return currentAlignment;
    }

    if (what == asIf || (asIf.data(ctx).isClassClass() && what.data(ctx).isClassClass())) {
        currentAlignment = what.data(ctx).typeMembers();
    } else {
        for (auto originalTp : asIf.data(ctx).typeMembers()) {
            auto name = originalTp.data(ctx).name;
            core::SymbolRef align;
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

std::shared_ptr<Type> Types::resultTypeAsSeenFrom(core::Context ctx, core::SymbolRef what, core::SymbolRef inWhat,
                                                  const std::vector<std::shared_ptr<Type>> &targs) {
    const core::Symbol &original = what.data(ctx);
    core::SymbolRef originalOwner = what.data(ctx).enclosingClass(ctx);

    if (originalOwner.data(ctx).typeMembers().empty() || (original.resultType == nullptr)) {
        return original.resultType;
    }

    std::vector<core::SymbolRef> currentAlignment = alignBaseTypeArgs(ctx, originalOwner, targs, inWhat);

    return instantiate(ctx, original.resultType, currentAlignment, targs);
}

shared_ptr<core::Type> Types::getProcReturnType(core::Context ctx, shared_ptr<core::Type> procType) {
    if (!procType->derivesFrom(ctx, core::Symbols::Proc())) {
        return core::Types::dynamic();
    }
    auto *applied = core::cast_type<core::AppliedType>(procType.get());
    if (applied == nullptr || applied->targs.empty()) {
        return core::Types::dynamic();
    }
    // Proc types have their return type as the first targ
    return applied->targs.front();
}

bool Types::isSubType(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2) {
    return isSubTypeUnderConstraint(ctx, core::TypeConstraint::EmptyFrozenConstraint, t1, t2);
}

bool core::TypeVar::isFullyDefined() {
    return false;
}

std::shared_ptr<Type> core::TypeVar::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    Error::raise("should never happen");
}

bool TypeVar::derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) {
    Error::raise("should never happen");
}

TypeVar::TypeVar(SymbolRef sym) : sym(sym) {}

void TypeVar::_sanityCheck(core::Context ctx) {
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

void AppliedType::_sanityCheck(core::Context ctx) {
    ENFORCE(this->klass.data(ctx).isClass());
    ENFORCE(this->klass != core::Symbols::untyped());

    ENFORCE(this->klass.data(ctx).typeMembers().size() == this->targs.size() ||
                (this->klass == Symbols::Array() && (this->targs.size() == 1)) ||
                (this->klass == Symbols::Hash() && (this->targs.size() == 3)) ||
                this->klass._id >= Symbols::Proc0()._id && this->klass._id <= Symbols::last_proc()._id,
            this->klass.data(ctx).name.toString(ctx));
    for (auto &targ : this->targs) {
        targ->sanityCheck(ctx);
    }
}

std::shared_ptr<Type> AppliedType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    core::SymbolRef method = this->klass.data(ctx).findMemberTransitive(ctx, name);

    if (method.exists()) {
        const core::Symbol &data = method.data(ctx);

        if (data.arguments().size() > i) { // todo: this should become actual argument matching
            shared_ptr<Type> resultType =
                Types::resultTypeAsSeenFrom(ctx, data.arguments()[i], this->klass, this->targs);
            if (!resultType) {
                resultType = Types::dynamic();
            }
            return resultType;
        } else {
            return Types::dynamic();
        }
    } else {
        return Types::dynamic();
    }
}

bool AppliedType::derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) {
    ClassType und(this->klass);
    return und.derivesFrom(gs, klass);
}

LambdaParam::LambdaParam(const SymbolRef definition) : definition(definition) {}
SelfTypeParam::SelfTypeParam(const SymbolRef definition) : definition(definition) {}

bool LambdaParam::derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

bool SelfTypeParam::derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

std::shared_ptr<Type> LambdaParam::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

std::shared_ptr<Type> SelfTypeParam::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return Types::dynamic()->getCallArgumentType(ctx, name, i);
}

std::shared_ptr<Type> LambdaParam::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                                std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfType,
                                                std::shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> block) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

std::shared_ptr<Type> SelfTypeParam::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                                  std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfType,
                                                  std::shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> block) {
    return Types::dynamic()->dispatchCall(ctx, name, callLoc, args, selfType, fullType, block);
}

void LambdaParam::_sanityCheck(core::Context ctx) {}
void SelfTypeParam::_sanityCheck(core::Context ctx) {}

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
    return isDynamic();
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
SendAndBlockLink::SendAndBlockLink(core::SymbolRef block) : block(block), constr(make_shared<core::TypeConstraint>()) {}

shared_ptr<Type> TupleType::elementType() {
    auto *ap = cast_type<AppliedType>(this->underlying.get());
    ENFORCE(ap);
    ENFORCE(ap->klass == core::Symbols::Array());
    ENFORCE(ap->targs.size() == 1);
    return ap->targs.front();
}
