#ifndef SORBET_PACKAGER_COMPUTE_PACKAGE_SCCS_H
#define SORBET_PACKAGER_COMPUTE_PACKAGE_SCCS_H

#include "common/common.h"
#include "core/GlobalState.h"
#include "core/packages/Condensation.h"
#include "core/packages/MangledName.h"

namespace sorbet::packager {

class ComputePackageSCCs {
    core::GlobalState &gs;

    int nextIndex = 1;

    // Metadata for Tarjan's algorithm
    // https://www.cs.cmu.edu/~15451-f18/lectures/lec19-DFS-strong-components.pdf provides a good overview of the
    // algorithm.
    struct NodeInfo {
        static constexpr int UNVISITED = 0;

        // A given package's index in the DFS traversal; ie. when it was first visited. The default value of 0 means the
        // package hasn't been visited yet.
        int index = UNVISITED;
        // The lowest index reachable from a given package (in the same SCC) by following any number of tree edges
        // and at most one back/cross edge
        int lowLink = 0;
        // Fast way to check if a package is on the stack
        bool onStack = false;
    };

    UnorderedMap<core::packages::MangledName, NodeInfo> nodeMap;
    // As we visit packages, we push them onto the stack. Once we find the "root" of an SCC, we can use the stack to
    // determine all packages in the SCC.
    std::vector<core::packages::MangledName> stack;

    core::packages::Condensation condensation;

    ComputePackageSCCs(core::GlobalState &gs) : gs{gs} {
        auto numPackages = gs.packageDB().packages().size();
        this->stack.reserve(numPackages);
        this->nodeMap.reserve(numPackages);
    }

    // DFS traversal for Tarjan's algorithm starting from pkgName, along with keeping track of some metadata needed for
    // detecting SCCs.
    template <core::packages::ImportType EdgeType, typename P>
    void strongConnect(core::packages::MangledName pkgName, NodeInfo &infoAtEntry, P &packageGraph) {
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

        for (auto &i : packageGraph.getImports(pkgName)) {
            // We want to consider all imports from test code, but only normal imports for application code.
            if constexpr (EdgeType == core::packages::ImportType::Normal) {
                if (i.type != core::packages::ImportType::Normal) {
                    continue;
                }
            }
            // We need to be careful with this; it's not valid after a call to `strongConnect`,
            // because our reference might disappear from underneath us during that call.
            auto &importInfo = this->nodeMap[i.mangledName];
            if (importInfo.index == NodeInfo::UNVISITED) {
                // This is a tree edge (ie. a forward edge that we haven't visited yet).
                this->strongConnect<EdgeType>(i.mangledName, importInfo, packageGraph);

                // Need to re-lookup for the reason above.
                auto &importInfo = this->nodeMap[i.mangledName];
                if (importInfo.index == NodeInfo::UNVISITED) {
                    // This is to handle early return above.
                    continue;
                }
                // Since we can follow any number of tree edges for lowLink, the lowLink of child is valid for this
                // package too.
                //
                // Note that we cannot use `infoAtEntry` here because it might have been invalidated.
                auto &pkgLink = this->nodeMap[pkgName].lowLink;
                pkgLink = std::min(pkgLink, importInfo.lowLink);
            } else if (importInfo.onStack) {
                // This is a back edge (edge to ancestor) or cross edge (edge to a different subtree). Since we can only
                // follow at most one back/cross edge, the best update we can make to lowlink of the current package is
                // the child's index.
                //
                // Note that we cannot use `infoAtEntry` here because it might have been invalidated.
                auto &pkgLink = this->nodeMap[pkgName].lowLink;
                pkgLink = std::min(pkgLink, importInfo.index);
            }
            // If the child package is already visited and not on the stack, it's in a different SCC, so no update to
            // the lowlink.
        }

        // We cannot re-use `infoAtEntry` here because `nodeMap` might have been re-allocated and
        // invalidate our reference.
        auto &ourInfo = this->nodeMap[pkgName];
        if (ourInfo.index == ourInfo.lowLink) {
            // This is the root of an SCC. This means that all packages on the stack from this package to the top of the
            // top of the stack are in the same SCC. Pop the stack until we reach the root of the SCC, and assign them
            // the same SCC ID.
            core::packages::MangledName poppedPkgName;
            auto &condensationNode =
                this->condensation.pushNode(EdgeType, packageDB.getPackageInfo(pkgName).isPreludePackage());
            auto sccId = condensationNode.id;

            // This SCC's layer, if all packages in the SCC have the same one.
            // TODO: handle the case where packager is enabled but layering is not
            std::optional<core::NameRef> condensationNodeLayer = std::nullopt;
            {
                auto firstPkgLayer = packageDB.getPackageInfo(this->stack.back()).layer;
                if (firstPkgLayer.exists()) {
                    condensationNodeLayer = firstPkgLayer;
                }
            }

            // Set the SCC ids for all of the members of the SCC
            do {
                poppedPkgName = this->stack.back();
                condensationNode.members.push_back(poppedPkgName);
                this->stack.pop_back();
                this->nodeMap[poppedPkgName].onStack = false;

                if constexpr (EdgeType == core::packages::ImportType::Normal) {
                    packageGraph.setSCCId(poppedPkgName, sccId);

                    if (condensationNodeLayer.has_value()) {
                        auto thisPkgLayer = packageDB.getPackageInfo(poppedPkgName).layer;
                        if (!thisPkgLayer.exists() || thisPkgLayer != condensationNodeLayer.value()) {
                            condensationNodeLayer = std::nullopt;
                        }
                    }
                } else if constexpr (EdgeType != core::packages::ImportType::Normal) {
                    packageGraph.setTestSCCId(poppedPkgName, sccId);

                    // If this is a test-only package, we also set the non-test SCC id to this same id.
                    if (gs.packageDB().getPackageInfo(poppedPkgName).file.isTestPackage(gs)) {
                        packageGraph.setSCCId(poppedPkgName, sccId);
                    } else {
                        // However, if this is the test package for application code, we add an implicit dependency on
                        // the application SCC. Those scc ids must exist at this point, as we've already traversed all
                        // packages once.
                        auto appSccId = packageGraph.getSCCId(poppedPkgName);
                        condensationNode.imports.insert(appSccId);
                    }
                }
            } while (poppedPkgName != pkgName);
            condensationNode.layer = condensationNodeLayer;

            if (!condensationNode.layer.has_value()) {
                // SCC has multiple layers, which implies layering violation
                condensationNode.bestStrictness = core::packages::StrictDependenciesLevel::False;
            } else if (condensationNode.members.size() > 1) {
                // More than one package in SCC, which implies cycle
                condensationNode.bestStrictness = core::packages::StrictDependenciesLevel::Layered;
            }

            // Iterate the imports of each member, building the edges of the condensation. This step is performed after
            // we've visited all members of the SCC once, to ensure that their ids have all been populated.
            for (auto name : condensationNode.members) {
                for (auto &i : packageGraph.getImports(name)) {
                    // The mangled name won't exist if the import was to a package that doesn't exist.
                    if (!i.mangledName.exists()) {
                        continue;
                    }

                    int impId = 0;

                    if constexpr (EdgeType == core::packages::ImportType::Normal) {
                        if (i.type != core::packages::ImportType::Normal) {
                            continue;
                        }
                        impId = packageGraph.getSCCId(i.mangledName);
                    } else {
                        // If we're processing imports from a test package, we can't tell if those imports are used only
                        // at application time, or application and test time. Conservatively, we consider the import to
                        // be to the test package, which transitively depends on the application code, ensuring the
                        // correct ordering for test packages.
                        impId = packageGraph.getTestSCCId(i.mangledName);
                    }

                    if (impId == sccId) {
                        continue;
                    }

                    condensationNode.imports.insert(impId);
                }
            }

            for (auto impSccId : condensationNode.imports) {
                auto &impScc = this->condensation.nodes()[impSccId];
                if (impScc.bestStrictness == core::packages::StrictDependenciesLevel::False) {
                    // imports a strict_dependencies 'false' package
                    condensationNode.bestStrictness = core::packages::StrictDependenciesLevel::False;
                } else if (impScc.layer.has_value() && condensationNode.layer.has_value() &&
                           packageDB.layerIndex(condensationNode.layer.value()) <
                               packageDB.layerIndex(impScc.layer.value())) {
                    // layering violation
                    condensationNode.bestStrictness = core::packages::StrictDependenciesLevel::False;
                } else if (condensationNode.bestStrictness == core::packages::StrictDependenciesLevel::Dag &&
                           impScc.bestStrictness != core::packages::StrictDependenciesLevel::Dag) {
                    // imports a package that is strict_dependencies 'layered' or 'layered_dag'
                    condensationNode.bestStrictness = core::packages::StrictDependenciesLevel::LayeredDag;
                }
            }
        }
    }

    // Tarjan's algorithm for finding strongly connected components
    template <core::packages::ImportType EdgeType, typename P> void tarjan(P &packageGraph) {
        this->nodeMap.clear();
        ENFORCE(this->stack.empty());
        for (auto package : gs.packageDB().packages()) {
            // We skip test-only packages when we're processing application edges.
            if constexpr (EdgeType == core::packages::ImportType::Normal) {
                if (gs.packageDB().getPackageInfo(package).file.isTestPackage(gs)) {
                    continue;
                }
            }

            auto &info = this->nodeMap[package];
            if (info.index == NodeInfo::UNVISITED) {
                this->strongConnect<EdgeType>(package, info, packageGraph);
            }
        }
    }

public:
    // NOTE: This function must be called every time an import is added or removed from a package.
    // It is relatively fast, so calling it on every __package.rb edit is an okay overapproximation for simplicity.
    template <typename P> static core::packages::Condensation run(core::GlobalState &gs, P &packageGraph) {
        Timer timeit(gs.tracer(), "packager::computeSCCs");
        ComputePackageSCCs scc(gs);

        // First, compute the SCCs for application code, and then for test code. This allows us to have more granular
        // SCCs, as test_import edges aren't subject to the same restrictions that import edges are.
        scc.tarjan<core::packages::ImportType::Normal>(packageGraph);
        scc.tarjan<core::packages::ImportType::TestHelper>(packageGraph);

        return std::move(scc.condensation);
    }
};

} // namespace sorbet::packager

#endif
