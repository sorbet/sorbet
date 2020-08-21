#include "ast/ArgParsing.h"
#include "ast/Helpers.h"
#include "cfg/builder/builder.h"
#include "common/typecase.h"
#include "core/Names.h"
#include "core/errors/cfg.h"
#include "core/errors/internal.h"

using namespace std;

namespace sorbet::cfg {

void CFGBuilder::conditionalJump(BasicBlock *from, LocalRef cond, BasicBlock *thenb, BasicBlock *elseb, CFG &inWhat,
                                 core::LocOffsets loc) {
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

void CFGBuilder::unconditionalJump(BasicBlock *from, BasicBlock *to, CFG &inWhat, core::LocOffsets loc) {
    to->flags |= CFG::WAS_JUMP_DESTINATION;
    if (from != inWhat.deadBlock()) {
        ENFORCE(!from->bexit.isCondSet(), "condition for block already set");
        ENFORCE(from->bexit.thenb == nullptr, "thenb already set");
        ENFORCE(from->bexit.elseb == nullptr, "elseb already set");
        from->bexit.cond = LocalRef::unconditional();
        from->bexit.elseb = to;
        from->bexit.thenb = to;
        from->bexit.loc = loc;
        to->backEdges.emplace_back(from);
    }
}

namespace {

LocalRef global2Local(CFGContext cctx, core::SymbolRef what) {
    // Note: this will add an empty local to aliases if 'what' is not there
    LocalRef &alias = cctx.aliases[what];
    if (!alias.exists()) {
        const core::SymbolData data = what.data(cctx.ctx);
        alias = cctx.newTemporary(data->name);
    }
    return alias;
}

LocalRef unresolvedIdent2Local(CFGContext cctx, ast::UnresolvedIdent *id) {
    core::SymbolRef klass;

    switch (id->kind) {
        case ast::UnresolvedIdent::Kind::Class:
            klass = cctx.ctx.owner.data(cctx.ctx)->enclosingClass(cctx.ctx);
            while (klass.data(cctx.ctx)->attachedClass(cctx.ctx).exists()) {
                klass = klass.data(cctx.ctx)->attachedClass(cctx.ctx);
            }
            break;
        case ast::UnresolvedIdent::Kind::Instance:
            ENFORCE(cctx.ctx.owner.data(cctx.ctx)->isMethod());
            klass = cctx.ctx.owner.data(cctx.ctx)->owner;
            break;
        case ast::UnresolvedIdent::Kind::Global:
            klass = core::Symbols::root();
            break;
        default:
            // These should have been removed in the namer
            Exception::notImplemented();
    }
    ENFORCE(klass.data(cctx.ctx)->isClassOrModule());

    auto sym = klass.data(cctx.ctx)->findMemberTransitive(cctx.ctx, id->name);
    if (!sym.exists()) {
        auto fnd = cctx.discoveredUndeclaredFields.find(id->name);
        if (fnd == cctx.discoveredUndeclaredFields.end()) {
            if (id->kind != ast::UnresolvedIdent::Kind::Global) {
                if (auto e = cctx.ctx.beginError(id->loc, core::errors::CFG::UndeclaredVariable)) {
                    e.setHeader("Use of undeclared variable `{}`", id->name.show(cctx.ctx));
                }
            }
            auto ret = cctx.newTemporary(id->name);
            cctx.discoveredUndeclaredFields[id->name] = ret;
            return ret;
        }
        return fnd->second;
    } else {
        return global2Local(cctx, sym);
    }
}

} // namespace

void CFGBuilder::jumpToDead(BasicBlock *from, CFG &inWhat, core::LocOffsets loc) {
    auto *db = inWhat.deadBlock();
    if (from != db) {
        ENFORCE(!from->bexit.isCondSet(), "condition for block already set");
        ENFORCE(from->bexit.thenb == nullptr, "thenb already set");
        ENFORCE(from->bexit.elseb == nullptr, "elseb already set");
        from->bexit.cond = LocalRef::unconditional();
        from->bexit.elseb = db;
        from->bexit.thenb = db;
        from->bexit.loc = loc;
        db->backEdges.emplace_back(from);
    }
}

void CFGBuilder::synthesizeExpr(BasicBlock *bb, LocalRef var, core::LocOffsets loc, unique_ptr<Instruction> inst) {
    auto &inserted = bb->exprs.emplace_back(var, loc, move(inst));
    inserted.value->isSynthetic = true;
}

BasicBlock *CFGBuilder::walkHash(CFGContext cctx, ast::Hash *h, BasicBlock *current, core::NameRef method) {
    InlinedVector<cfg::LocalRef, 2> vars;
    InlinedVector<core::LocOffsets, 2> locs;
    for (int i = 0; i < h->keys.size(); i++) {
        LocalRef keyTmp = cctx.newTemporary(core::Names::hashTemp());
        LocalRef valTmp = cctx.newTemporary(core::Names::hashTemp());
        current = walk(cctx.withTarget(keyTmp), h->keys[i].get(), current);
        current = walk(cctx.withTarget(valTmp), h->values[i].get(), current);
        vars.emplace_back(keyTmp);
        vars.emplace_back(valTmp);
        locs.emplace_back(h->keys[i]->loc);
        locs.emplace_back(h->values[i]->loc);
    }
    LocalRef magic = cctx.newTemporary(core::Names::magic());
    synthesizeExpr(current, magic, core::LocOffsets::none(), make_unique<Alias>(core::Symbols::Magic()));

    auto isPrivateOk = false;
    current->exprs.emplace_back(cctx.target, h->loc, make_unique<Send>(magic, method, h->loc, vars, locs, isPrivateOk));
    return current;
}

BasicBlock *CFGBuilder::joinBlocks(CFGContext cctx, BasicBlock *a, BasicBlock *b) {
    auto *join = cctx.inWhat.freshBlock(cctx.loops, a->rubyBlockId);
    unconditionalJump(a, join, cctx.inWhat, core::LocOffsets::none());
    unconditionalJump(b, join, cctx.inWhat, core::LocOffsets::none());
    return join;
}

tuple<LocalRef, BasicBlock *, BasicBlock *> CFGBuilder::walkDefault(CFGContext cctx, int argIndex,
                                                                    const core::ArgInfo &argInfo, LocalRef argLocal,
                                                                    core::LocOffsets argLoc, ast::TreePtr &def,
                                                                    BasicBlock *presentCont, BasicBlock *defaultCont) {
    auto defLoc = def->loc;

    auto *presentNext = cctx.inWhat.freshBlock(cctx.loops, presentCont->rubyBlockId);
    auto *defaultNext = cctx.inWhat.freshBlock(cctx.loops, presentCont->rubyBlockId);

    auto present = cctx.newTemporary(core::Names::argPresent());
    auto methodSymbol = cctx.inWhat.symbol;
    synthesizeExpr(presentCont, present, core::LocOffsets::none(), make_unique<ArgPresent>(methodSymbol, argIndex));
    conditionalJump(presentCont, present, presentNext, defaultNext, cctx.inWhat, argLoc);

    if (defaultCont != nullptr) {
        unconditionalJump(defaultCont, defaultNext, cctx.inWhat, core::LocOffsets::none());
    }

    // Walk the default, and check the type of its final value
    auto result = cctx.newTemporary(core::Names::statTemp());
    defaultNext = walk(cctx.withTarget(result), *def, defaultNext);

    if (argInfo.type != nullptr) {
        auto tmp = cctx.newTemporary(core::Names::castTemp());
        synthesizeExpr(defaultNext, tmp, defLoc, make_unique<Cast>(result, argInfo.type, core::Names::let()));
        cctx.inWhat.minLoops[tmp.id()] = CFG::MIN_LOOP_LET;
    }

    return {result, presentNext, defaultNext};
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
                auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1, current->rubyBlockId);
                // breakNotCalledBlock is only entered if break is not called in
                // the loop body
                auto breakNotCalledBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyBlockId);
                auto continueBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyBlockId);
                unconditionalJump(current, headerBlock, cctx.inWhat, a->loc);

                LocalRef condSym = cctx.newTemporary(core::Names::whileTemp());
                auto headerEnd = walk(cctx.withTarget(condSym).withLoopScope(headerBlock, continueBlock), a->cond.get(),
                                      headerBlock);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1, current->rubyBlockId);
                conditionalJump(headerEnd, condSym, bodyBlock, breakNotCalledBlock, cctx.inWhat, a->cond->loc);
                // finishHeader
                LocalRef bodySym = cctx.newTemporary(core::Names::statTemp());

                auto body = walk(cctx.withTarget(bodySym)
                                     .withLoopScope(headerBlock, continueBlock)
                                     .withBlockBreakTarget(cctx.target),
                                 a->body.get(), bodyBlock);
                unconditionalJump(body, headerBlock, cctx.inWhat, a->loc);

                synthesizeExpr(breakNotCalledBlock, cctx.target, a->loc, make_unique<Literal>(core::Types::nilClass()));
                unconditionalJump(breakNotCalledBlock, continueBlock, cctx.inWhat, a->loc);
                ret = continueBlock;

                /*
                 * This code:
                 *
                 *     a = while cond; break b; end
                 *
                 * generates this CFG:
                 *
                 *   ┌──▶ Loop Header ──────┐
                 *   │      │               │
                 *   │      │               ▼
                 *   │      ▼        breakNotCalledBlock
                 *   └─ Loop Body         a = nil
                 *          │               │
                 *        a = b             │
                 *          │               │
                 *          ▼               │
                 *    continueBlock ◀──────-┘
                 *
                 */
            },
            [&](ast::Return *a) {
                LocalRef retSym = cctx.newTemporary(core::Names::returnTemp());
                auto cont = walk(cctx.withTarget(retSym), a->expr.get(), current);
                cont->exprs.emplace_back(cctx.target, a->loc, make_unique<Return>(retSym)); // dead assign.
                jumpToDead(cont, cctx.inWhat, a->loc);
                ret = cctx.inWhat.deadBlock();
            },
            [&](ast::If *a) {
                LocalRef ifSym = cctx.newTemporary(core::Names::ifTemp());
                ENFORCE(ifSym.exists(), "ifSym does not exist");
                auto cont = walk(cctx.withTarget(ifSym), a->cond.get(), current);
                auto thenBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyBlockId);
                auto elseBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyBlockId);
                conditionalJump(cont, ifSym, thenBlock, elseBlock, cctx.inWhat, a->cond->loc);

                auto thenEnd = walk(cctx, a->thenp.get(), thenBlock);
                auto elseEnd = walk(cctx, a->elsep.get(), elseBlock);
                if (thenEnd != cctx.inWhat.deadBlock() || elseEnd != cctx.inWhat.deadBlock()) {
                    if (thenEnd == cctx.inWhat.deadBlock()) {
                        ret = elseEnd;
                    } else if (thenEnd == cctx.inWhat.deadBlock()) {
                        ret = thenEnd;
                    } else {
                        ret = cctx.inWhat.freshBlock(cctx.loops, current->rubyBlockId);
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
            [&](ast::UnresolvedIdent *id) {
                LocalRef loc = unresolvedIdent2Local(cctx, id);
                ENFORCE(loc.exists());
                current->exprs.emplace_back(cctx.target, id->loc, make_unique<Ident>(loc));

                ret = current;
            },
            [&](ast::UnresolvedConstantLit *a) { Exception::raise("Should have been eliminated by namer/resolver"); },
            [&](ast::ConstantLit *a) {
                auto aliasName = cctx.newTemporary(core::Names::cfgAlias());
                if (a->symbol == core::Symbols::StubModule()) {
                    current->exprs.emplace_back(aliasName, a->loc, make_unique<Alias>(core::Symbols::untyped()));
                } else {
                    current->exprs.emplace_back(aliasName, a->loc, make_unique<Alias>(a->symbol));
                }

                synthesizeExpr(current, cctx.target, a->loc, make_unique<Ident>(aliasName));

                if (a->original) {
                    auto *orig = ast::cast_tree<ast::UnresolvedConstantLit>(a->original);
                    if (auto nested = ast::cast_tree<ast::ConstantLit>(orig->scope)) {
                        LocalRef deadSym = cctx.newTemporary(core::Names::keepForIde());
                        current = walk(cctx.withTarget(deadSym), nested, current);
                    }
                }

                ret = current;
            },
            [&](ast::Local *a) {
                current->exprs.emplace_back(cctx.target, a->loc,
                                            make_unique<Ident>(cctx.inWhat.enterLocal(a->localVariable)));
                ret = current;
            },
            [&](ast::Assign *a) {
                LocalRef lhs;
                if (auto lhsIdent = ast::cast_tree<ast::ConstantLit>(a->lhs)) {
                    lhs = global2Local(cctx, lhsIdent->symbol);
                } else if (auto lhsLocal = ast::cast_tree<ast::Local>(a->lhs)) {
                    lhs = cctx.inWhat.enterLocal(lhsLocal->localVariable);
                } else if (auto ident = ast::cast_tree<ast::UnresolvedIdent>(a->lhs)) {
                    lhs = unresolvedIdent2Local(cctx, ident);
                    ENFORCE(lhs.exists());
                } else {
                    Exception::raise("should never be reached");
                }

                auto rhsCont = walk(cctx.withTarget(lhs), a->rhs.get(), current);
                rhsCont->exprs.emplace_back(cctx.target, a->loc, make_unique<Ident>(lhs));
                ret = rhsCont;
            },
            [&](ast::InsSeq *a) {
                for (auto &exp : a->stats) {
                    LocalRef temp = cctx.newTemporary(core::Names::statTemp());
                    current = walk(cctx.withTarget(temp), exp.get(), current);
                }
                ret = walk(cctx, a->expr.get(), current);
            },
            [&](ast::Send *s) {
                LocalRef recv;

                if (s->fun == core::Names::absurd()) {
                    if (auto cnst = ast::cast_tree<ast::ConstantLit>(s->recv)) {
                        if (cnst->symbol == core::Symbols::T()) {
                            if (s->args.size() != 1) {
                                if (auto e = cctx.ctx.beginError(s->loc, core::errors::CFG::MalformedTAbsurd)) {
                                    e.setHeader("`{}` expects exactly one argument but got `{}`", "T.absurd",
                                                s->args.size());
                                }
                                ret = current;
                                return;
                            }

                            if (ast::isa_tree<ast::Send>(s->args[0])) {
                                // Providing a send is the most common way T.absurd is misused
                                if (auto e = cctx.ctx.beginError(s->loc, core::errors::CFG::MalformedTAbsurd)) {
                                    e.setHeader("`{}` expects to be called on a variable, not a method call",
                                                "T.absurd", s->args.size());
                                }
                                ret = current;
                                return;
                            }

                            auto temp = cctx.newTemporary(core::Names::statTemp());
                            current = walk(cctx.withTarget(temp), s->args[0].get(), current);
                            current->exprs.emplace_back(cctx.target, s->loc, make_unique<TAbsurd>(temp));
                            ret = current;
                            return;
                        }
                    }
                }

                recv = cctx.newTemporary(core::Names::statTemp());
                current = walk(cctx.withTarget(recv), s->recv.get(), current);

                InlinedVector<LocalRef, 2> args;
                InlinedVector<core::LocOffsets, 2> argLocs;
                int argIdx = -1;
                for (auto &exp : s->args) {
                    argIdx++;
                    LocalRef temp;
                    temp = cctx.newTemporary(core::Names::statTemp());
                    if (argIdx + 1 == s->args.size() && ast::isa_tree<ast::Hash>(exp)) {
                        current = walkHash(cctx.withTarget(temp), ast::cast_tree<ast::Hash>(exp), current,
                                           core::Names::buildKeywordArgs());
                    } else {
                        current = walk(cctx.withTarget(temp), exp.get(), current);
                    }

                    args.emplace_back(temp);
                    argLocs.emplace_back(exp->loc);
                }

                if (s->block != nullptr) {
                    auto newRubyBlockId = ++cctx.inWhat.maxRubyBlockId;
                    vector<ast::ParsedArg> blockArgs =
                        ast::ArgParsing::parseArgs(ast::cast_tree<ast::Block>(s->block)->args);
                    vector<core::ArgInfo::ArgFlags> argFlags;
                    for (auto &e : blockArgs) {
                        auto &target = argFlags.emplace_back();
                        target.isKeyword = e.flags.isKeyword;
                        target.isRepeated = e.flags.isRepeated;
                        target.isDefault = e.flags.isDefault;
                        target.isShadow = e.flags.isShadow;
                    }
                    auto link = make_shared<core::SendAndBlockLink>(s->fun, move(argFlags), newRubyBlockId);
                    auto send =
                        make_unique<Send>(recv, s->fun, s->recv->loc, args, argLocs, !!s->flags.isPrivateOk, link);
                    LocalRef sendTemp = cctx.newTemporary(core::Names::blockPreCallTemp());
                    auto solveConstraint = make_unique<SolveConstraint>(link, sendTemp);
                    current->exprs.emplace_back(sendTemp, s->loc, move(send));
                    LocalRef restoreSelf = cctx.newTemporary(core::Names::selfRestore());
                    synthesizeExpr(current, restoreSelf, core::LocOffsets::none(),
                                   make_unique<Ident>(LocalRef::selfVariable()));

                    auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1, newRubyBlockId);
                    // solveConstraintBlock is only entered if break is not called
                    // in the block body.
                    auto solveConstraintBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyBlockId);
                    auto postBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyBlockId);
                    auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1, newRubyBlockId);

                    LocalRef argTemp = cctx.newTemporary(core::Names::blkArg());
                    LocalRef idxTmp = cctx.newTemporary(core::Names::blkArg());
                    bodyBlock->exprs.emplace_back(LocalRef::selfVariable(), s->loc,
                                                  make_unique<LoadSelf>(link, LocalRef::selfVariable()));
                    bodyBlock->exprs.emplace_back(argTemp, s->block->loc, make_unique<LoadYieldParams>(link));

                    for (int i = 0; i < blockArgs.size(); ++i) {
                        auto &arg = blockArgs[i];
                        LocalRef argLoc = cctx.inWhat.enterLocal(arg.local);

                        if (arg.flags.isRepeated) {
                            if (i != 0) {
                                // Mixing positional and rest args in blocks is
                                // not currently supported; drop in an untyped.
                                bodyBlock->exprs.emplace_back(argLoc, arg.loc,
                                                              make_unique<Alias>(core::Symbols::untyped()));
                            } else {
                                bodyBlock->exprs.emplace_back(argLoc, arg.loc, make_unique<Ident>(argTemp));
                            }
                            continue;
                        }

                        // Inserting a statement that does not directly map to any source text. Make its loc
                        // 0-length so LSP ignores it in queries.
                        core::LocOffsets zeroLengthLoc = arg.loc.copyWithZeroLength();
                        bodyBlock->exprs.emplace_back(
                            idxTmp, zeroLengthLoc,
                            make_unique<Literal>(core::make_type<core::LiteralType>(int64_t(i))));
                        InlinedVector<LocalRef, 2> idxVec{idxTmp};
                        InlinedVector<core::LocOffsets, 2> locs{zeroLengthLoc};
                        auto isPrivateOk = false;
                        bodyBlock->exprs.emplace_back(argLoc, arg.loc,
                                                      make_unique<Send>(argTemp, core::Names::squareBrackets(),
                                                                        s->block->loc, idxVec, locs, isPrivateOk));
                    }

                    conditionalJump(headerBlock, LocalRef::blockCall(), bodyBlock, solveConstraintBlock, cctx.inWhat,
                                    s->loc);

                    unconditionalJump(current, headerBlock, cctx.inWhat, s->loc);

                    LocalRef blockrv = cctx.newTemporary(core::Names::blockReturnTemp());
                    auto blockLast = walk(cctx.withTarget(blockrv)
                                              .withBlockBreakTarget(cctx.target)
                                              .withLoopScope(headerBlock, postBlock, true)
                                              .withSendAndBlockLink(link),
                                          ast::cast_tree<ast::Block>(s->block)->body.get(), bodyBlock);
                    if (blockLast != cctx.inWhat.deadBlock()) {
                        LocalRef dead = cctx.newTemporary(core::Names::blockReturnTemp());
                        synthesizeExpr(blockLast, dead, s->block->loc, make_unique<BlockReturn>(link, blockrv));
                    }

                    unconditionalJump(blockLast, headerBlock, cctx.inWhat, s->loc);
                    unconditionalJump(solveConstraintBlock, postBlock, cctx.inWhat, s->loc);

                    solveConstraintBlock->exprs.emplace_back(cctx.target, s->loc, move(solveConstraint));
                    current = postBlock;
                    synthesizeExpr(current, LocalRef::selfVariable(), s->loc, make_unique<Ident>(restoreSelf));

                    /*
                     * This code:
                     *
                     *     a = while cond; break b; end
                     *
                     * generates this CFG:
                     *
                     *   ┌──▶ headerBlock ──────┐
                     *   │      │               │
                     *   │      │               │
                     *   │      ▼               │
                     *   └─ Block Body          ▼
                     *          │    a = solveConstraintBlock
                     *        a = b             │
                     *          │               │
                     *          ▼               │
                     *      Post Block ◀───────-┘
                     *
                     */
                } else {
                    current->exprs.emplace_back(
                        cctx.target, s->loc,
                        make_unique<Send>(recv, s->fun, s->recv->loc, args, argLocs, !!s->flags.isPrivateOk));
                }

                ret = current;
            },

            [&](ast::Block *a) { Exception::raise("should never encounter a bare Block"); },

            [&](ast::Next *a) {
                LocalRef exprSym = cctx.newTemporary(core::Names::nextTemp());
                auto afterNext = walk(cctx.withTarget(exprSym), a->expr.get(), current);
                if (afterNext != cctx.inWhat.deadBlock() && cctx.isInsideRubyBlock) {
                    LocalRef dead = cctx.newTemporary(core::Names::nextTemp());
                    ENFORCE(cctx.link.get() != nullptr);
                    afterNext->exprs.emplace_back(dead, a->loc, make_unique<BlockReturn>(cctx.link, exprSym));
                }

                if (cctx.nextScope == nullptr) {
                    if (auto e = cctx.ctx.beginError(a->loc, core::errors::CFG::NoNextScope)) {
                        e.setHeader("No `{}` block around `{}`", "do", "next");
                    }
                    // I guess just keep going into deadcode?
                    unconditionalJump(afterNext, cctx.inWhat.deadBlock(), cctx.inWhat, a->loc);
                } else {
                    unconditionalJump(afterNext, cctx.nextScope, cctx.inWhat, a->loc);
                }

                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Break *a) {
                LocalRef exprSym = cctx.newTemporary(core::Names::returnTemp());
                auto afterBreak = walk(cctx.withTarget(exprSym), a->expr.get(), current);

                // Here, since cctx.blockBreakTarget refers to something outside of the block,
                // it will show up on the pinned variables list (with type of NilClass).
                // Then, since we are assigning to it at a higher loop level, we throw a
                // "changing type in loop" error.

                // To get around this, we first assign to a
                // temporary blockBreakAssign variable, and then assign blockBreakAssign to
                // cctx.blockBreakTarget. This allows us to silence this error, if the RHS is
                // a variable of type "blockBreakAssign". You can find the silencing code in
                // infer/environment.cc, if you search for "== core::Names::blockBreakAssign()".

                // This is a temporary hack until we change how pining works to handle this case.
                auto blockBreakAssign = cctx.newTemporary(core::Names::blockBreakAssign());
                afterBreak->exprs.emplace_back(blockBreakAssign, a->loc, make_unique<Ident>(exprSym));
                afterBreak->exprs.emplace_back(cctx.blockBreakTarget, a->loc, make_unique<Ident>(blockBreakAssign));

                // call intrinsic for break
                auto magic = cctx.newTemporary(core::Names::magic());
                auto ignored = cctx.newTemporary(core::Names::blockBreak());
                synthesizeExpr(afterBreak, magic, a->loc, make_unique<Alias>(core::Symbols::Magic()));
                InlinedVector<LocalRef, 2> args{exprSym};
                InlinedVector<core::LocOffsets, 2> locs{core::LocOffsets::none()};
                auto isPrivateOk = false;

                synthesizeExpr(afterBreak, ignored, core::LocOffsets::none(),
                               make_unique<Send>(magic, core::Names::blockBreak(), core::LocOffsets::none(), args, locs,
                                                 isPrivateOk));

                if (cctx.breakScope == nullptr) {
                    if (auto e = cctx.ctx.beginError(a->loc, core::errors::CFG::NoNextScope)) {
                        e.setHeader("No `{}` block around `{}`", "do", "break");
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
                    if (auto e = cctx.ctx.beginError(a->loc, core::errors::CFG::NoNextScope)) {
                        e.setHeader("No `{}` block around `{}`", "begin", "retry");
                    }
                    // I guess just keep going into deadcode?
                    unconditionalJump(current, cctx.inWhat.deadBlock(), cctx.inWhat, a->loc);
                } else {
                    auto magic = cctx.newTemporary(core::Names::magic());
                    synthesizeExpr(current, magic, core::LocOffsets::none(),
                                   make_unique<Alias>(core::Symbols::Magic()));
                    auto retryTemp = cctx.newTemporary(core::Names::retryTemp());
                    InlinedVector<cfg::LocalRef, 2> args{};
                    InlinedVector<core::LocOffsets, 2> argLocs{};
                    auto isPrivateOk = false;
                    synthesizeExpr(
                        current, retryTemp, core::LocOffsets::none(),
                        make_unique<Send>(magic, core::Names::retry(), what->loc, args, argLocs, isPrivateOk));
                    unconditionalJump(current, cctx.rescueScope, cctx.inWhat, a->loc);
                }
                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Rescue *a) {
                auto bodyRubyBlockId = ++cctx.inWhat.maxRubyBlockId;
                auto handlersRubyBlockId = bodyRubyBlockId + CFG::HANDLERS_BLOCK_OFFSET;
                auto ensureRubyBlockId = bodyRubyBlockId + CFG::ENSURE_BLOCK_OFFSET;
                auto elseRubyBlockId = bodyRubyBlockId + CFG::ELSE_BLOCK_OFFSET;
                cctx.inWhat.maxRubyBlockId = elseRubyBlockId;

                auto rescueHeaderBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyBlockId);
                unconditionalJump(current, rescueHeaderBlock, cctx.inWhat, a->loc);
                cctx.rescueScope = rescueHeaderBlock;

                // We have a simplified view of the control flow here but in
                // practise it has been reasonable on our codebase.
                // We don't model that each expression in the `body` or `else` could
                // throw, instead we model only never running anything in the
                // body, or running the whole thing. To do this we  have a magic
                // Unanalyzable variable at the top of the body using
                // `exceptionValue` and one at the end of the else using
                // `rescueEndTemp` which can jump into the rescue handlers.
                auto rescueHandlersBlock = cctx.inWhat.freshBlock(cctx.loops, handlersRubyBlockId);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops, bodyRubyBlockId);
                auto exceptionValue = cctx.newTemporary(core::Names::exceptionValue());
                synthesizeExpr(rescueHeaderBlock, exceptionValue, what->loc, make_unique<GetCurrentException>());
                conditionalJump(rescueHeaderBlock, exceptionValue, rescueHandlersBlock, bodyBlock, cctx.inWhat, a->loc);

                // cctx.loops += 1; // should formally be here but this makes us report a lot of false errors
                bodyBlock = walk(cctx, a->body.get(), bodyBlock);

                // else is only executed if body didn't raise an exception
                auto elseBody = cctx.inWhat.freshBlock(cctx.loops, elseRubyBlockId);
                synthesizeExpr(bodyBlock, exceptionValue, what->loc, make_unique<GetCurrentException>());
                conditionalJump(bodyBlock, exceptionValue, rescueHandlersBlock, elseBody, cctx.inWhat, a->loc);

                elseBody = walk(cctx, a->else_.get(), elseBody);
                auto ensureBody = cctx.inWhat.freshBlock(cctx.loops, ensureRubyBlockId);
                unconditionalJump(elseBody, ensureBody, cctx.inWhat, a->loc);

                auto magic = cctx.newTemporary(core::Names::magic());
                synthesizeExpr(current, magic, core::LocOffsets::none(), make_unique<Alias>(core::Symbols::Magic()));

                for (auto &expr : a->rescueCases) {
                    auto *rescueCase = ast::cast_tree<ast::RescueCase>(expr);
                    auto caseBody = cctx.inWhat.freshBlock(cctx.loops, handlersRubyBlockId);
                    auto &exceptions = rescueCase->exceptions;
                    auto added = false;
                    auto *local = ast::cast_tree<ast::Local>(rescueCase->var);
                    ENFORCE(local != nullptr, "rescue case var not a local?");

                    auto localVar = cctx.inWhat.enterLocal(local->localVariable);
                    rescueHandlersBlock->exprs.emplace_back(localVar, rescueCase->var->loc,
                                                            make_unique<Ident>(exceptionValue));

                    // Mark the exception as handled
                    synthesizeExpr(caseBody, exceptionValue, core::LocOffsets::none(),
                                   make_unique<Literal>(core::Types::nilClass()));

                    auto res = cctx.newTemporary(core::Names::keepForCfgTemp());
                    auto isPrivateOk = false;
                    auto args = {exceptionValue};
                    auto argLocs = {what->loc};
                    synthesizeExpr(caseBody, res, rescueCase->loc,
                                   make_unique<Send>(magic, core::Names::keepForCfg(), rescueCase->loc, args, argLocs,
                                                     isPrivateOk));

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
                        InlinedVector<cfg::LocalRef, 2> args;
                        InlinedVector<core::LocOffsets, 2> argLocs = {loc};
                        args.emplace_back(exceptionClass);

                        auto isPrivateOk = false;
                        rescueHandlersBlock->exprs.emplace_back(
                            isaCheck, loc,
                            make_unique<Send>(localVar, core::Names::isA_p(), loc, args, argLocs, isPrivateOk));

                        auto otherHandlerBlock = cctx.inWhat.freshBlock(cctx.loops, handlersRubyBlockId);
                        conditionalJump(rescueHandlersBlock, isaCheck, caseBody, otherHandlerBlock, cctx.inWhat, loc);
                        rescueHandlersBlock = otherHandlerBlock;
                    }
                    if (added) {
                        exceptions.pop_back();
                    }

                    caseBody = walk(cctx, rescueCase->body.get(), caseBody);
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
                ret = cctx.inWhat.freshBlock(cctx.loops, current->rubyBlockId);
                conditionalJump(ensureBody, gotoDeadTemp, cctx.inWhat.deadBlock(), ret, cctx.inWhat, a->loc);
            },

            [&](ast::Hash *h) { ret = walkHash(cctx, h, current, core::Names::buildHash()); },

            [&](ast::Array *a) {
                InlinedVector<LocalRef, 2> vars;
                InlinedVector<core::LocOffsets, 2> locs;
                for (auto &elem : a->elems) {
                    LocalRef tmp = cctx.newTemporary(core::Names::arrayTemp());
                    current = walk(cctx.withTarget(tmp), elem.get(), current);
                    vars.emplace_back(tmp);
                    locs.emplace_back(a->loc);
                }
                LocalRef magic = cctx.newTemporary(core::Names::magic());
                synthesizeExpr(current, magic, core::LocOffsets::none(), make_unique<Alias>(core::Symbols::Magic()));
                auto isPrivateOk = false;
                current->exprs.emplace_back(
                    cctx.target, a->loc,
                    make_unique<Send>(magic, core::Names::buildArray(), a->loc, vars, locs, isPrivateOk));
                ret = current;
            },

            [&](ast::Cast *c) {
                LocalRef tmp = cctx.newTemporary(core::Names::castTemp());
                current = walk(cctx.withTarget(tmp), c->arg.get(), current);
                if (c->cast == core::Names::uncheckedLet()) {
                    current->exprs.emplace_back(cctx.target, c->loc, make_unique<Ident>(tmp));
                } else {
                    current->exprs.emplace_back(cctx.target, c->loc, make_unique<Cast>(tmp, c->type, c->cast));
                }
                if (c->cast == core::Names::let()) {
                    cctx.inWhat.minLoops[cctx.target.id()] = CFG::MIN_LOOP_LET;
                }

                ret = current;
            },

            [&](ast::EmptyTree *n) { ret = current; },

            [&](ast::ClassDef *c) { Exception::raise("Should have been removed by FlattenWalk"); },
            [&](ast::MethodDef *c) { Exception::raise("Should have been removed by FlattenWalk"); },

            [&](ast::Expression *n) { Exception::raise("Unimplemented AST Node: {}", n->nodeName()); });

        // For, Rescue,
        // Symbol, Array,
        ENFORCE(ret != nullptr, "CFB builder ret unset");
        return ret;
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = cctx.ctx.beginError(what->loc, core::errors::Internal::InternalError)) {
            e.setHeader("Failed to convert tree to CFG (backtrace is above )");
        }
        throw;
    }
}

LocalRef CFGContext::newTemporary(core::NameRef name) {
    return inWhat.enterLocal(core::LocalVariable{name, ++temporaryCounter});
}
} // namespace sorbet::cfg
