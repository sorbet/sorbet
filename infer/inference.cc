#include "common/common.h"
#include "core/Loc.h"
#include "core/TypeConstraint.h"
#include "core/errors/infer.h"
#include "infer/SigSuggestion.h"
#include "infer/environment.h"
#include "infer/infer.h"

using namespace std;
namespace sorbet::infer {

unique_ptr<cfg::CFG> Inference::run(core::Context ctx, unique_ptr<cfg::CFG> cfg) {
    ENFORCE(cfg->symbol == ctx.owner);
    auto methodLoc = cfg->symbol.data(ctx)->loc();
    prodCounterInc("types.input.methods.typechecked");
    int typedSendCount = 0;
    int totalSendCount = 0;
    const int startErrorCount = ctx.state.totalErrors();
    auto guessTypes = true;
    unique_ptr<core::TypeConstraint> _constr;
    core::TypeConstraint *constr = &core::TypeConstraint::EmptyFrozenConstraint;
    if (cfg->symbol.data(ctx)->isGenericMethod()) {
        _constr = make_unique<core::TypeConstraint>();
        constr = _constr.get();
        for (core::SymbolRef typeArgument : cfg->symbol.data(ctx)->typeArguments()) {
            constr->rememberIsSubtype(ctx, typeArgument.data(ctx)->resultType,
                                      core::make_type<core::SelfTypeParam>(typeArgument));
        }
        if (!constr->solve(ctx)) {
            Exception::raise("should never happen");
        }
        guessTypes = false;
    }

    core::TypePtr methodReturnType = cfg->symbol.data(ctx)->resultType;
    auto missingReturnType = methodReturnType == nullptr;

    if (cfg->symbol.data(ctx)->name.data(ctx)->kind != core::NameKind::UTF8 ||
        cfg->symbol.data(ctx)->name == core::Names::staticInit() || !cfg->symbol.data(ctx)->loc().exists()) {
        guessTypes = false;
    }

    if (missingReturnType) {
        if (guessTypes) {
            ENFORCE(constr->isSolved() && constr->isEmpty());
            _constr = make_unique<core::TypeConstraint>();
            constr = _constr.get();
            auto returnTypeVar =
                core::Symbols::Sorbet::Private::Static_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_contravariant();
            InlinedVector<core::SymbolRef, 4> domainTemp;
            domainTemp.emplace_back(returnTypeVar);
            methodReturnType = returnTypeVar.data(ctx)->resultType;

            constr->defineDomain(ctx, domainTemp);
        } else {
            methodReturnType = core::Types::untyped(ctx, cfg->symbol);
        }
    } else {
        auto enclosingClass = cfg->symbol.data(ctx)->enclosingClass(ctx);
        methodReturnType =
            core::Types::instantiate(ctx,
                                     core::Types::resultTypeAsSeenFrom(ctx, cfg->symbol, enclosingClass,
                                                                       enclosingClass.data(ctx)->selfTypeArgs(ctx)),
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
    if (!cfg->basicBlocks.empty()) {
        ENFORCE(!cfg->symbol.data(ctx)->isAbstract());
    } else {
        ENFORCE(cfg->symbol.data(ctx)->isAbstract());
        return cfg;
    }
    for (auto it = cfg->forwardsTopoSort.rbegin(); it != cfg->forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        if (bb == cfg->deadBlock()) {
            continue;
        }
        Environment &current = outEnvironments[bb->id];
        current.vars.reserve(bb->args.size());
        for (cfg::VariableUseSite &arg : bb->args) {
            current.vars[arg.variable].typeAndOrigins.type = nullptr;
        }
        if (bb->backEdges.size() == 1) {
            auto *parent = bb->backEdges[0];
            bool isTrueBranch = parent->bexit.thenb == bb;
            if (!outEnvironments[parent->id].isDead) {
                Environment tempEnv(methodLoc);
                auto &envAsSeenFromBranch =
                    Environment::withCond(ctx, outEnvironments[parent->id], tempEnv, isTrueBranch, current.vars);
                current.populateFrom(ctx, envAsSeenFromBranch);
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
                    Environment::withCond(ctx, outEnvironments[parent->id], tempEnv, isTrueBranch, current.vars);
                if (!envAsSeenFromBranch.isDead) {
                    current.isDead = false;
                    current.mergeWith(ctx, envAsSeenFromBranch, parent->bexit.loc, *cfg.get(), bb, knowledgeFilter);
                }
            }
        }

        current.computePins(ctx, outEnvironments, *cfg.get(), bb);

        for (auto &uninitialized : current.vars) {
            if (uninitialized.second.typeAndOrigins.type.get() == nullptr) {
                uninitialized.second.typeAndOrigins.type = core::Types::nilClass();
                uninitialized.second.typeAndOrigins.origins.emplace_back(cfg->symbol.data(ctx)->loc());
            } else {
                uninitialized.second.typeAndOrigins.type->sanityCheck(ctx);
            }
        }

        for (auto &blockArg : bb->args) {
            current.getAndFillTypeAndOrigin(ctx, blockArg);
        }

        visited[bb->id] = true;
        if (current.isDead) {
            bb->firstDeadInstructionIdx = 0;
            // this block is unreachable.
            if (!bb->exprs.empty()) {
                for (auto &expr : bb->exprs) {
                    if (expr.value->isSynthetic) {
                        continue;
                    }
                    if (auto e = ctx.state.beginError(expr.loc, core::errors::Infer::DeadBranchInferencer)) {
                        e.setHeader("This code is unreachable");
                    }
                    break;
                }
            }
            continue;
        }

        core::Loc madeBlockDead;
        int i = 0;
        for (cfg::Binding &bind : bb->exprs) {
            i++;
            if (!current.isDead) {
                current.ensureGoodAssignTarget(ctx, bind.bind.variable);
                bind.bind.type = current.processBinding(ctx, bind, bb->outerLoops, cfg->minLoops[bind.bind.variable],
                                                        knowledgeFilter, *constr, methodReturnType);
                if (cfg::isa_instruction<cfg::Send>(bind.value.get())) {
                    totalSendCount++;
                    if (bind.bind.type && !bind.bind.type->isUntyped()) {
                        typedSendCount++;
                    } else if (bind.bind.type->hasUntyped()) {
                        DEBUG_ONLY(histogramInc("untyped.sources", bind.bind.type->untypedBlame()._id););
                        if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::UntypedValue)) {
                            e.setHeader("This code is untyped");
                        }
                    }
                }
                ENFORCE(bind.bind.type);
                bind.bind.type->sanityCheck(ctx);
                if (bind.bind.type->isBottom()) {
                    current.isDead = true;
                    madeBlockDead = bind.loc;
                }
                if (current.isDead) {
                    // this can also be result of evaluating an instruction, e.g. an always false hard_assert
                    bb->firstDeadInstructionIdx = i;
                }
            } else if (current.isDead && !bind.value->isSynthetic) {
                if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::DeadBranchInferencer)) {
                    e.setHeader("This code is unreachable");
                    e.addErrorLine(madeBlockDead, "This expression can never be computed");
                }
                break;
            }
        }
        if (!current.isDead) {
            ENFORCE(bb->firstDeadInstructionIdx == -1);
            current.getAndFillTypeAndOrigin(ctx, bb->bexit.cond);
            current.ensureGoodCondition(ctx, bb->bexit.cond.variable);
        } else {
            ENFORCE(bb->firstDeadInstructionIdx != -1);
        }
        histogramInc("infer.environment.size", current.vars.size());
        for (auto &pair : current.vars) {
            auto &k = pair.second.knowledge;
            histogramInc("infer.knowledge.truthy.yes.size", k.truthy->yesTypeTests.size());
            histogramInc("infer.knowledge.truthy.no.size", k.truthy->noTypeTests.size());
            histogramInc("infer.knowledge.falsy.yes.size", k.falsy->yesTypeTests.size());
            histogramInc("infer.knowledge.falsy.no.size", k.falsy->noTypeTests.size());
        }
    }
    if (startErrorCount == ctx.state.totalErrors()) {
        counterInc("infer.methods_typechecked.no_errors");
    }

    if ((missingReturnType || cfg->symbol.data(ctx)->hasGeneratedSig()) && guessTypes) {
        if (auto e = ctx.state.beginError(cfg->symbol.data(ctx)->loc(), core::errors::Infer::UntypedMethod)) {
            e.setHeader("This function does not have a `sig`");
            SigSuggestion::maybeSuggestSig(ctx, e, cfg, methodReturnType, *constr);
        }
    }

    prodCounterAdd("types.input.sends.typed", typedSendCount);
    prodCounterAdd("types.input.sends.total", totalSendCount);

    return cfg;
}
} // namespace sorbet::infer
