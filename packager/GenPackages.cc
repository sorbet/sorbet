#include "packager/GenPackages.h"
#include "core/errors/packager.h"
#include "packager/ComputePackageSCCs.h"

using namespace std;

namespace sorbet::packager {

namespace {
vector<core::packages::Import> computeNewImports(const core::GlobalState &gs,
                                                 const core::packages::PackageInfo &pkgInfo) {
    UnorderedSet<core::packages::MangledName> importMap;
    for (auto &import : pkgInfo.importedPackageNames) {
        auto &impPkgInfo = gs.packageDB().getPackageInfo(import.mangledName);
        if (impPkgInfo.exists() && impPkgInfo.isPreludePackage()) {
            // If the `__package.rb` already imports a prelude package, we should keep that import, even if it's not
            // referenced anywhere.
            importMap.emplace(import.mangledName);
        }
    }

    // TODO(neil): this loop is very similar to the loop in aggregateMissingImports, should find a way to deduplicate.
    // Can't deduplicate trivially because the loop in aggregateMissingImports skips entries where
    // !packageReferenceInfo.importNeeded or packageReferenceInfo.causesModularityError, as well if the import would
    // cause a visibility error. Maybe the common helper could take a function that filters?
    for (auto &[file, referencedPackages] : pkgInfo.packagesReferencedByFile) {
        for (auto &[packageName, packageReferenceInfo] : referencedPackages) {
            auto &pkgInfo = gs.packageDB().getPackageInfo(packageName);
            if (!pkgInfo.exists()) {
                continue;
            }
            // TODO(neil): this ignores strict dependencies/visibility violations and unconditionally adds an import.
            // Should we skip imports that would cause a strict dependencies/visibility error instead?
            importMap.emplace(packageName);
        }
    }

    vector<core::packages::Import> newImports;
    newImports.reserve(importMap.size());
    for (auto &mangledName : importMap) {
        newImports.emplace_back(mangledName, core::LocOffsets::none());
    }

    return newImports;
}

class ReferencesPackageGraph {
    core::packages::PackageDB &packageDB;
    const core::GlobalState &gs;

public:
    struct SCCInfo {
        int sccId;
        int testSccId;
    };
    UnorderedMap<core::packages::MangledName, SCCInfo> nodeMap;

    ReferencesPackageGraph(const core::GlobalState &gs, core::packages::PackageDB &packageDB)
        : packageDB(packageDB), gs(gs) {}

    vector<core::packages::Import> getImports(core::packages::MangledName packageName) {
        auto &pkgInfo = packageDB.getPackageInfo(packageName);
        ENFORCE(pkgInfo.exists());
        return computeNewImports(gs, pkgInfo);
    }

    void setSCCId(core::packages::MangledName packageName, int sccId) {
        nodeMap[packageName].sccId = sccId;
    }

    int getSCCId(core::packages::MangledName packageName) {
        return nodeMap[packageName].sccId;
    }

    void setTestSCCId(core::packages::MangledName packageName, int sccId) {
        nodeMap[packageName].testSccId = sccId;
    }

    int getTestSCCId(core::packages::MangledName packageName) {
        return nodeMap[packageName].testSccId;
    }
};

vector<core::packages::StrictDependenciesLevel>
computeBestStrictness(const core::GlobalState &gs, const core::packages::Condensation &condensation) {
    auto bestStrictness = vector<core::packages::StrictDependenciesLevel>(
        condensation.nodes().size(), core::packages::StrictDependenciesLevel::None);
    auto layerMap = vector<core::NameRef>(condensation.nodes().size(), core::NameRef::noName());
    for (auto &scc : condensation.nodes()) {
        ENFORCE(!scc.members.empty());
        auto &firstPkg = gs.packageDB().getPackageInfo(scc.members[0]);
        ENFORCE(firstPkg.exists());

        if (!firstPkg.layer.exists()) {
            bestStrictness[scc.id] = core::packages::StrictDependenciesLevel::False;
            continue;
        }

        core::NameRef sccLayer = firstPkg.layer;
        for (auto &member : scc.members) {
            auto &pkgInfo = gs.packageDB().getPackageInfo(member);
            ENFORCE(pkgInfo.exists());
            if (pkgInfo.layer != sccLayer) {
                // SCC has multiple layers, which implies layering violation
                bestStrictness[scc.id] = core::packages::StrictDependenciesLevel::False;
                break;
            }
        }
        if (bestStrictness[scc.id] == core::packages::StrictDependenciesLevel::False) {
            continue;
        }

        layerMap[scc.id] = sccLayer;

        for (auto &impSccId : scc.imports) {
            ENFORCE(bestStrictness[impSccId] != core::packages::StrictDependenciesLevel::None);
            if (bestStrictness[impSccId] == core::packages::StrictDependenciesLevel::False) {
                // imports a strict_dependencies 'false' package
                bestStrictness[scc.id] = core::packages::StrictDependenciesLevel::False;
                break;
            }

            ENFORCE(layerMap[impSccId].exists());
            if (gs.packageDB().layerIndex(layerMap[scc.id]) < gs.packageDB().layerIndex(layerMap[impSccId])) {
                // layering violation
                bestStrictness[scc.id] = core::packages::StrictDependenciesLevel::False;
                break;
            }

            if (bestStrictness[impSccId] != core::packages::StrictDependenciesLevel::Dag) {
                // imports a package that is strict_dependencies 'layered' or 'layered_dag'
                bestStrictness[scc.id] = core::packages::StrictDependenciesLevel::LayeredDag;
                // Can't break here, since a later import could still bring down the best strictness
            }
        }
        if (bestStrictness[scc.id] == core::packages::StrictDependenciesLevel::False) {
            continue;
        }

        if (scc.members.size() > 1) {
            // More than one package in SCC, which implies cycle
            bestStrictness[scc.id] = core::packages::StrictDependenciesLevel::Layered;
            continue;
        }

        if (bestStrictness[scc.id] == core::packages::StrictDependenciesLevel::None) {
            // We didn't encounter any import that brought down this SCC's strictness
            ENFORCE(scc.members.size() == 1);
            bestStrictness[scc.id] = core::packages::StrictDependenciesLevel::Dag;
        }
    }
    return bestStrictness;
}
}; // namespace

void GenPackages::run(core::GlobalState &gs) {
    Timer timeit(gs.tracer(), "gen_packages.run");

    auto toExport = gs.packageDB().exportsByPackage(gs);

    auto neededVisibleTo = UnorderedMap<core::packages::MangledName, UnorderedSet<core::packages::MangledName>>{};
    auto neededVisibleToTests = UnorderedMap<core::packages::MangledName, bool>{};
    if (gs.packageDB().anyUpdateVisibilityFor()) {
        Timer timeit(gs.tracer(), "gen_packages.run.build_needed_visible_to");
        for (auto pkgName : gs.packageDB().packages()) {
            auto &pkgInfo = gs.packageDB().getPackageInfo(pkgName);
            ENFORCE(pkgInfo.exists());
            // TODO(neil): this loop is similar to what we have in aggregateMissingImports, is there a way to dedup?
            for (auto &[file, referencedPackages] : pkgInfo.packagesReferencedByFile) {
                for (auto &[referencedPackageName, packageReferenceInfo] : referencedPackages) {
                    auto &referencedPackageInfo = gs.packageDB().getPackageInfo(referencedPackageName);
                    ENFORCE(referencedPackageInfo.exists());
                    if (gs.packageDB().updateVisibilityFor(referencedPackageName)) {
                        if (!referencedPackageInfo.isVisibleTo(gs, pkgInfo) ||
                            referencedPackageInfo.visibleToEverything()) {
                            // either:
                            // - it is a visibility error for pkgName to reference referencedPackageName, and we want to
                            // silence the error
                            // - referencedPackage has no visible_to restrictions, and we want to ratchet
                            // referencedPackage's `visible_to`s
                            //
                            // In either case, we'll add a new `visible_to` to referencedPackage's __package.db
                            auto pkg = gs.packageDB().findPackageByPath(gs, file);
                            auto &pkgInfo = gs.packageDB().getPackageInfo(pkg);
                            if (gs.packageDB().allowRelaxingTestVisibility() && pkgInfo.exists() &&
                                pkgInfo.testPackage()) {
                                // If --allow-relaxing-test-visibility, and this reference is in a test file, add
                                // `visible_to 'tests'`
                                neededVisibleToTests[referencedPackageName] = true;
                            } else {
                                // Otherwise, add `visible_to pkgName`
                                neededVisibleTo[referencedPackageName].insert(pkgName);
                            }
                        }
                    }
                }
            }
        }
    }

    for (auto pkgName : gs.packageDB().packages()) {
        auto &pkgInfo = gs.packageDB().getPackageInfo(pkgName);
        ENFORCE(pkgInfo.exists());
        auto importsAutocorrect = pkgInfo.aggregateMissingImports(gs);
        // It's a bit odd here that for imports, all the logic is in aggregateMissingImports, while for
        // aggregateMissingExports, we need to do some extra computations outside the method and pass that in.
        // The difference is that computing what imports are needed is a function only of the files in that package.
        // However, for exports, we need to look the symbols referenced by other packages, which necessitates a loop
        // over all files, and it doesn't make sense to rerun the loop for each package, so we do this work upfront
        auto exportsAutocorrect = pkgInfo.aggregateMissingExports(gs, toExport[pkgName]);
        // Similar idea for visible_to.
        // We always call aggregateMissingVisibleTo so that visible_to entries referencing non-existent packages
        // get deleted, even if we're not adding new visible_to entries for this package.
        auto emptyVisibleTos = UnorderedSet<core::packages::MangledName>{};
        auto visibleToAutocorrect = pkgInfo.aggregateMissingVisibleTo(
            gs, gs.packageDB().updateVisibilityFor(pkgName) ? neededVisibleTo[pkgName] : emptyVisibleTos,
            gs.packageDB().updateVisibilityFor(pkgName) && neededVisibleToTests[pkgName]);

        auto autocorrects = vector<core::AutocorrectSuggestion>{};
        auto missingTypes = vector<string>{};

        if (importsAutocorrect) {
            autocorrects.push_back(move(importsAutocorrect.value()));
            missingTypes.push_back("imports");
        }

        if (exportsAutocorrect) {
            autocorrects.push_back(move(exportsAutocorrect.value()));
            missingTypes.push_back("exports");
        }

        if (visibleToAutocorrect) {
            autocorrects.push_back(move(visibleToAutocorrect.value()));
            missingTypes.push_back(fmt::format("`{}`s", "visible_to"));
        }

        if (autocorrects.size() == 1) {
            if (auto e = gs.beginError(pkgInfo.declLoc(), core::errors::Packager::IncorrectPackageRB)) {
                e.setHeader("`{}` is missing {}", pkgInfo.show(gs), missingTypes[0]);
                e.addAutocorrect(move(autocorrects[0]));
            }
        } else if (autocorrects.size() == 2) {
            if (auto e = gs.beginError(pkgInfo.declLoc(), core::errors::Packager::IncorrectPackageRB)) {
                e.setHeader("`{}` is missing {} and {}", pkgInfo.show(gs), missingTypes[0], missingTypes[1]);
                auto combinedEdits = vector<core::AutocorrectSuggestion::Edit>{};
                combinedEdits.insert(combinedEdits.end(), make_move_iterator(autocorrects[0].edits.begin()),
                                     make_move_iterator(autocorrects[0].edits.end()));
                combinedEdits.insert(combinedEdits.end(), make_move_iterator(autocorrects[1].edits.begin()),
                                     make_move_iterator(autocorrects[1].edits.end()));
                core::AutocorrectSuggestion::mergeAdjacentEdits(combinedEdits);
                auto autocorrectTitle = fmt::format("Add missing {} and {}", missingTypes[0], missingTypes[1]);
                e.addAutocorrect(core::AutocorrectSuggestion{autocorrectTitle, move(combinedEdits)});
            }
        } else if (autocorrects.size() == 3) {
            if (auto e = gs.beginError(pkgInfo.declLoc(), core::errors::Packager::IncorrectPackageRB)) {
                e.setHeader("`{}` is missing {}, {} and {}", pkgInfo.show(gs), missingTypes[0], missingTypes[1],
                            missingTypes[2]);
                auto combinedEdits = vector<core::AutocorrectSuggestion::Edit>{};
                combinedEdits.insert(combinedEdits.end(), make_move_iterator(autocorrects[0].edits.begin()),
                                     make_move_iterator(autocorrects[0].edits.end()));
                combinedEdits.insert(combinedEdits.end(), make_move_iterator(autocorrects[1].edits.begin()),
                                     make_move_iterator(autocorrects[1].edits.end()));
                combinedEdits.insert(combinedEdits.end(), make_move_iterator(autocorrects[2].edits.begin()),
                                     make_move_iterator(autocorrects[2].edits.end()));
                core::AutocorrectSuggestion::mergeAdjacentEdits(combinedEdits);
                auto autocorrectTitle =
                    fmt::format("Add missing {}, {} and {}", missingTypes[0], missingTypes[1], missingTypes[2]);
                e.addAutocorrect(core::AutocorrectSuggestion{autocorrectTitle, move(combinedEdits)});
            }
        } else if (autocorrects.size() > 3) {
            ENFORCE(false);
        }
        // TODO(neil): for a file with no imports or exports, if both imports and exports are missing, we'll produce
        // something like:
        //   export MyPkg::Foo
        //   import OtherPkg
        // because e < i
        // TODO(neil): we should also delete imports that are unused but have a modularity error here
    }
}

void GenPackages::runStrict(core::GlobalState &gs) {
    Timer timeit(gs.tracer(), "gen_packages.run_strict");

    ReferencesPackageGraph referencesPackageGraph{gs, gs.packageDB()};
    auto condensation = ComputePackageSCCs::run(gs, referencesPackageGraph);
    auto bestStrictness = computeBestStrictness(gs, condensation);
    auto skipRaisingStrictness = vector<bool>(condensation.nodes().size(), false);
    for (auto &scc : condensation.nodes()) {
        for (auto &pkgName : scc.members) {
            auto &pkgInfo = gs.packageDB().getPackageInfo(pkgName);
            // TODO(neil): also check if this package is a build package
            if (pkgInfo.numFilesInPackage() == 0) {
                skipRaisingStrictness[scc.id] = true;
                break;
            }
        }
    }
    auto newStrictnessByPkg = UnorderedMap<core::packages::MangledName, core::packages::StrictDependenciesLevel>(
        gs.packageDB().packages().size());
    for (auto pkgName : gs.packageDB().packages()) {
        auto &pkgInfo = gs.packageDB().getPackageInfo(pkgName);
        ENFORCE(pkgInfo.exists());
        auto sccID = referencesPackageGraph.getSCCId(pkgName);
        auto newStrictDependenciesLevel = bestStrictness[sccID];
        if (newStrictDependenciesLevel < pkgInfo.strictDependenciesLevel || skipRaisingStrictness[sccID]) {
            newStrictDependenciesLevel = pkgInfo.strictDependenciesLevel;
        }
        newStrictnessByPkg[pkgName] = newStrictDependenciesLevel;
    }

    auto toExport = gs.packageDB().exportsByPackage(gs);

    for (auto pkgName : gs.packageDB().packages()) {
        auto &pkgInfo = gs.packageDB().getPackageInfo(pkgName);
        ENFORCE(pkgInfo.exists());

        auto existingContentsLoc = core::Loc(pkgInfo.file, pkgInfo.locs.loc)
                                       .adjust(gs, pkgInfo.locs.declLoc.length() + 1, -1 * (int32_t) "end"sv.size());
        auto existingContents = existingContentsLoc.source(gs);
        ENFORCE(existingContents.has_value());

        // TODO(neil): If the __package.rb has an unused import that is a modularity error to import, we'll report an
        // error on the import in packager.cc, even though we're deleting it here
        auto newContents = pkgInfo.renderPackageRbContents(gs, computeNewImports(gs, pkgInfo), move(toExport[pkgName]),
                                                           newStrictnessByPkg);

        if (existingContents.value() != newContents) {
            if (auto e = gs.beginError(pkgInfo.declLoc(), core::errors::Packager::IncorrectPackageRB)) {
                e.setHeader("`{}` has changes", pkgInfo.show(gs));
                bool isDidYouMean = false;
                bool hideEdit = true;
                e.addAutocorrect({"Update __package.rb", {{existingContentsLoc, newContents}}, isDidYouMean, hideEdit});
            }
        }
    }
}

} // namespace sorbet::packager
