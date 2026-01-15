#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/sort/sort.h"
#include "common/timers/Timer.h"
#include "core/core.h"
#include "core/errors/infer.h"
#include "core/errors/resolver.h"
#include "core/source_generator/source_generator.h"

#include "absl/algorithm/container.h"

#include "core/insert_method/insert_method.h"
#include "core/sig_finder/sig_finder.h"
#include "definition_validator/variance.h"

using namespace std;

namespace sorbet::definition_validator {

namespace {
struct Signature {
    struct {
        absl::InlinedVector<reference_wrapper<const core::ParamInfo>, 4> required;
        absl::InlinedVector<reference_wrapper<const core::ParamInfo>, 4> optional;
        std::optional<reference_wrapper<const core::ParamInfo>> rest;
    } pos, kw;
    bool syntheticBlk;
} left, right;

Signature decomposeSignature(const core::GlobalState &gs, core::MethodRef method) {
    Signature sig;
    for (auto &param : method.data(gs)->parameters) {
        if (param.flags.isBlock) {
            sig.syntheticBlk = param.isSyntheticBlockParameter();
            continue;
        }

        auto &dst = param.flags.isKeyword ? sig.kw : sig.pos;
        if (param.flags.isRepeated) {
            dst.rest = optional<reference_wrapper<const core::ParamInfo>>{param};
        } else if (param.flags.isDefault) {
            dst.optional.push_back(param);
        } else {
            dst.required.push_back(param);
        }
    }
    return sig;
}

// This is really just a useful helper function for this module: do not use it elsewhere.
//
// It makes certain assumptions that it is running for the sake of computing overrides that are not
// going to be true in other situations.
bool checkSubtype(const core::Context ctx, core::TypeConstraint &constr, const vector<core::TypePtr> &subSelfTypeArgs,
                  const core::TypePtr &sub, core::MethodRef subMethod, const core::TypePtr &super,
                  core::MethodRef superMethod, core::Polarity polarity,
                  core::ErrorSection::Collector &errorDetailsCollector) {
    if (sub == nullptr || super == nullptr) {
        // nullptr is just "unannotated" which is T.untyped
        return true;
    }

    auto subOwner = subMethod.data(ctx)->owner;
    auto superOwner = superMethod.data(ctx)->owner;

    // Need this to be true for the sake of resultTypeAsSeenFrom.
    ENFORCE(subOwner.data(ctx)->derivesFrom(ctx, superOwner));

    // We're only using the TypeConstraint as a way to have an easy way to replace a `TypeVar` with
    // a "skolem" type variable (type variable representing an unknown but specific type). In
    // Sorbet, those skolems are SelfTypeParam types that wrap a TypeParameterRef.
    //
    // Types::approximateTypeVars does this "replace all the TypeVar with SelfTypeParam" naturally
    // and in a predictable way (i.e., respecting polarity), so it's convenient do to this with
    // approximateTypeVars rather than build a dedicated function for this.
    //
    // However, it's neither required nor desired to use that constraint to type check the two types
    // themselves. If we somehow managed to construct the constr incorrectly, there might still be
    // un-skolemized TypeVar's in either type, which would then record new constraints during the
    // call to isSubType. To guarantee that never happens, we typecheck the type under the
    // EmptyFrozenConstraint, instead of this constraint that we should only be using for skolemization.
    //
    // Another approach might be to create the constr right here, instead of threading it around
    // everywhere. We've taken the aprpopach of only constructing the constraint once as an optimization.

    auto subType = core::Types::resultTypeAsSeenFrom(ctx, sub, subOwner, subOwner, subSelfTypeArgs);
    subType = core::Types::approximateTypeVars(ctx, subType, constr);
    auto superType = core::Types::resultTypeAsSeenFrom(ctx, super, superOwner, subOwner, subSelfTypeArgs);
    superType = core::Types::approximateTypeVars(ctx, superType, constr);

    switch (polarity) {
        case core::Polarity::Negative:
            return core::Types::isSubType(ctx, superType, subType, errorDetailsCollector);
        case core::Polarity::Positive:
            return core::Types::isSubType(ctx, subType, superType, errorDetailsCollector);
        case core::Polarity::Neutral:
            Exception::raise("{}: unexpected neutral polarity, did you mean to pass Positive?",
                             ctx.file.data(ctx).path());
    }
}

string superMethodKind(const core::Context ctx, core::MethodRef method) {
    auto methodData = method.data(ctx);
    ENFORCE(methodData->flags.isAbstract || methodData->flags.isOverridable || methodData->hasSig());
    if (methodData->flags.isAbstract) {
        return "abstract";
    } else if (methodData->flags.isOverridable) {
        return "overridable";
    } else {
        return "overridden";
    }
}

string implementationOf(const core::Context ctx, core::MethodRef method) {
    auto methodData = method.data(ctx);
    ENFORCE(methodData->flags.isAbstract || methodData->flags.isOverridable || methodData->hasSig());
    if (methodData->flags.isAbstract) {
        return "Implementation of abstract";
    } else if (methodData->flags.isOverridable) {
        return "Implementation of overridable";
    } else {
        return "Override of";
    }
}

enum class SplatKind { ARG, KWARG };

pair<std::string, std::string> formatSplat(const core::ParamInfo &arg, SplatKind kd, const core::GlobalState &gs) {
    auto rendered = arg.show(gs);

    std::string left;

    switch (kd) {
        case SplatKind::ARG:
            left = "*";
            break;
        case SplatKind::KWARG:
            left = "**";
            break;
    }

    // We have no choice but to perform string comparison here because argument name normalization
    // drops the information that a given splat arg may be anonymous.
    //
    // XXX cwong: This feels a little icky, but the alternative is to change name normalization to
    // preserve the fact that a given arg was initially a `core::Names::star` or
    // `core::Names::starStar`, which sounds complicated an error-prone. Since we have to render
    // the argument name anyway (so we can display the error message), we may as well do the stupid
    // check.
    return rendered == left ? pair("", rendered) : pair(left, rendered);
}

optional<core::AutocorrectSuggestion>
constructAllowIncompatibleAutocorrect(const core::Context ctx, const ast::ExpressionPtr &tree,
                                      const ast::MethodDef &methodDef, const std::string_view what, bool &didReport) {
    // With this design, we will report the autocorrect on the *first* reported error. There is a
    // case to be made that, from a UX perspective, the message should be attached to the *last*
    // error (the one that is most likely to be on the user's screen at the end). Due to how
    // sorbet propagates type information, however, there's also a good chance that later errors
    // are spurious, so attaching to the first (likely the most relevant) is fine too.
    //
    // In this particular case, it is entirely inconsequential, since `--sugest-unsafe` is already
    // meant to be the "shut up" cudgel anyway.
    if (!ctx.state.suggestUnsafe || didReport) {
        return nullopt;
    }

    auto methodLoc = ctx.locAt(methodDef.declLoc);

    // XXX cwong: This causes a duplicate error to be reported if the signature is syntactically
    // malformed (e.g. `allow_incompatible: :bad`). There are a few ways to resolve this (e.g.
    // have `findSignature` return an error vector and make it the caller's responsibility to
    // display them).
    auto parsedSig = sig_finder::SigFinder::findSignature(ctx, tree, methodLoc.copyWithZeroLength());
    if (!parsedSig.has_value()) {
        return nullopt;
    }

    auto *block = parsedSig->origSend.block();
    if (!block) {
        return nullopt;
    }

    didReport = true;

    auto blockBody = ast::cast_tree<ast::Send>(block->body);
    ENFORCE(blockBody != nullptr);

    auto replaceLoc = ctx.locAt(parsedSig->sig.seen.override_);

    if (!replaceLoc.exists()) {
        return nullopt;
    }

    vector<core::AutocorrectSuggestion::Edit> edits;
    edits.emplace_back(
        core::AutocorrectSuggestion::Edit{replaceLoc, fmt::format("override(allow_incompatible: {})", what)});
    return core::AutocorrectSuggestion{
        fmt::format("Add `{}` to `{}` in `{}` sig", fmt::format("allow_incompatible: {}", what), "override",
                    methodDef.symbol.data(ctx)->name.show(ctx)),
        std::move(edits),
    };
}

optional<core::AutocorrectSuggestion> constructAllowIncompatibleAutocorrect(const core::Context ctx,
                                                                            const ast::ExpressionPtr &tree,
                                                                            const ast::MethodDef &methodDef,
                                                                            const std::string_view what) {
    bool _;
    return constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, what, _);
}

// This walks two positional argument lists to ensure that they're compatibly typed (i.e. that every argument in the
// implementing method is either the same or a supertype of the abstract or overridable definition)
void matchPositional(const core::Context ctx, core::TypeConstraint &constr, const vector<core::TypePtr> &selfTypeArgs,
                     const ast::ExpressionPtr &tree,
                     absl::InlinedVector<reference_wrapper<const core::ParamInfo>, 4> &superArgs,
                     core::MethodRef superMethod,
                     absl::InlinedVector<reference_wrapper<const core::ParamInfo>, 4> &methodArgs,
                     const ast::MethodDef &methodDef, bool &reportedAutocorrect) {
    auto method = methodDef.symbol;
    auto idx = 0;
    auto maxLen = min(superArgs.size(), methodArgs.size());

    while (idx < maxLen) {
        auto &superArg = superArgs[idx].get();
        auto &methodArg = methodArgs[idx].get();

        core::ErrorSection::Collector errorDetailsCollector;
        if (!checkSubtype(ctx, constr, selfTypeArgs, methodArg.type, method, superArg.type, superMethod,
                          core::Polarity::Negative, errorDetailsCollector)) {
            if (auto e = ctx.state.beginError(methodArg.loc, core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                            methodArgs[idx].get().show(ctx), methodArg.type.show(ctx),
                            superMethodKind(ctx, superMethod), superMethod.show(ctx));
                e.addErrorLine(superArg.loc, "The super method parameter `{}` was declared here with type `{}`",
                               superArgs[idx].get().show(ctx), superArg.type.show(ctx));
                e.addErrorNote(
                    "A parameter's type must be a supertype of the same parameter's type on the super method.");
                e.addErrorSections(move(errorDetailsCollector));
                e.maybeAddAutocorrect(
                    constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
            }
        }
        idx++;
    }
}

// Ensure that two argument lists are compatible in shape and type and that method visibility is compatible
void validateCompatibleOverride(const core::Context ctx, const ast::ExpressionPtr &tree, core::MethodRef superMethod,
                                const ast::MethodDef &methodDef) {
    auto method = methodDef.symbol;
    auto reportedAutocorrect = false;
    if (method.data(ctx)->flags.isOverloaded) {
        // Don't try to check overloaded methods; It's not immediately clear how
        // to match overloads against their superclass definitions. Since we
        // Only permit overloading in the stdlib for now, this is no great loss.
        return;
    }

    if (superMethod.data(ctx)->flags.isGenericMethod != method.data(ctx)->flags.isGenericMethod &&
        method.data(ctx)->hasSig()) {
        if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
            if (superMethod.data(ctx)->flags.isGenericMethod) {
                e.setHeader("{} method `{}` must declare the same number of type parameters as the base method",
                            implementationOf(ctx, superMethod), superMethod.show(ctx));
            } else {
                e.setHeader("{} method `{}` must not declare type parameters", implementationOf(ctx, superMethod),
                            superMethod.show(ctx));
            }
            e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            e.maybeAddAutocorrect(
                constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
        }
        return;
    }

    unique_ptr<core::TypeConstraint> _constr;
    auto *constr = &core::TypeConstraint::EmptyFrozenConstraint;
    if (method.data(ctx)->flags.isGenericMethod) {
        ENFORCE(superMethod.data(ctx)->flags.isGenericMethod);
        const auto &methodTypeParameters = method.data(ctx)->typeParameters();
        const auto &superMethodTypeParameters = superMethod.data(ctx)->typeParameters();
        if (methodTypeParameters.size() != superMethodTypeParameters.size()) {
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("{} method `{}` must declare the same number of type parameters as the base method",
                            implementationOf(ctx, superMethod), superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");

                e.maybeAddAutocorrect(
                    constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
            }
            return;
        }

        _constr = make_unique<core::TypeConstraint>();
        constr = _constr.get();

        // This doesn't allow the type arguments to be declared in a different order, but it does
        // allow them to be declared with different names.
        //
        // The tradeoff is that this provides a cheap way to produce error messages at the
        // individual arg that is incompatible (versus only at the end once all constraints have
        // been collected) at the cost of rejecting compatible overrides.
        //
        // (An alternative might be to collect a constraint and then after validating all arguments,
        // attempt to find a substitution from one method's type params to the other method's type
        // params, and report an error if no substitution exists, but this tends to result in errors
        // that look like "it failed" with no further context.)
        for (size_t i = 0; i < methodTypeParameters.size(); i++) {
            auto typeParameter = methodTypeParameters[i];
            auto superTypeParameter = superMethodTypeParameters[i];

            constr->rememberIsSubtype(ctx, typeParameter.data(ctx)->resultType,
                                      core::make_type<core::SelfTypeParam>(superTypeParameter));
            constr->rememberIsSubtype(ctx, core::make_type<core::SelfTypeParam>(typeParameter),
                                      superTypeParameter.data(ctx)->resultType);

            constr->rememberIsSubtype(ctx, core::make_type<core::SelfTypeParam>(typeParameter),
                                      typeParameter.data(ctx)->resultType);
            constr->rememberIsSubtype(ctx, superTypeParameter.data(ctx)->resultType,
                                      core::make_type<core::SelfTypeParam>(superTypeParameter));
        }
    }

    auto superSig = decomposeSignature(ctx, superMethod);
    auto sig = decomposeSignature(ctx, method);

    if (!sig.pos.rest) {
        auto superSigPos = superSig.pos.required.size() + superSig.pos.optional.size();
        auto sigPos = sig.pos.required.size() + sig.pos.optional.size();
        if (superSigPos > sigPos) {
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("{} method `{}` must accept at least `{}` positional arguments",
                            implementationOf(ctx, superMethod), superMethod.show(ctx), superSigPos);
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
                e.maybeAddAutocorrect(
                    constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
            }
        }
    }

    if (auto superSigRest = superSig.pos.rest) {
        if (!sig.pos.rest) {
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
                auto [prefix, argName] = formatSplat(superSigRest->get(), SplatKind::ARG, ctx);
                e.setHeader("{} method `{}` must accept {}`{}`", implementationOf(ctx, superMethod),
                            superMethod.show(ctx), prefix, argName);
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");

                e.maybeAddAutocorrect(
                    constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
            }
        }
    }

    if (sig.pos.required.size() > superSig.pos.required.size()) {
        if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("{} method `{}` must accept no more than `{}` required argument(s)",
                        implementationOf(ctx, superMethod), superMethod.show(ctx), superSig.pos.required.size());
            e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            e.maybeAddAutocorrect(
                constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
        }
    }

    // For the sake of comparison, we always compare the two types as if they were being "observed"
    // in the child class, so we always instantiate with the sub class types.
    // Compute out here so we only have to allocate it once.
    auto selfTypeArgs = method.data(ctx)->owner.data(ctx)->selfTypeArgs(ctx);

    // match types of required positional arguments
    matchPositional(ctx, *constr, selfTypeArgs, tree, superSig.pos.required, superMethod, sig.pos.required, methodDef,
                    reportedAutocorrect);
    // match types of optional positional arguments
    matchPositional(ctx, *constr, selfTypeArgs, tree, superSig.pos.optional, superMethod, sig.pos.optional, methodDef,
                    reportedAutocorrect);

    if (!sig.kw.rest) {
        for (auto req : superSig.kw.required) {
            auto corresponding =
                absl::c_find_if(sig.kw.required, [&](const auto &r) { return r.get().name == req.get().name; });

            auto hasCorrespondingRequired = corresponding != sig.kw.required.end();
            if (!hasCorrespondingRequired) {
                corresponding =
                    absl::c_find_if(sig.kw.optional, [&](const auto &r) { return r.get().name == req.get().name; });
            }

            auto hasCorrespondingOptional = corresponding != sig.kw.optional.end();

            // if there is a corresponding parameter, make sure it has the right type
            if (hasCorrespondingRequired || hasCorrespondingOptional) {
                core::ErrorSection::Collector errorDetailsCollector;
                if (!checkSubtype(ctx, *constr, selfTypeArgs, corresponding->get().type, method, req.get().type,
                                  superMethod, core::Polarity::Negative, errorDetailsCollector)) {
                    if (auto e =
                            ctx.state.beginError(corresponding->get().loc, core::errors::Resolver::BadMethodOverride)) {
                        e.setHeader("Keyword parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                                    corresponding->get().show(ctx), corresponding->get().type.show(ctx),
                                    superMethodKind(ctx, superMethod), superMethod.show(ctx));
                        e.addErrorLine(req.get().loc,
                                       "The corresponding parameter `{}` was declared here with type `{}`",
                                       req.get().show(ctx), req.get().type.show(ctx));
                        e.addErrorNote(
                            "A parameter's type must be a supertype of the same parameter's type on the super method.");

                        e.maybeAddAutocorrect(
                            constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
                        e.addErrorSections(move(errorDetailsCollector));
                    }
                }
            } else {
                if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
                    e.setHeader("{} method `{}` is missing required keyword argument `{}`",
                                implementationOf(ctx, superMethod), superMethod.show(ctx), req.get().name.show(ctx));
                    e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
                    e.maybeAddAutocorrect(
                        constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
                }
            }
        }

        // make sure that optional parameters expect a compatible type, as well
        for (auto opt : superSig.kw.optional) {
            auto corresponding =
                absl::c_find_if(sig.kw.optional, [&](const auto &r) { return r.get().name == opt.get().name; });

            // if there is a corresponding parameter, make sure it has the sig type
            if (corresponding != sig.kw.optional.end()) {
                core::ErrorSection::Collector errorDetailsCollector;
                if (!checkSubtype(ctx, *constr, selfTypeArgs, corresponding->get().type, method, opt.get().type,
                                  superMethod, core::Polarity::Negative, errorDetailsCollector)) {
                    if (auto e =
                            ctx.state.beginError(corresponding->get().loc, core::errors::Resolver::BadMethodOverride)) {
                        e.setHeader("Keyword parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                                    corresponding->get().show(ctx), corresponding->get().type.show(ctx),
                                    superMethodKind(ctx, superMethod), superMethod.show(ctx));
                        e.addErrorLine(opt.get().loc,
                                       "The super method parameter `{}` was declared here with type `{}`",
                                       opt.get().show(ctx), opt.get().type.show(ctx));
                        e.addErrorNote(
                            "A parameter's type must be a supertype of the same parameter's type on the super method.");
                        e.maybeAddAutocorrect(
                            constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
                        e.addErrorSections(move(errorDetailsCollector));
                    }
                }
            } else if (absl::c_any_of(sig.kw.required, [&](const auto &r) { return r.get().name == opt.get().name; })) {
                if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
                    e.setHeader("{} method `{}` must redeclare keyword parameter `{}` as optional",
                                implementationOf(ctx, superMethod), superMethod.show(ctx), opt.get().name.show(ctx));
                    // Show the superMethod loc (declLoc) so the error message includes the default value
                    e.addErrorLine(superMethod.data(ctx)->loc(),
                                   "The optional super method parameter `{}` was declared here",
                                   opt.get().name.show(ctx));
                    e.maybeAddAutocorrect(
                        constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
                }
            } else {
                if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
                    e.setHeader("{} method `{}` must accept optional keyword parameter `{}`",
                                implementationOf(ctx, superMethod), superMethod.show(ctx), opt.get().name.show(ctx));
                    // Show the superMethod loc (declLoc) so the error message includes the default value
                    e.addErrorLine(superMethod.data(ctx)->loc(),
                                   "The optional super method parameter `{}` was declared here",
                                   opt.get().name.show(ctx));

                    e.maybeAddAutocorrect(
                        constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
                }
            }
        }
    }

    if (auto superSigRest = superSig.kw.rest) {
        core::ErrorSection::Collector errorDetailsCollector;
        if (!sig.kw.rest) {
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
                auto [prefix, argName] = formatSplat(superSigRest->get(), SplatKind::KWARG, ctx);
                e.setHeader("{} method `{}` must accept {}`{}`", implementationOf(ctx, superMethod),
                            superMethod.show(ctx), prefix, argName);
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
                e.maybeAddAutocorrect(
                    constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
            }
        } else if (!checkSubtype(ctx, *constr, selfTypeArgs, sig.kw.rest->get().type, method, superSigRest->get().type,
                                 superMethod, core::Polarity::Negative, errorDetailsCollector)) {
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Parameter **`{}` of type `{}` not compatible with type of {} method `{}`",
                            sig.kw.rest->get().show(ctx), sig.kw.rest->get().type.show(ctx),
                            superMethodKind(ctx, superMethod), superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(),
                               "The super method parameter **`{}` was declared here with type `{}`",
                               superSig.kw.rest->get().show(ctx), superSig.kw.rest->get().type.show(ctx));
                e.addErrorNote(
                    "A parameter's type must be a supertype of the same parameter's type on the super method.");
                e.addErrorSections(move(errorDetailsCollector));
                e.maybeAddAutocorrect(
                    constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
            }
        }
    }

    for (auto extra : sig.kw.required) {
        if (absl::c_any_of(superSig.kw.required, [&](const auto &l) { return l.get().name == extra.get().name; })) {
            continue;
        }
        if (absl::c_any_of(superSig.kw.optional, [&](const auto &l) { return l.get().name == extra.get().name; })) {
            // We would have already reported a more informative error above.
            continue;
        }
        if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("{} method `{}` contains extra required keyword argument `{}`",
                        implementationOf(ctx, superMethod), superMethod.show(ctx), extra.get().name.toString(ctx));
            e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            e.maybeAddAutocorrect(
                constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
        }
    }

    if (!superSig.syntheticBlk && sig.syntheticBlk) {
        if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("{} method `{}` must explicitly name a block argument", implementationOf(ctx, superMethod),
                        superMethod.show(ctx));
            e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            e.maybeAddAutocorrect(
                constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
        }
    } else {
        const auto &methodBlkParam = method.data(ctx)->parameters.back();
        const auto &superMethodBlkParam = superMethod.data(ctx)->parameters.back();

        core::ErrorSection::Collector errorDetailsCollector;
        if (!checkSubtype(ctx, *constr, selfTypeArgs, methodBlkParam.type, method, superMethodBlkParam.type,
                          superMethod, core::Polarity::Negative, errorDetailsCollector)) {
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Block parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                            methodBlkParam.parameterName(ctx), methodBlkParam.type.show(ctx),
                            superMethodKind(ctx, superMethod), superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(),
                               "The super method parameter `{}` was declared here with type `{}`",
                               superMethodBlkParam.show(ctx), superMethodBlkParam.type.show(ctx));
                e.addErrorNote(
                    "A parameter's type must be a supertype of the same parameter's type on the super method.");
                e.maybeAddAutocorrect(
                    constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
                e.addErrorSections(move(errorDetailsCollector));
            }
        }
    }

    {
        // make sure the return types are compatible

        auto superReturn = superMethod.data(ctx)->resultType;
        auto &methodReturn = method.data(ctx)->resultType;

        core::ErrorSection::Collector errorDetailsCollector;
        if (!checkSubtype(ctx, *constr, selfTypeArgs, methodReturn, method, superReturn, superMethod,
                          core::Polarity::Positive, errorDetailsCollector)) {
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
                auto methodReturnShow = methodReturn == core::Types::void_() ? "void" : methodReturn.show(ctx);
                e.setHeader("Return type `{}` does not match return type of {} method `{}`", methodReturnShow,
                            superMethodKind(ctx, superMethod), superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Super method defined here with return type `{}`",
                               superReturn.show(ctx));
                e.addErrorNote("A method's return type must be a subtype of the return type on the super method.");
                e.addErrorSections(move(errorDetailsCollector));
                e.maybeAddAutocorrect(
                    constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true", reportedAutocorrect));
            }
        }
    }

    // Check method visibility

    if (!method.data(ctx)->flags.allowIncompatibleOverrideVisibility &&
        ((method.data(ctx)->flags.isPrivate &&
          (superMethod.data(ctx)->flags.isProtected || superMethod.data(ctx)->isMethodPublic())) ||
         (method.data(ctx)->flags.isProtected && superMethod.data(ctx)->isMethodPublic()))) {
        if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
            auto modifier = method.data(ctx)->flags.isPrivate ? "private" : "protected";
            e.setHeader("Method `{}` is {} in `{}` but not in `{}`", method.data(ctx)->name.show(ctx), modifier,
                        method.data(ctx)->owner.show(ctx), superMethod.data(ctx)->owner.show(ctx));
            e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");

            auto len = method.data(ctx)->flags.isPrivate ? 7 : 9;
            auto loc = ctx.locAt(methodDef.declLoc).adjustLen(ctx, -(len + 1), len);
            if (ctx.state.suggestUnsafe) {
                e.maybeAddAutocorrect(
                    constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, ":visibility", reportedAutocorrect));
            } else if (loc.source(ctx) == modifier) {
                e.replaceWith("Replace with public", loc, "public");
            }
        }
    }
}

optional<core::AutocorrectSuggestion>
constructOverrideAutocorrect(const core::Context ctx, const ast::ExpressionPtr &tree, const ast::MethodDef &methodDef) {
    auto methodLoc = ctx.locAt(methodDef.declLoc);

    auto parsedSig = sig_finder::SigFinder::findSignature(ctx, tree, methodLoc.copyWithZeroLength());
    if (!parsedSig.has_value()) {
        return nullopt;
    }

    auto &origSend = parsedSig->origSend;

    // If the sig itself is generated, it doesn't make much sense to suggest an autocorrect for it.
    if (origSend.flags.isRewriterSynthesized) {
        return nullopt;
    }

    auto *block = origSend.block();
    if (!block) {
        return nullopt;
    }

    auto blockBody = ast::cast_tree<ast::Send>(block->body);
    ENFORCE(blockBody != nullptr);
    auto insertLoc = ctx.locAt(blockBody->loc.copyWithZeroLength());

    vector<core::AutocorrectSuggestion::Edit> edits;
    edits.emplace_back(core::AutocorrectSuggestion::Edit{insertLoc, "override."});
    return core::AutocorrectSuggestion{
        fmt::format("Add `{}` to `{}` sig", "override", methodDef.symbol.data(ctx)->name.show(ctx)),
        std::move(edits),
    };
}

void validateOverriding(const core::Context ctx, const ast::ExpressionPtr &tree, const ast::MethodDef &methodDef) {
    auto method = methodDef.symbol;
    auto klass = method.data(ctx)->owner;
    auto name = method.data(ctx)->name;
    auto klassData = klass.data(ctx);
    InlinedVector<core::MethodRef, 4> overriddenMethods;

    // Matches the behavior of the runtime checks
    // NOTE(jez): I don't think this check makes all that much sense, but I haven't thought about it.
    // We already deleted the corresponding check for `private`, and may want to revisit this, too.
    if (klassData->flags.isInterface && method.data(ctx)->flags.isProtected) {
        if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::NonPublicAbstract)) {
            e.setHeader("Interface method `{}` cannot be protected", method.show(ctx));
        }
    }

    if (method.data(ctx)->flags.isAbstract && klassData->isSingletonClass(ctx)) {
        auto attached = klassData->attachedClass(ctx);
        if (attached.exists() && attached.data(ctx)->isModule()) {
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::StaticAbstractModuleMethod)) {
                e.setHeader("Static methods in a module cannot be abstract");
            }
        }
    }

    if (klassData->superClass().exists()) {
        auto superMethod = klassData->superClass().data(ctx)->findMethodTransitive(ctx, name);
        if (superMethod.exists()) {
            if (superMethod.data(ctx)->flags.isOverloaded) {
                ENFORCE(!superMethod.data(ctx)->name.isOverloadName(ctx));
                auto overload = ctx.state.lookupNameUnique(core::UniqueNameKind::Overload, name, 1);
                superMethod = superMethod.data(ctx)->owner.data(ctx)->findMethod(ctx, overload);
                ENFORCE(superMethod.exists());
            }

            overriddenMethods.emplace_back(superMethod);
        }
    }
    for (const auto &mixin : klassData->mixins()) {
        auto superMethod = mixin.data(ctx)->findMethod(ctx, name);
        if (superMethod.exists()) {
            if (superMethod.data(ctx)->flags.isOverloaded) {
                ENFORCE(!superMethod.data(ctx)->name.isOverloadName(ctx));
                auto overload = ctx.state.lookupNameUnique(core::UniqueNameKind::Overload, name, 1);
                superMethod = superMethod.data(ctx)->owner.data(ctx)->findMethod(ctx, overload);
                ENFORCE(superMethod.exists());
            }

            overriddenMethods.emplace_back(superMethod);
        }
    }

    if (overriddenMethods.size() == 0 && method.data(ctx)->flags.isOverride &&
        !method.data(ctx)->flags.allowIncompatibleOverrideAll) {
        if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Method `{}` is marked `{}` but does not override anything", method.show(ctx), "override");
            e.maybeAddAutocorrect(constructAllowIncompatibleAutocorrect(ctx, tree, methodDef, "true"));
        }
    }

    // we don't raise override errors if the method implements an abstract method, which means we need to know ahead of
    // time whether any parent methods are abstract
    auto anyIsInterface = absl::c_any_of(overriddenMethods, [&](auto &m) { return m.data(ctx)->flags.isAbstract; });
    for (const auto &overriddenMethod : overriddenMethods) {
        if (overriddenMethod.data(ctx)->flags.isFinal) {
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::OverridesFinal)) {
                e.setHeader("`{}` was declared as final and cannot be overridden by `{}`", overriddenMethod.show(ctx),
                            method.show(ctx));
                e.addErrorLine(overriddenMethod.data(ctx)->loc(), "original method defined here");
            }
        }
        auto isRBI = absl::c_any_of(method.data(ctx)->locs(), [&](auto &loc) { return loc.file().data(ctx).isRBI(); });
        if (!method.data(ctx)->flags.isOverride && method.data(ctx)->hasSig() &&
            (overriddenMethod.data(ctx)->flags.isOverridable || overriddenMethod.data(ctx)->flags.isOverride) &&
            !anyIsInterface && overriddenMethod.data(ctx)->hasSig() && !method.data(ctx)->flags.isRewriterSynthesized &&
            !isRBI) {
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::UndeclaredOverride)) {
                e.setHeader("Method `{}` overrides an overridable method `{}` but is not declared with `{}`",
                            method.show(ctx), overriddenMethod.show(ctx), "override.");
                e.addErrorLine(overriddenMethod.data(ctx)->loc(), "defined here");

                e.maybeAddAutocorrect(constructOverrideAutocorrect(ctx, tree, methodDef));
            }
        }
        if (!method.data(ctx)->flags.isOverride && !method.data(ctx)->flags.isAbstract && method.data(ctx)->hasSig() &&
            overriddenMethod.data(ctx)->flags.isAbstract && overriddenMethod.data(ctx)->hasSig() &&
            !method.data(ctx)->flags.isRewriterSynthesized && !isRBI) {
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Resolver::UndeclaredOverride)) {
                e.setHeader("Method `{}` implements an abstract method `{}` but is not declared with `{}`",
                            method.show(ctx), overriddenMethod.show(ctx), "override.");
                e.addErrorLine(overriddenMethod.data(ctx)->loc(), "defined here");

                e.maybeAddAutocorrect(constructOverrideAutocorrect(ctx, tree, methodDef));
            }
        }
        if ((overriddenMethod.data(ctx)->flags.isAbstract || overriddenMethod.data(ctx)->flags.isOverridable ||
             (overriddenMethod.data(ctx)->hasSig() && method.data(ctx)->flags.isOverride)) &&
            !method.data(ctx)->flags.allowIncompatibleOverrideAll && !isRBI &&
            !method.data(ctx)->flags.isRewriterSynthesized &&
            overriddenMethod != core::Symbols::BasicObject_initialize()) {
            // We only ignore BasicObject#initialize for backwards compatibility.
            // One day, we may want to build something like overridable(allow_incompatible: true)
            // and mark certain methods in the standard library as possible to be overridden incompatibly,
            // without needing to write `override(allow_incompatible: true)`.
            // Further context: https://blog.jez.io/constructor-override-checking/
            validateCompatibleOverride(ctx, tree, overriddenMethod, methodDef);
        }
    }
}

core::LocOffsets getAncestorLoc(const core::GlobalState &gs, const ast::ClassDef &classDef,
                                const core::ClassOrModuleRef ancestor) {
    for (const auto &anc : classDef.ancestors) {
        const auto ancConst = ast::cast_tree<ast::ConstantLit>(anc);
        if (ancConst != nullptr && ancConst->symbol().dealias(gs) == ancestor) {
            return anc.loc();
        }
    }
    for (const auto &anc : classDef.singletonAncestors) {
        const auto ancConst = ast::cast_tree<ast::ConstantLit>(anc);
        if (ancConst != nullptr && ancConst->symbol().dealias(gs) == ancestor) {
            return anc.loc();
        }
    }
    // give up
    return classDef.loc;
}

void validateFinalAncestorHelper(core::Context ctx, const core::ClassOrModuleRef klass, const ast::ClassDef &classDef,
                                 const core::ClassOrModuleRef errMsgClass, const string_view verb) {
    for (const auto &mixin : klass.data(ctx)->mixins()) {
        if (!mixin.data(ctx)->flags.isFinal) {
            continue;
        }
        if (auto e = ctx.beginError(getAncestorLoc(ctx, classDef, mixin), core::errors::Resolver::FinalAncestor)) {
            e.setHeader("`{}` was declared as final and cannot be {} in `{}`", mixin.show(ctx), verb,
                        errMsgClass.show(ctx));
            e.addErrorLine(mixin.data(ctx)->loc(), "`{}` defined here", mixin.show(ctx));
        }
    }
}

void validateFinalMethodHelper(core::Context ctx, const core::ClassOrModuleRef klass, const ast::ClassDef &classDef,
                               const core::ClassOrModuleRef errMsgClass) {
    if (!klass.data(ctx)->flags.isFinal) {
        return;
    }
    for (const auto [name, sym] : klass.data(ctx)->members()) {
        // We only care about method symbols that exist.
        if (!sym.exists() || !sym.isMethod() ||
            // Method is 'final', and passes the check.
            sym.asMethodRef().data(ctx)->flags.isFinal ||
            // <static-init> is a fake method Sorbet synthesizes for typechecking.
            sym.name(ctx) == core::Names::staticInit() ||
            // <unresolved-ancestors> is a fake method Sorbet synthesizes to ensure class hierarchy changes in IDE take
            // slow path.
            sym.name(ctx) == core::Names::unresolvedAncestors()) {
            continue;
        }
        auto defLoc = sym.loc(ctx);
        if (auto e = ctx.state.beginError(defLoc, core::errors::Resolver::FinalModuleNonFinalMethod)) {
            e.setHeader("`{}` was declared as final but its method `{}` was not declared as final",
                        errMsgClass.show(ctx), sym.name(ctx).show(ctx));
            auto queryLoc = defLoc.copyWithZeroLength();
            auto parsedSig = sig_finder::SigFinder::findSignature(ctx, classDef, queryLoc);

            if (parsedSig.has_value() && parsedSig->origSend.funLoc.exists()) {
                auto funLoc = ctx.locAt(parsedSig->origSend.funLoc);
                e.replaceWith("Mark it as `sig(:final)`", funLoc, "sig(:final)");
            }
        }
    }
}

void validateFinal(core::Context ctx, const core::ClassOrModuleRef klass, const ast::ClassDef &classDef) {
    const auto superClass = klass.data(ctx)->superClass();
    if (superClass.exists()) {
        auto superClassData = superClass.data(ctx);
        if (superClassData->flags.isFinal) {
            if (auto e =
                    ctx.beginError(getAncestorLoc(ctx, classDef, superClass), core::errors::Resolver::FinalAncestor)) {
                e.setHeader("`{}` was declared as final and cannot be inherited by `{}`", superClass.show(ctx),
                            klass.show(ctx));
                e.addErrorLine(superClassData->loc(), "`{}` defined here", superClass.show(ctx));
            }
        } else if (superClass != core::Symbols::T_Enum() && superClassData->derivesFrom(ctx, core::Symbols::T_Enum()) &&
                   !klass.data(ctx)->name.isTEnumName(ctx)) {
            // We can't have the T::Enum rewriter pretend that the class with `enums do` is `final!` because it
            // is actually subclassed (for the sake of making a subclass for each enum value). But the runtime
            // will raise in this situation, so let's add a matching static error.
            if (auto e =
                    ctx.beginError(getAncestorLoc(ctx, classDef, superClass), core::errors::Resolver::FinalAncestor)) {
                e.setHeader("`{}` descends from `{}` and thus cannot be subclassed by `{}`", superClass.show(ctx),
                            "T::Enum", klass.show(ctx));
                e.addErrorLine(superClassData->loc(), "`{}` defined here", superClass.show(ctx));
            }
        }
    }
    validateFinalAncestorHelper(ctx, klass, classDef, klass, "included");
    validateFinalMethodHelper(ctx, klass, classDef, klass);
    const auto singleton = klass.data(ctx)->lookupSingletonClass(ctx);
    validateFinalAncestorHelper(ctx, singleton, classDef, klass, "extended");
    validateFinalMethodHelper(ctx, singleton, classDef, klass);
}

// Ignore RBI files for the purpose of checking sealed (unless there are no other files).
// Sealed violations in RBI files too frequently come from generated RBI files, and usually if
// people are using sealed!, they're trying to make the source available to Sorbet anyways.
// Regardless, the runtime will still ultimately check violations in untyped code.
core::FileRef bestNonRBIFile(core::Context ctx, const core::ClassOrModuleRef klass) {
    core::FileRef bestFile;
    for (const auto &cur : klass.data(ctx)->locs()) {
        auto curFile = cur.file();

        if (!bestFile.exists()) {
            bestFile = curFile;
            continue;
        }

        if (curFile.data(ctx).isRBI()) {
            continue;
        }

        if (bestFile.data(ctx).isRBI()) {
            bestFile = curFile;
            continue;
        }

        if (curFile.data(ctx).strictLevel > bestFile.data(ctx).strictLevel) {
            bestFile = curFile;
        }
    }

    return bestFile;
}

void validateSealedAncestorHelper(core::Context ctx, const core::ClassOrModuleRef klass, const ast::ClassDef &classDef,
                                  const core::ClassOrModuleRef errMsgClass, const string_view verb) {
    for (const auto &mixin : klass.data(ctx)->mixins()) {
        if (!mixin.data(ctx)->flags.isSealed) {
            continue;
        }

        auto klassFile = bestNonRBIFile(ctx, klass);

        // See the comment on the isPackageRBI call in `validateSealed` for more information about why we skip package
        // rbi files here.
        if (klassFile.data(ctx).isPackageRBI()) {
            continue;
        }

        if (absl::c_any_of(mixin.data(ctx)->sealedLocs(ctx),
                           [klassFile](auto loc) { return loc.file() == klassFile; })) {
            continue;
        }
        if (auto e = ctx.beginError(getAncestorLoc(ctx, classDef, mixin), core::errors::Resolver::SealedAncestor)) {
            e.setHeader("`{}` is sealed and cannot be {} in `{}`", mixin.show(ctx), verb, errMsgClass.show(ctx));
            for (auto loc : mixin.data(ctx)->sealedLocs(ctx)) {
                e.addErrorLine(loc, "`{}` was marked sealed and can only be {} in this file", mixin.show(ctx), verb);
            }
        }
    }
}

void validateSealed(core::Context ctx, const core::ClassOrModuleRef klass, const ast::ClassDef &classDef) {
    const auto superClass = klass.data(ctx)->superClass();
    if (superClass.exists() && superClass.data(ctx)->flags.isSealed) {
        auto file = bestNonRBIFile(ctx, klass);

        // Normally we would skip RBIs for the purpose of checking `sealed!`, but when running in stripe-packages mode
        // this may be the only definition available. Additionally, if some of the sealed subclasses are only exported
        // for use in tests, those subclasses will be appear in a separate `.test.package.rbi` file, despite the parent
        // class being defined in a `.package.rbi` file. Because this check would fail in those situations and the RBI
        // files will have been generated from valid sources, we assume that they are correct here and skip the check.
        if (!file.data(ctx).isPackageRBI()) {
            if (!absl::c_any_of(superClass.data(ctx)->sealedLocs(ctx),
                                [file](auto loc) { return loc.file() == file; })) {
                if (auto e = ctx.beginError(getAncestorLoc(ctx, classDef, superClass),
                                            core::errors::Resolver::SealedAncestor)) {
                    e.setHeader("`{}` is sealed and cannot be inherited by `{}`", superClass.show(ctx),
                                klass.show(ctx));
                    for (auto loc : superClass.data(ctx)->sealedLocs(ctx)) {
                        e.addErrorLine(loc, "`{}` was marked sealed and can only be inherited in this file",
                                       superClass.show(ctx));
                    }
                }
            }
        }
    }
    validateSealedAncestorHelper(ctx, klass, classDef, klass, "included");
    const auto singleton = klass.data(ctx)->lookupSingletonClass(ctx);
    validateSealedAncestorHelper(ctx, singleton, classDef, klass, "extended");
}

void validateSuperClass(core::Context ctx, const core::ClassOrModuleRef sym, const ast::ClassDef &classDef) {
    if (!sym.data(ctx)->isClass()) {
        // In this case, a class with the same name as a module has been defined, and we'll already have raised an error
        // in the namer.
        return;
    }

    // If the ancestors are empty or the first element is a todo, this means that there was syntactically no superclass
    if (classDef.ancestors.empty()) {
        return;
    }

    if (auto cnst = ast::cast_tree<ast::ConstantLit>(classDef.ancestors.front())) {
        if (cnst->symbol() == core::Symbols::todo()) {
            return;
        }
    }

    const auto superClass = sym.data(ctx)->superClass();
    if (!superClass.exists()) {
        // Happens for certain special classes at the top of the inheritance hierarchy.
        return;
    }

    // these will raise an error elsewhere
    if (superClass == core::Symbols::StubModule() || superClass == core::Symbols::StubSuperClass()) {
        return;
    }

    const auto superSingleton = superClass.data(ctx)->lookupSingletonClass(ctx);
    if (!superSingleton.exists()) {
        return;
    }

    if (superSingleton.data(ctx)->derivesFrom(ctx, core::Symbols::Class())) {
        return;
    }

    if (auto e = ctx.state.beginError(ctx.locAt(classDef.declLoc), core::errors::Resolver::NonClassSuperclass)) {
        auto superClassFqn = superClass.show(ctx);
        e.setHeader("The super class `{}` of `{}` does not derive from `{}`", superClassFqn, sym.show(ctx),
                    core::Symbols::Class().show(ctx));
        for (const auto loc : superClass.data(ctx)->locs()) {
            e.addErrorLine(loc, "{} defined here", superClassFqn);
        }
    }
}

void validateUselessRequiredAncestors(core::Context ctx, const core::ClassOrModuleRef sym) {
    auto data = sym.data(ctx);

    for (auto req : data->requiredAncestors(ctx)) {
        if (data->derivesFrom(ctx, req.symbol)) {
            if (auto e = ctx.state.beginError(req.loc, core::errors::Resolver::UselessRequiredAncestor)) {
                e.setHeader("`{}` is already {} by `{}`", req.symbol.show(ctx),
                            req.symbol.data(ctx)->isModule() ? "included" : "inherited", sym.show(ctx));
            }
        }
    }
}

void validateUnsatisfiedRequiredAncestors(core::Context ctx, const core::ClassOrModuleRef sym) {
    auto data = sym.data(ctx);
    if (data->isModule() || data->flags.isAbstract) {
        return;
    }
    for (auto req : data->requiredAncestorsTransitive(ctx)) {
        if (sym != req.symbol && !data->derivesFrom(ctx, req.symbol)) {
            if (auto e = ctx.state.beginError(data->loc(), core::errors::Resolver::UnsatisfiedRequiredAncestor)) {
                e.setHeader("`{}` must {} `{}` (required by `{}`)", sym.show(ctx),
                            req.symbol.data(ctx)->isModule() ? "include" : "inherit", req.symbol.show(ctx),
                            req.origin.show(ctx));
                e.addErrorLine(req.loc, "required by `{}` here", req.origin.show(ctx));
            }
        }
    }
}

void validateUnsatisfiableRequiredAncestors(core::Context ctx, const core::ClassOrModuleRef sym) {
    auto data = sym.data(ctx);

    vector<core::ClassOrModule::RequiredAncestor> requiredClasses;
    for (auto ancst : data->requiredAncestorsTransitive(ctx)) {
        if (ancst.symbol.data(ctx)->isClass()) {
            requiredClasses.emplace_back(ancst);
        }

        auto isSingletonClass = ancst.symbol.data(ctx)->isSingletonClass(ctx);
        auto typeArity = ancst.symbol.data(ctx)->typeArity(ctx);
        auto isModule = ancst.symbol == core::Symbols::Module() && typeArity == 1;

        if ((isSingletonClass && typeArity > 1) || (!isSingletonClass && typeArity > 0 && !isModule)) {
            if (auto e = ctx.state.beginError(data->loc(), core::errors::Resolver::UnsatisfiableRequiredAncestor)) {
                e.setHeader("`{}` can't require generic ancestor `{}` (unsupported)", sym.show(ctx),
                            ancst.symbol.show(ctx));
                e.addErrorLine(ancst.loc, "`{}` is required by `{}` here", ancst.symbol.show(ctx),
                               ancst.origin.show(ctx));
            }
        }
    }

    if (data->isClass() && data->flags.isAbstract) {
        for (auto ancst : requiredClasses) {
            if (!sym.data(ctx)->derivesFrom(ctx, ancst.symbol) && !ancst.symbol.data(ctx)->derivesFrom(ctx, sym)) {
                if (auto e = ctx.state.beginError(data->loc(), core::errors::Resolver::UnsatisfiableRequiredAncestor)) {
                    e.setHeader("`{}` requires unrelated class `{}` making it impossible to inherit", sym.show(ctx),
                                ancst.symbol.show(ctx));
                    e.addErrorLine(ancst.loc, "`{}` is required by `{}` here", ancst.symbol.show(ctx),
                                   ancst.origin.show(ctx));
                }
            }
        }
    }

    if (requiredClasses.size() <= 1) {
        return;
    }

    for (int i = 0; i < requiredClasses.size() - 1; i++) {
        auto ra1 = requiredClasses[i];
        for (int j = i + 1; j < requiredClasses.size(); j++) {
            auto ra2 = requiredClasses[j];
            if (!ra1.symbol.data(ctx)->derivesFrom(ctx, ra2.symbol) &&
                !ra2.symbol.data(ctx)->derivesFrom(ctx, ra1.symbol)) {
                if (auto e = ctx.state.beginError(data->loc(), core::errors::Resolver::UnsatisfiableRequiredAncestor)) {
                    e.setHeader("`{}` requires unrelated classes `{}` and `{}` making it impossible to {}",
                                sym.show(ctx), ra1.symbol.show(ctx), ra2.symbol.show(ctx),
                                data->isModule() ? "include" : "inherit");
                    e.addErrorLine(ra1.loc, "`{}` is required by `{}` here", ra1.symbol.show(ctx),
                                   ra1.origin.show(ctx));
                    e.addErrorLine(ra2.loc, "`{}` is required by `{}` here", ra2.symbol.show(ctx),
                                   ra2.origin.show(ctx));
                }
            }
        }
    }
}

void validateRequiredAncestors(core::Context ctx, const core::ClassOrModuleRef sym) {
    validateUselessRequiredAncestors(ctx, sym);
    validateUnsatisfiedRequiredAncestors(ctx, sym);
    validateUnsatisfiableRequiredAncestors(ctx, sym);
}

class ValidateWalk {
public:
    ValidateWalk(const ast::ExpressionPtr &topTree) : topTree(topTree) {}

private:
    // NOTE: A better representation for our AST might be to store a method's signature(s) within
    // the MethodDef itself, so that it's trivial to access a method's signature in the
    // preTransformMethodDef.
    //
    // definition_validator runs after class_flatten, which means that the signatures and the live in the
    // `<static-init>` method of a class, while the MethodDef nodes live at the top level of a class
    // body. Changing the representation would be tricky because signtures themselves are type
    // checked (powering things like hover/go-to-def inside signatures).
    //
    // Instead, we have to keep a reference to the whole `tree` in this walk, and use `SigFinder` to
    // find the signature for a given method (incurring a full walk of the tree) every time we want
    // to find a single method. That means instead of ValidateWalk only doing one walk of the tree,
    // it does `num_errors + 1` walks, which can be slow in the case of many errors.
    const ast::ExpressionPtr &topTree;

    UnorderedMap<core::ClassOrModuleRef, vector<core::MethodRef>> abstractCache;

    const vector<core::MethodRef> &getAbstractMethods(const core::GlobalState &gs, core::ClassOrModuleRef klass) {
        vector<core::MethodRef> abstract;
        auto ent = abstractCache.find(klass);
        if (ent != abstractCache.end()) {
            return ent->second;
        }

        auto superclass = klass.data(gs)->superClass();
        if (superclass.exists()) {
            auto &superclassMethods = getAbstractMethods(gs, superclass);
            // TODO(nelhage): This code could go quadratic or even exponential given
            // pathological arrangements of interfaces and abstract methods. Switch
            // to a better data structure if that is ever a problem.
            abstract.insert(abstract.end(), superclassMethods.begin(), superclassMethods.end());
        }

        for (auto ancst : klass.data(gs)->mixins()) {
            auto fromMixin = getAbstractMethods(gs, ancst);
            abstract.insert(abstract.end(), fromMixin.begin(), fromMixin.end());
        }

        auto isAbstract = klass.data(gs)->flags.isAbstract;
        if (isAbstract) {
            for (auto [name, sym] : klass.data(gs)->members()) {
                if (!sym.exists() || !sym.isMethod()) {
                    continue;
                }
                const auto &method = sym.asMethodRef().data(gs);
                // Ignore mangle renames, because users shouldn't have to create *another*
                // mangle rename error in order to implement such an abstract method.
                if (method->flags.isAbstract &&
                    !method->name.hasUniqueNameKind(gs, core::UniqueNameKind::MangleRename)) {
                    abstract.emplace_back(sym.asMethodRef());
                }
            }
        }

        auto &entry = abstractCache[klass];
        entry = std::move(abstract);
        return entry;
    }

    // if/when we get final classes, we can just mark subclasses of `T::Struct` as final and essentially subsume the
    // logic here.
    void validateTStructNotGrandparent(const core::GlobalState &gs, core::ClassOrModuleRef sym) {
        auto parent = sym.data(gs)->superClass();
        if (!parent.exists()) {
            return;
        }
        auto grandparent = parent.data(gs)->superClass();
        if (!grandparent.exists() || grandparent != core::Symbols::T_Struct()) {
            return;
        }
        if (auto e = gs.beginError(sym.data(gs)->loc(), core::errors::Resolver::SubclassingNotAllowed)) {
            auto parentName = parent.show(gs);
            e.setHeader("Subclassing `{}` is not allowed", parentName);
            auto parentDeclLoc = parent.data(gs)->loc();
            e.addErrorLine(parentDeclLoc, "`{}` is a subclass of `{}`", parentName, "T::Struct");
            if (gs.suggestUnsafe && parentDeclLoc.exists()) {
                auto declSource = parentDeclLoc.source(gs).value();
                auto ltTStruct = "< T::Struct"sv;
                if (absl::EndsWith(declSource, ltTStruct)) {
                    e.replaceWith("Replace with `T::InexactStruct`", parentDeclLoc, "{}< {}",
                                  declSource.substr(0, declSource.size() - ltTStruct.size()), "T::InexactStruct");
                }
            }
        }
    }

    string defineInheritedAbstractMethod(const core::GlobalState &gs, const core::ClassOrModuleRef sym,
                                         const core::MethodRef abstractMethodRef, const string &classOrModuleIndent) {
        auto showOptions = core::ShowOptions().withUseValidSyntax().withConcretizeIfAbstract();
        if (sym.data(gs)->attachedClass(gs).exists()) {
            showOptions = showOptions.withForceSelfPrefix();
        }
        auto methodDefinition =
            core::source_generator::prettyTypeForMethod(gs, abstractMethodRef, nullptr, showOptions);

        vector<string> indentedLines;
        absl::c_transform(
            absl::StrSplit(methodDefinition, "\n"), std::back_inserter(indentedLines),
            [classOrModuleIndent](auto &line) -> string { return fmt::format("{}  {}", classOrModuleIndent, line); });
        auto indentedMethodDefinition = absl::StrJoin(indentedLines, "\n");
        return indentedMethodDefinition;
    }

    void validateAbstract(const core::Context ctx, core::ClassOrModuleRef sym, const ast::ClassDef &classDef) {
        if (sym.data(ctx)->flags.isAbstract) {
            return;
        }

        if (ctx.file.data(ctx).isRBI()) {
            return;
        }

        auto missingAbstractMethods = findMissingAbstractMethods(ctx, sym);
        if (missingAbstractMethods.empty()) {
            return;
        }

        auto errorBuilder = ctx.beginError(classDef.declLoc, core::errors::Resolver::BadAbstractMethod);
        if (!errorBuilder) {
            return;
        }

        if (missingAbstractMethods.size() > 1) {
            errorBuilder.setHeader("Missing definitions for abstract methods in `{}`", sym.show(ctx));
        } else {
            errorBuilder.setHeader("Missing definition for abstract method `{}` in `{}`",
                                   missingAbstractMethods.front().show(ctx), sym.show(ctx));
        }

        auto classOrModuleDeclaredAt = ctx.locAt(classDef.declLoc);
        auto classOrModuleEndsAt = ctx.locAt(classDef.loc.copyEndWithZeroLength());
        auto edits =
            core::insert_method::run(ctx, missingAbstractMethods, sym, classOrModuleDeclaredAt, classOrModuleEndsAt);

        if (edits.empty()) {
            return;
        }

        errorBuilder.addAutocorrect(core::AutocorrectSuggestion{
            fmt::format("Define inherited abstract method{}", missingAbstractMethods.size() > 1 ? "s" : ""),
            edits,
        });
    }

    vector<core::MethodRef> findMissingAbstractMethods(const core::Context ctx, core::ClassOrModuleRef sym) {
        vector<core::MethodRef> result;

        auto &abstractMethods = getAbstractMethods(ctx, sym);
        if (abstractMethods.empty()) {
            return result;
        }

        for (auto proto : abstractMethods) {
            if (proto.data(ctx)->owner == sym) {
                continue;
            }

            // Overload signatures all have unique names, so to find the name of the concrete implementation we need to
            // use the original name, not the unique one.
            auto protoName = proto.data(ctx)->name;
            if (protoName.isOverloadName(ctx)) {
                protoName = protoName.dataUnique(ctx)->original;
            }

            auto concreteMethodRef = sym.data(ctx)->findConcreteMethodTransitive(ctx, protoName);
            if (concreteMethodRef.exists()) {
                continue;
            }

            result.emplace_back(proto);
        }

        // Sort missing methods to ensure consistent ordering in both the error message and the autocorrect output.
        fast_sort(result, [ctx](const auto &l, const auto &r) -> bool {
            return l.data(ctx)->name.show(ctx) < r.data(ctx)->name.show(ctx);
        });

        // Deduplicate methods to prevent suggesting duplicate corrections for methods that are missing from several
        // ancestors in the same inheritance chain.
        result.resize(std::distance(result.begin(), std::unique(result.begin(), result.end())));

        return result;
    }

public:
    void preTransformClassDef(core::Context ctx, const ast::ClassDef &classDef) {
        auto sym = classDef.symbol;
        auto singleton = sym.data(ctx)->lookupSingletonClass(ctx);
        validateTStructNotGrandparent(ctx, sym);
        if (!sym.data(ctx)->isSingletonClass(ctx)) {
            // Only validateAbstract for this class if we haven't already (we already have if this
            // is a `class << self` ClassDef)
            validateAbstract(ctx, sym, classDef);

            if (ctx.state.cacheSensitiveOptions.requiresAncestorEnabled) {
                validateRequiredAncestors(ctx, sym);
            }
        }
        validateAbstract(ctx, singleton, classDef);
        validateFinal(ctx, sym, classDef);
        validateSealed(ctx, sym, classDef);
        validateSuperClass(ctx, sym, classDef);

        if (ctx.state.cacheSensitiveOptions.requiresAncestorEnabled) {
            validateRequiredAncestors(ctx, singleton);
        }
    }

    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef) {
        auto methodData = methodDef.symbol.data(ctx);

        if (methodData->locs().empty()) {
            Exception::raise("method has no locs! ctx.file=\"{}\" method=\"{}\"", ctx.file.data(ctx).path(),
                             methodDef.symbol.show(ctx));
        }
        if (!methodData->loc().file().exists()) {
            Exception::raise("file for method does not exist! ctx.file=\"{}\" method=\"{}\"", ctx.file.data(ctx).path(),
                             methodDef.symbol.show(ctx));
        }

        if (ctx.file.data(ctx).isRBI() && !methodData->flags.isOverloaded && !methodData->hasSig()) {
            // Only check RBI files here, because non-RBI files will have this reported in
            // inference, to be able to report a suggested sig. Note that this might report multiple
            // errors (if a method is in a source file and an RBI file both without a sig), but that's okay.
            if (auto e = ctx.beginError(methodDef.declLoc, core::errors::Infer::UntypedMethod)) {
                e.setHeader("The method `{}` does not have a `{}`", methodData->name.show(ctx), "sig");
            }
        }

        // Only perform this check if this isn't a module from the stdlib, and
        // if there are type members in the owning context.
        // NOTE: We're skipping variance checks on the stdlib right now, as
        // Array and Hash are defined with their parameters as covariant, and as
        // a result most of their methods would fail this check.
        if (!methodData->loc().file().data(ctx).isStdlib()) {
            variance::validateMethodVariance(ctx, methodDef.symbol);
        }

        // See the comment in `VarianceValidator::validateMethod` for an explanation of why we don't
        // need to check types on instance variables.

        validateOverriding(ctx, this->topTree, methodDef);
    }
};
} // namespace

void runOne(core::Context ctx, const ast::ParsedFile &tree) {
    Timer timeit(ctx.state.tracer(), "validateSymbols", {{"file", string(tree.file.data(ctx).path())}});

    ValidateWalk validate(tree.tree);
    ast::ConstShallowWalk::apply(ctx, validate, tree.tree);
}

} // namespace sorbet::definition_validator
