#include "core/Loc.h"
#include "core/TypeConstraint.h"
#include "core/errors/infer.h"
#include "infer/environment.h"
#include "infer/infer.h"

#include "absl/algorithm/container.h"

using namespace std;
namespace sorbet::infer {

namespace {
struct OffsetAndPadding {
    u4 startOffset;
    u4 startPadding;
};

// TODO(jez) Might want to factor this out if we add more autocorrects that are "inject a line above this line"
OffsetAndPadding getStartOffset(core::Context ctx, core::Loc loc) {
    core::Loc::Detail startDetail = loc.position(ctx).first;
    u4 lineStart = core::Loc::pos2Offset(loc.file().data(ctx), {startDetail.line, 1});

    // This isn't the entire line, it's just the line up until the end of the method def:
    //     private def foo; end
    //             ^^^^^^^
    // but that's enough for us to find the initial padding on this line.
    std::string_view lineView = loc.file().data(ctx).source().substr(lineStart, startDetail.column);

    u4 startPadding = lineView.find_first_not_of(" \t");
    u4 startOffset = lineStart + startPadding;
    return {startOffset, startPadding};
}

void maybeSuggestSig(core::Context ctx, core::ErrorBuilder &e, core::SymbolRef methodSymbol,
                     const shared_ptr<core::Type> &methodReturnType, core::TypeConstraint &constr) {
    if (constr.solve(ctx)) {
        auto guessedType = core::Types::widen(ctx, core::Types::instantiate(ctx, methodReturnType, constr));

        bool isFullyDefined = guessedType->isFullyDefined();
        bool isUntyped = guessedType->isUntyped();

        // We don't want to suggest a sig when the method has a block argument because
        // - Sometimes we synthesize a block arg, and name it `<blk>` which is not a valid identifier name.
        // - We're not attempting to infer types for them, and we don't want people to think that procs can't be typed.
        auto isBlockArg = [&](const core::SymbolRef &arg) { return arg.data(ctx).isBlockArgument(); };
        bool hasBlockArg = absl::c_any_of(methodSymbol.data(ctx).arguments(), isBlockArg);

        if (isFullyDefined && !isUntyped && !hasBlockArg) {
            auto loc = methodSymbol.data(ctx).loc();
            auto [startOffset, startPadding] = getStartOffset(ctx, loc);
            core::Loc replacementLoc(loc.file(), startOffset, startOffset);
            stringstream ss;
            bool first = true;

            ss << "sig {";
            if (!methodSymbol.data(ctx).arguments().empty()) {
                ss << "params(";
                for (auto &argSym : methodSymbol.data(ctx).arguments()) {
                    if (!first) {
                        ss << ", ";
                    }
                    first = false;
                    ss << argSym.data(ctx).name.show(ctx) << ": " << core::Types::untypedUntracked()->show(ctx);
                }
                ss << ").";
            }

            string returnStr;
            if (methodSymbol.data(ctx).name == core::Names::initialize() ||
                core::Types::isSubType(ctx, core::Types::void_(), guessedType)) {
                returnStr = "void";
            } else {
                returnStr = fmt::format("returns({})", guessedType->show(ctx));
            }
            ss << returnStr;
            ss << "}";

            string spaces(startPadding, ' ');

            e.addAutocorrect(core::AutocorrectSuggestion(replacementLoc, fmt::format("{}\n{}", ss.str(), spaces)));
        }
    }
}
} // namespace

unique_ptr<cfg::CFG> Inference::run(core::Context ctx, unique_ptr<cfg::CFG> cfg) {
    auto methodLoc = ctx.owner.data(ctx).loc();
    prodCounterInc("types.input.methods.typechecked");
    int typedSendCount = 0;
    int totalSendCount = 0;
    const int startErrorCount = ctx.state.totalErrors();
    unique_ptr<core::TypeConstraint> _constr;
    core::TypeConstraint *constr = &core::TypeConstraint::EmptyFrozenConstraint;
    if (cfg->symbol.data(ctx).isGenericMethod()) {
        _constr = make_unique<core::TypeConstraint>();
        constr = _constr.get();
        for (core::SymbolRef typeArgument : cfg->symbol.data(ctx).typeArguments()) {
            constr->rememberIsSubtype(ctx, typeArgument.data(ctx).resultType,
                                      make_shared<core::SelfTypeParam>(typeArgument));
        }
        if (!constr->solve(ctx)) {
            Error::raise("should never happen");
        }
    }

    shared_ptr<core::Type> methodReturnType = cfg->symbol.data(ctx).resultType;
    auto missingReturnType = methodReturnType == nullptr;
    auto shouldHaveReturnType = true;

    if (cfg->symbol.data(ctx).name.data(ctx).kind != core::NameKind::UTF8 ||
        cfg->symbol.data(ctx).name == core::Names::staticInit() || !cfg->symbol.data(ctx).loc().exists()) {
        shouldHaveReturnType = false;
    }
    if (missingReturnType) {
        if (shouldHaveReturnType) {
            ENFORCE(constr->isSolved() && constr->isEmpty());
            _constr = make_unique<core::TypeConstraint>();
            constr = _constr.get();
            auto returnTypeVar =
                core::Symbols::RubyTyper_ReturnTypeInference_guessed_type_type_parameter_holder_tparam();
            InlinedVector<core::SymbolRef, 4> domainTemp;
            domainTemp.emplace_back(returnTypeVar);
            methodReturnType = returnTypeVar.data(ctx).resultType;

            constr->defineDomain(ctx, domainTemp);
        } else {
            methodReturnType = core::Types::untyped(ctx, cfg->symbol);
        }
    } else {
        auto enclosingClass = ctx.owner.data(ctx).enclosingClass(ctx);
        methodReturnType =
            core::Types::instantiate(ctx,
                                     core::Types::resultTypeAsSeenFrom(ctx, ctx.owner, enclosingClass,
                                                                       enclosingClass.data(ctx).selfTypeArgs(ctx)),
                                     *constr);
        methodReturnType = core::Types::replaceSelfType(ctx, methodReturnType, enclosingClass.data(ctx).selfType(ctx));
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
        ENFORCE(!cfg->symbol.data(ctx).isAbstract());
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
                uninitialized.second.typeAndOrigins.origins.emplace_back(ctx.owner.data(ctx).loc());
            } else {
                uninitialized.second.typeAndOrigins.type->sanityCheck(ctx);
            }
        }

        for (auto &blockArg : bb->args) {
            current.getAndFillTypeAndOrigin(ctx, blockArg);
        }

        visited[bb->id] = true;
        if (current.isDead) {
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
        for (cfg::Binding &bind : bb->exprs) {
            if (!current.isDead || cfg::isa_instruction<cfg::DebugEnvironment>(bind.value.get())) {
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
            } else if (current.isDead && !bind.value->isSynthetic) {
                if (auto e = ctx.state.beginError(bind.loc, core::errors::Infer::DeadBranchInferencer)) {
                    e.setHeader("This code is unreachable");
                    e.addErrorLine(madeBlockDead, "This expression can never be computed");
                }
                break;
            }
        }
        if (!current.isDead) {
            current.getAndFillTypeAndOrigin(ctx, bb->bexit.cond);
            current.ensureGoodCondition(ctx, bb->bexit.cond.variable);
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

    if (missingReturnType && shouldHaveReturnType) {
        if (auto e = ctx.state.beginError(cfg->symbol.data(ctx).loc(), core::errors::Infer::UntypedMethod)) {
            e.setHeader("This function does not have a `sig`");
            maybeSuggestSig(ctx, e, cfg->symbol, methodReturnType, *constr);
        }
    }

    prodCounterAdd("types.input.sends.typed", typedSendCount);
    prodCounterAdd("types.input.sends.total", totalSendCount);

    return cfg;
}
} // namespace sorbet::infer
