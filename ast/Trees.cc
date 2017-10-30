#include "Trees.h"
#include <sstream>

// makes lldb work. Don't remove please
template class std::unique_ptr<ruby_typer::ast::Expression>;
template class std::unique_ptr<ruby_typer::ast::Statement>;

using namespace std;

namespace ruby_typer {
namespace ast {

/** https://git.corp.stripe.com/gist/nelhage/51564501674174da24822e60ad770f64
 *
 *  [] - prototype only
 *
 *                 / Control Flow <- while, if, for, break, next, return, rescue, case
 * Pre-CFG-Node <-
 *                 \ Instruction <- assign, send, [new], ident, named_arg, hash, array, literals(symbols, ints, floats,
 * strings, constants, nil), constants(resolver will desugar it into literals), array_splat(*), hash_splat(**), self,
 * insseq, Block)
 *
 *                  \ Definition  <-  class(name, parent, mixins, body)
 *                                    module
 *                                    def
 *                                    defself
 *                                    const_assign
 *
 *
 *
 * know id for: top, bottom, kernel?, basicobject, class, module [postponed], unit, Hash, Array, String, Symbol, float,
 * int, numeric, double, unknown
 *
 *
 *
 * Desugar string concatenation into series of .to_s calls and string concatenations
 */

void printTabs(stringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

unique_ptr<Expression> Expression::fromStatement(unique_ptr<Statement> &statement) {
    Error::check(dynamic_cast<Expression *>(statement.get()));
    return unique_ptr<Expression>(dynamic_cast<Expression *>(statement.release()));
}

unique_ptr<Expression> Expression::fromStatement(unique_ptr<Statement> &&statement) {
    return fromStatement(statement);
}

ClassDef::ClassDef(SymbolRef symbol, unique_ptr<Expression> name, vector<unique_ptr<Expression>> &ancestors,
                   vector<unique_ptr<Statement>> &rhs, ClassDefKind kind)
    : Declaration(symbol), rhs(move(rhs)), name(move(name)), ancestors(move(ancestors)), kind(kind) {}

MethodDef::MethodDef(SymbolRef symbol, NameRef name, vector<unique_ptr<Expression>> &args, unique_ptr<Expression> rhs,
                     bool isSelf)
    : Declaration(symbol), rhs(move(rhs)), args(move(args)), name(name), isSelf(isSelf) {}

Declaration::Declaration(SymbolRef symbol) : symbol(symbol) {}

ConstDef::ConstDef(SymbolRef symbol, unique_ptr<Expression> rhs) : Declaration(symbol), rhs(move(rhs)) {}

If::If(unique_ptr<Expression> cond, unique_ptr<Expression> thenp, unique_ptr<Expression> elsep)
    : cond(move(cond)), thenp(move(thenp)), elsep(move(elsep)) {}

While::While(unique_ptr<Expression> cond, unique_ptr<Statement> body) : cond(move(cond)), body(move(body)) {}

Break::Break(unique_ptr<Expression> expr) : expr(move(expr)) {}

Next::Next(unique_ptr<Expression> expr) : expr(move(expr)) {}

BoolLit::BoolLit(bool value) : value(value) {}

Return::Return(unique_ptr<Expression> expr) : expr(move(expr)) {}

Yield::Yield(unique_ptr<Expression> expr) : expr(move(expr)) {}

Ident::Ident(SymbolRef symbol) : symbol(symbol), name(0) {
    Error::check(!symbol.isPlaceHolder()); // symbol is a valid symbol
}

Ident::Ident(NameRef name, SymbolRef symbol) : symbol(symbol), name(name) {
    Error::check(symbol.isPlaceHolder()); // symbol is a sentinel
}

Assign::Assign(unique_ptr<Expression> lhs, unique_ptr<Expression> rhs) : lhs(move(lhs)), rhs(move(rhs)) {}

Send::Send(unique_ptr<Expression> recv, NameRef fun, vector<unique_ptr<Expression>> &&args)
    : recv(move(recv)), fun(move(fun)), args(move(args)) {}

Super::Super(vector<unique_ptr<Expression>> &&args) : args(move(args)) {}

New::New(SymbolRef claz, vector<unique_ptr<Expression>> &&args) : claz(claz), args(move(args)) {}

NamedArg::NamedArg(NameRef name, unique_ptr<Expression> arg) : name(name), arg(move(arg)) {}

RestArg::RestArg(unique_ptr<Reference> arg) : expr(move(arg)) {}

KeywordArg::KeywordArg(std::unique_ptr<Reference> expr) : expr(move(expr)) {}

OptionalArg::OptionalArg(std::unique_ptr<Reference> expr, std::unique_ptr<Expression> default_)
    : expr(move(expr)), default_(move(default_)) {}

FloatLit::FloatLit(float value) : value(value) {}

IntLit::IntLit(int value) : value(value) {}

StringLit::StringLit(NameRef value) : value(value) {}

ConstantLit::ConstantLit(unique_ptr<Expression> scope, NameRef cnst) : cnst(cnst), scope(move(scope)) {}

ArraySplat::ArraySplat(unique_ptr<Expression> arg) : arg(move(arg)) {}

HashSplat::HashSplat(unique_ptr<Expression> arg) : arg(move(arg)) {}

Self::Self(SymbolRef claz) : claz(claz) {}

Block::Block(vector<unique_ptr<Expression>> &args, unique_ptr<Expression> body) : args(move(args)), body(move(body)){};

NotSupported::NotSupported(const string &why) : why(why) {}

SymbolLit::SymbolLit(NameRef name) : name(name) {}

Array::Array(vector<unique_ptr<Expression>> &elems) : elems(move(elems)) {}

InsSeq::InsSeq(vector<unique_ptr<Statement>> &&stats, unique_ptr<Expression> expr)
    : stats(move(stats)), expr(move(expr)) {}

void printElems(GlobalState &ctx, stringstream &buf, vector<unique_ptr<Expression>> &args, int tabs) {
    bool first = true;
    for (auto &a : args) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << a->toString(ctx, tabs + 1);
    }
};

void printArgs(GlobalState &ctx, stringstream &buf, vector<unique_ptr<Expression>> &args, int tabs) {
    buf << "(";
    printElems(ctx, buf, args, tabs);
    buf << ")";
}

string ConstDef::toString(GlobalState &ctx, int tabs) {

    return "constdef " + this->symbol.info(ctx, true).name.name(ctx).toString(ctx) + " = " +
           this->rhs->toString(ctx, tabs + 1);
}

string ConstDef::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "rhs = " << rhs->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string ClassDef::toString(GlobalState &ctx, int tabs) {
    stringstream buf;
    if (kind == ClassDefKind::Module) {
        buf << "module ";
    } else {
        buf << "class ";
    }
    buf << name->toString(ctx, tabs) << "<" << this->symbol.info(ctx, true).name.name(ctx).toString(ctx) << "> < ";
    printArgs(ctx, buf, this->ancestors, tabs);

    buf << endl;

    for (auto &a : this->rhs) {
        printTabs(buf, tabs + 1);
        buf << a->toString(ctx, tabs + 1) << endl << endl;
    }
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string ClassDef::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << "ClassDef{" << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << name->showRaw(ctx, tabs) << "<" << this->symbol.info(ctx, true).name.name(ctx).toString(ctx)
        << ">" << endl;
    printTabs(buf, tabs + 1);
    buf << "ancestors = [";
    bool first = true;
    for (auto &a : this->ancestors) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << a->showRaw(ctx, tabs + 2);
    }
    buf << "]" << endl;

    printTabs(buf, tabs + 1);
    buf << "rhs = [" << endl;

    for (auto &a : this->rhs) {
        printTabs(buf, tabs + 1);
        buf << a->showRaw(ctx, tabs + 2) << endl << endl;
    }
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string InsSeq::toString(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << "begin" << endl;
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 1);
        buf << a->toString(ctx, tabs + 1) << endl;
    }

    printTabs(buf, tabs + 1);
    buf << expr->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string InsSeq::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "stats = [" << endl;
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(ctx, tabs + 2) << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "]," << endl;

    printTabs(buf, tabs + 1);
    buf << "expr = " << expr->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string MethodDef::toString(GlobalState &ctx, int tabs) {
    stringstream buf;

    if (isSelf) {
        buf << "def self.";
    } else {
        buf << "def ";
    }
    buf << name.name(ctx).toString(ctx) << "<" << this->symbol.info(ctx, true).name.name(ctx).toString(ctx) << ">";
    buf << "(";
    bool first = true;
    for (auto &a : this->args) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << a->toString(ctx, tabs + 1);
    }
    buf << ")" << endl;
    printTabs(buf, tabs + 1);
    buf << this->rhs->toString(ctx, tabs + 1);
    return buf.str();
}

string MethodDef::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << "MethodDef{" << endl;
    printTabs(buf, tabs + 1);

    buf << "self = " << isSelf << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << name.name(ctx).toString(ctx) << "<" << this->symbol.info(ctx, true).name.name(ctx).toString(ctx)
        << ">" << endl;
    printTabs(buf, tabs + 1);
    buf << "args = [";
    bool first = true;
    for (auto &a : this->args) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << a->showRaw(ctx, tabs + 2);
    }
    buf << "]" << endl;
    printTabs(buf, tabs + 1);
    buf << "rhs = " << this->rhs->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "}";
    return buf.str();
}

string If::toString(GlobalState &ctx, int tabs) {
    stringstream buf;

    buf << "if " << this->cond->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << this->thenp->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "else" << endl;
    printTabs(buf, tabs + 1);
    buf << this->elsep->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string If::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;

    buf << "If{" << endl;
    printTabs(buf, tabs + 1);
    buf << "cond = " << this->cond->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "thenp = " << this->thenp->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "elsep = " << this->elsep->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Assign::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;

    buf << "Assign{" << endl;
    printTabs(buf, tabs + 1);
    buf << "lhs = " << this->lhs->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "rhs = " << this->rhs->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string While::toString(GlobalState &ctx, int tabs) {
    stringstream buf;

    buf << "while " << this->cond->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << this->body->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string While::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;

    buf << "While{" << endl;
    printTabs(buf, tabs + 1);
    buf << "cond = " << this->cond->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "body = " << this->body->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string EmptyTree::toString(GlobalState &ctx, int tabs) {
    return "<emptyTree>";
}

string StringLit::toString(GlobalState &ctx, int tabs) {
    return "\"" + this->value.name(ctx).toString(ctx) + "\"";
}

string StringLit::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ value = " + this->value.name(ctx).toString(ctx) + " }";
}

string ConstantLit::toString(GlobalState &ctx, int tabs) {
    return this->scope->toString(ctx, tabs) + "::" + this->cnst.name(ctx).toString(ctx);
}

string ConstantLit::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;

    buf << "ConstantLit{" << endl;
    printTabs(buf, tabs + 1);
    buf << "scope = " << this->scope->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "cnst = " << this->cnst.name(ctx).toString(ctx) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Ident::toString(GlobalState &ctx, int tabs) {
    if (!symbol.isPlaceHolder()) {
        return this->symbol.info(ctx, true).fullName(ctx);
    } else {
        return this->name.name(ctx).toString(ctx) + this->symbol.info(ctx, true).name.name(ctx).toString(ctx);
    }
}

string Ident::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << "Ident{" << endl;
    printTabs(buf, tabs + 1);
    buf << "symbol = " << this->symbol.info(ctx, true).name.name(ctx).toString(ctx) << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << (name.exists() ? this->name.name(ctx).toString(ctx) : "<none>") << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string HashSplat::toString(GlobalState &ctx, int tabs) {
    return "**" + this->arg->toString(ctx, tabs + 1);
}

string ArraySplat::toString(GlobalState &ctx, int tabs) {
    return "*" + this->arg->toString(ctx, tabs + 1);
}

string ArraySplat::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ arg = " + this->arg->showRaw(ctx, tabs + 1) + " }";
}

string HashSplat::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ arg = " + this->arg->showRaw(ctx, tabs + 1) + " }";
}

string Return::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(ctx, tabs + 1) + " }";
}

string Yield::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(ctx, tabs + 1) + " }";
}

string Next::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(ctx, tabs + 1) + " }";
}

string Break::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(ctx, tabs + 1) + " }";
}

string Return::toString(GlobalState &ctx, int tabs) {
    return "return " + this->expr->toString(ctx, tabs + 1);
}

string Yield::toString(GlobalState &ctx, int tabs) {
    return "yield(" + this->expr->toString(ctx, tabs + 1) + ")";
}

string Next::toString(GlobalState &ctx, int tabs) {
    return "next(" + this->expr->toString(ctx, tabs + 1) + ")";
}

string Self::toString(GlobalState &ctx, int tabs) {
    return "self(" + this->claz.info(ctx).name.name(ctx).toString(ctx) + ")";
}

string Break::toString(GlobalState &ctx, int tabs) {
    return "break(" + this->expr->toString(ctx, tabs + 1) + ")";
}

string IntLit::toString(GlobalState &ctx, int tabs) {
    return to_string(this->value);
}

string IntLit::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ value = " + this->toString(ctx, 0) + " }";
}

string FloatLit::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ value = " + this->toString(ctx, 0) + " }";
}

string BoolLit::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ value = " + this->toString(ctx, 0) + " }";
}

string NamedArg::toString(GlobalState &ctx, int tabs) {
    return this->name.name(ctx).toString(ctx) + " : " + this->arg->toString(ctx, tabs + 1);
}

string NamedArg::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << "NamedArg{" << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << this->name.name(ctx).toString(ctx) << endl;
    printTabs(buf, tabs + 1);
    buf << "arg = " << this->arg->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string FloatLit::toString(GlobalState &ctx, int tabs) {
    return to_string(this->value);
}

string BoolLit::toString(GlobalState &ctx, int tabs) {
    if (this->value)
        return "true";
    else
        return "false";
}

string Assign::toString(GlobalState &ctx, int tabs) {
    return this->lhs->toString(ctx, tabs) + " = " + this->rhs->toString(ctx, tabs);
}

string Rescue::toString(GlobalState &ctx, int tabs) {
    return "Rescue";
}

string Rescue::showRaw(GlobalState &ctx, int tabs) {
    return "Rescue";
}

string Send::toString(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << this->recv->toString(ctx, tabs) << "." << this->fun.name(ctx).toString(ctx);
    printArgs(ctx, buf, this->args, tabs);
    if (this->block != nullptr)
        buf << this->block->toString(ctx, tabs);

    return buf.str();
}

string Send::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "recv = " << this->recv->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << this->fun.name(ctx).toString(ctx) << endl;
    printTabs(buf, tabs + 1);
    buf << "block = ";
    if (this->block) {
        buf << this->block->showRaw(ctx, tabs + 2) << endl;
    } else {
        buf << "nullptr" << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "args = [" << endl;
    for (auto &a : args) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(ctx, tabs + 2) << endl;
    }
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string New::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "claz = " << this->claz.info(ctx).name.name(ctx).toString(ctx) << endl;
    printTabs(buf, tabs + 1);
    buf << "args = [" << endl;
    for (auto &a : args) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(ctx, tabs + 2) << endl;
    }
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string Super::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "args = [" << endl;
    for (auto &a : args) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(ctx, tabs + 2) << endl;
    }
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string Array::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "elems = [" << endl;
    for (auto &a : elems) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(ctx, tabs + 2) << endl;
    }
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string New::toString(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << "new " << this->claz.info(ctx).name.name(ctx).toString(ctx);
    printArgs(ctx, buf, this->args, tabs);
    return buf.str();
}

string Super::toString(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << "super";
    printArgs(ctx, buf, this->args, tabs);
    return buf.str();
}

string Array::toString(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << "[";
    printElems(ctx, buf, this->elems, tabs);
    buf << "]";
    return buf.str();
}

string Block::toString(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << " do |";
    printElems(ctx, buf, this->args, tabs + 1);
    buf << "|" << endl;
    printTabs(buf, tabs + 1);
    buf << this->body->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string Block::showRaw(GlobalState &ctx, int tabs) {
    stringstream buf;
    buf << "Block {" << endl;
    printTabs(buf, tabs + 1);
    buf << "args = [" << endl;
    for (auto &a : this->args) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(ctx, tabs + 2) << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "]" << endl;
    printTabs(buf, tabs + 1);
    buf << "body = " << this->body->showRaw(ctx, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string SymbolLit::toString(GlobalState &ctx, int tabs) {
    return ":" + this->name.name(ctx).toString(ctx);
}

std::string SymbolLit::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ name = " + this->name.name(ctx).toString(ctx) + " }";
}

std::string NotSupported::toString(GlobalState &ctx, int tabs) {
    return nodeName();
}

std::string RestArg::toString(GlobalState &ctx, int tabs) {
    return "*" + this->expr->toString(ctx, tabs);
}

std::string KeywordArg::toString(GlobalState &ctx, int tabs) {
    return this->expr->toString(ctx, tabs) + ":";
}

std::string OptionalArg::toString(GlobalState &ctx, int tabs) {
    return this->expr->toString(ctx, tabs) + " = " + this->default_->toString(ctx, tabs);
}

std::string NotSupported::nodeName() {
    return "<Not Supported (" + why + ")>";
}

std::string NotSupported::showRaw(GlobalState &ctx, int tabs) {
    return "Not Supported{ why = " + why + " }";
}

std::string Rescue::nodeName() {
    return "Rescue";
}
std::string Yield::nodeName() {
    return "Next";
}
std::string Next::nodeName() {
    return "Next";
}
std::string ClassDef::nodeName() {
    return "ClassDef";
}

std::string ConstDef::nodeName() {
    return "ConstDef";
}
std::string MethodDef::nodeName() {
    return "MethodDef";
}
std::string If::nodeName() {
    return "If";
}
std::string While::nodeName() {
    return "While";
}
std::string Ident::nodeName() {
    return "Ident";
}

std::string Return::nodeName() {
    return "Return";
}
std::string Break::nodeName() {
    return "Break";
}

std::string SymbolLit::nodeName() {
    return "SymbolLit";
}

std::string Assign::nodeName() {
    return "Assign";
}

std::string Send::nodeName() {
    return "Send";
}

std::string New::nodeName() {
    return "New";
}

std::string Super::nodeName() {
    return "Super";
}
std::string NamedArg::nodeName() {
    return "NamedArg";
}

std::string Array::nodeName() {
    return "Array";
}

std::string FloatLit::nodeName() {
    return "FloatLit";
}

std::string IntLit::nodeName() {
    return "IntLit";
}

std::string StringLit::nodeName() {
    return "StringLit";
}

std::string BoolLit::nodeName() {
    return "BoolLit";
}

std::string ConstantLit::nodeName() {
    return "ConstantLit";
}

std::string ArraySplat::nodeName() {
    return "ArraySplat";
}

std::string HashSplat::nodeName() {
    return "HashSplat";
}

std::string Self::nodeName() {
    return "Self";
}

std::string Block::nodeName() {
    return "Block";
}

std::string InsSeq::nodeName() {
    return "InsSeq";
}

std::string EmptyTree::nodeName() {
    return "EmptyTree";
}

std::string Nil::toString(GlobalState &ctx, int tabs) {
    return "nil";
}

std::string Nil::showRaw(GlobalState &ctx, int tabs) {
    return nodeName();
}
std::string EmptyTree::showRaw(GlobalState &ctx, int tabs) {
    return nodeName();
}

std::string Nil::nodeName() {
    return "Nil";
}
std::string RestArg::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(ctx, tabs) + " }";
}

std::string RestArg::nodeName() {
    return "RestArg";
}

std::string Self::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ claz = " + claz.toString(ctx) + " }";
}
std::string KeywordArg::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(ctx, tabs) + " }";
}

std::string KeywordArg::nodeName() {
    return "KeywordArg";
}

std::string OptionalArg::showRaw(GlobalState &ctx, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(ctx, tabs) + " }";
}

std::string OptionalArg::nodeName() {
    return "OptionalArg";
}
} // namespace ast
} // namespace ruby_typer
