#include "core/packages/Condensation.h"
#include "PackageDB.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "core/GlobalState.h"
#include <algorithm>

using namespace std;

namespace sorbet::core::packages {

namespace {

struct TraversalBuilder {
    absl::Span<const Condensation::Node> nodes;
    vector<uint32_t> remainingImports;

    vector<uint32_t> sccLengths;
    vector<uint32_t> stratumLengths;

    Condensation::Traversal result;

    TraversalBuilder(const core::GlobalState &gs, absl::Span<const Condensation::Node> nodes)
        : nodes{nodes}, remainingImports(this->nodes.size(), 0) {
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

    // Sort the given stratum by smallest member id. As members are sorted for a finalized condensation graph, the
    // first id in the members list is always the smallest.
    void sortStratum(absl::Span<uint32_t> stratum) const {
        fast_sort(stratum, [this](uint32_t leftId, uint32_t rightId) {
            // Members are sorted, and SCCs are disjoint, so their first members determine the order.
            return this->nodes[leftId].members.front().owner.id() < this->nodes[rightId].members.front().owner.id();
        });
    }

    // Record a single stratum, staging it into the resulting traversal.
    void recordStratum(absl::Span<const uint32_t> stratum) {
        stratumLengths.emplace_back(stratum.size());
        ENFORCE(!stratum.empty());

        for (auto sccId : stratum) {
            auto &node = this->nodes[sccId];
            ENFORCE(this->remainingImports[node.id] == 0);

            // Insert the members of the SCC into the packages vector.
            absl::c_copy(node.members, back_inserter(this->result.packages));

            this->result.sccs.emplace_back();
            this->sccLengths.emplace_back(node.members.size());
        }
    }

    // Traverse the nodes that depend on `current`, decrementing their remaining import counts. If the count goes to
    // zero, add that node to the next stratum.
    template <IsPrelude PreludeStratum>
    void computeNextStratum(absl::Span<const uint32_t> current, vector<uint32_t> &next) {
        for (auto sccId : current) {
            auto &node = this->nodes[sccId];
            for (auto dep : node.backEdges) {
                auto &remaining = this->remainingImports[dep];

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
    }

    Condensation::Traversal build() && {
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

bool cachedTraversalIsUsable(const PackageDB &db) {
    auto packages = db.packages();
    if (packages.empty()) {
        return true;
    }

    // NOTE: We treat a single package with a populated stratum as implying that all packages have their stratum set.
    // This relies heavily on us not copying the package DB if the package graph changes on the slow path.
    auto cacheValid = db.getPackageInfo(packages.front()).stratum != Stratum::NONE;

    ENFORCE(cacheValid == absl::c_all_of(packages,
                                         [&db](auto pkgName) {
                                             auto &info = db.getPackageInfo(pkgName);
                                             return info.exists() && info.stratum != Stratum::NONE;
                                         }),
            "PackageInfo::stratum must be set or unset consistently for all packages");

    return cacheValid;
}

// Used when rebuilding the cached traversal.
struct NodeStratum {
    Stratum stratum;
    uint32_t nodeId = 0;
};

} // namespace

const Condensation::Traversal Condensation::computeTraversal(const core::GlobalState &gs) const {
    TraversalBuilder builder(gs, this->nodes_);

    if (cachedTraversalIsUsable(gs.packageDB())) {
        builder.result.cached = true;

        auto &db = gs.packageDB();

        vector<NodeStratum> stratumMapping;
        stratumMapping.reserve(this->nodes().size());
        for (auto &node : this->nodes()) {
            ENFORCE(!node.members.empty());
            auto stratum = db.getPackageInfo(node.members.front()).stratum;
            stratumMapping.emplace_back(stratum, node.id);
        }

        fast_sort(stratumMapping, [](auto left, auto right) { return left.stratum < right.stratum; });

        vector<uint32_t> sccs;
        auto remaining = absl::MakeSpan(stratumMapping);
        while (!remaining.empty()) {
            auto stratum = remaining.front().stratum;
            auto end = absl::c_find_if(remaining, [stratum](auto &p) { return p.stratum != stratum; });
            auto stratumSize = std::distance(remaining.begin(), end);

            auto members = remaining.subspan(0, stratumSize);
            remaining = remaining.subspan(stratumSize);

            sccs.clear();
            absl::c_transform(members, back_inserter(sccs), [](auto &p) { return p.nodeId; });
            builder.sortStratum(absl::MakeSpan(sccs));
            builder.recordStratum(sccs);
        }
    } else {
        // All prelude package SCCs, and the set of non-prelude package SCCs that have no imports.
        auto [prelude, roots] = builder.roots();

        if (!prelude.empty()) {
            builder.sortStratum(absl::MakeSpan(prelude));
            // Record a single stratum for all prelude SCCs, emitting any packages whose imports were satisfied by the
            // prelude set to the roots.
            builder.recordStratum(prelude);
            builder.computeNextStratum<TraversalBuilder::IsPrelude::True>(prelude, roots);
        }

        // Insert the non-prelude SCCs that are ready but were skipped into the frontier, so that we process all of the
        // non-prelude nodes with no outstanding dependencies in one stratum.
        vector<uint32_t> next;
        while (!roots.empty()) {
            next.clear();

            builder.sortStratum(absl::MakeSpan(roots));
            builder.recordStratum(roots);
            builder.computeNextStratum<TraversalBuilder::IsPrelude::False>(roots, next);

            // The content of `next` isn't important at this point, and it will be cleared on the next iteration of the
            // loop. Morally this is `roots = next`, but we swap and clear instead to avoid any ambiguity about
            // reallocations.
            swap(roots, next);
        }
    }

    return move(builder).build();
}

void Condensation::Traversal::recordStrata(core::GlobalState &gs) const {
    auto &db = gs.packageDB();
    for (auto name : db.packages()) {
        db.getPackageInfoNonConst(name)->stratum = Stratum::NONE;
    }

    int ix = -1;
    for (auto stratum : this->strata) {
        ++ix;
        for (auto &scc : stratum) {
            for (auto name : scc.members) {
                db.getPackageInfoNonConst(name)->stratum = Stratum(ix);
            }
        }
    }
}

namespace {

uint32_t stratumSize(const PackageDB &db, absl::Span<const uint32_t> sccSizes,
                     absl::Span<const Condensation::Traversal::SCCInfo> stratum) {
    return absl::c_accumulate(stratum, 0L, [&db, sccSizes](auto acc, auto &sccInfo) {
        auto sccId = db.getPackageInfo(sccInfo.members.front()).sccID();
        ENFORCE(sccId.has_value());
        return acc + sccSizes[*sccId];
    });
}

struct StratumSplittingDecision {
    bool preludePresent = false;
    bool couldSplit = false;
};

StratumSplittingDecision couldSplitStratum(const PackageDB &db, absl::Span<const uint32_t> sccSizes,
                                           const Condensation::Traversal &traversal,
                                           optional<uint32_t> stratumSplitThreshold) {
    StratumSplittingDecision result;
    auto strata = absl::MakeSpan(traversal.strata);

    result.preludePresent =
        absl::c_any_of(db.packages(), [&db](auto pkgName) { return db.getPackageInfo(pkgName).isPreludePackage(); });

    if (!stratumSplitThreshold.has_value()) {
        return result;
    }

    // We explicitly skip the first stratum when prelude packages are present, as we require that all prelude
    // packages go in that stratum and all prelude packages must remain in the same stratum.
    if (result.preludePresent) {
        strata = strata.subspan(1);
    }

    result.couldSplit = absl::c_any_of(strata, [&db, sccSizes, threshold = *stratumSplitThreshold](auto stratum) {
        return stratumSize(db, sccSizes, stratum) > threshold;
    });

    return result;
}

bool couldCombineStrata(const PackageDB &db, absl::Span<const uint32_t> sccSizes,
                        const Condensation::Traversal &traversal, uint32_t minStratumSize) {
    return absl::c_any_of(traversal.strata, [&db, sccSizes, minStratumSize](auto stratum) {
        return stratumSize(db, sccSizes, stratum) < minStratumSize;
    });
}
} // namespace

void Condensation::Traversal::optimize(const PackageDB &db, optional<uint32_t> stratumSplitThreshold,
                                       optional<uint32_t> minStratumSize) {
    if (!stratumSplitThreshold.has_value() && !minStratumSize.has_value()) {
        return;
    }

    // Precompute SCC file sizes to help when determining if either optimization applies.
    vector<uint32_t> sccSizes;
    sccSizes.reserve(db.condensation().nodes().size());
    for (auto &node : db.condensation().nodes()) {
        sccSizes.emplace_back(absl::c_accumulate(node.members, 0L, [&db](auto acc, auto pkgName) {
            return acc + db.getPackageInfo(pkgName).numFilesInPackage;
        }));
    }

    // We split before considering combining, as it's not safe to split a combined stratum: splitting takes advantage
    // of the fact that no scc in a stratum has a dependency on any other scc, but combining adjacent strata invalidates
    // that assumption.
    auto [preludePresent, splittingPossible] = couldSplitStratum(db, sccSizes, *this, stratumSplitThreshold);
    if (splittingPossible) {
        ENFORCE(stratumSplitThreshold.has_value());
        auto originalStrata = std::exchange(this->strata, {});
        auto strata = absl::MakeSpan(originalStrata);

        auto start = 0;
        if (preludePresent) {
            this->strata.emplace_back(strata.front());
            start = this->strata.back().size();
            strata = strata.subspan(1);
        }

        for (auto stratum : strata) {
            if (stratumSize(db, sccSizes, stratum) > *stratumSplitThreshold) {
                // Disassemble the large stratum into one stratum for each SCC, relying on the next optimization to
                // recombine them.
                for (int i = 0; i < stratum.size(); ++i) {
                    this->strata.emplace_back(absl::MakeSpan(this->sccs).subspan(start, 1));
                    start += 1;
                }
            } else {
                this->strata.emplace_back(stratum);
                start += stratum.size();
            }
        }
    }

    if (minStratumSize.has_value() && couldCombineStrata(db, sccSizes, *this, *minStratumSize)) {
        auto strata = std::exchange(this->strata, {});

        auto start = 0;
        this->strata.emplace_back(strata.front());
        auto size = stratumSize(db, sccSizes, this->strata.back());

        for (auto stratum : absl::MakeSpan(strata).subspan(1)) {
            auto candidateSize = stratumSize(db, sccSizes, stratum);
            if (size < *minStratumSize) {
                auto length = this->strata.back().size() + stratum.size();
                this->strata.back() = absl::MakeSpan(this->sccs).subspan(start, length);
                size += candidateSize;
            } else {
                start += this->strata.back().size();
                this->strata.emplace_back(stratum);
                size = candidateSize;
            }
        }
    }
}

Condensation::Node &CondensationBuilder::pushNode(bool isPrelude) {
    auto id = this->condensation.nodes_.size();
    auto &node = this->condensation.nodes_.emplace_back(id, isPrelude);
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
