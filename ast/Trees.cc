#include "ast/Trees.h"
#include "common/formatting.h"
#include "common/typecase.h"
#include "core/Symbols.h"
#include <sstream>
#include <utility>

// makes lldb work. Don't remove please
template class std::unique_ptr<sorbet::ast::Expression>;
template class std::unique_ptr<sorbet::ast::Reference>;
template class std::unique_ptr<sorbet::ast::ClassDef>;
template class std::unique_ptr<sorbet::ast::MethodDef>;
template class std::unique_ptr<sorbet::ast::If>;
template class std::unique_ptr<sorbet::ast::While>;
template class std::unique_ptr<sorbet::ast::Break>;
template class std::unique_ptr<sorbet::ast::Retry>;
template class std::unique_ptr<sorbet::ast::Next>;
template class std::unique_ptr<sorbet::ast::Return>;
template class std::unique_ptr<sorbet::ast::RescueCase>;
template class std::unique_ptr<sorbet::ast::Rescue>;
template class std::unique_ptr<sorbet::ast::Local>;
template class std::unique_ptr<sorbet::ast::UnresolvedIdent>;
template class std::unique_ptr<sorbet::ast::RestArg>;
template class std::unique_ptr<sorbet::ast::KeywordArg>;
template class std::unique_ptr<sorbet::ast::OptionalArg>;
template class std::unique_ptr<sorbet::ast::BlockArg>;
template class std::unique_ptr<sorbet::ast::ShadowArg>;
template class std::unique_ptr<sorbet::ast::Assign>;
template class std::unique_ptr<sorbet::ast::Send>;
template class std::unique_ptr<sorbet::ast::Cast>;
template class std::unique_ptr<sorbet::ast::Hash>;
template class std::unique_ptr<sorbet::ast::Array>;
template class std::unique_ptr<sorbet::ast::Literal>;
template class std::unique_ptr<sorbet::ast::UnresolvedConstantLit>;
template class std::unique_ptr<sorbet::ast::ZSuperArgs>;
template class std::unique_ptr<sorbet::ast::Block>;
template class std::unique_ptr<sorbet::ast::InsSeq>;
template class std::unique_ptr<sorbet::ast::EmptyTree>;
template class std::unique_ptr<sorbet::ast::ConstantLit>;

using namespace std;

namespace sorbet::ast {

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

Expression::Expression(core::LocOffsets loc) : loc(loc) {}

Reference::Reference(core::LocOffsets loc) : Expression(loc) {}

ClassDef::ClassDef(core::LocOffsets loc, core::Loc declLoc, core::SymbolRef symbol, unique_ptr<Expression> name,
                   ANCESTORS_store ancestors, RHS_store rhs, ClassDef::Kind kind)
    : Declaration(loc, declLoc, symbol), kind(kind), rhs(std::move(rhs)), name(std::move(name)),
      ancestors(std::move(ancestors)) {
    categoryCounterInc("trees", "classdef");
    histogramInc("trees.classdef.kind", (int)kind);
    histogramInc("trees.classdef.ancestors", this->ancestors.size());
    _sanityCheck();
}

MethodDef::MethodDef(core::LocOffsets loc, core::Loc declLoc, core::SymbolRef symbol, core::NameRef name,
                     ARGS_store args, unique_ptr<Expression> rhs, Flags flags)
    : Declaration(loc, declLoc, symbol), rhs(std::move(rhs)), args(std::move(args)), name(name), flags(flags) {
    categoryCounterInc("trees", "methoddef");
    histogramInc("trees.methodDef.args", this->args.size());
    _sanityCheck();
}

Declaration::Declaration(core::LocOffsets loc, core::Loc declLoc, core::SymbolRef symbol)
    : Expression(loc), declLoc(declLoc), symbol(symbol) {}

If::If(core::LocOffsets loc, unique_ptr<Expression> cond, unique_ptr<Expression> thenp, unique_ptr<Expression> elsep)
    : Expression(loc), cond(std::move(cond)), thenp(std::move(thenp)), elsep(std::move(elsep)) {
    categoryCounterInc("trees", "if");
    _sanityCheck();
}

While::While(core::LocOffsets loc, unique_ptr<Expression> cond, unique_ptr<Expression> body)
    : Expression(loc), cond(std::move(cond)), body(std::move(body)) {
    categoryCounterInc("trees", "while");
    _sanityCheck();
}

Break::Break(core::LocOffsets loc, unique_ptr<Expression> expr) : Expression(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "break");
    _sanityCheck();
}

Retry::Retry(core::LocOffsets loc) : Expression(loc) {
    categoryCounterInc("trees", "retry");
    _sanityCheck();
}

Next::Next(core::LocOffsets loc, unique_ptr<Expression> expr) : Expression(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "next");
    _sanityCheck();
}

Return::Return(core::LocOffsets loc, unique_ptr<Expression> expr) : Expression(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "return");
    _sanityCheck();
}

RescueCase::RescueCase(core::LocOffsets loc, EXCEPTION_store exceptions, unique_ptr<Expression> var,
                       unique_ptr<Expression> body)
    : Expression(loc), exceptions(std::move(exceptions)), var(std::move(var)), body(std::move(body)) {
    categoryCounterInc("trees", "rescuecase");
    histogramInc("trees.rescueCase.exceptions", this->exceptions.size());
    _sanityCheck();
}

Rescue::Rescue(core::LocOffsets loc, unique_ptr<Expression> body, RESCUE_CASE_store rescueCases,
               unique_ptr<Expression> else_, unique_ptr<Expression> ensure)
    : Expression(loc), body(std::move(body)), rescueCases(std::move(rescueCases)), else_(std::move(else_)),
      ensure(std::move(ensure)) {
    categoryCounterInc("trees", "rescue");
    histogramInc("trees.rescue.rescuecases", this->rescueCases.size());
    _sanityCheck();
}

Local::Local(core::LocOffsets loc, core::LocalVariable localVariable1) : Reference(loc), localVariable(localVariable1) {
    categoryCounterInc("trees", "local");
    _sanityCheck();
}

UnresolvedIdent::UnresolvedIdent(core::LocOffsets loc, Kind kind, core::NameRef name)
    : Reference(loc), name(name), kind(kind) {
    categoryCounterInc("trees", "unresolvedident");
    _sanityCheck();
    _sanityCheck();
}

Assign::Assign(core::LocOffsets loc, unique_ptr<Expression> lhs, unique_ptr<Expression> rhs)
    : Expression(loc), lhs(std::move(lhs)), rhs(std::move(rhs)) {
    categoryCounterInc("trees", "assign");
    _sanityCheck();
}

Send::Send(core::LocOffsets loc, unique_ptr<Expression> recv, core::NameRef fun, Send::ARGS_store args,
           unique_ptr<Block> block, Flags flags)
    : Expression(loc), fun(fun), flags(flags), recv(std::move(recv)), args(std::move(args)), block(std::move(block)) {
    categoryCounterInc("trees", "send");
    if (block) {
        counterInc("trees.send.with_block");
    }
    histogramInc("trees.send.args", this->args.size());
    _sanityCheck();
}

Cast::Cast(core::LocOffsets loc, core::TypePtr ty, unique_ptr<Expression> arg, core::NameRef cast)
    : Expression(loc), cast(cast), type(std::move(ty)), arg(std::move(arg)) {
    categoryCounterInc("trees", "cast");
    _sanityCheck();
}

ZSuperArgs::ZSuperArgs(core::LocOffsets loc) : Expression(loc) {
    categoryCounterInc("trees", "zsuper");
    _sanityCheck();
}

RestArg::RestArg(core::LocOffsets loc, unique_ptr<Reference> arg) : Reference(loc), expr(std::move(arg)) {
    categoryCounterInc("trees", "restarg");
    _sanityCheck();
}

KeywordArg::KeywordArg(core::LocOffsets loc, unique_ptr<Reference> expr) : Reference(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "keywordarg");
    _sanityCheck();
}

OptionalArg::OptionalArg(core::LocOffsets loc, unique_ptr<Reference> expr, unique_ptr<Expression> default_)
    : Reference(loc), expr(std::move(expr)), default_(std::move(default_)) {
    categoryCounterInc("trees", "optionalarg");
    _sanityCheck();
}

ShadowArg::ShadowArg(core::LocOffsets loc, unique_ptr<Reference> expr) : Reference(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "shadowarg");
    _sanityCheck();
}

BlockArg::BlockArg(core::LocOffsets loc, unique_ptr<Reference> expr) : Reference(loc), expr(std::move(expr)) {
    categoryCounterInc("trees", "blockarg");
    _sanityCheck();
}

Literal::Literal(core::LocOffsets loc, const core::TypePtr &value) : Expression(loc), value(std::move(value)) {
    categoryCounterInc("trees", "literal");
    _sanityCheck();
}

UnresolvedConstantLit::UnresolvedConstantLit(core::LocOffsets loc, unique_ptr<Expression> scope, core::NameRef cnst)
    : Expression(loc), cnst(cnst), scope(std::move(scope)) {
    categoryCounterInc("trees", "constantlit");
    _sanityCheck();
}

ConstantLit::ConstantLit(core::LocOffsets loc, core::SymbolRef symbol, unique_ptr<UnresolvedConstantLit> original)
    : Expression(loc), symbol(symbol), original(std::move(original)) {
    categoryCounterInc("trees", "resolvedconstantlit");
    _sanityCheck();
}

optional<pair<core::SymbolRef, vector<core::NameRef>>>
ConstantLit::fullUnresolvedPath(const core::GlobalState &gs) const {
    if (this->symbol != core::Symbols::StubModule()) {
        return nullopt;
    }
    ENFORCE(!this->resolutionScopes.empty());
    vector<core::NameRef> namesFailedToResolve;
    auto nested = this;
    {
        while (!nested->resolutionScopes.front().exists()) {
            namesFailedToResolve.emplace_back(nested->original->cnst);
            ENFORCE(ast::cast_tree<ast::ConstantLit>(nested->original->scope.get()));
            nested = ast::cast_tree<ast::ConstantLit>(nested->original->scope.get());
            ENFORCE(nested->symbol == core::Symbols::StubModule());
            ENFORCE(!nested->resolutionScopes.empty());
        }
        namesFailedToResolve.emplace_back(nested->original->cnst);
        absl::c_reverse(namesFailedToResolve);
    }
    auto prefix = nested->resolutionScopes.front();
    return make_pair(prefix, move(namesFailedToResolve));
}

Block::Block(core::LocOffsets loc, MethodDef::ARGS_store args, unique_ptr<Expression> body)
    : Expression(loc), args(std::move(args)), body(std::move(body)) {
    categoryCounterInc("trees", "block");
    _sanityCheck();
};

Hash::Hash(core::LocOffsets loc, ENTRY_store keys, ENTRY_store values)
    : Expression(loc), keys(std::move(keys)), values(std::move(values)) {
    categoryCounterInc("trees", "hash");
    histogramInc("trees.hash.entries", this->keys.size());
    _sanityCheck();
}

Array::Array(core::LocOffsets loc, ENTRY_store elems) : Expression(loc), elems(std::move(elems)) {
    categoryCounterInc("trees", "array");
    histogramInc("trees.array.elems", this->elems.size());
    _sanityCheck();
}

InsSeq::InsSeq(core::LocOffsets loc, STATS_store stats, unique_ptr<Expression> expr)
    : Expression(loc), stats(std::move(stats)), expr(std::move(expr)) {
    categoryCounterInc("trees", "insseq");
    histogramInc("trees.insseq.stats", this->stats.size());
    _sanityCheck();
}

EmptyTree::EmptyTree() : Expression(core::LocOffsets::none()) {
    categoryCounterInc("trees", "emptytree");
    _sanityCheck();
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
        buf << a->toStringWithTabs(gs, tabs + 1);
    }
};

template <class T> void printArgs(const core::GlobalState &gs, stringstream &buf, T &args, int tabs) {
    buf << "(";
    printElems(gs, buf, args, tabs);
    buf << ")";
}

string ClassDef::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    stringstream buf;
    if (kind == ClassDef::Kind::Module) {
        buf << "module ";
    } else {
        buf << "class ";
    }
    buf << name->toStringWithTabs(gs, tabs) << "<" << this->symbol.dataAllowingNone(gs)->name.data(gs)->toString(gs)
        << "> < ";
    printArgs(gs, buf, this->ancestors, tabs);

    for (auto &a : this->rhs) {
        buf << '\n';
        printTabs(buf, tabs + 1);
        buf << a->toStringWithTabs(gs, tabs + 1) << '\n';
    }

    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string ClassDef::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "kind = " << (kind == ClassDef::Kind::Module ? "module" : "class") << '\n';
    printTabs(buf, tabs + 1);
    buf << "name = " << name->showRaw(gs, tabs + 1) << "<"
        << this->symbol.dataAllowingNone(gs)->name.data(gs)->showRaw(gs) << ">" << '\n';
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
    buf << "]" << '\n';

    printTabs(buf, tabs + 1);
    buf << "rhs = [" << '\n';

    for (auto &a : this->rhs) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << '\n';
        if (&a != &this->rhs.back()) {
            buf << '\n';
        }
    }
    printTabs(buf, tabs + 1);
    buf << "]" << '\n';
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string InsSeq::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    stringstream buf;
    buf << "begin" << '\n';
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 1);
        buf << a->toStringWithTabs(gs, tabs + 1) << '\n';
    }

    printTabs(buf, tabs + 1);
    buf << expr->toStringWithTabs(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string InsSeq::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "stats = [" << '\n';
    for (auto &a : this->stats) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << '\n';
    }
    printTabs(buf, tabs + 1);
    buf << "]," << '\n';

    printTabs(buf, tabs + 1);
    buf << "expr = " << expr->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string MethodDef::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    stringstream buf;

    if (this->flags.isSelfMethod) {
        buf << "def self.";
    } else {
        buf << "def ";
    }
    buf << name.data(gs)->toString(gs);
    auto &data = this->symbol.dataAllowingNone(gs);
    if (name != data->name) {
        buf << "<" << data->name.data(gs)->toString(gs) << ">";
    }
    buf << "(";
    bool first = true;
    if (this->symbol == core::Symbols::todo()) {
        for (auto &a : this->args) {
            if (!first) {
                buf << ", ";
            }
            first = false;
            buf << a->toStringWithTabs(gs, tabs + 1);
        }
    } else {
        for (auto &a : data->arguments()) {
            if (!first) {
                buf << ", ";
            }
            first = false;
            buf << a.argumentName(gs);
        }
    }
    buf << ")" << '\n';
    printTabs(buf, tabs + 1);
    buf << this->rhs->toStringWithTabs(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string MethodDef::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);

    auto stringifiedFlags = vector<string>{};
    if (this->flags.isSelfMethod) {
        stringifiedFlags.emplace_back("self");
    }
    if (this->flags.isRewriterSynthesized) {
        stringifiedFlags.emplace_back("rewriter");
    }
    buf << fmt::format("flags = {{{}}}\n", fmt::join(stringifiedFlags, ", "));

    printTabs(buf, tabs + 1);
    buf << "name = " << name.data(gs)->showRaw(gs) << "<"
        << this->symbol.dataAllowingNone(gs)->name.data(gs)->showRaw(gs) << ">" << '\n';
    printTabs(buf, tabs + 1);
    buf << "args = [";
    bool first = true;
    if (this->symbol == core::Symbols::todo()) {
        for (auto &a : this->args) {
            if (!first) {
                buf << ", ";
            }
            first = false;
            buf << a->showRaw(gs, tabs + 2);
        }
    } else {
        for (auto &a : this->args) {
            if (!first) {
                buf << ", ";
            }
            first = false;
            buf << a->showRaw(gs, tabs + 2);
        }
    }
    buf << "]" << '\n';
    printTabs(buf, tabs + 1);
    buf << "rhs = " << this->rhs->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string If::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    stringstream buf;

    buf << "if " << this->cond->toStringWithTabs(gs, tabs + 1) << '\n';
    printTabs(buf, tabs + 1);
    buf << this->thenp->toStringWithTabs(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "else" << '\n';
    printTabs(buf, tabs + 1);
    buf << this->elsep->toStringWithTabs(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string If::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;

    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "cond = " << this->cond->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs + 1);
    buf << "thenp = " << this->thenp->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs + 1);
    buf << "elsep = " << this->elsep->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Assign::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;

    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "lhs = " << this->lhs->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs + 1);
    buf << "rhs = " << this->rhs->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string While::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    stringstream buf;

    buf << "while " << this->cond->toStringWithTabs(gs, tabs + 1) << '\n';
    printTabs(buf, tabs + 1);
    buf << this->body->toStringWithTabs(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string While::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;

    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "cond = " << this->cond->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs + 1);
    buf << "body = " << this->body->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string EmptyTree::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "<emptyTree>";
}

string UnresolvedConstantLit::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->scope->toStringWithTabs(gs, tabs) + "::" + this->cnst.data(gs)->toString(gs);
}

string UnresolvedConstantLit::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;

    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "scope = " << this->scope->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs + 1);
    buf << "cnst = " << this->cnst.data(gs)->showRaw(gs) << '\n';
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string ConstantLit::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    if (symbol.exists() && symbol != core::Symbols::StubModule()) {
        return this->symbol.dataAllowingNone(gs)->showFullName(gs);
    }
    return "Unresolved: " + this->original->toStringWithTabs(gs, tabs);
}

string ConstantLit::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;

    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "orig = " << (this->original ? this->original->showRaw(gs, tabs + 1) : "nullptr") << '\n';
    printTabs(buf, tabs + 1);
    buf << "symbol = " << this->symbol.dataAllowingNone(gs)->showFullName(gs) << '\n';
    if (!resolutionScopes.empty()) {
        printTabs(buf, tabs + 1);
        buf << "resolutionScopes = "
            << fmt::format("[{}]", fmt::map_join(this->resolutionScopes.begin(), this->resolutionScopes.end(), ", ",
                                                 [&](auto sym) { return sym.data(gs)->showFullName(gs); }))
            << '\n';
    }
    printTabs(buf, tabs);

    buf << "}";
    return buf.str();
}

string Local::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->localVariable.toString(gs);
}

string Local::nodeName() {
    return "Local";
}

bool Expression::isSelfReference() const {
    auto asLocal = cast_tree_const<Local>(this);
    return asLocal && asLocal->localVariable == core::LocalVariable::selfVariable();
}

string Local::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "localVariable = " << this->localVariable.showRaw(gs) << '\n';
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string UnresolvedIdent::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->name.toString(gs);
}

string UnresolvedIdent::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "kind = ";
    switch (this->kind) {
        case Kind::Local:
            buf << "Local";
            break;
        case Kind::Instance:
            buf << "Instance";
            break;
        case Kind::Class:
            buf << "Class";
            break;
        case Kind::Global:
            buf << "Global";
            break;
    }
    buf << '\n';
    printTabs(buf, tabs + 1);
    buf << "name = " << this->name.showRaw(gs) << '\n';
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string Return::showRaw(const core::GlobalState &gs, int tabs) {
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

string Return::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "return " + this->expr->toStringWithTabs(gs, tabs + 1);
}

string Next::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "next(" + this->expr->toStringWithTabs(gs, tabs + 1) + ")";
}

string Break::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "break(" + this->expr->toStringWithTabs(gs, tabs + 1) + ")";
}

string Retry::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "retry";
}

string Literal::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ value = " + this->toStringWithTabs(gs, 0) + " }";
}

string Literal::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    string res;
    typecase(
        this->value.get(), [&](core::LiteralType *l) { res = l->showValue(gs); },
        [&](core::ClassType *l) {
            if (l->symbol == core::Symbols::NilClass()) {
                res = "nil";
            } else if (l->symbol == core::Symbols::FalseClass()) {
                res = "false";
            } else if (l->symbol == core::Symbols::TrueClass()) {
                res = "true";
            } else {
                res = "literal(" + this->value->toStringWithTabs(gs, tabs) + ")";
            }
        },
        [&](core::Type *t) { res = "literal(" + this->value->toStringWithTabs(gs, tabs) + ")"; });
    return res;
}

string Assign::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->lhs->toStringWithTabs(gs, tabs) + " = " + this->rhs->toStringWithTabs(gs, tabs);
}

string RescueCase::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
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
        buf << exception->toStringWithTabs(gs, tabs);
    }
    buf << " => " << this->var->toStringWithTabs(gs, tabs);
    buf << '\n';
    printTabs(buf, tabs);
    buf << this->body->toStringWithTabs(gs, tabs);
    return buf.str();
}

string RescueCase::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "exceptions = [" << '\n';
    for (auto &a : exceptions) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << '\n';
    }
    printTabs(buf, tabs + 1);
    buf << "]" << '\n';
    printTabs(buf, tabs + 1);
    buf << "var = " << this->var->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs + 1);
    buf << "body = " << this->body->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Rescue::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    stringstream buf;
    buf << this->body->toStringWithTabs(gs, tabs);
    for (auto &rescueCase : this->rescueCases) {
        buf << '\n';
        printTabs(buf, tabs - 1);
        buf << rescueCase->toStringWithTabs(gs, tabs);
    }
    if (cast_tree<EmptyTree>(this->else_.get()) == nullptr) {
        buf << '\n';
        printTabs(buf, tabs - 1);
        buf << "else" << '\n';
        printTabs(buf, tabs);
        buf << this->else_->toStringWithTabs(gs, tabs);
    }
    if (cast_tree<EmptyTree>(this->ensure.get()) == nullptr) {
        buf << '\n';
        printTabs(buf, tabs - 1);
        buf << "ensure" << '\n';
        printTabs(buf, tabs);
        buf << this->ensure->toStringWithTabs(gs, tabs);
    }
    return buf.str();
}

string Rescue::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "body = " << this->body->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs + 1);
    buf << "rescueCases = [" << '\n';
    for (auto &a : rescueCases) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << '\n';
    }
    printTabs(buf, tabs + 1);
    buf << "]" << '\n';
    printTabs(buf, tabs + 1);
    buf << "else = " << this->else_->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs + 1);
    buf << "ensure = " << this->ensure->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string Send::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    stringstream buf;
    buf << this->recv->toStringWithTabs(gs, tabs) << "." << this->fun.data(gs)->toString(gs);
    printArgs(gs, buf, this->args, tabs);
    if (this->block != nullptr) {
        buf << this->block->toStringWithTabs(gs, tabs);
    }

    return buf.str();
}

string Send::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "recv = " << this->recv->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs + 1);
    buf << "fun = " << this->fun.data(gs)->showRaw(gs) << '\n';
    printTabs(buf, tabs + 1);
    buf << "block = ";
    if (this->block) {
        buf << this->block->showRaw(gs, tabs + 1) << '\n';
    } else {
        buf << "nullptr" << '\n';
    }
    printTabs(buf, tabs + 1);
    buf << "args = [" << '\n';
    for (auto &a : args) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << '\n';
    }
    printTabs(buf, tabs + 1);
    buf << "]" << '\n';
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string Cast::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    stringstream buf;
    buf << "T." << this->cast.toString(gs);
    buf << "(" << this->arg->toStringWithTabs(gs, tabs) << ", " << this->type->toStringWithTabs(gs, tabs) << ")";

    return buf.str();
}

string Cast::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 2);
    buf << "cast = " << this->cast.showRaw(gs) << "," << '\n';
    printTabs(buf, tabs + 2);
    buf << "arg = " << this->arg->showRaw(gs, tabs + 2) << '\n';
    printTabs(buf, tabs + 2);
    buf << "type = " << this->type->toString(gs) << "," << '\n';
    printTabs(buf, tabs);
    buf << "}" << '\n';

    return buf.str();
}

string ZSuperArgs::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ }";
}

string Hash::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "pairs = [" << '\n';
    int i = -1;
    for (auto &key : keys) {
        i++;
        auto &value = values[i];

        printTabs(buf, tabs + 2);
        buf << "[" << '\n';
        printTabs(buf, tabs + 3);
        buf << "key = " << key->showRaw(gs, tabs + 3) << '\n';
        printTabs(buf, tabs + 3);
        buf << "value = " << value->showRaw(gs, tabs + 3) << '\n';
        printTabs(buf, tabs + 2);
        buf << "]" << '\n';
    }
    printTabs(buf, tabs + 1);
    buf << "]" << '\n';
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string Array::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "elems = [" << '\n';
    for (auto &a : elems) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << '\n';
    }
    printTabs(buf, tabs + 1);
    buf << "]" << '\n';
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
}

string ZSuperArgs::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "ZSuperArgs";
}

string Hash::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
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
        buf << key->toStringWithTabs(gs, tabs + 1);
        buf << " => ";
        buf << value->toStringWithTabs(gs, tabs + 1);
    }
    buf << "}";
    return buf.str();
}

string Array::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    stringstream buf;
    buf << "[";
    printElems(gs, buf, this->elems, tabs);
    buf << "]";
    return buf.str();
}

string Block::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    stringstream buf;
    buf << " do |";
    printElems(gs, buf, this->args, tabs + 1);
    buf << "|" << '\n';
    printTabs(buf, tabs + 1);
    buf << this->body->toStringWithTabs(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "end";
    return buf.str();
}

string Block::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << " {" << '\n';
    printTabs(buf, tabs + 1);
    buf << "args = [" << '\n';
    for (auto &a : this->args) {
        printTabs(buf, tabs + 2);
        buf << a->showRaw(gs, tabs + 2) << '\n';
    }
    printTabs(buf, tabs + 1);
    buf << "]" << '\n';
    printTabs(buf, tabs + 1);
    buf << "body = " << this->body->showRaw(gs, tabs + 1) << '\n';
    printTabs(buf, tabs);
    buf << "}";
    return buf.str();
}

string RestArg::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "*" + this->expr->toStringWithTabs(gs, tabs);
}

string KeywordArg::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->expr->toStringWithTabs(gs, tabs) + ":";
}

string OptionalArg::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    stringstream buf;
    buf << this->expr->toStringWithTabs(gs, tabs);
    if (this->default_) {
        buf << " = " << this->default_->toStringWithTabs(gs, tabs);
    }
    return buf.str();
}

string ShadowArg::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return this->expr->toStringWithTabs(gs, tabs);
}

string BlockArg::toStringWithTabs(const core::GlobalState &gs, int tabs) const {
    return "&" + this->expr->toStringWithTabs(gs, tabs);
}

string RescueCase::nodeName() {
    return "RescueCase";
}
string Rescue::nodeName() {
    return "Rescue";
}
string Next::nodeName() {
    return "Next";
}
string ClassDef::nodeName() {
    return "ClassDef";
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

string Literal::nodeName() {
    return "Literal";
}

core::NameRef Literal::asString(const core::GlobalState &gs) const {
    ENFORCE(isString(gs));
    auto t = core::cast_type<core::LiteralType>(value.get());
    core::NameRef res(gs, t->value);
    return res;
}

core::NameRef Literal::asSymbol(const core::GlobalState &gs) const {
    ENFORCE(isSymbol(gs));
    auto t = core::cast_type<core::LiteralType>(value.get());
    core::NameRef res(gs, t->value);
    return res;
}

bool Literal::isSymbol(const core::GlobalState &gs) const {
    auto t = core::cast_type<core::LiteralType>(value.get());
    return t && t->derivesFrom(gs, core::Symbols::Symbol());
}

bool Literal::isNil(const core::GlobalState &gs) const {
    return value->derivesFrom(gs, core::Symbols::NilClass());
}

bool Literal::isString(const core::GlobalState &gs) const {
    auto t = core::cast_type<core::LiteralType>(value.get());
    return t && t->derivesFrom(gs, core::Symbols::String());
}

bool Literal::isTrue(const core::GlobalState &gs) const {
    return value->derivesFrom(gs, core::Symbols::TrueClass());
}

bool Literal::isFalse(const core::GlobalState &gs) const {
    return value->derivesFrom(gs, core::Symbols::FalseClass());
}

string UnresolvedConstantLit::nodeName() {
    return "UnresolvedConstantLit";
}

string ConstantLit::nodeName() {
    return "ConstantLit";
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

string KeywordArg::showRaw(const core::GlobalState &gs, int tabs) {
    return nodeName() + "{ expr = " + expr->showRaw(gs, tabs) + " }";
}

string KeywordArg::nodeName() {
    return "KeywordArg";
}

string OptionalArg::showRaw(const core::GlobalState &gs, int tabs) {
    stringstream buf;
    buf << nodeName() << "{" << '\n';
    printTabs(buf, tabs + 1);
    buf << "expr = " + expr->showRaw(gs, tabs + 1) << '\n';
    if (default_) {
        printTabs(buf, tabs + 1);
        buf << "default_ = " + default_->showRaw(gs, tabs + 1) << '\n';
    }
    printTabs(buf, tabs);
    buf << "}";

    return buf.str();
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

ParsedFilesOrCancelled::ParsedFilesOrCancelled() : trees(nullopt){};
ParsedFilesOrCancelled::ParsedFilesOrCancelled(std::vector<ParsedFile> &&trees) : trees(move(trees)) {}

bool ParsedFilesOrCancelled::hasResult() const {
    return trees.has_value();
}

vector<ParsedFile> &ParsedFilesOrCancelled::result() {
    if (trees.has_value()) {
        return trees.value();
    }
    Exception::raise("Attempted to retrieve result of an AST pass that did not complete.");
}

} // namespace sorbet::ast
