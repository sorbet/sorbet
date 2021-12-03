#include "absl/base/casts.h"
#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "common/common.h"
#include "common/formatting.h"
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
    return this->symbol.show(gs);
}

string UnresolvedClassType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return this->show(gs);
}

string UnresolvedClassType::show(const GlobalState &gs) const {
    return fmt::format("{}::{} (unresolved)", this->scope.show(gs),
                       fmt::map_join(this->names, "::", [&](const auto &el) -> string { return el.show(gs); }));
}

string UnresolvedAppliedType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return this->show(gs);
}

string UnresolvedAppliedType::show(const GlobalState &gs) const {
    return fmt::format("{}[{}] (unresolved)", this->klass.show(gs),
                       fmt::map_join(targs, ", ", [&](auto targ) { return targ.show(gs); }));
}

string LiteralType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return fmt::format("{}({})", this->underlying(gs).toStringWithTabs(gs, tabs), showValue(gs));
}

string LiteralType::show(const GlobalState &gs) const {
    return fmt::format("{}({})", this->underlying(gs).show(gs), showValue(gs));
}

string LiteralType::showValue(const GlobalState &gs) const {
    switch (literalKind) {
        case LiteralType::LiteralTypeKind::String:
            return fmt::format("\"{}\"", absl::CEscape(asName(gs).show(gs)));
        case LiteralType::LiteralTypeKind::Symbol: {
            auto shown = asName(gs).show(gs);
            if (absl::StrContains(shown, " ")) {
                return fmt::format(":\"{}\"", absl::CEscape(shown));
            } else {
                return fmt::format(":{}", shown);
            }
        }
        case LiteralType::LiteralTypeKind::Integer:
            return to_string(asInteger());
        case LiteralType::LiteralTypeKind::Float:
            return to_string(asFloat());
    }
}

string TupleType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    auto thisTabs = buildTabs(tabs);
    auto nestedTabs = buildTabs(tabs + 1);
    fmt::format_to(std::back_inserter(buf), "TupleType {{\n");
    int i = -1;
    for (auto &el : this->elems) {
        i++;
        fmt::format_to(std::back_inserter(buf), "{}{} = {}\n", nestedTabs, i, el.toStringWithTabs(gs, tabs + 3));
    }
    fmt::format_to(std::back_inserter(buf), "{}}}", thisTabs);
    return to_string(buf);
}

string TupleType::show(const GlobalState &gs) const {
    return fmt::format("[{}]", fmt::map_join(this->elems, ", ", [&](const auto &el) -> string { return el.show(gs); }));
}

string TupleType::showWithMoreInfo(const GlobalState &gs) const {
    return fmt::format("{} ({}-tuple)", show(gs), this->elems.size());
}

string ShapeType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    fmt::memory_buffer buf;
    auto thisTabs = buildTabs(tabs);
    auto nestedTabs = buildTabs(tabs + 1);
    fmt::format_to(std::back_inserter(buf), "ShapeType {{\n");
    auto valueIterator = this->values.begin();
    for (auto &el : this->keys) {
        fmt::format_to(std::back_inserter(buf), "{}{} => {}\n", nestedTabs, el.toStringWithTabs(gs, tabs + 2),
                       (*valueIterator).toStringWithTabs(gs, tabs + 3));
        ++valueIterator;
    }
    fmt::format_to(std::back_inserter(buf), "{}}}", thisTabs);
    return to_string(buf);
}

string ShapeType::show(const GlobalState &gs) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "{{");
    auto valueIterator = this->values.begin();
    bool first = true;
    for (auto &key : this->keys) {
        if (first) {
            first = false;
        } else {
            fmt::format_to(std::back_inserter(buf), ", ");
        }
        auto underlying = cast_type_nonnull<LiteralType>(key).underlying(gs);
        ClassOrModuleRef undSymbol = cast_type_nonnull<ClassType>(underlying).symbol;
        if (undSymbol == Symbols::Symbol()) {
            fmt::format_to(std::back_inserter(buf), "{}: {}", cast_type_nonnull<LiteralType>(key).asName(gs).show(gs),
                           (*valueIterator).show(gs));
        } else {
            fmt::format_to(std::back_inserter(buf), "{} => {}", key.show(gs), (*valueIterator).show(gs));
        }
        ++valueIterator;
    }
    fmt::format_to(std::back_inserter(buf), "}}");
    return to_string(buf);
}

string ShapeType::showWithMoreInfo(const GlobalState &gs) const {
    return fmt::format("{} (shape of {})", show(gs), this->underlying(gs).show(gs));
}

string AliasType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return fmt::format("AliasType {{ symbol = {} }}", this->symbol.toStringFullName(gs));
}

string AliasType::show(const GlobalState &gs) const {
    return fmt::format("<Alias: {} >", this->symbol.showFullName(gs));
}

string AndType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    bool leftBrace = isa_type<OrType>(this->left);
    bool rightBrace = isa_type<OrType>(this->right);

    return fmt::format("{}{}{} & {}{}{}", leftBrace ? "(" : "", this->left.toStringWithTabs(gs, tabs + 2),
                       leftBrace ? ")" : "", rightBrace ? "(" : "", this->right.toStringWithTabs(gs, tabs + 2),
                       rightBrace ? ")" : "");
}

string showAnds(const GlobalState &, const TypePtr &, const TypePtr &);

string showAndElem(const GlobalState &gs, const TypePtr &ty) {
    if (auto andType = cast_type<AndType>(ty)) {
        return showAnds(gs, andType->left, andType->right);
    }
    return ty.show(gs);
}

string showAnds(const GlobalState &gs, const TypePtr &left, const TypePtr &right) {
    auto leftStr = showAndElem(gs, left);
    auto rightStr = showAndElem(gs, right);
    return fmt::format("{}, {}", leftStr, rightStr);
}

string AndType::show(const GlobalState &gs) const {
    auto str = showAnds(gs, this->left, this->right);
    return fmt::format("T.all({})", str);
}

string OrType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    bool leftBrace = isa_type<AndType>(this->left);
    bool rightBrace = isa_type<AndType>(this->right);

    return fmt::format("{}{}{} | {}{}{}", leftBrace ? "(" : "", this->left.toStringWithTabs(gs, tabs + 2),
                       leftBrace ? ")" : "", rightBrace ? "(" : "", this->right.toStringWithTabs(gs, tabs + 2),
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

pair<OrInfo, optional<string>> showOrs(const GlobalState &, const TypePtr &, const TypePtr &);

pair<OrInfo, optional<string>> showOrElem(const GlobalState &gs, const TypePtr &ty) {
    if (isa_type<ClassType>(ty)) {
        auto classType = cast_type_nonnull<ClassType>(ty);
        if (classType.symbol == Symbols::NilClass()) {
            return make_pair(OrInfo::nilInfo(), nullopt);
        } else if (classType.symbol == Symbols::TrueClass()) {
            return make_pair(OrInfo::trueInfo(), make_optional(classType.show(gs)));
        } else if (classType.symbol == Symbols::FalseClass()) {
            return make_pair(OrInfo::falseInfo(), make_optional(classType.show(gs)));
        }
    } else if (auto orType = cast_type<OrType>(ty)) {
        return showOrs(gs, orType->left, orType->right);
    }

    return make_pair(OrInfo::otherInfo(), make_optional(ty.show(gs)));
}

pair<OrInfo, optional<string>> showOrs(const GlobalState &gs, const TypePtr &left, const TypePtr &right) {
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
    auto shown = sym.data(gs)->name.show(gs);
    if (absl::StrContains(shown, " ")) {
        return fmt::format("T.type_parameter(:{})", absl::CEscape(shown));
    } else {
        return fmt::format("T.type_parameter(:{})", shown);
    }
}

string AppliedType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    auto thisTabs = buildTabs(tabs);
    auto nestedTabs = buildTabs(tabs + 1);
    auto twiceNestedTabs = buildTabs(tabs + 2);
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "AppliedType {{\n{}klass = {}\n{}targs = [\n", nestedTabs,
                   this->klass.toStringFullName(gs), nestedTabs);

    int i = -1;
    for (auto &targ : this->targs) {
        ++i;
        if (i < this->klass.data(gs)->typeMembers().size()) {
            auto tyMem = this->klass.data(gs)->typeMembers()[i];
            fmt::format_to(std::back_inserter(buf), "{}{} = {}\n", twiceNestedTabs, tyMem.name(gs).showRaw(gs),
                           targ.toStringWithTabs(gs, tabs + 3));
        } else {
            // this happens if we try to print type before resolver has processed stdlib
            fmt::format_to(std::back_inserter(buf), "{}EARLY_TYPE_MEMBER\n", twiceNestedTabs);
        }
    }
    fmt::format_to(std::back_inserter(buf), "{}]\n{}}}", nestedTabs, thisTabs);

    return to_string(buf);
}

string AppliedType::show(const GlobalState &gs) const {
    fmt::memory_buffer buf;
    if (this->klass == Symbols::Array()) {
        fmt::format_to(std::back_inserter(buf), "T::Array");
    } else if (this->klass == Symbols::Hash()) {
        fmt::format_to(std::back_inserter(buf), "T::Hash");
    } else if (this->klass == Symbols::Enumerable()) {
        fmt::format_to(std::back_inserter(buf), "T::Enumerable");
    } else if (this->klass == Symbols::Enumerator()) {
        fmt::format_to(std::back_inserter(buf), "T::Enumerator");
    } else if (this->klass == Symbols::Range()) {
        fmt::format_to(std::back_inserter(buf), "T::Range");
    } else if (this->klass == Symbols::Set()) {
        fmt::format_to(std::back_inserter(buf), "T::Set");
    } else {
        if (std::optional<int> procArity = Types::getProcArity(*this)) {
            fmt::format_to(std::back_inserter(buf), "T.proc");

            // The first element in targs is the return type.
            // The rest are the arguments (in the correct order)
            ENFORCE(this->targs.size() == *procArity + 1, "Proc must have exactly arity + 1 targs");
            auto targs_it = this->targs.begin();
            auto return_type = *targs_it;
            targs_it++;

            if (*procArity > 0) {
                fmt::format_to(std::back_inserter(buf), ".params(");
            }

            int arg_num = 0;
            fmt::format_to(std::back_inserter(buf), "{}",
                           fmt::map_join(
                               targs_it, this->targs.end(), ", ", [&](auto targ) -> auto {
                                   return fmt::format("arg{}: {}", arg_num++, targ.show(gs));
                               }));

            if (*procArity > 0) {
                fmt::format_to(std::back_inserter(buf), ")");
            }

            if (return_type == core::Types::void_()) {
                fmt::format_to(std::back_inserter(buf), ".void");
            } else {
                fmt::format_to(std::back_inserter(buf), ".returns({})", return_type.show(gs));
            }
            return to_string(buf);
        } else {
            fmt::format_to(std::back_inserter(buf), "{}", this->klass.show(gs));
        }
    }
    auto targs = this->targs;
    auto typeMembers = this->klass.data(gs)->typeMembers();
    if (typeMembers.size() < targs.size()) {
        targs.erase(targs.begin() + typeMembers.size());
    }
    auto it = targs.begin();
    for (auto typeMember : typeMembers) {
        auto tm = typeMember.asTypeMemberRef();
        if (tm.data(gs)->isFixed()) {
            it = targs.erase(it);
        } else if (typeMember.name(gs) == core::Names::Constants::AttachedClass()) {
            it = targs.erase(it);
        } else if (this->klass == Symbols::Hash() && typeMember == typeMembers.back()) {
            it = targs.erase(it);
        } else {
            it++;
        }
    }

    if (!targs.empty()) {
        fmt::format_to(std::back_inserter(buf), "[{}]",
                       fmt::map_join(targs, ", ", [&](auto targ) { return targ.show(gs); }));
    }
    return to_string(buf);
}

string LambdaParam::toStringWithTabs(const GlobalState &gs, int tabs) const {
    auto defName = this->definition.toStringFullName(gs);
    auto upperStr = this->upperBound.toString(gs);
    if (this->definition.data(gs)->isFixed()) {
        return fmt::format("LambdaParam({}, fixed={})", defName, upperStr);
    } else {
        auto lowerStr = this->lowerBound.toString(gs);
        return fmt::format("LambdaParam({}, lower={}, upper={})", defName, lowerStr, upperStr);
    }
}

string LambdaParam::show(const GlobalState &gs) const {
    return this->definition.show(gs);
}

string SelfTypeParam::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return fmt::format("SelfTypeParam({})", this->definition.toStringFullName(gs));
}

string SelfTypeParam::show(const GlobalState &gs) const {
    return this->definition.show(gs);
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
} // namespace sorbet::core
