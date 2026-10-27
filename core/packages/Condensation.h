#ifndef SORBET_CORE_PACKAGES_CONDENSATION_H
#define SORBET_CORE_PACKAGES_CONDENSATION_H

#include "PackageInfo.h"
#include "absl/types/span.h"
#include "common/common.h"
#include "core/packages/MangledName.h"
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace sorbet::core {
class GlobalState;
}

namespace sorbet::core::packages {

class CondensationBuilder;

class Condensation {
    friend class CondensationBuilder;

public:
    struct Node {
        // Sorted by mangled name when the condensation is built.
        std::vector<core::packages::MangledName> members;

        // The SCC IDs of the packages this SCC depends on.
        UnorderedSet<int> imports;

        // The SCC IDs of the packages that depend on this SCC.
        std::vector<int> backEdges;

        // The SCC ID of this node.
        const int id = 0;

        // Whether or not this is a node of application or test code.
        const bool isTest = false;

        // True when this node contains prelude packages.
        const bool isPrelude = false;

        Node(int id, bool isTest, bool isPrelude) : id{id}, isTest{isTest}, isPrelude{isPrelude} {}
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

    struct Traversal {
        // Packages ordered according to their dependencies in the package graph. It's safe to traverse this vector, and
        // assume that when processing a package all of its imports will have been seen already. Packages will occur
        // at least once and at most twice in this vector, with the second occurrence always being for the test code of
        // a package.
        std::vector<MangledName> packages;

        struct SCCInfo {
            // True when this group of packages represents a cycle of test dependencies.
            bool isTest = false;

            // A span of `this->packages` that's involved in this SCC, sorted by mangled name.
            absl::Span<MangledName> members;
        };

        // Spans of `this->packages` that correspond to an SCC. Within each stratum, application SCCs precede test
        // SCCs, and each group is ordered lexicographically by member mangled names.
        std::vector<SCCInfo> sccs;

        // Spans of `this->sccs` that have no import conflicts, and could go through the pipeline in the same stratum.
        std::vector<absl::Span<SCCInfo>> strata;

        // All dependencies of the focused packages.
        UnorderedSet<MangledName> upstream;

        // All dependants of the focused packages.
        UnorderedSet<MangledName> downstream;

        Traversal() = default;

        Traversal(const Traversal &other) = delete;
        Traversal(Traversal &&other) = default;

        Traversal &operator=(const Traversal &other) = default;
        Traversal &operator=(Traversal &&other) = default;

        // Record the application and test source stratum of each package in PackageInfo.
        void recordStrata(core::GlobalState &gs) const;
    };

    // Compute a traversal through the condensation graph, that yields groups of SCCs that have no dependencies on each
    // other. These groups are acceptable to typecheck in the same stratum, under the assumption that the package graph
    // is well-formed.
    const Traversal computeTraversal(const GlobalState &gs, const UnorderedSet<MangledName> *focusedPackages,
                                     bool skipDownstream) const;

    absl::Span<const Node> nodes() const {
        return this->nodes_;
    }

    // Fetch the set of packages that depend on the set of packages provided.
    UnorderedSet<MangledName> transitiveDependentsOf(const PackageDB &db,
                                                     const UnorderedSet<MangledName> &packages) const;
};

class CondensationBuilder {
    Condensation condensation;

public:
    Condensation::Node &pushNode(ImportType type, bool isPrelude);

    Condensation build() &&;
};

} // namespace sorbet::core::packages

#endif
