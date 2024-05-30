#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/sort/sort.h"
#include "common/timers/Timer.h"
#include "core/TypeErrorDiagnostics.h"
#include "core/core.h"
#include "core/errors/resolver.h"
#include "core/source_generator/source_generator.h"

#include "absl/algorithm/container.h"

#include "core/sig_finder/sig_finder.h"
#include "definition_validator/variance.h"
#include <regex>

using namespace std;

namespace sorbet::definition_validator {

namespace {
struct Signature {
    struct {
        absl::InlinedVector<reference_wrapper<const core::ArgInfo>, 4> required;
        absl::InlinedVector<reference_wrapper<const core::ArgInfo>, 4> optional;
        std::optional<reference_wrapper<const core::ArgInfo>> rest;
    } pos, kw;
    bool syntheticBlk;
} left, right;

Signature decomposeSignature(const core::GlobalState &gs, core::MethodRef method) {
    Signature sig;
    for (auto &arg : method.data(gs)->arguments) {
        if (arg.flags.isBlock) {
            sig.syntheticBlk = arg.isSyntheticBlockArgument();
            continue;
        }

        auto &dst = arg.flags.isKeyword ? sig.kw : sig.pos;
        if (arg.flags.isRepeated) {
            dst.rest = std::optional<reference_wrapper<const core::ArgInfo>>{arg};
        } else if (arg.flags.isDefault) {
            dst.optional.push_back(arg);
        } else {
            dst.required.push_back(arg);
        }
    }
    return sig;
}

// This is really just a useful helper function for this module: do not use it elsewhere.
//
// It makes certain assumptions that it is running for the sake of computing overrides that are not
// going to be true in other situations.
bool checkSubtype(const core::Context ctx, core::TypeConstraint &constr, const core::TypePtr &sub,
                  core::MethodRef subMethod, const core::TypePtr &super, core::MethodRef superMethod,
                  core::Polarity polarity, core::ErrorSection::Collector &errorDetailsCollector) {
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
    // Sorbet, those skolems are SelfTypeParam types that wrap a TypeArgumentRef.
    //
    // Types::approximate does this "replace all the TypeVar with SelfTypeParam" naturally and in a
    // predictable way (i.e., respecting polarity), so it's convenient do to this with approximate
    // rather than build a dedicated function for this.
    //
    // However, it's neither required nor desired to use that constraint to type check the two types
    // themselves. If we somehow managed to construct the constr incorrectly, there might still be
    // un-skolemized TypeVar's in either type, which would then record new constraints during the
    // call to isSubType. To guarantee that never happens, we typecheck the type under the
    // EmptyFrozenConstraint, instead of this constraint that we should only be using for skolemization.
    //
    // Another approach might be to create the constr right here, instead of threading it around
    // everywhere. We've taken the aprpopach of only constructing the constraint once as an optimization.

    // For the sake of comparison, we always compare the two types as if they were being "observed"
    // in the child class, so we always instantiate with the sub class types
    const auto &subSelfTypeArgs = subOwner.data(ctx)->selfTypeArgs(ctx);

    auto subType = core::Types::approximate(ctx, sub, constr);
    subType = core::Types::resultTypeAsSeenFrom(ctx, subType, subOwner, subOwner, subSelfTypeArgs);
    auto superType = core::Types::approximate(ctx, super, constr);
    superType = core::Types::resultTypeAsSeenFrom(ctx, superType, superOwner, subOwner, subSelfTypeArgs);

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

// This walks two positional argument lists to ensure that they're compatibly typed (i.e. that every argument in the
// implementing method is either the same or a supertype of the abstract or overridable definition)
void matchPositional(const core::Context ctx, core::TypeConstraint &constr,
                     absl::InlinedVector<reference_wrapper<const core::ArgInfo>, 4> &superArgs,
                     core::MethodRef superMethod,
                     absl::InlinedVector<reference_wrapper<const core::ArgInfo>, 4> &methodArgs,
                     core::MethodRef method) {
    auto idx = 0;
    auto maxLen = min(superArgs.size(), methodArgs.size());

    while (idx < maxLen) {
        auto &superArgType = superArgs[idx].get().type;
        auto &methodArgType = methodArgs[idx].get().type;

        core::ErrorSection::Collector errorDetailsCollector;
        if (!checkSubtype(ctx, constr, methodArgType, method, superArgType, superMethod, core::Polarity::Negative,
                          errorDetailsCollector)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                            methodArgs[idx].get().show(ctx), methodArgType.show(ctx), superMethodKind(ctx, superMethod),
                            superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(),
                               "The super method parameter `{}` was declared here with type `{}`",
                               superArgs[idx].get().show(ctx), superArgType.show(ctx));
                e.addErrorNote(
                    "A parameter's type must be a supertype of the same parameter's type on the super method.");
                core::TypeErrorDiagnostics::explainTypeMismatch(ctx, e, errorDetailsCollector, superArgType,
                                                                methodArgType);
            }
        }
        idx++;
    }
}

// Ensure that two argument lists are compatible in shape and type
void validateCompatibleOverride(const core::Context ctx, core::MethodRef superMethod, core::MethodRef method) {
    if (method.data(ctx)->flags.isOverloaded) {
        // Don't try to check overloaded methods; It's not immediately clear how
        // to match overloads against their superclass definitions. Since we
        // Only permit overloading in the stdlib for now, this is no great loss.
        return;
    }

    if (superMethod.data(ctx)->flags.isGenericMethod != method.data(ctx)->flags.isGenericMethod &&
        method.data(ctx)->hasSig()) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            if (superMethod.data(ctx)->flags.isGenericMethod) {
                e.setHeader("{} method `{}` must declare the same number of type parameters as the base method",
                            implementationOf(ctx, superMethod), superMethod.show(ctx));
            } else {
                e.setHeader("{} method `{}` must not declare type parameters", implementationOf(ctx, superMethod),
                            superMethod.show(ctx));
            }
            e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
        }
        return;
    }

    unique_ptr<core::TypeConstraint> _constr;
    auto *constr = &core::TypeConstraint::EmptyFrozenConstraint;
    if (method.data(ctx)->flags.isGenericMethod) {
        ENFORCE(superMethod.data(ctx)->flags.isGenericMethod);
        const auto &methodTypeArguments = method.data(ctx)->typeArguments();
        const auto &superMethodTypeArguments = superMethod.data(ctx)->typeArguments();
        if (methodTypeArguments.size() != superMethodTypeArguments.size()) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("{} method `{}` must declare the same number of type parameters as the base method",
                            implementationOf(ctx, superMethod), superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
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
        for (size_t i = 0; i < methodTypeArguments.size(); i++) {
            auto typeArgument = methodTypeArguments[i];
            auto superTypeArgument = superMethodTypeArguments[i];

            constr->rememberIsSubtype(ctx, typeArgument.data(ctx)->resultType,
                                      core::make_type<core::SelfTypeParam>(superTypeArgument));
            constr->rememberIsSubtype(ctx, core::make_type<core::SelfTypeParam>(typeArgument),
                                      superTypeArgument.data(ctx)->resultType);

            constr->rememberIsSubtype(ctx, core::make_type<core::SelfTypeParam>(typeArgument),
                                      typeArgument.data(ctx)->resultType);
            constr->rememberIsSubtype(ctx, superTypeArgument.data(ctx)->resultType,
                                      core::make_type<core::SelfTypeParam>(superTypeArgument));
        }
    }

    auto left = decomposeSignature(ctx, superMethod);
    auto right = decomposeSignature(ctx, method);

    if (!right.pos.rest) {
        auto leftPos = left.pos.required.size() + left.pos.optional.size();
        auto rightPos = right.pos.required.size() + right.pos.optional.size();
        if (leftPos > rightPos) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("{} method `{}` must accept at least `{}` positional arguments",
                            implementationOf(ctx, superMethod), superMethod.show(ctx), leftPos);
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            }
        }
    }

    if (auto leftRest = left.pos.rest) {
        if (!right.pos.rest) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("{} method `{}` must accept *`{}`", implementationOf(ctx, superMethod),
                            superMethod.show(ctx), leftRest->get().show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            }
        }
    }

    if (right.pos.required.size() > left.pos.required.size()) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("{} method `{}` must accept no more than `{}` required argument(s)",
                        implementationOf(ctx, superMethod), superMethod.show(ctx), left.pos.required.size());
            e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
        }
    }

    // match types of required positional arguments
    matchPositional(ctx, *constr, left.pos.required, superMethod, right.pos.required, method);
    // match types of optional positional arguments
    matchPositional(ctx, *constr, left.pos.optional, superMethod, right.pos.optional, method);

    if (!right.kw.rest) {
        for (auto req : left.kw.required) {
            auto corresponding =
                absl::c_find_if(right.kw.required, [&](const auto &r) { return r.get().name == req.get().name; });

            auto hasCorrespondingRequired = corresponding != right.kw.required.end();
            if (!hasCorrespondingRequired) {
                corresponding =
                    absl::c_find_if(right.kw.optional, [&](const auto &r) { return r.get().name == req.get().name; });
            }

            auto hasCorrespondingOptional = corresponding != right.kw.optional.end();

            // if there is a corresponding parameter, make sure it has the right type
            if (hasCorrespondingRequired || hasCorrespondingOptional) {
                core::ErrorSection::Collector errorDetailsCollector;
                if (!checkSubtype(ctx, *constr, corresponding->get().type, method, req.get().type, superMethod,
                                  core::Polarity::Negative, errorDetailsCollector)) {
                    if (auto e =
                            ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                        e.setHeader("Keyword parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                                    corresponding->get().show(ctx), corresponding->get().type.show(ctx),
                                    superMethodKind(ctx, superMethod), superMethod.show(ctx));
                        e.addErrorLine(superMethod.data(ctx)->loc(),
                                       "The corresponding parameter `{}` was declared here with type `{}`",
                                       req.get().show(ctx), req.get().type.show(ctx));
                        e.addErrorNote(
                            "A parameter's type must be a supertype of the same parameter's type on the super method.");
                        core::TypeErrorDiagnostics::explainTypeMismatch(ctx, e, errorDetailsCollector, req.get().type,
                                                                        corresponding->get().type);
                    }
                }
            } else {
                if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                    e.setHeader("{} method `{}` is missing required keyword argument `{}`",
                                implementationOf(ctx, superMethod), superMethod.show(ctx), req.get().name.show(ctx));
                    e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
                }
            }
        }

        // make sure that optional parameters expect a compatible type, as well
        for (auto opt : left.kw.optional) {
            auto corresponding =
                absl::c_find_if(right.kw.optional, [&](const auto &r) { return r.get().name == opt.get().name; });

            // if there is a corresponding parameter, make sure it has the right type
            if (corresponding != right.kw.optional.end()) {
                core::ErrorSection::Collector errorDetailsCollector;
                if (!checkSubtype(ctx, *constr, corresponding->get().type, method, opt.get().type, superMethod,
                                  core::Polarity::Negative, errorDetailsCollector)) {
                    if (auto e =
                            ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                        e.setHeader("Keyword parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                                    corresponding->get().show(ctx), corresponding->get().type.show(ctx),
                                    superMethodKind(ctx, superMethod), superMethod.show(ctx));
                        e.addErrorLine(superMethod.data(ctx)->loc(),
                                       "The super method parameter `{}` was declared here with type `{}`",
                                       opt.get().show(ctx), opt.get().type.show(ctx));
                        e.addErrorNote(
                            "A parameter's type must be a supertype of the same parameter's type on the super method.");
                        core::TypeErrorDiagnostics::explainTypeMismatch(ctx, e, errorDetailsCollector, opt.get().type,
                                                                        corresponding->get().type);
                    }
                }
            } else if (absl::c_any_of(right.kw.required,
                                      [&](const auto &r) { return r.get().name == opt.get().name; })) {
                if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                    e.setHeader("{} method `{}` must redeclare keyword parameter `{}` as optional",
                                implementationOf(ctx, superMethod), superMethod.show(ctx), opt.get().name.show(ctx));
                    // Show the superMethod loc (declLoc) so the error message includes the default value
                    e.addErrorLine(superMethod.data(ctx)->loc(),
                                   "The optional super method parameter `{}` was declared here",
                                   opt.get().name.show(ctx));
                }
            } else {
                if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                    e.setHeader("{} method `{}` must accept optional keyword parameter `{}`",
                                implementationOf(ctx, superMethod), superMethod.show(ctx), opt.get().name.show(ctx));
                    // Show the superMethod loc (declLoc) so the error message includes the default value
                    e.addErrorLine(superMethod.data(ctx)->loc(),
                                   "The optional super method parameter `{}` was declared here",
                                   opt.get().name.show(ctx));
                }
            }
        }
    }

    if (auto leftRest = left.kw.rest) {
        core::ErrorSection::Collector errorDetailsCollector;
        if (!right.kw.rest) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("{} method `{}` must accept **`{}`", implementationOf(ctx, superMethod),
                            superMethod.show(ctx), leftRest->get().show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            }
        } else if (!checkSubtype(ctx, *constr, right.kw.rest->get().type, method, leftRest->get().type, superMethod,
                                 core::Polarity::Negative, errorDetailsCollector)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Parameter **`{}` of type `{}` not compatible with type of {} method `{}`",
                            right.kw.rest->get().show(ctx), right.kw.rest->get().type.show(ctx),
                            superMethodKind(ctx, superMethod), superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(),
                               "The super method parameter **`{}` was declared here with type `{}`",
                               left.kw.rest->get().show(ctx), left.kw.rest->get().type.show(ctx));
                e.addErrorNote(
                    "A parameter's type must be a supertype of the same parameter's type on the super method.");
                core::TypeErrorDiagnostics::explainTypeMismatch(ctx, e, errorDetailsCollector, left.kw.rest->get().type,
                                                                right.kw.rest->get().type);
            }
        }
    }

    for (auto extra : right.kw.required) {
        if (absl::c_any_of(left.kw.required, [&](const auto &l) { return l.get().name == extra.get().name; })) {
            continue;
        }
        if (absl::c_any_of(left.kw.optional, [&](const auto &l) { return l.get().name == extra.get().name; })) {
            // We would have already reported a more informative error above.
            continue;
        }
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("{} method `{}` contains extra required keyword argument `{}`",
                        implementationOf(ctx, superMethod), superMethod.show(ctx), extra.get().name.toString(ctx));
            e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
        }
    }

    if (!left.syntheticBlk && right.syntheticBlk) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("{} method `{}` must explicitly name a block argument", implementationOf(ctx, superMethod),
                        superMethod.show(ctx));
            e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
        }
    } else {
        const auto &methodBlkArg = method.data(ctx)->arguments.back();
        const auto &superMethodBlkArg = superMethod.data(ctx)->arguments.back();

        core::ErrorSection::Collector errorDetailsCollector;
        if (!checkSubtype(ctx, *constr, methodBlkArg.type, method, superMethodBlkArg.type, superMethod,
                          core::Polarity::Negative, errorDetailsCollector)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Block parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                            methodBlkArg.argumentName(ctx), methodBlkArg.type.show(ctx),
                            superMethodKind(ctx, superMethod), superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(),
                               "The super method parameter `{}` was declared here with type `{}`",
                               superMethodBlkArg.show(ctx), superMethodBlkArg.type.show(ctx));
                e.addErrorNote(
                    "A parameter's type must be a supertype of the same parameter's type on the super method.");
                core::TypeErrorDiagnostics::explainTypeMismatch(ctx, e, errorDetailsCollector, superMethodBlkArg.type,
                                                                methodBlkArg.type);
            }
        }
    }

    {
        // make sure the return types are compatible

        auto superReturn = superMethod.data(ctx)->resultType;
        if (superReturn == core::Types::void_()) {
            // Mimics how `.void` methods are handled in cfg::Return case of environment.cc
            superReturn = core::Types::top();
        }

        auto &methodReturn = method.data(ctx)->resultType;
        // Don't have to do the void -> top trick, because if parent is top, the child method return
        // type can be whatever. And if parent is not top, then neither void nor T.anything in the
        // child return will be compatible with the parent, so we may as well keep it as `void` for
        // the sake of showing an error message in the terms that the user wrote ("where did this
        // T.anything come from? I wrote .void").

        core::ErrorSection::Collector errorDetailsCollector;
        if (!checkSubtype(ctx, *constr, methodReturn, method, superReturn, superMethod, core::Polarity::Positive,
                          errorDetailsCollector)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                auto methodReturnShow = methodReturn == core::Types::void_() ? "void" : methodReturn.show(ctx);
                e.setHeader("Return type `{}` does not match return type of {} method `{}`", methodReturnShow,
                            superMethodKind(ctx, superMethod), superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Super method defined here with return type `{}`",
                               superReturn.show(ctx));
                e.addErrorNote("A method's return type must be a subtype of the return type on the super method.");
                core::TypeErrorDiagnostics::explainTypeMismatch(ctx, e, errorDetailsCollector, superReturn,
                                                                methodReturn);
            }
        }
    }
}

optional<core::AutocorrectSuggestion>
constructOverrideAutocorrect(const core::Context ctx, const ast::ExpressionPtr &tree, core::MethodRef method) {
    auto methodLoc = method.data(ctx)->loc();
    auto parsedSig = sig_finder::SigFinder::findSignature(ctx, tree, methodLoc.copyWithZeroLength());
    if (!parsedSig.has_value()) {
        return nullopt;
    }

    ast::Block *block = parsedSig->origSend->block();
    if (!block) {
        return nullopt;
    }

    auto *blockBody = ast::cast_tree<ast::Send>(block->body);
    ENFORCE(blockBody != nullptr);
    auto insertLoc = ctx.locAt(blockBody->loc.copyWithZeroLength());

    vector<core::AutocorrectSuggestion::Edit> edits;
    edits.emplace_back(core::AutocorrectSuggestion::Edit{insertLoc, "override."});
    return core::AutocorrectSuggestion{
        fmt::format("Add `{}` to `{}` sig", "override", method.data(ctx)->name.show(ctx)),
        std::move(edits),
    };
}

void validateOverriding(const core::Context ctx, const ast::ExpressionPtr &tree, core::MethodRef method) {
    auto klass = method.data(ctx)->owner;
    auto name = method.data(ctx)->name;
    auto klassData = klass.data(ctx);
    InlinedVector<core::MethodRef, 4> overriddenMethods;

    // Matches the behavior of the runtime checks
    // NOTE(jez): I don't think this check makes all that much sense, but I haven't thought about it.
    // We already deleted the corresponding check for `private`, and may want to revisit this, too.
    if (klassData->flags.isInterface && method.data(ctx)->flags.isProtected) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::NonPublicAbstract)) {
            e.setHeader("Interface method `{}` cannot be protected", method.show(ctx));
        }
    }

    if (method.data(ctx)->flags.isAbstract && klassData->isSingletonClass(ctx)) {
        auto attached = klassData->attachedClass(ctx);
        if (attached.exists() && attached.data(ctx)->isModule()) {
            if (auto e =
                    ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::StaticAbstractModuleMethod)) {
                e.setHeader("Static methods in a module cannot be abstract");
            }
        }
    }

    if (klassData->superClass().exists()) {
        auto superMethod = klassData->superClass().data(ctx)->findMethodTransitive(ctx, name);
        if (superMethod.exists()) {
            overriddenMethods.emplace_back(superMethod);
        }
    }
    for (const auto &mixin : klassData->mixins()) {
        auto superMethod = mixin.data(ctx)->findMethod(ctx, name);
        if (superMethod.exists()) {
            overriddenMethods.emplace_back(superMethod);
        }
    }

    if (overriddenMethods.size() == 0 && method.data(ctx)->flags.isOverride &&
        !method.data(ctx)->flags.isIncompatibleOverride) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Method `{}` is marked `{}` but does not override anything", method.show(ctx), "override");
        }
    }

    // we don't raise override errors if the method implements an abstract method, which means we need to know ahead of
    // time whether any parent methods are abstract
    auto anyIsInterface = absl::c_any_of(overriddenMethods, [&](auto &m) { return m.data(ctx)->flags.isAbstract; });
    for (const auto &overriddenMethod : overriddenMethods) {
        if (overriddenMethod.data(ctx)->flags.isFinal) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::OverridesFinal)) {
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
            auto methodLoc = method.data(ctx)->loc();

            if (auto e = ctx.state.beginError(methodLoc, core::errors::Resolver::UndeclaredOverride)) {
                e.setHeader("Method `{}` overrides an overridable method `{}` but is not declared with `{}`",
                            method.show(ctx), overriddenMethod.show(ctx), "override.");
                e.addErrorLine(overriddenMethod.data(ctx)->loc(), "defined here");

                auto potentialAutocorrect = constructOverrideAutocorrect(ctx, tree, method);
                if (potentialAutocorrect.has_value()) {
                    e.addAutocorrect(std::move(*potentialAutocorrect));
                }
            }
        }
        if (!method.data(ctx)->flags.isOverride && !method.data(ctx)->flags.isAbstract && method.data(ctx)->hasSig() &&
            overriddenMethod.data(ctx)->flags.isAbstract && overriddenMethod.data(ctx)->hasSig() &&
            !method.data(ctx)->flags.isRewriterSynthesized && !isRBI) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::UndeclaredOverride)) {
                e.setHeader("Method `{}` implements an abstract method `{}` but is not declared with `{}`",
                            method.show(ctx), overriddenMethod.show(ctx), "override.");
                e.addErrorLine(overriddenMethod.data(ctx)->loc(), "defined here");

                auto potentialAutocorrect = constructOverrideAutocorrect(ctx, tree, method);
                if (potentialAutocorrect.has_value()) {
                    e.addAutocorrect(std::move(*potentialAutocorrect));
                }
            }
        }
        if ((overriddenMethod.data(ctx)->flags.isAbstract || overriddenMethod.data(ctx)->flags.isOverridable ||
             (overriddenMethod.data(ctx)->hasSig() && method.data(ctx)->flags.isOverride)) &&
            !method.data(ctx)->flags.isIncompatibleOverride && !isRBI &&
            !method.data(ctx)->flags.isRewriterSynthesized &&
            overriddenMethod != core::Symbols::BasicObject_initialize()) {
            // We only ignore BasicObject#initialize for backwards compatibility.
            // One day, we may want to build something like overridable(allow_incompatible: true)
            // and mark certain methods in the standard library as possible to be overridden incompatibly,
            // without needing to write `override(allow_incompatible: true)`.
            validateCompatibleOverride(ctx, overriddenMethod, method);
        }
    }
}

core::LocOffsets getAncestorLoc(const core::GlobalState &gs, const ast::ClassDef &classDef,
                                const core::ClassOrModuleRef ancestor) {
    for (const auto &anc : classDef.ancestors) {
        const auto ancConst = ast::cast_tree<ast::ConstantLit>(anc);
        if (ancConst != nullptr && ancConst->symbol.dealias(gs) == ancestor) {
            return anc.loc();
        }
    }
    for (const auto &anc : classDef.singletonAncestors) {
        const auto ancConst = ast::cast_tree<ast::ConstantLit>(anc);
        if (ancConst != nullptr && ancConst->symbol.dealias(gs) == ancestor) {
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

void validateFinalMethodHelper(core::Context ctx, const core::ClassOrModuleRef klass, const ast::ExpressionPtr &tree,
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
            auto parsedSig = sig_finder::SigFinder::findSignature(ctx, tree, queryLoc);

            if (parsedSig.has_value() && parsedSig->origSend->funLoc.exists()) {
                auto funLoc = ctx.locAt(parsedSig->origSend->funLoc);
                e.replaceWith("Mark it as `sig(:final)`", funLoc, "sig(:final)");
            }
        }
    }
}

void validateFinal(core::Context ctx, const core::ClassOrModuleRef klass, const ast::ExpressionPtr &tree) {
    const ast::ClassDef &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
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
    validateFinalMethodHelper(ctx, klass, tree, klass);
    const auto singleton = klass.data(ctx)->lookupSingletonClass(ctx);
    validateFinalAncestorHelper(ctx, singleton, classDef, klass, "extended");
    validateFinalMethodHelper(ctx, singleton, tree, klass);
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

    if (auto *cnst = ast::cast_tree<ast::ConstantLit>(classDef.ancestors.front())) {
        if (cnst->symbol == core::Symbols::todo()) {
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

        if ((isSingletonClass && typeArity > 1) || (!isSingletonClass && typeArity > 0)) {
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
    ValidateWalk(const ast::ExpressionPtr &tree) : tree(tree) {}

private:
    const ast::ExpressionPtr &tree;
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
                if (method->flags.isAbstract &&
                    // Ignore mangle renames, because users shouldn't have to create *another*
                    // mangle rename error in order to implement such an abstract method.
                    !(method->name.kind() == core::NameKind::UNIQUE &&
                      method->name.dataUnique(gs)->uniqueNameKind == core::UniqueNameKind::MangleRename)) {
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
        auto resultType = abstractMethodRef.data(gs)->resultType;
        auto methodDefinition = core::source_generator::prettyTypeForMethod(gs, abstractMethodRef, nullptr, resultType,
                                                                            nullptr, showOptions);

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
            errorBuilder.setHeader("Missing definitions for abstract methods");
        } else {
            errorBuilder.setHeader("Missing definition for abstract method `{}`",
                                   missingAbstractMethods.front().show(ctx));
        }

        auto classOrModuleDeclaredAt = ctx.locAt(classDef.declLoc);
        auto classOrModuleEndsAt = ctx.locAt(classDef.loc.copyEndWithZeroLength());
        auto hasSingleLineDefinition =
            classOrModuleDeclaredAt.position(ctx).first.line == classOrModuleEndsAt.position(ctx).second.line;

        auto hasEmptyBody = classDef.rhs.empty();
        if (classDef.rhs.size() == 1) {
            if (auto *mdef = ast::cast_tree<ast::MethodDef>(classDef.rhs[0])) {
                hasEmptyBody = ast::isa_tree<ast::EmptyTree>(mdef->rhs);
            }
        }

        auto [endLoc, indentLength] = classOrModuleEndsAt.findStartOfLine(ctx);
        string classOrModuleIndent(indentLength, ' ');
        auto insertAt = endLoc.adjust(ctx, -indentLength, 0);

        vector<core::AutocorrectSuggestion::Edit> edits;
        bool singleLineAndNoBody = hasSingleLineDefinition && hasEmptyBody;
        if (singleLineAndNoBody) {
            // First, break the class/module definition up onto multiple lines.
            auto endRange = classOrModuleDeclaredAt.copyEndWithZeroLength().join(classOrModuleEndsAt);
            edits.emplace_back(
                core::AutocorrectSuggestion::Edit{endRange, fmt::format("\n{}end", classOrModuleIndent)});

            // Then, modify our insertion strategy such that we add new methods to the top of the class/module
            // body rather than the bottom. This is a trick to ensure that we put the new methods within the new
            // class/module body that we just created.
            insertAt = classOrModuleDeclaredAt.copyEndWithZeroLength();
        }

        for (auto proto : missingAbstractMethods) {
            errorBuilder.addErrorLine(proto.data(ctx)->loc(), "`{}` defined here", proto.data(ctx)->name.show(ctx));

            if (hasSingleLineDefinition && !hasEmptyBody) {
                // We don't suggest autocorrects for this case because we cannot say for sure how the
                // class/module has been declared, and so we also can't determine how to modify it.
                continue;
            }

            auto indentedMethodDefinition = defineInheritedAbstractMethod(ctx, sym, proto, classOrModuleIndent);
            if (singleLineAndNoBody) {
                edits.emplace_back(insertAt, fmt::format("\n{}", indentedMethodDefinition));
            } else {
                edits.emplace_back(insertAt, fmt::format("{}\n{}", classOrModuleIndent, indentedMethodDefinition));
            }
        }

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

            auto concreteMethodRef = sym.data(ctx)->findConcreteMethodTransitive(ctx, proto.data(ctx)->name);
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
    void preTransformClassDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        auto sym = classDef.symbol;
        auto singleton = sym.data(ctx)->lookupSingletonClass(ctx);
        validateTStructNotGrandparent(ctx, sym);
        if (!sym.data(ctx)->isSingletonClass(ctx)) {
            // Only validateAbstract for this class if we haven't already (we already have if this
            // is a `class << self` ClassDef)
            validateAbstract(ctx, sym, classDef);

            if (ctx.state.requiresAncestorEnabled) {
                validateRequiredAncestors(ctx, sym);
            }
        }
        validateAbstract(ctx, singleton, classDef);
        validateFinal(ctx, sym, tree);
        validateSealed(ctx, sym, classDef);
        validateSuperClass(ctx, sym, classDef);

        if (ctx.state.requiresAncestorEnabled) {
            validateRequiredAncestors(ctx, singleton);
        }
    }

    void preTransformMethodDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        auto methodData = methodDef.symbol.data(ctx);
        auto ownerData = methodData->owner.data(ctx);

        if (methodData->locs().empty()) {
            Exception::raise("method has no locs! ctx.file=\"{}\" method=\"{}\"", ctx.file.data(ctx).path(),
                             methodDef.symbol.show(ctx));
        }
        if (!methodData->loc().file().exists()) {
            Exception::raise("file for method does not exist! ctx.file=\"{}\" method=\"{}\"", ctx.file.data(ctx).path(),
                             methodDef.symbol.show(ctx));
        }

        // Only perform this check if this isn't a module from the stdlib, and
        // if there are type members in the owning context.
        // NOTE: We're skipping variance checks on the stdlib right now, as
        // Array and Hash are defined with their parameters as covariant, and as
        // a result most of their methods would fail this check.
        if (!methodData->loc().file().data(ctx).isStdlib() && !ownerData->typeMembers().empty()) {
            variance::validateMethodVariance(ctx, methodDef.symbol);
        }

        // See the comment in `VarianceValidator::validateMethod` for an explanation of why we don't
        // need to check types on instance variables.

        validateOverriding(ctx, this->tree, methodDef.symbol);
    }

    void postTransformSend(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);
        if (send.fun != core::Names::new_()) {
            return;
        }

        auto *id = ast::cast_tree<ast::ConstantLit>(send.recv);
        if (id == nullptr || !id->symbol.exists() || !id->symbol.isClassOrModule()) {
            return;
        }

        auto symbol = id->symbol.asClassOrModuleRef().data(ctx);
        if (!symbol->flags.isAbstract) {
            return;
        }

        auto singletonClass = symbol->lookupSingletonClass(ctx.state);
        if (!singletonClass.exists()) {
            return;
        }

        auto method_new = singletonClass.data(ctx)->findMethodTransitive(ctx.state, core::Names::new_());
        // If the .new method we find is owned by Class, that means
        // there was no user defined .new method, which warrants an error.
        if (method_new.data(ctx)->owner == core::Symbols::Class()) {
            if (auto e = ctx.beginError(send.loc, core::errors::Resolver::AbstractClassInstantiated)) {
                auto symbolName = id->symbol.show(ctx);
                e.setHeader("Attempt to instantiate abstract class `{}`", symbolName);
                e.addErrorLine(id->symbol.loc(ctx), "`{}` defined here", symbolName);
            }
        }
    }
};
} // namespace

ast::ParsedFile runOne(core::Context ctx, ast::ParsedFile tree) {
    Timer timeit(ctx.state.tracer(), "validateSymbols");

    ValidateWalk validate(tree.tree);
    ast::TreeWalk::apply(ctx, validate, tree.tree);
    return tree;
}

} // namespace sorbet::definition_validator
