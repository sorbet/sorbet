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
    static auto res = make_shared<ClassType>(core::Symbols::top());
    return res;
}

shared_ptr<Type> Types::bottom() {
    static auto res = make_shared<ClassType>(core::Symbols::bottom());
    return res;
}

shared_ptr<Type> Types::nil() {
    static auto res = make_shared<ClassType>(core::Symbols::NilClass());
    return res;
}

shared_ptr<Type> Types::dynamic() {
    static auto res = make_shared<ClassType>(core::Symbols::untyped());
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
string classNameToString(const GlobalState &gs, core::NameRef nm) {
    const core::Name &name = nm.data(gs);
    if (name.kind == core::CONSTANT) {
        return name.cnst.original.toString(gs);
    } else {
        ENFORCE(name.kind == core::UNIQUE);
        ENFORCE(name.unique.uniqueNameKind == core::Singleton);
        return "<Class:" + classNameToString(gs, name.unique.original) + ">";
    }
}
}; // namespace

string ruby_typer::core::ClassType::toString(const GlobalState &gs, int tabs) {
    return classNameToString(gs, this->symbol.data(gs).name);
}

string ruby_typer::core::ClassType::show(const GlobalState &gs) {
    return classNameToString(gs, this->symbol.data(gs).name);
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
    return t != nullptr && t->symbol == core::Symbols::untyped();
}

bool Type::isTop() {
    auto *t = dynamic_cast<ClassType *>(this);
    return t != nullptr && t->symbol == core::Symbols::top();
}

bool Type::isBottom() {
    auto *t = cast_type<ClassType>(this);
    return t != nullptr && t->symbol == core::Symbols::bottom();
}

ruby_typer::core::LiteralType::LiteralType(int64_t val) : ProxyType(Types::Integer()), value(val) {}

ruby_typer::core::LiteralType::LiteralType(double val)
    : ProxyType(Types::Float()), value(*reinterpret_cast<u8 *>(&val)) {}

ruby_typer::core::LiteralType::LiteralType(core::SymbolRef klass, core::NameRef val)
    : ProxyType(klass == core::Symbols::String() ? Types::String() : Types::Symbol()), value(val._id) {
    ENFORCE(klass == core::Symbols::String() || klass == core::Symbols::Symbol());
}

ruby_typer::core::LiteralType::LiteralType(bool val)
    : ProxyType(val ? Types::trueClass() : Types::falseClass()), value(val ? 1 : 0) {}

string LiteralType::typeName() {
    return "LiteralType";
}

string LiteralType::toString(const GlobalState &gs, int tabs) {
    string value;
    SymbolRef undSymbol = cast_type<ClassType>(this->underlying.get())->symbol;
    if (undSymbol == Symbols::String()) {
        value = "\"" + NameRef(gs, this->value).toString(gs) + "\"";
    } else if (undSymbol == Symbols::Symbol()) {
        value = ":\"" + NameRef(gs, this->value).toString(gs) + "\"";
    } else if (undSymbol == Symbols::Integer()) {
        value = to_string(this->value);
    } else if (undSymbol == Symbols::Float()) {
        value = to_string(*reinterpret_cast<double *>(&(this->value)));
    } else if (undSymbol == Symbols::TrueClass()) {
        value = "true";
    } else if (undSymbol == Symbols::FalseClass()) {
        value = "false";
    } else {
        Error::raise("should not be reachable");
    }
    return this->underlying->toString(gs, tabs) + "(" + value + ")";
}

string LiteralType::show(const GlobalState &gs) {
    return this->underlying->show(gs);
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

string TupleType::toString(const GlobalState &gs, int tabs) {
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

string TupleType::show(const GlobalState &gs) {
    stringstream buf;
    buf << "[";
    bool first = true;
    for (auto &el : this->elems) {
        if (first) {
            first = false;
        } else {
            buf << ", ";
        }
        buf << el->show(gs);
    }
    buf << "]";
    return buf.str();
}

void TupleType::_sanityCheck(core::Context ctx) {
    ProxyType::_sanityCheck(ctx);
}

ruby_typer::core::ShapeType::ShapeType() : ProxyType(core::Types::hashOfUntyped()) {}

ruby_typer::core::ShapeType::ShapeType(vector<shared_ptr<LiteralType>> &keys, vector<shared_ptr<Type>> &values)
    : ProxyType(core::Types::hashOfUntyped()), keys(move(keys)), values(move(values)) {}

string ShapeType::toString(const GlobalState &gs, int tabs) {
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

string ShapeType::show(const GlobalState &gs) {
    stringstream buf;
    buf << "{";
    auto valueIterator = this->values.begin();
    bool first = true;
    for (auto &key : this->keys) {
        if (first) {
            first = false;
        } else {
            buf << ", ";
        }
        SymbolRef undSymbol = cast_type<ClassType>(key->underlying.get())->symbol;
        if (undSymbol == Symbols::Symbol()) {
            buf << NameRef(gs, key->value).toString(gs) << ": ";
        } else {
            buf << key->show(gs) << " => ";
        }
        buf << (*valueIterator)->show(gs);
        ++valueIterator;
    }
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

MagicType::MagicType() : ProxyType(make_shared<ClassType>(core::Symbols::Magic())) {}

string MagicType::toString(const GlobalState &gs, int tabs) {
    return underlying->toString(gs, tabs);
}

string MagicType::show(const GlobalState &gs) {
    return underlying->show(gs);
}

void MagicType::_sanityCheck(core::Context ctx) {
    ProxyType::_sanityCheck(ctx);
}

AliasType::AliasType(core::SymbolRef other) : symbol(other) {}

string AliasType::toString(const GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "AliasType { symbol = " << this->symbol.data(gs).fullName(gs) << " }";
    return buf.str();
}

string AliasType::show(const GlobalState &gs) {
    Error::raise("should never happen");
}

string AndType::toString(const GlobalState &gs, int tabs) {
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

string AndType::show(const GlobalState &gs) {
    stringstream buf;

    buf << "T.all(";
    buf << this->left->show(gs);
    buf << ", ";
    buf << this->right->show(gs);
    buf << ")";
    return buf.str();
}

string OrType::toString(const GlobalState &gs, int tabs) {
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

string OrType::show(const GlobalState &gs) {
    stringstream buf;

    buf << "T.any(";
    buf << this->left->show(gs);
    buf << ", ";
    buf << this->right->show(gs);
    buf << ")";
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

ruby_typer::core::TypeVar::TypeVar(NameRef name) : name(name) {}

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
    core::Symbol &original = what.data(ctx);
    core::SymbolRef originalOwner = what.data(ctx).enclosingClass(ctx);

    if (originalOwner.data(ctx).typeMembers().empty() || (original.resultType == nullptr)) {
        return original.resultType;
    }

    std::vector<core::SymbolRef> currentAlignment = alignBaseTypeArgs(ctx, originalOwner, targs, inWhat);

    auto res1 = original.resultType->instantiate(ctx, currentAlignment, targs);
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

std::string TypeVar::toString(const GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "TypeVar(" + name.toString(gs) + ") {" << endl;
    printTabs(buf, tabs + 1);
    buf << "instantiated = " + std::to_string(static_cast<int>(isInstantiated)) << endl;
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

std::string TypeVar::show(const GlobalState &gs) {
    if (isInstantiated) {
        return this->instantiation->show(gs);
    } else {
        return name.toString(gs);
    }
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

std::shared_ptr<Type> TypeVar::instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                           const std::vector<std::shared_ptr<Type>> &targs) {
    Error::notImplemented();
}

std::shared_ptr<Type> ClassType::instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                             const std::vector<std::shared_ptr<Type>> &targs) {
    return nullptr;
}

std::shared_ptr<Type> LiteralType::instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) {
    return nullptr;
}

std::shared_ptr<Type> MagicType::instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                             const std::vector<std::shared_ptr<Type>> &targs) {
    return nullptr;
}

std::shared_ptr<Type> TupleType::instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                             const std::vector<std::shared_ptr<Type>> &targs) {
    bool changed = false;
    std::vector<std::shared_ptr<Type>> newElems;
    for (auto &a : this->elems) {
        auto t = a->instantiate(ctx, params, targs);
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

std::shared_ptr<Type> ShapeType::instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                             const std::vector<std::shared_ptr<Type>> &targs) {
    bool changed = false;
    std::vector<std::shared_ptr<Type>> newValues;
    for (auto &a : this->values) {
        auto t = a->instantiate(ctx, params, targs);
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

std::shared_ptr<Type> OrType::instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                          const std::vector<std::shared_ptr<Type>> &targs) {
    auto left = this->left->instantiate(ctx, params, targs);
    auto right = this->right->instantiate(ctx, params, targs);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::buildOr(ctx, left, right);
    }
    return nullptr;
}

std::shared_ptr<Type> AndType::instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                           const std::vector<std::shared_ptr<Type>> &targs) {
    auto left = this->left->instantiate(ctx, params, targs);
    auto right = this->right->instantiate(ctx, params, targs);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::buildAnd(ctx, left, right);
    }
    return nullptr;
}

std::string AppliedType::toString(const GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "AppliedType {" << endl;
    printTabs(buf, tabs + 1);
    buf << "klass = " << this->klass.data(gs).fullName(gs) << endl;

    printTabs(buf, tabs + 1);
    buf << "targs = [" << endl;
    int i = -1;
    for (auto &targ : this->targs) {
        ++i;
        printTabs(buf, tabs + 2);
        auto tyMem = this->klass.data(gs).typeMembers()[i];
        buf << tyMem.data(gs).name.toString(gs) << " = " << targ->toString(gs, tabs + 3) << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "]" << endl;

    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

std::string AppliedType::show(const GlobalState &gs) {
    stringstream buf;
    if (this->klass == core::Symbols::Array()) {
        buf << "T::Array";
    } else if (this->klass == core::Symbols::Hash()) {
        buf << "T::Hash";
    } else {
        buf << classNameToString(gs, this->klass.data(gs).name);
    }
    buf << "[";

    bool first = true;
    for (auto &targ : this->targs) {
        if (first) {
            first = false;
        } else {
            buf << ", ";
        }
        buf << targ->show(gs);
    }

    buf << "]";
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
    ENFORCE(this->klass.data(ctx).isClass());
    ENFORCE(this->klass.data(ctx).typeMembers().size() == this->targs.size() ||
                (this->klass == Symbols::Hash() && (this->targs.size() == 3)),
            this->klass.data(ctx).name.toString(ctx));
    for (auto &targ : this->targs) {
        targ->sanityCheck(ctx);
    }
}

std::shared_ptr<Type> AppliedType::instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) {
    bool changed = false;
    std::vector<std::shared_ptr<Type>> newTargs;
    // TODO: make it not allocate if returns nullptr
    for (auto &a : this->targs) {
        auto t = a->instantiate(ctx, params, targs);
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
    core::SymbolRef method = this->klass.data(ctx).findMemberTransitive(ctx, name);

    if (method.exists()) {
        core::Symbol &data = method.data(ctx);

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

bool AppliedType::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    ClassType und(this->klass);
    return und.derivesFrom(ctx, klass);
}

LambdaParam::LambdaParam(const SymbolRef definition) : definition(definition) {}
SelfTypeParam::SelfTypeParam(const SymbolRef definition) : definition(definition) {}

std::string LambdaParam::toString(const GlobalState &gs, int tabs) {
    return "LambdaParam(" + this->definition.data(gs).fullName(gs) + ")";
}

std::string LambdaParam::show(const GlobalState &gs) {
    return classNameToString(gs, this->definition.data(gs).name);
}

std::string SelfTypeParam::toString(const GlobalState &gs, int tabs) {
    return "SelfTypeParam(" + this->definition.data(gs).fullName(gs) + ")";
}

std::string SelfTypeParam::show(const GlobalState &gs) {
    return classNameToString(gs, this->definition.data(gs).name);
}

std::string LambdaParam::typeName() {
    return "LambdaParam";
}

std::string SelfTypeParam::typeName() {
    return "SelfTypeParam";
}

bool LambdaParam::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

bool SelfTypeParam::derivesFrom(core::Context ctx, core::SymbolRef klass) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

std::shared_ptr<Type> LambdaParam::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

std::shared_ptr<Type> SelfTypeParam::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return Types::dynamic()->getCallArgumentType(ctx, name, i);
}

std::shared_ptr<Type> LambdaParam::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                                std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                                shared_ptr<Type> *block) {
    Error::raise("not implemented, not clear what it should do. Let's see this fire first.");
}

std::shared_ptr<Type> SelfTypeParam::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                                  std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                                  shared_ptr<Type> *block) {
    return Types::dynamic()->dispatchCall(ctx, name, callLoc, args, fullType, block);
}

void LambdaParam::_sanityCheck(core::Context ctx) {}
void SelfTypeParam::_sanityCheck(core::Context ctx) {}

bool LambdaParam::isFullyDefined() {
    return false;
}

bool SelfTypeParam::isFullyDefined() {
    return true;
}

std::shared_ptr<Type> SelfTypeParam::instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                                 const std::vector<std::shared_ptr<Type>> &targs) {
    return nullptr;
}

std::shared_ptr<Type> LambdaParam::instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) {
    for (auto &el : params) {
        if (el == this->definition) {
            return targs[&el - &params.front()];
        }
    }
    return nullptr;
}
