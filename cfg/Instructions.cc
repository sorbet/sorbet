#include "Instructions.h"

// helps debugging
template class std::unique_ptr<ruby_typer::cfg::Instruction>;

using namespace std;

namespace ruby_typer {
namespace cfg {

Return::Return(core::LocalVariable what) : what(what) {
    categoryCounterInc("cfg", "return");
}

string Return::toString(core::Context ctx) {
    return "return " + this->what.name.name(ctx).toString(ctx);
}

Send::Send(core::LocalVariable recv, core::NameRef fun, vector<core::LocalVariable> &args)
    : recv(recv), fun(fun), args(move(args)) {
    categoryCounterInc("cfg", "send");
    histogramInc("cfg.send.args", this->args.size());
}

FloatLit::FloatLit(double value) : value(value) {
    categoryCounterInc("cfg", "floatlit");
}

string FloatLit::toString(core::Context ctx) {
    return to_string(this->value);
}

IntLit::IntLit(int64_t value) : value(value) {
    categoryCounterInc("cfg", "intlit");
}

string IntLit::toString(core::Context ctx) {
    return to_string(this->value);
}

Ident::Ident(core::LocalVariable what) : what(what) {
    categoryCounterInc("cfg", "ident");
}

Alias::Alias(core::SymbolRef what) : what(what) {
    categoryCounterInc("cfg", "alias");
}

string Ident::toString(core::Context ctx) {
    return this->what.name.name(ctx).toString(ctx);
}

string Alias::toString(core::Context ctx) {
    return "alias " + this->what.info(ctx).name.name(ctx).toString(ctx);
}

string Send::toString(core::Context ctx) {
    stringstream buf;
    buf << this->recv.name.name(ctx).toString(ctx) << "." << this->fun.name(ctx).toString(ctx) << "(";
    bool isFirst = true;
    for (auto arg : this->args) {
        if (!isFirst) {
            buf << ", ";
        }
        isFirst = false;
        buf << arg.name.name(ctx).toString(ctx);
    }
    buf << ")";
    return buf.str();
}

string StringLit::toString(core::Context ctx) {
    return this->value.name(ctx).toString(ctx);
}

string SymbolLit::toString(core::Context ctx) {
    return "<symbol:" + this->value.name(ctx).toString(ctx) + ">";
}

string BoolLit::toString(core::Context ctx) {
    if (value) {
        return "true";
    } else {
        return "false";
    }
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
    buf << this->receiver.name.name(ctx).toString(ctx);
    buf << "#";
    buf << this->method.name(ctx).toString(ctx);
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
    buf << this->value.name.toString(ctx);
    buf << ", ";
    buf << this->type->toString(ctx);
    buf << ");";
    return buf.str();
}

string DebugEnvironment::toString(core::Context ctx) {
    return str;
}

} // namespace cfg
} // namespace ruby_typer
