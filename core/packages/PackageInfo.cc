#include "core/packages/PackageInfo.h"
#include "absl/strings/str_join.h"
#include "common/sort/sort.h"
#include "core/GlobalState.h"
#include "core/Loc.h"
#include "core/NameRef.h"
#include "core/Symbols.h"

#include <queue>

using namespace std;

namespace sorbet::core::packages {
string_view strictDependenciesLevelToString(StrictDependenciesLevel level) {
    switch (level) {
        case StrictDependenciesLevel::None:
            ENFORCE(false, "Should never show strict_dependencies 'none' to the user!");
            return "none";
        case StrictDependenciesLevel::False:
            return "false";
        case StrictDependenciesLevel::Layered:
            return "layered";
        case StrictDependenciesLevel::LayeredDag:
            return "layered_dag";
        case StrictDependenciesLevel::Dag:
            return "dag";
    }
}

PackageInfo &PackageInfo::from(core::GlobalState &gs, MangledName pkg) {
    ENFORCE(pkg.exists());
    return *gs.packageDB().getPackageInfoNonConst(pkg);
}

const PackageInfo &PackageInfo::from(const core::GlobalState &gs, MangledName pkg) {
    ENFORCE(pkg.exists());
    return gs.packageDB().getPackageInfo(pkg);
}

unique_ptr<PackageInfo> PackageInfo::deepCopy() const {
    ENFORCE(exists());
    return make_unique<PackageInfo>(*this);
}

// What order should these packages be in the import list?
// Returns -1 if a should come before b, 0 if they are equivalent, and 1 if a should come after b.
//
// This method replicates the logic used at Stripe to order packages and thus has all of the associated "quirks".
// In particular, the ordering in a given package is a function of the strictness level of the package, and it is
// not a simple "false < layered < layered_dag < dag" ordering. The ordering is as follows:
// For strictDependenciesLevel::False:
// - layering violations
// - imports to 'false' packages
// - imports to 'layered' or stricter packages
// - test imports
// For strictDependenciesLevel::Layered and LayeredDag:
// - layering violations
// - imports to 'false' packages
// - imports to 'layered' or 'layered_dag' packages
// - imports to 'dag' packages
// - test imports
// For strictDependenciesLevel::Dag:
// - layering violations
// - imports to 'false', 'layered', or 'layered_dag' packages
// - imports to 'dag' packages
// - test imports
// TODO(neil): explain the rationale behind this ordering (ie. why is not the simple "false < layered < layered_dag
// < dag" ordering)
int PackageInfo::orderImports(const core::GlobalState &gs, const PackageInfo &a, bool aIsTestImport,
                              const PackageInfo &b, bool bIsTestImport) const {
    // Test imports always come last, and aren't sorted by `strict_dependencies`
    if (aIsTestImport && bIsTestImport) {
        return orderByAlphabetical(gs, a, b);
    } else if (aIsTestImport && !bIsTestImport) {
        return 1;
    } else if (!aIsTestImport && bIsTestImport) {
        return -1;
    } // Neither is a test import

    auto strictnessCompareResult = orderByStrictness(gs.packageDB(), a, b);
    if (strictnessCompareResult == 0) {
        return orderByAlphabetical(gs, a, b);
    }
    return strictnessCompareResult;
}

int PackageInfo::orderByStrictness(const PackageDB &packageDB, const PackageInfo &a, const PackageInfo &b) const {
    if (!packageDB.enforceLayering() || strictDependenciesLevel == StrictDependenciesLevel::None ||
        a.strictDependenciesLevel == StrictDependenciesLevel::None ||
        b.strictDependenciesLevel == StrictDependenciesLevel::None || !a.layer.exists() || !b.layer.exists()) {
        return 0;
    }

    // Layering violations always come first
    auto aCausesLayeringViolation = causesLayeringViolation(packageDB, a.layer);
    auto bCausesLayeringViolation = causesLayeringViolation(packageDB, b.layer);
    if (aCausesLayeringViolation && bCausesLayeringViolation) {
        return 0;
    } else if (aCausesLayeringViolation && !bCausesLayeringViolation) {
        return -1;
    } else if (!aCausesLayeringViolation && bCausesLayeringViolation) {
        return 1;
    }

    switch (strictDependenciesLevel) {
        case StrictDependenciesLevel::None: {
            Exception::raise("Early exited from orderByStrictness");
        }

        case StrictDependenciesLevel::False: {
            // Sort order: Layering violations, false, layered or stricter
            switch (a.strictDependenciesLevel) {
                case StrictDependenciesLevel::None: {
                    Exception::raise("Early exited from orderByStrictness");
                }
                case StrictDependenciesLevel::False:
                    return b.strictDependenciesLevel == StrictDependenciesLevel::False ? 0 : -1;
                case StrictDependenciesLevel::Layered:
                case StrictDependenciesLevel::LayeredDag:
                case StrictDependenciesLevel::Dag:
                    return b.strictDependenciesLevel == StrictDependenciesLevel::False ? 1 : 0;
            }
        }
        case StrictDependenciesLevel::Layered:
        case StrictDependenciesLevel::LayeredDag: {
            // Sort order: Layering violations, false, layered or layered_dag, dag
            switch (a.strictDependenciesLevel) {
                case StrictDependenciesLevel::None: {
                    Exception::raise("Early exited from orderByStrictness");
                }
                case StrictDependenciesLevel::False:
                    return b.strictDependenciesLevel == StrictDependenciesLevel::False ? 0 : -1;
                case StrictDependenciesLevel::Layered:
                case StrictDependenciesLevel::LayeredDag:
                    switch (b.strictDependenciesLevel) {
                        case StrictDependenciesLevel::None: {
                            Exception::raise("Early exited from orderByStrictness");
                        }
                        case StrictDependenciesLevel::False:
                            return 1;
                        case StrictDependenciesLevel::Layered:
                        case StrictDependenciesLevel::LayeredDag:
                            return 0;
                        case StrictDependenciesLevel::Dag:
                            return -1;
                    }
                case StrictDependenciesLevel::Dag:
                    return b.strictDependenciesLevel == StrictDependenciesLevel::Dag ? 0 : 1;
            }
        }
        case StrictDependenciesLevel::Dag: {
            // Sort order: Layering violations, false or layered or layered_dag, dag
            switch (a.strictDependenciesLevel) {
                case StrictDependenciesLevel::None: {
                    Exception::raise("Early exited from orderByStrictness");
                }
                case StrictDependenciesLevel::False:
                case StrictDependenciesLevel::Layered:
                case StrictDependenciesLevel::LayeredDag:
                    return b.strictDependenciesLevel == StrictDependenciesLevel::Dag ? -1 : 0;
                case StrictDependenciesLevel::Dag:
                    return b.strictDependenciesLevel == StrictDependenciesLevel::Dag ? 0 : 1;
            }
        }
    }
}

int PackageInfo::orderByAlphabetical(const core::GlobalState &gs, const PackageInfo &a, const PackageInfo &b) const {
    auto aStrName = a.show(gs);
    auto bStrName = b.show(gs);
    if (aStrName == bStrName) {
        return 0;
    }
    return aStrName < bStrName ? -1 : 1;
}

// autocorrects

optional<core::AutocorrectSuggestion> PackageInfo::addImport(const core::GlobalState &gs, const PackageInfo &info,
                                                             ImportType importType) const {
    if (gs.packageDB().testPackages()) {
        if (importType != ImportType::Normal) {
            ENFORCE(false, "addImport called to add test_import when --test-packages is enabled");
            return nullopt;
        }
    }
    auto insertionLoc = core::Loc::none(this->file);
    optional<core::AutocorrectSuggestion::Edit> deleteTestImportEdit = nullopt;

    if (!importedPackageNames.empty()) {
        core::LocOffsets importToInsertAfter;
        for (auto &import : importedPackageNames) {
            if (import.mangledName == info.mangledName()) {
                if ((importType == ImportType::Normal && import.type != ImportType::Normal) ||
                    (importType == ImportType::TestHelper && import.type == ImportType::TestUnit)) {
                    // There's already an import for this package, so we'll "upgrade" it to the desired import.
                    // importToInsertAfter already tracks where we need to insert the import.  So we can craft an
                    // edit to delete the existing line, and then use the regular logic for adding an import to
                    // insert the `import`.
                    auto importLoc = core::Loc(fullLoc().file(), import.loc);
                    auto [lineStart, numWhitespace] = importLoc.findStartOfIndentation(gs);
                    auto beginPos =
                        lineStart.adjust(gs, -numWhitespace, 0).beginPos(); // -numWhitespace for the indentation
                    auto endPos = importLoc.endPos();
                    core::Loc replaceLoc(importLoc.file(), beginPos, endPos);

                    deleteTestImportEdit = {replaceLoc, ""};

                    // as a special-case: if we're converting a `test_import` to remove `only:`, then we want to
                    // re-insert it at this exact same point (since we sort those together.) Let's find the previous
                    // import!
                    // TODO: we should find and delete just the `, only: ...` in this case, but that's slightly
                    // tricky

                    if (importType == ImportType::TestHelper) {
                        insertionLoc = {importLoc.file(), beginPos - 1, beginPos - 1};
                        break;
                    }
                } else {
                    // we already import this, and if so, don't return an autocorrect
                    return nullopt;
                }
            }

            auto &importInfo = gs.packageDB().getPackageInfo(import.mangledName);
            if (!importInfo.exists()) {
                importToInsertAfter = import.loc;
                continue;
            }

            auto compareResult =
                orderImports(gs, info, importType != ImportType::Normal, importInfo, import.isTestImport());
            if (compareResult == 1 || compareResult == 0) {
                importToInsertAfter = import.loc;
            }
        }
        if (!importToInsertAfter.exists()) {
            // Insert before the first import
            core::Loc beforePackageName = {this->file, importedPackageNames.front().loc};
            auto [beforeImport, numWhitespace] = beforePackageName.findStartOfIndentation(gs);
            auto endOfPrevLine = beforeImport.adjust(gs, -numWhitespace - "\n"sv.size(), 0);
            insertionLoc = endOfPrevLine.copyWithZeroLength();
        } else {
            insertionLoc = core::Loc(this->file, importToInsertAfter.copyEndWithZeroLength());
        }
    } else {
        // if we don't have any imports, then we can try adding it
        // either before the first export, or if we have no
        // exports, then right before the final `end`
        int64_t exportOffset;
        if (!exports_.empty()) {
            exportOffset = exports_.front().loc.beginPos() - " "sv.size();
        } else {
            exportOffset = locs.loc.endPos() - "end\n"sv.size();
        }

        string_view file_source = this->file.data(gs).source();

        // Defensively guard against the first export loc or the package's loc being invalid.
        if (exportOffset <= 0 || exportOffset >= file_source.size()) {
            ENFORCE(false, "Failed to find a valid starting loc");
            return nullopt;
        }

        // we want to find the end of the last non-empty line, so
        // let's do something gross: walk backward until we find non-whitespace
        while (isspace(file_source[exportOffset])) {
            exportOffset--;
            // this shouldn't happen in a well-formatted
            // `__package.rb` file, but just to be safe
            if (exportOffset == 0) {
                return nullopt;
            }
        }
        insertionLoc = core::Loc(this->file, exportOffset + 1, exportOffset + 1);
    }
    ENFORCE(insertionLoc.exists());

    auto packageToImport = info.mangledName_.owner.show(gs);
    string_view importTypeHuman;
    string_view importTypeMethod;
    string_view importTypeTrailing = "";
    switch (importType) {
        case ImportType::Normal:
            importTypeHuman = "Import";
            importTypeMethod = "import";
            break;
        case ImportType::TestUnit:
            importTypeHuman = "Test Import";
            importTypeMethod = "test_import";
            importTypeTrailing = ", only: \"test_rb\"";
            break;
        case ImportType::TestHelper:
            importTypeHuman = "Test Import";
            importTypeMethod = "test_import";
            break;
    }
    auto suggestionTitle =
        fmt::format("{} `{}` in package `{}`", importTypeHuman, packageToImport, mangledName_.owner.show(gs));
    vector<core::AutocorrectSuggestion::Edit> edits = {
        {insertionLoc, fmt::format("\n  {} {}{}", importTypeMethod, packageToImport, importTypeTrailing)}};
    if (deleteTestImportEdit.has_value()) {
        edits.push_back(deleteTestImportEdit.value());
        suggestionTitle = fmt::format("Convert existing import to `{}`", importTypeMethod);
    }

    core::AutocorrectSuggestion suggestion(suggestionTitle, edits);
    return {suggestion};
}

optional<core::AutocorrectSuggestion> PackageInfo::addExport(const core::GlobalState &gs,
                                                             const core::SymbolRef newExport) const {
    auto newExportName = newExport.show(gs);
    auto exportLine = fmt::format("export {}", newExportName);
    auto pkgFile = this->file;
    auto insertionLoc = core::Loc::none(pkgFile);
    if (!exports_.empty()) {
        // TODO(neil): can we use use `absl::c_lower_bound` here?
        core::LocOffsets exportToInsertAfter;
        for (auto &e : exports_) {
            if (exportLine > core::Loc(pkgFile, e.loc).source(gs)) {
                exportToInsertAfter = e.loc;
            }
        }
        if (!exportToInsertAfter.exists()) {
            // Insert before the first export
            auto beforeConstantName = exports_.front().loc;
            auto [beforeExport, numWhitespace] = core::Loc(pkgFile, beforeConstantName).findStartOfIndentation(gs);
            auto endOfPrevLine = beforeExport.adjust(gs, -numWhitespace - "\n"sv.size(), 0);
            insertionLoc = endOfPrevLine.copyWithZeroLength();
        } else {
            insertionLoc = core::Loc(pkgFile, exportToInsertAfter.copyEndWithZeroLength());
        }
    } else {
        // if we don't have any exports, then we can try adding it right before the final `end`
        uint32_t exportOffset = this->locs.loc.endPos() - "end\n"sv.size();
        // we want to find the end of the last non-empty line, so
        // let's do something gross: walk backward until we find non-whitespace
        const auto file_source = this->file.data(gs).source();
        while (isspace(file_source[exportOffset])) {
            exportOffset--;
            // this shouldn't happen in a well-formatted
            // `__package.rb` file, but just to be safe
            if (exportOffset == 0) {
                return nullopt;
            }
        }
        insertionLoc = {this->file, exportOffset + 1, exportOffset + 1};
    }
    ENFORCE(insertionLoc.exists());

    core::AutocorrectSuggestion suggestion(
        fmt::format("Export `{}` in package `{}`", newExportName, mangledName_.owner.show(gs)),
        {{insertionLoc, fmt::format("\n  {}", exportLine)}});
    return {suggestion};
}

optional<core::AutocorrectSuggestion> PackageInfo::addVisibleToTests(const core::GlobalState &gs) const {
    auto visibleToLine = "visible_to 'tests'";
    auto pkgFile = this->file;
    auto insertionLoc = core::Loc::none(pkgFile);
    if (!visibleTo_.empty()) {
        insertionLoc = core::Loc(pkgFile, visibleTo_.back().loc.copyEndWithZeroLength());
    } else {
        // if we don't have any visible_to entries, then we can try adding it right before the final `end`
        uint32_t visibleToOffset = this->locs.loc.endPos() - "end\n"sv.size();
        // we want to find the end of the last non-empty line, so
        // let's do something gross: walk backward until we find non-whitespace
        const auto file_source = this->file.data(gs).source();
        while (isspace(file_source[visibleToOffset])) {
            visibleToOffset--;
            // this shouldn't happen in a well-formatted
            // `__package.rb` file, but just to be safe
            if (visibleToOffset == 0) {
                return nullopt;
            }
        }
        insertionLoc = {this->file, visibleToOffset + 1, visibleToOffset + 1};
    }
    ENFORCE(insertionLoc.exists());

    core::AutocorrectSuggestion suggestion(
        fmt::format("Add `visible_to 'tests'` in package `{}`", mangledName_.owner.show(gs)),
        {{insertionLoc, fmt::format("\n  {}", visibleToLine)}});
    return {suggestion};
}

optional<core::AutocorrectSuggestion> PackageInfo::addVisibleTo(const core::GlobalState &gs,
                                                                const MangledName &targetPackage) const {
    auto targetPackageName = targetPackage.owner.show(gs);
    auto visibleToLine = fmt::format("visible_to {}", targetPackageName);
    auto pkgFile = this->file;
    auto insertionLoc = core::Loc::none(pkgFile);
    if (!visibleTo_.empty()) {
        // TODO(neil): can we use use `absl::c_lower_bound` here?
        core::LocOffsets visibleToInsertAfter;
        for (auto &v : visibleTo_) {
            if (visibleToLine > core::Loc(pkgFile, v.loc).source(gs)) {
                visibleToInsertAfter = v.loc;
            }
        }
        if (!visibleToInsertAfter.exists()) {
            // Insert before the first visible_to
            auto beforeVisibleTo = visibleTo_.front().loc;
            auto [beforeVisibleToLoc, numWhitespace] = core::Loc(pkgFile, beforeVisibleTo).findStartOfIndentation(gs);
            auto endOfPrevLine = beforeVisibleToLoc.adjust(gs, -numWhitespace - "\n"sv.size(), 0);
            insertionLoc = endOfPrevLine.copyWithZeroLength();
        } else {
            insertionLoc = core::Loc(pkgFile, visibleToInsertAfter.copyEndWithZeroLength());
        }
    } else {
        // if we don't have any visible_to entries, then we can try adding it right before the final `end`
        uint32_t visibleToOffset = this->locs.loc.endPos() - "end\n"sv.size();
        // we want to find the end of the last non-empty line, so
        // let's do something gross: walk backward until we find non-whitespace
        const auto file_source = this->file.data(gs).source();
        while (isspace(file_source[visibleToOffset])) {
            visibleToOffset--;
            // this shouldn't happen in a well-formatted
            // `__package.rb` file, but just to be safe
            if (visibleToOffset == 0) {
                return nullopt;
            }
        }
        insertionLoc = {this->file, visibleToOffset + 1, visibleToOffset + 1};
    }
    ENFORCE(insertionLoc.exists());

    core::AutocorrectSuggestion suggestion(
        fmt::format("Add `visible_to {}` in package `{}`", targetPackageName, mangledName_.owner.show(gs)),
        {{insertionLoc, fmt::format("\n  {}", visibleToLine)}});
    return {suggestion};
}

const Import *PackageInfo::importsPackage(MangledName mangledName) const {
    if (!mangledName.exists()) {
        return nullptr;
    }

    auto imp = absl::c_find_if(importedPackageNames, [mangledName](auto &i) { return i.mangledName == mangledName; });
    if (imp == importedPackageNames.end()) {
        return nullptr;
    }

    return &*imp;
}

// Is it a layering violation to import otherPkg from this package?
bool PackageInfo::causesLayeringViolation(const PackageDB &packageDB, const PackageInfo &otherPkg) const {
    ENFORCE(exists());
    if (!otherPkg.layer.exists()) {
        return false;
    }

    return causesLayeringViolation(packageDB, otherPkg.layer);
}

bool PackageInfo::causesLayeringViolation(const PackageDB &packageDB, core::NameRef otherPkgLayer) const {
    ENFORCE(exists());
    if (!layer.exists()) {
        return false;
    }

    auto pkgLayer = layer;
    auto pkgLayerIndex = packageDB.layerIndex(pkgLayer);
    auto otherPkgLayerIndex = packageDB.layerIndex(otherPkgLayer);

    return pkgLayerIndex < otherPkgLayerIndex;
}

// What is the minimum strict dependencies level that this package's imports must have?
StrictDependenciesLevel PackageInfo::minimumStrictDependenciesLevel() const {
    switch (strictDependenciesLevel) {
        case StrictDependenciesLevel::None:
        case StrictDependenciesLevel::False:
            return StrictDependenciesLevel::False;
        case StrictDependenciesLevel::Layered:
        case StrictDependenciesLevel::LayeredDag:
            return StrictDependenciesLevel::Layered;
        case StrictDependenciesLevel::Dag:
            return StrictDependenciesLevel::Dag;
    }
}

namespace {
string renderPath(const core::GlobalState &gs, const vector<MangledName> &path) {
    // TODO(neil): if the cycle has a large number of nodes (10?), show partial path (first 5, ... (n omitted), last
    // 5) to prevent error being too long
    // Note: This function iterates through path in reverse order because pathTo generates it in that order, so
    // iterating reverse gives the regular order.
    string pathMessage;
    for (int i = path.size() - 1; i >= 0; i--) {
        auto name = gs.packageDB().getPackageInfo(path[i]).show(gs);
        bool showArrow = i > 0;
        pathMessage += core::ErrorColors::format("    `{}`{}\n", name, showArrow ? " →" : "");
    }
    return pathMessage;
}

} // namespace

// Returns a string representing the path to the given package from this package, if it exists. Note: this only
// looks at non-test imports.
optional<string> PackageInfo::pathTo(const core::GlobalState &gs, const MangledName dest) const {
    ENFORCE(exists());
    // Note: This implements BFS.
    auto src = mangledName();
    queue<MangledName> toVisit;
    // Maps from package to what package we came from to get to that package, used to construct the path
    // Ex. A -> B -> C means that prev[C] = B and prev[B] = A
    UnorderedMap<MangledName, MangledName> prev;
    UnorderedSet<MangledName> visited;

    toVisit.push(src);
    while (!toVisit.empty()) {
        auto curr = toVisit.front();
        toVisit.pop();
        auto [_it, inserted] = visited.emplace(curr);
        if (!inserted) {
            continue;
        }

        if (curr == dest) {
            // Found the target node, walk backward through the prev map to construct the path
            vector<MangledName> path;
            path.push_back(curr);
            while (curr != src) {
                curr = prev[curr];
                path.push_back(curr);
            }
            // Note: here, path will be in reverse order (ie. from dest -> src), and then renderPath iterates it in
            // reverse, to get the correct order. If you plan to use path directly, make sure to reverse it (and
            // then upate renderPath to iterate normally).
            return renderPath(gs, path);
        }

        auto &currInfo = gs.packageDB().getPackageInfo(curr);
        for (auto &import : currInfo.importedPackageNames) {
            auto &importInfo = gs.packageDB().getPackageInfo(import.mangledName);
            if (!importInfo.exists() || import.isTestImport() || visited.contains(import.mangledName)) {
                continue;
            }
            if (!prev.contains(import.mangledName)) {
                prev[import.mangledName] = curr;
            }
            toVisit.push(import.mangledName);
        }
    }

    // No path found
    return nullopt;
}

bool PackageInfo::isVisibleTo(const core::GlobalState &gs, const PackageInfo &importingPkgInfo,
                              const ImportType importType) const {
    if (visibleToEverything()) {
        return true;
    }

    if (gs.packageDB().testPackages()) {
        ENFORCE(importType == ImportType::Normal);
        if (visibleToTests() && importingPkgInfo.testPackage()) {
            return true;
        }
    } else {
        if (visibleToTests() && importType != ImportType::Normal) {
            return true;
        }
    }

    auto importingPkgName = importingPkgInfo.mangledName();

    return absl::c_any_of(visibleTo(), [&](const auto &vt) {
        if (vt.type == VisibleToType::Wildcard) {
            // a wildcard will match if it's a proper prefix of the package name
            auto curPkg = importingPkgName.owner;
            auto prefix = vt.mangledName.owner;
            do {
                if (curPkg == prefix) {
                    return true;
                }
                curPkg = curPkg.data(gs)->owner;
            } while (curPkg != core::Symbols::root());

            return false;
        } else {
            // otherwise it needs to be the same
            return vt.mangledName == importingPkgName;
        }
    });
}

void PackageInfo::trackPackageReferences(FileRef file, vector<pair<MangledName, PackageReferenceInfo>> &references) {
    packagesReferencedByFile[file].swap(references);
}

core::packages::ImportType PackageInfo::fileToImportType(const core::GlobalState &gs, core::FileRef file) {
    if (gs.packageDB().testPackages()) {
        return core::packages::ImportType::Normal;
    }

    if (file.data(gs).isPackagedTestHelper()) {
        return core::packages::ImportType::TestHelper;
    } else if (file.data(gs).isPackagedTest()) {
        return core::packages::ImportType::TestUnit;
    } else {
        return core::packages::ImportType::Normal;
    }
}

std::optional<core::AutocorrectSuggestion> PackageInfo::aggregateMissingImports(const core::GlobalState &gs) const {
    std::vector<core::AutocorrectSuggestion::Edit> allEdits;
    UnorderedMap<core::packages::MangledName, core::packages::ImportType> toImport;
    for (auto &[file, referencedPackages] : packagesReferencedByFile) {
        auto importType = fileToImportType(gs, file);
        for (auto &[packageName, packageReferenceInfo] : referencedPackages) {
            auto &pkgInfo = gs.packageDB().getPackageInfo(packageName);
            if (!packageReferenceInfo.importNeeded || packageReferenceInfo.causesModularityError || !pkgInfo.exists()) {
                continue;
            }

            // We should only skip adding the import if we're not going to add a visible_to to the package, since it
            // will be valid to import the package after the new visible_to is added
            if (!pkgInfo.isVisibleTo(gs, *this, importType) && !gs.packageDB().updateVisibilityFor(packageName)) {
                continue;
            }

            auto it = toImport.find(packageName);
            if (it != toImport.end()) {
                auto currentBroadestImport = it->second;
                if (importType < currentBroadestImport) {
                    it->second = importType;
                }
            } else {
                toImport[packageName] = importType;
            }
        }
    }
    for (auto &[packageName, importType] : toImport) {
        auto &pkgInfo = gs.packageDB().getPackageInfo(packageName);
        auto autocorrect = this->addImport(gs, pkgInfo, importType);
        if (autocorrect.has_value()) {
            allEdits.insert(allEdits.end(), make_move_iterator(autocorrect.value().edits.begin()),
                            make_move_iterator(autocorrect.value().edits.end()));
        }
    }
    if (allEdits.empty()) {
        return nullopt;
    }
    AutocorrectSuggestion::mergeAdjacentEdits(allEdits);
    return core::AutocorrectSuggestion{"Add missing imports", std::move(allEdits)};
}

std::optional<core::AutocorrectSuggestion>
PackageInfo::aggregateMissingExports(const core::GlobalState &gs, vector<core::SymbolRef> &toExport) const {
    std::vector<core::AutocorrectSuggestion::Edit> allEdits;
    for (auto &symbol : toExport) {
        switch (symbol.kind()) {
            case core::SymbolRef::Kind::ClassOrModule:
                if (symbol.asClassOrModuleRef().data(gs)->flags.isExported) {
                    continue;
                }
                break;
            case core::SymbolRef::Kind::FieldOrStaticField:
                if (symbol.asFieldRef().data(gs)->flags.isExported) {
                    continue;
                }
                break;
            case core::SymbolRef::Kind::TypeMember:
            case core::SymbolRef::Kind::Method:
            case core::SymbolRef::Kind::TypeParameter:
                ENFORCE(false, "trying to export something other than ClassOrModule or FieldOrStaticField");
        }
        auto autocorrect = addExport(gs, symbol);
        if (autocorrect.has_value()) {
            allEdits.insert(allEdits.end(), make_move_iterator(autocorrect.value().edits.begin()),
                            make_move_iterator(autocorrect.value().edits.end()));
        }
    }

    if (allEdits.empty()) {
        return nullopt;
    }

    AutocorrectSuggestion::mergeAdjacentEdits(allEdits);
    return core::AutocorrectSuggestion{"Add missing exports", std::move(allEdits)};
}

std::optional<core::AutocorrectSuggestion> PackageInfo::aggregateMissingVisibleTo(
    const core::GlobalState &gs, std::vector<core::packages::MangledName> &visibleTos, bool visibleToTests) const {
    std::vector<core::AutocorrectSuggestion::Edit> allEdits;
    for (auto &pkgName : visibleTos) {
        auto autocorrect = addVisibleTo(gs, pkgName);
        if (autocorrect.has_value()) {
            allEdits.insert(allEdits.end(), make_move_iterator(autocorrect.value().edits.begin()),
                            make_move_iterator(autocorrect.value().edits.end()));
        }
    }

    if (visibleToTests) {
        auto autocorrect = addVisibleToTests(gs);
        if (autocorrect.has_value()) {
            allEdits.insert(allEdits.end(), make_move_iterator(autocorrect.value().edits.begin()),
                            make_move_iterator(autocorrect.value().edits.end()));
        }
    }

    if (allEdits.empty()) {
        return nullopt;
    }

    AutocorrectSuggestion::mergeAdjacentEdits(allEdits);
    return core::AutocorrectSuggestion{fmt::format("Add missing `{}`", "visible_to"), std::move(allEdits)};
}

std::string PackageInfo::renderPackageRbContents(const core::GlobalState &gs) const {
    fmt::memory_buffer result;

    if (isPreludePackage()) {
        fmt::format_to(std::back_inserter(result), "  prelude_package\n\n");
    }

    for (auto &directive : extraDirectives_) {
        auto directiveSource = core::Loc(file, directive).source(gs);
        ENFORCE(directiveSource.has_value());
        fmt::format_to(std::back_inserter(result), "  {}\n", directiveSource.value());
    }
    if (locs.layer.exists()) {
        fmt::format_to(std::back_inserter(result), "  layer '{}'\n", layer.show(gs));
    }
    if (locs.strictDependenciesLevel.exists()) {
        fmt::format_to(std::back_inserter(result), "  strict_dependencies '{}'\n",
                       strictDependenciesLevelToString(strictDependenciesLevel));
    }
    if (locs.minTypedLevel.exists() && locs.testsMinTypedLevel.exists()) {
        ENFORCE(!gs.packageDB().testPackages());
        fmt::format_to(std::back_inserter(result), "  sorbet min_typed_level: '{}', tests_min_typed_level: '{}'\n",
                       core::SigilTraits<core::StrictLevel>::toString(minTypedLevel),
                       core::SigilTraits<core::StrictLevel>::toString(testsMinTypedLevel));
    } else if (locs.minTypedLevel.exists() && !locs.testsMinTypedLevel.exists()) {
        ENFORCE(gs.packageDB().testPackages());
        fmt::format_to(std::back_inserter(result), "  sorbet min_typed_level: '{}'\n",
                       core::SigilTraits<core::StrictLevel>::toString(minTypedLevel));
    }

    // currentPackageStrictDependenciesLevel -> vector<StrictDependenciesLevelForCategory, header>
    //
    // The import list in a __package.rb has headers to organize imports by their strict_dependencies level. For
    // example:
    //
    //   # strict_dependencies 'false':
    //   import FalsePackage
    //
    //   # strict_dependencies 'layered' or higher:
    //   import LayeredPackage
    //   import DagPackage
    //
    // This map contains the information required to intersperse those headers. When an with a strict_dependencies level
    // equal or higher to StrictDependenciesLevelForCategory is encountered, header will be printed once.
    //
    // NOTE: This information (the categories and how they are sorted) is duplicated with orderByStrictness.
    static const UnorderedMap<StrictDependenciesLevel, vector<pair<StrictDependenciesLevel, string>>> headerMap = {
        {StrictDependenciesLevel::None, {}},
        {StrictDependenciesLevel::False,
         {{StrictDependenciesLevel::False, "  # strict_dependencies 'false':\n"},
          {StrictDependenciesLevel::Layered, "  # strict_dependencies 'layered' or more strict:\n"}}},
        {StrictDependenciesLevel::Layered,
         {{StrictDependenciesLevel::False, "  # strict_dependencies 'false':\n"},
          {StrictDependenciesLevel::Layered, "  # strict_dependencies 'layered' or 'layered_dag':\n"},
          {StrictDependenciesLevel::Dag, "  # strict_dependencies 'dag':\n"}}},
        {StrictDependenciesLevel::LayeredDag,
         {{StrictDependenciesLevel::False, "  # strict_dependencies 'false':\n"},
          {StrictDependenciesLevel::Layered, "  # strict_dependencies 'layered' or 'layered_dag':\n"},
          {StrictDependenciesLevel::Dag, "  # strict_dependencies 'dag':\n"}}},
        {StrictDependenciesLevel::Dag,
         {{StrictDependenciesLevel::False, "  # strict_dependencies 'false', 'layered', or 'layered_dag':\n"},
          {StrictDependenciesLevel::Dag, "  # strict_dependencies 'dag':\n"}}},
    };

    // NOTE: this loop assumes importedPackageNames are already sorted by orderImports
    // TODO(neil): sort the imports
    bool layeringViolationsHeaderShown = false;
    bool testImportNewLineAdded = false;

    ENFORCE(headerMap.find(strictDependenciesLevel) != headerMap.end(),
            "should not happen, was a new StrictDependenciesLevel added?");
    auto &headers = headerMap.find(strictDependenciesLevel)->second;
    auto headersIt = headers.begin();
    for (auto &import : importedPackageNames) {
        auto impPackageName = import.mangledName.owner.show(gs);
        auto &impPkgInfo = gs.packageDB().getPackageInfo(import.mangledName);
        if (!impPkgInfo.exists()) {
            continue;
        }

        if (gs.packageDB().enforceLayering() && import.type == ImportType::Normal) {
            if (!layeringViolationsHeaderShown && causesLayeringViolation(gs.packageDB(), impPkgInfo)) {
                fmt::format_to(std::back_inserter(result), "\n  # layering violations:\n");
                layeringViolationsHeaderShown = true;
            }

            if (!causesLayeringViolation(gs.packageDB(), impPkgInfo)) {
                auto headerToPrint = string();
                // Find the last entry in [headersIt, headers.end()] that has a StrictDependenciesLevel less than or
                // equal to the StrictDependenciesLevel of the package being imported. It has to be last one rather than
                // just the next to handle headers being skipped.
                //
                // Ex. if only an dag package is imported, we want to skip the 'false' and 'layered'/'layered_dag'
                // headers, even though they'll be less than the dag package being imported.
                while (headersIt != headers.end() && headersIt->first <= impPkgInfo.strictDependenciesLevel) {
                    headerToPrint = headersIt->second;
                    headersIt++;
                }
                if (headerToPrint != "") {
                    fmt::format_to(std::back_inserter(result), "\n{}", headerToPrint);
                }
            }
        }

        if (!testImportNewLineAdded && import.type != ImportType::Normal) {
            fmt::format_to(std::back_inserter(result), "\n");
            testImportNewLineAdded = true;
        }

        switch (import.type) {
            case ImportType::Normal:
                if (import.usesInternals) {
                    ENFORCE(gs.packageDB().testPackages());
                    fmt::format_to(std::back_inserter(result), "  import {}, uses_internals: true\n", impPackageName);
                } else {
                    fmt::format_to(std::back_inserter(result), "  import {}\n", impPackageName);
                }
                break;
            case ImportType::TestHelper:
                ENFORCE(!gs.packageDB().testPackages());
                fmt::format_to(std::back_inserter(result), "  test_import {}\n", impPackageName);
                break;
            case ImportType::TestUnit:
                ENFORCE(!gs.packageDB().testPackages());
                fmt::format_to(std::back_inserter(result), "  test_import {}, only: \"test_rb\"\n", impPackageName);
                break;
        }
    }

    if (locs.exportAll.exists() || !exports_.empty()) {
        fmt::format_to(std::back_inserter(result), "\n");
    }
    if (locs.exportAll.exists()) {
        fmt::format_to(std::back_inserter(result), "  export_all!\n");
    } else {
        // TODO(neil): sort the exports
        for (auto &export_ : exports_) {
            auto exportSource = core::Loc(file, export_.loc).source(gs);
            ENFORCE(exportSource.has_value());
            fmt::format_to(std::back_inserter(result), "  {}\n", exportSource.value());
        }
    }

    if (!visibleTo().empty() || visibleToTests()) {
        fmt::format_to(std::back_inserter(result), "\n");
    }
    // TODO(neil): sort the visible_to
    for (auto &visibleTo : visibleTo()) {
        auto visibleToPackageName = visibleTo.mangledName.owner.show(gs);
        switch (visibleTo.type) {
            case VisibleToType::Normal:
                fmt::format_to(std::back_inserter(result), "  visible_to {}\n", visibleToPackageName);
                break;
            case VisibleToType::Wildcard:
                fmt::format_to(std::back_inserter(result), "  visible_to {}::*\n", visibleToPackageName);
                break;
        }
    }

    if (visibleToTests()) {
        fmt::format_to(std::back_inserter(result), "  visible_to 'tests'\n");
    }
    return to_string(result);
}

bool PackageInfo::operator==(const PackageInfo &rhs) const {
    return mangledName() == rhs.mangledName();
}

string PackageInfo::show(const core::GlobalState &gs) const {
    return this->mangledName().owner.show(gs);
}

namespace {

void addChildren(vector<ClassOrModuleRef> &work, core::ConstClassOrModuleData klass) {
    // This is an overallocation as we'll be skipping some members.
    work.reserve(work.size() + klass->members().size());
    for (auto &[key, sym] : klass->members()) {
        if (!sym.isClassOrModule()) {
            continue;
        }

        work.push_back(sym.asClassOrModuleRef());
    }
}

} // namespace

vector<MangledName> PackageInfo::directSubPackages(const core::GlobalState &gs) const {
    ENFORCE(this->exists());
    vector<MangledName> subpackages;

    vector<ClassOrModuleRef> work;
    addChildren(work, this->mangledName_.owner.data(gs));

    // Termination argument: we only have one loop in the hierarchy for root, ignoring the singleton/attached class
    // cycle for each symbol, and as we know we're already calling this for a valid package and only processing its
    // non-singleton class members, we know we won't find a cycle.
    while (!work.empty()) {
        auto sym = work.back();
        auto klass = sym.data(gs);
        work.pop_back();

        if (klass->isSingletonClass(gs)) {
            continue;
        }

        // This handles cases like:
        // A < PackageSpec
        // └─ A::B
        //    └─ A::B::Subpackage < PackageSpec
        if (klass->superClass() != core::Symbols::PackageSpec()) {
            addChildren(work, klass);
            continue;
        }

        subpackages.push_back(MangledName(sym));
    }

    if (subpackages.size() > 1) {
        // This will sort `A::Z::A` before `A::B`.
        fast_sort(subpackages, [&gs](MangledName l, MangledName r) {
            return l.owner.data(gs)->name.shortName(gs) < r.owner.data(gs)->name.shortName(gs);
        });
    }

    return subpackages;
}

PackageInfo::CanModifyResult PackageInfo::canModifySymbol(const core::GlobalState &gs, ClassOrModuleRef sym) const {
    ENFORCE(sym.exists());

    // Unpackaged code is allowed to modify anything.
    if (!this->exists()) {
        return CanModifyResult::CanModify;
    }

    // Ensure we're not working with a singleton class before performing any further checks.
    sym = sym.data(gs)->topAttachedClass(gs);

    auto symData = sym.data(gs);

    // It's not okay to modify anything in the package hierarchy, as we don't support packages with generics, or mixins
    // on packages.
    if (symData->packageRegistryOwner == sym) {
        return CanModifyResult::PackageSpec;
    }

    auto symPackage = symData->package;

    // The symbol is already owned by this package, so we only need to check if the symbol corresponds to the package
    // namespace, and if so that there aren't any subpackages, as that could introduce ordering dependencies that don't
    // work with package-directed type checking.
    if (symPackage == this->mangledName_) {
        if (!this->hasSubPackages || !symData->isPackageNamespace()) {
            return CanModifyResult::CanModify;
        } else {
            return CanModifyResult::Subpackages;
        }
    }

    // Modifying an unpackaged symbol is only allowed from prelude packages.
    if (!symPackage.exists()) {
        if (this->isPreludePackage()) {
            return CanModifyResult::CanModify;
        } else {
            return CanModifyResult::UnpackagedSymbol;
        }
    }

    // At this point, we know we're in a packaged context, trying to modify the symbol of another package, which is not
    // allowed.
    return CanModifyResult::NotOwner;
}

bool PackageInfo::canAccessInternalsOf(bool testPackages, MangledName other) const {
    ENFORCE(this->exists());
    ENFORCE(other.exists());

    if (this->mangledName_ == other) {
        return true;
    }

    if (!testPackages) {
        return false;
    }

    auto *imp = this->importsPackage(other);
    return imp && imp->usesInternals;
}

} // namespace sorbet::core::packages
