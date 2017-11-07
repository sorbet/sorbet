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

Statement::Statement(Loc loc) : loc(loc) {}

Expression::Expression(Loc loc) : Statement(loc) {}

Reference::Reference(Loc loc) : Expression(loc) {}

ControlFlow::ControlFlow(Loc loc) : Expression(loc) {}

ClassDef::ClassDef(Loc loc, SymbolRef symbol, unique_ptr<Expression> name, vector<unique_ptr<Expression>> &ancestors,
                   vector<unique_ptr<Statement>> &rhs, ClassDefKind kind)
    : Declaration(loc, symbol), rhs(move(rhs)), name(move(name)), ancestors(move(ancestors)), kind(kind) {}

MethodDef::MethodDef(Loc loc, SymbolRef symbol, NameRef name, vector<unique_ptr<Expression>> &args,
                     unique_ptr<Expression> rhs, bool isSelf)
    : Declaration(loc, symbol), rhs(move(rhs)), args(move(args)), name(name), isSelf(isSelf) {}

Declaration::Declaration(Loc loc, SymbolRef symbol) : Expression(loc), symbol(symbol) {}

ConstDef::ConstDef(Loc loc, SymbolRef symbol, unique_ptr<Expression> rhs) : Declaration(loc, symbol), rhs(move(rhs)) {}

If::If(Loc loc, unique_ptr<Expression> cond, unique_ptr<Expression> thenp, unique_ptr<Expression> elsep)
    : ControlFlow(loc), cond(move(cond)), thenp(move(thenp)), elsep(move(elsep)) {}

While::While(Loc loc, unique_ptr<Expression> cond, unique_ptr<Statement> body)
    : ControlFlow(loc), cond(move(cond)), body(move(body)) {}

Break::Break(Loc loc, unique_ptr<Expression> expr) : ControlFlow(loc), expr(move(expr)) {}

Next::Next(Loc loc, unique_ptr<Expression> expr) : ControlFlow(loc), expr(move(expr)) {}

BoolLit::BoolLit(Loc loc, bool value) : Expression(loc), value(value) {}

Return::Return(Loc loc, unique_ptr<Expression> expr) : ControlFlow(loc), expr(move(expr)) {}

Yield::Yield(Loc loc, unique_ptr<Expression> expr) : ControlFlow(loc), expr(move(expr)) {}

Ident::Ident(Loc loc, SymbolRef symbol) : Reference(loc), symbol(symbol) {
    Error::check(symbol.exists());
}

UnresolvedIdent::UnresolvedIdent(Loc loc, VarKind kind, NameRef name) : Reference(loc), kind(kind), name(name) {}

Assign::Assign(Loc loc, unique_ptr<Expression> lhs, unique_ptr<Expression> rhs)
    : Expression(loc), lhs(move(lhs)), rhs(move(rhs)) {}

Send::Send(Loc loc, unique_ptr<Expression> recv, NameRef fun, vector<unique_ptr<Expression>> &&args)
    : Expression(loc), recv(move(recv)), fun(move(fun)), args(move(args)) {}

Super::Super(Loc loc, vector<unique_ptr<Expression>> &&args) : Expression(loc), args(move(args)) {}

New::New(Loc loc, SymbolRef claz, vector<unique_ptr<Expression>> &&args)
    : Expression(loc), claz(claz), args(move(args)) {}

NamedArg::NamedArg(Loc loc, NameRef name, unique_ptr<Expression> arg) : Expression(loc), name(name), arg(move(arg)) {}

RestArg::RestArg(Loc loc, unique_ptr<Reference> arg) : Reference(loc), expr(move(arg)) {}

KeywordArg::KeywordArg(Loc loc, unique_ptr<Reference> expr) : Reference(loc), expr(move(expr)) {}

OptionalArg::OptionalArg(Loc loc, unique_ptr<Reference> expr, unique_ptr<Expression> default_)
    : Reference(loc), expr(move(expr)), default_(move(default_)) {}

ShadowArg::ShadowArg(Loc loc, unique_ptr<Reference> expr) : Reference(loc), expr(move(expr)) {}

Nil::Nil(Loc loc) : Expression(loc) {}

FloatLit::FloatLit(Loc loc, float value) : Expression(loc), value(value) {}

IntLit::IntLit(Loc loc, int value) : Expression(loc), value(value) {}

StringLit::StringLit(Loc loc, NameRef value) : Expression(loc), value(value) {}

ConstantLit::ConstantLit(Loc loc, unique_ptr<Expression> scope, NameRef cnst)
    : Expression(loc), cnst(cnst), scope(move(scope)) {}

ArraySplat::ArraySplat(Loc loc, unique_ptr<Expression> arg) : Expression(loc), arg(move(arg)) {}

HashSplat::HashSplat(Loc loc, unique_ptr<Expression> arg) : Expression(loc), arg(move(arg)) {}

Self::Self(Loc loc, SymbolRef claz) : Expression(loc), claz(claz) {}

Block::Block(Loc loc, vector<unique_ptr<Expression>> &args, unique_ptr<Expression> body)
    : Expression(loc), args(move(args)), body(move(body)){};

NotSupported::NotSupported(Loc loc, const string &why) : Expression(loc), why(why) {}

SymbolLit::SymbolLit(Loc loc, NameRef name) : Expression(loc), name(name) {}

Hash::Hash(Loc loc, vector<unique_ptr<Expression>> &keys, vector<unique_ptr<Expression>> &values)
    : Expression(loc), keys(move(keys)), values(move(values)) {}

Array::Array(Loc loc, vector<unique_ptr<Expression>> &elems) : Expression(loc), elems(move(elems)) {}

InsSeq::InsSeq(Loc loc, vector<unique_ptr<Statement>> &&stats, unique_ptr<Expression> expr)
    : Expression(loc), stats(move(stats)), expr(move(expr)) {}

EmptyTree::EmptyTree(Loc loc) : Expression(loc) {}

void printElems(GlobalState &gs, stringstream &buf, vector<unique_ptr<Expression>> &args, int tabs) {
    bool first = true;
    bool didshadow = false;
    for (auto &a : args) {
        if (!first) {
            if (dynamic_cast<ShadowArg *>(a.get()) && !didshadow) {
                buf << "; ";
                didshadow = true;
            } else {
                buf << ", ";
            }
        }
        first = false;
        buf << a->toString(gs, tabs + 1);
    }
};

void printArgs(GlobalState &gs, stringstream &buf, vector<unique_ptr<Expression>> &args, int tabs) {
    buf << "(";
    printElems(gs, buf, args, tabs);
    buf << ")";
}

string ConstDef::toString(GlobalState &gs, int tabs) {
    return "constdef " + this->symbol.info(gs, true).name.name(gs).toString(gs) + " = " +
           this->rhs->toString(gs, tabs + 1);
}

string ConstDef::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "rhs = " << rhs->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string ClassDef::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    if (kind == ClassDefKind::Module) {
        buf << "module ";
    } else {
        buf << "class ";
    }
    buf << name->toString(gs, tabs) << "<" << this->symbol.info(gs, true).name.name(gs).toString(gs) << "> < ";
    printArgs(gs, buf, this->ancestors, tabs);

    buf << endl;

    for (auto &a : this->rhs) {
        printTabs(buf, tabs + 1);
        buf << a->toString(gs, tabs + 1) << endl << endl;
    }
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string ClassDef::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "ClassDef{" << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << name->showRaw(gs, tabs) << "<" << this->symbol.info(gs, true).name.name(gs).toString(gs) << ">"
        << endl;
    printTabs(buf, tabs + 1);
    buf << "ancestors = [";
    bool first = true;
    for (auto &a : this->ancestors) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << a->showRaw(gs, tabs + 2);
    }
    buf << "]" << endl;

    printTabs(buf, tabs + 1);
    buf << "rhs = [" << endl;

    for (auto &a : this->rhs) {
        printTabs(buf, tabs + 1);
        buf << a->showRaw(gs, tabs + 2) << endl << endl;
    }
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string InsSeq::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "begin" << endl;
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 1);
        buf << a->toString(gs, tabs + 1) << endl;
    }

    printTabs(buf, tabs + 1);
    buf << expr->toString(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string InsSeq::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "stats = [" << endl;
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "]," << endl;

    printTabs(buf, tabs + 1);
    buf << "expr = " << expr->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string MethodDef::toString(GlobalState &gs, int tabs) {
    stringstream buf;

    if (isSelf) {
        buf << "def self.";
    } else {
        buf << "def ";
    }
    buf << name.name(gs).toString(gs) << "<" << this->symbol.info(gs, true).name.name(gs).toString(gs) << ">";
    buf << "(";
    bool first = true;
    for (auto &a : this->args) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << a->toString(gs, tabs + 1);
    }
    buf << ")" << endl;
    printTabs(buf, tabs + 1);
    buf << this->rhs->toString(gs, tabs + 1);
    return buf.str();
}

string MethodDef::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "MethodDef{" << endl;
    printTabs(buf, tabs + 1);

    buf << "self = " << isSelf << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << name.name(gs).toString(gs) << "<" << this->symbol.info(gs, true).name.name(gs).toString(gs)
        << ">" << endl;
    printTabs(buf, tabs + 1);
    buf << "args = [";
    bool first = true;
    for (auto &a : this->args) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << a->showRaw(gs, tabs + 2);
    }
    buf << "]" << endl;
    printTabs(buf, tabs + 1);
    buf << "rhs = " << this->rhs->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "}";
    return buf.str();
}

string If::toString(GlobalState &gs, int tabs) {
    stringstream buf;

    buf << "if " << this->cond->toString(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << this->thenp->toString(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "else" << endl;
    printTabs(buf, tabs + 1);
    buf << this->elsep->toString(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string If::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;

    buf << "If{" << endl;
    printTabs(buf, tabs + 1);
    buf << "cond = " << this->cond->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "thenp = " << this->thenp->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "elsep = " << this->elsep->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Assign::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;

    buf << "Assign{" << endl;
    printTabs(buf, tabs + 1);
    buf << "lhs = " << this->lhs->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "rhs = " << this->rhs->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string While::toString(GlobalState &gs, int tabs) {
    stringstream buf;

    buf << "while " << this->cond->toString(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << this->body->toString(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string While::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;

    buf << "While{" << endl;
    printTabs(buf, tabs + 1);
    buf << "cond = " << this->cond->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "body = " << this->body->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string EmptyTree::toString(GlobalState &gs, int tabs) {
    return "<emptyTree>";
}

string StringLit::toString(GlobalState &gs, int tabs) {
    return "\"" + this->value.name(gs).toString(gs) + "\"";
}

string StringLit::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ value = " + this->value.name(gs).toString(gs) + " }";
}

string ConstantLit::toString(GlobalState &gs, int tabs) {
    return this->scope->toString(gs, tabs) + "::" + this->cnst.name(gs).toString(gs);
}

string ConstantLit::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;

    buf << "ConstantLit{" << endl;
    printTabs(buf, tabs + 1);
    buf << "scope = " << this->scope->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "cnst = " << this->cnst.name(gs).toString(gs) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Ident::toString(GlobalState &gs, int tabs) {
    return this->symbol.info(gs, true).fullName(gs);
}

string Ident::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "Ident{" << endl;
    printTabs(buf, tabs + 1);
    buf << "symbol = " << this->symbol.info(gs, true).name.name(gs).toString(gs) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string UnresolvedIdent::toString(GlobalState &gs, int tabs) {
    return this->name.toString(gs);
}

string UnresolvedIdent::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "UnresolvedIdent{" << endl;
    printTabs(buf, tabs + 1);
    buf << "kind = ";
    switch (this->kind) {
        case Local:
            buf << "Local";
            break;
        case Instance:
            buf << "Instance";
            break;
        case Class:
            buf << "Class";
            break;
        case Global:
            buf << "Global";
            break;
    }
    buf << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << this->name.toString(gs) << endl;
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string HashSplat::toString(GlobalState &gs, int tabs) {
    return "**" + this->arg->toString(gs, tabs + 1);
}

string ArraySplat::toString(GlobalState &gs, int tabs) {
    return "*" + this->arg->toString(gs, tabs + 1);
}

string ArraySplat::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ arg = " + this->arg->showRaw(gs, tabs + 1) + " }";
}

string HashSplat::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ arg = " + this->arg->showRaw(gs, tabs + 1) + " }";
}

string Return::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(gs, tabs + 1) + " }";
}

string Yield::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(gs, tabs + 1) + " }";
}

string Next::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(gs, tabs + 1) + " }";
}

string Break::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(gs, tabs + 1) + " }";
}

string Return::toString(GlobalState &gs, int tabs) {
    return "return " + this->expr->toString(gs, tabs + 1);
}

string Yield::toString(GlobalState &gs, int tabs) {
    return "yield(" + this->expr->toString(gs, tabs + 1) + ")";
}

string Next::toString(GlobalState &gs, int tabs) {
    return "next(" + this->expr->toString(gs, tabs + 1) + ")";
}

string Self::toString(GlobalState &gs, int tabs) {
    if (this->claz.exists()) {
        return "self(" + this->claz.info(gs).name.name(gs).toString(gs) + ")";
    } else {
        return "self(TODO)";
    }
}

string Break::toString(GlobalState &gs, int tabs) {
    return "break(" + this->expr->toString(gs, tabs + 1) + ")";
}

string IntLit::toString(GlobalState &gs, int tabs) {
    return to_string(this->value);
}

string IntLit::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ value = " + this->toString(gs, 0) + " }";
}

string FloatLit::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ value = " + this->toString(gs, 0) + " }";
}

string BoolLit::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ value = " + this->toString(gs, 0) + " }";
}

string NamedArg::toString(GlobalState &gs, int tabs) {
    return this->name.name(gs).toString(gs) + " : " + this->arg->toString(gs, tabs + 1);
}

string NamedArg::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "NamedArg{" << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << this->name.name(gs).toString(gs) << endl;
    printTabs(buf, tabs + 1);
    buf << "arg = " << this->arg->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string FloatLit::toString(GlobalState &gs, int tabs) {
    return to_string(this->value);
}

string BoolLit::toString(GlobalState &gs, int tabs) {
    if (this->value)
        return "true";
    else
        return "false";
}

string Assign::toString(GlobalState &gs, int tabs) {
    return this->lhs->toString(gs, tabs) + " = " + this->rhs->toString(gs, tabs);
}

string Rescue::toString(GlobalState &gs, int tabs) {
    return "Rescue";
}

string Rescue::showRaw(GlobalState &gs, int tabs) {
    return "Rescue";
}

string Send::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << this->recv->toString(gs, tabs) << "." << this->fun.name(gs).toString(gs);
    printArgs(gs, buf, this->args, tabs);
    if (this->block != nullptr)
        buf << this->block->toString(gs, tabs);

    return buf.str();
}

string Send::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "recv = " << this->recv->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << this->fun.name(gs).toString(gs) << endl;
    printTabs(buf, tabs + 1);
    buf << "block = ";
    if (this->block) {
        buf << this->block->showRaw(gs, tabs + 2) << endl;
    } else {
        buf << "nullptr" << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "args = [" << endl;
    for (auto &a : args) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << endl;
    }
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string New::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "claz = " << this->claz.info(gs).name.name(gs).toString(gs) << endl;
    printTabs(buf, tabs + 1);
    buf << "args = [" << endl;
    for (auto &a : args) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << endl;
    }
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string Super::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "args = [" << endl;
    for (auto &a : args) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << endl;
    }
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string Hash::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "pairs = [" << endl;
    int i = 0;
    for (auto &key : keys) {
        auto &value = values[i];

        printTabs(buf, tabs + 2);
        buf << "[" << endl;
        printTabs(buf, tabs + 3);
        buf << "key = " << key->showRaw(gs, tabs + 4) << endl;
        printTabs(buf, tabs + 3);
        buf << "value = " << value->showRaw(gs, tabs + 4) << endl;
        printTabs(buf, tabs + 2);
        buf << "]" << endl;

        i++;
    }
    printTabs(buf, tabs + 1);
    buf << "]" << endl;
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string Array::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "elems = [" << endl;
    for (auto &a : elems) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << endl;
    }
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string New::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "new " << this->claz.info(gs).name.name(gs).toString(gs);
    printArgs(gs, buf, this->args, tabs);
    return buf.str();
}

string Super::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "super";
    printArgs(gs, buf, this->args, tabs);
    return buf.str();
}

string Hash::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "{";
    bool first = true;
    int i = 0;
    for (auto &key : this->keys) {
        auto &value = this->values[i];
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << key->toString(gs, tabs + 1);
        buf << " => ";
        buf << value->toString(gs, tabs + 1);
        i++;
    }
    buf << "}";
    return buf.str();
}

string Array::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "[";
    printElems(gs, buf, this->elems, tabs);
    buf << "]";
    return buf.str();
}

string Block::toString(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << " do |";
    printElems(gs, buf, this->args, tabs + 1);
    buf << "|" << endl;
    printTabs(buf, tabs + 1);
    buf << this->body->toString(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string Block::showRaw(GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "Block {" << endl;
    printTabs(buf, tabs + 1);
    buf << "args = [" << endl;
    for (auto &a : this->args) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "]" << endl;
    printTabs(buf, tabs + 1);
    buf << "body = " << this->body->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string SymbolLit::toString(GlobalState &gs, int tabs) {
    return ":" + this->name.name(gs).toString(gs);
}

string SymbolLit::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ name = " + this->name.name(gs).toString(gs) + " }";
}

string NotSupported::toString(GlobalState &gs, int tabs) {
    return nodeName();
}

string RestArg::toString(GlobalState &gs, int tabs) {
    return "*" + this->expr->toString(gs, tabs);
}

string KeywordArg::toString(GlobalState &gs, int tabs) {
    return this->expr->toString(gs, tabs) + ":";
}

string OptionalArg::toString(GlobalState &gs, int tabs) {
    return this->expr->toString(gs, tabs) + " = " + this->default_->toString(gs, tabs);
}

string ShadowArg::toString(GlobalState &gs, int tabs) {
    return this->expr->toString(gs, tabs);
}

string NotSupported::nodeName() {
    return "<Not Supported (" + why + ")>";
}

string NotSupported::showRaw(GlobalState &gs, int tabs) {
    return "Not Supported{ why = " + why + " }";
}

string Rescue::nodeName() {
    return "Rescue";
}
string Yield::nodeName() {
    return "Next";
}
string Next::nodeName() {
    return "Next";
}
string ClassDef::nodeName() {
    return "ClassDef";
}

string ConstDef::nodeName() {
    return "ConstDef";
}
string MethodDef::nodeName() {
    return "MethodDef";
}
string If::nodeName() {
    return "If";
}
string While::nodeName() {
    return "While";
}
string Ident::nodeName() {
    return "Ident";
}
string UnresolvedIdent::nodeName() {
    return "UnresolvedIdent";
}
string Return::nodeName() {
    return "Return";
}
string Break::nodeName() {
    return "Break";
}

string SymbolLit::nodeName() {
    return "SymbolLit";
}

string Assign::nodeName() {
    return "Assign";
}

string Send::nodeName() {
    return "Send";
}

string New::nodeName() {
    return "New";
}

string Super::nodeName() {
    return "Super";
}
string NamedArg::nodeName() {
    return "NamedArg";
}

string Hash::nodeName() {
    return "Hash";
}

string Array::nodeName() {
    return "Array";
}

string FloatLit::nodeName() {
    return "FloatLit";
}

string IntLit::nodeName() {
    return "IntLit";
}

string StringLit::nodeName() {
    return "StringLit";
}

string BoolLit::nodeName() {
    return "BoolLit";
}

string ConstantLit::nodeName() {
    return "ConstantLit";
}

string ArraySplat::nodeName() {
    return "ArraySplat";
}

string HashSplat::nodeName() {
    return "HashSplat";
}

string Self::nodeName() {
    return "Self";
}

string Block::nodeName() {
    return "Block";
}

string InsSeq::nodeName() {
    return "InsSeq";
}

string EmptyTree::nodeName() {
    return "EmptyTree";
}

string Nil::toString(GlobalState &gs, int tabs) {
    return "nil";
}

string Nil::showRaw(GlobalState &gs, int tabs) {
    return nodeName();
}
string EmptyTree::showRaw(GlobalState &gs, int tabs) {
    return nodeName();
}

string Nil::nodeName() {
    return "Nil";
}
string RestArg::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(gs, tabs) + " }";
}

string RestArg::nodeName() {
    return "RestArg";
}

string Self::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ claz = " + claz.toString(gs) + " }";
}
string KeywordArg::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(gs, tabs) + " }";
}

string KeywordArg::nodeName() {
    return "KeywordArg";
}

string OptionalArg::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(gs, tabs) + " }";
}

string OptionalArg::nodeName() {
    return "OptionalArg";
}

string ShadowArg::showRaw(GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(gs, tabs) + " }";
}

string ShadowArg::nodeName() {
    return "ShadowArg";
}
} // namespace ast
} // namespace ruby_typer
