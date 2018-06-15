#include "builder.h"
#include "core/Names/cfg.h"
#include "core/errors/cfg.h"
#include "core/errors/internal.h"

#include <unordered_map>

using namespace std;

namespace sorbet {
namespace cfg {

void conditionalJump(BasicBlock *from, core::LocalVariable cond, BasicBlock *thenb, BasicBlock *elseb, CFG &inWhat,
                     core::Loc loc) {
    thenb->flags |= CFG::WAS_JUMP_DESTINATION;
    elseb->flags |= CFG::WAS_JUMP_DESTINATION;
    if (from != inWhat.deadBlock()) {
        ENFORCE(!from->bexit.isCondSet(), "condition for block already set");
        ENFORCE(from->bexit.thenb == nullptr, "thenb already set");
        ENFORCE(from->bexit.elseb == nullptr, "elseb already set");
        from->bexit.cond = cond;
        from->bexit.thenb = thenb;
        from->bexit.elseb = elseb;
        from->bexit.loc = loc;
        thenb->backEdges.push_back(from);
        elseb->backEdges.push_back(from);
    }
}

void unconditionalJump(BasicBlock *from, BasicBlock *to, CFG &inWhat, core::Loc loc) {
    to->flags |= CFG::WAS_JUMP_DESTINATION;
    if (from != inWhat.deadBlock()) {
        ENFORCE(!from->bexit.isCondSet(), "condition for block already set");
        ENFORCE(from->bexit.thenb == nullptr, "thenb already set");
        ENFORCE(from->bexit.elseb == nullptr, "elseb already set");
        from->bexit.cond = core::LocalVariable::noVariable();
        from->bexit.elseb = to;
        from->bexit.thenb = to;
        from->bexit.loc = loc;
        to->backEdges.push_back(from);
    }
}

void jumpToDead(BasicBlock *from, CFG &inWhat, core::Loc loc) {
    auto *db = inWhat.deadBlock();
    if (from != db) {
        ENFORCE(!from->bexit.isCondSet(), "condition for block already set");
        ENFORCE(from->bexit.thenb == nullptr, "thenb already set");
        ENFORCE(from->bexit.elseb == nullptr, "elseb already set");
        from->bexit.cond = core::LocalVariable::noVariable();
        from->bexit.elseb = db;
        from->bexit.thenb = db;
        from->bexit.loc = loc;
        db->backEdges.push_back(from);
    }
}

core::LocalVariable global2Local(core::Context ctx, core::SymbolRef what, CFG &inWhat,
                                 unordered_map<core::SymbolRef, core::LocalVariable> &aliases) {
    core::LocalVariable &alias = aliases[what];
    if (!alias.exists()) {
        const core::Symbol &data = what.data(ctx);
        alias = ctx.state.newTemporary(data.name, inWhat.symbol);
    }
    return alias;
}

/** Convert `what` into a cfg, by starting to evaluate it in `current` inside method defined by `inWhat`.
 * store result of evaluation into `target`. Returns basic block in which evaluation should proceed.
 */
BasicBlock *CFGBuilder::walk(CFGContext cctx, ast::Expression *what, BasicBlock *current) {
    /** Try to pay additional attention not to duplicate any part of tree.
     * Though this may lead to more effictient and a better CFG if it was to be actually compiled into code
     * This will lead to duplicate typechecking and may lead to exponential explosion of typechecking time
     * for some code snippets. */
    ENFORCE(!current->bexit.isCondSet() || current == cctx.inWhat.deadBlock(),
            "current block has already been finalized!");

    try {
        BasicBlock *ret = nullptr;
        typecase(
            what,
            [&](ast::While *a) {
                auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1);
                auto continueBlock = cctx.inWhat.freshBlock(cctx.loops);
                unconditionalJump(current, headerBlock, cctx.inWhat, a->loc);

                core::LocalVariable condSym = cctx.ctx.state.newTemporary(core::Names::whileTemp(), cctx.inWhat.symbol);
                auto headerEnd = walk(cctx.withTarget(condSym).withLoopScope(headerBlock, continueBlock), a->cond.get(),
                                      headerBlock);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1);
                conditionalJump(headerEnd, condSym, bodyBlock, continueBlock, cctx.inWhat, a->cond->loc);
                // finishHeader
                core::LocalVariable bodySym = cctx.ctx.state.newTemporary(core::Names::statTemp(), cctx.inWhat.symbol);

                auto body =
                    walk(cctx.withTarget(bodySym).withLoopScope(headerBlock, continueBlock), a->body.get(), bodyBlock);
                unconditionalJump(body, headerBlock, cctx.inWhat, a->loc);

                continueBlock->exprs.emplace_back(cctx.target, a->loc, make_unique<Literal>(core::Types::nilClass()));
                ret = continueBlock;
            },
            [&](ast::Return *a) {
                core::LocalVariable retSym = cctx.ctx.state.newTemporary(core::Names::returnTemp(), cctx.inWhat.symbol);
                auto cont = walk(cctx.withTarget(retSym), a->expr.get(), current);
                cont->exprs.emplace_back(cctx.target, a->loc, make_unique<Return>(retSym)); // dead assign.
                jumpToDead(cont, cctx.inWhat, a->loc);
                ret = cctx.inWhat.deadBlock();
            },
            [&](ast::If *a) {
                core::LocalVariable ifSym = cctx.ctx.state.newTemporary(core::Names::ifTemp(), cctx.inWhat.symbol);
                ENFORCE(ifSym.exists(), "ifSym does not exist");
                auto cont = walk(cctx.withTarget(ifSym), a->cond.get(), current);
                auto thenBlock = cctx.inWhat.freshBlock(cctx.loops);
                auto elseBlock = cctx.inWhat.freshBlock(cctx.loops);
                conditionalJump(cont, ifSym, thenBlock, elseBlock, cctx.inWhat, a->cond->loc);

                auto thenEnd = walk(cctx, a->thenp.get(), thenBlock);
                auto elseEnd = walk(cctx, a->elsep.get(), elseBlock);
                if (thenEnd != cctx.inWhat.deadBlock() || elseEnd != cctx.inWhat.deadBlock()) {
                    if (thenEnd == cctx.inWhat.deadBlock()) {
                        ret = elseEnd;
                    } else if (thenEnd == cctx.inWhat.deadBlock()) {
                        ret = thenEnd;
                    } else {
                        ret = cctx.inWhat.freshBlock(cctx.loops);
                        unconditionalJump(thenEnd, ret, cctx.inWhat, a->loc);
                        unconditionalJump(elseEnd, ret, cctx.inWhat, a->loc);
                    }
                } else {
                    ret = cctx.inWhat.deadBlock();
                }
            },
            [&](ast::Literal *a) {
                current->exprs.emplace_back(cctx.target, a->loc, make_unique<Literal>(a->value));
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
                    // lhs = core::Symbols::todo();
                    Error::raise("should never be reached");
                }
                auto rhsCont = walk(cctx.withTarget(lhs), a->rhs.get(), current);
                rhsCont->exprs.emplace_back(cctx.target, a->loc, make_unique<Ident>(lhs));
                ret = rhsCont;
            },
            [&](ast::InsSeq *a) {
                for (auto &exp : a->stats) {
                    core::LocalVariable temp = cctx.ctx.state.newTemporary(core::Names::statTemp(), cctx.inWhat.symbol);
                    current = walk(cctx.withTarget(temp), exp.get(), current);
                }
                ret = walk(cctx, a->expr.get(), current);
            },
            [&](ast::Send *s) {
                core::LocalVariable recv;

                recv = cctx.ctx.state.newTemporary(core::Names::statTemp(), cctx.inWhat.symbol);
                current = walk(cctx.withTarget(recv), s->recv.get(), current);

                vector<core::LocalVariable> args;
                for (auto &exp : s->args) {
                    core::LocalVariable temp;
                    temp = cctx.ctx.state.newTemporary(core::Names::statTemp(), cctx.inWhat.symbol);
                    current = walk(cctx.withTarget(temp), exp.get(), current);

                    args.push_back(temp);
                }

                core::SymbolRef blockSym;
                if (s->block) {
                    blockSym = s->block->symbol;
                }

                if (s->block != nullptr) {
                    vector<core::LocalVariable> argsCopy = args;
                    core::SymbolRef sym = s->block->symbol;
                    auto link = make_shared<core::SendAndBlockLink>(sym);
                    auto send = make_unique<Send>(recv, s->fun, argsCopy, link);
                    auto solveConstraint = make_unique<SolveConstraint>(link);
                    core::LocalVariable sendTemp =
                        cctx.ctx.state.newTemporary(core::Names::blockPreCallTemp(), cctx.inWhat.symbol);
                    current->exprs.emplace_back(sendTemp, s->loc, move(send));
                    auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1);
                    auto postBlock = cctx.inWhat.freshBlock(cctx.loops);
                    auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1);

                    const core::Symbol &data = sym.data(cctx.ctx);

                    for (int i = 0; i < data.arguments().size(); ++i) {
                        auto &arg = s->block->args[i];

                        if (auto id = ast::cast_tree<ast::Local>(arg.get())) {
                            core::LocalVariable argLoc = id->localVariable;
                            bodyBlock->exprs.emplace_back(argLoc, arg->loc, make_unique<LoadYieldParam>(link, i));
                        } else {
                            Error::raise("Should have been removed by namer");
                        }
                    }

                    conditionalJump(headerBlock, core::LocalVariable::blockCall(), bodyBlock, postBlock, cctx.inWhat,
                                    s->loc);

                    unconditionalJump(current, headerBlock, cctx.inWhat, s->loc);

                    core::LocalVariable blockrv =
                        cctx.ctx.state.newTemporary(core::Names::blockReturnTemp(), cctx.inWhat.symbol);
                    auto blockLast = walk(cctx.withTarget(blockrv)
                                              .withLoopScope(headerBlock, postBlock, s->block->symbol)
                                              .withSendAndBlockLink(link),
                                          s->block->body.get(), bodyBlock);
                    if (blockLast != cctx.inWhat.deadBlock()) {
                        core::LocalVariable dead =
                            cctx.ctx.state.newTemporary(core::Names::blockReturnTemp(), cctx.inWhat.symbol);
                        blockLast->exprs.emplace_back(dead, s->block->loc, make_unique<BlockReturn>(link, blockrv));
                    }

                    unconditionalJump(blockLast, headerBlock, cctx.inWhat, s->loc);

                    current = postBlock;
                    current->exprs.emplace_back(cctx.target, s->loc, move(solveConstraint));
                } else {
                    current->exprs.emplace_back(cctx.target, s->loc, make_unique<Send>(recv, s->fun, args));
                }

                ret = current;
            },

            [&](ast::Block *a) { Error::raise("should never encounter a bare Block"); },

            [&](ast::Next *a) {
                core::LocalVariable exprSym =
                    cctx.ctx.state.newTemporary(core::Names::blockReturnTemp(), cctx.inWhat.symbol);
                auto afterNext = walk(cctx.withTarget(exprSym), a->expr.get(), current);
                if (afterNext != cctx.inWhat.deadBlock() && cctx.rubyBlock.exists()) {
                    core::LocalVariable dead =
                        cctx.ctx.state.newTemporary(core::Names::blockReturnTemp(), cctx.inWhat.symbol);
                    ENFORCE(cctx.link.get() != nullptr);
                    afterNext->exprs.emplace_back(dead, a->loc, make_unique<BlockReturn>(cctx.link, exprSym));
                }

                if (cctx.nextScope == nullptr) {
                    if (auto e = cctx.ctx.state.beginError(a->loc, core::errors::CFG::NoNextScope)) {
                        e.setHeader("No `do` block around `next`");
                    }
                    // I guess just keep going into deadcode?
                    unconditionalJump(afterNext, cctx.inWhat.deadBlock(), cctx.inWhat, a->loc);
                } else {
                    unconditionalJump(afterNext, cctx.nextScope, cctx.inWhat, a->loc);
                }

                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Break *a) {
                core::LocalVariable exprSym =
                    cctx.ctx.state.newTemporary(core::Names::returnTemp(), cctx.inWhat.symbol);
                auto afterBreak = walk(cctx.withTarget(exprSym), a->expr.get(), current);

                if (cctx.breakScope == nullptr) {
                    if (auto e = cctx.ctx.state.beginError(a->loc, core::errors::CFG::NoNextScope)) {
                        e.setHeader("No `do` block around `break`");
                    }
                    // I guess just keep going into deadcode?
                    unconditionalJump(afterBreak, cctx.inWhat.deadBlock(), cctx.inWhat, a->loc);
                } else {
                    unconditionalJump(afterBreak, cctx.breakScope, cctx.inWhat, a->loc);
                }
                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Retry *a) {
                if (cctx.rescueScope == nullptr) {
                    if (auto e = cctx.ctx.state.beginError(a->loc, core::errors::CFG::NoNextScope)) {
                        e.setHeader("No `begin` block around `retry`");
                    }
                    // I guess just keep going into deadcode?
                    unconditionalJump(current, cctx.inWhat.deadBlock(), cctx.inWhat, a->loc);
                } else {
                    unconditionalJump(current, cctx.rescueScope, cctx.inWhat, a->loc);
                }
                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Rescue *a) {
                auto rescueStartBlock = cctx.inWhat.freshBlock(cctx.loops);
                auto unanalyzableCondition = cctx.ctx.state.newTemporary(core::Names::rescueTemp(), cctx.inWhat.symbol);
                rescueStartBlock->exprs.emplace_back(unanalyzableCondition, what->loc, make_unique<Unanalyzable>());
                unconditionalJump(current, rescueStartBlock, cctx.inWhat, a->loc);
                cctx.rescueScope = rescueStartBlock;

                auto rescueHandlersBlock = cctx.inWhat.freshBlock(cctx.loops);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops);
                conditionalJump(rescueStartBlock, unanalyzableCondition, rescueHandlersBlock, bodyBlock, cctx.inWhat,
                                a->loc);

                //                cctx.loops += 1; // should formally be here but this makes us report a lot of false
                //                errors
                bodyBlock = walk(cctx, a->body.get(), bodyBlock);
                auto elseBody = cctx.inWhat.freshBlock(cctx.loops);
                unconditionalJump(bodyBlock, elseBody, cctx.inWhat, a->loc);

                elseBody = walk(cctx, a->else_.get(), elseBody);
                auto ensureBody = cctx.inWhat.freshBlock(cctx.loops);
                unconditionalJump(elseBody, ensureBody, cctx.inWhat, a->loc);

                auto throwAway = cctx.ctx.state.newTemporary(core::Names::rescueTemp(), cctx.inWhat.symbol);
                ensureBody = walk(cctx.withTarget(throwAway), a->ensure.get(), ensureBody);
                ret = cctx.inWhat.freshBlock(cctx.loops);
                unconditionalJump(ensureBody, ret, cctx.inWhat, a->loc);

                for (auto &rescueCase : a->rescueCases) {
                    auto caseBody = cctx.inWhat.freshBlock(cctx.loops);
                    auto &exceptions = rescueCase->exceptions;
                    auto added = false;
                    if (exceptions.empty()) {
                        // rescue without a class catches StandardError
                        exceptions.emplace_back(
                            make_unique<ast::Ident>(rescueCase->var->loc, core::Symbols::StandardError()));
                        added = true;
                    }
                    for (auto &ex : exceptions) {
                        auto loc = ex->loc;
                        auto exceptionClass =
                            cctx.ctx.state.newTemporary(core::Names::rescueTemp(), cctx.inWhat.symbol);
                        rescueHandlersBlock = walk(cctx.withTarget(exceptionClass), ex.get(), rescueHandlersBlock);

                        auto isaCheck = cctx.ctx.state.newTemporary(core::Names::rescueTemp(), cctx.inWhat.symbol);
                        vector<core::LocalVariable> args;
                        args.emplace_back(exceptionClass);

                        rescueHandlersBlock->exprs.emplace_back(
                            isaCheck, loc, make_unique<Send>(unanalyzableCondition, core::Names::is_a_p(), args));

                        auto otherHandlerBlock = cctx.inWhat.freshBlock(cctx.loops);
                        conditionalJump(rescueHandlersBlock, isaCheck, caseBody, otherHandlerBlock, cctx.inWhat, loc);
                        rescueHandlersBlock = otherHandlerBlock;
                    }
                    if (added) {
                        exceptions.pop_back();
                    }

                    auto *local = ast::cast_tree<ast::Local>(rescueCase->var.get());
                    ENFORCE(local != nullptr, "rescue case var not a local?");
                    caseBody->exprs.emplace_back(local->localVariable, rescueCase->var->loc,
                                                 make_unique<Ident>(unanalyzableCondition));

                    caseBody = walk(cctx, rescueCase->body.get(), caseBody);
                    if (ret == cctx.inWhat.deadBlock()) {
                        // If the main body -> else -> ensure -> ret path is dead, we need
                        // to have a path out of the exception handlers
                        ret = cctx.inWhat.freshBlock(cctx.loops);
                        ensureBody = ret;
                    }
                    unconditionalJump(caseBody, ensureBody, cctx.inWhat, a->loc);
                }

                // None of the handlers were taken
                jumpToDead(rescueHandlersBlock, cctx.inWhat, a->loc);
            },

            [&](ast::Hash *h) {
                vector<core::LocalVariable> vars;
                for (int i = 0; i < h->keys.size(); i++) {
                    core::LocalVariable keyTmp =
                        cctx.ctx.state.newTemporary(core::Names::hashTemp(), cctx.inWhat.symbol);
                    core::LocalVariable valTmp =
                        cctx.ctx.state.newTemporary(core::Names::hashTemp(), cctx.inWhat.symbol);
                    current = walk(cctx.withTarget(keyTmp), h->keys[i].get(), current);
                    current = walk(cctx.withTarget(valTmp), h->values[i].get(), current);
                    vars.push_back(keyTmp);
                    vars.push_back(valTmp);
                }
                core::LocalVariable magic = global2Local(cctx.ctx, core::Symbols::Magic(), cctx.inWhat, cctx.aliases);
                current->exprs.emplace_back(cctx.target, h->loc,
                                            make_unique<Send>(magic, core::Names::buildHash(), vars));
                ret = current;
            },

            [&](ast::Array *a) {
                vector<core::LocalVariable> vars;
                for (auto &elem : a->elems) {
                    core::LocalVariable tmp = cctx.ctx.state.newTemporary(core::Names::arrayTemp(), cctx.inWhat.symbol);
                    current = walk(cctx.withTarget(tmp), elem.get(), current);
                    vars.push_back(tmp);
                }
                core::LocalVariable magic = global2Local(cctx.ctx, core::Symbols::Magic(), cctx.inWhat, cctx.aliases);
                current->exprs.emplace_back(cctx.target, a->loc,
                                            make_unique<Send>(magic, core::Names::buildArray(), vars));
                ret = current;
            },

            [&](ast::Cast *c) {
                core::LocalVariable tmp = cctx.ctx.state.newTemporary(core::Names::castTemp(), cctx.inWhat.symbol);
                current = walk(cctx.withTarget(tmp), c->arg.get(), current);
                current->exprs.emplace_back(cctx.target, c->loc, make_unique<Cast>(tmp, c->type, c->cast));
                if (c->cast == core::Names::let()) {
                    cctx.inWhat.minLoops[cctx.target] = CFG::MIN_LOOP_LET;
                }

                ret = current;
            },

            [&](ast::EmptyTree *n) { ret = current; },

            [&](ast::ClassDef *c) { Error::raise("Should have been removed by FlattenWalk"); },
            [&](ast::MethodDef *c) { Error::raise("Should have been removed by FlattenWalk"); },

            [&](ast::Expression *n) { Error::raise("Unimplemented AST Node: ", n->nodeName()); });

        // For, Rescue,
        // Symbol, Array,
        ENFORCE(ret != nullptr, "CFB builder ret unset");
        return ret;
    } catch (SRubyException &) {
        if (auto e = cctx.ctx.state.beginError(what->loc, core::errors::Internal::InternalError)) {
            e.setHeader("Failed to convert tree to CFG (backtrace is above )");
        }
        throw;
    }
}

} // namespace cfg
} // namespace sorbet
