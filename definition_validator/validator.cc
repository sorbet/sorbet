#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/Timer.h"
#include "core/core.h"
#include "core/errors/resolver.h"

#include "absl/algorithm/container.h"

#include "definition_validator/variance.h"
#include "main/sig_finder/sig_finder.h"

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

// This returns true if `sub` is a subtype of `super`, but it also returns true if either one is `nullptr` or if either
// one is not fully defined. This is really just a useful helper function for this module: do not use it elsewhere.
bool checkSubtype(const core::Context ctx, const core::TypePtr &sub, const core::TypePtr &super) {
    if (sub == nullptr || super == nullptr) {
        return true;
    }

    // type-checking these in the presence of generic type parameters is tricky, so for now we're going to punt on
    // it. TODO: build up the machinery to type-check in the presence of type parameters!
    if (!super.isFullyDefined() || !sub.isFullyDefined()) {
        return true;
    }

    return core::Types::isSubType(ctx, sub, super);
}

string supermethodKind(const core::Context ctx, core::MethodRef method) {
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
void matchPositional(const core::Context ctx, absl::InlinedVector<reference_wrapper<const core::ArgInfo>, 4> &superArgs,
                     core::MethodRef superMethod,
                     absl::InlinedVector<reference_wrapper<const core::ArgInfo>, 4> &methodArgs,
                     core::MethodRef method) {
    auto idx = 0;
    auto maxLen = min(superArgs.size(), methodArgs.size());

    while (idx < maxLen) {
        auto superArgType = superArgs[idx].get().type;
        auto methodArgType = methodArgs[idx].get().type;

        if (!checkSubtype(ctx, superArgType, methodArgType)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                            methodArgs[idx].get().show(ctx), methodArgType.show(ctx), supermethodKind(ctx, superMethod),
                            superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(),
                               "The super method parameter `{}` was declared here with type `{}`",
                               superArgs[idx].get().show(ctx), superArgType.show(ctx));
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
    matchPositional(ctx, left.pos.required, superMethod, right.pos.required, method);
    // match types of optional positional arguments
    matchPositional(ctx, left.pos.optional, superMethod, right.pos.optional, method);

    if (!right.kw.rest) {
        for (auto req : left.kw.required) {
            auto corresponding =
                absl::c_find_if(right.kw.required, [&](const auto &r) { return r.get().name == req.get().name; });
            if (corresponding == right.kw.required.end()) {
                corresponding =
                    absl::c_find_if(right.kw.optional, [&](const auto &r) { return r.get().name == req.get().name; });
            }

            // if there is a corresponding parameter, make sure it has the right type
            if (corresponding != right.kw.required.end() && corresponding != right.kw.optional.end()) {
                if (!checkSubtype(ctx, req.get().type, corresponding->get().type)) {
                    if (auto e =
                            ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                        e.setHeader("Keyword parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                                    corresponding->get().show(ctx), corresponding->get().type.show(ctx),
                                    supermethodKind(ctx, superMethod), superMethod.show(ctx));
                        e.addErrorLine(superMethod.data(ctx)->loc(),
                                       "The corresponding parameter `{}` was declared here with type `{}`",
                                       req.get().show(ctx), req.get().type.show(ctx));
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
                if (!checkSubtype(ctx, opt.get().type, corresponding->get().type)) {
                    if (auto e =
                            ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                        e.setHeader("Keyword parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                                    corresponding->get().show(ctx), corresponding->get().type.show(ctx),
                                    supermethodKind(ctx, superMethod), superMethod.show(ctx));
                        e.addErrorLine(superMethod.data(ctx)->loc(),
                                       "The super method parameter `{}` was declared here with type `{}`",
                                       opt.get().show(ctx), opt.get().type.show(ctx));
                    }
                }
            }
        }
    }

    if (auto leftRest = left.kw.rest) {
        if (!right.kw.rest) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("{} method `{}` must accept **`{}`", implementationOf(ctx, superMethod),
                            superMethod.show(ctx), leftRest->get().show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            }
        } else if (!checkSubtype(ctx, leftRest->get().type, right.kw.rest->get().type)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Parameter **`{}` of type `{}` not compatible with type of {} method `{}`",
                            right.kw.rest->get().show(ctx), right.kw.rest->get().type.show(ctx),
                            supermethodKind(ctx, superMethod), superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(),
                               "The super method parameter **`{}` was declared here with type `{}`",
                               left.kw.rest->get().show(ctx), left.kw.rest->get().type.show(ctx));
            }
        }
    }

    for (auto extra : right.kw.required) {
        if (absl::c_any_of(left.kw.required, [&](const auto &l) { return l.get().name == extra.get().name; })) {
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
    }

    {
        // make sure the return types are compatible
        auto superReturn = superMethod.data(ctx)->resultType;
        auto methodReturn = method.data(ctx)->resultType;

        if (!checkSubtype(ctx, methodReturn, superReturn)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Return type `{}` does not match return type of {} method `{}`", methodReturn.show(ctx),
                            supermethodKind(ctx, superMethod), superMethod.show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Super method defined here with return type `{}`",
                               superReturn.show(ctx));
            }
        }
    }
}

void validateOverriding(const core::Context ctx, core::MethodRef method) {
    auto klass = method.data(ctx)->owner;
    auto name = method.data(ctx)->name;
    auto klassData = klass.data(ctx);
    InlinedVector<core::MethodRef, 4> overridenMethods;

    // both of these match the behavior of the runtime checks, which will only allow public methods to be defined in
    // interfaces
    if (klassData->flags.isInterface && method.data(ctx)->flags.isPrivate) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::NonPublicAbstract)) {
            e.setHeader("Interface method `{}` cannot be private", method.show(ctx));
        }
    }

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
            overridenMethods.emplace_back(superMethod);
        }
    }
    for (const auto &mixin : klassData->mixins()) {
        auto superMethod = mixin.data(ctx)->findMethod(ctx, name);
        if (superMethod.exists()) {
            overridenMethods.emplace_back(superMethod);
        }
    }

    if (overridenMethods.size() == 0 && method.data(ctx)->flags.isOverride &&
        !method.data(ctx)->flags.isIncompatibleOverride) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Method `{}` is marked `{}` but does not override anything", method.show(ctx), "override");
        }
    }

    // we don't raise override errors if the method implements an abstract method, which means we need to know ahead of
    // time whether any parent methods are abstract
    auto anyIsInterface = absl::c_any_of(overridenMethods, [&](auto &m) { return m.data(ctx)->flags.isAbstract; });
    for (const auto &overridenMethod : overridenMethods) {
        if (overridenMethod.data(ctx)->flags.isFinal) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::OverridesFinal)) {
                e.setHeader("`{}` was declared as final and cannot be overridden by `{}`", overridenMethod.show(ctx),
                            method.show(ctx));
                e.addErrorLine(overridenMethod.data(ctx)->loc(), "original method defined here");
            }
        }
        auto isRBI = absl::c_any_of(method.data(ctx)->locs(), [&](auto &loc) { return loc.file().data(ctx).isRBI(); });
        if (!method.data(ctx)->flags.isOverride && method.data(ctx)->hasSig() &&
            (overridenMethod.data(ctx)->flags.isOverridable || overridenMethod.data(ctx)->flags.isOverride) &&
            !anyIsInterface && overridenMethod.data(ctx)->hasSig() && !method.data(ctx)->flags.isRewriterSynthesized &&
            !isRBI) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::UndeclaredOverride)) {
                e.setHeader("Method `{}` overrides an overridable method `{}` but is not declared with `{}`",
                            method.show(ctx), overridenMethod.show(ctx), "override.");
                e.addErrorLine(overridenMethod.data(ctx)->loc(), "defined here");
            }
        }
        if (!method.data(ctx)->flags.isOverride && method.data(ctx)->hasSig() &&
            overridenMethod.data(ctx)->flags.isAbstract && overridenMethod.data(ctx)->hasSig() &&
            !method.data(ctx)->flags.isRewriterSynthesized && !isRBI) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::UndeclaredOverride)) {
                e.setHeader("Method `{}` implements an abstract method `{}` but is not declared with `{}`",
                            method.show(ctx), overridenMethod.show(ctx), "override.");
                e.addErrorLine(overridenMethod.data(ctx)->loc(), "defined here");
            }
        }
        if ((overridenMethod.data(ctx)->flags.isAbstract || overridenMethod.data(ctx)->flags.isOverridable ||
             (overridenMethod.data(ctx)->hasSig() && method.data(ctx)->flags.isOverride)) &&
            !method.data(ctx)->flags.isIncompatibleOverride && !isRBI &&
            !method.data(ctx)->flags.isRewriterSynthesized) {
            validateCompatibleOverride(ctx, overridenMethod, method);
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

void validateFinalMethodHelper(const core::GlobalState &gs, const core::ClassOrModuleRef klass,
                               const core::ClassOrModuleRef errMsgClass) {
    if (!klass.data(gs)->flags.isFinal) {
        return;
    }
    for (const auto [name, sym] : klass.data(gs)->members()) {
        // We only care about method symbols that exist.
        if (!sym.exists() || !sym.isMethod() ||
            // Method is 'final', and passes the check.
            sym.asMethodRef().data(gs)->flags.isFinal ||
            // <static-init> is a fake method Sorbet synthesizes for typechecking.
            sym.name(gs) == core::Names::staticInit() ||
            // <unresolved-ancestors> is a fake method Sorbet synthesizes to ensure class hierarchy changes in IDE take
            // slow path.
            sym.name(gs) == core::Names::unresolvedAncestors()) {
            continue;
        }
        auto defLoc = sym.loc(gs);
        if (auto e = gs.beginError(defLoc, core::errors::Resolver::FinalModuleNonFinalMethod)) {
            e.setHeader("`{}` was declared as final but its method `{}` was not declared as final",
                        errMsgClass.show(gs), sym.name(gs).show(gs));
            auto sigLoc = sorbet::sig_finder::findSignature(gs, sym);

            if (sigLoc.has_value()) {
                e.replaceWith("Mark it as `sig(:final)`", core::Loc{defLoc.file(), sigLoc.value().sig}, "sig(:final)");
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
    validateFinalMethodHelper(ctx, klass, klass);
    const auto singleton = klass.data(ctx)->lookupSingletonClass(ctx);
    validateFinalAncestorHelper(ctx, singleton, classDef, klass, "extended");
    validateFinalMethodHelper(ctx, singleton, klass);
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

    if (auto e = ctx.state.beginError(core::Loc(sym.data(ctx)->loc().file(), classDef.declLoc),
                                      core::errors::Resolver::NonClassSuperclass)) {
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

    vector<core::Symbol::RequiredAncestor> requiredClasses;
    for (auto ancst : data->requiredAncestorsTransitive(ctx)) {
        if (ancst.symbol.data(ctx)->isClass()) {
            requiredClasses.emplace_back(ancst);
        }

        if (ancst.symbol.data(ctx)->typeArity(ctx) > 0) {
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
private:
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
                if (sym.exists() && sym.isMethod() && sym.asMethodRef().data(gs)->flags.isAbstract) {
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
            e.addErrorLine(parent.data(gs)->loc(), "`{}` is a subclass of `T::Struct`", parentName);
        }
    }

    void validateAbstract(const core::GlobalState &gs, core::ClassOrModuleRef sym) {
        if (sym.data(gs)->flags.isAbstract) {
            return;
        }
        auto loc = sym.data(gs)->loc();
        if (loc.exists() && loc.file().data(gs).isRBI()) {
            return;
        }

        auto &abstract = getAbstractMethods(gs, sym);

        if (abstract.empty()) {
            return;
        }

        for (auto proto : abstract) {
            if (proto.data(gs)->owner == sym) {
                continue;
            }

            auto mem = sym.data(gs)->findConcreteMethodTransitive(gs, proto.data(gs)->name);
            if (!mem.exists()) {
                if (auto e = gs.beginError(loc, core::errors::Resolver::BadAbstractMethod)) {
                    e.setHeader("Missing definition for abstract method `{}`", proto.show(gs));
                    e.addErrorLine(proto.data(gs)->loc(), "defined here");
                }
            }
        }
    }

public:
    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        auto sym = classDef.symbol;
        auto singleton = sym.data(ctx)->lookupSingletonClass(ctx);
        validateTStructNotGrandparent(ctx, sym);
        if (!sym.data(ctx)->isSingletonClass(ctx)) {
            // Only validateAbstract for this class if we haven't already (we already have if this
            // is a `class << self` ClassDef)
            validateAbstract(ctx, sym);

            if (ctx.state.requiresAncestorEnabled) {
                validateRequiredAncestors(ctx, sym);
            }
        }
        validateAbstract(ctx, singleton);
        validateFinal(ctx, sym, classDef);
        validateSealed(ctx, sym, classDef);
        validateSuperClass(ctx, sym, classDef);

        if (ctx.state.requiresAncestorEnabled) {
            validateRequiredAncestors(ctx, singleton);
        }
        return tree;
    }

    ast::ExpressionPtr preTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        auto methodData = methodDef.symbol.data(ctx);
        auto ownerData = methodData->owner.data(ctx);

        // Only perform this check if this isn't a module from the stdlib, and
        // if there are type members in the owning context.
        // NOTE: We're skipping variance checks on the stdlib right now, as
        // Array and Hash are defined with their parameters as covariant, and as
        // a result most of their methods would fail this check.
        if (!methodData->loc().file().data(ctx).isStdlib() && !ownerData->typeMembers().empty()) {
            variance::validateMethodVariance(ctx, methodDef.symbol);
        }

        validateOverriding(ctx, methodDef.symbol);
        return tree;
    }
};
} // namespace

ast::ParsedFile runOne(core::Context ctx, ast::ParsedFile tree) {
    Timer timeit(ctx.state.tracer(), "validateSymbols");

    ValidateWalk validate;
    tree.tree = ast::ShallowMap::apply(ctx, validate, std::move(tree.tree));
    return tree;
}

} // namespace sorbet::definition_validator
