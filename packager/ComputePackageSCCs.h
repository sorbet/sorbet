#pragma once

#include "common/common.h"
#include "core/GlobalState.h"
#include "core/packages/Condensation.h"
#include "core/packages/MangledName.h"
#include <vector>

namespace sorbet::packager {

class Provider {
public:
    virtual std::vector<std::pair<core::packages::MangledName, core::packages::ImportType>>
    getImports(core::packages::MangledName packageName) = 0;
    virtual void setSCCId(core::packages::MangledName packageName, int SCCId) = 0;
    virtual int getSCCId(core::packages::MangledName packageName) = 0;
    virtual void setTestSCCId(core::packages::MangledName packageName, int SCCId) = 0;
};

class ComputePackageSCCs {
    core::GlobalState &gs;

    int nextIndex = 1;

    struct NodeInfo {
        static constexpr int UNVISITED = 0;

        int index = UNVISITED;
        int lowLink = 0;
        bool onStack = false;
    };

    UnorderedMap<core::packages::MangledName, NodeInfo> nodeMap;
    std::vector<core::packages::MangledName> stack;

    core::packages::Condensation condensation;

    ComputePackageSCCs(core::GlobalState &gs);

    template <core::packages::ImportType EdgeType>
    void strongConnect(core::packages::MangledName pkgName, NodeInfo &infoAtEntry, Provider &provider);

    template <core::packages::ImportType EdgeType> void tarjan(Provider &provider);

public:
    static core::packages::Condensation run(core::GlobalState &gs, Provider &provider);
};

} // namespace sorbet::packager
