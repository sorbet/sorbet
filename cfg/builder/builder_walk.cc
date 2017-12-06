#include "builder.h"

#include <unordered_map>

using namespace std;

namespace ruby_typer {
namespace cfg {

void conditionalJump(BasicBlock *from, core::LocalVariable cond, BasicBlock *thenb, BasicBlock *elseb, CFG &inWhat,
                     core::Loc loc) {
    if (from != inWhat.deadBlock()) {
        Error::check(!from->bexit.cond.exists());
        from->bexit.cond = cond;
        from->bexit.thenb = thenb;
        from->bexit.elseb = elseb;
        from->bexit.loc = loc;
        thenb->backEdges.push_back(from);
        elseb->backEdges.push_back(from);
    }
}

void unconditionalJump(BasicBlock *from, BasicBlock *to, CFG &inWhat, core::Loc loc) {
    if (from != inWhat.deadBlock()) {
        Error::check(!from->bexit.cond.exists());
        from->bexit.cond = core::NameRef(0);
        from->bexit.elseb = to;
        from->bexit.thenb = to;
        from->bexit.loc = loc;
        to->backEdges.push_back(from);
    }
}

void jumpToDead(BasicBlock *from, CFG &inWhat, core::Loc loc) {
    auto *db = inWhat.deadBlock();
    if (from != db) {
        Error::check(!from->bexit.cond.exists());
        from->bexit.cond = core::NameRef(0);
        from->bexit.elseb = db;
        from->bexit.thenb = db;
        from->bexit.loc = loc;
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
BasicBlock *CFGBuilder::walk(CFGContext cctx, ast::Expression *what, BasicBlock *current) {
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
                auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1, a->cond->loc, current);
                unconditionalJump(current, headerBlock, cctx.inWhat, a->loc);

                core::LocalVariable condSym = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                          core::Names::whileTemp(), cctx.inWhat.symbol);
                auto headerEnd = walk(cctx.withTarget(condSym).withScope(headerBlock), a->cond.get(), headerBlock);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1, a->body->loc, headerEnd);
                auto continueBlock = cctx.inWhat.freshBlock(cctx.loops, a->loc, headerEnd);
                conditionalJump(headerEnd, condSym, bodyBlock, continueBlock, cctx.inWhat, a->cond->loc);
                // finishHeader
                core::LocalVariable bodySym =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::statTemp(), cctx.inWhat.symbol);

                auto body = walk(cctx.withTarget(bodySym).withScope(headerBlock), a->body.get(), bodyBlock);
                unconditionalJump(body, headerBlock, cctx.inWhat, a->loc);

                continueBlock->exprs.emplace_back(
                    cctx.target, a->loc,
                    make_unique<Ident>(
                        global2Local(cctx.ctx, core::GlobalState::defn_nil(), cctx.inWhat, cctx.aliases)));
                ret = continueBlock;
            },
            [&](ast::Return *a) {
                core::LocalVariable retSym = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                         core::Names::returnTemp(), cctx.inWhat.symbol);
                auto cont = walk(cctx.withTarget(retSym), a->expr.get(), current);
                cont->exprs.emplace_back(cctx.target, a->loc, make_unique<Return>(retSym)); // dead assign.
                jumpToDead(cont, cctx.inWhat, a->loc);
                ret = cctx.inWhat.deadBlock();
            },
            [&](ast::If *a) {
                core::LocalVariable ifSym =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::ifTemp(), cctx.inWhat.symbol);
                Error::check(ifSym.exists());
                auto cont = walk(cctx.withTarget(ifSym), a->cond.get(), current);
                auto thenBlock = cctx.inWhat.freshBlock(cctx.loops, a->thenp->loc, cont);
                auto elseBlock = cctx.inWhat.freshBlock(cctx.loops, a->elsep->loc, cont);
                conditionalJump(cont, ifSym, thenBlock, elseBlock, cctx.inWhat, a->cond->loc);

                auto thenEnd = walk(cctx, a->thenp.get(), thenBlock);
                auto elseEnd = walk(cctx, a->elsep.get(), elseBlock);
                if (thenEnd != cctx.inWhat.deadBlock() || elseEnd != cctx.inWhat.deadBlock()) {
                    if (thenEnd == cctx.inWhat.deadBlock()) {
                        ret = elseEnd;
                    } else if (thenEnd == cctx.inWhat.deadBlock()) {
                        ret = thenEnd;
                    } else {
                        ret = cctx.inWhat.freshBlock(cctx.loops, a->loc, thenEnd); // could be elseEnd
                        unconditionalJump(thenEnd, ret, cctx.inWhat, a->loc);
                        unconditionalJump(elseEnd, ret, cctx.inWhat, a->loc);
                    }
                } else {
                    ret = cctx.inWhat.deadBlock();
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
                auto rhsCont = walk(cctx.withTarget(lhs), a->rhs.get(), current);
                rhsCont->exprs.emplace_back(cctx.target, a->loc, make_unique<Ident>(lhs));
                ret = rhsCont;
            },
            [&](ast::InsSeq *a) {
                for (auto &exp : a->stats) {
                    core::LocalVariable temp = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                           core::Names::statTemp(), cctx.inWhat.symbol);
                    current = walk(cctx.withTarget(temp), exp.get(), current);
                }
                ret = walk(cctx, a->expr.get(), current);
            },
            [&](ast::Send *s) {
                core::LocalVariable recv;

                recv =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::statTemp(), cctx.inWhat.symbol);
                current = walk(cctx.withTarget(recv), s->recv.get(), current);

                vector<core::LocalVariable> args;
                for (auto &exp : s->args) {
                    core::LocalVariable temp;
                    temp = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::statTemp(),
                                                       cctx.inWhat.symbol);
                    current = walk(cctx.withTarget(temp), exp.get(), current);

                    args.push_back(temp);
                }

                if (s->block != nullptr) {
                    auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1, s->block->loc, current);
                    auto postBlock = cctx.inWhat.freshBlock(cctx.loops, s->loc, headerBlock);
                    auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1, s->block->loc, headerBlock);
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
                                    cctx.inWhat, s->loc);

                    unconditionalJump(current, headerBlock, cctx.inWhat, s->loc);

                    // TODO: handle block arguments somehow??
                    core::LocalVariable blockrv = cctx.ctx.state.newTemporary(
                        core::UniqueNameKind::CFG, core::Names::blockReturnTemp(), cctx.inWhat.symbol);
                    auto blockLast =
                        walk(cctx.withTarget(blockrv).withScope(headerBlock), s->block->body.get(), bodyBlock);

                    unconditionalJump(blockLast, headerBlock, cctx.inWhat, s->loc);

                    current = postBlock;
                }

                current->exprs.emplace_back(cctx.target, s->loc, make_unique<Send>(recv, s->fun, args));
                ret = current;
            },

            [&](ast::Block *a) { Error::raise("should never encounter a bare Block"); },

            [&](ast::Next *a) {
                core::LocalVariable nextSym = cctx.ctx.state.newTemporary(
                    core::UniqueNameKind::CFG, core::Names::returnTemp(), cctx.inWhat.symbol);
                auto afterNext = walk(cctx.withTarget(nextSym), a->expr.get(), current);

                if (cctx.scope == nullptr) {
                    cctx.ctx.state.errors.error(a->loc, core::ErrorClass::NoNextScope, "No `do` block around `next`");
                    // I guess just keep going into deadcode?
                    unconditionalJump(afterNext, cctx.inWhat.deadBlock(), cctx.inWhat, a->loc);
                } else {
                    unconditionalJump(afterNext, cctx.scope, cctx.inWhat, a->loc);
                }

                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Rescue *a) {
                auto unanalyzableCondition = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                         core::Names::rescueTemp(), cctx.inWhat.symbol);
                current->exprs.emplace_back(unanalyzableCondition, what->loc, make_unique<Unanalyzable>());

                auto rescueHandlersBlock = cctx.inWhat.freshBlock(cctx.loops, a->body->loc, current);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops, a->body->loc, current);
                conditionalJump(current, unanalyzableCondition, rescueHandlersBlock, bodyBlock, cctx.inWhat, a->loc);

                bodyBlock = walk(cctx, a->body.get(), bodyBlock);
                auto elseBody = cctx.inWhat.freshBlock(cctx.loops, a->loc, bodyBlock);
                unconditionalJump(bodyBlock, elseBody, cctx.inWhat, a->loc);

                elseBody = walk(cctx, a->else_.get(), elseBody);
                ret = cctx.inWhat.freshBlock(cctx.loops, a->loc, elseBody);
                unconditionalJump(elseBody, ret, cctx.inWhat, a->loc);

                for (auto &rescueCase : a->rescueCases) {
                    auto caseBody = cctx.inWhat.freshBlock(cctx.loops, a->body->loc, rescueHandlersBlock);
                    auto &exceptions = rescueCase->exceptions;
                    auto added = false;
                    if (exceptions.empty()) {
                        // rescue without a class catches StandardError
                        exceptions.emplace_back(
                            make_unique<ast::Ident>(rescueCase->var->loc, cctx.ctx.state.defn_StandardError()));
                        added = true;
                    }
                    for (auto &ex : exceptions) {
                        auto loc = ex->loc;
                        auto exceptionClass = cctx.ctx.state.newTemporary(
                            core::UniqueNameKind::CFG, core::Names::rescueTemp(), cctx.inWhat.symbol);
                        rescueHandlersBlock = walk(cctx.withTarget(exceptionClass), ex.get(), rescueHandlersBlock);

                        auto isaCheck = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                    core::Names::rescueTemp(), cctx.inWhat.symbol);
                        std::vector<core::LocalVariable> args;
                        args.emplace_back(exceptionClass);

                        rescueHandlersBlock->exprs.emplace_back(
                            isaCheck, loc, make_unique<Send>(unanalyzableCondition, core::Names::is_a_p(), args));

                        auto otherHandlerBlock = cctx.inWhat.freshBlock(cctx.loops, loc, rescueHandlersBlock);
                        conditionalJump(rescueHandlersBlock, isaCheck, caseBody, otherHandlerBlock, cctx.inWhat, loc);
                        rescueHandlersBlock = otherHandlerBlock;
                    }
                    if (added) {
                        exceptions.pop_back();
                    }

                    if (auto *send = ast::cast_tree<ast::EmptyTree>(rescueCase->var.get())) {
                        // Nothing to do
                    } else if (auto *send = ast::cast_tree<ast::Send>(rescueCase->var.get())) {
                        Error::check(send->args.empty());
                        auto junk = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::rescueTemp(),
                                                                cctx.inWhat.symbol);
                        send->args.emplace_back(make_unique<ast::Local>(rescueCase->var->loc, unanalyzableCondition));
                        caseBody = walk(cctx.withTarget(junk), send, caseBody);
                    } else if (auto *local = ast::cast_tree<ast::Local>(rescueCase->var.get())) {
                        caseBody->exprs.emplace_back(local->localVariable, rescueCase->var->loc,
                                                     make_unique<Ident>(unanalyzableCondition));
                    } else if (auto *ident = ast::cast_tree<ast::Ident>(rescueCase->var.get())) {
                        auto lhs = global2Local(cctx.ctx, ident->symbol, cctx.inWhat, cctx.aliases);
                        caseBody->exprs.emplace_back(lhs, a->loc, make_unique<Ident>(unanalyzableCondition));
                    } else {
                        Error::raise("Invalid rescue var type");
                    }

                    caseBody = walk(cctx, rescueCase->body.get(), caseBody);
                    if (ret == cctx.inWhat.deadBlock()) {
                        // If the main body -> else -> ret path is dead, we need
                        // to have a path out of the exception handlers
                        ret = cctx.inWhat.freshBlock(cctx.loops, a->loc, caseBody);
                    }
                    unconditionalJump(caseBody, ret, cctx.inWhat, a->loc);
                }

                // None of the handlers were taken
                jumpToDead(rescueHandlersBlock, cctx.inWhat, a->loc);
            },

            [&](ast::Hash *h) {
                vector<core::LocalVariable> vars;
                for (int i = 0; i < h->keys.size(); i++) {
                    core::LocalVariable keyTmp = cctx.ctx.state.newTemporary(
                        core::UniqueNameKind::CFG, core::Names::hashTemp(), cctx.inWhat.symbol);
                    core::LocalVariable valTmp = cctx.ctx.state.newTemporary(
                        core::UniqueNameKind::CFG, core::Names::hashTemp(), cctx.inWhat.symbol);
                    current = walk(cctx.withTarget(keyTmp), h->keys[i].get(), current);
                    current = walk(cctx.withTarget(valTmp), h->values[i].get(), current);
                    vars.push_back(keyTmp);
                    vars.push_back(valTmp);
                }
                core::LocalVariable magic =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::hashTemp(), cctx.inWhat.symbol);
                current->exprs.emplace_back(magic, h->loc, make_unique<Alias>(cctx.ctx.state.defn_Magic()));
                current->exprs.emplace_back(cctx.target, h->loc,
                                            make_unique<Send>(magic, core::Names::buildHash(), vars));
                ret = current;
            },

            [&](ast::Array *a) {
                vector<core::LocalVariable> vars;
                for (auto &elem : a->elems) {
                    core::LocalVariable tmp = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                          core::Names::arrayTemp(), cctx.inWhat.symbol);
                    current = walk(cctx.withTarget(tmp), elem.get(), current);
                    vars.push_back(tmp);
                }
                core::LocalVariable magic = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                        core::Names::arrayTemp(), cctx.inWhat.symbol);
                current->exprs.emplace_back(magic, a->loc, make_unique<Alias>(cctx.ctx.state.defn_Magic()));
                current->exprs.emplace_back(cctx.target, a->loc,
                                            make_unique<Send>(magic, core::Names::buildArray(), vars));
                ret = current;
            },

            [&](ast::EmptyTree *n) { ret = current; },

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
