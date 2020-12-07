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
string varUseSiteToString(const core::GlobalState &gs, const CFG &cfg,
                          LocalRef variable, const core::TypePtr &type) {
    if (variable == LocalRef::unconditional() || type == nullptr) {
        return variable.toString(gs, cfg);
    } else {
        return fmt::format("{}: {}", variable.toString(gs, cfg), type.show(gs));
    }
}

string varUseSiteShowRaw(const core::GlobalState &gs, const CFG &cfg, int tabs,
                         LocalRef variable, const core::TypePtr &type) {
    if (variable == LocalRef::unconditional() || type == nullptr) {
        return fmt::format("VariableUseSite {{ variable = {} }}", variable.showRaw(gs, cfg));
    } else {
        return fmt::format("VariableUseSite {{\n{0}&nbsp;variable = {1},\n{0}&nbsp;type = {2},\n{0}}}",
                           spacesForTabLevel(tabs), variable.showRaw(gs, cfg), type.show(gs));
    }
}

} // namespace

Return::Return(LocalRef what) : variable(what) {
    categoryCounterInc("cfg", "return");
}

string SolveConstraint::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("Solve<{}, {}>", this->send.toString(gs, cfg), this->link->fun.toString(gs));
}

string SolveConstraint::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Solve {{ send = {}, link = {} }}", this->send.toString(gs, cfg), this->link->fun.showRaw(gs));
}

string Return::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("return {}", varUseSiteToString(gs, cfg, variable, type));
}

string Return::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Return {{\n{0}&nbsp;what = {1},\n{0}}}", spacesForTabLevel(tabs),
                       varUseSiteShowRaw(gs, cfg, tabs + 1, variable, type));
}

BlockReturn::BlockReturn(shared_ptr<core::SendAndBlockLink> link, LocalRef what) : variable(what), link(std::move(link)) {
    categoryCounterInc("cfg", "blockreturn");
}

string BlockReturn::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("blockreturn<{}> {}", this->link->fun.toString(gs),
                       varUseSiteToString(gs, cfg, this->variable, this->type));
}

string BlockReturn::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("BlockReturn {{\n{0}&nbsp;link = {1},\n{0}&nbsp;what = {2},\n{0}}}", spacesForTabLevel(tabs),
                       this->link->fun.showRaw(gs),
                       varUseSiteShowRaw(gs, cfg, tabs + 1, this->variable, this->type));
}

LoadSelf::LoadSelf(shared_ptr<core::SendAndBlockLink> link, LocalRef fallback)
    : fallback(fallback), link(std::move(link)) {
    categoryCounterInc("cfg", "loadself");
}

string LoadSelf::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return "loadSelf";
}

string LoadSelf::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("LoadSelf {{}}", spacesForTabLevel(tabs));
}

Send::Send(LocalRef recv, core::NameRef fun, core::LocOffsets receiverLoc, u2 numPosArgs,
           const InlinedVector<LocalRef, 2> &args, InlinedVector<core::LocOffsets, 2> argLocs, bool isPrivateOk,
           const shared_ptr<core::SendAndBlockLink> &link)
    : isPrivateOk(isPrivateOk), numPosArgs(numPosArgs), fun(fun), recv(recv), receiverLoc(receiverLoc),
      argLocs(std::move(argLocs)), link(move(link)) {
    ENFORCE(numPosArgs <= args.size(), "Expected {} positional arguments, but only have {} args", numPosArgs,
            args.size());

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
        this->value, [&](const core::LiteralType &l) { res = l.showValue(gs); },
        [&](const core::ClassType &l) {
            if (l.symbol == core::Symbols::NilClass()) {
                res = "nil";
            } else if (l.symbol == core::Symbols::FalseClass()) {
                res = "false";
            } else if (l.symbol == core::Symbols::TrueClass()) {
                res = "true";
            } else {
                res = fmt::format("literal({})", this->value.toStringWithTabs(gs, 0));
            }
        },
        [&](const core::TypePtr &t) { res = fmt::format("literal({})", this->value.toStringWithTabs(gs, 0)); });
    return res;
}

string Literal::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Literal {{ value = {} }}", this->value.show(gs));
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
        return fmt::format("alias {} ({})", this->what.data(gs)->name.toString(gs), name.toString(gs));
    } else {
        return fmt::format("alias {}", this->what.data(gs)->name.toString(gs));
    }
}

string Alias::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Alias {{ what = {} }}", this->what.data(gs)->show(gs));
}

string Send::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format(
        "{}.{}({})", this->recv.toString(gs, cfg), this->fun.toString(gs),
        fmt::map_join(this->args, ", ", [&](const auto &arg) -> string { return arg.toString(gs, cfg); }));
}

string Send::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format(
        "Send {{\n{0}&nbsp;recv = {1},\n{0}&nbsp;fun = {2},\n{0}&nbsp;args = ({3}),\n{0}}}", spacesForTabLevel(tabs),
        this->recv.toString(gs, cfg), this->fun.showRaw(gs),
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
    return fmt::format("cast({}, {});", varUseSiteToString(gs, cfg, this->variable, this->resultType), this->type.toString(gs));
}

string Cast::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Cast {{\n{0}&nbsp;cast = T.{1},\n{0}&nbsp;value = {2},\n{0}&nbsp;type = {3},\n{0}}}",
                       spacesForTabLevel(tabs), this->cast.show(gs),
                       varUseSiteShowRaw(gs, cfg, tabs + 1, this->variable, this->resultType),
                       this->type.show(gs));
}

string TAbsurd::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("T.absurd({})", varUseSiteToString(gs, cfg, this->variable, this->type));
}

string TAbsurd::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("TAbsurd {{\n{0}&nbsp;what = {1},\n{0}}}", spacesForTabLevel(tabs),
                       varUseSiteShowRaw(gs, cfg, tabs + 1, this->variable, this->type));
}

string VariableUseSite::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return varUseSiteToString(gs, cfg, this->variable, this->type);
}

string VariableUseSite::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return varUseSiteShowRaw(gs, cfg, tabs, this->variable, this->type);
}

} // namespace sorbet::cfg
