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

    struct Roots {
        // All the prelude package SCCs.
        vector<uint32_t> prelude;

        // The roots of the rest of the graph.
        vector<uint32_t> roots;
    };

    // Return the set of all root packages, for the first stratum in the traversal.
    Roots roots() {
        Roots result;

        // Seed the needed imports for the traversal from the roots.
        for (auto &node : this->nodes) {
            // Prelude packages behave a little differently from normal packages: we put them all into the first stratum
            // regardless of their imports. This has interesting effects on the `remainingImports` and `backEdges`
            // vectors that we track in the builder:
            // 1. `remainingImports` must be set to `0` to indicate that all imports have been satisfied. This is true,
            //    as all prelude packages will be processed in the same stratum.
            // 2. `backEdges` must not be populated for the imports of prelude packages. As prelude packages may only
            //    import other prelude packages populating `backEdges` would cause prelude packages to be queued into
            //    the next stratum. Skipping the imports of prelude packages when we're populating `backEdges` ensures
            //    that we don't accidentally requeue a prelude package for a future stratum.
            if (node.isPrelude) {
                // We collect all prelude nodes here, as they will all show up in the first stratum if there are any.
                result.prelude.emplace_back(node.id);
                this->remainingImports[node.id] = 0;
            } else {
                auto numImports = node.imports.size();
                this->remainingImports[node.id] = numImports;
                if (numImports == 0) {
                    result.roots.emplace_back(node.id);
                }

                for (auto imp : node.imports) {
                    this->backEdges[imp].push_back(node.id);
                }
            }
        }

        return result;
    }

    // Record a single stratum by processing each SCC id, marking the imports of those ids as satisfied. If any
    // dependent package no longer has any outstanding imports, its id will be added to `next`.
    void recordStratum(absl::Span<const uint32_t> stratum, vector<uint32_t> &next) {
        stratumLengths.emplace_back(stratum.size());
        ENFORCE(!stratum.empty());

        for (auto sccId : stratum) {
            auto &node = this->nodes[sccId];
            ENFORCE(this->remainingImports[node.id] == 0);

            // Insert the members of the SCC into the packages vector.
            absl::c_copy(node.members, back_inserter(this->result.packages));

            auto &scc = this->result.sccs.emplace_back();
            scc.isTest = node.isTest;
            this->sccLengths.emplace_back(node.members.size());

            // Queue up the dependents in the next frontier, decrementing their imports by one
            for (auto dep : this->backEdges[sccId]) {
                auto &remaining = this->remainingImports[dep];

                // Having a `remaining` value of <= 0 at this point implies that the scc should have been in this or a
                // previous stratum already. This would only be possible if there were a cycle in the condensation
                // graph, and it should be a DAG by construction.
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

    // All prelude package SCCs, and the set of non-prelude package SCCs that have no imports.
    auto [prelude, roots] = builder.roots();

    if (!prelude.empty()) {
        // Record a single stratum for all prelude SCCs, emitting any packages whose imports were satisfied by the
        // prelude set to the roots.
        builder.recordStratum(prelude, roots);
    }

    // Insert the non-prelude SCCs that are ready but were skipped into the frontier, so that we process all of the
    // non-prelude nodes with no outstanding dependencies in one stratum.
    vector<uint32_t> next;
    while (!roots.empty()) {
        next.clear();

        builder.recordStratum(roots, next);

        // The content of `next` isn't important at this point, and it will be cleared on the next iteration of the
        // loop. Morally this is `roots = next`, but we swap and clear instead to avoid any ambiguity about
        // reallocations.
        swap(roots, next);
    }

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
