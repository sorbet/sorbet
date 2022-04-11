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
        alias = cctx.newTemporary(what.name(cctx.ctx));
    }
    return alias;
}

LocalRef unresolvedIdent2Local(CFGContext cctx, const ast::UnresolvedIdent &id) {
    core::ClassOrModuleRef klass;

    switch (id.kind) {
        case ast::UnresolvedIdent::Kind::Class:
            klass = cctx.ctx.owner.enclosingClass(cctx.ctx);
            while (klass.data(cctx.ctx)->attachedClass(cctx.ctx).exists()) {
                klass = klass.data(cctx.ctx)->attachedClass(cctx.ctx);
            }
            break;
        case ast::UnresolvedIdent::Kind::Instance:
            ENFORCE(cctx.ctx.owner.isMethod());
            klass = cctx.ctx.owner.owner(cctx.ctx).asClassOrModuleRef();
            break;
        case ast::UnresolvedIdent::Kind::Global:
            klass = core::Symbols::root();
            break;
        default:
            // These should have been removed in the namer
            Exception::notImplemented();
    }

    auto sym = klass.data(cctx.ctx)->findMemberTransitive(cctx.ctx, id.name);
    if (!sym.exists()) {
        auto fnd = cctx.discoveredUndeclaredFields.find(id.name);
        if (fnd == cctx.discoveredUndeclaredFields.end()) {
            if (id.kind != ast::UnresolvedIdent::Kind::Global && id.name != core::Names::ivarNameMissing() &&
                id.name != core::Names::cvarNameMissing()) {
                if (auto e = cctx.ctx.beginError(id.loc, core::errors::CFG::UndeclaredVariable)) {
                    e.setHeader("Use of undeclared variable `{}`", id.name.show(cctx.ctx));
                }
            }
            auto ret = cctx.newTemporary(id.name);
            cctx.discoveredUndeclaredFields[id.name] = ret;
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

void CFGBuilder::synthesizeExpr(BasicBlock *bb, LocalRef var, core::LocOffsets loc, InstructionPtr inst) {
    auto &inserted = bb->exprs.emplace_back(var, loc, move(inst));
    inserted.value.setSynthetic();
}

BasicBlock *CFGBuilder::walkHash(CFGContext cctx, ast::Hash &h, BasicBlock *current, core::NameRef method) {
    InlinedVector<cfg::LocalRef, 2> vars;
    InlinedVector<core::LocOffsets, 2> locs;
    for (int i = 0; i < h.keys.size(); i++) {
        LocalRef keyTmp = cctx.newTemporary(core::Names::hashTemp());
        LocalRef valTmp = cctx.newTemporary(core::Names::hashTemp());
        current = walk(cctx.withTarget(keyTmp), h.keys[i], current);
        current = walk(cctx.withTarget(valTmp), h.values[i], current);
        vars.emplace_back(keyTmp);
        vars.emplace_back(valTmp);
        locs.emplace_back(h.keys[i].loc());
        locs.emplace_back(h.values[i].loc());
    }
    LocalRef magic = cctx.newTemporary(core::Names::magic());
    synthesizeExpr(current, magic, core::LocOffsets::none(), make_insn<Alias>(core::Symbols::Magic()));

    auto isPrivateOk = false;
    current->exprs.emplace_back(
        cctx.target, h.loc,
        make_insn<Send>(magic, h.loc, method, core::LocOffsets::none(), vars.size(), vars, locs, isPrivateOk));
    return current;
}

BasicBlock *CFGBuilder::joinBlocks(CFGContext cctx, BasicBlock *a, BasicBlock *b) {
    auto *join = cctx.inWhat.freshBlock(cctx.loops, a->rubyRegionId);
    unconditionalJump(a, join, cctx.inWhat, core::LocOffsets::none());
    unconditionalJump(b, join, cctx.inWhat, core::LocOffsets::none());
    return join;
}

tuple<LocalRef, BasicBlock *, BasicBlock *> CFGBuilder::walkDefault(CFGContext cctx, int argIndex,
                                                                    const core::ArgInfo &argInfo, LocalRef argLocal,
                                                                    core::LocOffsets argLoc, ast::ExpressionPtr &def,
                                                                    BasicBlock *presentCont, BasicBlock *defaultCont) {
    auto defLoc = def.loc();

    auto *presentNext = cctx.inWhat.freshBlock(cctx.loops, presentCont->rubyRegionId);
    auto *defaultNext = cctx.inWhat.freshBlock(cctx.loops, presentCont->rubyRegionId);

    auto present = cctx.newTemporary(core::Names::argPresent());
    auto methodSymbol = cctx.inWhat.symbol;
    synthesizeExpr(presentCont, present, argLoc, make_insn<ArgPresent>(methodSymbol, argIndex));
    conditionalJump(presentCont, present, presentNext, defaultNext, cctx.inWhat, argLoc);

    if (defaultCont != nullptr) {
        unconditionalJump(defaultCont, defaultNext, cctx.inWhat, core::LocOffsets::none());
    }

    // Walk the default, and check the type of its final value
    // Walk the default, and check the type of its final value, but discard the result of the cast.
    auto result = cctx.newTemporary(core::Names::statTemp());
    defaultNext = walk(cctx.withTarget(result), def, defaultNext);

    if (argInfo.type != nullptr) {
        auto tmp = cctx.newTemporary(core::Names::castTemp());
        synthesizeExpr(defaultNext, tmp, defLoc, make_insn<Cast>(result, argInfo.type, core::Names::let()));
        cctx.inWhat.minLoops[tmp.id()] = CFG::MIN_LOOP_LET;
    }

    return {result, presentNext, defaultNext};
}

/** Convert `what` into a cfg, by starting to evaluate it in `current` inside method defined by `inWhat`.
 * store result of evaluation into `target`. Returns basic block in which evaluation should proceed.
 */
BasicBlock *CFGBuilder::walk(CFGContext cctx, ast::ExpressionPtr &what, BasicBlock *current) {
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
            [&](ast::While &a) {
                auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1, current->rubyRegionId);
                // breakNotCalledBlock is only entered if break is not called in
                // the loop body
                auto breakNotCalledBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyRegionId);
                auto continueBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyRegionId);
                unconditionalJump(current, headerBlock, cctx.inWhat, a.loc);

                LocalRef condSym = cctx.newTemporary(core::Names::whileTemp());
                auto headerEnd =
                    walk(cctx.withTarget(condSym).withLoopScope(headerBlock, continueBlock), a.cond, headerBlock);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1, current->rubyRegionId);
                conditionalJump(headerEnd, condSym, bodyBlock, breakNotCalledBlock, cctx.inWhat, a.cond.loc());
                // finishHeader
                LocalRef bodySym = cctx.newTemporary(core::Names::statTemp());

                auto body = walk(
                    cctx.withTarget(bodySym).withLoopScope(headerBlock, continueBlock).withLoopBreakTarget(cctx.target),
                    a.body, bodyBlock);
                unconditionalJump(body, headerBlock, cctx.inWhat, a.loc);

                synthesizeExpr(breakNotCalledBlock, cctx.target, a.loc, make_insn<Literal>(core::Types::nilClass()));
                unconditionalJump(breakNotCalledBlock, continueBlock, cctx.inWhat, a.loc);
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
            [&](ast::Return &a) {
                LocalRef retSym = cctx.newTemporary(core::Names::returnTemp());
                auto cont = walk(cctx.withTarget(retSym), a.expr, current);
                cont->exprs.emplace_back(cctx.target, a.loc, make_insn<Return>(retSym, a.expr.loc())); // dead assign.
                jumpToDead(cont, cctx.inWhat, a.loc);
                ret = cctx.inWhat.deadBlock();
            },
            [&](ast::If &a) {
                LocalRef ifSym = cctx.newTemporary(core::Names::ifTemp());
                ENFORCE(ifSym.exists(), "ifSym does not exist");
                auto cont = walk(cctx.withTarget(ifSym), a.cond, current);
                auto thenBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyRegionId);
                auto elseBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyRegionId);
                conditionalJump(cont, ifSym, thenBlock, elseBlock, cctx.inWhat, a.cond.loc());

                auto thenEnd = walk(cctx, a.thenp, thenBlock);
                auto elseEnd = walk(cctx, a.elsep, elseBlock);
                if (thenEnd != cctx.inWhat.deadBlock() || elseEnd != cctx.inWhat.deadBlock()) {
                    if (thenEnd == cctx.inWhat.deadBlock()) {
                        ret = elseEnd;
                    } else if (elseEnd == cctx.inWhat.deadBlock()) {
                        ret = thenEnd;
                    } else {
                        ret = cctx.inWhat.freshBlock(cctx.loops, current->rubyRegionId);
                        unconditionalJump(thenEnd, ret, cctx.inWhat, a.loc);
                        unconditionalJump(elseEnd, ret, cctx.inWhat, a.loc);
                    }
                } else {
                    ret = cctx.inWhat.deadBlock();
                }
            },
            [&](const ast::Literal &a) {
                current->exprs.emplace_back(cctx.target, a.loc, make_insn<Literal>(a.value));
                ret = current;
            },
            [&](const ast::UnresolvedIdent &id) {
                LocalRef loc = unresolvedIdent2Local(cctx, id);
                ENFORCE(loc.exists());
                current->exprs.emplace_back(cctx.target, id.loc, make_insn<Ident>(loc));

                ret = current;
            },
            [&](const ast::UnresolvedConstantLit &a) {
                Exception::raise("Should have been eliminated by namer/resolver");
            },
            [&](ast::ConstantLit &a) {
                auto aliasName = cctx.newTemporary(core::Names::cfgAlias());
                if (a.symbol == core::Symbols::StubModule()) {
                    current->exprs.emplace_back(aliasName, a.loc, make_insn<Alias>(core::Symbols::untyped()));
                } else {
                    current->exprs.emplace_back(aliasName, a.loc, make_insn<Alias>(a.symbol));
                }

                synthesizeExpr(current, cctx.target, a.loc, make_insn<Ident>(aliasName));

                if (a.original) {
                    auto *orig = ast::cast_tree<ast::UnresolvedConstantLit>(a.original);
                    // Empirically, these are the only two cases we've needed so far to service the
                    // LSP requests we want (hover and completion), but that doesn't mean these are
                    // the **only** we'll ever want.
                    if (ast::isa_tree<ast::ConstantLit>(orig->scope)) {
                        LocalRef deadSym = cctx.newTemporary(core::Names::keepForIde());
                        current = walk(cctx.withTarget(deadSym), orig->scope, current);
                    } else if (ast::isa_tree<ast::Send>(orig->scope)) {
                        LocalRef deadSym = cctx.newTemporary(core::Names::keepForIde());
                        current = walk(cctx.withTarget(deadSym), orig->scope, current);
                    }
                }

                ret = current;
            },
            [&](const ast::Local &a) {
                current->exprs.emplace_back(cctx.target, a.loc,
                                            make_insn<Ident>(cctx.inWhat.enterLocal(a.localVariable)));
                ret = current;
            },
            [&](ast::Assign &a) {
                LocalRef lhs;
                if (auto lhsIdent = ast::cast_tree<ast::ConstantLit>(a.lhs)) {
                    lhs = global2Local(cctx, lhsIdent->symbol);
                } else if (auto lhsLocal = ast::cast_tree<ast::Local>(a.lhs)) {
                    lhs = cctx.inWhat.enterLocal(lhsLocal->localVariable);
                } else if (auto ident = ast::cast_tree<ast::UnresolvedIdent>(a.lhs)) {
                    lhs = unresolvedIdent2Local(cctx, *ident);
                    ENFORCE(lhs.exists());
                } else {
                    Exception::raise("should never be reached");
                }

                auto rhsCont = walk(cctx.withTarget(lhs), a.rhs, current);
                rhsCont->exprs.emplace_back(cctx.target, a.loc, make_insn<Ident>(lhs));
                ret = rhsCont;
            },
            [&](ast::InsSeq &a) {
                for (auto &exp : a.stats) {
                    LocalRef temp = cctx.newTemporary(core::Names::statTemp());
                    current = walk(cctx.withTarget(temp), exp, current);
                }
                ret = walk(cctx, a.expr, current);
            },
            [&](ast::Send &s) {
                LocalRef recv;

                if (s.fun == core::Names::absurd()) {
                    if (auto cnst = ast::cast_tree<ast::ConstantLit>(s.recv)) {
                        if (cnst->symbol == core::Symbols::T()) {
                            if (s.hasKwArgs()) {
                                if (auto e = cctx.ctx.beginError(s.loc, core::errors::CFG::MalformedTAbsurd)) {
                                    e.setHeader("`{}` does not accept keyword arguments", "T.absurd");
                                }
                                ret = current;
                                return;
                            }

                            if (s.numPosArgs() != 1) {
                                if (auto e = cctx.ctx.beginError(s.loc, core::errors::CFG::MalformedTAbsurd)) {
                                    e.setHeader("`{}` expects exactly one argument but got `{}`", "T.absurd",
                                                s.numPosArgs());
                                }
                                ret = current;
                                return;
                            }

                            auto &posArg0 = s.getPosArg(0);
                            if (!ast::isa_tree<ast::Local>(posArg0) && !ast::isa_tree<ast::UnresolvedIdent>(posArg0)) {
                                if (auto e = cctx.ctx.beginError(s.loc, core::errors::CFG::MalformedTAbsurd)) {
                                    // Providing a send is the most common way T.absurd is misused, so we provide a
                                    // little extra hint in the error message in that case.
                                    if (ast::isa_tree<ast::Send>(posArg0)) {
                                        e.setHeader("`{}` expects to be called on a variable, not a method call",
                                                    "T.absurd");
                                    } else {
                                        e.setHeader("`{}` expects to be called on a variable", "T.absurd");
                                    }
                                    e.addErrorLine(core::Loc(cctx.ctx.file, posArg0.loc()),
                                                   "Assign this expression to a variable, and use it in both the "
                                                   "conditional and the `{}` call",
                                                   "T.absurd");
                                }
                                ret = current;
                                return;
                            }

                            auto temp = cctx.newTemporary(core::Names::statTemp());
                            current = walk(cctx.withTarget(temp), posArg0, current);
                            current->exprs.emplace_back(cctx.target, s.loc, make_insn<TAbsurd>(temp));
                            ret = current;
                            return;
                        }
                    }
                }

                recv = cctx.newTemporary(core::Names::statTemp());
                current = walk(cctx.withTarget(recv), s.recv, current);

                InlinedVector<LocalRef, 2> args;
                InlinedVector<core::LocOffsets, 2> argLocs;
                const auto posEnd = s.numPosArgs();
                for (auto argIdx = 0; argIdx < posEnd; ++argIdx) {
                    auto &exp = s.getPosArg(argIdx);
                    LocalRef temp = cctx.newTemporary(core::Names::statTemp());
                    current = walk(cctx.withTarget(temp), exp, current);
                    args.emplace_back(temp);
                    argLocs.emplace_back(exp.loc());
                }

                const auto kwEnd = s.numKwArgs();
                for (auto argIdx = 0; argIdx < kwEnd; ++argIdx) {
                    auto &key = s.getKwKey(argIdx);
                    auto &val = s.getKwValue(argIdx);
                    LocalRef keyTmp = cctx.newTemporary(core::Names::hashTemp());
                    LocalRef valTmp = cctx.newTemporary(core::Names::hashTemp());
                    current = walk(cctx.withTarget(keyTmp), key, current);
                    current = walk(cctx.withTarget(valTmp), val, current);
                    args.emplace_back(keyTmp);
                    args.emplace_back(valTmp);
                    argLocs.emplace_back(key.loc());
                    argLocs.emplace_back(val.loc());
                }

                if (auto *exp = s.kwSplat()) {
                    LocalRef temp = cctx.newTemporary(core::Names::statTemp());
                    current = walk(cctx.withTarget(temp), *exp, current);
                    args.emplace_back(temp);
                    argLocs.emplace_back(exp->loc());
                }

                if (auto *block = s.block()) {
                    auto newRubyRegionId = ++cctx.inWhat.maxRubyRegionId;
                    auto &blockArgs = block->args;
                    vector<ast::ParsedArg> blockArgFlags = ast::ArgParsing::parseArgs(blockArgs);
                    vector<core::ArgInfo::ArgFlags> argFlags;
                    for (auto &e : blockArgFlags) {
                        argFlags.emplace_back(e.flags);
                    }
                    auto link = make_shared<core::SendAndBlockLink>(s.fun, move(argFlags), newRubyRegionId);
                    auto send = make_insn<Send>(recv, s.recv.loc(), s.fun, s.funLoc, s.numPosArgs(), args, argLocs,
                                                !!s.flags.isPrivateOk, link);
                    LocalRef sendTemp = cctx.newTemporary(core::Names::blockPreCallTemp());
                    auto solveConstraint = make_insn<SolveConstraint>(link, sendTemp);
                    current->exprs.emplace_back(sendTemp, s.loc, move(send));
                    LocalRef restoreSelf = cctx.newTemporary(core::Names::selfRestore());
                    synthesizeExpr(current, restoreSelf, core::LocOffsets::none(),
                                   make_insn<Ident>(LocalRef::selfVariable()));

                    auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1, newRubyRegionId);
                    // solveConstraintBlock is only entered if break is not called
                    // in the block body.
                    auto solveConstraintBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyRegionId);
                    auto postBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyRegionId);
                    auto bodyLoops = cctx.loops + 1;
                    auto bodyBlock = cctx.inWhat.freshBlock(bodyLoops, newRubyRegionId);

                    LocalRef argTemp = cctx.newTemporary(core::Names::blkArg());
                    bodyBlock->exprs.emplace_back(LocalRef::selfVariable(), s.loc,
                                                  make_insn<LoadSelf>(link, LocalRef::selfVariable()));
                    bodyBlock->exprs.emplace_back(argTemp, s.block()->loc, make_insn<LoadYieldParams>(link));

                    auto *argBlock = bodyBlock;
                    for (int i = 0; i < blockArgFlags.size(); ++i) {
                        auto &arg = blockArgFlags[i];
                        LocalRef argLoc = cctx.inWhat.enterLocal(arg.local);

                        if (arg.flags.isRepeated) {
                            // Mixing positional and rest args in blocks is
                            // not currently supported, but we'll handle that in
                            // inference.
                            argBlock->exprs.emplace_back(argLoc, arg.loc,
                                                         make_insn<YieldLoadArg>(i, arg.flags, argTemp));
                            continue;
                        }

                        if (auto *opt = ast::cast_tree<ast::OptionalArg>(blockArgs[i])) {
                            auto *presentBlock = cctx.inWhat.freshBlock(bodyLoops, newRubyRegionId);
                            auto *missingBlock = cctx.inWhat.freshBlock(bodyLoops, newRubyRegionId);

                            // add a test for YieldParamPresent
                            auto present = cctx.newTemporary(core::Names::argPresent());
                            synthesizeExpr(argBlock, present, arg.loc,
                                           make_insn<YieldParamPresent>(static_cast<uint16_t>(i)));
                            conditionalJump(argBlock, present, presentBlock, missingBlock, cctx.inWhat, arg.loc);

                            // make a new block for the present and missing blocks to join
                            argBlock = cctx.inWhat.freshBlock(bodyLoops, newRubyRegionId);

                            // compile the argument fetch in the present block
                            presentBlock->exprs.emplace_back(argLoc, arg.loc,
                                                             make_insn<YieldLoadArg>(i, arg.flags, argTemp));
                            unconditionalJump(presentBlock, argBlock, cctx.inWhat, arg.loc);

                            // compile the default expr in `missingBlock`
                            auto *missingLast = walk(cctx.withTarget(argLoc), opt->default_, missingBlock);
                            unconditionalJump(missingLast, argBlock, cctx.inWhat, arg.loc);
                        } else {
                            argBlock->exprs.emplace_back(argLoc, arg.loc,
                                                         make_insn<YieldLoadArg>(i, arg.flags, argTemp));
                        }
                    }

                    conditionalJump(headerBlock, LocalRef::blockCall(), bodyBlock, solveConstraintBlock, cctx.inWhat,
                                    s.loc);

                    unconditionalJump(current, headerBlock, cctx.inWhat, s.loc);

                    LocalRef blockrv = cctx.newTemporary(core::Names::blockReturnTemp());
                    auto blockLast = walk(cctx.withTarget(blockrv)
                                              .withBlockBreakTarget(cctx.target)
                                              .withLoopScope(headerBlock, postBlock, true)
                                              .withSendAndBlockLink(link),
                                          s.block()->body, argBlock);
                    if (blockLast != cctx.inWhat.deadBlock()) {
                        LocalRef dead = cctx.newTemporary(core::Names::blockReturnTemp());
                        synthesizeExpr(blockLast, dead, s.block()->loc, make_insn<BlockReturn>(link, blockrv));
                    }

                    unconditionalJump(blockLast, headerBlock, cctx.inWhat, s.loc);
                    unconditionalJump(solveConstraintBlock, postBlock, cctx.inWhat, s.loc);

                    solveConstraintBlock->exprs.emplace_back(cctx.target, s.loc, move(solveConstraint));
                    current = postBlock;
                    synthesizeExpr(current, LocalRef::selfVariable(), s.loc, make_insn<Ident>(restoreSelf));

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
                    current->exprs.emplace_back(cctx.target, s.loc,
                                                make_insn<Send>(recv, s.recv.loc(), s.fun, s.funLoc, s.numPosArgs(),
                                                                args, argLocs, !!s.flags.isPrivateOk));
                }

                ret = current;
            },

            [&](const ast::Block &a) { Exception::raise("should never encounter a bare Block"); },

            [&](ast::Next &a) {
                LocalRef exprSym = cctx.newTemporary(core::Names::nextTemp());
                auto afterNext = walk(cctx.withTarget(exprSym), a.expr, current);
                if (afterNext != cctx.inWhat.deadBlock() && cctx.isInsideRubyBlock) {
                    LocalRef dead = cctx.newTemporary(core::Names::nextTemp());
                    ENFORCE(cctx.link.get() != nullptr);
                    afterNext->exprs.emplace_back(dead, a.loc, make_insn<BlockReturn>(cctx.link, exprSym));
                }

                if (cctx.nextScope == nullptr) {
                    if (auto e = cctx.ctx.beginError(a.loc, core::errors::CFG::NoNextScope)) {
                        e.setHeader("No `{}` block around `{}`", "do", "next");
                    }
                    // I guess just keep going into deadcode?
                    unconditionalJump(afterNext, cctx.inWhat.deadBlock(), cctx.inWhat, a.loc);
                } else {
                    unconditionalJump(afterNext, cctx.nextScope, cctx.inWhat, a.loc);
                }

                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Break &a) {
                LocalRef exprSym = cctx.newTemporary(core::Names::returnTemp());
                auto afterBreak = walk(cctx.withTarget(exprSym), a.expr, current);

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
                afterBreak->exprs.emplace_back(blockBreakAssign, a.loc, make_insn<Ident>(exprSym));

                // Only emit `<blockBreak>` in a block context
                if (!cctx.breakIsJump) {
                    // call intrinsic for break
                    auto magic = cctx.newTemporary(core::Names::magic());
                    auto ignored = cctx.newTemporary(core::Names::blockBreak());
                    synthesizeExpr(afterBreak, magic, a.loc, make_insn<Alias>(core::Symbols::Magic()));
                    InlinedVector<LocalRef, 2> args{exprSym};
                    InlinedVector<core::LocOffsets, 2> locs{core::LocOffsets::none()};
                    auto isPrivateOk = false;

                    // This represents the throw in the Ruby VM to the appropriate control frame.
                    // It needs to come prior to the assignment (which shouldn't really be here,
                    // but see above for the rationale) because the actual assignment is a) done
                    // by the VM itself; and b) may not actually happen depending on the frames
                    // that the break unwinds through.
                    synthesizeExpr(afterBreak, ignored, core::LocOffsets::none(),
                                   make_insn<Send>(magic, core::LocOffsets::none(), core::Names::blockBreak(),
                                                   core::LocOffsets::none(), args.size(), args, locs, isPrivateOk));
                }

                afterBreak->exprs.emplace_back(cctx.blockBreakTarget, a.loc, make_insn<Ident>(blockBreakAssign));

                if (cctx.breakScope == nullptr) {
                    if (auto e = cctx.ctx.beginError(a.loc, core::errors::CFG::NoNextScope)) {
                        e.setHeader("No `{}` block around `{}`", "do", "break");
                    }
                    // I guess just keep going into deadcode?
                    unconditionalJump(afterBreak, cctx.inWhat.deadBlock(), cctx.inWhat, a.loc);
                } else {
                    unconditionalJump(afterBreak, cctx.breakScope, cctx.inWhat, a.loc);
                }
                ret = cctx.inWhat.deadBlock();
            },

            [&](const ast::Retry &a) {
                if (cctx.rescueScope == nullptr) {
                    if (auto e = cctx.ctx.beginError(a.loc, core::errors::CFG::NoNextScope)) {
                        e.setHeader("No `{}` block around `{}`", "begin", "retry");
                    }
                    // I guess just keep going into deadcode?
                    unconditionalJump(current, cctx.inWhat.deadBlock(), cctx.inWhat, a.loc);
                } else {
                    auto magic = cctx.newTemporary(core::Names::magic());
                    synthesizeExpr(current, magic, core::LocOffsets::none(), make_insn<Alias>(core::Symbols::Magic()));
                    auto retryTemp = cctx.newTemporary(core::Names::retryTemp());
                    InlinedVector<cfg::LocalRef, 2> args{};
                    InlinedVector<core::LocOffsets, 2> argLocs{};
                    auto isPrivateOk = false;
                    synthesizeExpr(current, retryTemp, core::LocOffsets::none(),
                                   make_insn<Send>(magic, what.loc(), core::Names::retry(), core::LocOffsets::none(),
                                                   args.size(), args, argLocs, isPrivateOk));
                    unconditionalJump(current, cctx.rescueScope, cctx.inWhat, a.loc);
                }
                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Rescue &a) {
                auto bodyRubyRegionId = ++cctx.inWhat.maxRubyRegionId;
                auto handlersRubyRegionId = bodyRubyRegionId + CFG::HANDLERS_REGION_OFFSET;
                auto ensureRubyRegionId = bodyRubyRegionId + CFG::ENSURE_REGION_OFFSET;
                auto elseRubyRegionId = bodyRubyRegionId + CFG::ELSE_REGION_OFFSET;
                cctx.inWhat.maxRubyRegionId = elseRubyRegionId;

                auto rescueHeaderBlock = cctx.inWhat.freshBlock(cctx.loops, current->rubyRegionId);
                unconditionalJump(current, rescueHeaderBlock, cctx.inWhat, a.loc);
                cctx.rescueScope = rescueHeaderBlock;

                // We have a simplified view of the control flow here but in
                // practise it has been reasonable on our codebase.
                // We don't model that each expression in the `body` or `else` could
                // throw, instead we model only never running anything in the
                // body, or running the whole thing. To do this we  have a magic
                // Unanalyzable variable at the top of the body using
                // `exceptionValue` and one at the end of the else using
                // `rescueEndTemp` which can jump into the rescue handlers.
                auto rescueHandlersBlock = cctx.inWhat.freshBlock(cctx.loops, handlersRubyRegionId);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops, bodyRubyRegionId);
                auto exceptionValue = cctx.newTemporary(core::Names::exceptionValue());
                synthesizeExpr(rescueHeaderBlock, exceptionValue, what.loc(), make_insn<GetCurrentException>());
                conditionalJump(rescueHeaderBlock, exceptionValue, rescueHandlersBlock, bodyBlock, cctx.inWhat, a.loc);

                // cctx.loops += 1; // should formally be here but this makes us report a lot of false errors
                bodyBlock = walk(cctx, a.body, bodyBlock);

                // else is only executed if body didn't raise an exception
                auto elseBody = cctx.inWhat.freshBlock(cctx.loops, elseRubyRegionId);
                synthesizeExpr(bodyBlock, exceptionValue, what.loc(), make_insn<GetCurrentException>());
                conditionalJump(bodyBlock, exceptionValue, rescueHandlersBlock, elseBody, cctx.inWhat, a.loc);

                elseBody = walk(cctx, a.else_, elseBody);
                auto ensureBody = cctx.inWhat.freshBlock(cctx.loops, ensureRubyRegionId);
                unconditionalJump(elseBody, ensureBody, cctx.inWhat, a.loc);

                auto magic = cctx.newTemporary(core::Names::magic());
                synthesizeExpr(current, magic, core::LocOffsets::none(), make_insn<Alias>(core::Symbols::Magic()));

                for (auto &expr : a.rescueCases) {
                    auto *rescueCase = ast::cast_tree<ast::RescueCase>(expr);
                    auto caseBody = cctx.inWhat.freshBlock(cctx.loops, handlersRubyRegionId);
                    auto &exceptions = rescueCase->exceptions;
                    auto added = false;
                    auto *local = ast::cast_tree<ast::Local>(rescueCase->var);
                    ENFORCE(local != nullptr, "rescue case var not a local?");

                    auto localVar = cctx.inWhat.enterLocal(local->localVariable);
                    rescueHandlersBlock->exprs.emplace_back(localVar, rescueCase->var.loc(),
                                                            make_insn<Ident>(exceptionValue));

                    // Mark the exception as handled
                    synthesizeExpr(caseBody, exceptionValue, core::LocOffsets::none(),
                                   make_insn<Literal>(core::Types::nilClass()));

                    auto res = cctx.newTemporary(core::Names::keepForCfgTemp());
                    auto isPrivateOk = false;
                    auto args = {exceptionValue};
                    auto argLocs = {what.loc()};
                    synthesizeExpr(caseBody, res, rescueCase->loc,
                                   make_insn<Send>(magic, rescueCase->loc, core::Names::keepForCfg(),
                                                   core::LocOffsets::none(), args.size(), args, argLocs, isPrivateOk));

                    if (exceptions.empty()) {
                        // rescue without a class catches StandardError
                        exceptions.emplace_back(
                            ast::MK::Constant(rescueCase->var.loc(), core::Symbols::StandardError()));
                        added = true;
                    }
                    for (auto &ex : exceptions) {
                        auto loc = ex.loc();
                        auto exceptionClass = cctx.newTemporary(core::Names::exceptionClassTemp());
                        rescueHandlersBlock = walk(cctx.withTarget(exceptionClass), ex, rescueHandlersBlock);

                        auto isaCheck = cctx.newTemporary(core::Names::isaCheckTemp());
                        InlinedVector<cfg::LocalRef, 2> args;
                        InlinedVector<core::LocOffsets, 2> argLocs = {loc};
                        args.emplace_back(exceptionClass);

                        auto isPrivateOk = false;
                        rescueHandlersBlock->exprs.emplace_back(isaCheck, loc,
                                                                make_insn<Send>(localVar, loc, core::Names::isA_p(),
                                                                                core::LocOffsets::none(), args.size(),
                                                                                args, argLocs, isPrivateOk));

                        auto otherHandlerBlock = cctx.inWhat.freshBlock(cctx.loops, handlersRubyRegionId);
                        conditionalJump(rescueHandlersBlock, isaCheck, caseBody, otherHandlerBlock, cctx.inWhat, loc);
                        rescueHandlersBlock = otherHandlerBlock;
                    }
                    if (added) {
                        exceptions.pop_back();
                    }

                    caseBody = walk(cctx, rescueCase->body, caseBody);
                    unconditionalJump(caseBody, ensureBody, cctx.inWhat, a.loc);
                }

                // This magic local remembers if none of the `rescue`s match,
                // and if so, after the ensure runs, we should jump to dead
                // since in Ruby the exception would propagate up the statck.
                auto gotoDeadTemp = cctx.newTemporary(core::Names::gotoDeadTemp());
                synthesizeExpr(rescueHandlersBlock, gotoDeadTemp, a.loc, make_insn<Literal>(core::Types::trueClass()));
                unconditionalJump(rescueHandlersBlock, ensureBody, cctx.inWhat, a.loc);

                auto throwAway = cctx.newTemporary(core::Names::throwAwayTemp());
                ensureBody = walk(cctx.withTarget(throwAway), a.ensure, ensureBody);
                ret = cctx.inWhat.freshBlock(cctx.loops, current->rubyRegionId);
                conditionalJump(ensureBody, gotoDeadTemp, cctx.inWhat.deadBlock(), ret, cctx.inWhat, a.loc);
            },

            [&](ast::Hash &h) { ret = walkHash(cctx, h, current, core::Names::buildHash()); },

            [&](ast::Array &a) {
                InlinedVector<LocalRef, 2> vars;
                InlinedVector<core::LocOffsets, 2> locs;
                for (auto &elem : a.elems) {
                    LocalRef tmp = cctx.newTemporary(core::Names::arrayTemp());
                    current = walk(cctx.withTarget(tmp), elem, current);
                    vars.emplace_back(tmp);
                    locs.emplace_back(a.loc);
                }
                LocalRef magic = cctx.newTemporary(core::Names::magic());
                synthesizeExpr(current, magic, core::LocOffsets::none(), make_insn<Alias>(core::Symbols::Magic()));
                auto isPrivateOk = false;
                current->exprs.emplace_back(cctx.target, a.loc,
                                            make_insn<Send>(magic, a.loc, core::Names::buildArray(),
                                                            core::LocOffsets::none(), vars.size(), vars, locs,
                                                            isPrivateOk));
                ret = current;
            },

            [&](ast::Cast &c) {
                LocalRef tmp = cctx.newTemporary(core::Names::castTemp());
                current = walk(cctx.withTarget(tmp), c.arg, current);
                if (c.cast == core::Names::uncheckedLet()) {
                    current->exprs.emplace_back(cctx.target, c.loc, make_insn<Ident>(tmp));
                } else if (c.cast == core::Names::bind()) {
                    if (c.arg.isSelfReference()) {
                        auto self = cctx.inWhat.enterLocal(core::LocalVariable::selfVariable());
                        current->exprs.emplace_back(self, c.loc, make_insn<Cast>(tmp, c.type, core::Names::cast()));
                        current->exprs.emplace_back(cctx.target, c.loc, make_insn<Ident>(self));

                        if (cctx.rescueScope) {
                            cctx.rescueScope->exprs.emplace_back(self, c.loc,
                                                                 make_insn<Cast>(tmp, c.type, core::Names::cast()));
                            cctx.rescueScope->exprs.emplace_back(cctx.target, c.loc, make_insn<Ident>(self));
                        }
                    } else {
                        if (auto e = cctx.ctx.beginError(what.loc(), core::errors::CFG::MalformedTBind)) {
                            e.setHeader("`{}` can only be used with `{}`", "T.bind", "self");
                        }
                    }
                } else {
                    current->exprs.emplace_back(cctx.target, c.loc, make_insn<Cast>(tmp, c.type, c.cast));
                }
                if (c.cast == core::Names::let()) {
                    cctx.inWhat.minLoops[cctx.target.id()] = CFG::MIN_LOOP_LET;
                }

                ret = current;
            },

            [&](const ast::EmptyTree &n) { ret = current; },

            [&](const ast::ClassDef &c) { Exception::raise("Should have been removed by FlattenWalk"); },
            [&](const ast::MethodDef &c) { Exception::raise("Should have been removed by FlattenWalk"); },

            [&](const ast::ExpressionPtr &n) { Exception::raise("Unimplemented AST Node: {}", what.nodeName()); });

        // For, Rescue,
        // Symbol, Array,
        ENFORCE(ret != nullptr, "CFB builder ret unset");
        return ret;
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = cctx.ctx.beginError(what.loc(), core::errors::Internal::InternalError)) {
            e.setHeader("Failed to convert tree to CFG (backtrace is above )");
        }
        throw;
    }
}

LocalRef CFGContext::newTemporary(core::NameRef name) {
    return inWhat.enterLocal(core::LocalVariable{name, ++temporaryCounter});
}
} // namespace sorbet::cfg
