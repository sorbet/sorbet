#ifndef SORBET_CORE_PACKAGES_CONDENSATION_H
#define SORBET_CORE_PACKAGES_CONDENSATION_H

#include "PackageInfo.h"
#include "absl/types/span.h"
#include "common/common.h"
#include "core/packages/MangledName.h"
#include <string>
#include <utility>
#include <vector>

namespace sorbet::core {
class GlobalState;
}

namespace sorbet::core::packages {

class Condensation {
public:
    struct Node {
        std::vector<core::packages::MangledName> members;

        // The SCC IDs of the packages this SCC depends on.
        UnorderedSet<int> imports;

        // The SCC ID of this node.
        const int id = 0;

        // Whether or not this is a node of application or test code.
        const bool isTest = false;

        Node(int id, bool isTest) : id{id}, isTest{isTest} {}
    };

private:
    // SCC ids in packages will be valid indices into this vector when the traversals of the package graph have
    // completed.
    std::vector<Node> nodes_;

public:
    Condensation() = default;

    Condensation(const Condensation &other) = delete;
    Condensation(Condensation &&other) = default;

    Condensation &operator=(const Condensation &other) = delete;
    Condensation &operator=(Condensation &&other) = default;

    Node &pushNode(ImportType type);

    struct Traversal {
        // Packages ordered according to their dependencies in the package graph. It's safe to traverse this vector, and
        // assume that when processing a package all of its imports will have been seen already. Packages will occur
        // at least once and at most twice in this vector, with the second occurrence always being for the test code of
        // a package.
        std::vector<MangledName> packages;

        struct SCCInfo {
            // True when this group of packages represents a cycle of test dependencies.
            bool isTest = false;

            // A span of `this->packages` that's involved in this SCC.
            absl::Span<MangledName> members;
        };

        // Spans of `this->packages` that correspond to an SCC.
        std::vector<SCCInfo> sccs;

        // Spans of `this->sccs` that have no import conflicts, and could go through the pipeline in parallel.
        std::vector<absl::Span<SCCInfo>> parallel;

        Traversal() = default;

        Traversal(const Traversal &other) = delete;
        Traversal(Traversal &&other) = default;

        Traversal &operator=(const Traversal &other) = default;
        Traversal &operator=(Traversal &&other) = default;
    };

    // Compute a traversal through the condensation graph, that yields groups of SCCs that have no dependencies on each
    // other. These groups are acceptable to typecheck in parallel, under the assumption that the package graph is
    // well-formed.
    const Traversal computeTraversal(const GlobalState &gs) const;

    absl::Span<const Node> nodes() const {
        return this->nodes_;
    }
};

} // namespace sorbet::core::packages

#endif
