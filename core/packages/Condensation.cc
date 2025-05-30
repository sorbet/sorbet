#include "core/packages/Condensation.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::core::packages {

Condensation::Node &Condensation::pushNode(ImportType type) {
    auto id = this->nodes_.size();
    auto isTest = type != ImportType::Normal;
    auto &node = this->nodes_.emplace_back(id, isTest);
    return node;
}

const Condensation::Traversal Condensation::computeTraversal(const core::GlobalState &gs) const {
    Traversal result;

    // The nodes we're currently exploring, and will explore on the next iteration.
    vector<uint32_t> frontier;
    vector<uint32_t> next;

    vector<int> neededImports(this->nodes_.size(), 0);
    vector<vector<uint32_t>> backEdges(this->nodes_.size());

    // Seed the needed imports for the traversal from the roots.
    for (auto &node : this->nodes_) {
        neededImports[node.id] = node.imports.size();

        if (node.imports.empty()) {
            frontier.emplace_back(node.id);
        }

        for (auto imp : node.imports) {
            backEdges[imp].push_back(node.id);
        }
    }

    // The reservations for `result.packages` and `result.sccs` are important: we are constructing spans into both
    // vectors as we traverse the graph, and a reallocation will invalidate those spans.
    result.packages.reserve(2 * gs.packageDB().packages().size());
    result.sccs.reserve(this->nodes_.size());

    while (!frontier.empty()) {
        next.clear();
        auto parallelStart = result.sccs.size();
        for (auto sccId : frontier) {
            auto &node = this->nodes_[sccId];
            ENFORCE(neededImports[node.id] == 0);

            auto sccStart = result.packages.size();

            // Insert the members of the SCC into the packages vector.
            absl::c_copy(node.members, back_inserter(result.packages));

            auto &scc = result.sccs.emplace_back();
            scc.isTest = node.isTest;
            scc.members = absl::MakeSpan(result.packages).subspan(sccStart);

            // Queue up the dependents in the next frontier, decrementing their imports by one
            for (auto dep : backEdges[sccId]) {
                auto &needed = neededImports[dep];

                ENFORCE(needed > 0);
                needed -= 1;

                // `neededImports` should never go below zero (as edges are unique in the condensation graph), but as a
                // defensive measure, we keep it signed and handle that case at runtime.
                if (needed <= 0) {
                    next.emplace_back(dep);
                }
            }
        }

        auto parallelGroup = absl::MakeSpan(result.sccs).subspan(parallelStart);
        ENFORCE(parallelGroup.size() > 0, "No packages made it through this iteration of the topo sort");
        result.parallel.emplace_back(parallelGroup);

        // TODO: should we sort the parallelGroup span here to ensure a consistent traversal order?

        swap(frontier, next);
    }

    return result;
}
} // namespace sorbet::core::packages
