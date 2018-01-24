#include "core/Types.h"
#include "../Context.h"
#include "../Names.h"
#include "../Types.h"
#include "common/common.h"

using namespace ruby_typer;
using namespace ruby_typer::core;
using namespace std;

// improve debugging.
template class std::shared_ptr<ruby_typer::core::Type>;
template class std::vector<core::Loc>;

shared_ptr<Type> Types::top() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_top());
    return res;
}

shared_ptr<Type> Types::bottom() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_bottom());
    return res;
}

shared_ptr<Type> Types::nil() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_NilClass());
    return res;
}

shared_ptr<Type> Types::dynamic() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_untyped());
    return res;
}
std::shared_ptr<Type> Types::trueClass() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_TrueClass());
    return res;
}

std::shared_ptr<Type> Types::falseClass() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_FalseClass());
    return res;
}

std::shared_ptr<Type> Types::Integer() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_Integer());
    return res;
}

std::shared_ptr<Type> Types::Float() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_Float());
    return res;
}

std::shared_ptr<Type> Types::arrayClass() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_Array());
    return res;
}

std::shared_ptr<Type> Types::hashClass() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_Hash());
    return res;
}

std::shared_ptr<Type> Types::arrayOfUntyped() {
    static vector<shared_ptr<Type>> targs{core::Types::dynamic()};
    static auto res = make_shared<core::AppliedType>(core::GlobalState::defn_Array(), targs);
    return res;
}

std::shared_ptr<Type> Types::hashOfUntyped() {
    static vector<shared_ptr<Type>> targs{core::Types::dynamic(), core::Types::dynamic()};
    static auto res = make_shared<core::AppliedType>(core::GlobalState::defn_Hash(), targs);
    return res;
}

std::shared_ptr<Type> Types::procClass() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_Proc());
    return res;
}

std::shared_ptr<Type> Types::String() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_String());
    return res;
}

std::shared_ptr<Type> Types::Symbol() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_Symbol());
    return res;
}

std::shared_ptr<Type> Types::Object() {
    static auto res = make_shared<ClassType>(core::GlobalState::defn_Object());
    return res;
}

std::shared_ptr<Type> Types::falsyTypes() {
    static auto res = OrType::make_shared(Types::nil(), Types::falseClass());
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
                                                     klass.info(ctx).fullName(ctx) + ") returned " +
                                                     result->toString(ctx) + ", which is not a subtype of the input");
    return result;
}

bool Types::canBeTruthy(core::Context ctx, std::shared_ptr<Type> what) {
    if (what->isDynamic()) {
        return true;
    }
    auto truthyPart = Types::dropSubtypesOf(ctx, Types::dropSubtypesOf(ctx, what, GlobalState::defn_NilClass()),
                                            GlobalState::defn_FalseClass());
    return !truthyPart->isBottom(); // check if truthyPart is empty
}

bool Types::canBeFalsy(core::Context ctx, std::shared_ptr<Type> what) {
    if (what->isDynamic()) {
        return true;
    }
    return Types::isSubType(ctx, Types::falseClass(), what) ||
           Types::isSubType(ctx, Types::nil(),
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

ruby_typer::core::ClassType::ClassType(ruby_typer::core::SymbolRef symbol) : symbol(symbol) {
    ENFORCE(symbol.exists());
}

namespace {
string classNameToString(GlobalState &gs, core::NameRef nm) {
    core::Name &name = nm.name(gs);
    if (name.kind == core::CONSTANT) {
        return name.cnst.original.toString(gs);
    } else {
        ENFORCE(name.kind == core::UNIQUE);
        ENFORCE(name.unique.uniqueNameKind == core::Singleton);
        return "<class:" + classNameToString(gs, name.unique.original) + ">";
    }
}
}; // namespace

string ruby_typer::core::ClassType::toString(GlobalState &gs, int tabs) {
    return classNameToString(gs, this->symbol.info(gs).name);
}

string ruby_typer::core::ClassType::typeName() {
    return "ClassType";
}

ruby_typer::core::ProxyType::ProxyType(shared_ptr<ruby_typer::core::Type> underlying) : underlying(move(underlying)) {}

void ProxyType::_sanityCheck(core::Context ctx) {
    ENFORCE(cast_type<ClassType>(this->underlying.get()) != nullptr ||
            cast_type<AppliedType>(this->underlying.get()) != nullptr);
    this->underlying->sanityCheck(ctx);
}

bool Type::isDynamic() {
    auto *t = cast_type<ClassType>(this);
    return t != nullptr && t->symbol == core::GlobalState::defn_untyped();
}

bool Type::isTop() {
    auto *t = dynamic_cast<ClassType *>(this);
    return t != nullptr && t->symbol == core::GlobalState::defn_top();
}

bool Type::isBottom() {
    auto *t = cast_type<ClassType>(this);
    return t != nullptr && t->symbol == core::GlobalState::defn_bottom();
}

ruby_typer::core::LiteralType::LiteralType(int64_t val) : ProxyType(Types::Integer()), value(val) {}

ruby_typer::core::LiteralType::LiteralType(double val)
    : ProxyType(Types::Float()), value(*reinterpret_cast<u8 *>(&val)) {}

ruby_typer::core::LiteralType::LiteralType(core::SymbolRef klass, core::NameRef val)
    : ProxyType(klass == core::GlobalState::defn_String() ? Types::String() : Types::Symbol()), value(val._id) {
    ENFORCE(klass == core::GlobalState::defn_String() || klass == core::GlobalState::defn_Symbol());
}

ruby_typer::core::LiteralType::LiteralType(bool val)
    : ProxyType(val ? Types::trueClass() : Types::falseClass()), value(val ? 1 : 0) {}

string LiteralType::typeName() {
    return "LiteralType";
}

string LiteralType::toString(GlobalState &gs, int tabs) {
    string value;
    SymbolRef undSymbol = cast_type<ClassType>(this->underlying.get())->symbol;
    if (undSymbol == GlobalState::defn_String()) {
        value = "\"" + NameRef(gs, this->value).toString(gs) + "\"";
    } else if (undSymbol == GlobalState::defn_Symbol()) {
        value = ":\"" + NameRef(gs, this->value).toString(gs) + "\"";
    } else if (undSymbol == GlobalState::defn_Integer()) {
        value = to_string(this->value);
    } else if (undSymbol == GlobalState::defn_Float()) {
        value = to_string(*reinterpret_cast<double *>(&(this->value)));
    } else if (undSymbol == GlobalState::defn_TrueClass()) {
        value = "true";
    } else if (undSymbol == GlobalState::defn_FalseClass()) {
        value = "false";
    } else {
        Error::raise("should not be reachable");
    }
    return this->underlying->toString(gs, tabs) + "(" + value + ")";
}

ruby_typer::core::TupleType::TupleType(vector<shared_ptr<Type>> &elements)
    : ProxyType(Types::arrayOfUntyped()), elems(move(elements)) {}

string TupleType::typeName() {
    return "TupleType";
}

string ShapeType::typeName() {
    return "ShapeType";
}

string MagicType::typeName() {
    return "MagicType";
}

string AliasType::typeName() {
    return "AliasType";
}

string AndType::typeName() {
    return "AndType";
}

AndType::AndType(shared_ptr<Type> left, shared_ptr<Type> right) : left(left), right(right) {}

OrType::OrType(shared_ptr<Type> left, shared_ptr<Type> right) : left(left), right(right) {}

string OrType::typeName() {
    return "OrType";
}

void printTabs(stringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

string TupleType::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "TupleType {" << endl;
    int i = -1;
    for (auto &el : this->elems) {
        i++;
        printTabs(buf, tabs + 1);
        buf << i << " = " << el->toString(gs, tabs + 3) << endl;
    }
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

void TupleType::_sanityCheck(core::Context ctx) {
    ProxyType::_sanityCheck(ctx);
}

ruby_typer::core::ShapeType::ShapeType() : ProxyType(core::Types::hashOfUntyped()) {}

ruby_typer::core::ShapeType::ShapeType(vector<shared_ptr<LiteralType>> &keys, vector<shared_ptr<Type>> &values)
    : ProxyType(core::Types::hashOfUntyped()), keys(move(keys)), values(move(values)) {}

string ShapeType::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "ShapeType {" << endl;
    auto valueIterator = this->values.begin();
    for (auto &el : this->keys) {
        printTabs(buf, tabs + 1);
        buf << el->toString(gs, tabs + 2) << " => " << (*valueIterator)->toString(gs, tabs + 2) << endl;
        ++valueIterator;
    }
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

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

MagicType::MagicType() : ProxyType(make_shared<ClassType>(core::GlobalState::defn_Magic())) {}

string MagicType::toString(GlobalState &gs, int tabs) {
    return underlying->toString(gs, tabs);
}

void MagicType::_sanityCheck(core::Context ctx) {
    ProxyType::_sanityCheck(ctx);
}

AliasType::AliasType(core::SymbolRef other) : symbol(other) {}

string AliasType::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "AliasType { symbol = " << this->symbol.info(gs).fullName(gs) << " }";
    return buf.str();
}

string AndType::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    bool leftBrace = isa_type<OrType>(this->left.get());
    bool rightBrace = isa_type<OrType>(this->right.get());

    if (leftBrace) {
        buf << "(";
    }
    buf << this->left->toString(gs, tabs + 2);
    if (leftBrace) {
        buf << ")";
    }
    buf << " & ";
    if (rightBrace) {
        buf << "(";
    }
    buf << this->right->toString(gs, tabs + 2);
    if (rightBrace) {
        buf << ")";
    }
    return buf.str();
}

string OrType::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    bool leftBrace = isa_type<AndType>(this->left.get());
    bool rightBrace = isa_type<AndType>(this->right.get());

    if (leftBrace) {
        buf << "(";
    }
    buf << this->left->toString(gs, tabs + 2);
    if (leftBrace) {
        buf << ")";
    }
    buf << " | ";
    if (rightBrace) {
        buf << "(";
    }
    buf << this->right->toString(gs, tabs + 2);
    if (rightBrace) {
        buf << ")";
    }
    return buf.str();
}

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
}

void OrType::_sanityCheck(core::Context ctx) {
    left->_sanityCheck(ctx);
    right->_sanityCheck(ctx);
    //    ENFORCE(!isa_type<ProxyType>(left.get()));
    //    ENFORCE(!isa_type<ProxyType>(right.get()));
    ENFORCE(!left->isDynamic());
    ENFORCE(!right->isDynamic());
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

int TypeConstructor::kind() {
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

ruby_typer::core::TypeVar::TypeVar(NameRef name) : name(name) {}

std::vector<core::SymbolRef> Types::alignBaseTypeArgs(core::Context ctx, core::SymbolRef what,
                                                      std::vector<std::shared_ptr<Type>> &targs, core::SymbolRef asIf) {
    ENFORCE(asIf.info(ctx).isClass());
    ENFORCE(what.info(ctx).isClass());
    std::vector<core::SymbolRef> currentAlignment;
    if (targs.empty()) {
        return currentAlignment;
    }

    if (what == asIf || asIf.info(ctx).isClassClass()) {
        currentAlignment = asIf.info(ctx).typeMembers();
    } else {
        for (auto originalTp : asIf.info(ctx).typeMembers()) {
            auto name = originalTp.info(ctx).name;
            core::SymbolRef align;
            int i = 0;
            for (auto x : what.info(ctx).typeMembers()) {
                if (x.info(ctx).name == name) {
                    align = x;
                    currentAlignment.push_back(x);
                    break;
                }
                i++;
            }
            ENFORCE(align.exists());
        }
    }
    ENFORCE(currentAlignment.size() == asIf.info(ctx).typeMembers().size());
    return currentAlignment;
}

std::shared_ptr<Type> Types::resultTypeAsSeenFrom(core::Context ctx, core::SymbolRef what, core::SymbolRef inWhat,
                                                  std::vector<std::shared_ptr<Type>> &targs) {
    core::Symbol &original = what.info(ctx);
    core::SymbolRef originalOwner = ctx.withOwner(what).enclosingClass();

    if (originalOwner.info(ctx).typeMembers().empty() || (original.resultType == nullptr)) {
        return original.resultType;
    }

    std::vector<core::SymbolRef> currentAlignment = alignBaseTypeArgs(ctx, originalOwner, targs, inWhat);

    auto res1 = original.resultType->instantiate(currentAlignment, targs);
    if (res1) {
        return res1;
    }
    return original.resultType;
}

bool ruby_typer::core::TypeVar::isFullyDefined() {
    return this->isInstantiated;
}

std::shared_ptr<Type> ruby_typer::core::TypeVar::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    ENFORCE(isInstantiated);
    return instantiation->getCallArgumentType(ctx, name, i);
}

bool TypeVar::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    ENFORCE(isInstantiated);
    return instantiation->derivesFrom(ctx, klass);
}

std::string TypeVar::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "TypeVar(" + name.toString(gs) + ") {" << endl;
    printTabs(buf, tabs + 1);
    buf << "instantiated = " + std::to_string(isInstantiated) << endl;
    if (isInstantiated) {
        printTabs(buf, tabs + 1);
        buf << "instantiation = " << this->instantiation->toString(gs, tabs + 1) << endl;
    }
    if (!this->upperConstraints.empty()) {
        printTabs(buf, tabs + 1);
        buf << "upperConstraints = [" << endl;
        int i = 0;
        for (auto &el : this->upperConstraints) {
            printTabs(buf, tabs + 2);
            buf << i++ << " = " << el->toString(gs, tabs + 3) << endl;
        }
        printTabs(buf, tabs + 1);
        buf << "]" << endl;
    }
    if (!this->lowerConstraints.empty()) {
        printTabs(buf, tabs + 1);
        buf << "lowerConstraints = [" << endl;
        int i = 0;
        for (auto &el : this->lowerConstraints) {
            printTabs(buf, tabs + 2);
            buf << i++ << " = " << el->toString(gs, tabs + 3) << endl;
        }
        printTabs(buf, tabs + 1);
        buf << "]" << endl;
    }
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

std::string TypeVar::typeName() {
    return "TypeVar";
}

void TypeVar::_sanityCheck(core::Context ctx) {
    ENFORCE(this->name.exists());
    for (auto &t : this->upperConstraints) {
        t->_sanityCheck(ctx);
    }
    for (auto &t : this->lowerConstraints) {
        t->_sanityCheck(ctx);
    }
}

std::shared_ptr<Type> TypeVar::instantiate(std::vector<SymbolRef> params, std::vector<std::shared_ptr<Type>> &targs) {
    Error::notImplemented();
}

std::shared_ptr<Type> ClassType::instantiate(std::vector<SymbolRef> params, std::vector<std::shared_ptr<Type>> &targs) {
    return nullptr;
}

std::shared_ptr<Type> LiteralType::instantiate(std::vector<SymbolRef> params,
                                               std::vector<std::shared_ptr<Type>> &targs) {
    return nullptr;
}

std::shared_ptr<Type> MagicType::instantiate(std::vector<SymbolRef> params, std::vector<std::shared_ptr<Type>> &targs) {
    return nullptr;
}

std::shared_ptr<Type> TupleType::instantiate(std::vector<SymbolRef> params, std::vector<std::shared_ptr<Type>> &targs) {
    bool changed = false;
    std::vector<std::shared_ptr<Type>> newElems;
    for (auto &a : this->elems) {
        auto t = a->instantiate(params, targs);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newElems.push_back(t);
        } else {
            newElems.push_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newElems[i]) {
            newElems[i] = this->elems[i];
            i++;
        }
        return make_shared<TupleType>(newElems);
    }
    return nullptr;
}

std::shared_ptr<Type> ShapeType::instantiate(std::vector<SymbolRef> params, std::vector<std::shared_ptr<Type>> &targs) {
    bool changed = false;
    std::vector<std::shared_ptr<Type>> newValues;
    for (auto &a : this->values) {
        auto t = a->instantiate(params, targs);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newValues.push_back(t);
        } else {
            newValues.push_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newValues[i]) {
            newValues[i] = this->values[i];
            i++;
        }
        return make_shared<ShapeType>(this->keys, newValues);
    }
    return nullptr;
}

std::shared_ptr<Type> OrType::instantiate(std::vector<SymbolRef> params, std::vector<std::shared_ptr<Type>> &targs) {
    auto left = this->left->instantiate(params, targs);
    auto right = this->right->instantiate(params, targs);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return OrType::make_shared(left, right);
    }
    return nullptr;
}

std::shared_ptr<Type> AndType::instantiate(std::vector<SymbolRef> params, std::vector<std::shared_ptr<Type>> &targs) {
    auto left = this->left->instantiate(params, targs);
    auto right = this->right->instantiate(params, targs);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return AndType::make_shared(left, right);
    }
    return nullptr;
}

std::string AppliedType::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "AppliedType {" << endl;
    printTabs(buf, tabs + 1);
    buf << "klass = " << this->klass.info(gs).fullName(gs) << endl;

    printTabs(buf, tabs + 1);
    buf << "targs = [" << endl;
    int i = 0;
    for (auto &targ : this->targs) {
        printTabs(buf, tabs + 2);
        buf << i << " = " << targ->toString(gs, tabs + 3) << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "]" << endl;

    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

bool AppliedType::isFullyDefined() {
    for (auto &targ : this->targs) {
        if (!targ->isFullyDefined()) {
            return false;
        }
    }
    return true;
}

std::string AppliedType::typeName() {
    return "AppliedType";
}

void AppliedType::_sanityCheck(core::Context ctx) {
    ENFORCE(this->klass.info(ctx).isClass());
    ENFORCE(this->klass.info(ctx).typeMembers().size() == this->targs.size());
    for (auto &targ : this->targs) {
        targ->sanityCheck(ctx);
    }
}

std::shared_ptr<Type> AppliedType::instantiate(std::vector<SymbolRef> params,
                                               std::vector<std::shared_ptr<Type>> &targs) {
    bool changed = false;
    std::vector<std::shared_ptr<Type>> newTargs;
    // TODO: make it not allocate if returns nullptr
    for (auto &a : this->targs) {
        auto t = a->instantiate(params, targs);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newTargs.push_back(t);
        } else {
            newTargs.push_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newTargs[i]) {
            newTargs[i] = this->targs[i];
            i++;
        }
        return make_shared<AppliedType>(this->klass, newTargs);
    }

    return nullptr;
}

std::shared_ptr<Type> AppliedType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    core::SymbolRef method = this->klass.info(ctx).findMemberTransitive(ctx, name);

    if (method.exists()) {
        core::Symbol &info = method.info(ctx);

        if (info.arguments().size() > i) { // todo: this should become actual argument matching
            shared_ptr<Type> resultType =
                Types::resultTypeAsSeenFrom(ctx, info.arguments()[i], this->klass, this->targs);
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

bool AppliedType::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    ClassType und(this->klass);
    return und.derivesFrom(ctx, klass);
}

LambdaParam::LambdaParam(const SymbolRef definition) : definition(definition) {}

std::string LambdaParam::toString(GlobalState &gs, int tabs) {
    return "LambdaParam(" + this->definition.toString(gs) + ")";
}

std::string LambdaParam::typeName() {
    return "LambdaParam";
}

bool LambdaParam::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    Error::raise("should not happen");
}

std::shared_ptr<Type> LambdaParam::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    Error::raise("should not happen");
}

std::shared_ptr<Type> LambdaParam::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                                std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                                bool hasBlock) {
    Error::raise("should not happen");
}

void LambdaParam::_sanityCheck(core::Context ctx) {}

bool LambdaParam::isFullyDefined() {
    return false;
}

std::shared_ptr<Type> LambdaParam::instantiate(std::vector<SymbolRef> params,
                                               std::vector<std::shared_ptr<Type>> &targs) {
    int i = 0;
    for (auto el : params) {
        if (el == this->definition) {
            return targs[i];
        }
    }
    return nullptr;
}
