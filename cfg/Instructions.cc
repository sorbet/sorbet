#include "Instructions.h"

// helps debugging
template class std::unique_ptr<ruby_typer::cfg::Instruction>;

using namespace std;

namespace ruby_typer {
namespace cfg {

Return::Return(core::LocalVariable what) : what(what) {
    categoryCounterInc("cfg", "return");
}

string Return::toString(const core::Context ctx) {
    return "return " + this->what.toString(ctx);
}

BlockReturn::BlockReturn(core::SymbolRef block, core::LocalVariable what) : block(block), what(what) {
    categoryCounterInc("cfg", "blockreturn");
}

string BlockReturn::toString(core::Context ctx) {
    return "blockreturn<" + this->block.data(ctx).fullName(ctx) + "> " + this->what.toString(ctx);
}

Send::Send(core::LocalVariable recv, core::NameRef fun, vector<core::LocalVariable> &args, core::SymbolRef block)
    : recv(recv), fun(fun), args(move(args)), block(block) {
    categoryCounterInc("cfg", "send");
    histogramInc("cfg.send.args", this->args.size());
}

FloatLit::FloatLit(double value) : value(value) {
    categoryCounterInc("cfg", "floatlit");
}

string FloatLit::toString(const core::Context ctx) {
    return to_string(this->value);
}

IntLit::IntLit(int64_t value) : value(value) {
    categoryCounterInc("cfg", "intlit");
}

string IntLit::toString(const core::Context ctx) {
    return to_string(this->value);
}

Ident::Ident(core::LocalVariable what) : what(what) {
    categoryCounterInc("cfg", "ident");
}

Alias::Alias(core::SymbolRef what) : what(what) {
    categoryCounterInc("cfg", "alias");
}

string Ident::toString(const core::Context ctx) {
    return this->what.toString(ctx);
}

string Alias::toString(const core::Context ctx) {
    return "alias " + this->what.data(ctx).name.data(ctx).toString(ctx);
}

string Send::toString(const core::Context ctx) {
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

string StringLit::toString(const core::Context ctx) {
    return this->value.data(ctx).toString(ctx);
}

string SymbolLit::toString(const core::Context ctx) {
    return "<symbol:" + this->value.data(ctx).toString(ctx) + ">";
}

string BoolLit::toString(const core::Context ctx) {
    if (value) {
        return "true";
    } else {
        return "false";
    }
}

string Self::toString(const core::Context ctx) {
    return "self";
}

string HashSplat::toString(const core::Context ctx) {
    Error::notImplemented();
}

string ArraySplat::toString(const core::Context ctx) {
    Error::notImplemented();
}

string LoadArg::toString(const core::Context ctx) {
    stringstream buf;
    buf << "load_arg(";
    buf << this->receiver.toString(ctx);
    buf << "#";
    buf << this->method.data(ctx).toString(ctx);
    buf << ", " << this->arg << ")";
    return buf.str();
}

string LoadYieldParam::toString(const core::Context ctx) {
    stringstream buf;
    buf << "load_yield_param(";
    buf << this->block.data(ctx).fullName(ctx);
    buf << ", " << this->arg << ")";
    return buf.str();
}

string Unanalyzable::toString(const core::Context ctx) {
    return "<unanalyzable>";
}

string NotSupported::toString(const core::Context ctx) {
    return "NotSupported(" + why + ")";
}

string Cast::toString(const core::Context ctx) {
    stringstream buf;
    buf << "cast(";
    buf << this->value.toString(ctx);
    buf << ", ";
    buf << this->type->toString(ctx);
    buf << ");";
    return buf.str();
}

string DebugEnvironment::toString(const core::Context ctx) {
    return str;
}

} // namespace cfg
} // namespace ruby_typer
