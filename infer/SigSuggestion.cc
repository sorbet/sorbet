#include "infer/SigSuggestion.h"
#include "common/common.h"
#include "core/Loc.h"
#include "core/TypeConstraint.h"

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

std::unique_ptr<u4> startOfExistingSig(core::Context ctx, core::Loc loc) {
    auto file = loc.file();
    ENFORCE(file.exists());
    auto textBeforeTheMethod = loc.file().data(ctx).source().substr(0, loc.beginPos());
    auto lastSigCurly = textBeforeTheMethod.rfind("sig {");
    auto lastSigDo = textBeforeTheMethod.rfind("sig do");
    if (lastSigCurly != string_view::npos) {
        if (lastSigDo != string_view::npos) {
            return make_unique<u4>(max(lastSigCurly, lastSigDo));
        } else {
            return make_unique<u4>(lastSigCurly);
        }
    } else {
        if (lastSigDo != string_view::npos) {
            return make_unique<u4>(lastSigDo);
        } else {
            // failed to find sig
            return std::unique_ptr<u4>{};
        }
    }
}

std::unique_ptr<u4> startOfExistingReturn(core::Context ctx, core::Loc loc) {
    auto file = loc.file();
    if (!file.exists()) {
        return std::unique_ptr<u4>{};
    }

    auto textBeforeTheMethod = file.data(ctx).source().substr(0, loc.beginPos());
    auto lastReturns = textBeforeTheMethod.rfind("returns(");
    auto lastVoid = textBeforeTheMethod.rfind("void");
    if (lastReturns != string_view::npos) {
        if (lastVoid != string_view::npos) {
            return make_unique<u4>(max(lastReturns, lastVoid));
        } else {
            return make_unique<u4>(lastReturns);
        }
    } else {
        if (lastVoid != string_view::npos) {
            return make_unique<u4>(lastVoid);
        } else {
            // failed to find sig
            return std::unique_ptr<u4>{};
        }
    }
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

core::SymbolRef closestOverridenMethod(core::Context ctx, core::SymbolRef enclosingClassSymbol, core::NameRef name) {
    auto enclosingClass = enclosingClassSymbol.data(ctx);
    ENFORCE(enclosingClass->isClassLinearizationComputed(), "Should have been linearized by resolver");

    for (const auto &mixin : enclosingClass->mixins()) {
        auto mixinMethod = mixin.data(ctx)->findMember(ctx, name);
        if (mixinMethod.exists()) {
            ENFORCE(mixinMethod.data(ctx)->isMethod());
            return mixinMethod;
        }
    }

    auto superClass = enclosingClass->superClass;
    if (!superClass.exists()) {
        core::SymbolRef none;
        return none;
    }

    auto superMethod = superClass.data(ctx)->findMember(ctx, name);
    if (superMethod.exists()) {
        return superMethod;
    } else {
        return closestOverridenMethod(ctx, superClass, name);
    }
}

bool parentNeedsOverridable(core::Context ctx, core::SymbolRef childSymbol, core::SymbolRef parentSymbol) {
    return
        // We're overriding a method...
        parentSymbol.exists() &&
        // and it has a sig...
        parentSymbol.data(ctx)->resultType != nullptr &&
        // that is neither overridable...
        !parentSymbol.data(ctx)->isOverridable() &&
        // nor abstract...
        !parentSymbol.data(ctx)->isAbstract() &&
        // nor the constructor...
        childSymbol.data(ctx)->name != core::Names::initialize() &&
        // in a file which we can edit...
        parentSymbol.data(ctx)->loc().file().exists() &&
        // and isn't defined in an RBI (because it might be codegen'd)
        !parentSymbol.data(ctx)->loc().file().data(ctx).isRBI();
}

} // namespace

bool SigSuggestion::maybeSuggestSig(core::Context ctx, core::ErrorBuilder &e, unique_ptr<cfg::CFG> &cfg,
                                    const shared_ptr<core::Type> &methodReturnType, core::TypeConstraint &constr) {
    core::SymbolRef methodSymbol = cfg->symbol;

    bool guessedSomethingUseful = false;
    shared_ptr<core::Type> guessedReturnType;
    if (!constr.isEmpty()) {
        if (!constr.solve(ctx)) {
            return false;
        }

        guessedReturnType = core::Types::widen(ctx, core::Types::instantiate(ctx, methodReturnType, constr));

        if (!guessedReturnType->isFullyDefined()) {
            guessedReturnType = core::Types::untypedUntracked();
        }

        guessedSomethingUseful = !guessedReturnType->isUntyped();
    } else {
        guessedReturnType = methodReturnType;
    }

    auto isBadArg = [&](const core::SymbolRef &arg) -> bool {
        return
            // Sometimes we synthesize a block arg, and name it `<blk>` which is not a valid identifier name.
            arg.data(ctx)->isBlockArgument() ||

            // runtime does not support rest args and key-rest args
            arg.data(ctx)->isRepeated() ||

            // sometimes variable does not have a name e.g. `def initialize (*)`
            arg.data(ctx)->name.data(ctx)->shortName(ctx).empty();
    };
    bool hasBadArg = absl::c_any_of(methodSymbol.data(ctx)->arguments(), isBadArg);
    if (hasBadArg) {
        return false;
    }

    auto guessedArgumentTypes = guessArgumentTypes(ctx, methodSymbol, cfg);

    auto enclosingClass = methodSymbol.data(ctx)->enclosingClass(ctx);
    auto closestMethod = closestOverridenMethod(ctx, enclosingClass, methodSymbol.data(ctx)->name);

    fmt::memory_buffer ss;
    if (closestMethod.exists()) {
        auto closestReturnType = closestMethod.data(ctx)->resultType;
        if (closestReturnType && !closestReturnType->isUntyped()) {
            guessedReturnType = closestReturnType;
        }

        for (const auto &arg : closestMethod.data(ctx)->arguments()) {
            auto argType = arg.data(ctx)->resultType;
            if (argType && !argType->isUntyped()) {
                guessedArgumentTypes[arg] = arg.data(ctx)->resultType;
            }
        }
    }

    auto loc = methodSymbol.data(ctx)->loc();
    // Sometimes the methodSymbol we're looking at has been synthesized by a DSL pass, so no 'def' exists in the source
    if (loc.file().data(ctx).source().substr(loc.beginPos(), 3) != "def") {
        return false;
    }

    bool first = true;

    fmt::format_to(ss, "sig {{generated.");
    if (!methodSymbol.data(ctx)->arguments().empty()) {
        fmt::format_to(ss, "params(");
        for (auto &argSym : methodSymbol.data(ctx)->arguments()) {
            if (!first) {
                fmt::format_to(ss, ", ");
            }
            first = false;
            auto argType = guessedArgumentTypes[argSym];
            shared_ptr<core::Type> chosenType;

            auto oldType = argSym.data(ctx)->resultType;
            if (!oldType || oldType->isUntyped()) {
                if (!argType || argType->isBottom()) {
                    chosenType = core::Types::untypedUntracked();
                } else {
                    guessedSomethingUseful = true;
                    chosenType = argType;
                }
            } else {
                // TODO: maybe combine the old and new types in some way?
                chosenType = oldType;
            }
            if (!ctx.state.suggestGarbageType) {
                fmt::format_to(ss, "{}: {}", argSym.data(ctx)->name.show(ctx), chosenType->show(ctx));
            } else {
                fmt::format_to(ss, "{}: ::Sorbet::GarbageType", argSym.data(ctx)->name.show(ctx));
            }
        }
        fmt::format_to(ss, ").");
    }
    if (!guessedSomethingUseful) {
        return false;
    }

    if (methodSymbol.data(ctx)->name != core::Names::initialize()) {
        if (closestMethod.exists()) {
            if (closestMethod.data(ctx)->isAbstract()) {
                fmt::format_to(ss, "implementation.");
            } else {
                fmt::format_to(ss, "override.");
            }
        }
    }

    bool suggestsVoid = methodSymbol.data(ctx)->name == core::Names::initialize() ||
                        (core::Types::isSubType(ctx, core::Types::void_(), guessedReturnType) &&
                         !guessedReturnType->isUntyped() && !guessedReturnType->isBottom());

    if (ctx.state.suggestGarbageType) {
        fmt::format_to(ss, "returns(::Sorbet::GarbageType)}}");
    } else if (suggestsVoid) {
        fmt::format_to(ss, "void}}");
    } else {
        fmt::format_to(ss, "returns({})}}", guessedReturnType->show(ctx));
    }

    auto [replacementLoc, padding] = findStartOfLine(ctx, loc);
    string spaces(padding, ' ');
    bool hasExistingSig = methodSymbol.data(ctx)->resultType != nullptr;

    if (!loc.file().exists()) {
        return false;
    }

    if (hasExistingSig && !methodSymbol.data(ctx)->hasGeneratedSig()) {
        return false;
    }

    if (hasExistingSig) {
        if (auto existingStart = startOfExistingSig(ctx, loc)) {
            replacementLoc = core::Loc(loc.file(), *existingStart, replacementLoc.endPos());
        } else {
            // Had existing sig, but couldn't find where it started, so give up suggesting a sig.
            return false;
        }
    }

    e.addAutocorrect(core::AutocorrectSuggestion(replacementLoc, fmt::format("{}\n{}", to_string(ss), spaces)));

    if (parentNeedsOverridable(ctx, methodSymbol, closestMethod)) {
        if (auto maybeOffset = startOfExistingReturn(ctx, closestMethod.data(ctx)->loc())) {
            auto offset = *maybeOffset;
            core::Loc overridableReturnLoc(closestMethod.data(ctx)->loc().file(), offset, offset);
            if (closestMethod.data(ctx)->hasGeneratedSig()) {
                e.addAutocorrect(core::AutocorrectSuggestion(overridableReturnLoc, "overridable."));
            } else {
                e.addAutocorrect(core::AutocorrectSuggestion(overridableReturnLoc, "generated.overridable."));
            }
        }
    }

    if (auto suggestion = maybeSuggestExtendTHelpers(ctx, methodSymbol)) {
        e.addAutocorrect(move(*suggestion));
    }
    return true;
}

} // namespace sorbet::infer
