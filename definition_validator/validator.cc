#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/core.h"
#include "core/errors/resolver.h"

#include "absl/algorithm/container.h"

#include "definition_validator/variance.h"

using namespace std;

namespace sorbet::definition_validator {

struct Signature {
    struct {
        absl::InlinedVector<const core::ArgInfo *, 4> required;
        absl::InlinedVector<const core::ArgInfo *, 4> optional;
        std::optional<const core::ArgInfo *> rest;
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
            dst.rest = std::optional<const core::ArgInfo *>{&arg};
        } else if (arg.flags.isDefault) {
            dst.optional.push_back(&arg);
        } else {
            dst.required.push_back(&arg);
        }
    }
    return sig;
}

// This returns true if `sub` is a subtype of `super`, but it also returns true if either one is `T.untyped` or if
// either one is not fully defined.
bool checkSubtype(const core::Context ctx, core::TypePtr sub, core::TypePtr super) {
    if (!sub || !super) {
        return true;
    }

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
void matchPositional(const core::Context ctx, absl::InlinedVector<const core::ArgInfo *, 4> &superArgs,
                     core::SymbolRef superMethod, absl::InlinedVector<const core::ArgInfo *, 4> &methodArgs,
                     core::SymbolRef method) {
    auto superPos = superArgs.begin();
    auto superEnd = superArgs.end();

    auto methodPos = methodArgs.begin();
    auto methodEnd = methodArgs.end();

    while (superPos != superEnd && methodPos != methodEnd) {
        auto superArgType = (*superPos)->type;
        auto methodArgType = (*methodPos)->type;

        if (!checkSubtype(ctx, superArgType, methodArgType)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                            (*methodPos)->show(ctx), methodArgType->show(ctx), supermethodKind(ctx, superMethod),
                            superMethod.data(ctx)->show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(),
                               "The corresponding parameter `{}` was declared here with type `{}`",
                               (*superPos)->show(ctx), superArgType->show(ctx));
            }
        }

        superPos++;
        methodPos++;
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
                            superMethod.data(ctx)->show(ctx), (*leftRest)->show(ctx));
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
                absl::c_find_if(right.kw.required, [&](const auto &r) { return r->name == req->name; });
            if (corresponding == right.kw.required.end()) {
                corresponding = absl::c_find_if(right.kw.optional, [&](const auto &r) { return r->name == req->name; });
            }

            // if there is a corresponding parameter, make sure it has the right type
            if (corresponding != right.kw.required.end() && corresponding != right.kw.optional.end()) {
                if (!checkSubtype(ctx, req->type, (*corresponding)->type)) {
                    if (auto e =
                            ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                        e.setHeader("Keyword parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                                    (*corresponding)->show(ctx), (*corresponding)->type->show(ctx),
                                    supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx));
                        e.addErrorLine(superMethod.data(ctx)->loc(),
                                       "The corresponding parameter `{}` was declared here with type `{}`",
                                       req->show(ctx), req->type->show(ctx));
                    }
                }
            } else {
                if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                    e.setHeader("Implementation of {} method `{}` is missing required keyword argument `{}`",
                                supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx),
                                req->name.show(ctx));
                    e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
                }
            }
        }

        // make sure that optional parameters expect a compatible type, as well
        for (auto opt : left.kw.optional) {
            auto corresponding =
                absl::c_find_if(right.kw.optional, [&](const auto &r) { return r->name == opt->name; });

            // if there is a corresponding parameter, make sure it has the right type
            if (corresponding != right.kw.optional.end()) {
                if (!checkSubtype(ctx, opt->type, (*corresponding)->type)) {
                    if (auto e =
                            ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                        e.setHeader("Keyword parameter `{}` of type `{}` not compatible with type of {} method `{}`",
                                    (*corresponding)->show(ctx), (*corresponding)->type->show(ctx),
                                    supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx));
                        e.addErrorLine(superMethod.data(ctx)->loc(),
                                       "The corresponding parameter `{}` was declared here with type `{}`",
                                       opt->show(ctx), opt->type->show(ctx));
                    }
                }
            }
        }
    }

    if (auto leftRest = left.kw.rest) {
        if (!right.kw.rest) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Implementation of {} method `{}` must accept **`{}`", supermethodKind(ctx, superMethod),
                            superMethod.data(ctx)->show(ctx), (*leftRest)->show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here");
            }
        } else if (!checkSubtype(ctx, (*leftRest)->type, (*right.kw.rest)->type)) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Parameter **`{}` of type `{}` not compatible with type of {} method `{}`",
                            (*right.kw.rest)->show(ctx), (*right.kw.rest)->type->show(ctx),
                            supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx));
                e.addErrorLine(superMethod.data(ctx)->loc(),
                               "The corresponding parameter **`{}` was declared here with type `{}`",
                               (*left.kw.rest)->show(ctx), (*left.kw.rest)->type->show(ctx));
            }
        }
    }

    for (auto extra : right.kw.required) {
        if (absl::c_any_of(left.kw.required, [&](const auto &l) { return l->name == extra->name; })) {
            continue;
        }
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Implementation of {} method `{}` contains extra required keyword argument `{}`",
                        supermethodKind(ctx, superMethod), superMethod.data(ctx)->show(ctx), extra->name.toString(ctx));
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
                e.addErrorLine(superMethod.data(ctx)->loc(), "Base method defined here with return type `{}`",
                               superReturn->show(ctx));
            }
        }
    }
}

void validateOverriding(const core::Context ctx, core::SymbolRef method) {
    auto klass = method.data(ctx)->owner;
    auto name = method.data(ctx)->name;
    ENFORCE(klass.data(ctx)->isClass());
    auto klassData = klass.data(ctx);
    InlinedVector<core::SymbolRef, 4> overridenMethods;

    // both of these match the behavior of the runtime checks, which will only allow public methods to be defined in
    // interfaces
    if (klassData->isClassInterface() && method.data(ctx)->isPrivate()) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::NonPublicAbstract)) {
            e.setHeader("Interface method `{}` cannot be private", method.show(ctx));
        }
    }

    if (klassData->isClassInterface() && method.data(ctx)->isProtected()) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::NonPublicAbstract)) {
            e.setHeader("Interface method `{}` cannot be protected", method.show(ctx));
        }
    }

    if (method.data(ctx)->isAbstract() && klassData->isClass() && klassData->isSingletonClass(ctx)) {
        auto attached = klassData->attachedClass(ctx);
        if (attached.exists() && attached.data(ctx)->isClassModule()) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadAbstractMethod)) {
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

    if (overridenMethods.size() == 0 && method.data(ctx)->isImplementation()) {
        if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Method `{}` is marked `{}` but does not implement anything", method.data(ctx)->show(ctx),
                        "implementation");
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
            !method.data(ctx)->isDSLSynthesized() && !isRBI) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::UndeclaredOverride)) {
                e.setHeader("Method `{}` overrides an overridable method `{}` but is not declared with `{}`",
                            method.data(ctx)->show(ctx), overridenMethod.data(ctx)->show(ctx), ".override");
                e.addErrorLine(overridenMethod.data(ctx)->loc(), "defined here");
            }
        }
        if (!method.data(ctx)->isImplementation() && !method.data(ctx)->isOverride() && method.data(ctx)->hasSig() &&
            overridenMethod.data(ctx)->isAbstract() && overridenMethod.data(ctx)->hasSig() &&
            !method.data(ctx)->isDSLSynthesized() && !isRBI) {
            if (auto e = ctx.state.beginError(method.data(ctx)->loc(), core::errors::Resolver::UndeclaredOverride)) {
                e.setHeader("Method `{}` implements an abstract method `{}` but is not declared with `{}`",
                            method.data(ctx)->show(ctx), overridenMethod.data(ctx)->show(ctx), ".implementation");
                e.addErrorLine(overridenMethod.data(ctx)->loc(), "defined here");
            }
        }
        if ((overridenMethod.data(ctx)->isAbstract() || overridenMethod.data(ctx)->isOverridable()) &&
            !method.data(ctx)->isIncompatibleOverride() && !isRBI && !method.data(ctx)->isDSLSynthesized()) {
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

core::Loc getAncestorLoc(const core::GlobalState &gs, const unique_ptr<ast::ClassDef> &classDef,
                         const core::SymbolRef ancestor) {
    for (const auto &anc : classDef->ancestors) {
        const auto ancConst = ast::cast_tree<ast::ConstantLit>(anc.get());
        if (ancConst != nullptr && ancConst->symbol.data(gs)->dealias(gs) == ancestor) {
            return anc->loc;
        }
    }
    for (const auto &anc : classDef->singletonAncestors) {
        const auto ancConst = ast::cast_tree<ast::ConstantLit>(anc.get());
        if (ancConst != nullptr && ancConst->symbol.data(gs)->dealias(gs) == ancestor) {
            return anc->loc;
        }
    }
    // give up
    return classDef->loc;
}

void validateFinalAncestorHelper(const core::GlobalState &gs, const core::SymbolRef klass,
                                 const unique_ptr<ast::ClassDef> &classDef, const core::SymbolRef errMsgClass,
                                 const string_view verb) {
    for (const auto &mixin : klass.data(gs)->mixins()) {
        if (!mixin.data(gs)->isClassFinal()) {
            continue;
        }
        if (auto e = gs.beginError(getAncestorLoc(gs, classDef, mixin), core::errors::Resolver::FinalAncestor)) {
            e.setHeader("`{}` was declared as final and cannot be {} in `{}`", mixin.data(gs)->show(gs), verb,
                        errMsgClass.data(gs)->show(gs));
            e.addErrorLine(mixin.data(gs)->loc(), "`{}` defined here", mixin.data(gs)->show(gs));
        }
    }
}

void validateFinalMethodHelper(const core::GlobalState &gs, const core::SymbolRef klass,
                               const core::SymbolRef errMsgClass) {
    if (!klass.data(gs)->isClassFinal()) {
        return;
    }
    for (const auto [name, sym] : klass.data(gs)->members()) {
        if (!sym.exists() || !sym.data(gs)->isMethod() || sym.data(gs)->name == core::Names::staticInit() ||
            sym.data(gs)->isFinalMethod()) {
            continue;
        }
        if (auto e = gs.beginError(sym.data(gs)->loc(), core::errors::Resolver::FinalModuleNonFinalMethod)) {
            e.setHeader("`{}` was declared as final but its method `{}` was not declared as final",
                        errMsgClass.data(gs)->show(gs), sym.data(gs)->name.show(gs));
        }
    }
}

void validateFinal(const core::GlobalState &gs, const core::SymbolRef klass,
                   const unique_ptr<ast::ClassDef> &classDef) {
    const auto superClass = klass.data(gs)->superClass();
    if (superClass.exists() && superClass.data(gs)->isClassFinal()) {
        if (auto e = gs.beginError(getAncestorLoc(gs, classDef, superClass), core::errors::Resolver::FinalAncestor)) {
            e.setHeader("`{}` was declared as final and cannot be inherited by `{}`", superClass.data(gs)->show(gs),
                        klass.data(gs)->show(gs));
            e.addErrorLine(superClass.data(gs)->loc(), "`{}` defined here", superClass.data(gs)->show(gs));
        }
    }
    validateFinalAncestorHelper(gs, klass, classDef, klass, "included");
    validateFinalMethodHelper(gs, klass, klass);
    const auto singleton = klass.data(gs)->lookupSingletonClass(gs);
    validateFinalAncestorHelper(gs, singleton, classDef, klass, "extended");
    validateFinalMethodHelper(gs, singleton, klass);
}

void validateSealedAncestorHelper(const core::GlobalState &gs, const core::SymbolRef klass,
                                  const unique_ptr<ast::ClassDef> &classDef, const core::SymbolRef errMsgClass,
                                  const string_view verb) {
    auto klassFile = klass.data(gs)->loc().file();
    for (const auto &mixin : klass.data(gs)->mixins()) {
        if (!mixin.data(gs)->isClassSealed()) {
            continue;
        }
        // Statically, we allow including / extending in any file that adds a loc to sealedLocs.
        // This is less restrictive than the runtime, because the runtime doesn't have to deal with RBI files.
        if (absl::c_any_of(mixin.data(gs)->sealedLocs(gs), [klassFile](auto loc) { return loc.file() == klassFile; })) {
            continue;
        }
        if (auto e = gs.beginError(getAncestorLoc(gs, classDef, mixin), core::errors::Resolver::SealedAncestor)) {
            e.setHeader("`{}` is sealed and cannot be {} in `{}`", mixin.data(gs)->show(gs), verb,
                        errMsgClass.data(gs)->show(gs));
            for (auto loc : mixin.data(gs)->sealedLocs(gs)) {
                e.addErrorLine(loc, "`{}` was marked sealed and can only be {} in this file", mixin.data(gs)->show(gs),
                               verb);
            }
        }
    }
}

void validateSealed(const core::GlobalState &gs, const core::SymbolRef klass,
                    const unique_ptr<ast::ClassDef> &classDef) {
    const auto superClass = klass.data(gs)->superClass();
    // Statically, we allow a subclass in any file that adds a loc to sealedLocs.
    // This is less restrictive than the runtime, because the runtime doesn't have to deal with RBI files.
    auto file = klass.data(gs)->loc().file();
    if (superClass.exists() && superClass.data(gs)->isClassSealed() &&
        !absl::c_any_of(superClass.data(gs)->sealedLocs(gs), [file](auto loc) { return loc.file() == file; })) {
        if (auto e = gs.beginError(getAncestorLoc(gs, classDef, superClass), core::errors::Resolver::SealedAncestor)) {
            e.setHeader("`{}` is sealed and cannot be inherited by `{}`", superClass.data(gs)->show(gs),
                        klass.data(gs)->show(gs));
            for (auto loc : superClass.data(gs)->sealedLocs(gs)) {
                e.addErrorLine(loc, "`{}` was marked sealed and can only be inherited in this file",
                               superClass.data(gs)->show(gs));
            }
        }
    }
    validateSealedAncestorHelper(gs, klass, classDef, klass, "included");
    const auto singleton = klass.data(gs)->lookupSingletonClass(gs);
    validateSealedAncestorHelper(gs, singleton, classDef, klass, "extended");
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

        auto isAbstract = klass.data(gs)->isClassAbstract();
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
        if (sym.data(gs)->isClassAbstract()) {
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
    unique_ptr<ast::ClassDef> preTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> classDef) {
        auto sym = classDef->symbol;
        auto singleton = sym.data(ctx)->lookupSingletonClass(ctx);
        validateTStructNotGrandparent(ctx.state, sym);
        validateAbstract(ctx.state, sym);
        validateAbstract(ctx.state, singleton);
        validateFinal(ctx.state, sym, classDef);
        validateSealed(ctx.state, sym, classDef);
        return classDef;
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> methodDef) {
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
        return methodDef;
    }
};

ast::ParsedFile runOne(core::Context ctx, ast::ParsedFile tree) {
    Timer timeit(ctx.state.tracer(), "validateSymbols");

    ValidateWalk validate;
    tree.tree = ast::TreeMap::apply(ctx, validate, std::move(tree.tree));
    return tree;
}

} // namespace sorbet::definition_validator
