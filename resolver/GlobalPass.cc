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
core::TypeMemberRef dealiasAt(const core::GlobalState &gs, core::TypeMemberRef tparam, core::ClassOrModuleRef klass,
                              const vector<vector<pair<core::TypeMemberRef, core::TypeMemberRef>>> &typeAliases) {
    if (tparam.data(gs)->owner == klass) {
        return tparam;
    } else {
        core::ClassOrModuleRef cursor;
        if (tparam.data(gs)->owner.asClassOrModuleRef().data(gs)->derivesFrom(gs, klass)) {
            cursor = tparam.data(gs)->owner.asClassOrModuleRef();
        } else if (klass.data(gs)->derivesFrom(gs, tparam.data(gs)->owner.asClassOrModuleRef())) {
            cursor = klass;
        }
        while (true) {
            if (!cursor.exists()) {
                return core::Symbols::noTypeMember();
            }
            for (auto aliasPair : typeAliases[cursor.id()]) {
                if (aliasPair.first == tparam) {
                    return dealiasAt(gs, aliasPair.second, klass, typeAliases);
                }
            }
            cursor = cursor.data(gs)->superClass();
        }
    }
}

bool resolveTypeMember(core::GlobalState &gs, core::ClassOrModuleRef parent, core::TypeMemberRef parentTypeMember,
                       core::ClassOrModuleRef sym,
                       vector<vector<pair<core::TypeMemberRef, core::TypeMemberRef>>> &typeAliases) {
    core::NameRef name = parentTypeMember.data(gs)->name;
    core::SymbolRef my = sym.data(gs)->findMember(gs, name);
    if (!my.exists()) {
        auto code =
            parent == core::Symbols::Enumerable() || parent.data(gs)->derivesFrom(gs, core::Symbols::Enumerable())
                ? core::errors::Resolver::EnumerableParentTypeNotDeclared
                : core::errors::Resolver::ParentTypeNotDeclared;

        if (auto e = gs.beginError(sym.data(gs)->loc(), code)) {
            // Note having this error is intentional, as it makes it easier to implement Sorbet's incremental mode.
            //
            // In the past, we have floated ideas of allowing users to skip redeclaring a parent's type members if
            // those type members are fixed, and simply copying the fixed type member down onto the child on the
            // user's behalf.
            //
            // But that runs afoul of our (simple) heuristic to retypecheck all files that simply mention a symbol
            // name on the LSP fast path. If a grandchild class was not forced to redeclare a grandparent's
            // `type_member`, then the grandparent class's file could be edited and Sorbet wouldn't include the
            // grandchild class's file in the set of files to retypecheck.
            e.setHeader("Type `{}` declared by parent `{}` must be re-declared in `{}`", name.show(gs), parent.show(gs),
                        sym.show(gs));
            e.addErrorLine(parentTypeMember.data(gs)->loc(), "`{}` declared in parent here", name.show(gs));
        }
        auto typeMember = gs.enterTypeMember(sym.data(gs)->loc(), sym, name, core::Variance::Invariant);
        typeMember.data(gs)->flags.isFixed = true;
        auto untyped = core::Types::untyped(gs, sym);
        typeMember.data(gs)->resultType = core::make_type<core::LambdaParam>(typeMember, untyped, untyped);
        typeAliases[sym.id()].emplace_back(parentTypeMember, typeMember);
        return false;
    }
    if (!my.isTypeMember()) {
        if (auto e = gs.beginError(my.loc(gs), core::errors::Resolver::NotATypeVariable)) {
            e.setHeader("Type variable `{}` needs to be declared as `= type_member(SOMETHING)`", name.show(gs));
        }
        auto synthesizedName = gs.freshNameUnique(core::UniqueNameKind::TypeVarName, name, 1);
        auto typeMember = gs.enterTypeMember(sym.data(gs)->loc(), sym, synthesizedName, core::Variance::Invariant);
        typeMember.data(gs)->flags.isFixed = true;
        auto untyped = core::Types::untyped(gs, sym);
        typeMember.data(gs)->resultType = core::make_type<core::LambdaParam>(typeMember, untyped, untyped);
        typeAliases[sym.id()].emplace_back(parentTypeMember, typeMember);
        return false;
    }

    auto myTypeMember = my.asTypeMemberRef();
    typeAliases[sym.id()].emplace_back(parentTypeMember, myTypeMember);
    auto myVariance = myTypeMember.data(gs)->variance();
    auto parentVariance = parentTypeMember.data(gs)->variance();
    if (!sym.data(gs)->derivesFrom(gs, core::Symbols::Class()) && myVariance != parentVariance &&
        myVariance != core::Variance::Invariant) {
        if (auto e = gs.beginError(myTypeMember.data(gs)->loc(), core::errors::Resolver::ParentVarianceMismatch)) {
            auto orInvariant = parentVariance == core::Variance::Invariant ? "" : " or invariant";
            e.setHeader("Type variance mismatch for `{}` with parent `{}`. Child `{}` should be `{}`{}, but "
                        "it is `{}`",
                        name.show(gs), parent.show(gs), sym.show(gs), core::Polarities::showVariance(parentVariance),
                        orInvariant, core::Polarities::showVariance(myVariance));
            e.addErrorLine(parentTypeMember.data(gs)->loc(), "Parent `{}` declared here", parent.show(gs));
        }
        return true;
    }
    return true;
} // namespace

void resolveTypeMembers(core::GlobalState &gs, core::ClassOrModuleRef sym,
                        vector<vector<pair<core::TypeMemberRef, core::TypeMemberRef>>> &typeAliases,
                        vector<bool> &resolved) {
    if (resolved[sym.id()]) {
        return;
    }
    resolved[sym.id()] = true;

    if (sym.data(gs)->superClass().exists()) {
        auto parent = sym.data(gs)->superClass();
        resolveTypeMembers(gs, parent, typeAliases, resolved);

        auto parentTypeMembers = parent.data(gs)->typeMembers();
        bool foundAll = true;
        for (auto parentTypeMember : parentTypeMembers) {
            bool foundThis = resolveTypeMember(gs, parent, parentTypeMember, sym, typeAliases);
            foundAll = foundAll && foundThis;
        }
        if (foundAll) {
            int parentIdx = 0;
            // check that type params are in the same order.
            for (auto parentTypeMember : parentTypeMembers) {
                auto my = dealiasAt(gs, parentTypeMember, sym, typeAliases);
                ENFORCE(my.exists(), "resolver failed to register type member aliases");
                if (sym.data(gs)->typeMembers()[parentIdx] != my) {
                    if (auto e = gs.beginError(my.data(gs)->loc(), core::errors::Resolver::TypeMembersInWrongOrder)) {
                        e.setHeader("Type members for `{}` repeated in wrong order", sym.show(gs));
                        e.addErrorLine(my.data(gs)->loc(), "Found type member with name `{}`",
                                       my.data(gs)->name.show(gs));
                        e.addErrorLine(sym.data(gs)->typeMembers()[parentIdx].data(gs)->loc(),
                                       "Expected type member with name `{}`",
                                       sym.data(gs)->typeMembers()[parentIdx].data(gs)->name.show(gs));
                        e.addErrorLine(parentTypeMember.data(gs)->loc(),
                                       "`{}` defined in parent here:", parentTypeMember.data(gs)->name.show(gs));
                    }
                    int childIdx = 0;
                    while (childIdx < sym.data(gs)->typeMembers().size() &&
                           sym.data(gs)->typeMembers()[childIdx] != my) {
                        childIdx++;
                    }
                    ENFORCE(childIdx < sym.data(gs)->typeMembers().size());
                    // quadratic
                    swap(sym.data(gs)->existingTypeMembers()[childIdx], sym.data(gs)->existingTypeMembers()[parentIdx]);
                }
                parentIdx++;
            }
        }
    }
    for (auto mixin : sym.data(gs)->mixins()) {
        resolveTypeMembers(gs, mixin, typeAliases, resolved);
        auto typeMembers = mixin.data(gs)->typeMembers();
        for (auto tm : typeMembers) {
            resolveTypeMember(gs, mixin, tm, sym, typeAliases);
        }
    }

    if (sym.data(gs)->isClass()) {
        for (auto tm : sym.data(gs)->typeMembers()) {
            // AttachedClass is covariant, but not controlled by the user.
            if (tm.data(gs)->name == core::Names::Constants::AttachedClass()) {
                continue;
            }

            auto myVariance = tm.data(gs)->variance();
            if (myVariance != core::Variance::Invariant) {
                auto loc = tm.data(gs)->loc();
                if (!loc.file().data(gs).isPayload()) {
                    if (auto e = gs.beginError(loc, core::errors::Resolver::VariantTypeMemberInClass)) {
                        e.setHeader("Classes can only have invariant type members");
                    }
                    return;
                }
            }
        }
    }

    // If this class has no type members, fix attached class early.
    if (sym.data(gs)->typeMembers().empty()) {
        sym.data(gs)->unsafeComputeExternalType(gs);
        auto singleton = sym.data(gs)->lookupSingletonClass(gs);
        if (singleton.exists()) {
            // AttachedClass doesn't exist on `T.untyped`, which is a problem
            // with RuntimeProfiled.
            auto attachedClass = singleton.data(gs)->findMember(gs, core::Names::Constants::AttachedClass());
            if (attachedClass.exists()) {
                auto *lambdaParam =
                    core::cast_type<core::LambdaParam>(attachedClass.asTypeMemberRef().data(gs)->resultType);
                ENFORCE(lambdaParam != nullptr);

                lambdaParam->lowerBound = core::Types::bottom();
                lambdaParam->upperBound = sym.data(gs)->externalType();
            }
        }
    }
}

}; // namespace

void Resolver::finalizeAncestors(core::GlobalState &gs) {
    Timer timer(gs.tracer(), "resolver.finalize_ancestors");
    int methodCount = 0;
    int classCount = 0;
    int singletonClassCount = 0;
    int moduleCount = 0;
    for (size_t i = 1; i < gs.methodsUsed(); ++i) {
        auto ref = core::MethodRef(gs, i);
        auto loc = ref.data(gs)->loc();
        if (loc.file().exists() && loc.file().data(gs).sourceType == core::File::Type::Normal) {
            methodCount++;
        }
    }
    for (int i = 1; i < gs.classAndModulesUsed(); ++i) {
        auto ref = core::ClassOrModuleRef(gs, i);
        if (!ref.data(gs)->isClassModuleSet()) {
            // we did not see a declaration for this type not did we see it used. Default to module.
            ref.data(gs)->setIsModule(true);

            // allow us to catch undeclared modules in LSP fast path, so we can report ambiguous
            // definition errors.
            ref.data(gs)->flags.isUndeclared = true;
        }
        auto loc = ref.data(gs)->loc();
        if (loc.file().exists() && loc.file().data(gs).sourceType == core::File::Type::Normal) {
            if (ref.data(gs)->isClass()) {
                classCount++;
            } else {
                moduleCount++;
            }
        }
        if (ref.data(gs)->superClass().exists() && ref.data(gs)->superClass() != core::Symbols::todo()) {
            continue;
        }
        if (ref == core::Symbols::Sorbet_Private_Static_ImplicitModuleSuperClass()) {
            // only happens if we run without stdlib
            ENFORCE(!core::Symbols::Sorbet_Private_Static_ImplicitModuleSuperClass().data(gs)->loc().exists());
            ref.data(gs)->setSuperClass(core::Symbols::BasicObject());
            continue;
        }

        auto attached = ref.data(gs)->attachedClass(gs);
        bool isSingleton = attached.exists() && attached != core::Symbols::untyped();
        if (isSingleton) {
            singletonClassCount++;
            if (attached == core::Symbols::BasicObject()) {
                ref.data(gs)->setSuperClass(core::Symbols::Class());
            } else if (attached.data(gs)->superClass() ==
                       core::Symbols::Sorbet_Private_Static_ImplicitModuleSuperClass()) {
                // Note: this depends on attached classes having lower indexes in name table than their singletons
                ref.data(gs)->setSuperClass(core::Symbols::Module());
            } else {
                ENFORCE(attached.data(gs)->superClass() != core::Symbols::todo());
                auto singleton = attached.data(gs)->superClass().data(gs)->singletonClass(gs);
                ref.data(gs)->setSuperClass(singleton);
            }
        } else {
            if (ref.data(gs)->isClass()) {
                if (!core::Symbols::Object().data(gs)->derivesFrom(gs, ref) && core::Symbols::Object() != ref) {
                    ref.data(gs)->setSuperClass(core::Symbols::Object());
                }
            } else {
                if (!core::Symbols::BasicObject().data(gs)->derivesFrom(gs, ref) &&
                    core::Symbols::BasicObject() != ref) {
                    ref.data(gs)->setSuperClass(core::Symbols::Sorbet_Private_Static_ImplicitModuleSuperClass());
                }
            }
        }
    }

    prodCounterAdd("types.input.modules.total", moduleCount);
    prodCounterAdd("types.input.classes.total", classCount);
    prodCounterAdd("types.input.singleton_classes.total", singletonClassCount);
    prodCounterAdd("types.input.methods.total", methodCount);
}

void Resolver::finalizeSymbols(core::GlobalState &gs) {
    Timer timer(gs.tracer(), "resolver.finalize_resolution");
    // TODO(nelhage): Properly this first loop should go in finalizeAncestors,
    // but we currently compute mixes_in_class_methods during the same AST walk
    // that resolves types and we don't want to introduce additional passes if
    // we don't have to. It would be a tractable refactor to merge it
    // `ResolveConstantsWalk` if it becomes necessary to process earlier.
    for (uint32_t i = 1; i < gs.classAndModulesUsed(); ++i) {
        auto sym = core::ClassOrModuleRef(gs, i);

        if (sym.data(gs)->flags.isLinearizationComputed) {
            // Without this, the addMixin below for mixedInClassMethods is not idempotent on the
            // fast path, and will accidentally mix a `ClassMethods` module into all children (not
            // just the class that has the `include` triggering the mixes_in_class_methods, but all
            // subclasses of that class).
            continue;
        }

        core::ClassOrModuleRef singleton;
        for (auto ancst : sym.data(gs)->mixins()) {
            // Reading the fake property created in resolver#resolveClassMethodsJob(){}
            auto mixedInClassMethods = ancst.data(gs)->findMethod(gs, core::Names::mixedInClassMethods());
            if (!mixedInClassMethods.exists()) {
                continue;
            }
            if (!singleton.exists()) {
                singleton = sym.data(gs)->singletonClass(gs);
            }

            auto &resultType = mixedInClassMethods.data(gs)->resultType;
            ENFORCE(resultType != nullptr && core::isa_type<core::TupleType>(resultType));
            auto types = core::cast_type<core::TupleType>(resultType);

            for (auto &type : types->elems) {
                ENFORCE(core::isa_type<core::ClassType>(type));
                auto classType = core::cast_type_nonnull<core::ClassType>(type);
                if (!singleton.data(gs)->addMixin(gs, classType.symbol)) {
                    // Should never happen. We check in ResolveConstantsWalk that classMethods are a module before
                    // adding it as a member.
                    ENFORCE(false);
                }
            }
        }
    }

    gs.computeLinearization();

    vector<vector<pair<core::TypeMemberRef, core::TypeMemberRef>>> typeAliases;
    typeAliases.resize(gs.classAndModulesUsed());
    vector<bool> resolved;
    resolved.resize(gs.classAndModulesUsed());
    for (int i = 1; i < gs.classAndModulesUsed(); ++i) {
        auto sym = core::ClassOrModuleRef(gs, i);
        resolveTypeMembers(gs, sym, typeAliases, resolved);

        if (gs.requiresAncestorEnabled) {
            // Precompute the list of all required ancestors for this symbol
            sym.data(gs)->computeRequiredAncestorLinearization(gs);
        }
    }
}

} // namespace sorbet::resolver
