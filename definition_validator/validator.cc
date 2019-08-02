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
        absl::InlinedVector<core::NameRef, 4> required;
        absl::InlinedVector<core::NameRef, 4> optional;
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
            dst.optional.push_back(arg.name);
        } else {
            dst.required.push_back(arg.name);
        }
    }
    return sig;
}

// Eventually this should check the appropriate subtype relationships on types,
// as well, but for now we just look at the argument shapes and ensure that they
// are compatible.
void validateCompatibleOverride(const core::GlobalState &gs, core::SymbolRef superMethod, core::SymbolRef method) {
    if (method.data(gs)->isOverloaded()) {
        // Don't try to check overloaded methods; It's not immediately clear how
        // to match overloads against their superclass definitions. Since we
        // Only permit overloading in the stdlib for now, this is no great loss.
        return;
    }

    auto left = decomposeSignature(gs, superMethod);
    auto right = decomposeSignature(gs, method);

    if (!right.pos.rest) {
        auto leftPos = left.pos.required.size() + left.pos.optional.size();
        auto rightPos = right.pos.required.size() + right.pos.optional.size();
        if (leftPos > rightPos) {
            if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Implementation of abstract method `{}` must accept at least `{}` positional arguments",
                            superMethod.data(gs)->show(gs), leftPos);
                e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
            }
        }
    }

    if (auto leftRest = left.pos.rest) {
        if (!right.pos.rest) {
            if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Implementation of abstract method `{}` must accept *`{}`", superMethod.data(gs)->show(gs),
                            (*leftRest)->show(gs));
                e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
            }
        }
    }

    if (right.pos.required.size() > left.pos.required.size()) {
        if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Implementation of abstract method `{}` must accept no more than `{}` required argument(s)",
                        superMethod.data(gs)->show(gs), left.pos.required.size());
            e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
        }
    }

    if (!right.kw.rest) {
        for (auto req : left.kw.required) {
            if (absl::c_any_of(right.kw.required, [&](const auto &r) { return r == req; })) {
                continue;
            }
            if (absl::c_any_of(right.kw.optional, [&](const auto &r) { return r == req; })) {
                continue;
            }
            if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Implementation of abstract method `{}` is missing required keyword argument `{}`",
                            superMethod.data(gs)->show(gs), req.show(gs));
                e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
            }
        }
    }

    if (auto leftRest = left.kw.rest) {
        if (!right.kw.rest) {
            if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Implementation of abstract method `{}` must accept **`{}`", superMethod.data(gs)->show(gs),
                            (*leftRest)->show(gs));
                e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
            }
        }
    }

    for (auto extra : right.kw.required) {
        if (absl::c_any_of(left.kw.required, [&](const auto &l) { return l == extra; })) {
            continue;
        }
        if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Implementation of abstract method `{}` contains extra required keyword argument `{}`",
                        superMethod.data(gs)->show(gs), extra.toString(gs));
            e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
        }
    }

    if (!left.syntheticBlk && right.syntheticBlk) {
        if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Implementation of abstract method `{}` must explicitly name a block argument",
                        superMethod.data(gs)->show(gs));
            e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
        }
    }
}

void validateOverriding(const core::GlobalState &gs, core::SymbolRef method) {
    auto klass = method.data(gs)->owner;
    auto name = method.data(gs)->name;
    ENFORCE(klass.data(gs)->isClass());
    auto klassData = klass.data(gs);
    InlinedVector<core::SymbolRef, 4> overridenMethods;

    // both of these match the behavior of the runtime checks, which will only allow public methods to be defined in
    // interfaces
    if (klassData->isClassInterface() && method.data(gs)->isPrivate()) {
        if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::NonPublicAbstract)) {
            e.setHeader("Interface method `{}` cannot be private", method.show(gs));
        }
    }

    if (klassData->isClassInterface() && method.data(gs)->isProtected()) {
        if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::NonPublicAbstract)) {
            e.setHeader("Interface method `{}` cannot be protected", method.show(gs));
        }
    }

    if (method.data(gs)->isAbstract() && klassData->isClass() && klassData->isSingletonClass(gs)) {
        auto attached = klassData->attachedClass(gs);
        if (attached.exists() && attached.data(gs)->isClassModule()) {
            if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadAbstractMethod)) {
                e.setHeader("Static methods in a module cannot be abstract");
            }
        }
    }

    if (klassData->superClass().exists()) {
        auto superMethod = klassData->superClass().data(gs)->findMemberTransitive(gs, name);
        if (superMethod.exists()) {
            overridenMethods.emplace_back(superMethod);
        }
    }
    for (const auto &mixin : klassData->mixins()) {
        auto superMethod = mixin.data(gs)->findMember(gs, name);
        if (superMethod.exists()) {
            overridenMethods.emplace_back(superMethod);
        }
    }

    for (const auto &overridenMethod : overridenMethods) {
        if (overridenMethod.data(gs)->isFinalMethod()) {
            if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::OverridesFinal)) {
                e.setHeader("`{}` was declared as final and cannot be overridden by `{}`",
                            overridenMethod.data(gs)->show(gs), method.data(gs)->show(gs));
                e.addErrorLine(overridenMethod.data(gs)->loc(), "original method defined here");
            }
        }
        auto isRBI = absl::c_any_of(method.data(gs)->locs(), [&](auto &loc) { return loc.file().data(gs).isRBI(); });
        if ((overridenMethod.data(gs)->isAbstract() || overridenMethod.data(gs)->isOverridable()) &&
            !method.data(gs)->isIncompatibleOverride() && !isRBI) {
            validateCompatibleOverride(gs, overridenMethod, method);
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
        if (!sym.data(gs)->isMethod() || sym.data(gs)->name == core::Names::staticInit() ||
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
                if (sym.data(gs)->isMethod() && sym.data(gs)->isAbstract()) {
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

        validateOverriding(ctx.state, methodDef->symbol);
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
