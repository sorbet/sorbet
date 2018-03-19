#include "environment.h"
#include "infer.h"

using namespace std;
namespace ruby_typer {

bool isSyntheticBlock(core::Context ctx, cfg::BasicBlock *bb) {
    if (bb->exprs.size() > 3) {
        return false;
    }
    core::LocalVariable allowedLocal;
    for (auto &expr : bb->exprs) {
        if (cfg::isa_instruction<cfg::DebugEnvironment>(expr.value.get())) {
            continue;
        }
        if (auto lit = cfg::cast_instruction<cfg::Literal>(expr.value.get())) {
            if (lit->value->derivesFrom(ctx, core::Symbols::NilClass())) {
                allowedLocal = expr.bind;
                continue;
            }
        }

        if (expr.bind._name == core::Names::finalReturn()) {
            auto *ret = cfg::cast_instruction<cfg::Return>(expr.value.get());
            if (ret) {
                if (ret->what == allowedLocal) {
                    continue;
                }
            }
        }

        return false;
    }
    return true;
}

unique_ptr<cfg::CFG> infer::Inference::run(core::Context ctx, unique_ptr<cfg::CFG> cfg) {
    core::counterInc("infer.methods_typechecked");
    const int startErrorCount = ctx.state.totalErrors();
    vector<Environment> outEnvironments;
    outEnvironments.resize(cfg->maxBasicBlockId);
    for (int i = 0; i < cfg->basicBlocks.size(); i++) {
        outEnvironments[cfg->forwardsTopoSort[i]->id].bb = cfg->forwardsTopoSort[i];
    }
    vector<bool> visited;
    visited.resize(cfg->maxBasicBlockId);
    KnowledgeFilter knowledgeFilter(ctx, cfg);
    if (!cfg->basicBlocks.empty()) {
        ENFORCE(!cfg->symbol.data(ctx).isAbstract());
    }
    for (auto it = cfg->forwardsTopoSort.rbegin(); it != cfg->forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        if (bb == cfg->deadBlock()) {
            continue;
        }
        Environment &current = outEnvironments[bb->id];
        current.vars.reserve(bb->args.size());
        current.types.resize(bb->args.size());
        current.knowledge.resize(bb->args.size());
        for (core::LocalVariable arg : bb->args) {
            current.vars.push_back(arg);
        }
        if (bb->backEdges.size() == 1) {
            auto *parent = bb->backEdges[0];
            bool isTrueBranch = parent->bexit.thenb == bb;
            Environment tempEnv;
            auto &envAsSeenFromBranch =
                Environment::withCond(ctx, outEnvironments[parent->id], tempEnv, isTrueBranch, current.vars);
            current.populateFrom(ctx, envAsSeenFromBranch);
        } else {
            current.isDead = (bb != cfg->entry());
            for (cfg::BasicBlock *parent : bb->backEdges) {
                if (!visited[parent->id]) {
                    continue;
                }
                bool isTrueBranch = parent->bexit.thenb == bb;
                Environment tempEnv;
                auto &envAsSeenFromBranch =
                    Environment::withCond(ctx, outEnvironments[parent->id], tempEnv, isTrueBranch, current.vars);
                if (!envAsSeenFromBranch.isDead) {
                    current.isDead = false;
                    current.mergeWith(ctx, envAsSeenFromBranch, parent->bexit.loc, *cfg.get(), bb, knowledgeFilter);
                }
            }
        }

        current.computePins(ctx, outEnvironments, *cfg.get(), bb);

        int i = -1;
        for (auto &uninitialized : current.types) {
            i++;
            if (uninitialized.type.get() == nullptr) {
                uninitialized.type = core::Types::nilClass();
                uninitialized.origins.push_back(ctx.owner.data(ctx).definitionLoc);
            } else {
                uninitialized.type->sanityCheck(ctx);
            }
        }

        visited[bb->id] = true;
        if (current.isDead) {
            // this block is unreachable.
            if (!bb->exprs.empty() && !(isSyntheticBlock(ctx, bb)) // synthetic final return
            ) {
                if (auto e = ctx.state.beginError(bb->exprs[0].loc, core::errors::Infer::DeadBranchInferencer)) {
                    e.setHeader("This code is unreachable");
                }
            }
            continue;
        }

        for (cfg::Binding &bind : bb->exprs) {
            if (!current.isDead || cfg::isa_instruction<cfg::DebugEnvironment>(bind.value.get())) {
                current.ensureGoodAssignTarget(ctx, bind.bind);
                bind.tpe = current.processBinding(ctx, bind, bb->outerLoops, cfg->minLoops[bind.bind], knowledgeFilter);
                bind.tpe->sanityCheck(ctx);
                if (bind.tpe->isBottom()) {
                    current.isDead = true;
                }
            }
        }
        if (!current.isDead) {
            current.ensureGoodCondition(ctx, bb->bexit.cond);
        }
        core::histogramInc("infer.environment.size", current.vars.size());
        for (auto &k : current.knowledge) {
            core::histogramInc("infer.knowledge.truthy.yes.size", k.truthy->yesTypeTests.size());
            core::histogramInc("infer.knowledge.truthy.no.size", k.truthy->noTypeTests.size());
            core::histogramInc("infer.knowledge.falsy.yes.size", k.falsy->yesTypeTests.size());
            core::histogramInc("infer.knowledge.falsy.no.size", k.falsy->noTypeTests.size());
        }
    }
    if (startErrorCount == ctx.state.totalErrors()) {
        core::counterInc("infer.methods_typechecked.no_errors");
    }
    return cfg;
}
} // namespace ruby_typer