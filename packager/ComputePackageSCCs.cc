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

template <core::packages::ImportType EdgeType>
void ComputePackageSCCs::strongConnect(core::packages::MangledName pkgName, NodeInfo &infoAtEntry, Provider &provider) {
    auto &packageDB = gs.packageDB();
    if (!packageDB.getPackageInfo(pkgName).exists()) {
        return;
    }

    infoAtEntry.index = this->nextIndex;
    infoAtEntry.lowLink = this->nextIndex;
    this->nextIndex++;
    this->stack.push_back(pkgName);
    infoAtEntry.onStack = true;

    for (auto [name, type] : provider.getImports(pkgName)) {
        if constexpr (EdgeType == core::packages::ImportType::Normal) {
            if (type != core::packages::ImportType::Normal) {
                continue;
            }
        }
        auto &importInfo = this->nodeMap[name];
        if (importInfo.index == NodeInfo::UNVISITED) {
            this->strongConnect<EdgeType>(name, importInfo, provider);

            auto &importInfo = this->nodeMap[name];
            if (importInfo.index == NodeInfo::UNVISITED) {
                continue;
            }
            auto &pkgLink = this->nodeMap[pkgName].lowLink;
            pkgLink = std::min(pkgLink, importInfo.lowLink);
        } else if (importInfo.onStack) {
            auto &pkgLink = this->nodeMap[pkgName].lowLink;
            pkgLink = std::min(pkgLink, importInfo.index);
        }
    }

    auto &ourInfo = this->nodeMap[pkgName];
    if (ourInfo.index == ourInfo.lowLink) {
        core::packages::MangledName poppedPkgName;
        auto &condensationNode = this->condensation.pushNode(EdgeType);
        auto sccId = condensationNode.id;

        do {
            poppedPkgName = this->stack.back();
            condensationNode.members.push_back(poppedPkgName);
            this->stack.pop_back();
            this->nodeMap[poppedPkgName].onStack = false;

            if constexpr (EdgeType == core::packages::ImportType::Normal) {
                provider.setSCCId(poppedPkgName, sccId);
            } else if constexpr (EdgeType != core::packages::ImportType::Normal) {
                provider.setTestSCCId(poppedPkgName, sccId);

                auto appSccId = provider.getSCCId(poppedPkgName);
                condensationNode.imports.insert(appSccId);
            }
        } while (poppedPkgName != pkgName);

        for (auto name : condensationNode.members) {
            for (auto [name, type] : provider.getImports(name)) {
                if constexpr (EdgeType == core::packages::ImportType::Normal) {
                    if (type != core::packages::ImportType::Normal) {
                        continue;
                    }
                }

                if (!name.exists()) {
                    continue;
                }

                auto impId = provider.getSCCId(name);
                if (impId == sccId) {
                    continue;
                }

                condensationNode.imports.insert(impId);
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
