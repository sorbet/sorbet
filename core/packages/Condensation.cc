#include "core/packages/Condensation.h"
#include "common/strings/formatting.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::core::packages {

Condensation::Node &Condensation::pushNode(ImportType type, bool isPrelude) {
    auto id = this->nodes_.size();
    auto isTest = type != ImportType::Normal;
    auto &node = this->nodes_.emplace_back(id, isTest, isPrelude);
    return node;
}

namespace {

struct TraversalBuilder {
    absl::Span<const Condensation::Node> nodes;
    vector<uint32_t> remainingImports;
    vector<vector<uint32_t>> backEdges;

    vector<uint32_t> sccLengths;
    vector<uint32_t> stratumLengths;

    Condensation::Traversal result;

    TraversalBuilder(const core::GlobalState &gs, absl::Span<const Condensation::Node> nodes)
        : nodes{nodes}, remainingImports(this->nodes.size(), 0), backEdges(this->nodes.size()) {
        result.packages.reserve(gs.packageDB().packages().size());
    }

    // Return the set of all root packages, for the first stratum in the traversal.
    vector<uint32_t> roots() {
        vector<uint32_t> frontier;

        // Seed the needed imports for the traversal from the roots.
        for (auto &node : this->nodes) {
            this->remainingImports[node.id] = node.imports.size();

            if (node.imports.empty()) {
                frontier.emplace_back(node.id);
            }

            for (auto imp : node.imports) {
                this->backEdges[imp].push_back(node.id);
            }
        }

        return frontier;
    }

    void traverseFrom(absl::Span<const uint32_t> roots) {
        vector<uint32_t> frontier(roots.begin(), roots.end());
        vector<uint32_t> next;

        while (!frontier.empty()) {
            next.clear();

            stratumLengths.emplace_back(frontier.size());

            for (auto sccId : frontier) {
                auto &node = this->nodes[sccId];
                ENFORCE(remainingImports[node.id] == 0);

                // Insert the members of the SCC into the packages vector.
                absl::c_copy(node.members, back_inserter(result.packages));

                auto &scc = result.sccs.emplace_back();
                scc.isTest = node.isTest;
                sccLengths.emplace_back(node.members.size());

                // Queue up the dependents in the next frontier, decrementing their imports by one
                for (auto dep : backEdges[sccId]) {
                    auto &remaining = remainingImports[dep];

                    // Having a `remaining` value of <= 0 at this point implies that the scc should have been in the
                    // frontier already. This would only be possible if there were a cycle in the condensation graph,
                    // and it should be a DAG by construction.
                    ENFORCE(remaining > 0);
                    remaining -= 1;

                    // `remaining` should never go below zero (as edges are unique in the condensation graph), but as a
                    // defensive measure, we keep it signed and check for `<= 0` instead of `== 0` to guard against that
                    // case at runtime.
                    if (remaining <= 0) {
                        next.emplace_back(dep);
                    }
                }
            }

            // The content of `next` isn't important at this point, and it will be cleared on the next iteration of the
            // loop. Morally this is `frontier = next`, but we swap and clear instead to avoid any ambiguity about
            // reallocations.
            swap(frontier, next);
        }
    }

    Condensation::Traversal build() {
        // Fill in the scc member spans
        ENFORCE(this->result.sccs.size() == this->sccLengths.size());
        int i = -1;
        size_t offset = 0;
        for (auto length : this->sccLengths) {
            i++;
            this->result.sccs[i].members = absl::MakeSpan(this->result.packages).subspan(offset, length);
            offset += length;
        }

        // Fill in the stratum spans, now that the scc vector is fully defined
        this->result.strata.reserve(this->stratumLengths.size());
        offset = 0;
        for (auto length : this->stratumLengths) {
            this->result.strata.emplace_back(absl::MakeSpan(this->result.sccs).subspan(offset, length));
            offset += length;
        }

        return move(this->result);
    }
};

} // namespace

const Condensation::Traversal Condensation::computeTraversal(const core::GlobalState &gs) const {
    TraversalBuilder builder(gs, this->nodes_);

    // The SCCs that have no imports of their own.
    auto roots = builder.roots();

    builder.traverseFrom(roots);

    return builder.build();
}

UnorderedMap<MangledName, Condensation::Traversal::StratumInfo>
Condensation::Traversal::buildStratumMapping(const core::GlobalState &gs) const {
    UnorderedMap<MangledName, StratumInfo> result;

    int ix = -1;
    for (auto stratum : this->strata) {
        ++ix;
        for (auto &scc : stratum) {
            if (scc.isTest) {
                for (auto name : scc.members) {
                    result[name].testStratum = ix;
                }
            } else {
                for (auto name : scc.members) {
                    result[name].applicationStratum = ix;
                }
            }
        }
    }

    return result;
}

} // namespace sorbet::core::packages
