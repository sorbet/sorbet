#include "ast/Helpers.h"
#include "cfg/builder/builder.h"
#include "core/Names.h"
#include "core/errors/cfg.h"
#include "core/errors/internal.h"

using namespace std;

namespace sorbet::cfg {

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
        thenb->backEdges.emplace_back(from);
        elseb->backEdges.emplace_back(from);
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
        to->backEdges.emplace_back(from);
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
        db->backEdges.emplace_back(from);
    }
}

core::LocalVariable global2Local(CFGContext cctx, core::SymbolRef what, CFG &inWhat,
                                 UnorderedMap<core::SymbolRef, core::LocalVariable> &aliases) {
    // Note: this will add an empty local to aliases if 'what' is not there
    core::LocalVariable &alias = aliases[what];
    if (!alias.exists()) {
        const core::SymbolData data = what.data(cctx.ctx);
        alias = cctx.newTemporary(data->name);
    }
    return alias;
}

void synthesizeExpr(BasicBlock *bb, core::LocalVariable var, core::Loc loc, unique_ptr<Instruction> inst) {
    auto &inserted = bb->exprs.emplace_back(var, loc, move(inst));
    inserted.value->isSynthetic = true;
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

                core::LocalVariable condSym = cctx.newTemporary(core::Names::whileTemp());
                auto headerEnd = walk(cctx.withTarget(condSym).withLoopScope(headerBlock, continueBlock), a->cond.get(),
                                      headerBlock);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1);
                conditionalJump(headerEnd, condSym, bodyBlock, continueBlock, cctx.inWhat, a->cond->loc);
                // finishHeader
                core::LocalVariable bodySym = cctx.newTemporary(core::Names::statTemp());

                auto body =
                    walk(cctx.withTarget(bodySym).withLoopScope(headerBlock, continueBlock), a->body.get(), bodyBlock);
                unconditionalJump(body, headerBlock, cctx.inWhat, a->loc);

                synthesizeExpr(continueBlock, cctx.target, a->loc, make_unique<Literal>(core::Types::nilClass()));
                ret = continueBlock;
            },
            [&](ast::Return *a) {
                core::LocalVariable retSym = cctx.newTemporary(core::Names::returnTemp());
                auto cont = walk(cctx.withTarget(retSym), a->expr.get(), current);
                cont->exprs.emplace_back(cctx.target, a->loc, make_unique<Return>(retSym)); // dead assign.
                jumpToDead(cont, cctx.inWhat, a->loc);
                ret = cctx.inWhat.deadBlock();
            },
            [&](ast::If *a) {
                core::LocalVariable ifSym = cctx.newTemporary(core::Names::ifTemp());
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
            [&](ast::UnresolvedConstantLit *a) { Exception::raise("Should have been eliminated by namer/resolver"); },
            [&](ast::Field *a) {
                current->exprs.emplace_back(
                    cctx.target, a->loc, make_unique<Ident>(global2Local(cctx, a->symbol, cctx.inWhat, cctx.aliases)));
                ret = current;
            },
            [&](ast::ConstantLit *a) {
                if (a->original) {
                    if (auto nested = ast::cast_tree<ast::ConstantLit>(a->original->scope.get())) {
                        core::LocalVariable deadSym = cctx.newTemporary(core::Names::keepForIde());
                        current = walk(cctx.withTarget(deadSym), nested, current);
                    }
                }
                if (!a->typeAlias) {
                    current->exprs.emplace_back(cctx.target, a->loc, make_unique<Alias>(a->constantSymbol()));
                    ret = current;
                } else {
                    ret = walk(cctx, a->typeAlias.get(), current);
                }
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
                core::LocalVariable lhs;
                if (auto lhsIdent = ast::cast_tree<ast::ConstantLit>(a->lhs.get())) {
                    lhs = global2Local(cctx, lhsIdent->constantSymbol(), cctx.inWhat, cctx.aliases);
                } else if (auto field = ast::cast_tree<ast::Field>(a->lhs.get())) {
                    lhs = global2Local(cctx, field->symbol, cctx.inWhat, cctx.aliases);
                } else if (auto lhsLocal = ast::cast_tree<ast::Local>(a->lhs.get())) {
                    lhs = lhsLocal->localVariable;
                } else {
                    // TODO(nelhage): Once namer is complete this should be a
                    // fatal error
                    // lhs = core::Symbols::todo();
                    Exception::raise("should never be reached");
                }

                auto rhsCont = walk(cctx.withTarget(lhs), a->rhs.get(), current);
                rhsCont->exprs.emplace_back(cctx.target, a->loc, make_unique<Ident>(lhs));
                ret = rhsCont;
            },
            [&](ast::InsSeq *a) {
                for (auto &exp : a->stats) {
                    core::LocalVariable temp = cctx.newTemporary(core::Names::statTemp());
                    current = walk(cctx.withTarget(temp), exp.get(), current);
                }
                ret = walk(cctx, a->expr.get(), current);
            },
            [&](ast::Send *s) {
                core::LocalVariable recv;

                recv = cctx.newTemporary(core::Names::statTemp());
                current = walk(cctx.withTarget(recv), s->recv.get(), current);

                InlinedVector<core::LocalVariable, 2> args;
                InlinedVector<core::Loc, 2> argLocs;
                for (auto &exp : s->args) {
                    core::LocalVariable temp;
                    temp = cctx.newTemporary(core::Names::statTemp());
                    current = walk(cctx.withTarget(temp), exp.get(), current);

                    args.emplace_back(temp);
                    argLocs.emplace_back(exp->loc);
                }

                core::SymbolRef blockSym;
                if (s->block) {
                    blockSym = s->block->symbol;
                }

                if (s->block != nullptr) {
                    core::SymbolRef sym = s->block->symbol;
                    auto link = make_shared<core::SendAndBlockLink>(sym, s->fun);
                    auto send = make_unique<Send>(recv, s->fun, s->recv->loc, args, argLocs, link);
                    auto solveConstraint = make_unique<SolveConstraint>(link);
                    core::LocalVariable sendTemp = cctx.newTemporary(core::Names::blockPreCallTemp());
                    current->exprs.emplace_back(sendTemp, s->loc, move(send));
                    auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1);
                    auto postBlock = cctx.inWhat.freshBlock(cctx.loops);
                    auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1);

                    const core::SymbolData data = sym.data(cctx.ctx);

                    core::LocalVariable argTemp = cctx.newTemporary(core::Names::blkArg());
                    core::LocalVariable idxTmp = cctx.newTemporary(core::Names::blkArg());
                    bodyBlock->exprs.emplace_back(argTemp, data->loc(), make_unique<LoadYieldParams>(link, sym));

                    for (int i = 0; i < data->arguments().size(); ++i) {
                        auto &arg = s->block->args[i];
                        auto id = ast::MK::arg2Local(arg.get());
                        ENFORCE(id, "Should have been removed by namer");
                        core::LocalVariable argLoc = id->localVariable;

                        if (data->arguments()[i].data(cctx.ctx)->isRepeated()) {
                            if (i != 0) {
                                // Mixing positional and rest args in blocks is
                                // not currently supported; drop in an untyped.
                                bodyBlock->exprs.emplace_back(argLoc, id->loc,
                                                              make_unique<Alias>(core::Symbols::untyped()));
                            } else {
                                bodyBlock->exprs.emplace_back(argLoc, id->loc, make_unique<Ident>(argTemp));
                            }
                            continue;
                        }

                        bodyBlock->exprs.emplace_back(
                            idxTmp, id->loc, make_unique<Literal>(core::make_type<core::LiteralType>(int64_t(i))));
                        InlinedVector<core::LocalVariable, 2> idxVec{idxTmp};
                        InlinedVector<core::Loc, 2> locs{id->loc};
                        bodyBlock->exprs.emplace_back(
                            argLoc, id->loc,
                            make_unique<Send>(argTemp, core::Names::squareBrackets(), s->block->loc, idxVec, locs));
                    }

                    conditionalJump(headerBlock, core::LocalVariable::blockCall(), bodyBlock, postBlock, cctx.inWhat,
                                    s->loc);

                    unconditionalJump(current, headerBlock, cctx.inWhat, s->loc);

                    core::LocalVariable blockrv = cctx.newTemporary(core::Names::blockReturnTemp());
                    auto blockLast = walk(cctx.withTarget(blockrv)
                                              .withLoopScope(headerBlock, postBlock, s->block->symbol)
                                              .withSendAndBlockLink(link),
                                          s->block->body.get(), bodyBlock);
                    if (blockLast != cctx.inWhat.deadBlock()) {
                        core::LocalVariable dead = cctx.newTemporary(core::Names::blockReturnTemp());
                        synthesizeExpr(blockLast, dead, s->block->loc, make_unique<BlockReturn>(link, blockrv));
                    }

                    unconditionalJump(blockLast, headerBlock, cctx.inWhat, s->loc);

                    current = postBlock;
                    current->exprs.emplace_back(cctx.target, s->loc, move(solveConstraint));
                } else {
                    current->exprs.emplace_back(cctx.target, s->loc,
                                                make_unique<Send>(recv, s->fun, s->recv->loc, args, argLocs));
                }

                ret = current;
            },

            [&](ast::Block *a) { Exception::raise("should never encounter a bare Block"); },

            [&](ast::Next *a) {
                core::LocalVariable exprSym = cctx.newTemporary(core::Names::nextTemp());
                auto afterNext = walk(cctx.withTarget(exprSym), a->expr.get(), current);
                if (afterNext != cctx.inWhat.deadBlock() && cctx.rubyBlock.exists()) {
                    core::LocalVariable dead = cctx.newTemporary(core::Names::nextTemp());
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
                core::LocalVariable exprSym = cctx.newTemporary(core::Names::returnTemp());
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
                unconditionalJump(current, rescueStartBlock, cctx.inWhat, a->loc);
                cctx.rescueScope = rescueStartBlock;

                // We have a simplified view of the control flow here but in
                // practise it has been reasonable on our codebase.
                // We don't model that each expression in the `body` or `else` could
                // throw, instead we model only never running anything in the
                // body, or running the whole thing. To do this we  have a magic
                // Unanalyzable variable at the top of the body using
                // `rescueStartTemp` and one at the end of the else using
                // `rescueEndTemp` which can jump into the rescue handlers.
                auto rescueHandlersBlock = cctx.inWhat.freshBlock(cctx.loops);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops);
                auto rescueStartTemp = cctx.newTemporary(core::Names::rescueStartTemp());
                synthesizeExpr(rescueStartBlock, rescueStartTemp, what->loc, make_unique<Unanalyzable>());
                conditionalJump(rescueStartBlock, rescueStartTemp, rescueHandlersBlock, bodyBlock, cctx.inWhat, a->loc);

                // cctx.loops += 1; // should formally be here but this makes us report a lot of false errors
                bodyBlock = walk(cctx, a->body.get(), bodyBlock);
                auto elseBody = cctx.inWhat.freshBlock(cctx.loops);
                unconditionalJump(bodyBlock, elseBody, cctx.inWhat, a->loc);

                elseBody = walk(cctx, a->else_.get(), elseBody);
                auto ensureBody = cctx.inWhat.freshBlock(cctx.loops);

                auto shouldEnsureBlock = cctx.inWhat.freshBlock(cctx.loops);
                unconditionalJump(elseBody, shouldEnsureBlock, cctx.inWhat, a->loc);
                auto rescueEndTemp = cctx.newTemporary(core::Names::rescueEndTemp());
                synthesizeExpr(shouldEnsureBlock, rescueEndTemp, what->loc, make_unique<Unanalyzable>());
                conditionalJump(shouldEnsureBlock, rescueEndTemp, rescueHandlersBlock, ensureBody, cctx.inWhat, a->loc);

                for (auto &rescueCase : a->rescueCases) {
                    auto caseBody = cctx.inWhat.freshBlock(cctx.loops);
                    auto &exceptions = rescueCase->exceptions;
                    auto added = false;
                    auto *local = ast::cast_tree<ast::Local>(rescueCase->var.get());
                    ENFORCE(local != nullptr, "rescue case var not a local?");
                    rescueHandlersBlock->exprs.emplace_back(local->localVariable, rescueCase->var->loc,
                                                            make_unique<Unanalyzable>());

                    if (exceptions.empty()) {
                        // rescue without a class catches StandardError
                        exceptions.emplace_back(
                            ast::MK::Constant(rescueCase->var->loc, core::Symbols::StandardError()));
                        added = true;
                    }
                    for (auto &ex : exceptions) {
                        auto loc = ex->loc;
                        auto exceptionClass = cctx.newTemporary(core::Names::exceptionClassTemp());
                        rescueHandlersBlock = walk(cctx.withTarget(exceptionClass), ex.get(), rescueHandlersBlock);

                        auto isaCheck = cctx.newTemporary(core::Names::isaCheckTemp());
                        InlinedVector<core::LocalVariable, 2> args;
                        InlinedVector<core::Loc, 2> argLocs = {loc};
                        args.emplace_back(exceptionClass);

                        rescueHandlersBlock->exprs.emplace_back(
                            isaCheck, loc,
                            make_unique<Send>(local->localVariable, core::Names::is_a_p(), loc, args, argLocs));

                        auto otherHandlerBlock = cctx.inWhat.freshBlock(cctx.loops);
                        conditionalJump(rescueHandlersBlock, isaCheck, caseBody, otherHandlerBlock, cctx.inWhat, loc);
                        rescueHandlersBlock = otherHandlerBlock;
                    }
                    if (added) {
                        exceptions.pop_back();
                    }

                    caseBody = walk(cctx, rescueCase->body.get(), caseBody);
                    if (ret == cctx.inWhat.deadBlock()) {
                        // If the main body -> else -> ensure -> ret path is dead, we need
                        // to have a path out of the exception handlers
                        ret = cctx.inWhat.freshBlock(cctx.loops);
                        ensureBody = ret;
                    }
                    unconditionalJump(caseBody, ensureBody, cctx.inWhat, a->loc);
                }

                // This magic local remembers if none of the `rescue`s match,
                // and if so, after the ensure runs, we should jump to dead
                // since in Ruby the exception would propagate up the statck.
                auto gotoDeadTemp = cctx.newTemporary(core::Names::gotoDeadTemp());
                synthesizeExpr(rescueHandlersBlock, gotoDeadTemp, a->loc,
                               make_unique<Literal>(core::make_type<core::LiteralType>(true)));
                unconditionalJump(rescueHandlersBlock, ensureBody, cctx.inWhat, a->loc);

                auto throwAway = cctx.newTemporary(core::Names::throwAwayTemp());
                ensureBody = walk(cctx.withTarget(throwAway), a->ensure.get(), ensureBody);
                ret = cctx.inWhat.freshBlock(cctx.loops);
                conditionalJump(ensureBody, gotoDeadTemp, cctx.inWhat.deadBlock(), ret, cctx.inWhat, a->loc);
            },

            [&](ast::Hash *h) {
                InlinedVector<core::LocalVariable, 2> vars;
                InlinedVector<core::Loc, 2> locs;
                for (int i = 0; i < h->keys.size(); i++) {
                    core::LocalVariable keyTmp = cctx.newTemporary(core::Names::hashTemp());
                    core::LocalVariable valTmp = cctx.newTemporary(core::Names::hashTemp());
                    current = walk(cctx.withTarget(keyTmp), h->keys[i].get(), current);
                    current = walk(cctx.withTarget(valTmp), h->values[i].get(), current);
                    vars.emplace_back(keyTmp);
                    vars.emplace_back(valTmp);
                    locs.emplace_back(h->keys[i]->loc);
                    locs.emplace_back(h->values[i]->loc);
                }
                core::LocalVariable magic = cctx.newTemporary(core::Names::magic());
                synthesizeExpr(current, magic, core::Loc::none(), make_unique<Alias>(core::Symbols::Magic()));

                current->exprs.emplace_back(cctx.target, h->loc,
                                            make_unique<Send>(magic, core::Names::buildHash(), h->loc, vars, locs));
                ret = current;
            },

            [&](ast::Array *a) {
                InlinedVector<core::LocalVariable, 2> vars;
                InlinedVector<core::Loc, 2> locs;
                for (auto &elem : a->elems) {
                    core::LocalVariable tmp = cctx.newTemporary(core::Names::arrayTemp());
                    current = walk(cctx.withTarget(tmp), elem.get(), current);
                    vars.emplace_back(tmp);
                    locs.emplace_back(a->loc);
                }
                core::LocalVariable magic = cctx.newTemporary(core::Names::magic());
                synthesizeExpr(current, magic, core::Loc::none(), make_unique<Alias>(core::Symbols::Magic()));
                current->exprs.emplace_back(cctx.target, a->loc,
                                            make_unique<Send>(magic, core::Names::buildArray(), a->loc, vars, locs));
                ret = current;
            },

            [&](ast::Cast *c) {
                core::LocalVariable tmp = cctx.newTemporary(core::Names::castTemp());
                current = walk(cctx.withTarget(tmp), c->arg.get(), current);
                current->exprs.emplace_back(cctx.target, c->loc, make_unique<Cast>(tmp, c->type, c->cast));
                if (c->cast == core::Names::let()) {
                    cctx.inWhat.minLoops[cctx.target] = CFG::MIN_LOOP_LET;
                }

                ret = current;
            },

            [&](ast::EmptyTree *n) { ret = current; },

            [&](ast::ClassDef *c) { Exception::raise("Should have been removed by FlattenWalk"); },
            [&](ast::MethodDef *c) { Exception::raise("Should have been removed by FlattenWalk"); },

            [&](ast::Expression *n) { Exception::raise("Unimplemented AST Node: ", n->nodeName()); });

        // For, Rescue,
        // Symbol, Array,
        ENFORCE(ret != nullptr, "CFB builder ret unset");
        return ret;
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = cctx.ctx.state.beginError(what->loc, core::errors::Internal::InternalError)) {
            e.setHeader("Failed to convert tree to CFG (backtrace is above )");
        }
        throw;
    }
}

core::LocalVariable CFGContext::newTemporary(core::NameRef name) {
    return core::LocalVariable{name, ++temporaryCounter};
}

} // namespace sorbet::cfg
