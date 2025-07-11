#include "packager/ComputePackageSCCs.h"
#include "core/packages/PackageInfo.h"
#include <algorithm>

using namespace std;

namespace sorbet::packager {

ComputePackageSCCs::ComputePackageSCCs(core::GlobalState &gs) : gs{gs} {
    auto numPackages = gs.packageDB().packages().size();
    this->stack.reserve(numPackages);
    this->nodeMap.reserve(numPackages);
}

// DFS traversal for Tarjan's algorithm starting from pkgName, along with keeping track of some metadata needed for
// detecting SCCs.
template <core::packages::ImportType EdgeType>
void ComputePackageSCCs::strongConnect(core::packages::MangledName pkgName, NodeInfo &infoAtEntry, Provider &provider) {
    auto &packageDB = gs.packageDB();
    if (!packageDB.getPackageInfo(pkgName).exists()) {
        // This is to handle the case where the user imports a package that doesn't exist.
        return;
    }

    infoAtEntry.index = this->nextIndex;
    infoAtEntry.lowLink = this->nextIndex;
    this->nextIndex++;
    this->stack.push_back(pkgName);
    infoAtEntry.onStack = true;

    for (auto [name, type] : provider.getImports(pkgName)) {
        // We want to consider all imports from test code, but only normal imports for application code.
        if constexpr (EdgeType == core::packages::ImportType::Normal) {
            if (type != core::packages::ImportType::Normal) {
                continue;
            }
        }
        // We need to be careful with this; it's not valid after a call to `strongConnect`,
        // because our reference might disappear from underneath us during that call.
        auto &importInfo = this->nodeMap[name];
        if (importInfo.index == NodeInfo::UNVISITED) {
            // This is a tree edge (ie. a forward edge that we haven't visited yet).
            this->strongConnect<EdgeType>(name, importInfo, provider);

            // Need to re-lookup for the reason above.
            auto &importInfo = this->nodeMap[name];
            if (importInfo.index == NodeInfo::UNVISITED) {
                // This is to handle early return above.
                continue;
            }
            // Since we can follow any number of tree edges for lowLink, the lowLink of child is valid for this package
            // too.
            //
            // Note that we cannot use `infoAtEntry` here because it might have been invalidated.
            auto &pkgLink = this->nodeMap[pkgName].lowLink;
            pkgLink = std::min(pkgLink, importInfo.lowLink);
        } else if (importInfo.onStack) {
            // This is a back edge (edge to ancestor) or cross edge (edge to a different subtree). Since we can only
            // follow at most one back/cross edge, the best update we can make to lowlink of the current package is the
            // child's index.
            //
            // Note that we cannot use `infoAtEntry` here because it might have been invalidated.
            auto &pkgLink = this->nodeMap[pkgName].lowLink;
            pkgLink = std::min(pkgLink, importInfo.index);
        }
        // If the child package is already visited and not on the stack, it's in a different SCC, so no update to the
        // lowlink.
    }

    // We cannot re-use `infoAtEntry` here because `nodeMap` might have been re-allocated and invalidate our reference.
    auto &ourInfo = this->nodeMap[pkgName];
    if (ourInfo.index == ourInfo.lowLink) {
        // This is the root of an SCC. This means that all packages on the stack from this package to the top of the top
        // of the stack are in the same SCC. Pop the stack until we reach the root of the SCC, and assign them the same
        // SCC ID.
        core::packages::MangledName poppedPkgName;
        auto &condensationNode = this->condensation.pushNode(EdgeType);
        auto sccId = condensationNode.id;
        // This SCC layer, if all packages in the SCC have the same one.
        optional<core::NameRef> condensationNodeLayer = nullopt;
        auto firstPkgLayer = packageDB.getPackageInfo(this->stack.back()).layer();
        if (firstPkgLayer.has_value()) {
            condensationNodeLayer = firstPkgLayer.value().first;
        }

        // Set the SCC ids for all of the members of the SCC
        do {
            poppedPkgName = this->stack.back();
            condensationNode.members.push_back(poppedPkgName);
            this->stack.pop_back();
            this->nodeMap[poppedPkgName].onStack = false;

            if constexpr (EdgeType == core::packages::ImportType::Normal) {
                provider.setSCCId(poppedPkgName, sccId);
                if (condensationNodeLayer.has_value()) {
                    auto thisPkgLayer = packageDB.getPackageInfo(poppedPkgName).layer();
                    if (thisPkgLayer.has_value()) {
                        if (thisPkgLayer.value().first != condensationNodeLayer.value()) {
                            condensationNodeLayer = nullopt;
                        }
                    }
                }
            } else if constexpr (EdgeType != core::packages::ImportType::Normal) {
                provider.setTestSCCId(poppedPkgName, sccId);

                // Tests have an implicit dependency on their package's application code. Those scc ids must
                // exist at this point, as we've already traversed all packages once.
                auto appSccId = provider.getSCCId(poppedPkgName);
                condensationNode.imports.insert(appSccId);
            }
        } while (poppedPkgName != pkgName);

        if (!condensationNodeLayer.has_value()) {
            condensationNode.bestStrictness = core::packages::StrictDependenciesLevel::False;
        } else if (condensationNode.members.size() > 1) {
            condensationNode.bestStrictness = core::packages::StrictDependenciesLevel::Layered;
        }

        // Iterate the imports of each member, building the edges of the condensation. This step is performed after
        // we've visited all members of the SCC once, to ensure that their ids have all been populated.
        for (auto name : condensationNode.members) {
            for (auto [name, type] : provider.getImports(name)) {
                // We want to consider all imports from test code, but only normal imports for application code.
                if constexpr (EdgeType == core::packages::ImportType::Normal) {
                    if (type != core::packages::ImportType::Normal) {
                        continue;
                    }
                }

                // The mangled name won't exist if the import was to a package that doesn't exist.
                if (!name.exists()) {
                    continue;
                }

                // All of the imports of every member of the SCC will have been processed in the recursive step, so
                // we can assume the scc id of the target exists. Additionally, all imports are to the original
                // application code, which is why we don't consider using the `testSccID` here.
                auto impId = provider.getSCCId(name);
                if (impId == sccId) {
                    continue;
                }

                condensationNode.imports.insert(impId);
                auto &impScc = this->condensation.nodes()[impId];
                // TODO: check for layering violations
                if (impScc.bestStrictness == core::packages::StrictDependenciesLevel::False) {
                    condensationNode.bestStrictness = core::packages::StrictDependenciesLevel::False;
                } else if (condensationNode.bestStrictness == core::packages::StrictDependenciesLevel::Dag &&
                           impScc.bestStrictness != core::packages::StrictDependenciesLevel::Dag) {
                    condensationNode.bestStrictness = core::packages::StrictDependenciesLevel::LayeredDag;
                }
            }
        }
    }
}

template <core::packages::ImportType EdgeType> void ComputePackageSCCs::tarjan(Provider &provider) {
    this->nodeMap.clear();
    ENFORCE(this->stack.empty());
    for (auto package : gs.packageDB().packages()) {
        auto &info = this->nodeMap[package];
        if (info.index == NodeInfo::UNVISITED) {
            this->strongConnect<EdgeType>(package, info, provider);
        }
    }
}

core::packages::Condensation ComputePackageSCCs::run(core::GlobalState &gs, Provider &provider) {
    Timer timeit(gs.tracer(), "packager::computeSCCs");
    ComputePackageSCCs scc(gs);

    scc.tarjan<core::packages::ImportType::Normal>(provider);
    scc.tarjan<core::packages::ImportType::TestHelper>(provider);

    return move(scc.condensation);
}

template void ComputePackageSCCs::strongConnect<core::packages::ImportType::Normal>(core::packages::MangledName,
                                                                                    NodeInfo &, Provider &);
template void ComputePackageSCCs::strongConnect<core::packages::ImportType::TestHelper>(core::packages::MangledName,
                                                                                        NodeInfo &, Provider &);
template void ComputePackageSCCs::tarjan<core::packages::ImportType::Normal>(Provider &);
template void ComputePackageSCCs::tarjan<core::packages::ImportType::TestHelper>(Provider &);

} // namespace sorbet::packager
