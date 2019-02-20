#include "Instructions.h"

#include "core/Names.h"
#include "core/TypeConstraint.h"
#include <utility>
// helps debugging
template class std::unique_ptr<sorbet::cfg::Instruction>;

using namespace std;

namespace sorbet::cfg {

Return::Return(core::LocalVariable what) : what(what) {
    categoryCounterInc("cfg", "return");
}

string SolveConstraint::toString(core::Context ctx) {
    return fmt::format("Solve<{}>", this->link->block.data(ctx)->showFullName(ctx));
}

string Return::toString(core::Context ctx) {
    return fmt::format("return {}", this->what.toString(ctx));
}

BlockReturn::BlockReturn(const shared_ptr<core::SendAndBlockLink> &link, core::LocalVariable what)
    : link(move(link)), what(what) {
    categoryCounterInc("cfg", "blockreturn");
}

string BlockReturn::toString(core::Context ctx) {
    return fmt::format("blockreturn<{}> {}", this->link->block.data(ctx)->showFullName(ctx), this->what.toString(ctx));
}

Send::Send(core::LocalVariable recv, core::NameRef fun, core::Loc receiverLoc,
           const InlinedVector<core::LocalVariable, 2> &args, const InlinedVector<core::Loc, 2> &argLocs,
           const shared_ptr<core::SendAndBlockLink> &link)
    : recv(recv), fun(fun), receiverLoc(receiverLoc), argLocs(argLocs), link(move(link)) {
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
    typecase(this->value.get(), [&](core::LiteralType *l) { res = l->showValue(ctx); },
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

Ident::Ident(core::LocalVariable what) : what(what) {
    categoryCounterInc("cfg", "ident");
}

Alias::Alias(core::SymbolRef what) : what(what) {
    categoryCounterInc("cfg", "alias");
}

string Ident::toString(core::Context ctx) {
    return this->what.toString(ctx);
}

string Alias::toString(core::Context ctx) {
    return fmt::format("alias {}", this->what.data(ctx)->name.data(ctx)->toString(ctx));
}

string Send::toString(core::Context ctx) {
    return fmt::format("{}.{}({})", this->recv.toString(ctx), this->fun.data(ctx)->toString(ctx),
                       fmt::map_join(this->args, ", ", [&](const auto &arg) -> string { return arg.toString(ctx); }));
}

string Self::toString(core::Context ctx) {
    return "self";
}

string LoadArg::toString(core::Context ctx) {
    return fmt::format("load_arg({})", this->arg.data(ctx)->show(ctx));
}

string LoadYieldParams::toString(core::Context ctx) {
    return fmt::format("load_yield_params({}, {})", this->link->block.data(ctx)->showFullName(ctx),
                       this->block.show(ctx));
}

string Unanalyzable::toString(core::Context ctx) {
    return "<unanalyzable>";
}

string NotSupported::toString(core::Context ctx) {
    return fmt::format("NotSupported({})", why);
}

string Cast::toString(core::Context ctx) {
    return fmt::format("cast({}, {});", this->value.toString(ctx), this->type->toString(ctx));
}

string DebugEnvironment::toString(core::Context ctx) {
    return str;
}

DebugEnvironment::DebugEnvironment(core::GlobalState::AnnotationPos pos) : pos(pos) {
    isSynthetic = true;
}

string VariableUseSite::toString(core::Context ctx) const {
    if (this->type) {
        return fmt::format("{}: {}", this->variable.toString(ctx), this->type->show(ctx));
    }
    return this->variable.toString(ctx);
}
} // namespace sorbet::cfg
