#include "Instructions.h"

#include "common/formatting.h"
#include "common/typecase.h"
#include "core/Names.h"
#include "core/TypeConstraint.h"
#include <utility>

using namespace std;

namespace sorbet::cfg {

namespace {
string spacesForTabLevel(int tabs) {
    fmt::memory_buffer ss;
    for (int i = 0; i < tabs; i++) {
        fmt::format_to(std::back_inserter(ss), "&nbsp;");
    }
    return to_string(ss);
}
} // namespace

#define CASE_STATEMENT(CASE_BODY, T) \
    case Tag::T: {                   \
        CASE_BODY(T)                 \
        break;                       \
    }

#define GENERATE_TAG_SWITCH(tag, body)            \
    switch (tag) {                                \
        CASE_STATEMENT(body, Ident)               \
        CASE_STATEMENT(body, Alias)               \
        CASE_STATEMENT(body, SolveConstraint)     \
        CASE_STATEMENT(body, Send)                \
        CASE_STATEMENT(body, Return)              \
        CASE_STATEMENT(body, BlockReturn)         \
        CASE_STATEMENT(body, LoadSelf)            \
        CASE_STATEMENT(body, Literal)             \
        CASE_STATEMENT(body, GetCurrentException) \
        CASE_STATEMENT(body, LoadArg)             \
        CASE_STATEMENT(body, ArgPresent)          \
        CASE_STATEMENT(body, LoadYieldParams)     \
        CASE_STATEMENT(body, YieldParamPresent)   \
        CASE_STATEMENT(body, YieldLoadArg)        \
        CASE_STATEMENT(body, Cast)                \
        CASE_STATEMENT(body, TAbsurd)             \
    }

std::string InstructionPtr::toString(const core::GlobalState &gs, const CFG &cfg) const {
    auto *ptr = get();

#define TO_STRING(name) return static_cast<const name *>(ptr)->toString(gs, cfg);
    GENERATE_TAG_SWITCH(tag(), TO_STRING)
#undef TO_STRING
}

std::string InstructionPtr::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    auto *ptr = get();

#define SHOW_RAW(name) return static_cast<const name *>(ptr)->showRaw(gs, cfg, tabs);
    GENERATE_TAG_SWITCH(tag(), SHOW_RAW)
#undef SHOW_RAW
}

void InstructionPtr::deleteTagged(Tag tag, void *expr) noexcept {
    ENFORCE(expr != nullptr);

#define DELETE_INSN(name) delete static_cast<name *>(expr);
    GENERATE_TAG_SWITCH(tag, DELETE_INSN)
#undef DELETE_INSN
}

Return::Return(LocalRef what, core::LocOffsets whatLoc) : what(what), whatLoc(whatLoc) {
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
    : fallback(fallback), link(std::move(link)) {
    categoryCounterInc("cfg", "loadself");
}

string LoadSelf::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("loadSelf({})", this->link->fun.toString(gs));
}

string LoadSelf::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("LoadSelf {{ link = {} }}", this->link->fun.showRaw(gs));
}

Send::Send(LocalRef recv, core::LocOffsets receiverLoc, core::NameRef fun, core::LocOffsets funLoc, uint16_t numPosArgs,
           const InlinedVector<LocalRef, 2> &args, InlinedVector<core::LocOffsets, 2> argLocs, bool isPrivateOk,
           const shared_ptr<core::SendAndBlockLink> &link)
    : isPrivateOk(isPrivateOk), numPosArgs(numPosArgs), fun(fun), recv(recv), funLoc(funLoc), receiverLoc(receiverLoc),
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

core::LocOffsets Send::locWithoutBlock(core::LocOffsets bindLoc) {
    if (this->link == nullptr) {
        // This location is slightly better, because it will include the last `)` if that exists,
        // which means that queries for things like
        //
        //     foo()
        //     #   ^ completion: ...
        //
        // will match.
        return bindLoc;
    }

    if (!this->argLocs.empty()) {
        return this->receiverLoc.join(this->argLocs.back());
    }

    return this->receiverLoc.join(this->funLoc);
}

Literal::Literal(const core::TypePtr &value) : value(move(value)) {
    categoryCounterInc("cfg", "literal");
}

string Literal::toString(const core::GlobalState &gs, const CFG &cfg) const {
    string res;
    typecase(
        this->value, [&](const core::LiteralType &l) { res = l.showValue(gs); },
        [&](const core::LiteralIntegerType &i) { res = i.showValue(gs); },
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
        return fmt::format("alias {} ({})", this->what.name(gs).toString(gs), name.toString(gs));
    } else {
        return fmt::format("alias {}", this->what.name(gs).toString(gs));
    }
}

string Alias::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Alias {{ what = {} }}", this->what.show(gs));
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
    return this->method.data(gs)->arguments[this->argId];
}

string ArgPresent::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("arg_present({})", this->argument(gs).argumentName(gs));
}

string ArgPresent::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("ArgPresent {{ argument = {} }}", this->argument(gs).argumentName(gs));
}

const core::ArgInfo &ArgPresent::argument(const core::GlobalState &gs) const {
    return this->method.data(gs)->arguments[this->argId];
}

string LoadYieldParams::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("load_yield_params({})", this->link->fun.toString(gs));
}

string LoadYieldParams::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("LoadYieldParams {{ link = {0} }}", this->link->fun.showRaw(gs));
}

string YieldParamPresent::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("yield_param_present({})", this->argId);
}

string YieldParamPresent::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("YieldParamPresent {{ argId = {} }}", this->argId);
}

string YieldLoadArg::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("yield_load_arg({}, {})", this->argId, this->yieldParam.toString(gs, cfg));
}

string YieldLoadArg::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("YieldLoadArg {{ argId = {}, yieldParam = {} }}", this->argId,
                       this->yieldParam.showRaw(gs, cfg));
}

string GetCurrentException::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return "<get-current-exception>";
}

string GetCurrentException::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("GetCurrentException {{}}", spacesForTabLevel(tabs));
}

string Cast::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return fmt::format("cast({}, {});", this->value.toString(gs, cfg), this->type.show(gs));
}

string Cast::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    return fmt::format("Cast {{\n{0}&nbsp;cast = T.{1},\n{0}&nbsp;value = {2},\n{0}&nbsp;type = {3},\n{0}}}",
                       spacesForTabLevel(tabs), this->cast.show(gs), this->value.showRaw(gs, cfg, tabs + 1),
                       this->type.show(gs));
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
        return fmt::format("{}: {}", this->variable.toString(gs, cfg), this->type.show(gs));
    }
}

string VariableUseSite::showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs) const {
    if (this->variable == LocalRef::unconditional() || this->type == nullptr) {
        return fmt::format("VariableUseSite {{ variable = {} }}", this->variable.showRaw(gs, cfg));
    } else {
        return fmt::format("VariableUseSite {{\n{0}&nbsp;variable = {1},\n{0}&nbsp;type = {2},\n{0}}}",
                           spacesForTabLevel(tabs), this->variable.showRaw(gs, cfg), this->type.show(gs));
    }
}

} // namespace sorbet::cfg
