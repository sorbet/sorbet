#include "Trees.h"
#include <sstream>

// makes lldb work. Don't remove please
template class std::unique_ptr<ruby_typer::ast::Expression>;
template class std::unique_ptr<ruby_typer::ast::Statement>;

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

void printTabs(std::stringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

ClassDef::ClassDef(SymbolRef symbol, std::unique_ptr<Expression> name, std::vector<std::unique_ptr<Statement>> &rhs,
                   bool isModule)
    : Declaration(symbol), rhs(std::move(rhs)), name(std::move(name)), isModule(isModule) {}

MethodDef::MethodDef(SymbolRef symbol, NameRef name, std::vector<std::unique_ptr<Expression>> &args,
                     std::unique_ptr<Expression> rhs, bool isSelf)
    : Declaration(symbol), rhs(std::move(rhs)), args(std::move(args)), name(name), isSelf(isSelf) {}

Declaration::Declaration(SymbolRef symbol) : symbol(symbol) {}

ConstDef::ConstDef(SymbolRef symbol, std::unique_ptr<Expression> rhs) : Declaration(symbol), rhs(std::move(rhs)) {}

If::If(std::unique_ptr<Expression> cond, std::unique_ptr<Expression> thenp, std::unique_ptr<Expression> elsep)
    : cond(std::move(cond)), thenp(std::move(thenp)), elsep(std::move(elsep)) {}

Breakable::Breakable(u1 break_tag) : break_tag(break_tag) {}

While::While(u1 break_tag, std::unique_ptr<Expression> cond, std::unique_ptr<Statement> body)
    : Breakable(break_tag), cond(std::move(cond)), body(std::move(body)) {}

Break::Break(u1 break_tag) : break_tag(break_tag) {}

Next::Next(u1 break_tag) : break_tag(break_tag) {}

BoolLit::BoolLit(bool value) : value(value) {}

std::string Next::toString(ContextBase &ctx, int tabs) {
    return "next";
}

Return::Return(std::unique_ptr<Expression> expr) : expr(std::move(expr)) {}

Ident::Ident(SymbolRef symbol) : symbol(symbol), name(0) {
    Error::check(!symbol.isSynthetic()); // symbol is a valid symbol
}

Ident::Ident(NameRef name, SymbolRef symbol) : symbol(symbol), name(name) {
    Error::check(symbol.isSynthetic()); // symbol is a sentinel
}

Assign::Assign(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

Send::Send(std::unique_ptr<Expression> recv, NameRef fun, std::vector<std::unique_ptr<Expression>> &&args)
    :

      recv(std::move(recv)), fun(std::move(fun)), args(std::move(args)) {}

Super::Super(std::vector<std::unique_ptr<Expression>> &&args) : args(std::move(args)) {}

New::New(SymbolRef claz, std::vector<std::unique_ptr<Expression>> &&args) : claz(claz), args(std::move(args)) {}

NamedArg::NamedArg(NameRef name, std::unique_ptr<Expression> arg) : name(name), arg(std::move(arg)) {}

FloatLit::FloatLit(float value) : value(value) {}

IntLit::IntLit(int value) : value(value) {}

StringLit::StringLit(NameRef value) : value(value) {}

ConstantLit::ConstantLit(std::unique_ptr<Expression> scope, NameRef cnst) : cnst(cnst), scope(std::move(scope)) {}

ArraySplat::ArraySplat(std::unique_ptr<Expression> arg) : arg(std::move(arg)) {}

HashSplat::HashSplat(std::unique_ptr<Expression> arg) : arg(std::move(arg)) {}

Self::Self(SymbolRef claz) : claz(claz) {}

Block::Block(std::unique_ptr<Send> send, std::vector<std::unique_ptr<Expression>> &args,
             std::unique_ptr<Expression> rhs)
    : send(std::move(send)), rhs(std::move(rhs)), args(std::move(args)){};

NotSupported::NotSupported(const std::string &why) : why(why) {}

Symbol::Symbol(NameRef name) : name(name) {}

Array::Array(std::vector<std::unique_ptr<Expression>> &elems) : elems(std::move(elems)) {}

InsSeq::InsSeq(std::vector<std::unique_ptr<Statement>> &&stats, std::unique_ptr<Expression> expr)
    : stats(std::move(stats)), expr(std::move(expr)) {}

std::string ConstDef::toString(ContextBase &ctx, int tabs) {
    return "constdef " + this->symbol.info(ctx).name.name(ctx).toString(ctx) + " = " +
           this->rhs->toString(ctx, tabs + 1);
}

std::string ClassDef::toString(ContextBase &ctx, int tabs) {
    std::stringstream buf;
    if (isModule) {
        buf << "module ";
    } else {
        buf << "class ";
    }
    buf << name->toString(ctx, tabs) << "<" << this->symbol.info(ctx).name.name(ctx).toString(ctx) << ">" << std::endl;

    for (auto &a : this->rhs) {
        printTabs(buf, tabs + 1);
        buf << a->toString(ctx, tabs + 1) << std::endl << std::endl;
    }
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

std::string InsSeq::toString(ContextBase &ctx, int tabs) {
    std::stringstream buf;
    buf << "begin" << std::endl;
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 1);
        buf << a->toString(ctx, tabs + 1) << std::endl;
    }

    printTabs(buf, tabs + 1);
    buf << expr->toString(ctx, tabs + 1) << std::endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

std::string MethodDef::toString(ContextBase &ctx, int tabs) {
    std::stringstream buf;

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
    buf << ")" << std::endl;
    printTabs(buf, tabs + 1);
    buf << this->rhs->toString(ctx, tabs + 1);
    return buf.str();
}

std::string If::toString(ContextBase &ctx, int tabs) {
    std::stringstream buf;

    buf << "if " << this->cond->toString(ctx, tabs + 1) << std::endl;
    printTabs(buf, tabs + 1);
    buf << this->thenp->toString(ctx, tabs + 1) << std::endl;
    printTabs(buf, tabs);
    buf << "else " << std::endl;
    printTabs(buf, tabs + 1);
    buf << this->elsep->toString(ctx, tabs + 1) << std::endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

std::string While::toString(ContextBase &ctx, int tabs) {
    std::stringstream buf;

    buf << "while " << this->cond->toString(ctx, tabs + 1) << std::endl;
    printTabs(buf, tabs + 1);
    buf << this->body->toString(ctx, tabs + 1) << std::endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

std::string EmptyTree::toString(ContextBase &ctx, int tabs) {
    return "<emtpyTree>";
}

std::string ArraySplat::toString(ContextBase &ctx, int tabs) {
    return "*" + this->arg->toString(ctx, tabs + 1);
}

std::string StringLit::toString(ContextBase &ctx, int tabs) {
    return "\"" + this->value.name(ctx).toString(ctx) + "\"";
}

std::string ConstantLit::toString(ContextBase &ctx, int tabs) {
    return this->scope->toString(ctx, tabs) + "::" + this->cnst.name(ctx).toString(ctx);
}

std::string Ident::toString(ContextBase &ctx, int tabs) {
    if (!symbol.isSynthetic()) {
        return this->symbol.info(ctx).name.name(ctx).toString(ctx);
    } else {
        return this->name.name(ctx).toString(ctx) + this->symbol.info(ctx).name.name(ctx).toString(ctx);
    }
}

std::string HashSplat::toString(ContextBase &ctx, int tabs) {
    return "**" + this->arg->toString(ctx, tabs + 1);
}

std::string Return::toString(ContextBase &ctx, int tabs) {
    return "return " + this->expr->toString(ctx, tabs + 1);
}

std::string Self::toString(ContextBase &ctx, int tabs) {
    return "self(" + this->claz.info(ctx).name.name(ctx).toString(ctx) + ")";
}

std::string Break::toString(ContextBase &ctx, int tabs) {
    return "break";
}

std::string IntLit::toString(ContextBase &ctx, int tabs) {
    return std::to_string(this->value);
}

std::string NamedArg::toString(ContextBase &ctx, int tabs) {
    return this->name.name(ctx).toString(ctx) + " : " + this->arg->toString(ctx, tabs + 1);
}

std::string FloatLit::toString(ContextBase &ctx, int tabs) {
    return std::to_string(this->value);
}

std::string BoolLit::toString(ContextBase &ctx, int tabs) {
    return std::to_string(this->value);
}

std::string Assign::toString(ContextBase &ctx, int tabs) {
    return this->lhs->toString(ctx, tabs) + " = " + this->rhs->toString(ctx, tabs);
}

std::string Rescue::toString(ContextBase &ctx, int tabs) {
    return "Rescue";
}

void printElems(ContextBase &ctx, std::stringstream &buf, std::vector<std::unique_ptr<Expression>> &args, int tabs) {
    bool first = true;
    for (auto &a : args) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << a->toString(ctx, tabs + 1);
    }
};

void printArgs(ContextBase &ctx, std::stringstream &buf, std::vector<std::unique_ptr<Expression>> &args, int tabs) {
    buf << "(";
    printElems(ctx, buf, args, tabs);
    buf << ")";
}

std::string Send::toString(ContextBase &ctx, int tabs) {
    std::stringstream buf;
    buf << this->recv->toString(ctx, tabs) << "." << this->fun.name(ctx).toString(ctx);
    printArgs(ctx, buf, this->args, tabs);

    return buf.str();
}

std::string New::toString(ContextBase &ctx, int tabs) {
    std::stringstream buf;
    buf << "new " << this->claz.info(ctx).name.name(ctx).toString(ctx);
    printArgs(ctx, buf, this->args, tabs);
    return buf.str();
}

std::string Super::toString(ContextBase &ctx, int tabs) {
    std::stringstream buf;
    buf << "super";
    printArgs(ctx, buf, this->args, tabs);
    return buf.str();
}

std::string Array::toString(ContextBase &ctx, int tabs) {
    std::stringstream buf;
    buf << "[";
    printElems(ctx, buf, this->elems, tabs);
    buf << "]";
    return buf.str();
}

std::string Block::toString(ContextBase &ctx, int tabs) {
    std::stringstream buf;
    buf << this->send->toString(ctx, tabs);
    buf << " do |";
    printElems(ctx, buf, this->args, tabs + 1);
    buf << "|" << std::endl;
    printTabs(buf, tabs + 1);
    buf << this->rhs->toString(ctx, tabs + 1) << std::endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

std::string Symbol::toString(ContextBase &ctx, int tabs) {
    return ":" + this->name.name(ctx).toString(ctx);
}

std::string NotSupported::toString(ContextBase &ctx, int tabs) {
    return "<Not Supported (" + why + ")>";
}
} // namespace ast
} // namespace ruby_typer
