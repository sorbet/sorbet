#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/Timer.h"
#include "core/core.h"
#include "core/errors/resolver.h"

#include "absl/algorithm/container.h"

#include "definition_validator/variance.h"

using namespace std;

namespace sorbet::definition_validator {

struct Signature {
    struct {
        absl::InlinedVector<reference_wrapper<const core::ArgInfo>, 4> required;
        absl::InlinedVector<reference_wrapper<const core::ArgInfo>, 4> optional;
        std::optional<reference_wrapper<const core::ArgInfo>> rest;
    } pos, kw;
    bool syntheticBlk;
} left, right;

Signature decomposeSignature(const core::GlobalState &gs, core::SymbolRef method) {
    Signature sig;
    for (auto &arg : method.data(gs)->arguments()) {
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
bool checkSubtype(const core::Context ctx, core::TypePtr sub, core::TypePtr super) {
    if (sub == nullptr || super == nullptr) {
        return true;
    }

    // type-checking these in the presence of generic type parameters is tricky, so for now we're going to punt on
    // it. TODO: build up the machinery to type-check in the presence of type parameters!
    if (!super->isFullyDefined() || !sub->isFullyDefined()) {
        return true;
    }

    return core::Types::isSubType(ctx, sub, super);
}

string supermethodKind(const core::Context ctx, core::SymbolRef method) {
    auto methodData = method.data(ctx);
    ENFORCE(methodData->isAbstract() || methodData->isOverridable());
    if (methodData->isAbstract()) {
        return "abstract";
    } else {
        return "overridable";
    }
}

// This walks two positional argument lists to ensure that they're compatibly typed (i.e. that every argument in the
// implementing method is either the same or a supertype of the abstract or overridable definition)
void matchPositional(const core::Context ctx, absl::InlinedVector<reference_wrapper<const core::ArgInfo>, 4> &superArgs,
                     core::SymbolRef superMethod,
                     absl::InlinedVector<reference_wrapper<const core::ArgInfo>, 4> &methodArgs,
                     core::SymbolRef method) {
    auto idx = 0;
    auto maxLen = min(superArgs.size(), methodArgs.size());

    while (idx < maxLen) {
        auto superArgType = superArgs[idx].get().type;
        auto methodArgType = methodArgs[idx].get().type;

        if (!checkSubtype(ctx, superArgType, methodArgType)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                            methodArgs[idx].get().show(ctx), methodArgType->show(ctx),
                            supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(),
                               "The super method parameter `{}` was declared here with type `{}`",
                               superArgs[idx].get().show(ctx), superArgType->show(ctx));
            }
        }
        idx++;
    }
}

// Ensure that two argument lists are compatible in shape and type
void validateCompatibleOverride(const core::Context ctx, core::SymbolRef superMethod, core::SymbolRef method) {
    if (method.data(ctx)->isOverloaded()) {
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
                e.setHeader("Implementation of {} method `{}` must accept at least `{}` positional arguments",
                            supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx), leftPos);
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            }
        }
    }

    if (auto leftRest = left.pos.rest) {
        if (!right.pos.rest) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Implementation of {} method `{}` must accept *`{}`", supermethodKind(ctx, superMethod),
                            superMethod.data(ctx)->show(ctx), leftRest->get().show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            }
        }
    }

    if (right.pos.required.size() > left.pos.required.size()) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Implementation of {} method `{}` must accept no more than `{}` required argument(s)",
                        supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx), left.pos.required.size());
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
                                    corresponding->get().show(ctx), corresponding->get().type->show(ctx),
                                    supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx));
                        e.addErrorLine(superMethod.data(ctx)->loc(),
                                       "The corresponding parameter `{}` was declared here with type `{}`",
                                       req.get().show(ctx), req.get().type->show(ctx));
                    }
                }
            } else {
                if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                    e.setHeader("Implementation of {} method `{}` is missing required keyword argument `{}`",
                                supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx),
                                req.get().name.show(ctx));
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
                                    corresponding->get().show(ctx), corresponding->get().type->show(ctx),
                                    supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx));
                        e.addErrorLine(superMethod.data(ctx)->loc(),
                                       "The super method parameter `{}` was declared here with type `{}`",
                                       opt.get().show(ctx), opt.get().type->show(ctx));
                    }
                }
            }
        }
    }

    if (auto leftRest = left.kw.rest) {
        if (!right.kw.rest) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Implementation of {} method `{}` must accept **`{}`", supermethodKind(ctx, superMethod),
                            superMethod.data(ctx)->show(ctx), leftRest->get().show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            }
        } else if (!checkSubtype(ctx, leftRest->get().type, right.kw.rest->get().type)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Parameter **`{}` of type `{}` not compatible with type of {} method `{}`",
                            right.kw.rest->get().show(ctx), right.kw.rest->get().type->show(ctx),
                            supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(),
                               "The super method parameter **`{}` was declared here with type `{}`",
                               left.kw.rest->get().show(ctx), left.kw.rest->get().type->show(ctx));
            }
        }
    }

    for (auto extra : right.kw.required) {
        if (absl::c_any_of(left.kw.required, [&](const auto &l) { return l.get().name == extra.get().name; })) {
            continue;
        }
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Implementation of {} method `{}` contains extra required keyword argument `{}`",
                        supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx),
                        extra.get().name.toString(ctx));
            e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
        }
    }

    if (!left.syntheticBlk && right.syntheticBlk) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Implementation of {} method `{}` must explicitly name a block argument",
                        supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx));
            e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
        }
    }

    {
        // make sure the return types are compatible
        auto superReturn = superMethod.data(ctx)->resultType;
        auto methodReturn = method.data(ctx)->resultType;

        if (!checkSubtype(ctx, methodReturn, superReturn)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Return type `{}` does not match return type of {} method `{}`", methodReturn->show(ctx),
                            supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Super method defined here with return type `{}`",
                               superReturn->show(ctx));
            }
        }
    }
}

void validateOverriding(const core::Context ctx, core::SymbolRef method) {
    auto klass = method.data(ctx)->owner;
    auto name = method.data(ctx)->name;
    ENFORCE(klass.data(ctx)->isClassOrModule());
    auto klassData = klass.data(ctx);
    InlinedVector<core::SymbolRef, 4> overridenMethods;

    // both of these match the behavior of the runtime checks, which will only allow public methods to be defined in
    // interfaces
    if (klassData->isClassOrModuleInterface() && method.data(ctx)->isPrivate()) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::NonPublicAbstract)) {
            e.setHeader("Interface method `{}` cannot be private", method.show(ctx));
        }
    }

    if (klassData->isClassOrModuleInterface() && method.data(ctx)->isProtected()) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::NonPublicAbstract)) {
            e.setHeader("Interface method `{}` cannot be protected", method.show(ctx));
        }
    }

    if (method.data(ctx)->isAbstract() && klassData->isClassOrModule() && klassData->isSingletonClass(ctx)) {
        auto attached = klassData->attachedClass(ctx);
        if (attached.exists() && attached.data(ctx)->isClassOrModuleModule()) {
            if (auto e =
                    ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::StaticAbstractModuleMethod)) {
                e.setHeader("Static methods in a module cannot be abstract");
            }
        }
    }

    if (klassData->superClass().exists()) {
        auto superMethod = klassData->superClass().data(ctx)->findMemberTransitive(ctx, name);
        if (superMethod.exists()) {
            overridenMethods.emplace_back(superMethod);
        }
    }
    for (const auto &mixin : klassData->mixins()) {
        auto superMethod = mixin.data(ctx)->findMember(ctx, name);
        if (superMethod.exists()) {
            overridenMethods.emplace_back(superMethod);
        }
    }

    if (overridenMethods.size() == 0 && method.data(ctx)->isOverride() && !method.data(ctx)->isIncompatibleOverride()) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Method `{}` is marked `{}` but does not override anything", method.data(ctx)->show(ctx),
                        "override");
        }
    }

    // we don't raise override errors if the method implements an abstract method, which means we need to know ahead of
    // time whether any parent methods are abstract
    auto anyIsInterface = absl::c_any_of(overridenMethods, [&](auto &m) { return m.data(ctx)->isAbstract(); });
    for (const auto &overridenMethod : overridenMethods) {
        if (overridenMethod.data(ctx)->isFinalMethod()) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::OverridesFinal)) {
                e.setHeader("`{}` was declared as final and cannot be overridden by `{}`",
                            overridenMethod.data(ctx)->show(ctx), method.data(ctx)->show(ctx));
                e.addErrorLine(overridenMethod.data(ctx)->loc(), "original method defined here");
            }
        }
        auto isRBI = absl::c_any_of(method.data(ctx)->locs(), [&](auto &loc) { return loc.file().data(ctx).isRBI(); });
        if (!method.data(ctx)->isOverride() && method.data(ctx)->hasSig() &&
            overridenMethod.data(ctx)->isOverridable() && !anyIsInterface && overridenMethod.data(ctx)->hasSig() &&
            !method.data(ctx)->isRewriterSynthesized() && !isRBI) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::UndeclaredOverride)) {
                e.setHeader("Method `{}` overrides an overridable method `{}` but is not declared with `{}`",
                            method.data(ctx)->show(ctx), overridenMethod.data(ctx)->show(ctx), "override.");
                e.addErrorLine(overridenMethod.data(ctx)->loc(), "defined here");
            }
        }
        if (!method.data(ctx)->isOverride() && method.data(ctx)->hasSig() && overridenMethod.data(ctx)->isAbstract() &&
            overridenMethod.data(ctx)->hasSig() && !method.data(ctx)->isRewriterSynthesized() && !isRBI) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::UndeclaredOverride)) {
                e.setHeader("Method `{}` implements an abstract method `{}` but is not declared with `{}`",
                            method.data(ctx)->show(ctx), overridenMethod.data(ctx)->show(ctx), "override.");
                e.addErrorLine(overridenMethod.data(ctx)->loc(), "defined here");
            }
        }
        if ((overridenMethod.data(ctx)->isAbstract() || overridenMethod.data(ctx)->isOverridable()) &&
            !method.data(ctx)->isIncompatibleOverride() && !isRBI && !method.data(ctx)->isRewriterSynthesized()) {
            if (overridenMethod.data(ctx)->isFinalMethod()) {
                if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::OverridesFinal)) {
                    e.setHeader("Method overrides a final method `{}`", overridenMethod.data(ctx)->show(ctx));
                    e.addErrorLine(overridenMethod.data(ctx)->loc(), "defined here");
                }
            }
            if ((overridenMethod.data(ctx)->isAbstract() || overridenMethod.data(ctx)->isOverridable()) &&
                !method.data(ctx)->isIncompatibleOverride()) {
                validateCompatibleOverride(ctx, overridenMethod, method);
            }
        }
    }
}

core::LocOffsets getAncestorLoc(const core::GlobalState &gs, const ast::ClassDef *classDef,
                                const core::SymbolRef ancestor) {
    for (const auto &anc : classDef->ancestors) {
        const auto ancConst = ast::cast_tree_const<ast::ConstantLit>(anc);
        if (ancConst != nullptr && ancConst->symbol.data(gs)->dealias(gs) == ancestor) {
            return anc->loc;
        }
    }
    for (const auto &anc : classDef->singletonAncestors) {
        const auto ancConst = ast::cast_tree_const<ast::ConstantLit>(anc);
        if (ancConst != nullptr && ancConst->symbol.data(gs)->dealias(gs) == ancestor) {
            return anc->loc;
        }
    }
    // give up
    return classDef->loc;
}

void validateFinalAncestorHelper(core::Context ctx, const core::SymbolRef klass, const ast::ClassDef *classDef,
                                 const core::SymbolRef errMsgClass, const string_view verb) {
    for (const auto &mixin : klass.data(ctx)->mixins()) {
        if (!mixin.data(ctx)->isClassOrModuleFinal()) {
            continue;
        }
        if (auto e = ctx.beginError(getAncestorLoc(ctx, classDef, mixin), core::errors::Resolver::FinalAncestor)) {
            e.setHeader("`{}` was declared as final and cannot be {} in `{}`", mixin.data(ctx)->show(ctx), verb,
                        errMsgClass.data(ctx)->show(ctx));
            e.addErrorLine(mixin.data(ctx)->loc(), "`{}` defined here", mixin.data(ctx)->show(ctx));
        }
    }
}

void validateFinalMethodHelper(const core::GlobalState &gs, const core::SymbolRef klass,
                               const core::SymbolRef errMsgClass) {
    if (!klass.data(gs)->isClassOrModuleFinal()) {
        return;
    }
    for (const auto [name, sym] : klass.data(gs)->members()) {
        // We only care about method symbols that exist.
        if (!sym.exists() || !sym.data(gs)->isMethod() ||
            // Method is 'final', and passes the check.
            sym.data(gs)->isFinalMethod() ||
            // <static-init> is a fake method Sorbet synthesizes for typechecking.
            sym.data(gs)->name == core::Names::staticInit() ||
            // <unresolved-ancestors> is a fake method Sorbet synthesizes to ensure class hierarchy changes in IDE take
            // slow path.
            sym.data(gs)->name == core::Names::unresolvedAncestors()) {
            continue;
        }
        if (auto e = gs.beginError(sym.data(gs)->loc(), core::errors::Resolver::FinalModuleNonFinalMethod)) {
            e.setHeader("`{}` was declared as final but its method `{}` was not declared as final",
                        errMsgClass.data(gs)->show(gs), sym.data(gs)->name.show(gs));
        }
    }
}

void validateFinal(core::Context ctx, const core::SymbolRef klass, const ast::ClassDef *classDef) {
    const auto superClass = klass.data(ctx)->superClass();
    if (superClass.exists() && superClass.data(ctx)->isClassOrModuleFinal()) {
        if (auto e = ctx.beginError(getAncestorLoc(ctx, classDef, superClass), core::errors::Resolver::FinalAncestor)) {
            e.setHeader("`{}` was declared as final and cannot be inherited by `{}`", superClass.data(ctx)->show(ctx),
                        klass.data(ctx)->show(ctx));
            e.addErrorLine(superClass.data(ctx)->loc(), "`{}` defined here", superClass.data(ctx)->show(ctx));
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
core::FileRef bestNonRBIFile(core::Context ctx, const core::SymbolRef klass) {
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

void validateSealedAncestorHelper(core::Context ctx, const core::SymbolRef klass, const ast::ClassDef *classDef,
                                  const core::SymbolRef errMsgClass, const string_view verb) {
    auto klassFile = bestNonRBIFile(ctx, klass);
    for (const auto &mixin : klass.data(ctx)->mixins()) {
        if (!mixin.data(ctx)->isClassOrModuleSealed()) {
            continue;
        }
        // TODO(jez) sealedLocs is actually always one loc. We should add an ENFORCE or error message for this.
        if (absl::c_any_of(mixin.data(ctx)->sealedLocs(ctx),
                           [klassFile](auto loc) { return loc.file() == klassFile; })) {
            continue;
        }
        if (auto e = ctx.beginError(getAncestorLoc(ctx, classDef, mixin), core::errors::Resolver::SealedAncestor)) {
            e.setHeader("`{}` is sealed and cannot be {} in `{}`", mixin.data(ctx)->show(ctx), verb,
                        errMsgClass.data(ctx)->show(ctx));
            for (auto loc : mixin.data(ctx)->sealedLocs(ctx)) {
                e.addErrorLine(loc, "`{}` was marked sealed and can only be {} in this file",
                               mixin.data(ctx)->show(ctx), verb);
            }
        }
    }
}

void validateSealed(core::Context ctx, const core::SymbolRef klass, const ast::ClassDef *classDef) {
    const auto superClass = klass.data(ctx)->superClass();
    auto file = bestNonRBIFile(ctx, klass);
    if (superClass.exists() && superClass.data(ctx)->isClassOrModuleSealed() &&
        !absl::c_any_of(superClass.data(ctx)->sealedLocs(ctx), [file](auto loc) { return loc.file() == file; })) {
        if (auto e =
                ctx.beginError(getAncestorLoc(ctx, classDef, superClass), core::errors::Resolver::SealedAncestor)) {
            e.setHeader("`{}` is sealed and cannot be inherited by `{}`", superClass.data(ctx)->show(ctx),
                        klass.data(ctx)->show(ctx));
            for (auto loc : superClass.data(ctx)->sealedLocs(ctx)) {
                e.addErrorLine(loc, "`{}` was marked sealed and can only be inherited in this file",
                               superClass.data(ctx)->show(ctx));
            }
        }
    }
    validateSealedAncestorHelper(ctx, klass, classDef, klass, "included");
    const auto singleton = klass.data(ctx)->lookupSingletonClass(ctx);
    validateSealedAncestorHelper(ctx, singleton, classDef, klass, "extended");
}

class ValidateWalk {
private:
    UnorderedMap<core::SymbolRef, vector<core::SymbolRef>> abstractCache;

    const vector<core::SymbolRef> &getAbstractMethods(const core::GlobalState &gs, core::SymbolRef klass) {
        vector<core::SymbolRef> abstract;
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

        auto isAbstract = klass.data(gs)->isClassOrModuleAbstract();
        if (isAbstract) {
            for (auto [name, sym] : klass.data(gs)->members()) {
                if (sym.exists() && sym.data(gs)->isMethod() && sym.data(gs)->isAbstract()) {
                    abstract.emplace_back(sym);
                }
            }
        }

        auto &entry = abstractCache[klass];
        entry = std::move(abstract);
        return entry;
    }

    // if/when we get final classes, we can just mark subclasses of `T::Struct` as final and essentially subsume the
    // logic here.
    void validateTStructNotGrandparent(const core::GlobalState &gs, core::SymbolRef sym) {
        auto parent = sym.data(gs)->superClass();
        if (!parent.exists()) {
            return;
        }
        auto grandparent = parent.data(gs)->superClass();
        if (!grandparent.exists() || grandparent != core::Symbols::T_Struct()) {
            return;
        }
        if (auto e = gs.beginError(sym.data(gs)->loc(), core::errors::Resolver::SubclassingNotAllowed)) {
            auto parentName = parent.data(gs)->show(gs);
            e.setHeader("Subclassing `{}` is not allowed", parentName);
            e.addErrorLine(parent.data(gs)->loc(), "`{}` is a subclass of `T::Struct`", parentName);
        }
    }

    void validateAbstract(const core::GlobalState &gs, core::SymbolRef sym) {
        if (sym.data(gs)->isClassOrModuleAbstract()) {
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
                    e.setHeader("Missing definition for abstract method `{}`", proto.data(gs)->show(gs));
                    e.addErrorLine(proto.data(gs)->loc(), "defined here");
                }
            }
        }
    }

public:
    ast::TreePtr preTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto *classDef = ast::cast_tree_const<ast::ClassDef>(tree);
        auto sym = classDef->symbol;
        auto singleton = sym.data(ctx)->lookupSingletonClass(ctx);
        validateTStructNotGrandparent(ctx, sym);
        validateAbstract(ctx, sym);
        validateAbstract(ctx, singleton);
        validateFinal(ctx, sym, classDef);
        validateSealed(ctx, sym, classDef);
        return tree;
    }

    ast::TreePtr preTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        auto *methodDef = ast::cast_tree<ast::MethodDef>(tree);
        auto methodData = methodDef->symbol.data(ctx);
        auto ownerData = methodData->owner.data(ctx);

        // Only perform this check if this isn't a module from the stdlib, and
        // if there are type members in the owning context.
        // NOTE: We're skipping variance checks on the stdlib right now, as
        // Array and Hash are defined with their parameters as covariant, and as
        // a result most of their methods would fail this check.
        if (!methodData->loc().file().data(ctx).isStdlib() && !ownerData->typeMembers().empty()) {
            variance::validateMethodVariance(ctx, methodDef->symbol);
        }

        validateOverriding(ctx, methodDef->symbol);
        return tree;
    }
};

ast::ParsedFile runOne(core::Context ctx, ast::ParsedFile tree) {
    Timer timeit(ctx.state.tracer(), "validateSymbols");

    ValidateWalk validate;
    tree.tree = ast::ShallowMap::apply(ctx, validate, std::move(tree.tree));
    return tree;
}

} // namespace sorbet::definition_validator
