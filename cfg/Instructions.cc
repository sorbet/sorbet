#include "Instructions.h"

#include "core/Names/cfg.h"
#include "core/TypeConstraint.h"
#include <utility>
// helps debugging
template class std::unique_ptr<sorbet::cfg::Instruction>;

using namespace std;

namespace sorbet {
namespace cfg {

Return::Return(core::LocalVariable what) : what(what) {
    core::categoryCounterInc("cfg", "return");
}

string SolveConstraint::toString(core::Context ctx) {
    return "Solve<" + this->link->block.data(ctx).fullName(ctx) + ">";
}

string Return::toString(core::Context ctx) {
    return "return " + this->what.toString(ctx);
}

BlockReturn::BlockReturn(std::shared_ptr<core::SendAndBlockLink> link, core::LocalVariable what)
    : link(std::move(link)), what(what) {
    core::categoryCounterInc("cfg", "blockreturn");
}

string BlockReturn::toString(core::Context ctx) {
    return "blockreturn<" + this->link->block.data(ctx).fullName(ctx) + "> " + this->what.toString(ctx);
}

Send::Send(core::LocalVariable recv, core::NameRef fun, vector<core::LocalVariable> &args,
           std::shared_ptr<core::SendAndBlockLink> link)
    : recv(recv), fun(fun), args(move(args)), link(std::move(link)) {
    core::categoryCounterInc("cfg", "send");
    core::histogramInc("cfg.send.args", this->args.size());
}

Literal::Literal(std::shared_ptr<core::Type> value) : value(std::move(value)) {
    core::categoryCounterInc("cfg", "literal");
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
                     res = "literal(" + this->value->toString(ctx, 0) + ")";
                 }
             },
             [&](core::Type *t) { res = "literal(" + this->value->toString(ctx, 0) + ")"; });
    return res;
}

Ident::Ident(core::LocalVariable what) : what(what) {
    core::categoryCounterInc("cfg", "ident");
}

Alias::Alias(core::SymbolRef what) : what(what) {
    core::categoryCounterInc("cfg", "alias");
}

string Ident::toString(core::Context ctx) {
    return this->what.toString(ctx);
}

string Alias::toString(core::Context ctx) {
    return "alias " + this->what.data(ctx).name.data(ctx).toString(ctx);
}

string Send::toString(core::Context ctx) {
    stringstream buf;

    buf << this->recv.toString(ctx) << "." << this->fun.data(ctx).toString(ctx) << "(";
    bool isFirst = true;
    for (auto arg : this->args) {
        if (!isFirst) {
            buf << ", ";
        }
        isFirst = false;
        buf << arg.toString(ctx);
    }
    buf << ")";
    return buf.str();
}

string Self::toString(core::Context ctx) {
    return "self";
}

string HashSplat::toString(core::Context ctx) {
    Error::notImplemented();
}

string ArraySplat::toString(core::Context ctx) {
    Error::notImplemented();
}

string LoadArg::toString(core::Context ctx) {
    stringstream buf;
    buf << "load_arg(";
    buf << this->receiver.toString(ctx);
    buf << "#";
    buf << this->method.data(ctx).toString(ctx);
    buf << ", " << this->arg << ")";
    return buf.str();
}

string LoadYieldParam::toString(core::Context ctx) {
    stringstream buf;
    buf << "load_yield_param(";
    buf << this->link->block.data(ctx).fullName(ctx);
    buf << ", " << this->arg << ")";
    return buf.str();
}

string Unanalyzable::toString(core::Context ctx) {
    return "<unanalyzable>";
}

string NotSupported::toString(core::Context ctx) {
    return "NotSupported(" + why + ")";
}

string Cast::toString(core::Context ctx) {
    stringstream buf;
    buf << "cast(";
    buf << this->value.toString(ctx);
    buf << ", ";
    buf << this->type->toString(ctx);
    buf << ");";
    return buf.str();
}

string DebugEnvironment::toString(core::Context ctx) {
    return str;
}
} // namespace cfg
} // namespace sorbet
