#include "core/packages/Condensation.h"
#include "MangledName.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "core/GlobalState.h"
#include <algorithm>
#include <tuple>

using namespace std;

namespace sorbet::core::packages {

namespace {

struct TraversalBuilder {
    absl::Span<const Condensation::Node> nodes;
    vector<uint32_t> remainingImports;

    vector<uint32_t> sccLengths;
    vector<uint32_t> stratumLengths;

    // Indicates if this node will be part of the traversal at all
    vector<bool> skipNode;

    Condensation::Traversal result;

    TraversalBuilder(const core::GlobalState &gs, absl::Span<const Condensation::Node> nodes,
                     UnorderedSet<MangledName> *focusedPackages, bool skipDownstream)
        : nodes{nodes}, remainingImports(this->nodes.size(), 0) {
        result.packages.reserve(gs.packageDB().packages().size());

        // Nothing is skipped to start with.
        this->skipNode = vector(this->nodes.size(), false);

        // The high-level summary of the focused algorithm is:
        // 1. we mark all prelude packages as required, and part of the upstream set.
        // 2. we traverse down to the leaves of the condensation graph from the focused set to mark the downstream set
        // 3. we traverse from the union of the focused and downstream sets towards the roots, to mark the upstream set.
        //
        // These sets make up the focused edit.
        if (focusedPackages) {
            this->skipNode.assign(this->nodes.size(), true);

            // Mark all `prelude!` packages as not skippable
            for (auto node : this->nodes) {
                if (!node.isPrelude) {
                    continue;
                }

                this->skipNode[node.id] = false;

                for (auto pkg : node.members) {
                    if (focusedPackages->contains(pkg)) {
                        continue;
                    }

                    this->result.upstream.insert(pkg);
                }
            }

            auto &db = gs.packageDB();
            vector<int> work;

            // Mark the focused set as not skippable, and augment it with all the members of the involved sccs
            {
                UnorderedSet<MangledName> augmented;
                for (auto pkgName : *focusedPackages) {
                    if (!pkgName.exists()) {
                        continue;
                    }

                    auto &info = db.getPackageInfo(pkgName);
                    ENFORCE(info.exists());

                    this->skipNode[info.sccID().value()] = false;

                    for (auto pkgName : this->nodes[info.sccID().value()].members) {
                        augmented.insert(pkgName);
                    }

                    if (!info.usesTestPackages) {
                        this->skipNode[info.testSccID().value()] = false;

                        for (auto pkgName : this->nodes[info.testSccID().value()].members) {
                            augmented.insert(pkgName);
                        }
                    }
                }

                // `augmented` represents the original set augmented with scc members
                std::swap(*focusedPackages, augmented);
            }

            if (!skipDownstream) {
                // Seed the traversal with the dependants of the focused set, also marking the focused set as not
                // skippable.
                for (auto pkgName : *focusedPackages) {
                    if (!pkgName.exists()) {
                        continue;
                    }

                    auto &info = db.getPackageInfo(pkgName);
                    ENFORCE(info.exists());

                    absl::c_copy(this->nodes[info.sccID().value()].backEdges, back_inserter(work));

                    if (!info.usesTestPackages) {
                        absl::c_copy(this->nodes[info.testSccID().value()].backEdges, back_inserter(work));
                    }
                }

                // Traverse towards the leaves, building the downstream set.
                while (!work.empty()) {
                    auto id = work.back();
                    work.pop_back();
                    if (!this->skipNode[id]) {
                        continue;
                    }

                    this->skipNode[id] = false;

                    auto &node = this->nodes[id];
                    this->result.downstream.insert(node.members.begin(), node.members.end());
                    absl::c_copy(node.backEdges, back_inserter(work));
                }
            }

            // Seed the traversal with the dependencies of everything marked as not skippable.
            for (auto &node : this->nodes) {
                if (this->skipNode[node.id]) {
                    continue;
                }

                absl::c_copy(node.imports, back_inserter(work));
            }

            fast_sort(work);
            work.erase(std::unique(work.begin(), work.end()), work.end());

            // Traverse towards the roots from the focused set, building the upstreams set at the same time.
            while (!work.empty()) {
                auto id = work.back();
                work.pop_back();
                if (!this->skipNode[id]) {
                    continue;
                }

                this->skipNode[id] = false;

                auto &node = this->nodes[id];
                this->result.upstream.insert(node.members.begin(), node.members.end());
                absl::c_copy(node.imports, back_inserter(work));
            }

            work.clear();
        }
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
            // regardless of their imports. This has interesting effects on the `remainingImports` we track in the
            // builder, and how we use the `backEdges` of the condensation graph:
            // 1. `remainingImports` must be set to `0` to indicate that all imports have been satisfied. This is true,
            //    as all prelude packages will be processed in the same stratum.
            // 2. `backEdges` must be filtered for prelude packages to ignore any other packages present in the prelude
            //    subgraph. We do this during `recordStratum` for the prelude layer only, skipping any back edges to
            //    other prelude packages.
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
            }
        }

        return result;
    }

    enum class IsPrelude {
        True,
        False,
    };

    // Sort the given stratum by test/application status, and then by smallest member id. As members are sorted for a
    // finalized condensation graph, the first id in the members list is always the smallest.
    void sortStratum(absl::Span<uint32_t> stratum) const {
        fast_sort(stratum, [this](uint32_t leftId, uint32_t rightId) {
            const auto &left = this->nodes[leftId];
            const auto &right = this->nodes[rightId];
            // Prelude strata can contain both application and test SCCs. Keep application code first.
            // Members are sorted, and SCCs of the same kind are disjoint, so their first members determine the order.
            auto leftFirstMemberId = left.members.front().owner.id();
            auto rightFirstMemberId = right.members.front().owner.id();
            return std::tie(left.isTest, leftFirstMemberId) < std::tie(right.isTest, rightFirstMemberId);
        });
    }

    // Record a single stratum by processing each SCC id, marking the imports of those ids as satisfied. If any
    // dependent package no longer has any outstanding imports, its id will be added to `next`.
    template <IsPrelude PreludeStratum> void recordStratum(absl::Span<const uint32_t> stratum, vector<uint32_t> &next) {
        ENFORCE(!stratum.empty());

        auto numSccs = 0;

        for (auto sccId : stratum) {
            if (this->skipNode[sccId]) {
                ENFORCE(!this->nodes[sccId].isPrelude);
                continue;
            }

            numSccs++;

            auto &node = this->nodes[sccId];
            ENFORCE(this->remainingImports[node.id] == 0);

            // Insert the members of the SCC into the packages vector.
            absl::c_copy(node.members, back_inserter(this->result.packages));

            auto &scc = this->result.sccs.emplace_back();
            scc.isTest = node.isTest;
            this->sccLengths.emplace_back(node.members.size());

            // Queue up the dependents in the next frontier, decrementing their imports by one
            for (auto dep : this->nodes[sccId].backEdges) {
                auto &remaining = this->remainingImports[dep];

                // Prelude packages are processed all in one go, so we ignore any dependencies within that subgraph.
                if constexpr (PreludeStratum == IsPrelude::True) {
                    if (this->nodes[dep].isPrelude) {
                        continue;
                    }
                }

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

        if (numSccs > 0) {
            this->stratumLengths.emplace_back(numSccs);
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

const Condensation::Traversal Condensation::computeTraversal(const core::GlobalState &gs,
                                                             UnorderedSet<MangledName> *focusedPackages,
                                                             bool skipDownstream) const {
    TraversalBuilder builder(gs, this->nodes_, focusedPackages, skipDownstream);

    // All prelude package SCCs, and the set of non-prelude package SCCs that have no imports.
    auto [prelude, roots] = builder.roots();

    if (!prelude.empty()) {
        builder.sortStratum(absl::MakeSpan(prelude));
        // Record a single stratum for all prelude SCCs, emitting any packages whose imports were satisfied by the
        // prelude set to the roots.
        builder.recordStratum<TraversalBuilder::IsPrelude::True>(prelude, roots);
    }

    // Insert the non-prelude SCCs that are ready but were skipped into the frontier, so that we process all of the
    // non-prelude nodes with no outstanding dependencies in one stratum.
    vector<uint32_t> next;
    while (!roots.empty()) {
        next.clear();

        builder.sortStratum(absl::MakeSpan(roots));
        builder.recordStratum<TraversalBuilder::IsPrelude::False>(roots, next);

        // The content of `next` isn't important at this point, and it will be cleared on the next iteration of the
        // loop. Morally this is `roots = next`, but we swap and clear instead to avoid any ambiguity about
        // reallocations.
        swap(roots, next);
    }

    return builder.build();
}

void Condensation::Traversal::recordStrata(core::GlobalState &gs) const {
    auto &db = gs.packageDB();
    for (auto name : db.packages()) {
        db.getPackageInfoNonConst(name)->stratum = {};
    }

    int ix = -1;
    for (auto stratum : this->strata) {
        ++ix;
        for (auto &scc : stratum) {
            for (auto name : scc.members) {
                // TODO(trevor): we can switch to maintaining a single stratum once we've fully migrated to
                // test-packages.
                auto &info = db.getPackageInfoNonConst(name)->stratum;
                if (db.getPackageInfo(name).usesTestPackages) {
                    ENFORCE(info.applicationStratum == Stratum::NONE);
                    ENFORCE(info.testStratum == Stratum::NONE);
                    info.applicationStratum = Stratum(ix);
                    info.testStratum = Stratum(ix);
                } else if (scc.isTest) {
                    ENFORCE(info.testStratum == Stratum::NONE);
                    info.testStratum = Stratum(ix);
                } else {
                    ENFORCE(info.applicationStratum == Stratum::NONE);
                    info.applicationStratum = Stratum(ix);
                }
            }
        }
    }
}

Condensation::Node &CondensationBuilder::pushNode(ImportType type, bool isPrelude) {
    auto id = this->condensation.nodes_.size();
    auto isTest = type != ImportType::Normal;
    auto &node = this->condensation.nodes_.emplace_back(id, isTest, isPrelude);
    return node;
}

Condensation CondensationBuilder::build() && {
    // Now that we're finalizing the condensation graph, sort each node's members and build its back edges.
    for (auto &node : this->condensation.nodes_) {
        ENFORCE(!node.members.empty());
        // With stable package identities, member ordering should not depend on SCC discovery order.
        fast_sort(node.members, [](MangledName left, MangledName right) { return left.owner.id() < right.owner.id(); });
        for (auto imp : node.imports) {
            this->condensation.nodes_[imp].backEdges.push_back(node.id);
        }
    }

    return move(this->condensation);
}

UnorderedSet<MangledName> Condensation::transitiveDependentsOf(const PackageDB &db,
                                                               const UnorderedSet<MangledName> &packages) const {
    vector<int> frontier, next;
    absl::c_transform(packages, back_inserter(frontier), [&db](auto pkg) {
        auto &info = db.getPackageInfo(pkg);
        ENFORCE(info.exists());

        // TODO(trevor) remove this comment once we've fully migrated to test-packages
        // Right now this is an over approximation, as we can't tell if the package specified in `packages` is
        // referring to the tests, or the application code.
        auto scc = info.sccID();
        ENFORCE(scc.has_value());
        return scc.value();
    });

    vector<bool> visited(this->nodes_.size(), false);
    while (!frontier.empty()) {
        next.clear();

        for (auto scc : frontier) {
            if (visited[scc]) {
                continue;
            }

            visited[scc] = true;

            auto &node = this->nodes_[scc];

            next.insert(next.end(), node.backEdges.begin(), node.backEdges.end());
        }

        std::swap(frontier, next);
    }

    UnorderedSet<MangledName> downstream;

    for (auto &node : this->nodes_) {
        if (!visited[node.id]) {
            continue;
        }

        for (auto pkg : node.members) {
            downstream.insert(pkg);
        }
    }

    return downstream;
}

} // namespace sorbet::core::packages
