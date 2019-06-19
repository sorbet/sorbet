#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/core.h"
#include "core/errors/resolver.h"

#include "absl/algorithm/container.h"

using namespace std;

namespace sorbet::definition_validator {

struct Signature {
    struct {
        absl::InlinedVector<core::SymbolRef, 4> required;
        absl::InlinedVector<core::SymbolRef, 4> optional;
        core::SymbolRef rest;
    } pos, kw;
    core::SymbolRef blk;
} left, right;

Signature decomposeSignature(const core::GlobalState &gs, core::SymbolRef method) {
    Signature sig;
    for (auto arg : method.data(gs)->arguments()) {
        if (arg.data(gs)->isBlockArgument()) {
            sig.blk = arg;
            continue;
        }

        auto &dst = arg.data(gs)->isKeyword() ? sig.kw : sig.pos;
        if (arg.data(gs)->isRepeated()) {
            dst.rest = arg;
        } else if (arg.data(gs)->isOptional()) {
            dst.optional.push_back(arg);
        } else {
            dst.required.push_back(arg);
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

    if (!right.pos.rest.exists()) {
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

    if (left.pos.rest.exists() && !right.pos.rest.exists()) {
        if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Implementation of abstract method `{}` must accept *`{}`", superMethod.data(gs)->show(gs),
                        left.pos.rest.data(gs)->argumentName(gs));
            e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
        }
    }

    if (right.pos.required.size() > left.pos.required.size()) {
        if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Implementation of abstract method `{}` must accept no more than `{}` required argument(s)",
                        superMethod.data(gs)->show(gs), left.pos.required.size());
            e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
        }
    }

    if (!right.kw.rest.exists()) {
        for (auto req : left.kw.required) {
            auto nm = req.data(gs)->name;
            if (absl::c_any_of(right.kw.required, [&](auto &r) { return r.data(gs)->name == nm; }))
                continue;
            if (absl::c_any_of(right.kw.optional, [&](auto &r) { return r.data(gs)->name == nm; }))
                continue;
            if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
                e.setHeader("Implementation of abstract method `{}` is missing required keyword argument `{}`",
                            superMethod.data(gs)->show(gs), req.data(gs)->argumentName(gs));
                e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
            }
        }
    }

    if (left.kw.rest.exists() && !right.kw.rest.exists()) {
        if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Implementation of abstract method `{}` must accept **`{}`", superMethod.data(gs)->show(gs),
                        left.kw.rest.data(gs)->argumentName(gs));
            e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
        }
    }

    for (auto extra : right.kw.required) {
        if (absl::c_any_of(left.kw.required, [&](auto l) { return l.data(gs)->name == extra.data(gs)->name; })) {
            continue;
        }
        if (auto e = gs.beginError(method.data(gs)->loc(), core::errors::Resolver::BadMethodOverride)) {
            e.setHeader("Implementation of abstract method `{}` contains extra required keyword argument `{}`",
                        superMethod.data(gs)->show(gs), extra.data(gs)->argumentName(gs));
            e.addErrorLine(superMethod.data(gs)->loc(), "Base method defined here");
        }
    }

    ENFORCE(left.blk.exists() && right.blk.exists(), "Broken assumption: every method has a block argument.");
    // TODO This is a raw string comparison on argument names
    if (left.blk.data(gs)->argumentName(gs) != core::Names::blkArg().data(gs)->shortName(gs) &&
        right.blk.data(gs)->argumentName(gs) == core::Names::blkArg().data(gs)->shortName(gs)) {
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
                e.setHeader("Method overrides a final method `{}`", overridenMethod.data(gs)->show(gs));
                e.addErrorLine(overridenMethod.data(gs)->loc(), "defined here");
            }
        }
        if ((overridenMethod.data(gs)->isAbstract() || overridenMethod.data(gs)->isOverridable()) &&
            (method.data(gs)->isImplementation() || method.data(gs)->isOverride())) {
            validateCompatibleOverride(gs, overridenMethod, method);
        }
    }
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
            // TODO(nelhage): This code coud go quadratic or even exponential given
            // pathological arrangments of interfaces and abstract methods. Switch
            // to a better data structure if that is ever a problem.
            abstract.insert(abstract.end(), superclassMethods.begin(), superclassMethods.end());
        }

        for (auto ancst : klass.data(gs)->mixins()) {
            auto fromMixin = getAbstractMethods(gs, ancst);
            abstract.insert(abstract.end(), fromMixin.begin(), fromMixin.end());
        }

        auto isAbstract = klass.data(gs)->isClassAbstract();
        if (isAbstract) {
            for (auto mem : klass.data(gs)->members()) {
                if (mem.second.data(gs)->isMethod() && mem.second.data(gs)->isAbstract()) {
                    abstract.emplace_back(mem.second);
                }
            }
        }

        auto &entry = abstractCache[klass];
        entry = std::move(abstract);
        return entry;
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
        validateAbstract(ctx.state, sym);
        auto singleton = sym.data(ctx)->lookupSingletonClass(ctx);
        validateAbstract(ctx.state, singleton);
        return classDef;
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> methodDef) {
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
