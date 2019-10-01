#include "Instructions.h"

#include "common/typecase.h"
#include "core/Names.h"
#include "core/TypeConstraint.h"
#include <utility>
// helps debugging
template class std::unique_ptr<sorbet::cfg::Instruction>;

using namespace std;

namespace sorbet::cfg {

namespace {
string spacesForTabLevel(int tabs) {
    fmt::memory_buffer ss;
    for (int i = 0; i < tabs; i++) {
        fmt::format_to(ss, "&nbsp;");
    }
    return to_string(ss);
}
} // namespace

Return::Return(core::LocalVariable what) : what(what) {
    categoryCounterInc("cfg", "return");
}

string SolveConstraint::toString(core::Context ctx) {
    return fmt::format("Solve<{}>", this->link->fun.toString(ctx));
}

string SolveConstraint::showRaw(core::Context ctx, int tabs) {
    return fmt::format("Solve {{ link = {} }}", this->link->fun.showRaw(ctx));
}

string Return::toString(core::Context ctx) {
    return fmt::format("return {}", this->what.toString(ctx));
}

string Return::showRaw(core::Context ctx, int tabs) {
    return fmt::format("Return {{\n{0}&nbsp;what = {1},\n{0}}}", spacesForTabLevel(tabs),
                       this->what.showRaw(ctx, tabs + 1));
}

BlockReturn::BlockReturn(shared_ptr<core::SendAndBlockLink> link, core::LocalVariable what)
    : link(std::move(link)), what(what) {
    categoryCounterInc("cfg", "blockreturn");
}

string BlockReturn::toString(core::Context ctx) {
    return fmt::format("blockreturn<{}> {}", this->link->fun.toString(ctx), this->what.toString(ctx));
}

string BlockReturn::showRaw(core::Context ctx, int tabs) {
    return fmt::format("BlockReturn {{\n{0}&nbsp;link = {1},\n{0}&nbsp;what = {2},\n{0}}}", spacesForTabLevel(tabs),
                       this->link->fun.showRaw(ctx), this->what.showRaw(ctx, tabs + 1));
}

LoadSelf::LoadSelf(shared_ptr<core::SendAndBlockLink> link, core::LocalVariable fallback)
    : link(std::move(link)), fallback(fallback) {
    categoryCounterInc("cfg", "loadself");
}

string LoadSelf::toString(core::Context ctx) {
    return "loadSelf";
}

string LoadSelf::showRaw(core::Context ctx, int tabs) {
    return fmt::format("LoadSelf {{}}", spacesForTabLevel(tabs));
}

Send::Send(core::LocalVariable recv, core::NameRef fun, core::Loc receiverLoc,
           const InlinedVector<core::LocalVariable, 2> &args, InlinedVector<core::Loc, 2> argLocs, bool isPrivateOk,
           const shared_ptr<core::SendAndBlockLink> &link)
    : recv(recv), fun(fun), receiverLoc(receiverLoc), argLocs(std::move(argLocs)), isPrivateOk(isPrivateOk),
      link(move(link)) {
    this->args.resize(args.size());
    int i = 0;
    for (const auto &e : args) {
        this->args[i].variable = e;
        i++;
    }
    categoryCounterInc("cfg", "send");
    histogramInc("cfg.send.args", this->args.size());
}

Literal::Literal(const core::TypePtr &value) : value(move(value)) {
    categoryCounterInc("cfg", "literal");
}

string Literal::toString(core::Context ctx) {
    string res;
    typecase(
        this->value.get(), [&](core::LiteralType *l) { res = l->showValue(ctx); },
        [&](core::ClassType *l) {
            if (l->symbol == core::Symbols::NilClass()) {
                res = "nil";
            } else if (l->symbol == core::Symbols::FalseClass()) {
                res = "false";
            } else if (l->symbol == core::Symbols::TrueClass()) {
                res = "true";
            } else {
                res = fmt::format("literal({})", this->value->toStringWithTabs(ctx, 0));
            }
        },
        [&](core::Type *t) { res = fmt::format("literal({})", this->value->toStringWithTabs(ctx, 0)); });
    return res;
}

string Literal::showRaw(core::Context ctx, int tabs) {
    return fmt::format("Literal {{ value = {} }}", this->value->show(ctx));
}

Ident::Ident(core::LocalVariable what) : what(what) {
    categoryCounterInc("cfg", "ident");
}

Alias::Alias(core::SymbolRef what) : what(what) {
    categoryCounterInc("cfg", "alias");
}

string Ident::toString(core::Context ctx) {
    return this->what.toString(ctx);
}

string Ident::showRaw(core::Context ctx, int tabs) {
    return fmt::format("Ident {{\n{0}&nbsp;what = {1},\n{0}}}", spacesForTabLevel(tabs), this->what.showRaw(ctx));
}

string Alias::toString(core::Context ctx) {
    return fmt::format("alias {}", this->what.data(ctx)->name.data(ctx)->toString(ctx));
}

string Alias::showRaw(core::Context ctx, int tabs) {
    return fmt::format("Alias {{ what = {} }}", this->what.data(ctx)->show(ctx));
}

string Send::toString(core::Context ctx) {
    return fmt::format("{}.{}({})", this->recv.toString(ctx), this->fun.data(ctx)->toString(ctx),
                       fmt::map_join(this->args, ", ", [&](const auto &arg) -> string { return arg.toString(ctx); }));
}

string Send::showRaw(core::Context ctx, int tabs) {
    return fmt::format(
        "Send {{\n{0}&nbsp;recv = {1},\n{0}&nbsp;fun = {2},\n{0}&nbsp;args = ({3}),\n{0}}}", spacesForTabLevel(tabs),
        this->recv.toString(ctx), this->fun.data(ctx)->showRaw(ctx),
        fmt::map_join(this->args, ", ", [&](const auto &arg) -> string { return arg.showRaw(ctx, tabs + 1); }));
}

string LoadArg::toString(core::Context ctx) {
    return fmt::format("load_arg({})", this->argument(ctx).argumentName(ctx));
}

string LoadArg::showRaw(core::Context ctx, int tabs) {
    return fmt::format("LoadArg {{ argument = {} }}", this->argument(ctx).argumentName(ctx));
}

const core::ArgInfo &LoadArg::argument(const core::GlobalState &gs) const {
    return this->method.data(gs)->arguments()[this->argId];
}

string LoadYieldParams::toString(core::Context ctx) {
    return fmt::format("load_yield_params({})", this->link->fun.toString(ctx));
}

string LoadYieldParams::showRaw(core::Context ctx, int tabs) {
    return fmt::format("LoadYieldParams {{ link = {0} }}", this->link->fun.showRaw(ctx));
}

string Unanalyzable::toString(core::Context ctx) {
    return "<unanalyzable>";
}

string Unanalyzable::showRaw(core::Context ctx, int tabs) {
    return fmt::format("Unanalyzable {{}}", spacesForTabLevel(tabs));
}

string NotSupported::toString(core::Context ctx) {
    return fmt::format("NotSupported({})", why);
}

string NotSupported::showRaw(core::Context ctx, int tabs) {
    return fmt::format("NotSupported {{\n{0}&nbsp;why = {1},\n{0}}}", spacesForTabLevel(tabs), why);
}

string Cast::toString(core::Context ctx) {
    return fmt::format("cast({}, {});", this->value.toString(ctx), this->type->toString(ctx));
}

string Cast::showRaw(core::Context ctx, int tabs) {
    return fmt::format("Cast {{\n{0}&nbsp;cast = T.{1},\n{0}&nbsp;value = {2},\n{0}&nbsp;type = {3},\n{0}}}",
                       spacesForTabLevel(tabs), this->cast.data(ctx)->show(ctx), this->value.showRaw(ctx, tabs + 1),
                       this->type->show(ctx));
}

string TAbsurd::toString(core::Context ctx) {
    return fmt::format("T.absurd({})", this->what.toString(ctx));
}

string TAbsurd::showRaw(core::Context ctx, int tabs) {
    return fmt::format("TAbsurd {{\n{0}&nbsp;what = {1},\n{0}}}", spacesForTabLevel(tabs),
                       this->what.showRaw(ctx, tabs + 1));
}

string VariableUseSite::toString(core::Context ctx) const {
    if (this->type) {
        return fmt::format("{}: {}", this->variable.toString(ctx), this->type->show(ctx));
    }
    return this->variable.toString(ctx);
}

string VariableUseSite::showRaw(core::Context ctx, int tabs) const {
    if (this->type == nullptr) {
        return fmt::format("VariableUseSite {{ variable = {} }}", this->variable.showRaw(ctx));
    } else {
        return fmt::format("VariableUseSite {{\n{0}&nbsp;variable = {1},\n{0}&nbsp;type = {2},\n{0}}}",
                           spacesForTabLevel(tabs), this->variable.showRaw(ctx), this->type->show(ctx));
    }
}
} // namespace sorbet::cfg
