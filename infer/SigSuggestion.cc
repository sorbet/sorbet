#include "infer/SigSuggestion.h"
#include "common/common.h"
#include "core/Loc.h"
#include "core/TypeConstraint.h"
#include "core/lsp/QueryResponse.h"
#include <optional>

using namespace std;

namespace sorbet::infer {

namespace {

bool extendsTSig(core::Context ctx, core::SymbolRef enclosingClass) {
    ENFORCE(enclosingClass.exists());
    auto enclosingSingletonClass = enclosingClass.data(ctx)->lookupSingletonClass(ctx);
    ENFORCE(enclosingSingletonClass.exists());
    return enclosingSingletonClass.data(ctx)->derivesFrom(ctx, core::Symbols::T_Sig());
}

optional<core::AutocorrectSuggestion::Edit> maybeSuggestExtendTSig(core::Context ctx, core::SymbolRef methodSymbol) {
    auto method = methodSymbol.data(ctx);

    auto enclosingClass = method->enclosingClass(ctx).data(ctx)->topAttachedClass(ctx);
    if (extendsTSig(ctx, enclosingClass)) {
        // No need to suggest here, because it already has 'extend T::Sig'
        return nullopt;
    }

    auto inFileOfMethod = [&](const auto &loc) { return loc.file() == method->loc().file(); };
    auto classLocs = enclosingClass.data(ctx)->locs();
    auto classLoc = absl::c_find_if(classLocs, inFileOfMethod);

    if (classLoc == classLocs.end()) {
        // Couldn't a loc for the enclosing class in this file, give up.
        // An alternative heuristic here might be "found a file that we can write to"
        return nullopt;
    }

    auto [classStart, classEnd] = classLoc->position(ctx);

    core::Loc::Detail thisLineStart = {classStart.line, 1};
    auto thisLineLoc = core::Loc::fromDetails(ctx, classLoc->file(), thisLineStart, thisLineStart);
    ENFORCE(thisLineLoc.has_value());
    auto [_, thisLinePadding] = thisLineLoc.value().findStartOfLine(ctx);

    core::Loc::Detail nextLineStart = {classStart.line + 1, 1};
    auto nextLineLoc = core::Loc::fromDetails(ctx, classLoc->file(), nextLineStart, nextLineStart);
    if (!nextLineLoc.has_value()) {
        return nullopt;
    }
    auto [replacementLoc, nextLinePadding] = nextLineLoc.value().findStartOfLine(ctx);

    // Preserve the indentation of the line below us.
    string prefix(max(thisLinePadding + 2, nextLinePadding), ' ');
    return core::AutocorrectSuggestion::Edit{nextLineLoc.value(), fmt::format("{}extend T::Sig\n", prefix)};
}

core::TypePtr extractArgType(core::Context ctx, cfg::Send &send, core::DispatchComponent &component, int argId) {
    ENFORCE(component.method.exists() && component.method != core::Symbols::untyped());
    const auto &args = component.method.data(ctx)->arguments();
    if (argId >= args.size()) {
        return nullptr;
    }
    const auto &to = args[argId].type;
    if (!to || !to.isFullyDefined()) {
        return nullptr;
    }
    return to;
}

void extractSendArgumentKnowledge(core::Context ctx, core::LocOffsets bindLoc, cfg::Send *snd,
                                  const UnorderedMap<cfg::LocalRef, InlinedVector<core::NameRef, 1>> &blockLocals,
                                  UnorderedMap<core::NameRef, core::TypePtr> &blockArgRequirements) {
    InlinedVector<unique_ptr<core::TypeAndOrigins>, 2> typeAndOriginsOwner;
    InlinedVector<const core::TypeAndOrigins *, 2> args;

    args.reserve(snd->args.size());
    for (cfg::VariableUseSite &arg : snd->args) {
        auto &t = typeAndOriginsOwner.emplace_back(make_unique<core::TypeAndOrigins>());
        t->type = arg.type;
        t->origins.emplace_back(core::Loc::none());
        args.emplace_back(t.get());
    }

    core::CallLocs locs{
        ctx.file,
        bindLoc,
        snd->receiverLoc,
        snd->argLocs,
    };
    // Since this is a "fake" dispatch and we are not going to display the errors anyway,
    // core::Loc::none() should be okay here.
    auto originForUninitialized = core::Loc::none();
    core::DispatchArgs dispatchArgs{snd->fun,       locs,           snd->numPosArgs,
                                    args,           snd->recv.type, snd->recv.type,
                                    snd->recv.type, snd->link,      originForUninitialized};
    auto dispatchInfo = snd->recv.type.dispatchCall(ctx, dispatchArgs);

    int i = -1;

    // See if we can learn what types should they have
    for (auto &arg : snd->args) {
        i++;
        // See if we can learn about what functions are expected to exist on arguments
        auto fnd = blockLocals.find(arg.variable);
        if (fnd == blockLocals.end()) {
            continue;
        }
        core::TypePtr thisType;
        auto iter = &dispatchInfo;
        while (iter != nullptr) {
            if (iter->main.method.exists() && iter->main.method != core::Symbols::untyped()) {
                auto argType = extractArgType(ctx, *snd, iter->main, i);
                if (argType && !argType.isUntyped()) {
                    if (!thisType) {
                        thisType = argType;
                    } else {
                        // 'or' together every dispatch component for _this_ usage site
                        thisType = core::Types::lub(ctx, thisType, argType);
                    }
                }
            }

            iter = iter->secondary.get();
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

UnorderedMap<core::NameRef, core::TypePtr> guessArgumentTypes(core::Context ctx, core::SymbolRef methodSymbol,
                                                              unique_ptr<cfg::CFG> &cfg) {
    // What variables by the end of basic block could plausibly contain what arguments.
    vector<UnorderedMap<cfg::LocalRef, InlinedVector<core::NameRef, 1>>> localsStoringArguments;
    localsStoringArguments.resize(cfg->maxBasicBlockId);

    // indicates what type should an argument have for basic block to execute
    vector<UnorderedMap<core::NameRef, core::TypePtr>> argTypesForBBToPass;
    argTypesForBBToPass.resize(cfg->maxBasicBlockId);

    // This loop computes per-block requirements... Should be a method on its own
    for (auto it = cfg->forwardsTopoSort.rbegin(); it != cfg->forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        if (bb == cfg->deadBlock()) {
            continue;
        }
        UnorderedMap<cfg::LocalRef, InlinedVector<core::NameRef, 1>> &blockLocals = localsStoringArguments[bb->id];
        UnorderedMap<core::NameRef, core::TypePtr> &blockArgRequirements = argTypesForBBToPass[bb->id];

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
            InlinedVector<core::NameRef, 1> newInsert;

            if (auto load = cfg::cast_instruction<cfg::LoadArg>(bind.value.get())) {
                newInsert.emplace_back(load->argument(ctx).name);
            } else if (auto ident = cfg::cast_instruction<cfg::Ident>(bind.value.get())) {
                auto fnd = blockLocals.find(ident->what);
                if (fnd != blockLocals.end()) {
                    newInsert.insert(newInsert.end(), fnd->second.begin(), fnd->second.end());
                }
            } else if (auto snd = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
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
                blockLocals[bind.bind.variable] = std::move(newInsert);
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
            UnorderedMap<core::NameRef, core::TypePtr> entryRequirements;
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

core::SymbolRef closestOverridenMethod(core::Context ctx, core::SymbolRef enclosingClassSymbol, core::NameRef name) {
    auto enclosingClass = enclosingClassSymbol.data(ctx);
    ENFORCE(enclosingClass->isClassOrModuleLinearizationComputed(), "Should have been linearized by resolver");

    for (const auto &mixin : enclosingClass->mixins()) {
        auto mixinMethod = mixin.data(ctx)->findMember(ctx, name);
        if (mixinMethod.exists() && !mixinMethod.data(ctx)->isZSuperMethod()) {
            // Skipping ZSuper methods because they won't have a meaningful signature for the sake
            // of sig suggestion.
            return mixinMethod;
        }
    }

    auto superClass = enclosingClass->superClass();
    if (!superClass.exists()) {
        core::SymbolRef none;
        return none;
    }

    auto superMethod = superClass.data(ctx)->findMember(ctx, name);
    if (superMethod.exists() && !superMethod.data(ctx)->isZSuperMethod()) {
        return superMethod;
    } else {
        return closestOverridenMethod(ctx, superClass, name);
    }
}

bool childNeedsOverride(core::Context ctx, core::SymbolRef childSymbol, core::SymbolRef parentSymbol) {
    return
        // We're overriding a method...
        parentSymbol.exists() &&
        // in a file which we can edit...
        parentSymbol.data(ctx)->loc().file().exists() &&
        // defined outside an RBI (because it might be codegen'd)...
        !parentSymbol.data(ctx)->loc().file().data(ctx).isRBI() &&
        // that isn't the constructor...
        childSymbol.data(ctx)->name != core::Names::initialize() &&
        // and wasn't Rewriter synthesized (beause we can't change DSL'd sigs).
        !parentSymbol.data(ctx)->isRewriterSynthesized() &&
        // It has a sig...
        parentSymbol.data(ctx)->resultType != nullptr &&
        //  that is either overridable...
        (parentSymbol.data(ctx)->isOverridable() ||
         // or override...
         parentSymbol.data(ctx)->isOverride());
}

} // namespace

optional<core::AutocorrectSuggestion> SigSuggestion::maybeSuggestSig(core::Context ctx, unique_ptr<cfg::CFG> &cfg,
                                                                     const core::TypePtr &methodReturnType,
                                                                     core::TypeConstraint &constr) {
    core::SymbolRef methodSymbol = cfg->symbol;

    bool guessedSomethingUseful = false;

    auto lspQueryMatches = ctx.state.lspQuery.matchesSuggestSig(methodSymbol);
    if (lspQueryMatches) {
        // Even a sig with no useful types is still useful interactively (saves on typing)
        guessedSomethingUseful = true;
    }

    core::TypePtr guessedReturnType;
    if (!constr.isEmpty()) {
        if (!constr.solve(ctx)) {
            return nullopt;
        }

        guessedReturnType = core::Types::widen(ctx, core::Types::instantiate(ctx, methodReturnType, constr));

        if (!guessedReturnType.isFullyDefined()) {
            guessedReturnType = core::Types::untypedUntracked();
        }

        guessedSomethingUseful |= !guessedReturnType.isUntyped();
    } else {
        guessedReturnType = methodReturnType;
    }

    auto isBadArg = [&](const core::ArgInfo &arg) -> bool {
        return
            // runtime does not support rest args and key-rest args
            arg.flags.isRepeated ||

            // sometimes variable does not have a name e.g. `def initialize (*)`
            arg.name.data(ctx)->shortName(ctx).empty();
    };
    bool hasBadArg = absl::c_any_of(methodSymbol.data(ctx)->arguments(), isBadArg);
    if (hasBadArg) {
        return nullopt;
    }

    auto guessedArgumentTypes = guessArgumentTypes(ctx, methodSymbol, cfg);

    auto enclosingClass = methodSymbol.data(ctx)->enclosingClass(ctx);
    auto closestMethod = closestOverridenMethod(ctx, enclosingClass, methodSymbol.data(ctx)->name);

    fmt::memory_buffer ss;
    if (closestMethod.exists()) {
        auto closestReturnType = closestMethod.data(ctx)->resultType;
        if (closestReturnType && !closestReturnType.isUntyped()) {
            guessedReturnType = closestReturnType;
        }

        for (const auto &arg : closestMethod.data(ctx)->arguments()) {
            if (arg.type && !arg.type.isUntyped()) {
                guessedArgumentTypes[arg.name] = arg.type;
            }
        }
    }

    auto loc = methodSymbol.data(ctx)->loc();
    // Sometimes the methodSymbol we're looking at has been synthesized by a Rewriter pass, so no 'def' exists in the
    // source
    if (loc.file().data(ctx).source().substr(loc.beginPos(), 3) != "def") {
        return nullopt;
    }

    fmt::format_to(ss, "sig {{");

    ENFORCE(!methodSymbol.data(ctx)->arguments().empty(), "There should always be at least one arg (the block arg).");
    bool onlyArgumentIsBlkArg = methodSymbol.data(ctx)->arguments().size() == 1 &&
                                methodSymbol.data(ctx)->arguments()[0].isSyntheticBlockArgument();

    if (methodSymbol.data(ctx)->name != core::Names::initialize()) {
        // Only need override / implementation if the parent has a sig
        if (closestMethod.exists() && closestMethod.data(ctx)->resultType != nullptr) {
            if (closestMethod.data(ctx)->isAbstract() || childNeedsOverride(ctx, methodSymbol, closestMethod)) {
                fmt::format_to(ss, "override.");
            }
        }
    }

    if (!onlyArgumentIsBlkArg) {
        fmt::format_to(ss, "params(");

        bool first = true;
        for (auto &argSym : methodSymbol.data(ctx)->arguments()) {
            // WARNING: This is doing raw string equality--don't cargo cult this!
            // You almost certainly want to compare NameRef's for equality instead.
            // We need to compare strings here because we're running with a frozen global state
            // (and thus can't take the string that we get from `argumentName` and enter it as a name).
            if (argSym.argumentName(ctx) == core::Names::blkArg().data(ctx)->shortName(ctx)) {
                // Never write "<blk>: ..." in the params of a generated sig, because this doesn't parse.
                // (We add a block argument to every method if it doesn't mention one.)
                continue;
            }
            if (!first) {
                fmt::format_to(ss, ", ");
            }
            first = false;
            auto argType = guessedArgumentTypes[argSym.name];
            core::TypePtr chosenType;

            auto oldType = argSym.type;
            if (!oldType || oldType.isUntyped()) {
                if (!argType || argType.isBottom()) {
                    chosenType = core::Types::untypedUntracked();
                } else {
                    guessedSomethingUseful = true;
                    chosenType = core::Types::widen(ctx, argType);
                }
            } else {
                // TODO: maybe combine the old and new types in some way?
                chosenType = oldType;
            }
            fmt::format_to(ss, "{}: {}", argSym.argumentName(ctx), chosenType.show(ctx));
        }
        fmt::format_to(ss, ").");
    }
    if (!guessedSomethingUseful) {
        return nullopt;
    }

    bool suggestsVoid = methodSymbol.data(ctx)->name == core::Names::initialize() ||
                        (core::Types::isSubType(ctx, core::Types::void_(), guessedReturnType) &&
                         !guessedReturnType.isUntyped() && !guessedReturnType.isBottom());

    if (suggestsVoid) {
        fmt::format_to(ss, "void}}");
    } else {
        fmt::format_to(ss, "returns({})}}", guessedReturnType.show(ctx));
    }

    auto [replacementLoc, padding] = loc.findStartOfLine(ctx);
    string spaces(padding, ' ');
    bool hasExistingSig = methodSymbol.data(ctx)->resultType != nullptr;

    if (!loc.file().exists()) {
        return nullopt;
    }

    if (hasExistingSig) {
        return nullopt;
    }

    vector<core::AutocorrectSuggestion::Edit> edits;

    auto sig = to_string(ss);
    auto replacementContents = fmt::format("{}\n{}", sig, spaces);
    edits.emplace_back(core::AutocorrectSuggestion::Edit{replacementLoc, replacementContents});

    if (lspQueryMatches) {
        core::lsp::QueryResponse::pushQueryResponse(
            ctx, core::lsp::EditResponse(replacementLoc, std::move(replacementContents)));
    }

    if (auto edit = maybeSuggestExtendTSig(ctx, methodSymbol)) {
        edits.emplace_back(edit.value());
    }

    return core::AutocorrectSuggestion{fmt::format("Add `{}`", sig), edits};
}

} // namespace sorbet::infer
