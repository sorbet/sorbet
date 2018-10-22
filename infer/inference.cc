#include "absl/algorithm/container.h"
#include "core/Loc.h"
#include "core/TypeConstraint.h"
#include "core/errors/infer.h"
#include "infer/environment.h"
#include "infer/infer.h"

#include "absl/algorithm/container.h"

using namespace std;
namespace sorbet::infer {

namespace {
struct LocAndColumn {
    core::Loc loc;
    u4 padding;
};

//
// For a given Loc, returns
//
// - the Loc corresponding to the first non-whitespace character on this line, and
// - how many characters of the start of this line are whitespace.
//
LocAndColumn findStartOfLine(core::Context ctx, core::Loc loc) {
    core::Loc::Detail startDetail = loc.position(ctx).first;
    u4 lineStart = core::Loc::pos2Offset(loc.file().data(ctx), {startDetail.line, 1});
    std::string_view lineView = loc.file().data(ctx).source().substr(lineStart);

    u4 padding = lineView.find_first_not_of(" \t");
    u4 startOffset = lineStart + padding;
    return {core::Loc(loc.file(), startOffset, startOffset), padding};
}

// Walks the chain of attached classes to find the one at the end of the chain.
core::SymbolRef topAttachedClass(core::Context ctx, core::SymbolRef classSymbol) {
    while (true) {
        auto attachedClass = classSymbol.data(ctx)->attachedClass(ctx);
        if (!attachedClass.exists()) {
            break;
        }
        classSymbol = attachedClass;
    }
    return classSymbol;
}

bool extendsTHelpers(core::Context ctx, core::SymbolRef enclosingClass) {
    ENFORCE(enclosingClass.exists());
    auto enclosingSingletonClass = enclosingClass.data(ctx)->lookupSingletonClass(ctx);
    ENFORCE(enclosingSingletonClass.exists());
    return enclosingSingletonClass.data(ctx)->derivesFrom(ctx, core::Symbols::T_Helpers());
}

unique_ptr<core::AutocorrectSuggestion> maybeSuggestExtendTHelpers(core::Context ctx, core::SymbolRef methodSymbol) {
    auto method = methodSymbol.data(ctx);

    auto enclosingClass = topAttachedClass(ctx, method->enclosingClass(ctx));
    if (extendsTHelpers(ctx, enclosingClass)) {
        // No need to suggest here, because it already has 'extend T::Helpers'
        return unique_ptr<core::AutocorrectSuggestion>{};
    }

    auto inFileOfMethod = [&](const auto &loc) { return loc.file() == method->loc().file(); };
    auto classLocs = enclosingClass.data(ctx)->locs();
    auto classLoc = absl::c_find_if(classLocs, inFileOfMethod);

    if (classLoc == classLocs.end()) {
        // Couldn't a loc for the enclosing class in this file, give up.
        // TODO(jez) We might be able to expand this heuristic to be "found a file that we can write to"
        return unique_ptr<core::AutocorrectSuggestion>{};
    }

    auto [classStart, classEnd] = classLoc->position(ctx);
    ENFORCE(classStart.line + 1 <= classLoc->file().data(ctx).line_breaks().size());
    core::Loc::Detail nextLineStart = {classStart.line + 1, 1};
    core::Loc nextLineLoc = core::Loc::fromDetails(ctx, classLoc->file(), nextLineStart, nextLineStart);
    auto [replacementLoc, nextLinePadding] = findStartOfLine(ctx, nextLineLoc);

    // Preserve the indentation of the line below us.
    string prefix(nextLinePadding, ' ');
    return make_unique<core::AutocorrectSuggestion>(nextLineLoc, fmt::format("{}extend T::Helpers\n", prefix));
}

shared_ptr<core::Type> extractArgType(core::Context ctx, cfg::Send &send, core::DispatchComponent &component,
                                      int argId) {
    // The high level idea is the following: we will use a covariant type parameter to extract the type from dispatch
    // logic
    auto constr = make_shared<core::TypeConstraint>();
    auto linkCopy =
        send.link ? send.link->duplicate() : make_shared<core::SendAndBlockLink>(core::Symbols::noSymbol(), send.fun);

    auto probeTypeSym =
        core::Symbols::RubyTyper_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_covariant();
    InlinedVector<core::SymbolRef, 4> domainTemp;
    InlinedVector<pair<core::SymbolRef, shared_ptr<core::Type>>, 4> solutions;
    domainTemp.emplace_back(probeTypeSym);
    if (send.link && send.link->constr) {
        for (auto domainSym : send.link->constr->getDomain()) {
            domainTemp.emplace_back(domainSym);
            solutions.emplace_back(make_pair(domainSym, send.link->constr->getInstantiation(domainSym)));
        }
    }

    auto probe = probeTypeSym.data(ctx)->resultType;
    constr->defineDomain(ctx, domainTemp);
    for (auto &pair : solutions) {
        auto tparam = pair.first.data(ctx)->resultType;
        core::Types::isSubTypeUnderConstraint(ctx, *constr, tparam, pair.second);
        core::Types::isSubTypeUnderConstraint(ctx, *constr, pair.second, tparam);
    }

    linkCopy->constr = constr;

    core::CallLocs locs{
        send.receiverLoc,
        send.receiverLoc,
        send.argLocs,
    };
    InlinedVector<unique_ptr<core::TypeAndOrigins>, 2> typeAndOriginsOwner;
    InlinedVector<const core::TypeAndOrigins *, 2> args;

    args.reserve(send.args.size());
    int i = -1;
    for (cfg::VariableUseSite &arg : send.args) {
        i++;
        shared_ptr<core::Type> type;
        if (i != argId) {
            type = arg.type;
        } else {
            type = probe;
        }
        auto &t = typeAndOriginsOwner.emplace_back(make_unique<core::TypeAndOrigins>());
        t->type = type;
        t->origins.emplace_back(core::Loc::none());
        args.emplace_back(t.get());
    }
    core::DispatchArgs dispatchArgs{send.fun, locs, args, send.recv.type, send.recv.type, linkCopy};

    send.recv.type->dispatchCall(ctx, dispatchArgs);
    if (!constr->isSolved()) {
        constr->solve(ctx);
    }
    if (!constr->isSolved()) {
        return nullptr;
    }
    return constr->getInstantiation(probeTypeSym);
}

void extractSendArgumentKnowledge(
    core::Context ctx, core::Loc bindLoc, cfg::Send *snd,
    const UnorderedMap<core::LocalVariable, InlinedVector<core::SymbolRef, 1>> &blockLocals,
    UnorderedMap<core::SymbolRef, shared_ptr<core::Type>> &blockArgRequirements) {
    InlinedVector<unique_ptr<core::TypeAndOrigins>, 2> typeAndOriginsOwner;
    InlinedVector<const core::TypeAndOrigins *, 2> args;

    args.reserve(snd->args.size());
    for (cfg::VariableUseSite &arg : snd->args) {
        auto &t = typeAndOriginsOwner.emplace_back(make_unique<core::TypeAndOrigins>());
        t->type = arg.type;
        t->origins.emplace_back(core::Loc::none());
    }

    core::CallLocs locs{
        bindLoc,
        snd->receiverLoc,
        snd->argLocs,
    };
    core::DispatchArgs dispatchArgs{snd->fun, locs, args, snd->recv.type, snd->recv.type, snd->link};
    auto dispatchInfo = snd->recv.type->dispatchCall(ctx, dispatchArgs);

    int i = -1;

    // See if we can learn what types should they have
    for (auto &arg : snd->args) {
        i++;
        // See if we can learn about what functions are expected to exist on arguments
        auto fnd = blockLocals.find(arg.variable);
        if (fnd == blockLocals.end()) {
            continue;
        }
        shared_ptr<core::Type> thisType;
        for (auto &component : dispatchInfo.components) {
            auto argType = extractArgType(ctx, *snd, component, i);
            if (argType && !argType->isUntyped()) {
                if (!thisType) {
                    thisType = argType;
                } else {
                    // 'or' together every dispatch component for _this_ usage site
                    thisType = core::Types::lub(ctx, thisType, argType);
                }
            }
        }
        if (!thisType) {
            continue;
        }
        for (auto argSym : fnd->second) {
            auto &r = blockArgRequirements[argSym];
            if (!r) {
                r = thisType;
            } else {
                // 'and' this usage site against all the other usage sites
                r = core::Types::glb(ctx, r, thisType);
            }
        }
    }
}

UnorderedMap<core::SymbolRef, shared_ptr<core::Type>>
guessArgumentTypes(core::Context ctx, core::SymbolRef methodSymbol, unique_ptr<cfg::CFG> &cfg) {
    // What variables by the end of basic block could plausibly contain what arguments.
    vector<UnorderedMap<core::LocalVariable, InlinedVector<core::SymbolRef, 1>>> localsStoringArguments;
    localsStoringArguments.resize(cfg->maxBasicBlockId);

    // what methods have been called on arguments, per basic block
    vector<UnorderedMap<core::SymbolRef, InlinedVector<core::NameRef, 1>>> methodsCalledOnArguments;
    methodsCalledOnArguments.resize(cfg->maxBasicBlockId);

    // indicates what type should an argument have for basic block to execute
    vector<UnorderedMap<core::SymbolRef, shared_ptr<core::Type>>> argTypesForBBToPass;
    argTypesForBBToPass.resize(cfg->maxBasicBlockId);

    // This loop computes per-block requirements... Should be a method on its own
    for (auto it = cfg->forwardsTopoSort.rbegin(); it != cfg->forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        if (bb == cfg->deadBlock()) {
            continue;
        }
        UnorderedMap<core::LocalVariable, InlinedVector<core::SymbolRef, 1>> &blockLocals =
            localsStoringArguments[bb->id];
        UnorderedMap<core::SymbolRef, InlinedVector<core::NameRef, 1>> &blockMethodsCalledOnArguments =
            methodsCalledOnArguments[bb->id];
        UnorderedMap<core::SymbolRef, shared_ptr<core::Type>> &blockArgRequirements = argTypesForBBToPass[bb->id];

        for (auto bbparent : bb->backEdges) {
            for (auto kv : localsStoringArguments[bbparent->id]) {
                for (auto argSym : kv.second) {
                    if (!absl::c_linear_search(blockLocals[kv.first], argSym)) {
                        blockLocals[kv.first].push_back(argSym);
                    }
                }
            }
        }

        int i = 0;

        for (cfg::Binding &bind : bb->exprs) {
            i++;
            if (bb->firstDeadInstructionIdx >= 0 && i >= bb->firstDeadInstructionIdx) {
                break;
            }
            InlinedVector<core::SymbolRef, 1> newInsert;

            if (auto load = cfg::cast_instruction<cfg::LoadArg>(bind.value.get())) {
                newInsert.emplace_back(load->arg);
            } else if (auto ident = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
                auto fnd = blockLocals.find(ident->what);
                if (fnd != blockLocals.end()) {
                    newInsert.insert(newInsert.end(), fnd->second.begin(), fnd->second.end());
                }
            } else if (auto snd = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
                // See if we can learn about what functions are expected to exist on arguments
                auto fnd = blockLocals.find(snd->recv.variable);
                if (fnd != blockLocals.end()) {
                    for (auto &arg : fnd->second) {
                        if (!absl::c_linear_search(blockMethodsCalledOnArguments[arg], snd->fun)) {
                            blockMethodsCalledOnArguments[arg].push_back(snd->fun);
                        }
                    }
                }

                // see if we have at least a single call argument that is a method argument
                bool shouldFindArgumentTypes = false;
                for (auto &arg : snd->args) {
                    auto fnd = blockLocals.find(arg.variable);
                    if (fnd != blockLocals.end() && !fnd->second.empty()) {
                        shouldFindArgumentTypes = true;
                        break;
                    }
                }

                if (shouldFindArgumentTypes) {
                    extractSendArgumentKnowledge(ctx, bind.loc, snd, blockLocals, blockArgRequirements);
                }
            }

            // publish changes

            if (!newInsert.empty()) {
                blockLocals[bind.bind.variable] = move(newInsert);
            } else {
                blockLocals.erase(bind.bind.variable);
            }
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto it = cfg->forwardsTopoSort.rbegin(); it != cfg->forwardsTopoSort.rend(); ++it) {
            cfg::BasicBlock *bb = *it;
            UnorderedMap<core::SymbolRef, shared_ptr<core::Type>> entryRequirements;
            for (auto bbparent : bb->backEdges) {
                if (bbparent->firstDeadInstructionIdx >= 0 && bb != cfg->deadBlock()) {
                    continue;
                }
                for (auto &kv : argTypesForBBToPass[bbparent->id]) {
                    auto &cur = entryRequirements[kv.first];
                    if (!cur) {
                        cur = kv.second;
                        continue;
                    }
                    cur = core::Types::lub(ctx, cur, kv.second);
                }
            }
            auto &thisConstraints = argTypesForBBToPass[bb->id];
            for (auto &kv : entryRequirements) {
                auto &target = thisConstraints[kv.first];
                if (!target) {
                    target = kv.second;
                }
                auto newRequirement = core::Types::glb(ctx, target, kv.second);
                if (newRequirement != target) {
                    changed = true;
                    target = newRequirement;
                }
            }
        }
    }

    return argTypesForBBToPass[cfg->deadBlock()->id];
}

void maybeSuggestSig(core::Context ctx, core::ErrorBuilder &e, core::SymbolRef methodSymbol,
                     const shared_ptr<core::Type> &methodReturnType, core::TypeConstraint &constr,
                     unique_ptr<cfg::CFG> &cfg) {
    if (!constr.solve(ctx)) {
        return;
    }

    auto guessedReturnType = core::Types::widen(ctx, core::Types::instantiate(ctx, methodReturnType, constr));

    if (!guessedReturnType->isFullyDefined()) {
        return;
    }
    if (guessedReturnType->isUntyped()) {
        return;
    }

    auto isBadArg = [&](const core::SymbolRef &arg) -> bool {
        return arg.data(ctx)->isBlockArgument() || // Sometimes we synthesize a block arg,
                                                   // and name it `<blk>` which is not a
                                                   // valid identifier name.

               arg.data(ctx)->isRepeated() || // runtime does not support rest args
                                              // and key-rest args

               arg.data(ctx)->name.data(ctx)->shortName(ctx).empty(); // sometimes variable does not have
                                                                      // a name e.g. `def initialize (*)`
    };
    bool hasBadArg = absl::c_any_of(methodSymbol.data(ctx)->arguments(), isBadArg);
    if (hasBadArg) {
        return;
    }

    auto loc = methodSymbol.data(ctx)->loc();
    // Sometimes the methodSymbol we're looking at has been synthesized by a DSL pass, so no 'def' exists in the source
    if (loc.file().data(ctx).source().substr(loc.beginPos(), 3) != "def") {
        return;
    }

    fmt::memory_buffer ss;
    bool first = true;

    auto argumentTypes = guessArgumentTypes(ctx, methodSymbol, cfg);

    fmt::format_to(ss, "sig {{");
    if (!methodSymbol.data(ctx)->arguments().empty()) {
        fmt::format_to(ss, "params(");
        for (auto &argSym : methodSymbol.data(ctx)->arguments()) {
            if (!first) {
                fmt::format_to(ss, ", ");
            }
            first = false;
            auto argType = argumentTypes[argSym];
            if (!argType || argType->isBottom()) {
                argType = core::Types::untypedUntracked();
            }
            fmt::format_to(ss, "{}: {}", argSym.data(ctx)->name.show(ctx), argType->show(ctx));
        }
        fmt::format_to(ss, ").");
    }

    if (methodSymbol.data(ctx)->name == core::Names::initialize() ||
        core::Types::isSubType(ctx, core::Types::void_(), guessedReturnType)) {
        fmt::format_to(ss, "void}}");
    } else {
        fmt::format_to(ss, "returns({})}}", guessedReturnType->show(ctx));
    }

    auto [replacementLoc, padding] = findStartOfLine(ctx, loc);
    string spaces(padding, ' ');

    e.addAutocorrect(core::AutocorrectSuggestion(replacementLoc, fmt::format("{}\n{}", to_string(ss), spaces)));

    if (auto suggestion = maybeSuggestExtendTHelpers(ctx, methodSymbol)) {
        e.addAutocorrect(move(*suggestion));
    }
}
} // namespace

unique_ptr<cfg::CFG> Inference::run(core::Context ctx, unique_ptr<cfg::CFG> cfg) {
    auto methodLoc = ctx.owner.data(ctx)->loc();
    prodCounterInc("types.input.methods.typechecked");
    int typedSendCount = 0;
    int totalSendCount = 0;
    const int startErrorCount = ctx.state.totalErrors();
    unique_ptr<core::TypeConstraint> _constr;
    core::TypeConstraint *constr = &core::TypeConstraint::EmptyFrozenConstraint;
    if (cfg->symbol.data(ctx)->isGenericMethod()) {
        _constr = make_unique<core::TypeConstraint>();
        constr = _constr.get();
        for (core::SymbolRef typeArgument : cfg->symbol.data(ctx)->typeArguments()) {
            constr->rememberIsSubtype(ctx, typeArgument.data(ctx)->resultType,
                                      make_shared<core::SelfTypeParam>(typeArgument));
        }
        if (!constr->solve(ctx)) {
            Error::raise("should never happen");
        }
    }

    shared_ptr<core::Type> methodReturnType = cfg->symbol.data(ctx)->resultType;
    auto missingReturnType = methodReturnType == nullptr;
    auto shouldHaveReturnType = true;

    if (cfg->symbol.data(ctx)->name.data(ctx)->kind != core::NameKind::UTF8 ||
        cfg->symbol.data(ctx)->name == core::Names::staticInit() || !cfg->symbol.data(ctx)->loc().exists()) {
        shouldHaveReturnType = false;
    }
    if (missingReturnType) {
        if (shouldHaveReturnType) {
            ENFORCE(constr->isSolved() && constr->isEmpty());
            _constr = make_unique<core::TypeConstraint>();
            constr = _constr.get();
            auto returnTypeVar =
                core::Symbols::RubyTyper_ReturnTypeInference_guessed_type_type_parameter_holder_tparam_contravariant();
            InlinedVector<core::SymbolRef, 4> domainTemp;
            domainTemp.emplace_back(returnTypeVar);
            methodReturnType = returnTypeVar.data(ctx)->resultType;

            constr->defineDomain(ctx, domainTemp);
        } else {
            methodReturnType = core::Types::untyped(ctx, cfg->symbol);
        }
    } else {
        auto enclosingClass = ctx.owner.data(ctx)->enclosingClass(ctx);
        methodReturnType =
            core::Types::instantiate(ctx,
                                     core::Types::resultTypeAsSeenFrom(ctx, ctx.owner, enclosingClass,
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
                uninitialized.second.typeAndOrigins.origins.emplace_back(ctx.owner.data(ctx)->loc());
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

    if (missingReturnType && shouldHaveReturnType) {
        if (auto e = ctx.state.beginError(cfg->symbol.data(ctx)->loc(), core::errors::Infer::UntypedMethod)) {
            e.setHeader("This function does not have a `sig`");
            maybeSuggestSig(ctx, e, cfg->symbol, methodReturnType, *constr, cfg);
        }
    }

    prodCounterAdd("types.input.sends.typed", typedSendCount);
    prodCounterAdd("types.input.sends.total", totalSendCount);

    return cfg;
}
} // namespace sorbet::infer
