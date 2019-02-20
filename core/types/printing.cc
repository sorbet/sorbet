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
    return this->symbol.data(gs)->show(gs);
}

string ClassType::show(const GlobalState &gs) const {
    return this->symbol.data(gs)->show(gs);
}

string ClassType::typeName() const {
    return "ClassType";
}

string LiteralType::typeName() const {
    return "LiteralType";
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
            fmt::format_to(buf, "{}: {}", NameRef(gs, cast_type<LiteralType>(key.get())->value).toString(gs),
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
    return fmt::format("AliasType {{ symbol = {} }}", this->symbol.data(gs)->showFullName(gs));
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

string showOrs(const GlobalState &gs, TypePtr left, TypePtr right) {
    auto *lt = cast_type<OrType>(left.get());
    auto *rt = cast_type<OrType>(right.get());
    return fmt::format("{}, {}", lt != nullptr ? showOrs(gs, lt->left, lt->right) : left->show(gs),
                       rt != nullptr ? showOrs(gs, rt->left, rt->right) : right->show(gs));
}

string showOrSpecialCase(const GlobalState &gs, TypePtr type, TypePtr rest) {
    auto *ct = cast_type<ClassType>(type.get());
    if (ct != nullptr && ct->symbol == Symbols::NilClass()) {
        return fmt::format("T.nilable({})", rest->show(gs));
    }
    return "";
}

string OrType::show(const GlobalState &gs) const {
    string ret;
    if (!(ret = showOrSpecialCase(gs, this->left, this->right)).empty()) {
        return ret;
    }
    if (!(ret = showOrSpecialCase(gs, this->right, this->left)).empty()) {
        return ret;
    }

    return fmt::format("T.any({})", showOrs(gs, this->left, this->right));
}

string TypeVar::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return fmt::format("TypeVar({})", sym.data(gs)->name.toString(gs));
}

string TypeVar::show(const GlobalState &gs) const {
    return sym.data(gs)->name.toString(gs);
}

string TypeVar::typeName() const {
    return "TypeVar";
}

string AppliedType::toStringWithTabs(const GlobalState &gs, int tabs) const {
    auto thisTabs = buildTabs(tabs);
    auto nestedTabs = buildTabs(tabs + 1);
    auto twiceNestedTabs = buildTabs(tabs + 2);
    fmt::memory_buffer buf;
    fmt::format_to(buf, "AppliedType {{\n{}klass = {}\n{}targs = [\n", nestedTabs,
                   this->klass.data(gs)->showFullName(gs), nestedTabs);

    int i = -1;
    for (auto &targ : this->targs) {
        ++i;
        if (i < this->klass.data(gs)->typeMembers().size()) {
            auto tyMem = this->klass.data(gs)->typeMembers()[i];
            fmt::format_to(buf, "{}{} = {}\n", twiceNestedTabs, tyMem.data(gs)->name.toString(gs),
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
    } else if (this->klass == Symbols::Range()) {
        fmt::format_to(buf, "T::Range");
    } else if (this->klass == Symbols::Set()) {
        fmt::format_to(buf, "T::Set");
    } else {
        bool isProc = false;
        int proc_arity = 0;
        for (int i = 0; i <= Symbols::MAX_PROC_ARITY; i++) {
            if (this->klass == Symbols::Proc(i)) {
                isProc = true;
                proc_arity = i;
                break;
            }
        }

        if (isProc) {
            fmt::format_to(buf, "T.proc");

            // The first element in targs is the return type.
            // The rest are the arguments (in the correct order)
            ENFORCE(this->targs.size() == proc_arity + 1, "Proc must have exactly arity + 1 targs");
            auto targs_it = this->targs.begin();
            auto return_type = *targs_it;
            targs_it++;

            if (proc_arity > 0) {
                fmt::format_to(buf, ".params(");
            }

            int arg_num = 0;
            fmt::format_to(buf, "{}", fmt::map_join(targs_it, this->targs.end(), ", ", [&](auto targ) -> auto {
                               return fmt::format("arg{}: {}", arg_num++, targ->show(gs));
                           }));

            if (proc_arity > 0) {
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

string AppliedType::typeName() const {
    return "AppliedType";
}

string LambdaParam::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return fmt::format("LambdaParam({})", this->definition.data(gs)->showFullName(gs));
}

string LambdaParam::show(const GlobalState &gs) const {
    return this->definition.data(gs)->show(gs);
}

string SelfTypeParam::toStringWithTabs(const GlobalState &gs, int tabs) const {
    return fmt::format("SelfTypeParam({})", this->definition.data(gs)->showFullName(gs));
}

string SelfTypeParam::show(const GlobalState &gs) const {
    return this->definition.data(gs)->show(gs);
}

string LambdaParam::typeName() const {
    return "LambdaParam";
}

string SelfTypeParam::typeName() const {
    return "SelfTypeParam";
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
