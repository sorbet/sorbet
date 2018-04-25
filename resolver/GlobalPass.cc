#include "core/Names/resolver.h"
#include "core/core.h"
#include "core/errors/resolver.h"
#include "resolver/resolver.h"

#include <map>
#include <vector>

using namespace std;

namespace ruby_typer {
namespace resolver {

namespace {
bool resolveTypeMember(core::GlobalState &gs, core::SymbolRef parent, core::SymbolRef parentTypeMember,
                       core::SymbolRef sym) {
    core::NameRef name = parentTypeMember.data(gs).name;
    auto parentVariance = parentTypeMember.data(gs).variance();
    auto &inSym = sym.data(gs);
    core::SymbolRef my = inSym.findMember(gs, name);
    if (!my.exists()) {
        if (parent == core::Symbols::Enumerable() || parent.data(gs).derivesFrom(gs, core::Symbols::Enumerable())) {
            my = gs.enterTypeMember(inSym.definitionLoc, sym, name, parentVariance);
        } else {
            if (auto e = gs.beginError(inSym.definitionLoc, core::errors::Resolver::ParentTypeNotDeclared)) {
                e.setHeader("Type `{}` declared by parent `{}` should be declared again", name.toString(gs),
                            parent.data(gs).show(gs));
            }
            return false;
        }
    }
    auto &data = my.data(gs);
    if (!data.isTypeMember() && !data.isTypeArgument()) {
        if (auto e = gs.beginError(inSym.definitionLoc, core::errors::Resolver::NotATypeVariable)) {
            e.setHeader("Type variable `{}` needs to be declared as `= type_member(SOMETHING)`", name.toString(gs));
        }
        return false;
    }
    auto myVariance = data.variance();
    if (!inSym.derivesFrom(gs, core::Symbols::Class()) && (myVariance != parentVariance)) {
        // this requirement can be loosened. You can go from variant to invariant.
        if (auto e = gs.beginError(data.definitionLoc, core::errors::Resolver::ParentVarianceMismatch)) {
            e.setHeader("Type variance mismatch with parent `{}`", parent.data(gs).show(gs));
        }
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
                auto loc = tp.data(gs).definitionLoc;
                if (!loc.file.data(gs).isPayload()) {
                    if (auto e = gs.beginError(tp.data(gs).definitionLoc,
                                               core::errors::Resolver::VariantTypeMemberInClass)) {
                        e.setHeader("Classes can only have invariant type members");
                    }
                    return;
                }
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
                    if (auto e =
                            gs.beginError(my.data(gs).definitionLoc, core::errors::Resolver::TypeMembersInWrongOrder)) {
                        e.setHeader("Type members in wrong order");
                    }
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

    for (core::SymbolRef mixin : inSym.mixins()) {
        for (core::SymbolRef tp : mixin.data(gs).typeMembers()) {
            resolveTypeMember(gs, mixin, tp, sym);
        }
    }

    // TODO: this will be the right moment to implement checks for correct locations of co&contra variant types.
}

void validateAbstract(core::GlobalState &gs, unordered_map<core::SymbolRef, vector<core::SymbolRef>> &abstractCache,
                      core::SymbolRef sym) {
    vector<core::SymbolRef> abstract;

    auto superclass = sym.data(gs).superClass;
    auto fnd = abstractCache.find(superclass);
    if (fnd != abstractCache.end()) {
        abstract.insert(abstract.end(), fnd->second.begin(), fnd->second.end());
    }
    for (auto ancst : sym.data(gs).mixins()) {
        auto fnd = abstractCache.find(ancst);
        if (fnd != abstractCache.end()) {
            abstract.insert(abstract.end(), fnd->second.begin(), fnd->second.end());
        }
    }

    auto isAbstract = sym.data(gs).isClassAbstract();
    if (isAbstract) {
        for (auto mem : sym.data(gs).members) {
            if (mem.second.data(gs).isMethod() && mem.second.data(gs).isAbstract()) {
                abstract.push_back(mem.second);
            }
        }
    }

    if (abstract.empty()) {
        return;
    }

    for (auto proto : abstract) {
        if (proto.data(gs).owner == sym) {
            continue;
        }

        auto mem = sym.data(gs).findConcreteMethodTransitive(gs, proto.data(gs).name);
        if (!isAbstract && !mem.exists()) {
            if (auto e = gs.beginError(sym.data(gs).definitionLoc, core::errors::Resolver::BadAbstractMethod)) {
                e.setHeader("Missing definition for abstract method `{}`", proto.data(gs).show(gs));
                e.addErrorLine(proto.data(gs).definitionLoc, "defined here");
            }
        }
    }

    abstractCache[sym] = move(abstract);
}
}; // namespace

void Resolver::finalizeResolution(core::GlobalState &gs) {
    int methodCount = 0;
    int classCount = 0;
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto &data = core::SymbolRef(&gs, i).data(gs);
        if (data.definitionLoc.file.exists() &&
            data.definitionLoc.file.data(gs).source_type == core::File::Type::Normal) {
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
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto sym = core::SymbolRef(&gs, i);
        if (sym.data(gs).isClass()) {
            resolveTypeMembers(gs, sym);
        }
    }

    unordered_map<core::SymbolRef, vector<core::SymbolRef>> abstractCache;

    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto sym = core::SymbolRef(&gs, i);
        if (sym.data(gs).isClass()) {
            validateAbstract(gs, abstractCache, sym);
        }
    }
    core::prodCounterAdd("types.input.classes.total", classCount);
    core::prodCounterAdd("types.input.methods.total", methodCount);
}
}; // namespace resolver

} // namespace ruby_typer
