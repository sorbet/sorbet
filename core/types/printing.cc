#include "absl/base/casts.h"
#include "absl/strings/escaping.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"

using namespace std;

namespace sorbet::core {

namespace {

string buildTabs(int count) {
    string res(count * 2, ' ');
    return res;
}

} // namespace

string ClassType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return this->show(gs);
}

string ClassType::show(const GlobalState &gs) const {
    return this->symbol.data(gs)->show(gs);
}

string UnresolvedClassType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return this->show(gs);
}

string UnresolvedClassType::show(const GlobalState &gs) const {
    return fmt::format(
        "{}::{} (unresolved)", this->scope.data(gs)->show(gs),
        fmt::map_join(this->names, "::", [&](const auto &el) -> string { return el.data(gs)->show(gs); }));
}

string LiteralType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return fmt::format("{}({})", this->underlying()->toStringWithTabs(gs, tabs), showValue(gs));
}

string LiteralType::show(const GlobalState &gs) const {
    return fmt::format("{}({})", this->underlying()->show(gs), showValue(gs));
}

string LiteralType::showValue(const GlobalState &gs) const {
    SymbolRef undSymbol = cast_type<ClassType>(this->underlying().get())->symbol;
    if (undSymbol == Symbols::String()) {
        return fmt::format("\"{}\"", absl::CEscape(NameRef(gs, this->value).show(gs)));
    } else if (undSymbol == Symbols::Symbol()) {
        return fmt::format(":\"{}\"", absl::CEscape(NameRef(gs, this->value).show(gs)));
    } else if (undSymbol == Symbols::Integer()) {
        return to_string(this->value);
    } else if (undSymbol == Symbols::Float()) {
        return to_string(absl::bit_cast<double>(this->value));
    } else if (undSymbol == Symbols::TrueClass()) {
        return "true";
    } else if (undSymbol == Symbols::FalseClass()) {
        return "false";
    } else {
        Exception::raise("should not be reachable");
    }
}

string TupleType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    auto thisTabs = buildTabs(tabs);
    auto nestedTabs = buildTabs(tabs + 1);
    fmt::format_to(buf, "TupleType {{\n");
    int i = -1;
    for (auto &el : this->elems) {
        i++;
        fmt::format_to(buf, "{}{} = {}\n", nestedTabs, i, el->toStringWithTabs(gs, tabs + 3));
    }
    fmt::format_to(buf, "{}}}", thisTabs);
    return to_string(buf);
}

string TupleType::show(const GlobalState &gs) const {
    return fmt::format("[{}]",
                       fmt::map_join(this->elems, ", ", [&](const auto &el) -> string { return el->show(gs); }));
}

string TupleType::showWithMoreInfo(const GlobalState &gs) const {
    return fmt::format("{} ({}-tuple)", show(gs), this->elems.size());
}

string ShapeType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    auto thisTabs = buildTabs(tabs);
    auto nestedTabs = buildTabs(tabs + 1);
    fmt::format_to(buf, "ShapeType {{\n");
    auto valueIterator = this->values.begin();
    for (auto &el : this->keys) {
        fmt::format_to(buf, "{}{} => {}\n", nestedTabs, el->toStringWithTabs(gs, tabs + 2),
                       (*valueIterator)->toStringWithTabs(gs, tabs + 3));
        ++valueIterator;
    }
    fmt::format_to(buf, "{}}}", thisTabs);
    return to_string(buf);
}

string ShapeType::show(const GlobalState &gs) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{{");
    auto valueIterator = this->values.begin();
    bool first = true;
    for (auto &key : this->keys) {
        if (first) {
            first = false;
        } else {
            fmt::format_to(buf, ", ");
        }
        SymbolRef undSymbol = cast_type<ClassType>(cast_type<LiteralType>(key.get())->underlying().get())->symbol;
        if (undSymbol == Symbols::Symbol()) {
            fmt::format_to(buf, "{}: {}", NameRef(gs, cast_type<LiteralType>(key.get())->value).show(gs),
                           (*valueIterator)->show(gs));
        } else {
            fmt::format_to(buf, "{} => {}", key->show(gs), (*valueIterator)->show(gs));
        }
        ++valueIterator;
    }
    fmt::format_to(buf, "}}");
    return to_string(buf);
}

string AliasType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return fmt::format("AliasType {{ symbol = {} }}", this->symbol.data(gs)->toStringFullName(gs));
}

string AliasType::show(const GlobalState &gs) const {
    return fmt::format("<Alias: {} >", this->symbol.data(gs)->showFullName(gs));
}

string AndType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    bool leftBrace = isa_type<OrType>(this->left.get());
    bool rightBrace = isa_type<OrType>(this->right.get());

    return fmt::format("{}{}{} & {}{}{}", leftBrace ? "(" : "", this->left->toStringWithTabs(gs, tabs + 2),
                       leftBrace ? ")" : "", rightBrace ? "(" : "", this->right->toStringWithTabs(gs, tabs + 2),
                       rightBrace ? ")" : "");
}

string AndType::show(const GlobalState &gs) const {
    return fmt::format("T.all({}, {})", this->left->show(gs), this->right->show(gs));
}

string OrType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    bool leftBrace = isa_type<AndType>(this->left.get());
    bool rightBrace = isa_type<AndType>(this->right.get());

    return fmt::format("{}{}{} | {}{}{}", leftBrace ? "(" : "", this->left->toStringWithTabs(gs, tabs + 2),
                       leftBrace ? ")" : "", rightBrace ? "(" : "", this->right->toStringWithTabs(gs, tabs + 2),
                       rightBrace ? ")" : "");
}

/**
 * Metadata collected while traversing OrType for pretty-printing.
 */
struct OrInfo {
    /// True when the leaves of the OrType contains a NilClass.
    bool containsNil{false};

    /// True when the leaves of the OrType contains a FalseClass.
    bool containsFalse{false};

    /// True when the leaves of the OrType contains a TrueClass.
    bool containsTrue{false};

    /// True when the leaves of the OrType contains a type that is none of the
    /// obove cases is present (Integer, for example).
    bool containsOther{false};

    /// True when there are more than one non-NilClass types present in the
    /// leaves of the OrTYpe.
    bool containsMultiple{false};

    bool isBoolean() const {
        return containsTrue && containsFalse && !containsOther;
    }

    void markContainsMultiple() {
        this->containsMultiple = true;
    }

    OrInfo() {}

    static OrInfo nilInfo() {
        OrInfo res;
        res.containsNil = true;
        return res;
    }

    static OrInfo trueInfo() {
        OrInfo res;
        res.containsTrue = true;
        return res;
    }

    static OrInfo falseInfo() {
        OrInfo res;
        res.containsFalse = true;
        return res;
    }

    static OrInfo otherInfo() {
        OrInfo res;
        res.containsOther = true;
        return res;
    }

    static OrInfo merge(const OrInfo &left, const OrInfo &right) {
        OrInfo res;
        res.containsNil = left.containsNil || right.containsNil;
        res.containsFalse = left.containsFalse || right.containsFalse;
        res.containsTrue = left.containsTrue || right.containsTrue;
        res.containsOther = left.containsOther || right.containsOther;
        res.containsMultiple = left.containsMultiple || right.containsMultiple;
        return res;
    }
};

pair<OrInfo, optional<string>> showOrs(const GlobalState &, TypePtr, TypePtr);

pair<OrInfo, optional<string>> showOrElem(const GlobalState &gs, TypePtr ty) {
    if (auto classType = cast_type<ClassType>(ty.get())) {
        if (classType->symbol == Symbols::NilClass()) {
            return make_pair(OrInfo::nilInfo(), nullopt);
        } else if (classType->symbol == Symbols::TrueClass()) {
            return make_pair(OrInfo::trueInfo(), make_optional(classType->show(gs)));
        } else if (classType->symbol == Symbols::FalseClass()) {
            return make_pair(OrInfo::falseInfo(), make_optional(classType->show(gs)));
        }
    } else if (auto orType = cast_type<OrType>(ty.get())) {
        return showOrs(gs, orType->left, orType->right);
    }

    return make_pair(OrInfo::otherInfo(), make_optional(ty->show(gs)));
}

pair<OrInfo, optional<string>> showOrs(const GlobalState &gs, TypePtr left, TypePtr right) {
    auto [leftInfo, leftStr] = showOrElem(gs, left);
    auto [rightInfo, rightStr] = showOrElem(gs, right);

    OrInfo merged = OrInfo::merge(leftInfo, rightInfo);

    if (leftStr.has_value() && rightStr.has_value()) {
        merged.markContainsMultiple();
        return make_pair(merged, make_optional(fmt::format("{}, {}", *leftStr, *rightStr)));
    } else if (leftStr.has_value()) {
        return make_pair(merged, leftStr);
    } else {
        return make_pair(merged, rightStr);
    }
}

string OrType::show(const GlobalState &gs) const {
    auto [info, str] = showOrs(gs, this->left, this->right);

    // If str is empty at this point, all of the types present in the flattened
    // OrType are NilClass.
    if (!str.has_value()) {
        return Symbols::NilClass().show(gs);
    }

    string res;
    if (info.isBoolean()) {
        res = "T::Boolean";
    } else if (info.containsMultiple) {
        res = fmt::format("T.any({})", *str);
    } else {
        res = *str;
    }

    if (info.containsNil) {
        return fmt::format("T.nilable({})", res);
    } else {
        return res;
    }
}

string TypeVar::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return fmt::format("TypeVar({})", sym.data(gs)->name.showRaw(gs));
}

string TypeVar::show(const GlobalState &gs) const {
    return sym.data(gs)->name.show(gs);
}

string AppliedType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    auto thisTabs = buildTabs(tabs);
    auto nestedTabs = buildTabs(tabs + 1);
    auto twiceNestedTabs = buildTabs(tabs + 2);
    fmt::memory_buffer buf;
    fmt::format_to(buf, "AppliedType {{\n{}klass = {}\n{}targs = [\n", nestedTabs,
                   this->klass.data(gs)->toStringFullName(gs), nestedTabs);

    int i = -1;
    for (auto &targ : this->targs) {
        ++i;
        if (i < this->klass.data(gs)->typeMembers().size()) {
            auto tyMem = this->klass.data(gs)->typeMembers()[i];
            fmt::format_to(buf, "{}{} = {}\n", twiceNestedTabs, tyMem.data(gs)->name.showRaw(gs),
                           targ->toStringWithTabs(gs, tabs + 3));
        } else {
            // this happens if we try to print type before resolver has processed stdlib
            fmt::format_to(buf, "{}EARLY_TYPE_MEMBER\n", twiceNestedTabs);
        }
    }
    fmt::format_to(buf, "{}]\n{}}}", nestedTabs, thisTabs);

    return to_string(buf);
}

string AppliedType::show(const GlobalState &gs) const {
    fmt::memory_buffer buf;
    if (this->klass == Symbols::Array()) {
        fmt::format_to(buf, "T::Array");
    } else if (this->klass == Symbols::Hash()) {
        fmt::format_to(buf, "T::Hash");
    } else if (this->klass == Symbols::Enumerable()) {
        fmt::format_to(buf, "T::Enumerable");
    } else if (this->klass == Symbols::Enumerator()) {
        fmt::format_to(buf, "T::Enumerator");
    } else if (this->klass == Symbols::Range()) {
        fmt::format_to(buf, "T::Range");
    } else if (this->klass == Symbols::Set()) {
        fmt::format_to(buf, "T::Set");
    } else {
        if (std::optional<int> procArity = Types::getProcArity(*this)) {
            fmt::format_to(buf, "T.proc");

            // The first element in targs is the return type.
            // The rest are the arguments (in the correct order)
            ENFORCE(this->targs.size() == *procArity + 1, "Proc must have exactly arity + 1 targs");
            auto targs_it = this->targs.begin();
            auto return_type = *targs_it;
            targs_it++;

            if (*procArity > 0) {
                fmt::format_to(buf, ".params(");
            }

            int arg_num = 0;
            fmt::format_to(buf, "{}",
                           fmt::map_join(
                               targs_it, this->targs.end(), ", ", [&](auto targ) -> auto {
                                   return fmt::format("arg{}: {}", arg_num++, targ->show(gs));
                               }));

            if (*procArity > 0) {
                fmt::format_to(buf, ")");
            }

            fmt::format_to(buf, ".returns({})", return_type->show(gs));
            return to_string(buf);
        } else {
            fmt::format_to(buf, "{}", this->klass.data(gs)->show(gs));
        }
    }
    auto targs = this->targs;
    auto typeMembers = this->klass.data(gs)->typeMembers();
    if (typeMembers.size() < targs.size()) {
        targs.erase(targs.begin() + typeMembers.size());
    }
    auto it = targs.begin();
    for (auto typeMember : typeMembers) {
        if (typeMember.data(gs)->isFixed()) {
            it = targs.erase(it);
        } else if (this->klass == Symbols::Hash() && typeMember == typeMembers.back()) {
            it = targs.erase(it);
        } else {
            it++;
        }
    }

    if (!targs.empty()) {
        fmt::format_to(buf, "[{}]", fmt::map_join(targs, ", ", [&](auto targ) { return targ->show(gs); }));
    }
    return to_string(buf);
}

string LambdaParam::toStringWithTabs(const GlobalState &gs, int tabs) const {
    auto defName = this->definition.data(gs)->toStringFullName(gs);
    auto upperStr = this->upperBound->toString(gs);
    if (this->definition.data(gs)->isFixed()) {
        return fmt::format("LambdaParam({}, fixed={})", defName, upperStr);
    } else {
        auto lowerStr = this->lowerBound->toString(gs);
        return fmt::format("LambdaParam({}, lower={}, upper={})", defName, lowerStr, upperStr);
    }
}

string LambdaParam::show(const GlobalState &gs) const {
    return this->definition.data(gs)->show(gs);
}

string SelfTypeParam::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return fmt::format("SelfTypeParam({})", this->definition.data(gs)->toStringFullName(gs));
}

string SelfTypeParam::show(const GlobalState &gs) const {
    return this->definition.data(gs)->show(gs);
}

string SelfType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return show(gs);
}

string SelfType::show(const GlobalState &gs) const {
    return "T.self_type()";
}

string SelfType::showValue(const GlobalState &gs) const {
    return show(gs);
}

string TypeVar::typeName() const {
    return "TypeVar";
}

string UnresolvedClassType::typeName() const {
    return "UnresolvedClassType";
}

string ClassType::typeName() const {
    return "ClassType";
}

string LiteralType::typeName() const {
    return "LiteralType";
}

string TupleType::typeName() const {
    return "TupleType";
}

string ShapeType::typeName() const {
    return "ShapeType";
}

string AliasType::typeName() const {
    return "AliasType";
}

string AndType::typeName() const {
    return "AndType";
}

string OrType::typeName() const {
    return "OrType";
}

string AppliedType::typeName() const {
    return "AppliedType";
}

string LambdaParam::typeName() const {
    return "LambdaParam";
}

string SelfTypeParam::typeName() const {
    return "SelfTypeParam";
}
} // namespace sorbet::core
