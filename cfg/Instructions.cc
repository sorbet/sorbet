#include "Instructions.h"

#include "common/formatting.h"
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

Return::Return(LocalRef what) : what(what) {
    categoryCounterInc("cfg", "return");
}

string SolveConstraint::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("Solve<{}, {}>", this->send.toString(gs, cfg), this->link->fun.toString(gs));
}

string SolveConstraint::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Solve {{ send = {}, link = {} }}", this->send.toString(gs, cfg), this->link->fun.showRaw(gs));
}

string Return::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("return {}", this->what.toString(gs, cfg));
}

string Return::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Return {{\n{0}&nbsp;what = {1},\n{0}}}", spacesForTabLevel(tabs),
                       this->what.showRaw(gs, cfg, tabs + 1));
}

BlockReturn::BlockReturn(shared_ptr<core::SendAndBlockLink> link, LocalRef what) : link(std::move(link)), what(what) {
    categoryCounterInc("cfg", "blockreturn");
}

string BlockReturn::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("blockreturn<{}> {}", this->link->fun.toString(gs), this->what.toString(gs, cfg));
}

string BlockReturn::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("BlockReturn {{\n{0}&nbsp;link = {1},\n{0}&nbsp;what = {2},\n{0}}}", spacesForTabLevel(tabs),
                       this->link->fun.showRaw(gs), this->what.showRaw(gs, cfg, tabs + 1));
}

LoadSelf::LoadSelf(shared_ptr<core::SendAndBlockLink> link, LocalRef fallback)
    : link(std::move(link)), fallback(fallback) {
    categoryCounterInc("cfg", "loadself");
}

string LoadSelf::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return "loadSelf";
}

string LoadSelf::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("LoadSelf {{}}", spacesForTabLevel(tabs));
}

Send::Send(LocalRef recv, core::NameRef fun, core::LocOffsets receiverLoc, u1 numPosArgs,
           const InlinedVector<LocalRef, 2> &args, InlinedVector<core::LocOffsets, 2> argLocs, bool isPrivateOk,
           const shared_ptr<core::SendAndBlockLink> &link)
    : recv(recv), fun(fun), receiverLoc(receiverLoc), numPosArgs{numPosArgs}, argLocs(std::move(argLocs)),
      isPrivateOk(isPrivateOk), link(move(link)) {
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

string Literal::toString(const core::GlobalState &gs, const CFG &cfg) const {
    string res;
    typecase(
        this->value.get(), [&](core::LiteralType *l) { res = l->showValue(gs); },
        [&](core::ClassType *l) {
            if (l->symbol == core::Symbols::NilClass()) {
                res = "nil";
            } else if (l->symbol == core::Symbols::FalseClass()) {
                res = "false";
            } else if (l->symbol == core::Symbols::TrueClass()) {
                res = "true";
            } else {
                res = fmt::format("literal({})", this->value->toStringWithTabs(gs, 0));
            }
        },
        [&](core::Type *t) { res = fmt::format("literal({})", this->value->toStringWithTabs(gs, 0)); });
    return res;
}

string Literal::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Literal {{ value = {} }}", this->value->show(gs));
}

Ident::Ident(LocalRef what) : what(what) {
    categoryCounterInc("cfg", "ident");
}

Alias::Alias(core::SymbolRef what, core::NameRef name) : what(what), name(name) {
    // what == undeclaredFieldStub -> name.exists()
    ENFORCE(what != core::Symbols::Magic_undeclaredFieldStub() || name.exists(),
            "Missing name for undeclared field alias!");

    // name.exists() -> what == undeclaredFieldStub()
    ENFORCE(!name.exists() || what == core::Symbols::Magic_undeclaredFieldStub(), "Name provided for known alias!");
    categoryCounterInc("cfg", "alias");
}

string Ident::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return this->what.toString(gs, cfg);
}

string Ident::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Ident {{\n{0}&nbsp;what = {1},\n{0}}}", spacesForTabLevel(tabs), this->what.showRaw(gs, cfg));
}

string Alias::toString(const core::GlobalState &gs, const CFG &cfg) const {
    if (name.exists()) {
        return fmt::format("alias {} ({})", this->what.data(gs)->name.data(gs)->toString(gs), name.toString(gs));
    } else {
        return fmt::format("alias {}", this->what.data(gs)->name.data(gs)->toString(gs));
    }
}

string Alias::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Alias {{ what = {} }}", this->what.data(gs)->show(gs));
}

string Send::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format(
        "{}.{}({})", this->recv.toString(gs, cfg), this->fun.data(gs)->toString(gs),
        fmt::map_join(this->args, ", ", [&](const auto &arg) -> string { return arg.toString(gs, cfg); }));
}

string Send::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format(
        "Send {{\n{0}&nbsp;recv = {1},\n{0}&nbsp;fun = {2},\n{0}&nbsp;args = ({3}),\n{0}}}", spacesForTabLevel(tabs),
        this->recv.toString(gs, cfg), this->fun.data(gs)->showRaw(gs),
        fmt::map_join(this->args, ", ", [&](const auto &arg) -> string { return arg.showRaw(gs, cfg, tabs + 1); }));
}

string LoadArg::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("load_arg({})", this->argument(gs).argumentName(gs));
}

string LoadArg::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("LoadArg {{ argument = {} }}", this->argument(gs).argumentName(gs));
}

const core::ArgInfo &LoadArg::argument(const core::GlobalState &gs) const {
    return this->method.data(gs)->arguments()[this->argId];
}

string ArgPresent::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("arg_present({})", this->argument(gs).argumentName(gs));
}

string ArgPresent::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("ArgPresent {{ argument = {} }}", this->argument(gs).argumentName(gs));
}

const core::ArgInfo &ArgPresent::argument(const core::GlobalState &gs) const {
    return this->method.data(gs)->arguments()[this->argId];
}

string LoadYieldParams::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("load_yield_params({})", this->link->fun.toString(gs));
}

string LoadYieldParams::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("LoadYieldParams {{ link = {0} }}", this->link->fun.showRaw(gs));
}

string GetCurrentException::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return "<get-current-exception>";
}

string GetCurrentException::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("GetCurrentException {{}}", spacesForTabLevel(tabs));
}

string Cast::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("cast({}, {});", this->value.toString(gs, cfg), this->type->toString(gs));
}

string Cast::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Cast {{\n{0}&nbsp;cast = T.{1},\n{0}&nbsp;value = {2},\n{0}&nbsp;type = {3},\n{0}}}",
                       spacesForTabLevel(tabs), this->cast.data(gs)->show(gs), this->value.showRaw(gs, cfg, tabs + 1),
                       this->type->show(gs));
}

string TAbsurd::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("T.absurd({})", this->what.toString(gs, cfg));
}

string TAbsurd::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("TAbsurd {{\n{0}&nbsp;what = {1},\n{0}}}", spacesForTabLevel(tabs),
                       this->what.showRaw(gs, cfg, tabs + 1));
}

string VariableUseSite::toString(const core::GlobalState &gs, const CFG &cfg) const {
    if (this->variable == LocalRef::unconditional() || this->type == nullptr) {
        return this->variable.toString(gs, cfg);
    } else {
        return fmt::format("{}: {}", this->variable.toString(gs, cfg), this->type->show(gs));
    }
}

string VariableUseSite::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    if (this->variable == LocalRef::unconditional() || this->type == nullptr) {
        return fmt::format("VariableUseSite {{ variable = {} }}", this->variable.showRaw(gs, cfg));
    } else {
        return fmt::format("VariableUseSite {{\n{0}&nbsp;variable = {1},\n{0}&nbsp;type = {2},\n{0}}}",
                           spacesForTabLevel(tabs), this->variable.showRaw(gs, cfg), this->type->show(gs));
    }
}

} // namespace sorbet::cfg
