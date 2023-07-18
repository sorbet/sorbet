#include "common/common.h"
#include "common/timers/Timer.h"
#include "core/Loc.h"
#include "core/TypeConstraint.h"
#include "core/TypeErrorDiagnostics.h"
#include "core/errors/infer.h"
#include "core/lsp/QueryResponse.h"
#include "infer/SigSuggestion.h"
#include "infer/environment.h"
#include "infer/infer.h"

using namespace std;
namespace sorbet::infer {

unique_ptr<cfg::CFG> Inference::run(core::Context ctx, unique_ptr<cfg::CFG> cfg) {
    Timer timeit(ctx.state.tracer(), "Inference::run", {{"func", string(cfg->symbol.toStringFullName(ctx))}});
    ENFORCE(cfg->symbol == ctx.owner.asMethodRef());
    auto methodLoc = cfg->symbol.data(ctx)->loc();
    prodCounterInc("types.input.methods.typechecked");
    int typedSendCount = 0;
    int totalSendCount = 0;
    const int startErrorCount = ctx.state.totalErrors();
    auto guessTypes = true;
    unique_ptr<core::TypeConstraint> _constr;
    core::TypeConstraint *constr = &core::TypeConstraint::EmptyFrozenConstraint;
    if (cfg->symbol.data(ctx)->flags.isGenericMethod) {
        _constr = make_unique<core::TypeConstraint>();
        constr = _constr.get();
        for (auto typeArgument : cfg->symbol.data(ctx)->typeArguments()) {
            constr->rememberIsSubtype(ctx, typeArgument.data(ctx)->resultType,
                                      core::make_type<core::SelfTypeParam>(typeArgument));
        }
        if (!constr->solve(ctx)) {
            Exception::raise("Constraint should always solve after creating empty TypeConstraint with all upper bounds "
                             "set to to SelfTypeParam of itself");
        }
        guessTypes = false;
    }

    core::TypePtr methodReturnType = cfg->symbol.data(ctx)->resultType;
    auto missingReturnType = methodReturnType == nullptr;

    if (cfg->symbol.data(ctx)->name.kind() != core::NameKind::UTF8 ||
        cfg->symbol.data(ctx)->name == core::Names::staticInit() || !cfg->symbol.data(ctx)->loc().exists()) {
        guessTypes = false;
    }

    if (missingReturnType) {
        if (guessTypes) {
            ENFORCE(constr->isSolved() && constr->isEmpty());
            _constr = make_unique<core::TypeConstraint>();
            constr = _constr.get();
            auto returnTypeVar = core::Symbols::
                Sorbet_Private_Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_contravariant();
            InlinedVector<core::TypeArgumentRef, 4> domainTemp;
            domainTemp.emplace_back(returnTypeVar);
            methodReturnType = returnTypeVar.data(ctx)->resultType;

            constr->defineDomain(ctx, domainTemp);
        } else if (cfg->symbol.data(ctx)->name.isAnyStaticInitName(ctx)) {
            methodReturnType = core::Types::top();
        } else {
            methodReturnType = core::Types::untyped(cfg->symbol);
        }
    } else {
        auto enclosingClass = cfg->symbol.enclosingClass(ctx);
        methodReturnType = core::Types::instantiate(
            ctx,
            core::Types::resultTypeAsSeenFrom(ctx, cfg->symbol.data(ctx)->resultType, cfg->symbol.data(ctx)->owner,
                                              enclosingClass, enclosingClass.data(ctx)->selfTypeArgs(ctx)),
            *constr);
        methodReturnType = core::Types::replaceSelfType(ctx, methodReturnType, enclosingClass.data(ctx)->selfType(ctx));
    }

    vector<Environment> outEnvironments;
    outEnvironments.reserve(cfg->maxBasicBlockId);
    for (int i = 0; i < cfg->maxBasicBlockId; i++) {
        outEnvironments.emplace_back(methodLoc);
    }
    for (int i = 0; i < cfg->basicBlocks.size(); i++) {
        outEnvironments[cfg->forwardsTopoSort[i]->id].bb = cfg->forwardsTopoSort[i];
    }
    vector<bool> visited;
    visited.resize(cfg->maxBasicBlockId);
    KnowledgeFilter knowledgeFilter(ctx, cfg);
    for (auto it = cfg->forwardsTopoSort.rbegin(); it != cfg->forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        if (bb == cfg->deadBlock()) {
            for (const auto &bind : bb->exprs) {
                if (bind.value.isSynthetic() || bind.loc.empty()) {
                    continue;
                }

                if (auto e = ctx.beginError(bind.loc, core::errors::Infer::DeadBranchInferencer)) {
                    e.setHeader("This expression appears after an unconditional return");
                }

                // Only report the expression expression in the dead block
                break;
            }
            continue;
        }
        Environment &current = outEnvironments[bb->id];
        current.initializeBasicBlockArgs(*bb);

        // We very much want to limit access to "global" data structures downstream.
        // In particular, processBinding should only need to know about the current binding (nothing
        // about the surrounding BasicBlock, and definitely nothing about BasicBlocks that don't
        // contain the current binding being processed).
        //
        // That means that only here in inference do we know about the structure of the CFG.
        // But we want to pluck some limited information about the parent block and pass it into
        // processBinding, only so that some errors that would already be reported can be improved.
        // parentUpdateKnowledgeReceiver is such a thing.
        //
        // In general we should carefully limit the set of information that processBinding requires,
        // to make it easy to reason about correctness.
        optional<cfg::BasicBlock::BlockExitCondInfo> parentUpdateKnowledgeReceiver;

        if (bb->backEdges.size() == 1) {
            auto *parent = bb->backEdges[0];
            bool isTrueBranch = parent->bexit.thenb == bb;
            if (!outEnvironments[parent->id].isDead) {
                Environment tempEnv(methodLoc);
                auto &envAsSeenFromBranch =
                    Environment::withCond(ctx, outEnvironments[parent->id], tempEnv, isTrueBranch, current.vars());
                current.populateFrom(ctx, envAsSeenFromBranch);

                parentUpdateKnowledgeReceiver = parent->maybeGetUpdateKnowledgeReceiver(*cfg);
            } else {
                current.isDead = true;
            }
        } else {
            current.isDead = (bb != cfg->entry());
            for (cfg::BasicBlock *parent : bb->backEdges) {
                if (!visited[parent->id] || outEnvironments[parent->id].isDead) {
                    continue;
                }
                bool isTrueBranch = parent->bexit.thenb == bb;
                Environment tempEnv(methodLoc);
                auto &envAsSeenFromBranch =
                    Environment::withCond(ctx, outEnvironments[parent->id], tempEnv, isTrueBranch, current.vars());
                if (!envAsSeenFromBranch.isDead) {
                    current.isDead = false;
                    current.mergeWith(ctx, envAsSeenFromBranch, *cfg.get(), bb, knowledgeFilter);
                }
            }
        }

        current.computePins(ctx, outEnvironments, *cfg.get(), bb);
        current.setUninitializedVarsToNil(ctx, cfg->symbol.data(ctx)->loc());

        for (auto &blockArg : bb->args) {
            current.getAndFillTypeAndOrigin(ctx, blockArg);
        }

        visited[bb->id] = true;
        if (current.isDead) {
            bb->firstDeadInstructionIdx = 0;
            // this block is unreachable.
            if (!bb->exprs.empty()) {
                // This is a bit complicated:
                //
                //   1. If this block is only dead because all jumps into this block are dead,
                //      we already reported an error and don't want a duplicate.
                //   2. If the block consists only of synthetic bindings or T.absurd, we don't
                //      want to issue an error.
                //   3. If the block contains a send of the form <Magic>.<nil-for-safe-navigation>(x),
                //      we want to issue an UnnecessarySafeNavigationError, extracting
                //      type-and-origin info from x. (This magic form is inserted by the desugarer
                //      for a "safe navigation" operation, e.g., `x&.foo`.)
                //   4. Otherwise, we want to issue a DeadBranchInferencer error, taking the first
                //      (non-synthetic, non-"T.absurd") instruction in the block as the loc of the
                //      error.
                cfg::InstructionPtr *unreachableInstruction = nullptr;
                core::Loc locForUnreachable;
                bool dueToSafeNavigation = false;

                if (absl::c_any_of(bb->backEdges, [&](const auto &bb) { return !outEnvironments[bb->id].isDead; })) {
                    for (auto &expr : bb->exprs) {
                        if (expr.value.isSynthetic()) {
                            continue;
                        }
                        if (cfg::isa_instruction<cfg::TAbsurd>(expr.value)) {
                            continue;
                        }

                        auto send = cfg::cast_instruction<cfg::Send>(expr.value);
                        if (send != nullptr && send->fun == core::Names::nilForSafeNavigation()) {
                            ENFORCE(send->args.size() == 1, "Broken invariant from desugar");
                            unreachableInstruction = &expr.value;
                            locForUnreachable = core::Loc(ctx.file, send->argLocs[0]);

                            // The arg loc for the synthetic variable created for the purpose of this safe navigation
                            // check is a bit of a hack. It's intentionally one character too short so that for
                            // completion requests it doesn't match `x&.|` (which would defeat completion requests.)
                            auto maybeExpand = locForUnreachable.adjust(ctx, 0, 1);
                            if (maybeExpand.source(ctx) == "&.") {
                                locForUnreachable = maybeExpand;
                            }
                            dueToSafeNavigation = true;
                            break;
                        } else if (unreachableInstruction == nullptr) {
                            unreachableInstruction = &expr.value;
                            locForUnreachable = ctx.locAt(expr.loc);
                        } else {
                            // Expand the loc to cover the entire dead basic block
                            locForUnreachable = locForUnreachable.join(ctx.locAt(expr.loc));
                        }
                    }
                }

                if (unreachableInstruction != nullptr) {
                    auto *send = cfg::cast_instruction<cfg::Send>(*unreachableInstruction);
                    if (dueToSafeNavigation && send != nullptr) {
                        if (auto e = ctx.state.beginError(locForUnreachable,
                                                          core::errors::Infer::UnnecessarySafeNavigation)) {
                            auto ty = current.getAndFillTypeAndOrigin(ctx, send->args[0]);

                            e.setHeader("Used `{}` operator on `{}`, which can never be nil", "&.", ty.type.show(ctx));
                            e.addErrorSection(ty.explainGot(ctx, current.locForUninitialized()));
                            if (locForUnreachable.source(ctx) == "&.") {
                                e.replaceWith("Replace with `.`", locForUnreachable, ".");
                            }
                        }
                    } else if (auto e =
                                   ctx.state.beginError(locForUnreachable, core::errors::Infer::DeadBranchInferencer)) {
                        auto *ident = cfg::cast_instruction<cfg::Ident>(*unreachableInstruction);

                        bool andAndOrOr = false;
                        if (ident != nullptr) {
                            auto name = ident->what.data(*cfg)._name;
                            if (name.kind() == core::NameKind::UNIQUE &&
                                name.dataUnique(ctx)->original == core::Names::andAnd()) {
                                e.setHeader("Left side of `{}` condition was always `{}`", "&&", "truthy");
                                andAndOrOr = true;
                            } else if (name.kind() == core::NameKind::UNIQUE &&
                                       name.dataUnique(ctx)->original == core::Names::orOr()) {
                                e.setHeader("Left side of `{}` condition was always `{}`", "||", "falsy");
                                andAndOrOr = true;
                            }
                        }
                        if (!andAndOrOr) {
                            e.setHeader("This code is unreachable");
                        }

                        for (const auto &prevBasicBlock : bb->backEdges) {
                            const auto &prevEnv = outEnvironments[prevBasicBlock->id];
                            if (prevEnv.isDead) {
                                // This prevous block doesn't actually matter, because it was dead
                                // (never got to evaluating its jump condition), so don't clutter
                                // the error message.
                                continue;
                            }

                            const auto &cond = prevBasicBlock->bexit.cond;
                            if (cond.type == nullptr) {
                                // This previous block is actually a future block we haven't processed yet.
                                // (Remember: our inference pass is an approximate forwards toposort
                                // of a graph that can have cycles). It can't have been a block that
                                // caused the current error.
                                continue;
                            }

                            auto alwaysWhat = prevBasicBlock->bexit.thenb->id == bb->id ? "falsy" : "truthy";
                            auto bexitLoc = ctx.locAt(prevBasicBlock->bexit.loc);
                            e.addErrorLine(bexitLoc, "This condition was always `{}` (`{}`)", alwaysWhat,
                                           cond.type.show(ctx));

                            if (ctx.state.suggestUnsafe.has_value() && bexitLoc.exists()) {
                                e.replaceWith(fmt::format("Wrap in `{}`", *ctx.state.suggestUnsafe), bexitLoc, "{}({})",
                                              *ctx.state.suggestUnsafe, bexitLoc.source(ctx).value());
                            }

                            auto ty = prevEnv.getTypeAndOrigin(ctx, cond.variable);
                            e.addErrorSection(ty.explainGot(ctx, prevEnv.locForUninitialized()));
                        }

                        if (andAndOrOr) {
                            e.addErrorNote("If this is intentional, either delete the redundant code or restructure\n"
                                           "    it to use `{}` so that Sorbet can check for exhaustiveness.",
                                           "T.absurd");
                        }
                    }
                }
            }
            continue;
        }

        core::Loc madeBlockDead;
        int i = 0;
        for (cfg::Binding &bind : bb->exprs) {
            i++;
            if (!current.isDead || !ctx.state.lspQuery.isEmpty()) {
                bind.bind.type =
                    current.processBinding(ctx, *cfg, bind, bb->outerLoops, bind.bind.variable.minLoops(*cfg),
                                           knowledgeFilter, *constr, methodReturnType, parentUpdateKnowledgeReceiver);
                if (cfg::isa_instruction<cfg::Send>(bind.value)) {
                    totalSendCount++;
                    if (bind.bind.type && !bind.bind.type.isUntyped()) {
                        typedSendCount++;
                    }
                }
                ENFORCE(bind.bind.type);
                bind.bind.type.sanityCheck(ctx);
                if (bind.bind.type.isBottom()) {
                    current.isDead = true;
                    madeBlockDead = ctx.locAt(bind.loc);
                }
                if (current.isDead && bb->firstDeadInstructionIdx == -1) {
                    // this can also be result of evaluating an instruction, e.g. an always false hard_assert
                    bb->firstDeadInstructionIdx = i;
                }
            } else if (ctx.state.lspQuery.isEmpty() && current.isDead && !bind.value.isSynthetic()) {
                if (auto e = ctx.beginError(bind.loc, core::errors::Infer::DeadBranchInferencer)) {
                    e.setHeader("This code is unreachable");
                    e.addErrorLine(madeBlockDead, "This expression always raises or can never be computed");
                }
                break;
            }
        }
        if (!current.isDead) {
            ENFORCE(bb->firstDeadInstructionIdx == -1);
            auto bexitTpo = current.getAndFillTypeAndOrigin(ctx, bb->bexit.cond);
            if (bexitTpo.type.isUntyped()) {
                auto what = core::errors::Infer::errorClassForUntyped(ctx, ctx.file, bexitTpo.type);
                if (auto e = ctx.beginError(bb->bexit.loc, what)) {
                    e.setHeader("Conditional branch on `{}`", "T.untyped");
                    core::TypeErrorDiagnostics::explainUntyped(ctx, e, what, bexitTpo, methodLoc);
                }
            }
        } else {
            ENFORCE(bb->firstDeadInstructionIdx != -1);
        }
        histogramInc("infer.environment.size", current.vars().size());
        for (auto &pair : current.vars()) {
            pair.second.knowledge.emitKnowledgeSizeMetric();
        }
    }
    if (startErrorCount == ctx.state.totalErrors()) {
        counterInc("infer.methods_typechecked.no_errors");
    }

    if (missingReturnType && guessTypes) {
        if (auto e = ctx.state.beginError(cfg->symbol.data(ctx)->loc(), core::errors::Infer::UntypedMethod)) {
            e.setHeader("The method `{}` does not have a `{}`", cfg->symbol.data(ctx)->name.show(ctx), "sig");
            auto maybeAutocorrect = SigSuggestion::maybeSuggestSig(ctx, cfg, methodReturnType, *constr);
            if (maybeAutocorrect.has_value()) {
                e.addAutocorrect(move(maybeAutocorrect.value()));
            } else if (cfg->symbol.data(ctx)->owner.data(ctx)->derivesFrom(ctx, core::Symbols::Struct())) {
                e.addErrorNote("Struct classes defined with `{}` are hard to use in `{}` files.\n"
                               "    Consider using `{}` instead.",
                               "Struct", "# typed: strict", "T::Struct");
            }
        } else if (ctx.state.lspQuery.matchesSuggestSig(cfg->symbol)) {
            // Force maybeSuggestSig to run just to respond to the query (discard the result)
            SigSuggestion::maybeSuggestSig(ctx, cfg, methodReturnType, *constr);
        }
    }

    // TODO(jez) Delete these?
    prodCounterAdd("types.input.sends.typed", typedSendCount);
    prodCounterAdd("types.input.sends.total", totalSendCount);

    return cfg;
}
} // namespace sorbet::infer
