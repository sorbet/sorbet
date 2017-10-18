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

ClassDef::ClassDef(SymbolRef symbol, unique_ptr<Expression> name, vector<unique_ptr<Statement>> &rhs, ClassDefKind kind)
    : Declaration(symbol), rhs(move(rhs)), name(move(name)), kind(kind) {}

MethodDef::MethodDef(SymbolRef symbol, NameRef name, vector<unique_ptr<Expression>> &args, unique_ptr<Expression> rhs,
                     bool isSelf)
    : Declaration(symbol), rhs(move(rhs)), args(move(args)), name(name), isSelf(isSelf) {}

Declaration::Declaration(SymbolRef symbol) : symbol(symbol) {}

ConstDef::ConstDef(SymbolRef symbol, unique_ptr<Expression> rhs) : Declaration(symbol), rhs(move(rhs)) {}

If::If(unique_ptr<Expression> cond, unique_ptr<Expression> thenp, unique_ptr<Expression> elsep)
    : cond(move(cond)), thenp(move(thenp)), elsep(move(elsep)) {}

Breakable::Breakable(u1 break_tag) : break_tag(break_tag) {}

While::While(u1 break_tag, unique_ptr<Expression> cond, unique_ptr<Statement> body)
    : Breakable(break_tag), cond(move(cond)), body(move(body)) {}

Break::Break(u1 break_tag) : break_tag(break_tag) {}

Next::Next(u1 break_tag) : break_tag(break_tag) {}

BoolLit::BoolLit(bool value) : value(value) {}

string Next::toString(ContextBase &ctx, int tabs) {
    return "next";
}

Return::Return(unique_ptr<Expression> expr) : expr(move(expr)) {}

Ident::Ident(SymbolRef symbol) : symbol(symbol), name(0) {
    Error::check(!symbol.isSynthetic()); // symbol is a valid symbol
}

Ident::Ident(NameRef name, SymbolRef symbol) : symbol(symbol), name(name) {
    Error::check(symbol.isSynthetic()); // symbol is a sentinel
}

Assign::Assign(unique_ptr<Expression> lhs, unique_ptr<Expression> rhs) : lhs(move(lhs)), rhs(move(rhs)) {}

Send::Send(unique_ptr<Expression> recv, NameRef fun, vector<unique_ptr<Expression>> &&args)
    :

      recv(move(recv)), fun(move(fun)), args(move(args)) {}

Super::Super(vector<unique_ptr<Expression>> &&args) : args(move(args)) {}

New::New(SymbolRef claz, vector<unique_ptr<Expression>> &&args) : claz(claz), args(move(args)) {}

NamedArg::NamedArg(NameRef name, unique_ptr<Expression> arg) : name(name), arg(move(arg)) {}

FloatLit::FloatLit(float value) : value(value) {}

IntLit::IntLit(int value) : value(value) {}

StringLit::StringLit(NameRef value) : value(value) {}

ConstantLit::ConstantLit(unique_ptr<Expression> scope, NameRef cnst) : cnst(cnst), scope(move(scope)) {}

ArraySplat::ArraySplat(unique_ptr<Expression> arg) : arg(move(arg)) {}

HashSplat::HashSplat(unique_ptr<Expression> arg) : arg(move(arg)) {}

Self::Self(SymbolRef claz) : claz(claz) {}

Block::Block(unique_ptr<Send> send, vector<unique_ptr<Expression>> &args, unique_ptr<Expression> rhs)
    : send(move(send)), rhs(move(rhs)), args(move(args)){};

NotSupported::NotSupported(const string &why) : why(why) {}

Symbol::Symbol(NameRef name) : name(name) {}

Array::Array(vector<unique_ptr<Expression>> &elems) : elems(move(elems)) {}

InsSeq::InsSeq(vector<unique_ptr<Statement>> &&stats, unique_ptr<Expression> expr)
    : stats(move(stats)), expr(move(expr)) {}

string ConstDef::toString(ContextBase &ctx, int tabs) {
    return "constdef " + this->symbol.info(ctx).name.name(ctx).toString(ctx) + " = " +
           this->rhs->toString(ctx, tabs + 1);
}

string ClassDef::toString(ContextBase &ctx, int tabs) {
    stringstream buf;
    if (kind == ClassDefKind::Module) {
        buf << "module ";
    } else {
        buf << "class ";
    }
    buf << name->toString(ctx, tabs) << "<" << this->symbol.info(ctx).name.name(ctx).toString(ctx) << ">" << endl;

    for (auto &a : this->rhs) {
        printTabs(buf, tabs + 1);
        buf << a->toString(ctx, tabs + 1) << endl << endl;
    }
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string InsSeq::toString(ContextBase &ctx, int tabs) {
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

string MethodDef::toString(ContextBase &ctx, int tabs) {
    stringstream buf;

    if (isSelf) {
        buf << "def self.";
    } else {
        buf << "def ";
    }
    buf << name.name(ctx).toString(ctx) << "<" << this->symbol.info(ctx).name.name(ctx).toString(ctx) << ">";
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

string If::toString(ContextBase &ctx, int tabs) {
    stringstream buf;

    buf << "if " << this->cond->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << this->thenp->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "else " << endl;
    printTabs(buf, tabs + 1);
    buf << this->elsep->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string While::toString(ContextBase &ctx, int tabs) {
    stringstream buf;

    buf << "while " << this->cond->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << this->body->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string EmptyTree::toString(ContextBase &ctx, int tabs) {
    return "<emtpyTree>";
}

string ArraySplat::toString(ContextBase &ctx, int tabs) {
    return "*" + this->arg->toString(ctx, tabs + 1);
}

string StringLit::toString(ContextBase &ctx, int tabs) {
    return "\"" + this->value.name(ctx).toString(ctx) + "\"";
}

string ConstantLit::toString(ContextBase &ctx, int tabs) {
    return this->scope->toString(ctx, tabs) + "::" + this->cnst.name(ctx).toString(ctx);
}

string Ident::toString(ContextBase &ctx, int tabs) {
    if (!symbol.isSynthetic()) {
        return this->symbol.info(ctx).name.name(ctx).toString(ctx);
    } else {
        return this->name.name(ctx).toString(ctx) + this->symbol.info(ctx).name.name(ctx).toString(ctx);
    }
}

string HashSplat::toString(ContextBase &ctx, int tabs) {
    return "**" + this->arg->toString(ctx, tabs + 1);
}

string Return::toString(ContextBase &ctx, int tabs) {
    return "return " + this->expr->toString(ctx, tabs + 1);
}

string Self::toString(ContextBase &ctx, int tabs) {
    return "self(" + this->claz.info(ctx).name.name(ctx).toString(ctx) + ")";
}

string Break::toString(ContextBase &ctx, int tabs) {
    return "break";
}

string IntLit::toString(ContextBase &ctx, int tabs) {
    return to_string(this->value);
}

string NamedArg::toString(ContextBase &ctx, int tabs) {
    return this->name.name(ctx).toString(ctx) + " : " + this->arg->toString(ctx, tabs + 1);
}

string FloatLit::toString(ContextBase &ctx, int tabs) {
    return to_string(this->value);
}

string BoolLit::toString(ContextBase &ctx, int tabs) {
    if (this->value)
        return "true";
    else
        return "false";
}

string Assign::toString(ContextBase &ctx, int tabs) {
    return this->lhs->toString(ctx, tabs) + " = " + this->rhs->toString(ctx, tabs);
}

string Rescue::toString(ContextBase &ctx, int tabs) {
    return "Rescue";
}

void printElems(ContextBase &ctx, stringstream &buf, vector<unique_ptr<Expression>> &args, int tabs) {
    bool first = true;
    for (auto &a : args) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << a->toString(ctx, tabs + 1);
    }
};

void printArgs(ContextBase &ctx, stringstream &buf, vector<unique_ptr<Expression>> &args, int tabs) {
    buf << "(";
    printElems(ctx, buf, args, tabs);
    buf << ")";
}

string Send::toString(ContextBase &ctx, int tabs) {
    stringstream buf;
    buf << this->recv->toString(ctx, tabs) << "." << this->fun.name(ctx).toString(ctx);
    printArgs(ctx, buf, this->args, tabs);

    return buf.str();
}

string New::toString(ContextBase &ctx, int tabs) {
    stringstream buf;
    buf << "new " << this->claz.info(ctx).name.name(ctx).toString(ctx);
    printArgs(ctx, buf, this->args, tabs);
    return buf.str();
}

string Super::toString(ContextBase &ctx, int tabs) {
    stringstream buf;
    buf << "super";
    printArgs(ctx, buf, this->args, tabs);
    return buf.str();
}

string Array::toString(ContextBase &ctx, int tabs) {
    stringstream buf;
    buf << "[";
    printElems(ctx, buf, this->elems, tabs);
    buf << "]";
    return buf.str();
}

string Block::toString(ContextBase &ctx, int tabs) {
    stringstream buf;
    buf << this->send->toString(ctx, tabs);
    buf << " do |";
    printElems(ctx, buf, this->args, tabs + 1);
    buf << "|" << endl;
    printTabs(buf, tabs + 1);
    buf << this->rhs->toString(ctx, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string Symbol::toString(ContextBase &ctx, int tabs) {
    return ":" + this->name.name(ctx).toString(ctx);
}


std::string NotSupported::toString(ContextBase &ctx, int tabs) {
    return nodeName();
}

std::string NotSupported::nodeName() {
        return "<Not Supported (" + why + ")>";
    }

std::string Rescue::nodeName() {
    return "Rescue";
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

std::string Symbol::nodeName() {
    return "Symbol";
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
} // namespace ast
} // namespace ruby_typer
