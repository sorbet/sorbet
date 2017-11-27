
#include "builder.h"

#include <algorithm>
#include <algorithm> // sort
#include <climits>   // INT_MAX
#include <sstream>
#include <unordered_map>

using namespace std;

namespace ruby_typer {
namespace cfg {

void conditionalJump(BasicBlock *from, core::LocalVariable cond, BasicBlock *thenb, BasicBlock *elseb, CFG &inWhat) {
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
        from->bexit.cond = core::NameRef(0);
        from->bexit.elseb = to;
        from->bexit.thenb = to;
        to->backEdges.push_back(from);
    }
}

void jumpToDead(BasicBlock *from, CFG &inWhat) {
    auto *db = inWhat.deadBlock();
    if (from != db) {
        Error::check(!from->bexit.cond.exists());
        from->bexit.cond = core::NameRef(0);
        from->bexit.elseb = db;
        from->bexit.thenb = db;
        db->backEdges.push_back(from);
    }
}

core::LocalVariable global2Local(core::Context ctx, core::SymbolRef what, CFG &inWhat,
                                 std::unordered_map<core::SymbolRef, core::LocalVariable> &aliases) {
    core::LocalVariable &alias = aliases[what];
    if (!alias.exists()) {
        core::Symbol &info = what.info(ctx);
        alias = ctx.state.newTemporary(core::UniqueNameKind::CFG, info.name, inWhat.symbol);
    }
    return alias;
}

/** Convert `what` into a cfg, by starting to evaluate it in `current` inside method defined by `inWhat`.
 * store result of evaluation into `target`. Returns basic block in which evaluation should proceed.
 */
BasicBlock *CFGBuilder::walk(CFGContext cctx, CFG &cfg, ast::Expression *what, BasicBlock *current) {
    /** Try to pay additional attention not to duplicate any part of tree.
     * Though this may lead to more effictient and a better CFG if it was to be actually compiled into code
     * This will lead to duplicate typechecking and may lead to exponential explosion of typechecking time
     * for some code snippets. */
    Error::check(!current->bexit.cond.exists());

    try {
        BasicBlock *ret = nullptr;
        typecase(
            what,
            [&](ast::While *a) {
                auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1, current);
                unconditionalJump(current, headerBlock, cctx.inWhat);

                core::LocalVariable condSym = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                          core::Names::whileTemp(), cctx.inWhat.symbol);
                auto headerEnd = walk(cctx.withTarget(condSym).withScope(headerBlock), cfg, a->cond.get(), headerBlock);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1, headerEnd);
                auto continueBlock = cctx.inWhat.freshBlock(cctx.loops, headerEnd);
                conditionalJump(headerEnd, condSym, bodyBlock, continueBlock, cctx.inWhat);
                // finishHeader
                core::LocalVariable bodySym =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::statTemp(), cctx.inWhat.symbol);

                auto body = walk(cctx.withTarget(bodySym).withScope(headerBlock), cfg, a->body.get(), bodyBlock);
                unconditionalJump(body, headerBlock, cctx.inWhat);

                continueBlock->exprs.emplace_back(cctx.target, a->loc, make_unique<Nil>());
                ret = continueBlock;
            },
            [&](ast::Return *a) {
                core::LocalVariable retSym = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                         core::Names::returnTemp(), cctx.inWhat.symbol);
                auto cont = walk(cctx.withTarget(retSym), cfg, a->expr.get(), current);
                cont->exprs.emplace_back(cctx.target, a->loc, make_unique<Return>(retSym)); // dead assign.
                jumpToDead(cont, cctx.inWhat);
                ret = cfg.deadBlock();
            },
            [&](ast::If *a) {
                core::LocalVariable ifSym =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::ifTemp(), cctx.inWhat.symbol);
                Error::check(ifSym.exists());
                auto cont = walk(cctx.withTarget(ifSym), cfg, a->cond.get(), current);
                auto thenBlock = cctx.inWhat.freshBlock(cctx.loops, cont);
                auto elseBlock = cctx.inWhat.freshBlock(cctx.loops, cont);
                conditionalJump(cont, ifSym, thenBlock, elseBlock, cctx.inWhat);

                auto thenEnd = walk(cctx, cfg, a->thenp.get(), thenBlock);
                auto elseEnd = walk(cctx, cfg, a->elsep.get(), elseBlock);
                if (thenEnd != cfg.deadBlock() || elseEnd != cfg.deadBlock()) {
                    if (thenEnd == cfg.deadBlock()) {
                        ret = elseEnd;
                    } else if (thenEnd == cfg.deadBlock()) {
                        ret = thenEnd;
                    } else {
                        ret = cfg.freshBlock(cctx.loops, thenEnd); // could be elseEnd
                        unconditionalJump(thenEnd, ret, cctx.inWhat);
                        unconditionalJump(elseEnd, ret, cctx.inWhat);
                    }
                } else {
                    ret = cfg.deadBlock();
                }
            },
            [&](ast::IntLit *a) {
                current->exprs.emplace_back(cctx.target, a->loc, make_unique<IntLit>(a->value));
                ret = current;
            },
            [&](ast::FloatLit *a) {
                current->exprs.emplace_back(cctx.target, a->loc, make_unique<FloatLit>(a->value));
                ret = current;
            },
            [&](ast::StringLit *a) {
                current->exprs.emplace_back(cctx.target, a->loc, make_unique<StringLit>(a->value));
                ret = current;
            },
            [&](ast::SymbolLit *a) {
                current->exprs.emplace_back(cctx.target, a->loc, make_unique<SymbolLit>(a->name));
                ret = current;
            },
            [&](ast::BoolLit *a) {
                current->exprs.emplace_back(cctx.target, a->loc, make_unique<BoolLit>(a->value));
                ret = current;
            },
            [&](ast::ConstantLit *a) { Error::raise("Should have been eliminated by namer/resolver"); },
            [&](ast::Ident *a) {
                current->exprs.emplace_back(
                    cctx.target, a->loc,
                    make_unique<Ident>(global2Local(cctx.ctx, a->symbol, cctx.inWhat, cctx.aliases)));
                ret = current;
            },
            [&](ast::Local *a) {
                current->exprs.emplace_back(cctx.target, a->loc, make_unique<Ident>(a->localVariable));
                ret = current;
            },
            [&](ast::Self *a) {
                current->exprs.emplace_back(cctx.target, a->loc, make_unique<Self>(a->claz));
                ret = current;
            },
            [&](ast::Assign *a) {
                auto lhsIdent = ast::cast_tree<ast::Ident>(a->lhs.get());
                core::LocalVariable lhs;
                if (lhsIdent != nullptr) {
                    lhs = global2Local(cctx.ctx, lhsIdent->symbol, cctx.inWhat, cctx.aliases);
                } else if (auto lhsLocal = ast::cast_tree<ast::Local>(a->lhs.get())) {
                    lhs = lhsLocal->localVariable;
                } else {
                    // TODO(nelhage): Once namer is complete this should be a
                    // fatal error
                    // lhs = cctx.ctx.state.defn_todo();
                    Error::check(false, "should never be reached");
                }
                auto rhsCont = walk(cctx.withTarget(lhs), cfg, a->rhs.get(), current);
                rhsCont->exprs.emplace_back(cctx.target, a->loc, make_unique<Ident>(lhs));
                ret = rhsCont;
            },
            [&](ast::InsSeq *a) {
                for (auto &exp : a->stats) {
                    core::LocalVariable temp = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                           core::Names::statTemp(), cctx.inWhat.symbol);
                    current = walk(cctx.withTarget(temp), cfg, exp.get(), current);
                }
                ret = walk(cctx, cfg, a->expr.get(), current);
            },
            [&](ast::Send *s) {
                core::LocalVariable recv;

                recv =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::statTemp(), cctx.inWhat.symbol);
                current = walk(cctx.withTarget(recv), cfg, s->recv.get(), current);

                vector<core::LocalVariable> args;
                for (auto &exp : s->args) {
                    core::LocalVariable temp;
                    temp = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::statTemp(),
                                                       cctx.inWhat.symbol);
                    current = walk(cctx.withTarget(temp), cfg, exp.get(), current);

                    args.push_back(temp);
                }

                if (s->block != nullptr) {
                    auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1, current);
                    auto postBlock = cctx.inWhat.freshBlock(cctx.loops, headerBlock);
                    auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1, headerBlock);
                    core::SymbolRef sym = s->block->symbol;
                    core::Symbol &info = sym.info(cctx.ctx);

                    for (int i = 0; i < info.argumentsOrMixins.size(); ++i) {
                        auto &arg = s->block->args[i];

                        if (auto id = ast::cast_tree<ast::Local>(arg.get())) {
                            core::LocalVariable argLoc = id->localVariable;
                            cctx.aliases[info.argumentsOrMixins[i]] = argLoc;
                            bodyBlock->exprs.emplace_back(argLoc, arg->loc, make_unique<LoadArg>(recv, s->fun, i));
                        } else {
                            Error::check(false, "Should have been removed by namer");
                        }
                    }

                    conditionalJump(headerBlock, core::LocalVariable(core::Names::blockCall()), bodyBlock, postBlock,
                                    cctx.inWhat);

                    unconditionalJump(current, headerBlock, cctx.inWhat);

                    // TODO: handle block arguments somehow??
                    core::LocalVariable blockrv = cctx.ctx.state.newTemporary(
                        core::UniqueNameKind::CFG, core::Names::blockReturnTemp(), cctx.inWhat.symbol);
                    auto blockLast =
                        walk(cctx.withTarget(blockrv).withScope(headerBlock), cfg, s->block->body.get(), bodyBlock);

                    unconditionalJump(blockLast, headerBlock, cctx.inWhat);

                    current = postBlock;
                }

                current->exprs.emplace_back(cctx.target, s->loc, make_unique<Send>(recv, s->fun, args));
                ret = current;
            },

            [&](ast::Block *a) { Error::raise("should never encounter a bare Block"); },

            [&](ast::Next *a) {
                core::LocalVariable nextSym = cctx.ctx.state.newTemporary(
                    core::UniqueNameKind::CFG, core::Names::returnTemp(), cctx.inWhat.symbol);
                auto afterNext = walk(cctx.withTarget(nextSym), cfg, a->expr.get(), current);

                if (cctx.scope == nullptr) {
                    cctx.ctx.state.errors.error(a->loc, core::ErrorClass::NoNextScope, "No `do` block around `next`");
                    // I guess just keep going into deadcode?
                    unconditionalJump(afterNext, cfg.deadBlock(), cctx.inWhat);
                } else {
                    unconditionalJump(afterNext, cctx.scope, cctx.inWhat);
                }

                ret = cfg.deadBlock();
            },

            [&](ast::Rescue *a) {
                core::LocalVariable unanalyzableCondition = cctx.ctx.state.newTemporary(
                    core::UniqueNameKind::CFG, core::Names::rescueTemp(), cctx.inWhat.symbol);
                ret = current;

                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops, current);
                auto rescueHandlersBlock = cctx.inWhat.freshBlock(cctx.loops, current);
                conditionalJump(current, unanalyzableCondition, bodyBlock, rescueHandlersBlock, cctx.inWhat);
                bodyBlock = walk(cctx, cfg, a->body.get(), bodyBlock);

                ret = cfg.freshBlock(cctx.loops, bodyBlock);
                unconditionalJump(bodyBlock, ret, cctx.inWhat);

                for (auto &rescueCase : a->rescueCases) {
                    bodyBlock = cctx.inWhat.freshBlock(cctx.loops, rescueHandlersBlock);
                    auto newRescueHandlersBlock = cctx.inWhat.freshBlock(cctx.loops, rescueHandlersBlock);
                    conditionalJump(rescueHandlersBlock, unanalyzableCondition, bodyBlock, newRescueHandlersBlock,
                                    cctx.inWhat);
                    rescueHandlersBlock = newRescueHandlersBlock;

                    // TODO somehow make the result of this an OR type of all the rescueCase->exceptions
                    // TODO if rescueCase->var is a ast::Send, pass in something of type rescueCase->exceptions
                    bodyBlock = walk(cctx, cfg, rescueCase->var.get(), bodyBlock);
                    bodyBlock = walk(cctx, cfg, rescueCase->body.get(), bodyBlock);
                    unconditionalJump(bodyBlock, ret, cctx.inWhat);
                }

                bodyBlock = cctx.inWhat.freshBlock(cctx.loops, rescueHandlersBlock);
                conditionalJump(rescueHandlersBlock, unanalyzableCondition, bodyBlock, ret, cctx.inWhat);
                bodyBlock = walk(cctx, cfg, a->else_.get(), bodyBlock);
                unconditionalJump(bodyBlock, ret, cctx.inWhat);
            },

            [&](ast::Hash *h) {
                vector<core::LocalVariable> vars;
                for (int i = 0; i < h->keys.size(); i++) {
                    core::LocalVariable keyTmp = cctx.ctx.state.newTemporary(
                        core::UniqueNameKind::CFG, core::Names::hashTemp(), cctx.inWhat.symbol);
                    core::LocalVariable valTmp = cctx.ctx.state.newTemporary(
                        core::UniqueNameKind::CFG, core::Names::hashTemp(), cctx.inWhat.symbol);
                    current = walk(cctx.withTarget(keyTmp), cfg, h->keys[i].get(), current);
                    current = walk(cctx.withTarget(valTmp), cfg, h->values[i].get(), current);
                    vars.push_back(keyTmp);
                    vars.push_back(valTmp);
                }
                core::LocalVariable hash =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::hashTemp(), cctx.inWhat.symbol);
                current->exprs.emplace_back(hash, h->loc, make_unique<Alias>(cctx.ctx.state.defn_emptyHash()));
                current->exprs.emplace_back(cctx.target, h->loc,
                                            make_unique<Send>(hash, core::Names::buildHash(), vars));
                ret = current;
            },

            [&](ast::Expression *n) {
                current->exprs.emplace_back(cctx.target, n->loc, make_unique<NotSupported>(""));
                ret = current;
            });

        /*[&](ast::Break *a) {}, */
        // For, Rescue,
        // Symbol, NamedArg, Array,
        Error::check(ret != nullptr);
        return ret;
    } catch (...) {
        cctx.ctx.state.errors.error(what->loc, core::ErrorClass::Internal,
                                    "Failed to convert tree to CFG (backtrace is above )");
        throw;
    }
}

} // namespace cfg
} // namespace ruby_typer
