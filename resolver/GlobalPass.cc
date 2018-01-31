#include "core/core.h"
#include "core/errors/resolver.h"
#include "resolver/resolver.h"

namespace ruby_typer {
namespace resolver {

bool resolveTypeMember(core::GlobalState &gs, core::SymbolRef parent, core::SymbolRef parentTypeMember,
                       core::SymbolRef sym) {
    core::NameRef name = parentTypeMember.data(gs).name;
    auto parentVariance = parentTypeMember.data(gs).variance();
    auto &inSym = sym.data(gs);
    core::SymbolRef my = inSym.findMember(gs, name);
    if (!my.exists()) {
        gs.error(inSym.definitionLoc, core::errors::Resolver::ParentTypeNotDeclared,
                 "Type {} declared by parent {} should be declared again", name.toString(gs),
                 parent.data(gs).fullName(gs));
        return false;
    }
    auto myVariance = my.data(gs).variance();
    if (!inSym.derivesFrom(gs, core::Symbols::Class()) && (myVariance != parentVariance)) {
        // this requirement can be loosened. You can go from variant to invariant.
        gs.error(my.data(gs).definitionLoc, core::errors::Resolver::ParentVarianceMismatch,
                 "Type variance mismatch with parent {}", parent.data(gs).fullName(gs));
        return true;
    }
    inSym.typeAliases.emplace_back(parentTypeMember, my);
    return true;
}

void resolveTypeMembers(core::GlobalState &gs, core::SymbolRef sym) {
    auto &inSym = sym.data(gs);
    ENFORCE(inSym.isClass());
    if (inSym.isClassClass()) {
        for (core::SymbolRef tp : inSym.typeMembers()) {
            auto myVariance = tp.data(gs).variance();
            if (myVariance != core::Variance::Invariant) {
                gs.error(tp.data(gs).definitionLoc, core::errors::Resolver::VariantTypeMemberInClass,
                         "Classes can only have invariant type members");
                return;
            }
        }
    }

    if (inSym.superClass.exists()) {
        auto parent = inSym.superClass;
        auto tps = parent.data(gs).typeMembers();
        bool foundAll = true;
        for (core::SymbolRef tp : tps) {
            bool foundThis = resolveTypeMember(gs, parent, tp, sym);
            foundAll = foundAll && foundThis;
        }
        if (foundAll) {
            int i = 0;
            // check that type params are in the same order.
            for (core::SymbolRef tp : tps) {
                core::SymbolRef my = tp.dealiasAt(gs, sym);
                ENFORCE(my.exists(), "resolver failed to register type member aliases");
                if (inSym.typeMembers()[i] != my) {
                    gs.error(my.data(gs).definitionLoc, core::errors::Resolver::TypeMembersInWrongOrder,
                             "Type members in wrong order");
                    int foundIdx = 0;
                    while (foundIdx < inSym.typeMembers().size() && inSym.typeMembers()[foundIdx] != my) {
                        foundIdx++;
                    }
                    ENFORCE(foundIdx < inSym.typeMembers().size());
                    // quadratic
                    std::swap(inSym.typeMembers()[foundIdx], inSym.typeMembers()[i]);
                }
                i++;
            }
        }
    }

    for (core::SymbolRef mixin : inSym.mixins(gs)) {
        for (core::SymbolRef tp : mixin.data(gs).typeMembers()) {
            resolveTypeMember(gs, mixin, tp, sym);
        }
    }

    // TODO: this will be the right moment to implement checks for correct locations of co&contra variant types.
}

void Resolver::finalizeResolution(core::GlobalState &gs) {
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto &data = core::SymbolRef(&gs, i).data(gs);
        if (!data.isClass()) {
            continue;
        }
        if (!data.isClassModuleSet()) {
            // we did not see a declaration for this type not did we see it used. Default to module.
            data.setIsModule(true);
        }
        if (data.superClass != core::Symbols::todo()) {
            continue;
        }

        auto attached = data.attachedClass(gs);
        bool isSingleton = attached.exists() && attached != core::Symbols::untyped();
        if (isSingleton) {
            if (attached == core::Symbols::BasicObject()) {
                data.superClass = core::Symbols::Class();
            } else if (!attached.data(gs).superClass.exists()) {
                data.superClass = core::Symbols::Module();
            } else {
                ENFORCE(attached.data(gs).superClass != core::Symbols::todo());
                data.superClass = attached.data(gs).superClass.data(gs).singletonClass(gs);
            }
        } else {
            data.superClass = core::Symbols::Object();
        }
    }
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto sym = core::SymbolRef(&gs, i);
        if (sym.data(gs).isClass()) {
            resolveTypeMembers(gs, sym);
        }
    }
}

} // namespace resolver
} // namespace ruby_typer
