
#include "CFG.h"
#include <algorithm>
#include <sstream>

// helps debugging
template class std::unique_ptr<ruby_typer::cfg::CFG>;

using namespace std;

namespace ruby_typer {
namespace cfg {

unique_ptr<CFG> CFG::buildFor(ast::Context ctx, ast::MethodDef &md) {
    unique_ptr<CFG> res(new CFG); // private constructor
    res->symbol = md.symbol;
    auto retSym = ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::returnMethodTemp(), md.symbol);
    auto cont = res->walk(ctx, md.rhs.get(), res->entry(), *res.get(), retSym);
    auto retSym1 = ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::returnMethodTemp(), md.symbol);

    cont->exprs.emplace_back(retSym1, make_unique<Return>(retSym)); // dead assign.
    cont->bexit.cond = ast::ContextBase::defn_cfg_never();
    cont->bexit.thenb = res->deadBlock();
    cont->bexit.elseb = res->deadBlock();
    return res;
}

BasicBlock *CFG::freshBlock() {
    this->basicBlocks.emplace_back(new BasicBlock());
    return this->basicBlocks.back().get();
}

CFG::CFG() {
    freshBlock(); // entry;
    freshBlock(); // dead code;
    deadBlock()->bexit.elseb = deadBlock();
    deadBlock()->bexit.thenb = deadBlock();
    deadBlock()->bexit.cond = ast::ContextBase::defn_cfg_never();
}

void conditionalJump(BasicBlock *from, ast::SymbolRef cond, BasicBlock *thenb, BasicBlock *elseb, CFG &inWhat) {
    if (from != inWhat.deadBlock()) {
        Error::check(!from->bexit.cond.exists());
        from->bexit.cond = cond;
        from->bexit.thenb = thenb;
        from->bexit.elseb = elseb;
        thenb->backEdges.push_back(from);
        elseb->backEdges.push_back(from);
    }
}

void unconditionalJump(BasicBlock *from, BasicBlock *to, CFG &inWhat) {
    if (from != inWhat.deadBlock()) {
        Error::check(!from->bexit.cond.exists());
        from->bexit.cond = ast::ContextBase::defn_cfg_always();
        from->bexit.elseb = to;
        from->bexit.thenb = to;
        to->backEdges.push_back(from);
    }
}

void jumpToDead(BasicBlock *from, CFG &inWhat) {
    auto *db = inWhat.deadBlock();
    if (from != db) {
        Error::check(!from->bexit.cond.exists());
        from->bexit.cond = ast::ContextBase::defn_cfg_never();
        from->bexit.elseb = db;
        from->bexit.thenb = db;
    }
}

/** Convert `what` into a cfg, by starting to evaluate it in `current` inside method defined by `inWhat`.
 * store result of evaluation into `target`. Returns basic block in which evaluation should proceed.
 */
BasicBlock *CFG::walk(ast::Context ctx, ast::Statement *what, BasicBlock *current, CFG &inWhat, ast::SymbolRef target) {
    /** Try to pay additional attention not to duplicate any part of tree.
     * Though this may lead to more effictient and a better CFG if it was to be actually compiled into code
     * This will lead to duplicate typechecking and may lead to exponential explosion of typechecking time
     * for some code snippets. */
    BasicBlock *ret = nullptr;
    typecase(
        what,
        [&](ast::While *a) {
            auto headerBlock = inWhat.freshBlock();
            unconditionalJump(current, headerBlock, inWhat);

            auto condSym = ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::whileTemp(), inWhat.symbol);
            auto headerEnd = walk(ctx, a->cond.get(), headerBlock, inWhat, condSym);
            auto bodyBlock = inWhat.freshBlock();
            auto continueBlock = inWhat.freshBlock();
            conditionalJump(headerEnd, condSym, bodyBlock, continueBlock, inWhat);
            // finishHeader
            auto bodySym = ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::statTemp(), inWhat.symbol);

            auto body = walk(ctx, a->body.get(), bodyBlock, inWhat, bodySym);
            unconditionalJump(body, headerBlock, inWhat);

            continueBlock->exprs.emplace_back(target, make_unique<Nil>());
            ret = continueBlock;
        },
        [&](ast::Return *a) {
            auto retSym = ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::returnTemp(), inWhat.symbol);
            auto cont = walk(ctx, a->expr.get(), current, inWhat, retSym);
            cont->exprs.emplace_back(target, make_unique<Return>(retSym)); // dead assign.
            jumpToDead(cont, inWhat);
            ret = deadBlock();
        },
        [&](ast::If *a) {
            auto ifSym = ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::ifTemp(), inWhat.symbol);
            Error::check(ifSym.exists());
            auto thenBlock = inWhat.freshBlock();
            auto elseBlock = inWhat.freshBlock();
            auto cont = walk(ctx, a->cond.get(), current, inWhat, ifSym);
            conditionalJump(cont, ifSym, thenBlock, elseBlock, inWhat);

            auto thenEnd = walk(ctx, a->thenp.get(), thenBlock, inWhat, target);
            auto elseEnd = walk(ctx, a->elsep.get(), elseBlock, inWhat, target);
            if (thenEnd != deadBlock() || elseEnd != deadBlock()) {
                if (thenEnd == deadBlock()) {
                    ret = elseEnd;
                } else if (thenEnd == deadBlock()) {
                    ret = thenEnd;
                } else {
                    ret = freshBlock();
                    unconditionalJump(thenEnd, ret, inWhat);
                    unconditionalJump(elseEnd, ret, inWhat);
                }
            } else {
                ret = deadBlock();
            }
        },
        [&](ast::IntLit *a) {
            current->exprs.emplace_back(target, make_unique<IntLit>(a->value));
            ret = current;
        },
        [&](ast::FloatLit *a) {
            current->exprs.emplace_back(target, make_unique<FloatLit>(a->value));
            ret = current;
        },
        [&](ast::StringLit *a) {
            current->exprs.emplace_back(target, make_unique<StringLit>(a->value));
            ret = current;
        },
        [&](ast::BoolLit *a) {
            current->exprs.emplace_back(target, make_unique<BoolLit>(a->value));
            ret = current;
        },
        [&](ast::ConstantLit *a) {
            current->exprs.emplace_back(target, make_unique<ConstantLit>(a->cnst));
            ret = current;
        },
        [&](ast::Ident *a) {
            current->exprs.emplace_back(target, make_unique<Ident>(a->symbol));
            ret = current;
        },
        [&](ast::Self *a) {
            current->exprs.emplace_back(target, make_unique<Self>(a->claz));
            ret = current;
        },
        [&](ast::Assign *a) {
            auto lhsIdent = dynamic_cast<ast::Ident *>(a->lhs.get());
            Error::check(lhsIdent != nullptr);
            auto rhsCont = walk(ctx, a->rhs.get(), current, inWhat, lhsIdent->symbol);
            rhsCont->exprs.emplace_back(target, make_unique<Ident>(lhsIdent->symbol));
            ret = rhsCont;
        },
        [&](ast::InsSeq *a) {
            for (auto &exp : a->stats) {
                auto temp = ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::statTemp(), inWhat.symbol);
                current = walk(ctx, exp.get(), current, inWhat, temp);
            }
            ret = walk(ctx, a->expr.get(), current, inWhat, target);
        },
        [&](ast::Statement *n) {
            current->exprs.emplace_back(target, make_unique<NotSupported>(""));
            ret = current;
        });
    /*[&](ast::Break *a) {}, [&](ast::Next *a) {}, [&](ast::Block *a) {},*/
    //[&](ast::Send *a) {});
    // For, Next, Rescue,
    // Symbol, Send, New, Super, NamedArg, Hash, Array,
    // ArraySplat, HashAplat, Block,
    Error::check(ret != nullptr);
    return ret;
}

string CFG::toString(ast::Context ctx) {
    stringstream buf;
    buf << "digraph " << this->symbol.info(ctx).name.name(ctx).toString(ctx) << " {" << endl;
    buf << "    bb0 [shape = invhouse];" << endl;
    buf << "    bb1 [shape = parallelogram];" << endl << endl;
    for (int i = 0; i < this->basicBlocks.size(); i++) {
        auto text = this->basicBlocks[i]->toString(ctx);
        buf << "    bb" << i << " [label = \"" << text << "\"];" << endl;
        auto thenI = find_if(this->basicBlocks.begin(), this->basicBlocks.end(),
                             [&](auto &a) { return a.get() == this->basicBlocks[i]->bexit.thenb; });
        auto elseI = find_if(this->basicBlocks.begin(), this->basicBlocks.end(),
                             [&](auto &a) { return a.get() == this->basicBlocks[i]->bexit.elseb; });
        buf << "    bb" << i << " -> bb" << thenI - this->basicBlocks.begin() << ";" << endl;
        if (this->basicBlocks[i]->bexit.cond != ctx.state.defn_cfg_always() &&
            this->basicBlocks[i]->bexit.cond != ctx.state.defn_cfg_never()) {
            buf << "    bb" << i << " -> bb" << elseI - this->basicBlocks.begin() << ";" << endl << endl;
        }
    }
    buf << "}";
    return buf.str();
}

string BasicBlock::toString(ast::Context ctx) {
    stringstream buf;
    for (auto &exp : this->exprs) {
        buf << exp.bind.info(ctx).name.name(ctx).toString(ctx) << " = " << exp.value->toString(ctx);
        buf << "\\n"; // intentional! graphviz will do interpolation.
    }
    buf << this->bexit.cond.info(ctx).name.name(ctx).toString(ctx);
    return buf.str();
}

Binding::Binding(const ast::SymbolRef &bind, unique_ptr<Instruction> value) : bind(bind), value(move(value)) {}

Return::Return(const ast::SymbolRef &what) : what(what) {}

string Return::toString(ast::Context ctx) {
    return "return " + this->what.info(ctx).name.name(ctx).toString(ctx);
}

New::New(const ast::SymbolRef &claz, vector<ast::SymbolRef> &args) : klass(klass), args(move(args)) {}

string New::toString(ast::Context ctx) {
    stringstream buf;
    buf << "new " << this->klass.info(ctx).name.name(ctx).toString(ctx) << "(";
    bool isFirst = true;
    for (auto arg : this->args) {
        if (!isFirst) {
            buf << " ,";
        }
        isFirst = true;
        buf << arg.info(ctx).name.name(ctx).toString(ctx);
    }
    buf << ")";
    return buf.str();
}

Super::Super(vector<ast::SymbolRef> &args) : args(move(args)) {}

string Super::toString(ast::Context ctx) {
    stringstream buf;
    buf << "super(";
    bool isFirst = true;
    for (auto arg : this->args) {
        if (!isFirst) {
            buf << " ,";
        }
        isFirst = true;
        buf << arg.info(ctx).name.name(ctx).toString(ctx);
    }
    buf << ")";
    return buf.str();
}

FloatLit::FloatLit(float value) : value(value) {}

string FloatLit::toString(ast::Context ctx) {
    return to_string(this->value);
}

IntLit::IntLit(int value) : value(value) {}

string IntLit::toString(ast::Context ctx) {
    return to_string(this->value);
}

Ident::Ident(const ast::SymbolRef &what) : what(what) {}

string Ident::toString(ast::Context ctx) {
    return this->what.info(ctx).name.name(ctx).toString(ctx);
}

string Send::toString(ast::Context ctx) {
    stringstream buf;
    buf << this->recv.info(ctx).name.name(ctx).toString(ctx) << "." << this->fun.name(ctx).toString(ctx) << "(";
    bool isFirst = true;
    for (auto arg : this->args) {
        if (!isFirst) {
            buf << " ,";
        }
        isFirst = true;
        buf << arg.info(ctx).name.name(ctx).toString(ctx);
    }
    buf << ")";
    return buf.str();
}

string StringLit::toString(ast::Context ctx) {
    return this->value.name(ctx).toString(ctx);
}

string BoolLit::toString(ast::Context ctx) {
    if (value) {
        return "true";
    } else {
        return "false";
    }
}

string ConstantLit::toString(ast::Context ctx) {
    return this->cnst.name(ctx).toString(ctx);
}

string Nil::toString(ast::Context ctx) {
    return "nil";
}

string Self::toString(ast::Context ctx) {
    return "self";
}

string NotSupported::toString(ast::Context ctx) {
    return "NotSupported(" + why + ")";
}
} // namespace cfg
} // namespace ruby_typer