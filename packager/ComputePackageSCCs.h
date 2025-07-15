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

    ComputePackageSCCs(core::GlobalState &gs);

    template <core::packages::ImportType EdgeType>
    void strongConnect(core::packages::MangledName pkgName, NodeInfo &infoAtEntry, Provider &provider);

    template <core::packages::ImportType EdgeType> void tarjan(Provider &provider);

public:
    static core::packages::Condensation run(core::GlobalState &gs, Provider &provider);
};

} // namespace sorbet::packager
