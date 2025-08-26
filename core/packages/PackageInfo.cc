#include "core/packages/PackageInfo.h"
#include "absl/strings/str_join.h"
#include "core/GlobalState.h"
#include "core/Loc.h"
#include "core/NameRef.h"
#include "core/Symbols.h"

#include <queue>

using namespace std;

namespace sorbet::core::packages {
string_view strictDependenciesLevelToString(StrictDependenciesLevel level) {
    switch (level) {
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

string FullyQualifiedName::show(const core::GlobalState &gs) const {
    return absl::StrJoin(parts, "::", NameFormatter(gs));
}

Import Import::prelude(MangledName mangledName, core::LocOffsets declLoc) {
    Import res{mangledName, ImportType::Normal, declLoc};
    res.isPrelude_ = true;
    return res;
}

bool Export::lexCmp(const Export &a, const Export &b) {
    return absl::c_lexicographical_compare(a.parts(), b.parts(),
                                           [](auto a, auto b) -> bool { return a.rawId() < b.rawId(); });
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
    if (!packageDB.enforceLayering() || !strictDependenciesLevel().has_value() ||
        !a.strictDependenciesLevel().has_value() || !b.strictDependenciesLevel().has_value() ||
        !a.layer().has_value() || !b.layer().has_value()) {
        return 0;
    }

    // Layering violations always come first
    auto aCausesLayeringViolation = causesLayeringViolation(packageDB, a.layer().value().first);
    auto bCausesLayeringViolation = causesLayeringViolation(packageDB, b.layer().value().first);
    if (aCausesLayeringViolation && bCausesLayeringViolation) {
        return 0;
    } else if (aCausesLayeringViolation && !bCausesLayeringViolation) {
        return -1;
    } else if (!aCausesLayeringViolation && bCausesLayeringViolation) {
        return 1;
    }

    auto aStrictDependenciesLevel = a.strictDependenciesLevel().value().first;
    auto bStrictDependenciesLevel = b.strictDependenciesLevel().value().first;
    switch (strictDependenciesLevel().value().first) {
        case StrictDependenciesLevel::False: {
            // Sort order: Layering violations, false, layered or stricter
            switch (aStrictDependenciesLevel) {
                case StrictDependenciesLevel::False:
                    return bStrictDependenciesLevel == StrictDependenciesLevel::False ? 0 : -1;
                case StrictDependenciesLevel::Layered:
                case StrictDependenciesLevel::LayeredDag:
                case StrictDependenciesLevel::Dag:
                    return bStrictDependenciesLevel == StrictDependenciesLevel::False ? 1 : 0;
            }
        }
        case StrictDependenciesLevel::Layered:
        case StrictDependenciesLevel::LayeredDag: {
            // Sort order: Layering violations, false, layered or layered_dag, dag
            switch (aStrictDependenciesLevel) {
                case StrictDependenciesLevel::False:
                    return bStrictDependenciesLevel == StrictDependenciesLevel::False ? 0 : -1;
                case StrictDependenciesLevel::Layered:
                case StrictDependenciesLevel::LayeredDag:
                    switch (bStrictDependenciesLevel) {
                        case StrictDependenciesLevel::False:
                            return 1;
                        case StrictDependenciesLevel::Layered:
                        case StrictDependenciesLevel::LayeredDag:
                            return 0;
                        case StrictDependenciesLevel::Dag:
                            return -1;
                    }
                case StrictDependenciesLevel::Dag:
                    return bStrictDependenciesLevel == StrictDependenciesLevel::Dag ? 0 : 1;
            }
        }
        case StrictDependenciesLevel::Dag: {
            // Sort order: Layering violations, false or layered or layered_dag, dag
            switch (aStrictDependenciesLevel) {
                case StrictDependenciesLevel::False:
                case StrictDependenciesLevel::Layered:
                case StrictDependenciesLevel::LayeredDag:
                    return bStrictDependenciesLevel == StrictDependenciesLevel::Dag ? -1 : 0;
                case StrictDependenciesLevel::Dag:
                    return bStrictDependenciesLevel == StrictDependenciesLevel::Dag ? 0 : 1;
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
    auto insertionLoc = core::Loc::none(loc.file());
    optional<core::AutocorrectSuggestion::Edit> deleteTestImportEdit = nullopt;

    // Find the first non-prelude import (if one exists) so that we don't recommend inserting near an implicit import
    // of a prelude package.
    auto firstImport = absl::c_find_if_not(importedPackageNames, [](auto &i) { return i.isPrelude(); });
    if (firstImport != importedPackageNames.end()) {
        core::LocOffsets importToInsertAfter;
        for (auto &import : importedPackageNames) {
            if (import.isPrelude()) {
                continue;
            }

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
            core::Loc beforePackageName = {loc.file(), firstImport->loc};
            auto [beforeImport, numWhitespace] = beforePackageName.findStartOfIndentation(gs);
            auto endOfPrevLine = beforeImport.adjust(gs, -numWhitespace - 1, 0);
            insertionLoc = endOfPrevLine.copyWithZeroLength();
        } else {
            insertionLoc = core::Loc(loc.file(), importToInsertAfter.copyEndWithZeroLength());
        }
    } else {
        // if we don't have any imports, then we can try adding it
        // either before the first export, or if we have no
        // exports, then right before the final `end`
        int64_t exportLoc;
        if (!exports_.empty()) {
            exportLoc = exports_.front().loc.beginPos() - "export "sv.size() - 1;
        } else {
            exportLoc = loc.endPos() - "end"sv.size() - 1;
        }

        string_view file_source = loc.file().data(gs).source();

        // Defensively guard against the first export loc or the package's loc being invalid.
        if (exportLoc <= 0 || exportLoc >= file_source.size()) {
            ENFORCE(false, "Failed to find a valid starting loc");
            return nullopt;
        }

        // we want to find the end of the last non-empty line, so
        // let's do something gross: walk backward until we find non-whitespace
        while (isspace(file_source[exportLoc])) {
            exportLoc--;
            // this shouldn't happen in a well-formatted
            // `__package.rb` file, but just to be safe
            if (exportLoc == 0) {
                return nullopt;
            }
        }
        insertionLoc = core::Loc(loc.file(), exportLoc + 1, exportLoc + 1);
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
    auto pkgFile = loc.file();
    auto insertionLoc = core::Loc::none(pkgFile);
    if (!exports_.empty()) {
        core::LocOffsets exportToInsertAfter;
        for (auto &e : exports_) {
            if (newExport.show(gs) > e.fqn.show(gs)) {
                exportToInsertAfter = e.loc;
            }
        }
        if (!exportToInsertAfter.exists()) {
            // Insert before the first export
            auto beforeConstantName = exports_.front().loc;
            auto [beforeExport, numWhitespace] = core::Loc(pkgFile, beforeConstantName).findStartOfIndentation(gs);
            auto endOfPrevLine = beforeExport.adjust(gs, -numWhitespace - 1, 0);
            insertionLoc = endOfPrevLine.copyWithZeroLength();
        } else {
            insertionLoc = core::Loc(pkgFile, exportToInsertAfter.copyEndWithZeroLength());
        }
    } else {
        // if we don't have any exports, then we can try adding it right before the final `end`
        uint32_t exportLoc = loc.endPos() - "end"sv.size() - 1;
        // we want to find the end of the last non-empty line, so
        // let's do something gross: walk backward until we find non-whitespace
        const auto &file_source = loc.file().data(gs).source();
        while (isspace(file_source[exportLoc])) {
            exportLoc--;
            // this shouldn't happen in a well-formatted
            // `__package.rb` file, but just to be safe
            if (exportLoc == 0) {
                return nullopt;
            }
        }
        insertionLoc = {loc.file(), exportLoc + 1, exportLoc + 1};
    }
    ENFORCE(insertionLoc.exists());

    auto strName = newExport.show(gs);
    core::AutocorrectSuggestion suggestion(
        fmt::format("Export `{}` in package `{}`", strName, mangledName_.owner.show(gs)),
        {{insertionLoc, fmt::format("\n  export {}", strName)}});
    return {suggestion};
}

optional<ImportType> PackageInfo::importsPackage(MangledName mangledName) const {
    if (!mangledName.exists()) {
        return nullopt;
    }

    auto imp = absl::c_find_if(importedPackageNames, [mangledName](auto &i) { return i.mangledName == mangledName; });
    if (imp == importedPackageNames.end()) {
        return nullopt;
    }

    return imp->type;
}

// Is it a layering violation to import otherPkg from this package?
bool PackageInfo::causesLayeringViolation(const PackageDB &packageDB, const PackageInfo &otherPkg) const {
    ENFORCE(exists());
    if (!otherPkg.layer().has_value()) {
        return false;
    }

    return causesLayeringViolation(packageDB, otherPkg.layer().value().first);
}

bool PackageInfo::causesLayeringViolation(const PackageDB &packageDB, core::NameRef otherPkgLayer) const {
    ENFORCE(exists());
    if (!layer().has_value()) {
        return false;
    }

    auto pkgLayer = layer().value().first;
    auto pkgLayerIndex = packageDB.layerIndex(pkgLayer);
    auto otherPkgLayerIndex = packageDB.layerIndex(otherPkgLayer);

    return pkgLayerIndex < otherPkgLayerIndex;
}

// What is the minimum strict dependencies level that this package's imports must have?
StrictDependenciesLevel PackageInfo::minimumStrictDependenciesLevel() const {
    if (!strictDependenciesLevel().has_value()) {
        return StrictDependenciesLevel::False;
    }

    switch (strictDependenciesLevel().value().first) {
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

void PackageInfo::trackPackageReference(const core::FileRef file, const core::packages::MangledName package,
                                        const PackageReferenceInfo packageReferenceInfo) {
    auto r = referencedPackages.find(package);
    if (r != referencedPackages.end()) {
        r->second.first.insert(file);
    } else {
        referencedPackages[package] = {{file}, packageReferenceInfo};
    }
}

void PackageInfo::untrackPackageReferencesFor(const core::FileRef file) {
    for (auto &[_, v] : referencedPackages) {
        auto files = v.first;
        auto it = files.find(file);
        if (it == files.end()) {
            continue;
        }
        files.erase(it);
    }
    erase_if(referencedPackages, [](const auto &x) { return x.second.first.empty(); });
}

bool PackageInfo::operator==(const PackageInfo &rhs) const {
    return mangledName() == rhs.mangledName();
}

string PackageInfo::show(const core::GlobalState &gs) const {
    return this->mangledName().owner.show(gs);
}

} // namespace sorbet::core::packages
