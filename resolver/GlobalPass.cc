#include "absl/algorithm/container.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/resolver.h"
#include "resolver/resolver.h"

#include <map>
#include <vector>

using namespace std;

namespace sorbet::resolver {

namespace {
core::SymbolRef dealiasAt(const core::GlobalState &gs, core::SymbolRef tparam, core::SymbolRef klass,
                          const vector<vector<pair<core::SymbolRef, core::SymbolRef>>> &typeAliases) {
    ENFORCE(tparam.data(gs).isTypeMember());
    if (tparam.data(gs).owner == klass) {
        return tparam;
    } else {
        core::SymbolRef cursor;
        if (tparam.data(gs).owner.data(gs).derivesFrom(gs, klass)) {
            cursor = tparam.data(gs).owner;
        } else if (klass.data(gs).derivesFrom(gs, tparam.data(gs).owner)) {
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
            cursor = cursor.data(gs).superClass;
        }
    }
}

bool resolveTypeMember(core::GlobalState &gs, core::SymbolRef parent, core::SymbolRef parentTypeMember,
                       core::SymbolRef sym, vector<vector<pair<core::SymbolRef, core::SymbolRef>>> &typeAliases) {
    core::NameRef name = parentTypeMember.data(gs).name;
    auto parentVariance = parentTypeMember.data(gs).variance();
    auto &inSym = sym.data(gs);
    core::SymbolRef my = inSym.findMember(gs, name);
    bool ok = true;
    if (!my.exists()) {
        if (!(parent == core::Symbols::Enumerable() || parent.data(gs).derivesFrom(gs, core::Symbols::Enumerable()))) {
            if (auto e = gs.beginError(inSym.loc(), core::errors::Resolver::ParentTypeNotDeclared)) {
                e.setHeader("Type `{}` declared by parent `{}` should be declared again", name.show(gs),
                            parent.data(gs).show(gs));
            }
            ok = false;
        }
        my = gs.enterTypeMember(inSym.loc(), sym, name, core::Variance::Invariant);
    }
    if (!ok) {
        return false;
    }
    auto &data = my.data(gs);
    if (!data.isTypeMember() && !data.isTypeArgument()) {
        if (auto e = gs.beginError(data.loc(), core::errors::Resolver::NotATypeVariable)) {
            e.setHeader("Type variable `{}` needs to be declared as `= type_member(SOMETHING)`", name.show(gs));
        }
        return false;
    }
    auto myVariance = data.variance();
    if (!inSym.derivesFrom(gs, core::Symbols::Class()) && myVariance != parentVariance &&
        myVariance != core::Variance::Invariant) {
        if (auto e = gs.beginError(data.loc(), core::errors::Resolver::ParentVarianceMismatch)) {
            e.setHeader("Type variance mismatch with parent `{}`", parent.data(gs).show(gs));
        }
        return true;
    }
    typeAliases[sym._id].emplace_back(parentTypeMember, my);
    return true;
}

void resolveTypeMembers(core::GlobalState &gs, core::SymbolRef sym,
                        vector<vector<pair<core::SymbolRef, core::SymbolRef>>> &typeAliases) {
    auto &inSym = sym.data(gs);
    ENFORCE(inSym.isClass());

    if (inSym.superClass.exists()) {
        auto parent = inSym.superClass;
        auto tps = parent.data(gs).typeMembers();
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
                if (inSym.typeMembers()[i] != my) {
                    if (auto e = gs.beginError(my.data(gs).loc(), core::errors::Resolver::TypeMembersInWrongOrder)) {
                        e.setHeader("Type members in wrong order");
                    }
                    int foundIdx = 0;
                    while (foundIdx < inSym.typeMembers().size() && inSym.typeMembers()[foundIdx] != my) {
                        foundIdx++;
                    }
                    ENFORCE(foundIdx < inSym.typeMembers().size());
                    // quadratic
                    swap(inSym.typeMembers()[foundIdx], inSym.typeMembers()[i]);
                }
                i++;
            }
        }
    }

    for (core::SymbolRef mixin : inSym.mixins()) {
        for (core::SymbolRef tp : mixin.data(gs).typeMembers()) {
            resolveTypeMember(gs, mixin, tp, sym, typeAliases);
        }
    }

    if (inSym.isClassClass()) {
        for (core::SymbolRef tp : inSym.typeMembers()) {
            auto myVariance = tp.data(gs).variance();
            if (myVariance != core::Variance::Invariant) {
                auto loc = tp.data(gs).loc();
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

    auto superclass = klass.data(gs).superClass;
    if (superclass.exists()) {
        auto &superclassMethods = getAbstractMethods(gs, abstractCache, superclass);
        // TODO(nelhage): This code coud go quadratic or even exponential given
        // pathological arrangments of interfaces and abstract methods. Switch
        // to a better data structure if that is ever a problem.
        abstract.insert(abstract.end(), superclassMethods.begin(), superclassMethods.end());
    }

    for (auto ancst : klass.data(gs).mixins()) {
        auto fromMixin = getAbstractMethods(gs, abstractCache, ancst);
        abstract.insert(abstract.end(), fromMixin.begin(), fromMixin.end());
    }

    auto isAbstract = klass.data(gs).isClassAbstract();
    if (isAbstract) {
        for (auto mem : klass.data(gs).members) {
            if (mem.second.data(gs).isMethod() && mem.second.data(gs).isAbstract()) {
                abstract.emplace_back(mem.second);
            }
        }
    }

    auto &entry = abstractCache[klass];
    entry = move(abstract);
    return entry;
}

void validateAbstract(core::GlobalState &gs, UnorderedMap<core::SymbolRef, vector<core::SymbolRef>> &abstractCache,
                      core::SymbolRef sym) {
    if (sym.data(gs).isClassAbstract()) {
        return;
    }
    auto &abstract = getAbstractMethods(gs, abstractCache, sym);

    if (abstract.empty()) {
        return;
    }

    for (auto proto : abstract) {
        if (proto.data(gs).owner == sym) {
            continue;
        }

        auto mem = sym.data(gs).findConcreteMethodTransitive(gs, proto.data(gs).name);
        auto loc = sym.data(gs).loc();
        if (!mem.exists() && !loc.file().data(gs).isRBI()) {
            if (auto e = gs.beginError(loc, core::errors::Resolver::BadAbstractMethod)) {
                e.setHeader("Missing definition for abstract method `{}`", proto.data(gs).show(gs));
                e.addErrorLine(proto.data(gs).loc(), "defined here");
            }
        }
    }
}
}; // namespace

void Resolver::finalizeAncestors(core::GlobalState &gs) {
    gs.trace("Finalizing ancestor chains");
    int methodCount = 0;
    int classCount = 0;
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto ref = core::SymbolRef(&gs, i);
        auto &data = ref.data(gs);
        auto loc = data.loc();
        if (loc.file().exists() && loc.file().data(gs).sourceType == core::File::Type::Normal) {
            if (data.isMethod()) {
                methodCount++;
            } else if (data.isClass()) {
                classCount++;
            }
        }
        if (!data.isClass()) {
            continue;
        }
        classCount++;
        if (!data.isClassModuleSet()) {
            // we did not see a declaration for this type not did we see it used. Default to module.
            data.setIsModule(true);
        }
        if (data.superClass.exists() && data.superClass != core::Symbols::todo()) {
            continue;
        }
        if (ref == core::Symbols::RubyTyper_ImplicitModuleSuperClass()) {
            // only happens if we run without stdlib
            ENFORCE(!core::Symbols::RubyTyper_ImplicitModuleSuperClass().data(gs).loc().exists());
            data.superClass = core::Symbols::BasicObject();
            continue;
        }

        auto attached = data.attachedClass(gs);
        bool isSingleton = attached.exists() && attached != core::Symbols::untyped();
        if (isSingleton) {
            if (attached == core::Symbols::BasicObject()) {
                data.superClass = core::Symbols::Class();
            } else if (attached.data(gs).superClass == core::Symbols::RubyTyper_ImplicitModuleSuperClass()) {
                // Note: this depends on attached classes having lower indexes in name table than their singletons
                data.superClass = core::Symbols::Module();
            } else {
                ENFORCE(attached.data(gs).superClass != core::Symbols::todo());
                data.superClass = attached.data(gs).superClass.data(gs).singletonClass(gs);
            }
        } else {
            if (data.isClassClass()) {
                if (!core::Symbols::Object().data(gs).derivesFrom(gs, ref) && core::Symbols::Object() != ref) {
                    data.superClass = core::Symbols::Object();
                }
            } else {
                if (!core::Symbols::BasicObject().data(gs).derivesFrom(gs, ref) &&
                    core::Symbols::BasicObject() != ref) {
                    data.superClass = core::Symbols::RubyTyper_ImplicitModuleSuperClass();
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
        Error::raise("Loop in mixins");
    }
    if (parent.data(gs).derivesFrom(gs, mixin)) {
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
    auto &data = ofClass.data(gs);
    ENFORCE(data.isClass());
    if (!data.isClassLinearizationComputed()) {
        if (data.superClass.exists()) {
            computeLinearization(gs, data.superClass);
        }
        InlinedVector<core::SymbolRef, 4> currentMixins = data.mixins();
        InlinedVector<core::SymbolRef, 4> newMixins;
        for (auto mixin : currentMixins) {
            if (mixin == data.superClass) {
                continue;
            }
            if (mixin.data(gs).superClass == core::Symbols::StubAncestor() ||
                mixin.data(gs).superClass == core::Symbols::StubClass()) {
                newMixins.emplace_back(mixin);
                continue;
            }
            ENFORCE(mixin.data(gs).isClass());
            ParentLinearizationInformation mixinLinearization = computeLinearization(gs, mixin);

            if (!mixin.data(gs).isClassModule()) {
                if (mixin != core::Symbols::SinatraBase() && mixin != core::Symbols::BasicObject()) {
                    // This is a class but Sinatra pass `include`'s it.
                    // Because Sinatra does weird stuff and that's how we model it :-()
                    if (auto e = gs.beginError(data.loc(), core::errors::Resolver::IncludesNonModule)) {
                        e.setHeader("Only modules can be `{}`d. This module or class includes `{}`", "include",
                                    mixin.data(gs).show(gs));
                    }
                }
                // insert all transitive parents of class to bring methods back.
                auto allMixins = mixinLinearization.fullLinearizationSlow(gs);
                newMixins.insert(newMixins.begin(), allMixins.begin(), allMixins.end());
            } else {
                int pos = 0;
                pos = maybeAddMixin(gs, ofClass, newMixins, mixin, data.superClass, pos);
                for (auto &mixinLinearizationComponent : mixinLinearization.mixins) {
                    pos = maybeAddMixin(gs, ofClass, newMixins, mixinLinearizationComponent, data.superClass, pos);
                }
            }
        }
        data.mixins() = move(newMixins);
        data.setClassLinearizationComputed();
        if (debug_mode) {
            for (auto oldMixin : currentMixins) {
                ENFORCE(ofClass.data(gs).derivesFrom(gs, oldMixin),
                        ofClass.data(gs).fullName(gs) + " no longer derrives from " + oldMixin.data(gs).fullName(gs));
            }
        }
    }
    ENFORCE(data.isClassLinearizationComputed());
    return ParentLinearizationInformation{data.mixins(), data.superClass, ofClass};
}

void fullLinearizationSlowImpl(core::GlobalState &gs, const ParentLinearizationInformation &info,
                               InlinedVector<core::SymbolRef, 4> &acc) {
    ENFORCE(!absl::c_linear_search(acc, info.klass));
    acc.emplace_back(info.klass);

    for (auto m : info.mixins) {
        if (!absl::c_linear_search(acc, m)) {
            if (m.data(gs).isClassModule()) {
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
    gs.trace("Computing linearization");
    // TODO: this does not support `prepend`
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto &data = core::SymbolRef(&gs, i).data(gs);
        if (!data.isClass()) {
            continue;
        }
        computeLinearization(gs, core::SymbolRef(&gs, i));
    }
}

void Resolver::finalizeResolution(core::GlobalState &gs) {
    gs.trace("Finalizing resolution");
    // TODO(nelhage): Properly this first loop should go in finalizeAncestors,
    // but we currently compute mixes_in_class_methods during the same AST walk
    // that resolves types and we don't want to introduce additional passes if
    // we don't have to. It would be a tractable refactor to merge it
    // `ResolveConstantsWalk` if it becomes necessary to process earlier.
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto sym = core::SymbolRef(&gs, i);
        if (!sym.data(gs).isClass()) {
            continue;
        }

        core::SymbolRef singleton;
        for (auto ancst : sym.data(gs).mixins()) {
            auto classMethods = ancst.data(gs).findMember(gs, core::Names::classMethods());
            if (!classMethods.exists()) {
                continue;
            }
            if (!singleton.exists()) {
                singleton = sym.data(gs).singletonClass(gs);
            }
            singleton.data(gs).mixins().emplace_back(classMethods);
        }
    }

    vector<vector<pair<core::SymbolRef, core::SymbolRef>>> typeAliases;
    typeAliases.resize(gs.symbolsUsed());

    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto sym = core::SymbolRef(&gs, i);
        if (sym.data(gs).isClass()) {
            resolveTypeMembers(gs, sym, typeAliases);
        }
    }

    computeLinearization(gs);
    UnorderedMap<core::SymbolRef, vector<core::SymbolRef>> abstractCache;

    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto sym = core::SymbolRef(&gs, i);
        if (sym.data(gs).isClass()) {
            validateAbstract(gs, abstractCache, sym);
        }
    }
}

} // namespace sorbet::resolver
