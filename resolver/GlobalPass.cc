#include "common/Timer.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/resolver.h"
#include "resolver/resolver.h"

#include "absl/algorithm/container.h"

#include <map>
#include <vector>

using namespace std;

namespace sorbet::resolver {

namespace {
core::SymbolRef dealiasAt(const core::GlobalState &gs, core::SymbolRef tparam, core::SymbolRef klass,
                          const vector<vector<pair<core::SymbolRef, core::SymbolRef>>> &typeAliases) {
    ENFORCE(tparam.data(gs)->isTypeMember());
    if (tparam.data(gs)->owner == klass) {
        return tparam;
    } else {
        core::SymbolRef cursor;
        if (tparam.data(gs)->owner.data(gs)->derivesFrom(gs, klass)) {
            cursor = tparam.data(gs)->owner;
        } else if (klass.data(gs)->derivesFrom(gs, tparam.data(gs)->owner)) {
            cursor = klass;
        }
        while (true) {
            if (!cursor.exists()) {
                return cursor;
            }
            for (auto aliasPair : typeAliases[cursor._id]) {
                if (aliasPair.first == tparam) {
                    return dealiasAt(gs, aliasPair.second, klass, typeAliases);
                }
            }
            cursor = cursor.data(gs)->superClass();
        }
    }
}

bool resolveTypeMember(core::GlobalState &gs, core::SymbolRef parent, core::SymbolRef parentTypeMember,
                       core::SymbolRef sym, vector<vector<pair<core::SymbolRef, core::SymbolRef>>> &typeAliases) {
    core::NameRef name = parentTypeMember.data(gs)->name;
    core::SymbolRef my = sym.data(gs)->findMember(gs, name);
    if (!my.exists()) {
        auto code =
            parent == core::Symbols::Enumerable() || parent.data(gs)->derivesFrom(gs, core::Symbols::Enumerable())
                ? core::errors::Resolver::EnumerableParentTypeNotDeclared
                : core::errors::Resolver::ParentTypeNotDeclared;
        if (auto e = gs.beginError(sym.data(gs)->loc(), code)) {
            e.setHeader("Type `{}` declared by parent `{}` must be declared again", name.show(gs),
                        parent.data(gs)->show(gs));
        }
        my = gs.enterTypeMember(sym.data(gs)->loc(), sym, name, core::Variance::Invariant);
        my.data(gs)->setFixed();
        my.data(gs)->resultType = core::Types::untyped(gs, sym);
        return false;
    }
    const auto &data = my.data(gs);
    if (!data->isTypeMember() && !data->isTypeArgument()) {
        if (auto e = gs.beginError(data->loc(), core::errors::Resolver::NotATypeVariable)) {
            e.setHeader("Type variable `{}` needs to be declared as `= type_member(SOMETHING)`", name.show(gs));
        }
        return false;
    }
    auto myVariance = data->variance();
    auto parentVariance = parentTypeMember.data(gs)->variance();
    if (!sym.data(gs)->derivesFrom(gs, core::Symbols::Class()) && myVariance != parentVariance &&
        myVariance != core::Variance::Invariant) {
        if (auto e = gs.beginError(data->loc(), core::errors::Resolver::ParentVarianceMismatch)) {
            e.setHeader("Type variance mismatch with parent `{}`", parent.data(gs)->show(gs));
        }
        return true;
    }
    typeAliases[sym._id].emplace_back(parentTypeMember, my);
    return true;
}

void resolveTypeMembers(core::GlobalState &gs, core::SymbolRef sym,
                        vector<vector<pair<core::SymbolRef, core::SymbolRef>>> &typeAliases, vector<bool> &resolved) {
    ENFORCE(sym.data(gs)->isClass());
    if (resolved[sym._id]) {
        return;
    }
    resolved[sym._id] = true;

    if (sym.data(gs)->superClass().exists()) {
        auto parent = sym.data(gs)->superClass();
        resolveTypeMembers(gs, parent, typeAliases, resolved);

        auto tps = parent.data(gs)->typeMembers();
        bool foundAll = true;
        for (core::SymbolRef tp : tps) {
            bool foundThis = resolveTypeMember(gs, parent, tp, sym, typeAliases);
            foundAll = foundAll && foundThis;
        }
        if (foundAll) {
            int i = 0;
            // check that type params are in the same order.
            for (core::SymbolRef tp : tps) {
                core::SymbolRef my = dealiasAt(gs, tp, sym, typeAliases);
                ENFORCE(my.exists(), "resolver failed to register type member aliases");
                if (sym.data(gs)->typeMembers()[i] != my) {
                    if (auto e = gs.beginError(my.data(gs)->loc(), core::errors::Resolver::TypeMembersInWrongOrder)) {
                        e.setHeader("Type members in wrong order");
                    }
                    int foundIdx = 0;
                    while (foundIdx < sym.data(gs)->typeMembers().size() &&
                           sym.data(gs)->typeMembers()[foundIdx] != my) {
                        foundIdx++;
                    }
                    ENFORCE(foundIdx < sym.data(gs)->typeMembers().size());
                    // quadratic
                    swap(sym.data(gs)->typeMembers()[foundIdx], sym.data(gs)->typeMembers()[i]);
                }
                i++;
            }
        }
    }
    auto mixins = sym.data(gs)->mixins();
    for (core::SymbolRef mixin : mixins) {
        resolveTypeMembers(gs, mixin, typeAliases, resolved);
        auto typeMembers = mixin.data(gs)->typeMembers();
        for (core::SymbolRef tp : typeMembers) {
            resolveTypeMember(gs, mixin, tp, sym, typeAliases);
        }
    }

    if (sym.data(gs)->isClassClass()) {
        for (core::SymbolRef tp : sym.data(gs)->typeMembers()) {
            auto myVariance = tp.data(gs)->variance();
            if (myVariance != core::Variance::Invariant) {
                auto loc = tp.data(gs)->loc();
                if (!loc.file().data(gs).isPayload()) {
                    if (auto e = gs.beginError(loc, core::errors::Resolver::VariantTypeMemberInClass)) {
                        e.setHeader("Classes can only have invariant type members");
                    }
                    return;
                }
            }
        }
    }

    // TODO: this will be the right moment to implement checks for correct locations of co&contra variant types.
}

const vector<core::SymbolRef> &getAbstractMethods(core::GlobalState &gs,
                                                  UnorderedMap<core::SymbolRef, vector<core::SymbolRef>> &abstractCache,
                                                  core::SymbolRef klass) {
    vector<core::SymbolRef> abstract;
    auto ent = abstractCache.find(klass);
    if (ent != abstractCache.end()) {
        return ent->second;
    }

    auto superclass = klass.data(gs)->superClass();
    if (superclass.exists()) {
        auto &superclassMethods = getAbstractMethods(gs, abstractCache, superclass);
        // TODO(nelhage): This code coud go quadratic or even exponential given
        // pathological arrangments of interfaces and abstract methods. Switch
        // to a better data structure if that is ever a problem.
        abstract.insert(abstract.end(), superclassMethods.begin(), superclassMethods.end());
    }

    for (auto ancst : klass.data(gs)->mixins()) {
        auto fromMixin = getAbstractMethods(gs, abstractCache, ancst);
        abstract.insert(abstract.end(), fromMixin.begin(), fromMixin.end());
    }

    auto isAbstract = klass.data(gs)->isClassAbstract();
    if (isAbstract) {
        for (auto mem : klass.data(gs)->members) {
            if (mem.second.data(gs)->isMethod() && mem.second.data(gs)->isAbstract()) {
                abstract.emplace_back(mem.second);
            }
        }
    }

    auto &entry = abstractCache[klass];
    entry = std::move(abstract);
    return entry;
}

struct Signature {
    struct {
        absl::InlinedVector<core::SymbolRef, 4> required;
        absl::InlinedVector<core::SymbolRef, 4> optional;
        core::SymbolRef rest;
    } pos, kw;
    core::SymbolRef blk;
} left, right;

Signature decomposeSignature(core::GlobalState &gs, core::SymbolRef method) {
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
void validateCompatibleOverride(core::GlobalState &gs, core::SymbolRef superMethod, core::SymbolRef method) {
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

void validateOverriding(core::GlobalState &gs, core::SymbolRef method) {
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

void validateAbstract(core::GlobalState &gs, UnorderedMap<core::SymbolRef, vector<core::SymbolRef>> &abstractCache,
                      core::SymbolRef sym) {
    if (sym.data(gs)->isClassAbstract()) {
        return;
    }
    auto loc = sym.data(gs)->loc();
    if (loc.exists() && loc.file().data(gs).isRBI()) {
        return;
    }

    auto &abstract = getAbstractMethods(gs, abstractCache, sym);

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
}; // namespace

void Resolver::finalizeAncestors(core::GlobalState &gs) {
    Timer timer(gs.errorQueue->logger, "resolver.finalize_ancestors");
    int methodCount = 0;
    int classCount = 0;
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto ref = core::SymbolRef(&gs, i);
        auto loc = ref.data(gs)->loc();
        if (loc.file().exists() && loc.file().data(gs).sourceType == core::File::Type::Normal) {
            if (ref.data(gs)->isMethod()) {
                methodCount++;
            } else if (ref.data(gs)->isClass()) {
                classCount++;
            }
        }
        if (!ref.data(gs)->isClass()) {
            continue;
        }
        classCount++;
        if (!ref.data(gs)->isClassModuleSet()) {
            // we did not see a declaration for this type not did we see it used. Default to module.
            ref.data(gs)->setIsModule(true);
        }
        if (ref.data(gs)->superClass().exists() && ref.data(gs)->superClass() != core::Symbols::todo()) {
            continue;
        }
        if (ref == core::Symbols::Sorbet::Private::Static_ImplicitModuleSuperClass()) {
            // only happens if we run without stdlib
            ENFORCE(!core::Symbols::Sorbet::Private::Static_ImplicitModuleSuperClass().data(gs)->loc().exists());
            ref.data(gs)->setSuperClass(core::Symbols::BasicObject());
            continue;
        }

        auto attached = ref.data(gs)->attachedClass(gs);
        bool isSingleton = attached.exists() && attached != core::Symbols::untyped();
        if (isSingleton) {
            if (attached == core::Symbols::BasicObject()) {
                ref.data(gs)->setSuperClass(core::Symbols::Class());
            } else if (attached.data(gs)->superClass() == core::Symbols::Sorbet::Private::Static_ImplicitModuleSuperClass()) {
                // Note: this depends on attached classes having lower indexes in name table than their singletons
                ref.data(gs)->setSuperClass(core::Symbols::Module());
            } else {
                ENFORCE(attached.data(gs)->superClass() != core::Symbols::todo());
                ref.data(gs)->setSuperClass(attached.data(gs)->superClass().data(gs)->singletonClass(gs));
            }
        } else {
            if (ref.data(gs)->isClassClass()) {
                if (!core::Symbols::Object().data(gs)->derivesFrom(gs, ref) && core::Symbols::Object() != ref) {
                    ref.data(gs)->setSuperClass(core::Symbols::Object());
                }
            } else {
                if (!core::Symbols::BasicObject().data(gs)->derivesFrom(gs, ref) &&
                    core::Symbols::BasicObject() != ref) {
                    ref.data(gs)->setSuperClass(core::Symbols::Sorbet::Private::Static_ImplicitModuleSuperClass());
                }
            }
        }
    }

    prodCounterAdd("types.input.classes.total", classCount);
    prodCounterAdd("types.input.methods.total", methodCount);
}

struct ParentLinearizationInformation {
    const InlinedVector<core::SymbolRef, 4> &mixins;
    core::SymbolRef superClass;
    core::SymbolRef klass;
    InlinedVector<core::SymbolRef, 4> fullLinearizationSlow(core::GlobalState &gs);
};

int maybeAddMixin(core::GlobalState &gs, core::SymbolRef forSym, InlinedVector<core::SymbolRef, 4> &mixinList,
                  core::SymbolRef mixin, core::SymbolRef parent, int pos) {
    if (forSym == mixin) {
        Exception::raise("Loop in mixins");
    }
    if (parent.data(gs)->derivesFrom(gs, mixin)) {
        return pos;
    }
    auto fnd = find(mixinList.begin(), mixinList.end(), mixin);
    if (fnd != mixinList.end()) {
        auto newPos = fnd - mixinList.begin();
        if (newPos >= pos) {
            return newPos + 1;
        }
        return pos;
    } else {
        mixinList.insert(mixinList.begin() + pos, mixin);
        return pos + 1;
    }
}

// ** This implements Dmitry's understanding of Ruby linerarization with an optimization that common
// tails of class linearization aren't copied around.
// In order to obtain Ruby-side ancestors, one would need to walk superclass chain and concatenate `mixins`.
// The algorithm is harder to explain than to code, so just follow code & tests if `testdata/resolver/linearization`
ParentLinearizationInformation computeLinearization(core::GlobalState &gs, core::SymbolRef ofClass) {
    ENFORCE(ofClass.exists());
    auto data = ofClass.data(gs);
    ENFORCE(data->isClass());
    if (!data->isClassLinearizationComputed()) {
        if (data->superClass().exists()) {
            computeLinearization(gs, data->superClass());
        }
        InlinedVector<core::SymbolRef, 4> currentMixins = data->mixins();
        InlinedVector<core::SymbolRef, 4> newMixins;
        for (auto mixin : currentMixins) {
            if (mixin == data->superClass()) {
                continue;
            }
            if (mixin.data(gs)->superClass() == core::Symbols::StubAncestor() ||
                mixin.data(gs)->superClass() == core::Symbols::StubModule()) {
                newMixins.emplace_back(mixin);
                continue;
            }
            ENFORCE(mixin.data(gs)->isClass());
            ParentLinearizationInformation mixinLinearization = computeLinearization(gs, mixin);

            if (!mixin.data(gs)->isClassModule()) {
                if (mixin != core::Symbols::SinatraBase() && mixin != core::Symbols::BasicObject()) {
                    // This is a class but Sinatra pass `include`'s it.
                    // Because Sinatra does weird stuff and that's how we model it :-()
                    if (auto e = gs.beginError(data->loc(), core::errors::Resolver::IncludesNonModule)) {
                        e.setHeader("Only modules can be `{}`d. This module or class includes `{}`", "include",
                                    mixin.data(gs)->show(gs));
                    }
                }
                // insert all transitive parents of class to bring methods back.
                auto allMixins = mixinLinearization.fullLinearizationSlow(gs);
                newMixins.insert(newMixins.begin(), allMixins.begin(), allMixins.end());
            } else {
                int pos = 0;
                pos = maybeAddMixin(gs, ofClass, newMixins, mixin, data->superClass(), pos);
                for (auto &mixinLinearizationComponent : mixinLinearization.mixins) {
                    pos = maybeAddMixin(gs, ofClass, newMixins, mixinLinearizationComponent, data->superClass(), pos);
                }
            }
        }
        data->mixins() = std::move(newMixins);
        data->setClassLinearizationComputed();
        if (debug_mode) {
            for (auto oldMixin : currentMixins) {
                ENFORCE(ofClass.data(gs)->derivesFrom(gs, oldMixin), "{} no longer derives from {}",
                        ofClass.data(gs)->showFullName(gs), oldMixin.data(gs)->showFullName(gs));
            }
        }
    }
    ENFORCE(data->isClassLinearizationComputed());
    return ParentLinearizationInformation{data->mixins(), data->superClass(), ofClass};
}

void fullLinearizationSlowImpl(core::GlobalState &gs, const ParentLinearizationInformation &info,
                               InlinedVector<core::SymbolRef, 4> &acc) {
    ENFORCE(!absl::c_linear_search(acc, info.klass));
    acc.emplace_back(info.klass);

    for (auto m : info.mixins) {
        if (!absl::c_linear_search(acc, m)) {
            if (m.data(gs)->isClassModule()) {
                acc.emplace_back(m);
            } else {
                fullLinearizationSlowImpl(gs, computeLinearization(gs, m), acc);
            }
        }
    }
    if (info.superClass.exists()) {
        if (!absl::c_linear_search(acc, info.superClass)) {
            fullLinearizationSlowImpl(gs, computeLinearization(gs, info.superClass), acc);
        }
    }
    return;
};
InlinedVector<core::SymbolRef, 4> ParentLinearizationInformation::fullLinearizationSlow(core::GlobalState &gs) {
    InlinedVector<core::SymbolRef, 4> res;
    fullLinearizationSlowImpl(gs, *this, res);
    return res;
}

void computeLinearization(core::GlobalState &gs) {
    Timer timer(gs.errorQueue->logger, "resolver.compute_linearization");

    // TODO: this does not support `prepend`
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        const auto &data = core::SymbolRef(&gs, i).data(gs);
        if (!data->isClass()) {
            continue;
        }
        computeLinearization(gs, core::SymbolRef(&gs, i));
    }
}

void Resolver::finalizeSymbols(core::GlobalState &gs) {
    Timer timer(gs.errorQueue->logger, "resolver.finalize_resolution");
    // TODO(nelhage): Properly this first loop should go in finalizeAncestors,
    // but we currently compute mixes_in_class_methods during the same AST walk
    // that resolves types and we don't want to introduce additional passes if
    // we don't have to. It would be a tractable refactor to merge it
    // `ResolveConstantsWalk` if it becomes necessary to process earlier.
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto sym = core::SymbolRef(&gs, i);
        if (!sym.data(gs)->isClass()) {
            continue;
        }

        core::SymbolRef singleton;
        for (auto ancst : sym.data(gs)->mixins()) {
            auto classMethods = ancst.data(gs)->findMember(gs, core::Names::classMethods());
            if (!classMethods.exists()) {
                continue;
            }
            if (!singleton.exists()) {
                singleton = sym.data(gs)->singletonClass(gs);
            }
            singleton.data(gs)->mixins().emplace_back(classMethods);
        }
    }

    computeLinearization(gs);

    vector<vector<pair<core::SymbolRef, core::SymbolRef>>> typeAliases;
    typeAliases.resize(gs.symbolsUsed());
    vector<bool> resolved;
    resolved.resize(gs.symbolsUsed());
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto sym = core::SymbolRef(&gs, i);
        if (sym.data(gs)->isClass()) {
            resolveTypeMembers(gs, sym, typeAliases, resolved);
        }
    }
}

void Resolver::validateSymbols(core::GlobalState &gs) {
    UnorderedMap<core::SymbolRef, vector<core::SymbolRef>> abstractCache;

    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto sym = core::SymbolRef(&gs, i);
        if (!sym.data(gs)->loc().exists() || sym.data(gs)->loc().file().data(gs).isPayload()) {
            continue;
        }
        if (sym.data(gs)->isClass()) {
            validateAbstract(gs, abstractCache, sym);
        }
        if (sym.data(gs)->isMethod() && !sym.data(gs)->isBlockSymbol(gs)) {
            validateOverriding(gs, sym);
        }
    }
}

} // namespace sorbet::resolver
