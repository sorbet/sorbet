#include "ast/Helpers.h"
#include "ast/ParamParsing.h"
#include "cfg/builder/builder.h"
#include "common/typecase.h"
#include "core/Names.h"
#include "core/errors/cfg.h"
#include "core/errors/internal.h"

using namespace std;

namespace sorbet::cfg {

void CFGBuilder::conditionalJump(BasicBlock *from, LocalRef cond, BasicBlock *thenb, BasicBlock *elseb, CFG &inWhat,
                                 core::LocOffsets loc) {
    thenb->flags.wasJumpDestination = true;
    elseb->flags.wasJumpDestination = true;
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
    to->flags.wasJumpDestination = true;
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
    if (what == core::Symbols::StubModule()) {
        // We don't need all stub module assignments to alias to the same temporary.
        // (The fact that there's a StubModule at all means an error was already reported elsewhere)
        return cctx.newTemporary(what.name(cctx.ctx));
    }

    // Note: this will add an empty local to aliases if 'what' is not there
    LocalRef &alias = cctx.aliases[what];
    if (!alias.exists()) {
        alias = cctx.newTemporary(what.name(cctx.ctx));
    }
    return alias;
}

pair<LocalRef, bool> unresolvedIdent2Local(CFGContext cctx, const ast::UnresolvedIdent &id, bool isAssign) {
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
        auto hasError = id.kind != ast::UnresolvedIdent::Kind::Global && id.name != core::Names::ivarNameMissing() &&
                        id.name != core::Names::cvarNameMissing();

        auto fnd = cctx.discoveredUndeclaredFields.find(id.name);
        if (fnd == cctx.discoveredUndeclaredFields.end()) {
            // For reads (not assigns) we only report the problem on the first offense.
            if (hasError && !isAssign) {
                if (auto e = cctx.ctx.beginError(id.loc, core::errors::CFG::UndeclaredVariable)) {
                    e.setHeader("Use of undeclared variable `{}`", id.name.show(cctx.ctx));
                    e.addErrorNote("Use `{}` to declare this variable.\n"
                                   "    For more information, see https://sorbet.org/docs/type-annotations",
                                   "T.let");
                }
            }
            auto ret = cctx.newTemporary(id.name);
            cctx.discoveredUndeclaredFields[id.name] = ret;
            return {ret, hasError && isAssign};
        }
        return {fnd->second, hasError && isAssign};
    } else {
        return {global2Local(cctx, sym), false};
    }
}

bool sendRecvIsT(ast::Send &s) {
    if (auto cnst = ast::cast_tree<ast::ConstantLit>(s.recv)) {
        return cnst->symbol() == core::Symbols::T();
    } else {
        return false;
    }
}

bool isKernelLambda(ast::Send &s) {
    if (s.fun != core::Names::lambda() && s.fun != core::Names::lambdaTLet()) {
        return false;
    }

    // Only handle `-> () {}` and `Kernel.lambda` lambdas for now, because there's nothing stopping
    // someone from defining a method called `lambda` on their own that behaves differently.
    //
    // We could revisit this in the future but for now let's be conservative.

    auto cnst = ast::cast_tree<ast::ConstantLit>(s.recv);
    return cnst != nullptr && cnst->symbol() == core::Symbols::Kernel();
}

InstructionPtr maybeMakeTypeParameterAlias(CFGContext &cctx, ast::Send &s) {
    const auto &ctx = cctx.ctx;
    auto method = cctx.inWhat.symbol;
    if (!method.data(ctx)->flags.isGenericMethod) {
        // Using staticInit as a crude proxy for "is inside a `sig` block"
        // This means we do not report as many errors as we should (but cheaply guards against false positives)
        if (!method.data(ctx)->name.isAnyStaticInitName(ctx)) {
            if (auto e = ctx.beginError(s.loc, core::errors::CFG::UnknownTypeParameter)) {
                e.setHeader("Method `{}` does not declare any type parameters", method.show(ctx));
                e.addErrorLine(method.data(ctx)->loc(), "`{}` defined here", method.show(ctx));
            }
        }

        return nullptr;
    }

    if (s.numPosArgs() != 1 || !ast::isa_tree<ast::Literal>(s.getPosArg(0))) {
        // Infer will report normal type error
        return nullptr;
    }
    const auto &namedLiteral = ast::cast_tree_nonnull<ast::Literal>(s.getPosArg(0));
    if (!namedLiteral.isSymbol()) {
        // Infer will report normal type error
        return nullptr;
    }

    auto typeVarName = ctx.state.lookupNameUnique(core::UniqueNameKind::TypeVarName, namedLiteral.asSymbol(), 1);
    if (!typeVarName.exists()) {
        if (auto e = ctx.beginError(namedLiteral.loc, core::errors::CFG::UnknownTypeParameter)) {
            e.setHeader("Type parameter `{}` does not exist on `{}`", namedLiteral.toStringWithTabs(ctx, 0),
                        method.show(ctx));
            e.addErrorLine(method.data(ctx)->loc(), "`{}` defined here", method.show(ctx));
        }
        return nullptr;
    }

    core::TypeParameterRef typeParam;
    for (const auto &it : method.data(ctx)->typeParameters()) {
        if (it.data(ctx)->name == typeVarName) {
            typeParam = it;
        }
    }

    if (!typeParam.exists()) {
        if (auto e = ctx.beginError(namedLiteral.loc, core::errors::CFG::UnknownTypeParameter)) {
            e.setHeader("Type parameter `{}` does not exist on `{}`", namedLiteral.toStringWithTabs(ctx, 0),
                        method.show(ctx));
            e.addErrorLine(method.data(ctx)->loc(), "`{}` defined here", method.show(ctx));
        }
        return nullptr;
    }

    return make_insn<Alias>(typeParam);
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
    LocalRef magic = cctx.newTemporary(core::Names::magic());
    InlinedVector<cfg::LocalRef, 2> vars;
    InlinedVector<core::LocOffsets, 2> locs;
    for (auto [key, val] : h.kviter()) {
        LocalRef keyTmp = cctx.newTemporary(core::Names::hashTemp());
        LocalRef valTmp = cctx.newTemporary(core::Names::hashTemp());
        current = walk(cctx.withTarget(keyTmp), key, current);
        current = walk(cctx.withTarget(valTmp), val, current);
        vars.emplace_back(keyTmp);
        vars.emplace_back(valTmp);
        locs.emplace_back(key.loc());
        locs.emplace_back(val.loc());
    }
    synthesizeExpr(current, magic, core::LocOffsets::none(), make_insn<Alias>(core::Symbols::Magic()));

    auto isPrivateOk = false;
    current->exprs.emplace_back(cctx.target, h.loc,
                                make_insn<Send>(magic, h.loc, method, core::LocOffsets::none(), vars.size(), vars,
                                                std::move(locs), isPrivateOk));
    return current;
}

// This doesn't actually "walk" an empty tree, because there's nothing interesting to walk in one.
// Instead, if conforms to mostly the same interface that `walk` (i.e., returns a BasicBlock *);
BasicBlock *CFGBuilder::walkEmptyTreeInIf(CFGContext cctx, core::LocOffsets nilLoc, BasicBlock *current) {
    synthesizeExpr(current, cctx.target, nilLoc, make_insn<Literal>(core::Types::nilClass()));
    return current;
}

BasicBlock *CFGBuilder::walkBlockReturn(CFGContext cctx, core::LocOffsets loc, ast::ExpressionPtr &expr,
                                        BasicBlock *current) {
    LocalRef exprSym = cctx.newTemporary(core::Names::nextTemp());
    auto afterNext = walk(cctx.withTarget(exprSym), expr, current);
    if (afterNext != cctx.inWhat.deadBlock() && cctx.isInsideRubyBlock) {
        LocalRef dead = cctx.newTemporary(core::Names::nextTemp());
        ENFORCE(cctx.link != nullptr);
        ENFORCE(cctx.link->get() != nullptr);
        afterNext->exprs.emplace_back(dead, loc, make_insn<BlockReturn>(*cctx.link, exprSym));
    }

    if (cctx.nextScope == nullptr) {
        if (auto e = cctx.ctx.beginError(loc, core::errors::CFG::NoNextScope)) {
            e.setHeader("No `{}` block around `{}`", "do", "next");
        }
        // I guess just keep going into deadcode?
        unconditionalJump(afterNext, cctx.inWhat.deadBlock(), cctx.inWhat, loc);
    } else {
        unconditionalJump(afterNext, cctx.nextScope, cctx.inWhat, loc);
    }

    return cctx.inWhat.deadBlock();
}

BasicBlock *CFGBuilder::joinBlocks(CFGContext cctx, BasicBlock *a, BasicBlock *b) {
    auto *join = cctx.inWhat.freshBlock(cctx.loops);
    unconditionalJump(a, join, cctx.inWhat, core::LocOffsets::none());
    unconditionalJump(b, join, cctx.inWhat, core::LocOffsets::none());
    return join;
}

tuple<LocalRef, BasicBlock *, BasicBlock *> CFGBuilder::walkDefault(CFGContext cctx, int paramIndex,
                                                                    const core::ParamInfo &paramInfo,
                                                                    LocalRef paramLocal, core::LocOffsets paramLoc,
                                                                    ast::ExpressionPtr &def, BasicBlock *presentCont,
                                                                    BasicBlock *defaultCont) {
    auto defLoc = def.loc();

    auto *presentNext = cctx.inWhat.freshBlock(cctx.loops);
    auto *defaultNext = cctx.inWhat.freshBlock(cctx.loops);

    auto present = cctx.newTemporary(core::Names::argPresent());
    auto methodSymbol = cctx.inWhat.symbol;
    synthesizeExpr(presentCont, present, paramLoc, make_insn<ArgPresent>(methodSymbol, paramIndex));
    conditionalJump(presentCont, present, presentNext, defaultNext, cctx.inWhat, paramLoc);

    if (defaultCont != nullptr) {
        unconditionalJump(defaultCont, defaultNext, cctx.inWhat, core::LocOffsets::none());
    }

    // Walk the default, and check the type of its final value
    // Walk the default, and check the type of its final value, but discard the result of the cast.
    auto result = cctx.newTemporary(core::Names::statTemp());
    defaultNext = walk(cctx.withTarget(result), def, defaultNext);

    if (paramInfo.type != nullptr) {
        auto tmp = cctx.newTemporary(core::Names::castTemp());
        synthesizeExpr(defaultNext, tmp, defLoc, make_insn<Cast>(result, defLoc, paramInfo.type, core::Names::let()));
        cctx.inWhat.minLoops[tmp.id()] = CFG::MIN_LOOP_LET;
    }

    return {result, presentNext, defaultNext};
}

BasicBlock *CFGBuilder::buildExceptionHandler(CFGContext cctx, ast::ExpressionPtr &ex, BasicBlock *caseBody,
                                              cfg::LocalRef exceptionValue, BasicBlock *rescueHandlersBlock) {
    auto loc = ex.loc();
    auto exceptionClass = cctx.newTemporary(core::Names::exceptionClassTemp());
    rescueHandlersBlock = walk(cctx.withTarget(exceptionClass), ex, rescueHandlersBlock);

    auto isaCheck = cctx.newTemporary(core::Names::isaCheckTemp());
    InlinedVector<cfg::LocalRef, 2> args;
    InlinedVector<core::LocOffsets, 2> argLocs = {loc};
    args.emplace_back(exceptionValue);

    auto isPrivateOk = false;
    rescueHandlersBlock->exprs.emplace_back(isaCheck, loc,
                                            make_insn<Send>(exceptionClass, loc, core::Names::tripleEq(),
                                                            loc.copyWithZeroLength(), args.size(), args,
                                                            std::move(argLocs), isPrivateOk));

    auto otherHandlerBlock = cctx.inWhat.freshBlock(cctx.loops);
    conditionalJump(rescueHandlersBlock, isaCheck, caseBody, otherHandlerBlock, cctx.inWhat, loc);

    return otherHandlerBlock;
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
                auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1);
                // breakNotCalledBlock is only entered if break is not called in
                // the loop body
                auto breakNotCalledBlock = cctx.inWhat.freshBlock(cctx.loops);
                auto continueBlock = cctx.inWhat.freshBlock(cctx.loops);
                unconditionalJump(current, headerBlock, cctx.inWhat, a.loc);

                LocalRef condSym = cctx.newTemporary(core::Names::whileTemp());
                auto headerEnd =
                    walk(cctx.withTarget(condSym).withLoopScope(headerBlock, continueBlock), a.cond, headerBlock);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1);
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
                if (cctx.isInsideLambda) {
                    ret = walkBlockReturn(cctx, a.loc, a.expr, current);
                    return;
                }

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
                auto thenBlock = cctx.inWhat.freshBlock(cctx.loops);
                auto elseBlock = cctx.inWhat.freshBlock(cctx.loops);
                conditionalJump(cont, ifSym, thenBlock, elseBlock, cctx.inWhat, a.cond.loc());

                auto thenEnd = ast::isa_tree<ast::EmptyTree>(a.thenp) ? walkEmptyTreeInIf(cctx, a.loc, thenBlock)
                                                                      : walk(cctx, a.thenp, thenBlock);
                auto elseEnd = ast::isa_tree<ast::EmptyTree>(a.elsep) ? walkEmptyTreeInIf(cctx, a.loc, elseBlock)
                                                                      : walk(cctx, a.elsep, elseBlock);
                if (thenEnd != cctx.inWhat.deadBlock() || elseEnd != cctx.inWhat.deadBlock()) {
                    if (thenEnd == cctx.inWhat.deadBlock()) {
                        ret = elseEnd;
                    } else if (elseEnd == cctx.inWhat.deadBlock()) {
                        ret = thenEnd;
                    } else {
                        ret = cctx.inWhat.freshBlock(cctx.loops);
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
                auto isAssign = false;
                auto [loc, _foundError] = unresolvedIdent2Local(cctx, id, isAssign);
                ENFORCE(loc.exists());
                current->exprs.emplace_back(cctx.target, id.loc, make_insn<Ident>(loc));

                ret = current;
            },
            [&](const ast::UnresolvedConstantLit &a) {
                Exception::raise("Should have been eliminated by namer/resolver");
            },
            [&](ast::ConstantLit &a) {
                auto aliasName = cctx.newTemporary(core::Names::cfgAlias());
                auto loc = a.loc();

                if (auto sym = a.symbol(); sym == core::Symbols::StubModule()) {
                    current->exprs.emplace_back(aliasName, loc, make_insn<Alias>(core::Symbols::untyped()));
                } else {
                    current->exprs.emplace_back(aliasName, loc, make_insn<Alias>(sym));
                }

                synthesizeExpr(current, cctx.target, loc, make_insn<Ident>(aliasName));

                if (auto *orig = a.original()) {
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
            [&](const ast::Self &a) {
                // We still model `self` in the CFG as a local variable, to support `bind`
                current->exprs.emplace_back(cctx.target, a.loc, make_insn<Ident>(LocalRef::selfVariable()));
                ret = current;
            },
            [&](ast::Assign &a) {
                LocalRef lhs;
                if (auto lhsIdent = ast::cast_tree<ast::ConstantLit>(a.lhs)) {
                    lhs = global2Local(cctx, lhsIdent->symbol());
                } else if (auto lhsLocal = ast::cast_tree<ast::Local>(a.lhs)) {
                    lhs = cctx.inWhat.enterLocal(lhsLocal->localVariable);
                } else if (auto ident = ast::cast_tree<ast::UnresolvedIdent>(a.lhs)) {
                    auto isAssign = true;
                    auto [newLhs, foundError] = unresolvedIdent2Local(cctx, *ident, isAssign);
                    lhs = newLhs;
                    // Detect if we would have reported an error
                    // Only do this transformation if we're sure that it would produce an error, so
                    // that we don't pay the performance cost of inflating the CFG needlessly.
                    auto shouldReportErrorOn =
                        cctx.ctx.state.shouldReportErrorOn(cctx.ctx.file, core::errors::CFG::UndeclaredVariable);
                    if (foundError && shouldReportErrorOn) {
                        auto zeroLoc = a.loc.copyWithZeroLength();
                        auto magic = ast::MK::Constant(zeroLoc, core::Symbols::Magic());
                        core::NameRef fieldKind;
                        if (ident->kind == ast::UnresolvedIdent::Kind::Class) {
                            fieldKind = core::Names::class_();
                        } else {
                            ENFORCE(cctx.ctx.owner.isMethod());
                            auto owner = cctx.ctx.owner.owner(cctx.ctx).asClassOrModuleRef();
                            if (owner.data(cctx.ctx)->isSingletonClass(cctx.ctx)) {
                                fieldKind = core::Names::singletonClassInstance();
                            } else {
                                fieldKind = core::Names::instance();
                            }
                        }

                        // Mutate a.rhs before walking.
                        a.rhs =
                            ast::MK::Send4(a.lhs.loc(), move(magic), core::Names::suggestFieldType(), zeroLoc,
                                           move(a.rhs), ast::MK::String(zeroLoc, fieldKind),
                                           ast::MK::String(zeroLoc, cctx.ctx.owner.asMethodRef().data(cctx.ctx)->name),
                                           ast::MK::Symbol(zeroLoc, ident->name));
                    }
                    ENFORCE(lhs.exists());
                } else {
                    Exception::raise("Unexpected Assign::lhs in builder_walk.cc: {}", a.nodeName());
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

                // For performance, we do the name check first (single integer comparison in the
                // common case)
                if (s.fun == core::Names::absurd() && sendRecvIsT(s)) {
                    if (s.hasKwArgs()) {
                        if (auto e = cctx.ctx.beginError(s.loc, core::errors::CFG::MalformedTAbsurd)) {
                            e.setHeader("`{}` does not accept keyword arguments", "T.absurd");
                        }
                        ret = current;
                        return;
                    }

                    if (s.numPosArgs() != 1) {
                        if (auto e = cctx.ctx.beginError(s.loc, core::errors::CFG::MalformedTAbsurd)) {
                            e.setHeader("`{}` expects exactly one argument but got `{}`", "T.absurd", s.numPosArgs());
                        }
                        ret = current;
                        return;
                    }

                    auto &posArg0 = s.getPosArg(0);
                    if (!ast::isa_tree<ast::Local>(posArg0) && !ast::isa_tree<ast::UnresolvedIdent>(posArg0) &&
                        !ast::isa_tree<ast::Self>(posArg0)) {
                        if (auto e = cctx.ctx.beginError(s.loc, core::errors::CFG::MalformedTAbsurd)) {
                            // Providing a send is the most common way T.absurd is misused, so we provide a
                            // little extra hint in the error message in that case.
                            if (ast::isa_tree<ast::Send>(posArg0)) {
                                e.setHeader("`{}` expects to be called on a variable, not a method call", "T.absurd");
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
                } else if (s.fun == core::Names::attachedClass() && sendRecvIsT(s)) {
                    s.recv = ast::MK::Magic(s.recv.loc());
                    s.addPosArg(ast::MK::Self(s.recv.loc()));
                } else if (s.fun == core::Names::typeParameter() && sendRecvIsT(s)) {
                    if (auto insn = maybeMakeTypeParameterAlias(cctx, s)) {
                        current->exprs.emplace_back(cctx.target, s.loc, move(insn));
                        ret = current;
                        return;
                    }
                }

                recv = cctx.newTemporary(core::Names::statTemp());
                current = walk(cctx.withTarget(recv), s.recv, current);

                InlinedVector<LocalRef, 2> args;
                InlinedVector<core::LocOffsets, 2> argLocs;
                for (auto &exp : s.posArgs()) {
                    LocalRef temp = cctx.newTemporary(core::Names::statTemp());
                    current = walk(cctx.withTarget(temp), exp, current);
                    args.emplace_back(temp);
                    argLocs.emplace_back(exp.loc());
                }

                for (auto [key, value] : s.kwArgPairs()) {
                    LocalRef keyTmp = cctx.newTemporary(core::Names::hashTemp());
                    LocalRef valTmp = cctx.newTemporary(core::Names::hashTemp());
                    current = walk(cctx.withTarget(keyTmp), key, current);
                    current = walk(cctx.withTarget(valTmp), value, current);
                    args.emplace_back(keyTmp);
                    args.emplace_back(valTmp);
                    argLocs.emplace_back(key.loc());
                    argLocs.emplace_back(value.loc());
                }

                if (auto *exp = s.kwSplat()) {
                    LocalRef temp = cctx.newTemporary(core::Names::statTemp());
                    current = walk(cctx.withTarget(temp), *exp, current);
                    args.emplace_back(temp);
                    argLocs.emplace_back(exp->loc());
                }

                if (auto *block = s.block()) {
                    auto &blockParams = block->params;
                    vector<core::ParsedParam> blockParamFlags = ast::ParamParsing::parseParams(blockParams);
                    vector<core::ParamInfo::Flags> paramFlags;
                    for (auto &e : blockParamFlags) {
                        paramFlags.emplace_back(e.flags);
                    }
                    auto link = make_shared<core::SendAndBlockLink>(s.fun, block->loc, move(paramFlags));
                    auto send = make_insn<Send>(recv, s.recv.loc(), s.fun, s.funLoc, s.numPosArgs(), args,
                                                std::move(argLocs), !!s.flags.isPrivateOk, link);
                    LocalRef sendTemp = cctx.newTemporary(core::Names::blockPreCallTemp());
                    auto solveConstraint = make_insn<SolveConstraint>(link, sendTemp);
                    current->exprs.emplace_back(sendTemp, s.loc, move(send));
                    LocalRef restoreSelf = cctx.newTemporary(core::Names::selfRestore());
                    synthesizeExpr(current, restoreSelf, core::LocOffsets::none(),
                                   make_insn<Ident>(LocalRef::selfVariable()));

                    auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1);
                    // solveConstraintBlock is only entered if break is not called
                    // in the block body.
                    auto solveConstraintBlock = cctx.inWhat.freshBlock(cctx.loops);
                    auto postBlock = cctx.inWhat.freshBlock(cctx.loops);
                    auto bodyLoops = cctx.loops + 1;
                    auto bodyBlock = cctx.inWhat.freshBlock(bodyLoops);

                    bodyBlock->exprs.emplace_back(LocalRef::selfVariable(), s.loc,
                                                  make_insn<LoadSelf>(link, LocalRef::selfVariable()));

                    auto *argBlock = bodyBlock;
                    if (!blockParamFlags.empty()) {
                        LocalRef argTemp = cctx.newTemporary(core::Names::blkArg());
                        bodyBlock->exprs.emplace_back(argTemp, s.block()->loc, make_insn<LoadYieldParams>(link));

                        for (int i = 0; i < blockParamFlags.size(); ++i) {
                            auto &arg = blockParamFlags[i];
                            LocalRef argLoc = cctx.inWhat.enterLocal(arg.local);

                            if (arg.flags.isRepeated) {
                                // Mixing positional and rest args in blocks is
                                // not currently supported, but we'll handle that in
                                // inference.
                                argBlock->exprs.emplace_back(argLoc, arg.loc,
                                                             make_insn<YieldLoadArg>(i, arg.flags, argTemp));
                                continue;
                            }

                            if (auto opt = ast::cast_tree<ast::OptionalParam>(blockParams[i])) {
                                auto *presentBlock = cctx.inWhat.freshBlock(bodyLoops);
                                auto *missingBlock = cctx.inWhat.freshBlock(bodyLoops);

                                // add a test for YieldParamPresent
                                auto present = cctx.newTemporary(core::Names::argPresent());
                                synthesizeExpr(argBlock, present, arg.loc,
                                               make_insn<YieldParamPresent>(static_cast<uint16_t>(i)));
                                conditionalJump(argBlock, present, presentBlock, missingBlock, cctx.inWhat, arg.loc);

                                // make a new block for the present and missing blocks to join
                                argBlock = cctx.inWhat.freshBlock(bodyLoops);

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
                    }

                    conditionalJump(headerBlock, LocalRef::blockCall(), bodyBlock, solveConstraintBlock, cctx.inWhat,
                                    s.loc);

                    unconditionalJump(current, headerBlock, cctx.inWhat, s.loc);

                    LocalRef blockrv = cctx.newTemporary(core::Names::blockReturnTemp());
                    BasicBlock *blockLast;
                    {
                        auto newCctx = cctx.withTarget(blockrv)
                                           .withBlockBreakTarget(cctx.target)
                                           .withLoopScope(headerBlock, postBlock, true)
                                           .withSendAndBlockLink(link);
                        if (isKernelLambda(s)) {
                            newCctx.isInsideLambda = true;
                        }
                        blockLast = walk(newCctx, s.block()->body, argBlock);
                    }
                    if (blockLast != cctx.inWhat.deadBlock()) {
                        LocalRef dead = cctx.newTemporary(core::Names::blockReturnTemp());

                        core::LocOffsets blockReturnLoc = s.block()->loc;
                        if (blockLast->exprs.empty() || isa_instruction<LoadSelf>(blockLast->exprs.back().value) ||
                            isa_instruction<YieldLoadArg>(blockLast->exprs.back().value)) {
                            auto blockEndPos = blockReturnLoc.copyEndWithZeroLength();
                            auto endKwLoc = cctx.ctx.locAt(blockEndPos).adjustLen(cctx.ctx, -3, 3);
                            auto endBraceLoc = cctx.ctx.locAt(blockEndPos).adjustLen(cctx.ctx, -1, 1);
                            if (endKwLoc.source(cctx.ctx) == "end") {
                                blockReturnLoc = endKwLoc.offsets();
                            } else if (endBraceLoc.source(cctx.ctx) == "}") {
                                blockReturnLoc = endBraceLoc.offsets();
                            }
                        } else {
                            blockReturnLoc = blockLast->exprs.back().loc;
                        }

                        synthesizeExpr(blockLast, dead, blockReturnLoc,
                                       make_insn<BlockReturn>(std::move(link), blockrv));
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
                                                                args, std::move(argLocs), !!s.flags.isPrivateOk));
                }

                ret = current;
            },

            [&](const ast::Block &a) { Exception::raise("should never encounter a bare Block"); },

            [&](ast::Next &a) { ret = walkBlockReturn(cctx, a.loc, a.expr, current); },

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
                                                   core::LocOffsets::none(), args.size(), args, std::move(locs),
                                                   isPrivateOk));
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
                                                   args.size(), args, std::move(argLocs), isPrivateOk));
                    unconditionalJump(current, cctx.rescueScope, cctx.inWhat, a.loc);
                }
                ret = cctx.inWhat.deadBlock();
            },

            [&](ast::Rescue &a) {
                auto rescueHeaderBlock = cctx.inWhat.freshBlock(cctx.loops);
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
                auto rescueHandlersBlock = cctx.inWhat.freshBlock(cctx.loops);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops);
                auto exceptionValue = cctx.newTemporary(core::Names::exceptionValue());
                // In `rescue; ...; end`, we don't want the conditional jumps' variables nor the
                // GetCurrentException calls to look like they blame to the whole `rescue; ...; end`
                // body. Better to just point at the `rescue` keyword.
                //
                // Might be better to point at exception variable like the "e" in `rescue => e`, but
                // that involves picking one (of possibly many) rescueCases.
                auto rescueKeywordLoc = (!a.rescueCases.empty() && (a.rescueCases.front().loc().endPos() -
                                                                    a.rescueCases.front().loc().beginPos()) > 6)
                                            ? core::LocOffsets{a.rescueCases.front().loc().beginPos(),
                                                               a.rescueCases.front().loc().beginPos() + 6}
                                            : a.loc.copyWithZeroLength();
                synthesizeExpr(rescueHeaderBlock, exceptionValue, rescueKeywordLoc, make_insn<GetCurrentException>());
                conditionalJump(rescueHeaderBlock, exceptionValue, rescueHandlersBlock, bodyBlock, cctx.inWhat,
                                rescueKeywordLoc);

                // cctx.loops += 1; // should formally be here but this makes us report a lot of false errors
                bodyBlock = walk(cctx, a.body, bodyBlock);

                // else is only executed if body didn't raise an exception
                auto elseBody = cctx.inWhat.freshBlock(cctx.loops);
                synthesizeExpr(bodyBlock, exceptionValue, rescueKeywordLoc, make_insn<GetCurrentException>());
                conditionalJump(bodyBlock, exceptionValue, rescueHandlersBlock, elseBody, cctx.inWhat,
                                rescueKeywordLoc);

                elseBody = walk(cctx, a.else_, elseBody);
                auto ensureBody = cctx.inWhat.freshBlock(cctx.loops);
                unconditionalJump(elseBody, ensureBody, cctx.inWhat, a.loc);

                for (auto &expr : a.rescueCases) {
                    auto rescueCase = ast::cast_tree<ast::RescueCase>(expr);
                    auto caseBody = cctx.inWhat.freshBlock(cctx.loops);
                    auto &exceptions = rescueCase->exceptions;
                    auto local = ast::cast_tree<ast::Local>(rescueCase->var);
                    ENFORCE(local != nullptr, "rescue case var not a local?");

                    auto localVar = cctx.inWhat.enterLocal(local->localVariable);
                    caseBody->exprs.emplace_back(localVar, rescueCase->var.loc(), make_insn<Ident>(exceptionValue));

                    // We don't support typed exceptions in `ensure` yet.
                    // We have a lot of tests that show why, but it boils down to a combination of
                    // Sorbet's "simplified view of the control flow" for exceptions (see comment above)
                    // and Sorbet's dead code detection.
                    if (!ast::isa_tree<ast::EmptyTree>(a.ensure)) {
                        auto zloc = rescueCase->var.loc().copyWithZeroLength();
                        auto unsafe = ast::MK::Unsafe(
                            zloc, ast::make_expression<ast::Local>(zloc, exceptionValue.data(cctx.inWhat)));
                        ensureBody = walk(cctx.withTarget(localVar), unsafe, ensureBody);
                    }

                    // Mark the exception as handled
                    synthesizeExpr(caseBody, exceptionValue, core::LocOffsets::none(),
                                   make_insn<Literal>(core::Types::nilClass()));

                    auto res = cctx.newTemporary(core::Names::keepForCfgTemp());
                    synthesizeExpr(caseBody, res, rescueCase->loc, make_insn<KeepAlive>(exceptionValue));

                    if (exceptions.empty()) {
                        // rescue without a class catches StandardError
                        auto ex = ast::MK::Constant(rescueCase->var.loc(), core::Symbols::StandardError());
                        rescueHandlersBlock =
                            buildExceptionHandler(cctx, ex, caseBody, exceptionValue, rescueHandlersBlock);
                    } else {
                        for (auto &ex : exceptions) {
                            rescueHandlersBlock =
                                buildExceptionHandler(cctx, ex, caseBody, exceptionValue, rescueHandlersBlock);
                        }
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
                ret = cctx.inWhat.freshBlock(cctx.loops);
                conditionalJump(ensureBody, gotoDeadTemp, cctx.inWhat.deadBlock(), ret, cctx.inWhat, a.loc);
            },

            [&](ast::Hash &h) { ret = walkHash(cctx, h, current, core::Names::buildHash()); },

            [&](ast::Array &a) {
                LocalRef magic = cctx.newTemporary(core::Names::magic());
                InlinedVector<LocalRef, 2> vars;
                InlinedVector<core::LocOffsets, 2> locs;
                for (auto &elem : a.elems) {
                    LocalRef tmp = cctx.newTemporary(core::Names::arrayTemp());
                    current = walk(cctx.withTarget(tmp), elem, current);
                    vars.emplace_back(tmp);
                    locs.emplace_back(a.loc);
                }
                synthesizeExpr(current, magic, core::LocOffsets::none(), make_insn<Alias>(core::Symbols::Magic()));
                auto isPrivateOk = false;
                current->exprs.emplace_back(cctx.target, a.loc,
                                            make_insn<Send>(magic, a.loc, core::Names::buildArray(),
                                                            core::LocOffsets::none(), vars.size(), vars,
                                                            std::move(locs), isPrivateOk));
                ret = current;
            },

            [&](ast::Cast &c) {
                // c.typeExpr will be nullptr in the lambdaTLet case (i.e., T.let(->(){}, ...)).
                // It's moved into the `Kernel#<lambda T.let>`
                if (c.typeExpr != nullptr) {
                    // This is kind of gross, but it is the only way to ensure that the bits in the
                    // type expression make it into the CFG for LSP to hit on their locations.
                    LocalRef deadSym = cctx.newTemporary(core::Names::keepForIde());
                    current = walk(cctx.withTarget(deadSym), c.typeExpr, current);
                    // Ensure later passes don't delete the results of the typeExpr.
                    current->exprs.emplace_back(deadSym, core::LocOffsets::none(), make_insn<KeepAlive>(deadSym));
                }
                LocalRef tmp = cctx.newTemporary(core::Names::castTemp());
                core::LocOffsets argLoc = c.arg.loc();
                current = walk(cctx.withTarget(tmp), c.arg, current);
                if (c.cast == core::Names::uncheckedLet()) {
                    current->exprs.emplace_back(cctx.target, c.loc, make_insn<Ident>(tmp));
                } else if (c.cast == core::Names::bind() || c.cast == core::Names::syntheticBind()) {
                    auto isSynthetic = c.cast == core::Names::syntheticBind();
                    if (c.arg.isSelfReference()) {
                        auto self = LocalRef::selfVariable();
                        auto &inserted = current->exprs.emplace_back(
                            self, c.loc, make_insn<Cast>(tmp, argLoc, c.type, core::Names::cast()));
                        if (isSynthetic) {
                            inserted.value.setSynthetic();
                        }
                        current->exprs.emplace_back(cctx.target, c.loc, make_insn<Ident>(self));

                        if (cctx.rescueScope) {
                            cctx.rescueScope->exprs.emplace_back(
                                self, c.loc, make_insn<Cast>(tmp, argLoc, c.type, core::Names::cast()));
                            cctx.rescueScope->exprs.emplace_back(cctx.target, c.loc, make_insn<Ident>(self));
                        }
                    } else {
                        if (auto e = cctx.ctx.beginError(what.loc(), core::errors::CFG::MalformedTBind)) {
                            e.setHeader("`{}` can only be used with `{}`", "T.bind", "self");
                        }
                    }
                } else {
                    current->exprs.emplace_back(cctx.target, c.loc, make_insn<Cast>(tmp, argLoc, c.type, c.cast));
                }
                if (c.cast == core::Names::let()) {
                    cctx.inWhat.minLoops[cctx.target.id()] = CFG::MIN_LOOP_LET;
                }

                ret = current;
            },

            [&](ast::RuntimeMethodDefinition &rmd) {
                current->exprs.emplace_back(
                    cctx.target, rmd.loc.copyWithZeroLength(),
                    make_insn<Literal>(core::make_type<core::NamedLiteralType>(core::Symbols::Symbol(), rmd.name)));
                ret = current;
            },

            [&](const ast::EmptyTree &n) {
                // TODO(jez) We might want to ENFORCE(false) here, and make all attempts to walk an
                // empty tree be handled by the parent node where the `EmptyTree` is a child, so
                // that there's more context to be able to handle the `EmptyTree` in context. For
                // example, how the `ast::If` case handles `EmptyTree` so that the loc of the
                // `EmptyTree` can be set to the loc of the whole `if` expression.
                ret = current;
            },

            [&](const ast::ClassDef &c) { Exception::raise("Should have been removed by FlattenWalk"); },
            [&](const ast::MethodDef &c) { Exception::raise("Should have been removed by FlattenWalk"); },

            [&](const ast::ExpressionPtr &n) {
                if (n == nullptr) {
                    Exception::raise("Tried to convert `nullptr` to CFG in ctx.file=\"{}\"",
                                     cctx.ctx.file.data(cctx.ctx).path());
                } else {
                    Exception::raise("Unimplemented AST Node: {}", what.nodeName());
                }
            });

        // For, Rescue,
        // Symbol, Array,
        ENFORCE(ret != nullptr, "CFG builder ret unset");
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
