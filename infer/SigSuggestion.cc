#include "infer/SigSuggestion.h"
#include "common/common.h"
#include "core/Loc.h"
#include "core/TypeConstraint.h"
#include "core/TypeErrorDiagnostics.h"
#include "core/lsp/QueryResponse.h"
#include <optional>

using namespace std;

namespace sorbet::infer {

namespace {

core::TypePtr extractArgType(core::Context ctx, cfg::Send &send, core::DispatchComponent &component,
                             optional<core::NameRef> keyword, int argId) {
    ENFORCE(component.method.exists());
    const auto &args = component.method.data(ctx)->arguments;
    if (argId >= args.size()) {
        return nullptr;
    }

    core::TypePtr to = nullptr;
    if (keyword.has_value()) {
        auto name = *keyword;
        for (auto &argInfo : args) {
            if (argInfo.flags.isKeyword && argInfo.name == name) {
                to = argInfo.type;
                break;
            }
        }
        if (to == nullptr) {
            return nullptr;
        }
    } else {
        auto &argInfo = args[argId];
        if (argInfo.flags.isKeyword) {
            // TODO(trevor) this is possibly due to the argument being a keyword args splat. Right now we ignore this
            // case, but it would be great to give a hash or shape suggestion instead.
            return nullptr;
        }

        to = argInfo.type;
    }

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
        ctx.file, bindLoc, snd->receiverLoc, snd->funLoc, snd->argLocs,
    };

    auto numPosArgs = snd->numPosArgs;
    auto numKwArgs = snd->args.size() - numPosArgs;
    bool hasKwSplat = numKwArgs % 2 == 1;

    // Since this is a "fake" dispatch and we are not going to display the errors anyway,
    // core::Loc::none() should be okay here.
    auto originForUninitialized = core::Loc::none();
    auto originForFullType = core::Loc::none();
    auto suppressErrors = true;
    core::DispatchArgs dispatchArgs{snd->fun,
                                    locs,
                                    numPosArgs,
                                    args,
                                    snd->recv.type,
                                    {snd->recv.type, {originForFullType}},
                                    snd->recv.type,
                                    snd->link,
                                    originForUninitialized,
                                    snd->isPrivateOk,
                                    suppressErrors};
    auto dispatchInfo = snd->recv.type.dispatchCall(ctx, dispatchArgs);

    // See if we can learn what types should they have
    bool inKwArgs = false;
    for (int i = 0, argIdx = 0; i < snd->args.size(); i++, argIdx++) {
        inKwArgs = inKwArgs || (i >= numPosArgs && !hasKwSplat);

        // extract the keyword argument name when the send contains inlined keyword arguments
        optional<core::NameRef> keyword;
        if (inKwArgs) {
            if (core::isa_type<core::NamedLiteralType>(snd->args[i].type)) {
                auto lit = core::cast_type_nonnull<core::NamedLiteralType>(snd->args[i].type);
                if (lit.literalKind == core::NamedLiteralType::LiteralTypeKind::Symbol) {
                    keyword = lit.asName();
                }
            }

            // increment over the keyword argument symbol
            i++;
        }

        auto &arg = snd->args[i];

        // See if we can learn about what functions are expected to exist on arguments
        auto fnd = blockLocals.find(arg.variable);
        if (fnd == blockLocals.end()) {
            continue;
        }

        core::TypePtr thisType;
        auto iter = &dispatchInfo;
        while (iter != nullptr) {
            if (iter->main.method.exists()) {
                auto argType = extractArgType(ctx, *snd, iter->main, keyword, argIdx);
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

UnorderedMap<core::NameRef, core::TypePtr> guessArgumentTypes(core::Context ctx, core::MethodRef methodSymbol,
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

            if (auto load = cfg::cast_instruction<cfg::LoadArg>(bind.value)) {
                newInsert.emplace_back(load->argument(ctx).name);
            } else if (auto ident = cfg::cast_instruction<cfg::Ident>(bind.value)) {
                auto fnd = blockLocals.find(ident->what);
                if (fnd != blockLocals.end()) {
                    newInsert.insert(newInsert.end(), fnd->second.begin(), fnd->second.end());
                }
            } else if (auto snd = cfg::cast_instruction<cfg::Send>(bind.value)) {
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

core::MethodRef closestOverridenMethod(core::Context ctx, core::ClassOrModuleRef enclosingClassSymbol,
                                       core::NameRef name) {
    auto enclosingClass = enclosingClassSymbol.data(ctx);
    ENFORCE(enclosingClass->flags.isLinearizationComputed, "Should have been linearized by resolver");

    for (const auto &mixin : enclosingClass->mixins()) {
        auto mixinMethod = mixin.data(ctx)->findMethod(ctx, name);
        if (mixinMethod.exists()) {
            return mixinMethod;
        }
    }

    auto superClass = enclosingClass->superClass();
    if (!superClass.exists()) {
        return core::Symbols::noMethod();
    }

    auto superMethod = superClass.data(ctx)->findMethod(ctx, name);
    if (superMethod.exists()) {
        return superMethod;
    } else {
        return closestOverridenMethod(ctx, superClass, name);
    }
}

bool childNeedsOverride(core::Context ctx, core::MethodRef childSymbol, core::MethodRef parentSymbol) {
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
        !parentSymbol.data(ctx)->flags.isRewriterSynthesized &&
        // It has a sig...
        parentSymbol.data(ctx)->resultType != nullptr &&
        //  that is either overridable...
        (parentSymbol.data(ctx)->flags.isOverridable ||
         // or override...
         parentSymbol.data(ctx)->flags.isOverride);
}

} // namespace

optional<core::AutocorrectSuggestion> SigSuggestion::maybeSuggestSig(core::Context ctx, unique_ptr<cfg::CFG> &cfg,
                                                                     const core::TypePtr &methodReturnType,
                                                                     core::TypeConstraint &constr) {
    core::MethodRef methodSymbol = cfg->symbol;

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
            arg.name.shortName(ctx).empty();
    };
    bool hasBadArg = absl::c_any_of(methodSymbol.data(ctx)->arguments, isBadArg);
    if (hasBadArg) {
        return nullopt;
    }

    auto guessedArgumentTypes = guessArgumentTypes(ctx, methodSymbol, cfg);

    auto enclosingClass = methodSymbol.enclosingClass(ctx);
    auto closestMethod = closestOverridenMethod(ctx, enclosingClass, methodSymbol.data(ctx)->name);

    fmt::memory_buffer ss;
    if (closestMethod.exists()) {
        auto closestReturnType = closestMethod.data(ctx)->resultType;
        if (closestReturnType && !closestReturnType.isUntyped()) {
            guessedReturnType = closestReturnType;
        }

        for (const auto &arg : closestMethod.data(ctx)->arguments) {
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

    fmt::format_to(std::back_inserter(ss), "sig");
    if (enclosingClass.data(ctx)->flags.isFinal) {
        fmt::format_to(std::back_inserter(ss), "(:final)");
    }
    fmt::format_to(std::back_inserter(ss), " {{ ");

    ENFORCE(!methodSymbol.data(ctx)->arguments.empty(), "There should always be at least one arg (the block arg).");
    bool onlyArgumentIsBlkArg = methodSymbol.data(ctx)->arguments.size() == 1 &&
                                methodSymbol.data(ctx)->arguments[0].isSyntheticBlockArgument();

    if (methodSymbol.data(ctx)->name != core::Names::initialize()) {
        // Only need override / implementation if the parent has a sig
        if (closestMethod.exists() && closestMethod.data(ctx)->resultType != nullptr) {
            if (closestMethod.data(ctx)->flags.isAbstract || childNeedsOverride(ctx, methodSymbol, closestMethod)) {
                fmt::format_to(std::back_inserter(ss), "override.");
            }
        }
    }

    if (!onlyArgumentIsBlkArg) {
        fmt::format_to(std::back_inserter(ss), "params(");

        bool first = true;
        for (auto &argSym : methodSymbol.data(ctx)->arguments) {
            // WARNING: This is doing raw string equality--don't cargo cult this!
            // You almost certainly want to compare NameRef's for equality instead.
            // We need to compare strings here because we're running with a frozen global state
            // (and thus can't take the string that we get from `argumentName` and enter it as a name).
            if (argSym.argumentName(ctx) == core::Names::blkArg().shortName(ctx)) {
                // Never write "<blk>: ..." in the params of a generated sig, because this doesn't parse.
                // (We add a block argument to every method if it doesn't mention one.)
                continue;
            }
            if (!first) {
                fmt::format_to(std::back_inserter(ss), ", ");
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
            auto options = core::ShowOptions().withShowForRBI();
            fmt::format_to(std::back_inserter(ss), "{}: {}", argSym.argumentName(ctx), chosenType.show(ctx, options));
        }
        fmt::format_to(std::back_inserter(ss), ").");
    }
    if (!guessedSomethingUseful && !ctx.state.suggestUnsafe.has_value()) {
        // We don't want to condition people to start inserting a bunch of useless signatures filled
        // with `T.untyped`, unless they've explicitly opted into the behavior with
        // `--suggest-unsafe` (which usually suggests that they're doing some sort of codemod and
        // they know what they're asking for).
        return nullopt;
    }

    bool suggestsVoid = methodSymbol.data(ctx)->name == core::Names::initialize() ||
                        (core::Types::isSubType(ctx, core::Types::void_(), guessedReturnType) &&
                         !guessedReturnType.isUntyped() && !guessedReturnType.isBottom());

    if (suggestsVoid) {
        fmt::format_to(std::back_inserter(ss), "void }}");
    } else {
        auto options = core::ShowOptions().withShowForRBI();
        fmt::format_to(std::back_inserter(ss), "returns({}) }}", guessedReturnType.show(ctx, options));
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

    auto topAttachedClass = enclosingClass.data(ctx)->topAttachedClass(ctx);
    if (auto edit = core::TypeErrorDiagnostics::editForDSLMethod(ctx, ctx.file, replacementLoc, topAttachedClass,
                                                                 core::Symbols::T_Sig(), "")) {
        if (edit->loc == edits.back().loc) {
            // Merge edits if we need to insert the `extend T::Sig` at the same point as the `sig`,
            edits.back().replacement = edit->replacement + edits.back().replacement;
        } else {
            edits.emplace_back(edit.value());
        }
    }

    return core::AutocorrectSuggestion{fmt::format("Add `{}`", sig), edits};
}

} // namespace sorbet::infer
