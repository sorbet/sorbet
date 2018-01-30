#include "Trees.h"
#include <sstream>

// makes lldb work. Don't remove please
template class std::unique_ptr<ruby_typer::ast::Expression>;
template class std::unique_ptr<ruby_typer::ast::Reference>;

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

Expression::Expression(core::Loc loc) : loc(loc) {}

Reference::Reference(core::Loc loc) : Expression(loc) {}

ClassDef::ClassDef(core::Loc loc, core::SymbolRef symbol, unique_ptr<Expression> name, ANCESTORS_store ancestors,
                   RHS_store rhs, ClassDefKind kind)
    : Declaration(loc, symbol), rhs(move(rhs)), name(move(name)), ancestors(move(ancestors)), kind(kind) {
    categoryCounterInc("trees", "classdef");
    histogramInc("trees.classdef.kind", (int)kind);
    histogramInc("trees.classdef.ancestors", this->ancestors.size());
}

MethodDef::MethodDef(core::Loc loc, core::SymbolRef symbol, core::NameRef name, ARGS_store args,
                     unique_ptr<Expression> rhs, bool isSelf)
    : Declaration(loc, symbol), rhs(move(rhs)), args(move(args)), name(name), isSelf(isSelf) {
    categoryCounterInc("trees", "methoddef");
    histogramInc("trees.methodDef.args", this->args.size());
}

Declaration::Declaration(core::Loc loc, core::SymbolRef symbol) : Expression(loc), symbol(symbol) {}

ConstDef::ConstDef(core::Loc loc, core::SymbolRef symbol, unique_ptr<Expression> rhs)
    : Declaration(loc, symbol), rhs(move(rhs)) {
    categoryCounterInc("trees", "constdef");
}

If::If(core::Loc loc, unique_ptr<Expression> cond, unique_ptr<Expression> thenp, unique_ptr<Expression> elsep)
    : Expression(loc), cond(move(cond)), thenp(move(thenp)), elsep(move(elsep)) {
    categoryCounterInc("trees", "if");
}

While::While(core::Loc loc, unique_ptr<Expression> cond, unique_ptr<Expression> body)
    : Expression(loc), cond(move(cond)), body(move(body)) {
    categoryCounterInc("trees", "while");
}

Break::Break(core::Loc loc, unique_ptr<Expression> expr) : Expression(loc), expr(move(expr)) {
    categoryCounterInc("trees", "break");
}

Retry::Retry(core::Loc loc) : Expression(loc) {
    categoryCounterInc("trees", "retry");
}

Next::Next(core::Loc loc, unique_ptr<Expression> expr) : Expression(loc), expr(move(expr)) {
    categoryCounterInc("trees", "next");
}

BoolLit::BoolLit(core::Loc loc, bool value) : Expression(loc), value(value) {
    categoryCounterInc("trees", "boollit");
}

Return::Return(core::Loc loc, unique_ptr<Expression> expr) : Expression(loc), expr(move(expr)) {
    categoryCounterInc("trees", "return");
}

Yield::Yield(core::Loc loc, unique_ptr<Expression> expr) : Expression(loc), expr(move(expr)) {
    categoryCounterInc("trees", "yield");
}

RescueCase::RescueCase(core::Loc loc, EXCEPTION_store exceptions, unique_ptr<Expression> var,
                       unique_ptr<Expression> body)
    : Expression(loc), exceptions(move(exceptions)), var(move(var)), body(move(body)) {
    categoryCounterInc("trees", "rescuecase");
    histogramInc("trees.rescueCase.exceptions", this->exceptions.size());
}

Rescue::Rescue(core::Loc loc, unique_ptr<Expression> body, RESCUE_CASE_store rescueCases, unique_ptr<Expression> else_,
               unique_ptr<Expression> ensure)
    : Expression(loc), body(move(body)), rescueCases(move(rescueCases)), else_(move(else_)), ensure(move(ensure)) {
    categoryCounterInc("trees", "rescue");
    histogramInc("trees.rescue.rescuecases", this->rescueCases.size());
}

Ident::Ident(core::Loc loc, core::SymbolRef symbol) : Reference(loc), symbol(symbol) {
    ENFORCE(symbol.exists(), "Creating Ident over non-existing symbol");
    categoryCounterInc("trees", "ident");
}

Local::Local(core::Loc loc, core::LocalVariable localVariable1) : Expression(loc), localVariable(localVariable1) {
    categoryCounterInc("trees", "local");
}

UnresolvedIdent::UnresolvedIdent(core::Loc loc, VarKind kind, core::NameRef name)
    : Reference(loc), kind(kind), name(name) {
    categoryCounterInc("trees", "unresolvedident");
}

Assign::Assign(core::Loc loc, unique_ptr<Expression> lhs, unique_ptr<Expression> rhs)
    : Expression(loc), lhs(move(lhs)), rhs(move(rhs)) {
    categoryCounterInc("trees", "assign");
}

Send::Send(core::Loc loc, unique_ptr<Expression> recv, core::NameRef fun, Send::ARGS_store args,
           unique_ptr<Block> block)
    : Expression(loc), recv(move(recv)), fun(fun), args(move(args)), block(move(block)) {
    categoryCounterInc("trees", "send");
    if (block) {
        counterInc("trees.send.with_block");
    }
    histogramInc("trees.send.args", this->args.size());
}

Cast::Cast(core::Loc loc, std::shared_ptr<core::Type> ty, std::unique_ptr<Expression> arg, bool assertType)
    : Expression(loc), type(ty), arg(move(arg)), assertType(assertType) {
    categoryCounterInc("Trees", "Cast");
}

ZSuperArgs::ZSuperArgs(core::Loc loc) : Expression(loc) {
    categoryCounterInc("trees", "zsuper");
}

RestArg::RestArg(core::Loc loc, unique_ptr<Reference> arg) : Reference(loc), expr(move(arg)) {
    categoryCounterInc("trees", "restarg");
}

KeywordArg::KeywordArg(core::Loc loc, unique_ptr<Reference> expr) : Reference(loc), expr(move(expr)) {
    categoryCounterInc("trees", "keywordarg");
}

OptionalArg::OptionalArg(core::Loc loc, unique_ptr<Reference> expr, unique_ptr<Expression> default_)
    : Reference(loc), expr(move(expr)), default_(move(default_)) {
    categoryCounterInc("trees", "optionalarg");
}

ShadowArg::ShadowArg(core::Loc loc, unique_ptr<Reference> expr) : Reference(loc), expr(move(expr)) {
    categoryCounterInc("trees", "shadowarg");
}

BlockArg::BlockArg(core::Loc loc, unique_ptr<Reference> expr) : Reference(loc), expr(move(expr)) {
    categoryCounterInc("trees", "blockarg");
}

FloatLit::FloatLit(core::Loc loc, double value) : Expression(loc), value(value) {
    categoryCounterInc("trees", "floatlit");
}

IntLit::IntLit(core::Loc loc, int64_t value) : Expression(loc), value(value) {
    categoryCounterInc("trees", "intlit");
}

StringLit::StringLit(core::Loc loc, core::NameRef value) : Expression(loc), value(value) {
    categoryCounterInc("trees", "stringlit");
}

ConstantLit::ConstantLit(core::Loc loc, unique_ptr<Expression> scope, core::NameRef cnst)
    : Expression(loc), cnst(cnst), scope(move(scope)) {
    categoryCounterInc("trees", "constantlit");
}

ArraySplat::ArraySplat(core::Loc loc, unique_ptr<Expression> arg) : Expression(loc), arg(move(arg)) {
    categoryCounterInc("trees", "arraysplat");
}

HashSplat::HashSplat(core::Loc loc, unique_ptr<Expression> arg) : Expression(loc), arg(move(arg)) {
    categoryCounterInc("trees", "hashsplat");
}

Self::Self(core::Loc loc, core::SymbolRef claz) : Expression(loc), claz(claz) {
    categoryCounterInc("trees", "self");
}

Block::Block(core::Loc loc, MethodDef::ARGS_store args, unique_ptr<Expression> body)
    : Expression(loc), args(move(args)), body(move(body)) {
    categoryCounterInc("trees", "block");
};

SymbolLit::SymbolLit(core::Loc loc, core::NameRef name) : Expression(loc), name(name) {
    categoryCounterInc("trees", "symbollit");
}

Hash::Hash(core::Loc loc, ENTRY_store keys, ENTRY_store values)
    : Expression(loc), keys(move(keys)), values(move(values)) {
    categoryCounterInc("trees", "hash");
    histogramInc("trees.hash.entries", this->keys.size());
}

Array::Array(core::Loc loc, ENTRY_store elems) : Expression(loc), elems(move(elems)) {
    categoryCounterInc("trees", "array");
    histogramInc("trees.array.elems", this->elems.size());
}

InsSeq::InsSeq(core::Loc loc, STATS_store stats, unique_ptr<Expression> expr)
    : Expression(loc), stats(move(stats)), expr(move(expr)) {
    categoryCounterInc("trees", "insseq");
    histogramInc("trees.insseq.stats", this->stats.size());
}

EmptyTree::EmptyTree(core::Loc loc) : Expression(loc) {
    categoryCounterInc("trees", "emptytree");
}

template <class T> void printElems(const core::GlobalState &gs, stringstream &buf, T &args, int tabs) {
    bool first = true;
    bool didshadow = false;
    for (auto &a : args) {
        if (!first) {
            if (cast_tree<ShadowArg>(a.get()) && !didshadow) {
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

template <class T> void printArgs(const core::GlobalState &gs, stringstream &buf, T &args, int tabs) {
    buf << "(";
    printElems(gs, buf, args, tabs);
    buf << ")";
}

string ConstDef::toString(const core::GlobalState &gs, int tabs) {
    return "constdef " + this->symbol.data(gs, true).name.data(gs).toString(gs) + " = " +
           this->rhs->toString(gs, tabs + 1);
}

string ConstDef::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "rhs = " << rhs->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string ClassDef::toString(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    if (kind == ClassDefKind::Module) {
        buf << "module ";
    } else {
        buf << "class ";
    }
    buf << name->toString(gs, tabs) << "<" << this->symbol.data(gs, true).name.data(gs).toString(gs) << "> < ";
    printArgs(gs, buf, this->ancestors, tabs);

    for (auto &a : this->rhs) {
        buf << endl;
        printTabs(buf, tabs + 1);
        buf << a->toString(gs, tabs + 1) << endl;
    }

    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string ClassDef::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "ClassDef{" << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << name->showRaw(gs, tabs + 1) << "<" << this->symbol.data(gs, true).name.data(gs).toString(gs)
        << ">" << endl;
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
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << endl;
        if (&a != &this->rhs.back()) {
            buf << endl;
        }
    }
    printTabs(buf, tabs + 1);
    buf << "]" << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string InsSeq::toString(const core::GlobalState &gs, int tabs) {
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

string InsSeq::showRaw(const core::GlobalState &gs, int tabs) {
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
    buf << "expr = " << expr->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string MethodDef::toString(const core::GlobalState &gs, int tabs) {
    stringstream buf;

    if (isSelf) {
        buf << "def self.";
    } else {
        buf << "def ";
    }
    auto &data = this->symbol.data(gs, true);
    buf << name.data(gs).toString(gs) << "<" << data.name.data(gs).toString(gs) << ">";
    buf << "(";
    bool first = true;
    if (this->symbol == core::GlobalState::defn_todo()) {
        for (auto &a : this->args) {
            if (!first) {
                buf << ", ";
            }
            first = false;
            buf << a->toString(gs, tabs + 1);
        }
    } else {
        for (auto &a : data.arguments()) {
            if (!first) {
                buf << ", ";
            }
            first = false;
            buf << a.data(gs).name.toString(gs);
        }
    }
    buf << ")" << endl;
    printTabs(buf, tabs + 1);
    buf << this->rhs->toString(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string MethodDef::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "MethodDef{" << endl;
    printTabs(buf, tabs + 1);

    buf << "self = " << isSelf << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << name.data(gs).toString(gs) << "<" << this->symbol.data(gs, true).name.data(gs).toString(gs)
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
    buf << "rhs = " << this->rhs->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string If::toString(const core::GlobalState &gs, int tabs) {
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

string If::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;

    buf << "If{" << endl;
    printTabs(buf, tabs + 1);
    buf << "cond = " << this->cond->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << "thenp = " << this->thenp->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << "elsep = " << this->elsep->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Assign::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;

    buf << "Assign{" << endl;
    printTabs(buf, tabs + 1);
    buf << "lhs = " << this->lhs->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << "rhs = " << this->rhs->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string While::toString(const core::GlobalState &gs, int tabs) {
    stringstream buf;

    buf << "while " << this->cond->toString(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << this->body->toString(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string While::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;

    buf << "While{" << endl;
    printTabs(buf, tabs + 1);
    buf << "cond = " << this->cond->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << "body = " << this->body->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string EmptyTree::toString(const core::GlobalState &gs, int tabs) {
    return "<emptyTree>";
}

string StringLit::toString(const core::GlobalState &gs, int tabs) {
    return "\"" + this->value.data(gs).toString(gs) + "\"";
}

string StringLit::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ value = " + this->value.data(gs).toString(gs) + " }";
}

string ConstantLit::toString(const core::GlobalState &gs, int tabs) {
    return this->scope->toString(gs, tabs) + "::" + this->cnst.data(gs).toString(gs);
}

string ConstantLit::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;

    buf << "ConstantLit{" << endl;
    printTabs(buf, tabs + 1);
    buf << "scope = " << this->scope->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << "cnst = " << this->cnst.data(gs).toString(gs) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Ident::toString(const core::GlobalState &gs, int tabs) {
    return this->symbol.data(gs, true).fullName(gs);
}

std::string Local::toString(const core::GlobalState &gs, int tabs) {
    return this->localVariable.toString(gs);
}

std::string Local::nodeName() {
    return "Local";
}

string Ident::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "Ident{" << endl;
    printTabs(buf, tabs + 1);
    buf << "symbol = " << this->symbol.data(gs, true).name.data(gs).toString(gs) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Local::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "Local{" << endl;
    printTabs(buf, tabs + 1);
    buf << "localVariable = " << this->localVariable.toString(gs) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string UnresolvedIdent::toString(const core::GlobalState &gs, int tabs) {
    return this->name.toString(gs);
}

string UnresolvedIdent::showRaw(const core::GlobalState &gs, int tabs) {
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

string HashSplat::toString(const core::GlobalState &gs, int tabs) {
    return "**" + this->arg->toString(gs, tabs + 1);
}

string ArraySplat::toString(const core::GlobalState &gs, int tabs) {
    return "*" + this->arg->toString(gs, tabs + 1);
}

string ArraySplat::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ arg = " + this->arg->showRaw(gs, tabs + 1) + " }";
}

string HashSplat::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ arg = " + this->arg->showRaw(gs, tabs + 1) + " }";
}

string Return::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(gs, tabs + 1) + " }";
}

string Yield::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(gs, tabs + 1) + " }";
}

string Next::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(gs, tabs + 1) + " }";
}

string Break::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + this->expr->showRaw(gs, tabs + 1) + " }";
}

string Retry::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{}";
}

string Return::toString(const core::GlobalState &gs, int tabs) {
    return "return " + this->expr->toString(gs, tabs + 1);
}

string Yield::toString(const core::GlobalState &gs, int tabs) {
    return "yield(" + this->expr->toString(gs, tabs + 1) + ")";
}

string Next::toString(const core::GlobalState &gs, int tabs) {
    return "next(" + this->expr->toString(gs, tabs + 1) + ")";
}

string Self::toString(const core::GlobalState &gs, int tabs) {
    if (this->claz.exists()) {
        return "self(" + this->claz.data(gs).name.data(gs).toString(gs) + ")";
    } else {
        return "self(TODO)";
    }
}

string Break::toString(const core::GlobalState &gs, int tabs) {
    return "break(" + this->expr->toString(gs, tabs + 1) + ")";
}

string Retry::toString(const core::GlobalState &gs, int tabs) {
    return "retry";
}

string IntLit::toString(const core::GlobalState &gs, int tabs) {
    return to_string(this->value);
}

string IntLit::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ value = " + this->toString(gs, 0) + " }";
}

string FloatLit::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ value = " + this->toString(gs, 0) + " }";
}

string BoolLit::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ value = " + this->toString(gs, 0) + " }";
}

string FloatLit::toString(const core::GlobalState &gs, int tabs) {
    return to_string(this->value);
}

string BoolLit::toString(const core::GlobalState &gs, int tabs) {
    if (this->value) {
        return "true";
    } else {
        return "false";
    }
}

string Assign::toString(const core::GlobalState &gs, int tabs) {
    return this->lhs->toString(gs, tabs) + " = " + this->rhs->toString(gs, tabs);
}

string RescueCase::toString(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "rescue";
    bool first = true;
    for (auto &exception : this->exceptions) {
        if (first) {
            first = false;
            buf << " ";
        } else {
            buf << ", ";
        }
        buf << exception->toString(gs, tabs);
    }
    buf << " => " << this->var->toString(gs, tabs);
    buf << endl;
    printTabs(buf, tabs);
    buf << this->body->toString(gs, tabs);
    return buf.str();
}

string RescueCase::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "exceptions = [" << endl;
    for (auto &a : exceptions) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "]" << endl;
    printTabs(buf, tabs + 1);
    buf << "var = " << this->var->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << "body = " << this->body->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Rescue::toString(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << this->body->toString(gs, tabs);
    for (auto &rescueCase : this->rescueCases) {
        buf << endl;
        printTabs(buf, tabs - 1);
        buf << rescueCase->toString(gs, tabs);
    }
    if (dynamic_cast<EmptyTree *>(this->else_.get()) == nullptr) {
        buf << endl;
        printTabs(buf, tabs - 1);
        buf << "else" << endl;
        printTabs(buf, tabs);
        buf << this->else_->toString(gs, tabs);
    }
    if (dynamic_cast<EmptyTree *>(this->ensure.get()) == nullptr) {
        buf << endl;
        printTabs(buf, tabs - 1);
        buf << "ensure" << endl;
        printTabs(buf, tabs);
        buf << this->ensure->toString(gs, tabs);
    }
    return buf.str();
}

string Rescue::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "body = " << this->body->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << "rescueCases = [" << endl;
    for (auto &a : rescueCases) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "]" << endl;
    printTabs(buf, tabs + 1);
    buf << "else = " << this->else_->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << "ensure = " << this->ensure->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Send::toString(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << this->recv->toString(gs, tabs) << "." << this->fun.data(gs).toString(gs);
    printArgs(gs, buf, this->args, tabs);
    if (this->block != nullptr) {
        buf << this->block->toString(gs, tabs);
    }

    return buf.str();
}

string Send::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "recv = " << this->recv->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs + 1);
    buf << "name = " << this->fun.data(gs).toString(gs) << endl;
    printTabs(buf, tabs + 1);
    buf << "block = ";
    if (this->block) {
        buf << this->block->showRaw(gs, tabs + 1) << endl;
    } else {
        buf << "nullptr" << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "args = [" << endl;
    for (auto &a : args) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "]" << endl;
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string Cast::toString(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    if (this->assertType) {
        buf << "T.assert_type!";
    } else {
        buf << "T.cast";
    }
    buf << "(" << this->arg->toString(gs, tabs) << ", " << this->type->toString(gs, tabs) << ")";

    return buf.str();
}

string Cast::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 2);
    buf << "assertType = " << this->assertType << "," << endl;
    printTabs(buf, tabs + 2);
    buf << "arg = " << this->arg->showRaw(gs, tabs + 2) << endl;
    printTabs(buf, tabs + 2);
    buf << "type = " << this->type->toString(gs) << "," << endl;
    printTabs(buf, tabs);
    buf << "}" << endl;

    return buf.str();
}

string ZSuperArgs::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ }";
}

string Hash::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "pairs = [" << endl;
    int i = -1;
    for (auto &key : keys) {
        i++;
        auto &value = values[i];

        printTabs(buf, tabs + 2);
        buf << "[" << endl;
        printTabs(buf, tabs + 3);
        buf << "key = " << key->showRaw(gs, tabs + 3) << endl;
        printTabs(buf, tabs + 3);
        buf << "value = " << value->showRaw(gs, tabs + 3) << endl;
        printTabs(buf, tabs + 2);
        buf << "]" << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "]" << endl;
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string Array::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << endl;
    printTabs(buf, tabs + 1);
    buf << "elems = [" << endl;
    for (auto &a : elems) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << endl;
    }
    printTabs(buf, tabs + 1);
    buf << "]" << endl;
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string ZSuperArgs::toString(const core::GlobalState &gs, int tabs) {
    return "ZSuperArgs";
}

string Hash::toString(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "{";
    bool first = true;
    int i = -1;
    for (auto &key : this->keys) {
        i++;
        auto &value = this->values[i];
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << key->toString(gs, tabs + 1);
        buf << " => ";
        buf << value->toString(gs, tabs + 1);
    }
    buf << "}";
    return buf.str();
}

string Array::toString(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << "[";
    printElems(gs, buf, this->elems, tabs);
    buf << "]";
    return buf.str();
}

string Block::toString(const core::GlobalState &gs, int tabs) {
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

string Block::showRaw(const core::GlobalState &gs, int tabs) {
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
    buf << "body = " << this->body->showRaw(gs, tabs + 1) << endl;
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string SymbolLit::toString(const core::GlobalState &gs, int tabs) {
    return ":" + this->name.data(gs).toString(gs);
}

string SymbolLit::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ name = " + this->name.data(gs).toString(gs) + " }";
}

string RestArg::toString(const core::GlobalState &gs, int tabs) {
    return "*" + this->expr->toString(gs, tabs);
}

string KeywordArg::toString(const core::GlobalState &gs, int tabs) {
    return this->expr->toString(gs, tabs) + ":";
}

string OptionalArg::toString(const core::GlobalState &gs, int tabs) {
    return this->expr->toString(gs, tabs) + " = " + this->default_->toString(gs, tabs);
}

string ShadowArg::toString(const core::GlobalState &gs, int tabs) {
    return this->expr->toString(gs, tabs);
}

string BlockArg::toString(const core::GlobalState &gs, int tabs) {
    return "&" + this->expr->toString(gs, tabs);
}

string RescueCase::nodeName() {
    return "RescueCase";
}
string Rescue::nodeName() {
    return "Rescue";
}
string Yield::nodeName() {
    return "Yield";
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
string Retry::nodeName() {
    return "Retry";
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

string Cast::nodeName() {
    return "Cast";
}

string ZSuperArgs::nodeName() {
    return "ZSuperArgs";
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

string EmptyTree::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName();
}

string RestArg::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(gs, tabs) + " }";
}

string RestArg::nodeName() {
    return "RestArg";
}

string Self::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ claz = " + this->claz.data(gs).fullName(gs) + " }";
}
string KeywordArg::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(gs, tabs) + " }";
}

string KeywordArg::nodeName() {
    return "KeywordArg";
}

string OptionalArg::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(gs, tabs) + " }";
}

string OptionalArg::nodeName() {
    return "OptionalArg";
}

string ShadowArg::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(gs, tabs) + " }";
}

string BlockArg::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(gs, tabs) + " }";
}

string ShadowArg::nodeName() {
    return "ShadowArg";
}

string BlockArg::nodeName() {
    return "BlockArg";
}
} // namespace ast
} // namespace ruby_typer
