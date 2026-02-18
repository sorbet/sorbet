#include "packager/GenPackages.h"
#include "core/errors/packager.h"

using namespace std;

namespace sorbet::packager {

namespace {
// TODO(neil): deduplicate, this helper is also declared in VisibilityChecker.cc
core::SymbolRef getEnumClassForEnumValue(const core::GlobalState &gs, core::SymbolRef sym) {
    if (sym.isStaticField(gs) && sym.owner(gs).isClassOrModule()) {
        auto owner = sym.owner(gs);
        // There's a hidden class like `MyEnum::X$1` between `MyEnum::X` and `T::Enum` in the ancestor chain.
        if (owner.asClassOrModuleRef().data(gs)->superClass() == core::Symbols::T_Enum()) {
            return owner;
        }
    }

    return core::Symbols::noSymbol();
}

bool ownerWillBeExported(const core::GlobalState &gs, const vector<core::SymbolRef> &alreadyExported,
                         core::ClassOrModuleRef owner) {
    while (owner != core::Symbols::root()) {
        if (absl::c_find(alreadyExported, owner) != alreadyExported.end()) {
            return true;
        }
        owner = owner.data(gs)->owner;
    }
    return false;
}

void exportClassOrModule(const core::GlobalState &gs,
                         UnorderedMap<core::packages::MangledName, vector<core::SymbolRef>> &toExport,
                         core::ClassOrModuleRef symbol, vector<core::FileRef> &referencingFiles) {
    auto data = symbol.data(gs);
    auto owningPackage = data->package;
    if (!owningPackage.exists() || gs.packageDB().getPackageInfo(owningPackage).locs.exportAll.exists() ||
        data->flags.isExported) {
        return;
    }

    for (auto &f : referencingFiles) {
        auto packageForF = gs.packageDB().getPackageNameForFile(f);
        if (packageForF == owningPackage || gs.packageDB().allowRelaxedPackagerChecksFor(packageForF)) {
            continue;
        }

        if (ownerWillBeExported(gs, toExport[owningPackage], data->owner)) {
            // No need to check the rest of referencingFiles, we're already going to export the owner
            break;
        }

        toExport[owningPackage].push_back(symbol);
        break;
    }
}

void exportField(const core::GlobalState &gs,
                 UnorderedMap<core::packages::MangledName, vector<core::SymbolRef>> &toExport, core::FieldRef symbol,
                 vector<core::FileRef> &referencingFiles) {
    auto data = symbol.data(gs);
    auto owningPackage = data->owner.data(gs)->package;
    if (!owningPackage.exists() || gs.packageDB().getPackageInfo(owningPackage).locs.exportAll.exists() ||
        data->flags.isExported) {
        return;
    }

    for (auto &f : referencingFiles) {
        auto packageForF = gs.packageDB().getPackageNameForFile(f);
        if (packageForF == owningPackage || gs.packageDB().allowRelaxedPackagerChecksFor(packageForF)) {
            continue;
        }

        if (ownerWillBeExported(gs, toExport[owningPackage], data->owner)) {
            // No need to check the rest of referencingFiles, we're already going to export the owner
            break;
        }

        auto maybeEnumClass = getEnumClassForEnumValue(gs, core::SymbolRef(symbol));
        if (maybeEnumClass.exists()) {
            // No need to check if maybeEnumClass is already going to be exported since we have a ownerWillBeExported
            // call above
            toExport[owningPackage].push_back(maybeEnumClass);
        } else {
            toExport[owningPackage].push_back(symbol);
        }
        break;
    }
}
}; // namespace

void GenPackages::run(core::GlobalState &gs) {
    Timer timeit(gs.tracer(), "gen_packages.run");
    auto referencingFiles = UnorderedMap<core::SymbolRef, vector<core::FileRef>>{};
    {
        // symbolsReferencedByFile is a map from file -> [symbol] referenced in that file
        // This loop computes the inverse: referencingFiles is a map from symbol -> [file] that reference that symbol
        Timer timeit(gs.tracer(), "gen_packages.run.build_referencing_files");
        auto numFiles = gs.getFiles().size();
        for (auto i = 1; i < numFiles; i++) {
            core::FileRef fref(i);
            auto referencedSymbols = gs.getSymbolsReferencedByFile(fref);
            for (auto &symbol : referencedSymbols) {
                referencingFiles[symbol].push_back(fref);
            }
        }
    }

    auto toExport = UnorderedMap<core::packages::MangledName, vector<core::SymbolRef>>{};
    for (uint32_t i = 1; i < gs.classAndModulesUsed(); ++i) {
        auto classOrModuleRef = core::ClassOrModuleRef(gs, i);
        exportClassOrModule(gs, toExport, classOrModuleRef, referencingFiles[classOrModuleRef]);
    }
    for (uint32_t i = 1; i < gs.fieldsUsed(); ++i) {
        auto fieldRef = core::FieldRef(gs, i);
        exportField(gs, toExport, fieldRef, referencingFiles[fieldRef]);
    }

    auto neededVisibleTo = UnorderedMap<core::packages::MangledName, vector<core::packages::MangledName>>{};
    if (gs.packageDB().anyUpdateVisibilityFor()) {
        for (auto pkgName : gs.packageDB().packages()) {
            auto &pkgInfo = gs.packageDB().getPackageInfo(pkgName);
            ENFORCE(pkgInfo.exists());
            // TODO(neil): this loop is similar to what we have in aggregateMissingImports, is there a way to dedup?
            for (auto &[file, referencedPackages] : pkgInfo.packagesReferencedByFile) {
                for (auto &[referencedPackageName, packageReferenceInfo] : referencedPackages) {
                    auto &referencedPackageInfo = gs.packageDB().getPackageInfo(referencedPackageName);
                    ENFORCE(referencedPackageInfo.exists());
                    if (!referencedPackageInfo.isVisibleTo(gs, pkgInfo,
                                                           core::packages::PackageInfo::fileToImportType(gs, file))) {
                        // it is a visibility error for pkgName to reference referencedPackageName,
                        // so need to add visible_to pkgName to referencedPackageName's __package.db
                        neededVisibleTo[referencedPackageName].push_back(pkgName);
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
        auto visibleToAutocorrect = gs.packageDB().updateVisibilityFor(pkgName)
                                        ? pkgInfo.aggregateMissingVisibleTo(gs, neededVisibleTo[pkgName])
                                        : nullopt;

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

} // namespace sorbet::packager
