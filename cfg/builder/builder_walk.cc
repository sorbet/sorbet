#include "builder.h"
#include "core/Names/cfg.h"
#include "core/errors/cfg.h"
#include "core/errors/internal.h"

#include <unordered_map>

using namespace std;

namespace ruby_typer {
namespace cfg {

void conditionalJump(BasicBlock *from, core::LocalVariable cond, BasicBlock *thenb, BasicBlock *elseb, CFG &inWhat,
                     core::Loc loc) {
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
    if (from != inWhat.deadBlock()) {
        ENFORCE(!from->bexit.isCondSet(), "condition for block already set");
        ENFORCE(from->bexit.thenb == nullptr, "thenb already set");
        ENFORCE(from->bexit.elseb == nullptr, "elseb already set");
        from->bexit.cond = core::NameRef::noName();
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
        from->bexit.cond = core::NameRef::noName();
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
    ENFORCE(!current->bexit.isCondSet() || current == cctx.inWhat.deadBlock(),
            "current block has already been finalized!");

    try {
        BasicBlock *ret = nullptr;
        typecase(
            what,
            [&](ast::While *a) {
                auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1, current);
                unconditionalJump(current, headerBlock, cctx.inWhat, a->loc);

                core::LocalVariable condSym = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                          core::Names::whileTemp(), cctx.inWhat.symbol);
                auto headerEnd = walk(cctx.withTarget(condSym), a->cond.get(), headerBlock);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1, headerEnd);
                auto continueBlock = cctx.inWhat.freshBlock(cctx.loops, headerEnd);
                conditionalJump(headerEnd, condSym, bodyBlock, continueBlock, cctx.inWhat, a->cond->loc);
                // finishHeader
                core::LocalVariable bodySym =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::statTemp(), cctx.inWhat.symbol);

                auto body =
                    walk(cctx.withTarget(bodySym).withLoopScope(headerBlock, continueBlock), a->body.get(), bodyBlock);
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
                ENFORCE(ifSym.exists(), "ifSym does not exist");
                auto cont = walk(cctx.withTarget(ifSym), a->cond.get(), current);
                auto thenBlock = cctx.inWhat.freshBlock(cctx.loops, cont);
                auto elseBlock = cctx.inWhat.freshBlock(cctx.loops, cont);
                conditionalJump(cont, ifSym, thenBlock, elseBlock, cctx.inWhat, a->cond->loc);

                auto thenEnd = walk(cctx, a->thenp.get(), thenBlock);
                auto elseEnd = walk(cctx, a->elsep.get(), elseBlock);
                if (thenEnd != cctx.inWhat.deadBlock() || elseEnd != cctx.inWhat.deadBlock()) {
                    if (thenEnd == cctx.inWhat.deadBlock()) {
                        ret = elseEnd;
                    } else if (thenEnd == cctx.inWhat.deadBlock()) {
                        ret = thenEnd;
                    } else {
                        ret = cctx.inWhat.freshBlock(cctx.loops, thenEnd); // could be elseEnd
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
                    Error::raise("should never be reached");
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
                            Error::raise("Should have been removed by namer");
                        }
                    }

                    conditionalJump(headerBlock, core::LocalVariable(core::Names::blockCall()), bodyBlock, postBlock,
                                    cctx.inWhat, s->loc);

                    unconditionalJump(current, headerBlock, cctx.inWhat, s->loc);

                    // TODO: handle block arguments somehow??
                    core::LocalVariable blockrv = cctx.ctx.state.newTemporary(
                        core::UniqueNameKind::CFG, core::Names::blockReturnTemp(), cctx.inWhat.symbol);
                    auto blockLast = walk(cctx.withTarget(blockrv).withLoopScope(headerBlock, postBlock),
                                          s->block->body.get(), bodyBlock);

                    unconditionalJump(blockLast, headerBlock, cctx.inWhat, s->loc);

                    current = postBlock;
                }

                current->exprs.emplace_back(cctx.target, s->loc, make_unique<Send>(recv, s->fun, args));
                ret = current;
            },

            [&](ast::Block *a) { Error::raise("should never encounter a bare Block"); },

            [&](ast::Next *a) {
                core::LocalVariable exprSym = cctx.ctx.state.newTemporary(
                    core::UniqueNameKind::CFG, core::Names::returnTemp(), cctx.inWhat.symbol);
                auto afterNext = walk(cctx.withTarget(exprSym), a->expr.get(), current);

                if (cctx.nextScope == nullptr) {
                    cctx.ctx.state.errors.error(a->loc, core::errors::CFG::NoNextScope, "No `do` block around `next`");
                    // I guess just keep going into deadcode?
                    unconditionalJump(afterNext, cctx.inWhat.deadBlock(), cctx.inWhat, a->loc);
                } else {
                    unconditionalJump(afterNext, cctx.nextScope, cctx.inWhat, a->loc);
                }

                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Break *a) {
                core::LocalVariable exprSym = cctx.ctx.state.newTemporary(
                    core::UniqueNameKind::CFG, core::Names::returnTemp(), cctx.inWhat.symbol);
                auto afterBreak = walk(cctx.withTarget(exprSym), a->expr.get(), current);

                if (cctx.breakScope == nullptr) {
                    cctx.ctx.state.errors.error(a->loc, core::errors::CFG::NoNextScope, "No `do` block around `break`");
                    // I guess just keep going into deadcode?
                    unconditionalJump(afterBreak, cctx.inWhat.deadBlock(), cctx.inWhat, a->loc);
                } else {
                    unconditionalJump(afterBreak, cctx.breakScope, cctx.inWhat, a->loc);
                }
                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Retry *a) {
                if (cctx.rescueScope == nullptr) {
                    cctx.ctx.state.errors.error(a->loc, core::errors::CFG::NoNextScope,
                                                "No `begin` block around `retry`");
                    // I guess just keep going into deadcode?
                    unconditionalJump(current, cctx.inWhat.deadBlock(), cctx.inWhat, a->loc);
                } else {
                    unconditionalJump(current, cctx.rescueScope, cctx.inWhat, a->loc);
                }
                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Rescue *a) {
                auto rescueStartBlock = cctx.inWhat.freshBlock(cctx.loops, current);
                auto unanalyzableCondition = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG,
                                                                         core::Names::rescueTemp(), cctx.inWhat.symbol);
                rescueStartBlock->exprs.emplace_back(unanalyzableCondition, what->loc, make_unique<Unanalyzable>());
                unconditionalJump(current, rescueStartBlock, cctx.inWhat, a->loc);
                cctx.rescueScope = rescueStartBlock;

                auto rescueHandlersBlock = cctx.inWhat.freshBlock(cctx.loops, rescueStartBlock);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops, rescueStartBlock);
                conditionalJump(rescueStartBlock, unanalyzableCondition, rescueHandlersBlock, bodyBlock, cctx.inWhat,
                                a->loc);

                bodyBlock = walk(cctx, a->body.get(), bodyBlock);
                auto elseBody = cctx.inWhat.freshBlock(cctx.loops, bodyBlock);
                unconditionalJump(bodyBlock, elseBody, cctx.inWhat, a->loc);

                elseBody = walk(cctx, a->else_.get(), elseBody);
                auto ensureBody = cctx.inWhat.freshBlock(cctx.loops, elseBody);
                unconditionalJump(elseBody, ensureBody, cctx.inWhat, a->loc);

                auto throwAway = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::rescueTemp(),
                                                             cctx.inWhat.symbol);
                ensureBody = walk(cctx.withTarget(throwAway), a->ensure.get(), ensureBody);
                ret = cctx.inWhat.freshBlock(cctx.loops, ensureBody);
                unconditionalJump(ensureBody, ret, cctx.inWhat, a->loc);

                for (auto &rescueCase : a->rescueCases) {
                    auto caseBody = cctx.inWhat.freshBlock(cctx.loops, rescueHandlersBlock);
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

                        auto otherHandlerBlock = cctx.inWhat.freshBlock(cctx.loops, rescueHandlersBlock);
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
                        ret = cctx.inWhat.freshBlock(cctx.loops, caseBody);
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

            [&](ast::Cast *c) {
                core::LocalVariable tmp =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::castTemp(), cctx.inWhat.symbol);
                current = walk(cctx.withTarget(tmp), c->arg.get(), current);
                current->exprs.emplace_back(cctx.target, c->loc, make_unique<Cast>(tmp, c->type, c->assertType));
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
    } catch (...) {
        cctx.ctx.state.errors.error(what->loc, core::errors::Internal::InternalError,
                                    "Failed to convert tree to CFG (backtrace is above )");
        throw;
    }
}

} // namespace cfg
} // namespace ruby_typer
