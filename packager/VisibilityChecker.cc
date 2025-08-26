#include "packager/VisibilityChecker.h"
#include "absl/algorithm/container.h"
#include "absl/strings/match.h"
#include "ast/treemap/treemap.h"
#include "common/concurrency/Parallel.h"
#include "core/Context.h"
#include "core/errors/packager.h"

using namespace std;

using namespace std::literals::string_view_literals;

namespace sorbet::packager {

namespace {

static core::SymbolRef getEnumClassForEnumValue(const core::GlobalState &gs, core::SymbolRef sym) {
    if (sym.isStaticField(gs) && sym.owner(gs).isClassOrModule()) {
        auto owner = sym.owner(gs);
        // There's a hidden class like `MyEnum::X$1` between `MyEnum::X` and `T::Enum` in the ancestor chain.
        if (owner.asClassOrModuleRef().data(gs)->superClass() == core::Symbols::T_Enum()) {
            return owner;
        }
    }

    return core::Symbols::noSymbol();
}

core::ClassOrModuleRef getScopeForPackage(const core::GlobalState &gs, absl::Span<const core::NameRef> parts,
                                          core::ClassOrModuleRef startingFrom) {
    auto result = startingFrom;
    ENFORCE(result.exists());
    for (auto it = parts.rbegin(); it != parts.rend(); it++) {
        auto nextScope = result.data(gs)->findMember(gs, *it);
        if (!nextScope.exists() || !nextScope.isClassOrModule()) {
            return core::Symbols::noClassOrModule();
        }

        result = nextScope.asClassOrModuleRef();
    }

    if (result == core::Symbols::root()) {
        return core::Symbols::noClassOrModule();
    }

    return result;
}

// For each __package.rb file, traverse the resolved tree and apply the visibility annotations to the symbols.
class PropagateVisibility final {
    core::packages::PackageInfo &package;

    // Blames which location (export) caused a symbol to first be marked exported.
    struct ExportBlame {
        core::SymbolRef exportedBy;
        core::LocOffsets firstExportedAt;
    };
    UnorderedMap<core::SymbolRef, ExportBlame> explicitlyExported;

    // In general, it's not good to compare arbitrary LocOffsets, because they might not be in the
    // same file and thus not comparable, thus they have no `operator<` on them.
    //
    // Since we know that we're only going to compare locs from within a file, we can define our own
    // function here.
    static constexpr auto COMPARE_EXPORT_LOCS = [](const core::LocOffsets &left, const core::LocOffsets &right) {
        if (left.beginPos() < right.beginPos()) {
            return true;
        } else if (left.beginPos() == right.beginPos()) {
            return left.endPos() < right.endPos();
        } else {
            return false;
        }
    };

    struct DuplicateExportError {
        core::SymbolRef duplicate;
        core::SymbolRef prefix;
        core::LocOffsets firstExportedAt;
    };
    // Collect duplicate export errors into a map and report at the end of this file.
    // - uses an ordered map, because we want to report them in a determinstic order
    // - unique by `export` loc, because we want to only report a given `export` line as bad once
    map<core::LocOffsets, DuplicateExportError, decltype(COMPARE_EXPORT_LOCS)> duplicateExports =
        decltype(duplicateExports)(COMPARE_EXPORT_LOCS);

    void checkDuplicateExport(core::MutableContext ctx, core::SymbolRef currentExportLineSym,
                              core::LocOffsets currentExportLineLoc, core::SymbolRef sym, bool alreadyExported) {
        // Only report duplicate errors if there's an entry in the explicitlyExported map. We don't add to the
        // explicitlyExported map when marking a parent namespace as exported (in exportParentNamespace).
        auto isExplicitlyExported = this->explicitlyExported.find(sym) != this->explicitlyExported.end();
        if (isExplicitlyExported) {
            // If we're not at the top, then the current export is more general.
            // Report the error on the line that previously exported this symbol.
            auto atTop = sym == currentExportLineSym;
            auto errLoc = atTop ? currentExportLineLoc : this->explicitlyExported[sym].firstExportedAt;

            if (alreadyExported && duplicateExports.find(errLoc) == duplicateExports.end()) {
                auto firstExportedAt = atTop ? this->explicitlyExported[sym].firstExportedAt : currentExportLineLoc;
                duplicateExports[errLoc] = DuplicateExportError{sym, currentExportLineSym, firstExportedAt};
            }

        } else if (currentExportLineLoc.exists() &&
                   // Don't treat singleton classes as explicitly exported, so they never show up in
                   // duplicate export errors.
                   (!sym.isClassOrModule() || !sym.asClassOrModuleRef().data(ctx)->isSingletonClass(ctx))) {
            this->explicitlyExported[sym] =
                ExportBlame{.exportedBy = currentExportLineSym, .firstExportedAt = currentExportLineLoc};
        }
    }

    void recursiveSetIsExported(core::MutableContext ctx, bool setExportedTo, core::SymbolRef currentExportLineSym,
                                core::LocOffsets currentExportLineLoc, core::SymbolRef sym) {
        // Stop recursing at package boundary
        if (this->package.mangledName() != sym.enclosingClass(ctx).data(ctx)->package) {
            return;
        }

        switch (sym.kind()) {
            case core::SymbolRef::Kind::ClassOrModule: {
                auto klassData = sym.asClassOrModuleRef().data(ctx);

                if (setExportedTo) {
                    checkDuplicateExport(ctx, currentExportLineSym, currentExportLineLoc, sym,
                                         klassData->flags.isExported);
                }

                klassData->flags.isExported = setExportedTo;

                // This `members` call does not have a stable order--we recover a determinstic order
                // by sorting duplicate errors at the end of PropagateVisibility
                for (const auto &[name, child] : klassData->members()) {
                    if (name == core::Names::attached()) {
                        // There is a cycle between a class and its singleton, and this avoids infinite recursion.
                        continue;
                    }

                    recursiveSetIsExported(ctx, setExportedTo, currentExportLineSym, currentExportLineLoc, child);
                }
                break;
            }

            case core::SymbolRef::Kind::FieldOrStaticField: {
                auto fieldData = sym.asFieldRef().data(ctx);
                if (!fieldData->flags.isStaticField) {
                    break;
                }

                if (setExportedTo) {
                    checkDuplicateExport(ctx, currentExportLineSym, currentExportLineLoc, sym,
                                         fieldData->flags.isExported);
                }

                fieldData->flags.isExported = setExportedTo;
                break;
            }

            case core::SymbolRef::Kind::TypeMember:
            case core::SymbolRef::Kind::Method:
            case core::SymbolRef::Kind::TypeParameter:
                break;
        }
    }

    void exportParentNamespace(core::GlobalState &gs, core::ClassOrModuleRef owner) {
        // Implicitly export parent namespace (symbol owner) until we hit the root of the package.
        // NOTE that we make an exception for namespaces that define behavior: these CANNOT get exported implicitly,
        // as that violates the private-by-default paradigm.
        while (owner.exists() && !owner.data(gs)->flags.isExported && !owner.data(gs)->flags.isBehaviorDefining &&
               this->package.mangledName() == owner.data(gs)->package) {
            owner.data(gs)->flags.isExported = true;
            owner = owner.data(gs)->owner;
        }
    }

    // TODO(jez) This function is annoying that it has to recurse up the owner chain. It would be
    // better if we didn't have to do this, but that would involve persisting the test and non-test
    // root symbols for a package onto the PackageInfo itself, which is tricky.
    //
    // This is very unsatisfying, because it looks a lot like us re-introducing FullyQualifiedName,
    // which was half of the point of moving Symbols into the package database in the first place.
    pair<core::ClassOrModuleRef, core::ClassOrModuleRef> getScopesForPackage(const core::GlobalState &gs) {
        vector<core::NameRef> parts;
        auto owner = package.mangledName().owner;
        while (owner != core::Symbols::root() && owner != core::Symbols::PackageSpecRegistry()) {
            auto ownerData = owner.data(gs);
            parts.emplace_back(ownerData->name);
            owner = ownerData->owner;
        }

        auto nonTestScope = getScopeForPackage(gs, parts, core::Symbols::root());
        auto testNamespace = core::Symbols::root().data(gs)->findMember(gs, core::packages::PackageDB::TEST_NAMESPACE);
        core::ClassOrModuleRef testScope;
        if (testNamespace.exists() && testNamespace.isClassOrModule()) {
            testScope = getScopeForPackage(gs, parts, testNamespace.asClassOrModuleRef());
        }

        return {nonTestScope, testScope};
    }

    void unsetAllExportedInPackage(core::MutableContext ctx) {
        auto [nonTestScope, testScope] = getScopesForPackage(ctx);

        auto setExportedTo = false;

        // loc is never used in `recursiveSetIsExported` if `setExportedTo` is false, so just say "none"
        auto currentExportLineLoc = core::LocOffsets::none();
        if (nonTestScope.exists()) {
            recursiveSetIsExported(ctx, setExportedTo, nonTestScope, currentExportLineLoc, nonTestScope);
        }
        if (testScope.exists()) {
            recursiveSetIsExported(ctx, setExportedTo, testScope, currentExportLineLoc, testScope);
        }

        // Shouldn't have been touched, because currentExportLineLoc was none, but let's just clear it to be safe.
        explicitlyExported.clear();
    }

    bool ignoreRBIExportEnforcement(core::MutableContext ctx, core::FileRef file) {
        const auto path = file.data(ctx).path();

        return absl::c_any_of(ctx.state.packageDB().skipRBIExportEnforcementDirs(),
                              [&](const string &dir) { return absl::StartsWith(path, dir); });
    }

    // Checks that the package that a symbol is defined in can be exported from the package we're currently checking.
    void checkExportPackage(core::MutableContext ctx, core::LocOffsets loc, core::SymbolRef sym) {
        ENFORCE(!sym.locs(ctx).empty()); // Can't be empty

        bool allRBI = absl::c_all_of(sym.locs(ctx), [&](const core::Loc &loc) {
            return loc.file().data(ctx).isRBI() && !ignoreRBIExportEnforcement(ctx, loc.file());
        });

        if (allRBI) {
            if (auto e = ctx.beginError(loc, core::errors::Packager::InvalidExport)) {
                e.setHeader("Cannot export `{}` because it is only defined in an RBI file", sym.show(ctx));
                e.addErrorLine(sym.loc(ctx), "Defined here");
            }
        }

        auto symPackage = sym.enclosingClass(ctx).data(ctx)->package;
        if (symPackage != this->package.mangledName()) {
            if (auto e = ctx.beginError(loc, core::errors::Packager::InvalidExport)) {
                e.setHeader("Cannot export `{}` because it is owned by another package", sym.show(ctx));
                e.addErrorLine(sym.loc(ctx), "Defined here");
            }
        }

        // If sym is an enum value, it can't be exported directly. Instead, its wrapping enum class must be exported.
        //
        // This was originally an implementation limitation, but is now an intentional choice. The considerations:
        //
        // - Each additional export was previously expensive in the rewriter-based package visibility checker.
        // - Nothing prevents `MyEnum.deserialize('x')` to simply hide visibility violations.
        // - It was hard for end users to know whether an enum had only exported some values intentionally.
        //   In practice people just exported the new values without thinking.
        //
        // See also how when we get a visibility violation for an enum value not being exported we export the entire
        // enum, not the specific enum value, to avoid conflict-inducing churn on `__package.rb` files.
        auto enumClass = getEnumClassForEnumValue(ctx.state, sym);
        if (enumClass.exists()) {
            if (auto e = ctx.beginError(loc, core::errors::Packager::InvalidExport)) {
                string enumClassName = enumClass.show(ctx);
                e.setHeader("Cannot export enum value `{}`. Instead, export the entire enum `{}`", sym.show(ctx),
                            enumClassName);
                e.addErrorLine(sym.loc(ctx), "Defined here");

                e.addAutocorrect(core::AutocorrectSuggestion{
                    fmt::format("Export `{}`", enumClassName),
                    {core::AutocorrectSuggestion::Edit{core::Loc{package.fullLoc().file(), loc},
                                                       fmt::format("export {}", enumClassName)}}});
            }
        }
    }

    PropagateVisibility(core::packages::PackageInfo &package) : package{package} {}

public:
    // Find uses of export and mark the symbols they mention as exported.
    void postTransformSend(core::MutableContext ctx, const ast::Send &send) {
        if (send.fun != core::Names::export_()) {
            return;
        }

        if (send.numPosArgs() != 1) {
            // an error will have been raised in the packager pass
            return;
        }

        auto lit = ast::cast_tree<ast::ConstantLit>(send.getPosArg(0));
        if (lit == nullptr) {
            // Already reported an error in packager.cc
            return;
        }

        // This is a syntactically valid export. It might export something that doesn't exist, but
        // that doesn't matter: the rest of the pipeline depends on being able to see the `export`
        // lines locations for the purposes of autocorrects, so let's at least record that there is
        // an export here.
        this->package.exports_.emplace_back(send.loc);

        if (lit->symbol() == core::Symbols::StubModule()) {
            // Don't attempt to export a symbol that doesn't exist. Resolver reported an error already.
            return;
        }

        string_view kind;
        auto sym = lit->symbol();
        switch (sym.kind()) {
            case core::SymbolRef::Kind::ClassOrModule: {
                checkExportPackage(ctx, send.loc, sym);
                auto setExportedTo = true;
                recursiveSetIsExported(ctx, setExportedTo, sym, send.loc, sym);

                // When exporting a symbol, we also export its parent namespace. This is a bit of a hack, and it would
                // be great to remove this, but this was the behavior of the previous packager implementation.
                exportParentNamespace(ctx, sym.asClassOrModuleRef().data(ctx)->owner);
                return;
            }

            case core::SymbolRef::Kind::FieldOrStaticField: {
                checkExportPackage(ctx, send.loc, sym);
                auto setExportedTo = true;
                recursiveSetIsExported(ctx, setExportedTo, sym, send.loc, sym);

                // When exporting a field, we also export its parent namespace. This is a bit of a hack, and it would be
                // great to remove this, but this was the behavior of the previous packager implementation.
                exportParentNamespace(ctx, sym.asFieldRef().data(ctx)->owner);
                return;
            }

            case core::SymbolRef::Kind::Method: {
                kind = "method"sv;
                break;
            }
            case core::SymbolRef::Kind::TypeParameter: {
                kind = "type argument"sv;
                break;
            }
            case core::SymbolRef::Kind::TypeMember: {
                kind = "type member"sv;
                break;
            }
        }

        if (auto e = ctx.beginError(send.loc, core::errors::Packager::InvalidExport)) {
            e.setHeader("Only classes, modules, or constants may be exported");
            e.addErrorLine(sym.loc(ctx), "Defined here");
            e.addErrorNote("`{}` is a `{}`", sym.show(ctx), kind);
        }
    }

    static void run(core::GlobalState &gs, ast::ParsedFile &f) {
        if (!f.file.data(gs).isPackage(gs)) {
            return;
        }

        auto pkgName = gs.packageDB().getPackageNameForFile(f.file);
        if (!pkgName.exists()) {
            return;
        }

        auto package = gs.packageDB().getPackageInfoNonConst(pkgName);
        ENFORCE(package->exists(), "Package is associated with a file, but doesn't exist");

        core::MutableContext ctx{gs, core::Symbols::root(), f.file};
        PropagateVisibility pass{*package};
        pass.unsetAllExportedInPackage(ctx);
        ast::ConstTreeWalk::apply(ctx, pass, f.tree);

        auto exportAll = package->locs.exportAll;
        if (exportAll.exists() && !package->exports_.empty()) {
            if (auto e = ctx.beginError(exportAll, core::errors::Packager::ExportConflict)) {
                e.setHeader("Package `{}` declares `{}` and therefore should not use explicit exports",
                            package->mangledName().owner.show(ctx), "export_all!");

                auto edits = vector<core::AutocorrectSuggestion::Edit>{};
                for (const auto &export_ : package->exports_) {
                    auto replaceLoc = ctx.locAt(export_.loc);
                    auto [indentedStart, numSpaces] = replaceLoc.findStartOfIndentation(ctx);
                    // Remove leading whitespace
                    replaceLoc = replaceLoc.adjust(ctx, -1 * numSpaces, 0);
                    if (replaceLoc.beginPos() != 0) {
                        // Remove leading newline
                        replaceLoc = replaceLoc.adjust(ctx, -1, 0);
                    }
                    edits.emplace_back(core::AutocorrectSuggestion::Edit{replaceLoc, ""});
                }
                e.addAutocorrect({"Delete every export", edits});
            }
        }

        for (const auto [errLoc, err] : pass.duplicateExports) {
            if (auto e = ctx.beginError(errLoc, core::errors::Packager::ExportConflict)) {
                if (err.duplicate == err.prefix) {
                    e.setHeader("Duplicate export of `{}`", err.duplicate.show(ctx));
                    e.addErrorLine(ctx.locAt(err.firstExportedAt), "Previously exported here");
                } else {
                    e.setHeader("Cannot export `{}` because another exported name `{}` is a prefix of it",
                                err.duplicate.show(ctx), err.prefix.show(ctx));
                    e.addErrorLine(ctx.locAt(err.firstExportedAt), "Prefix exported here");
                }
            }
        }
    }
};

enum class FileType {
    ProdFile,
    TestHelperFile,
    TestUnitFile,
};

const FileType fileTypeFromCtx(const core::Context ctx) {
    if (ctx.file.data(ctx).isPackagedTestHelper()) {
        return FileType::TestHelperFile;
    } else if (ctx.file.data(ctx).isPackagedTest()) {
        return FileType::TestUnitFile;
    } else {
        return FileType::ProdFile;
    }
}

class VisibilityCheckerPass final {
    void addExportInfo(core::Context ctx, core::ErrorBuilder &e, core::SymbolRef litSymbol, bool definesBehavior) {
        auto definedHereLoc = litSymbol.loc(ctx);
        if (definesBehavior) {
            e.addErrorLine(definedHereLoc, "Defined here");
        } else {
            e.addErrorSection(core::ErrorSection(
                core::ErrorColors::format("`{}` does not define behavior and thus will not be automatically exported",
                                          litSymbol.show(ctx)),
                {core::ErrorLine(definedHereLoc, "")}));
            e.addErrorNote("Either export it manually, or better, "
                           "restructure the code so that package namespaces do not define behavior.");
        }
    }

    void addImportExportAutocorrect(core::Context ctx, core::ErrorBuilder &e,
                                    optional<core::AutocorrectSuggestion> &&importAutocorrect,
                                    optional<core::AutocorrectSuggestion> &&exportAutocorrect) {
        auto &db = ctx.state.packageDB();
        auto hasAutocorrect = importAutocorrect.has_value() || exportAutocorrect.has_value();

        if (importAutocorrect.has_value() && exportAutocorrect.has_value()) {
            auto combinedTitle = fmt::format("{} and {}", importAutocorrect->title, exportAutocorrect->title);
            importAutocorrect->edits.insert(importAutocorrect->edits.end(),
                                            make_move_iterator(exportAutocorrect->edits.begin()),
                                            make_move_iterator(exportAutocorrect->edits.end()));
            e.addAutocorrect(core::AutocorrectSuggestion{combinedTitle, move(importAutocorrect->edits)});
        } else if (importAutocorrect.has_value()) {
            e.addAutocorrect(std::move(importAutocorrect.value()));
        } else if (exportAutocorrect.has_value()) {
            e.addAutocorrect(std::move(exportAutocorrect.value()));
        }

        if (hasAutocorrect && !db.errorHint().empty()) {
            e.addErrorNote("{}", db.errorHint());
        }
    }

public:
    const core::packages::PackageInfo &package;
    const FileType fileType;
    UnorderedMap<core::packages::MangledName, core::packages::PackageReferenceInfo> packageReferences;

    // We only want to validate visibility for usages of constants, not definitions.
    // postTransformConstantLit does not discriminate, so we have to remember whether a given
    // ConstantLit was a definition.
    UnorderedSet<void *> constantAssignmentDefinitions;

    VisibilityCheckerPass(core::Context ctx, const core::packages::PackageInfo &package)
        : package{package}, fileType{fileTypeFromCtx(ctx)} {}

    bool isAnyTestFile() const {
        return fileType != FileType::ProdFile;
    }

    void preTransformAssign(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);
        auto lhs = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (lhs != nullptr) {
            constantAssignmentDefinitions.insert(lhs.get());
        }
    }

    void postTransformAssign(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);
        auto lhs = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (lhs != nullptr) {
            constantAssignmentDefinitions.erase(lhs.get());
        }
    }

    void postTransformConstantLit(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &lit = ast::cast_tree_nonnull<ast::ConstantLit>(tree);
        if (constantAssignmentDefinitions.contains(tree.get())) {
            return;
        }

        auto litSymbol = lit.symbol();
        if (!litSymbol.isClassOrModule() && !litSymbol.isFieldOrStaticField()) {
            return;
        }

        auto loc = litSymbol.loc(ctx);

        auto otherFile = loc.file();
        if (!otherFile.exists()) {
            return;
        }

        // If the imported symbol comes from the test namespace, we must also be in the test namespace.
        if ((otherFile.data(ctx).isPackagedTestHelper() || otherFile.data(ctx).isPackagedTest()) &&
            !this->isAnyTestFile()) {
            if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::UsedTestOnlyName)) {
                e.setHeader("`{}` is defined in a test namespace and cannot be referenced in a non-test file",
                            litSymbol.show(ctx));
            }
            return;
        }

        auto &db = ctx.state.packageDB();

        // no need to check visibility for these cases
        auto otherPackage = litSymbol.enclosingClass(ctx).data(ctx)->package;
        if (!otherPackage.exists() || this->package.mangledName() == otherPackage) {
            return;
        }
        auto &pkg = ctx.state.packageDB().getPackageInfo(otherPackage);

        bool isExported = pkg.locs.exportAll.exists();
        if (litSymbol.isClassOrModule()) {
            isExported = isExported || litSymbol.asClassOrModuleRef().data(ctx)->flags.isExported;
        } else if (litSymbol.isFieldOrStaticField()) {
            isExported = isExported || litSymbol.asFieldRef().data(ctx)->flags.isExported;
        }
        isExported = isExported || db.allowRelaxedPackagerChecksFor(this->package.mangledName());
        bool definesBehavior =
            !litSymbol.isClassOrModule() || litSymbol.asClassOrModuleRef().data(ctx)->flags.isBehaviorDefining;
        auto currentImportType = this->package.importsPackage(otherPackage);
        auto wasImported = currentImportType.has_value();

        // Is this a test import (whether test helper or not) used in a production context?
        auto testImportInProd = wasImported && currentImportType.value() != core::packages::ImportType::Normal &&
                                this->fileType == FileType::ProdFile;
        // Is this a test import not intended for use in helpers?
        auto testUnitImportInHelper = wasImported &&
                                      currentImportType.value() == core::packages::ImportType::TestUnit &&
                                      this->fileType != FileType::TestUnitFile;
        bool importNeeded = !wasImported || testImportInProd || testUnitImportInHelper;
        packageReferences[otherPackage] = {importNeeded, false};

        if (importNeeded || !isExported) {
            bool isTestImport = otherFile.data(ctx).isPackagedTestHelper() || this->fileType != FileType::ProdFile;
            core::packages::ImportType autocorrectedImportType = core::packages::ImportType::Normal;
            if (isTestImport) {
                if (this->fileType == FileType::TestHelperFile) {
                    autocorrectedImportType = core::packages::ImportType::TestHelper;
                } else {
                    autocorrectedImportType = core::packages::ImportType::TestUnit;
                }
            }
            auto strictDepsLevel = this->package.strictDependenciesLevel;
            auto importStrictDepsLevel = pkg.strictDependenciesLevel;
            bool layeringViolation = false;
            bool strictDependenciesTooLow = false;
            bool causesCycle = false;
            optional<string> path;
            if (!isTestImport && db.enforceLayering()) {
                layeringViolation = strictDepsLevel > core::packages::StrictDependenciesLevel::False &&
                                    this->package.causesLayeringViolation(db, pkg);
                strictDependenciesTooLow = importStrictDepsLevel != core::packages::StrictDependenciesLevel::None &&
                                           importStrictDepsLevel < this->package.minimumStrictDependenciesLevel();
                // If there's a path from the imported packaged to this package, then adding the import will close
                // the loop and cause a cycle.
                path = pkg.pathTo(ctx, this->package.mangledName());
                causesCycle =
                    strictDepsLevel >= core::packages::StrictDependenciesLevel::LayeredDag && path.has_value();
            }
            if (!causesCycle && !layeringViolation && !strictDependenciesTooLow) {
                if (db.genPackages()) {
                    // TODO(neil): this is technically incorrect since it means export errors won't be reported at all
                    // until we implement export handling in genPackages mode
                    return;
                }

                std::optional<core::AutocorrectSuggestion> importAutocorrect;
                if (importNeeded) {
                    if (auto exp = this->package.addImport(ctx, pkg, autocorrectedImportType)) {
                        importAutocorrect.emplace(exp.value());
                    }
                }
                std::optional<core::AutocorrectSuggestion> exportAutocorrect;
                if (!isExported) {
                    auto symToExport = litSymbol;
                    auto enumClass = getEnumClassForEnumValue(ctx.state, symToExport);
                    if (enumClass.exists()) {
                        symToExport = enumClass;
                    }
                    if (definesBehavior) {
                        // For compatibility with gen-packages, we do _not_ add an export if it doesn't define
                        // behavior. This is mostly because it's easier to get Sorbet to behave like gen-packages
                        // than the other way around.
                        //
                        // If we move to a world where all __package.rb edits are done via Sorbet autocorrects,
                        // we could make this addExport call unconditional.
                        if (auto exp = pkg.addExport(ctx, symToExport)) {
                            exportAutocorrect.emplace(exp.value());
                        }
                    }
                }

                if (!isExported && !wasImported) {
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::MissingImport)) {
                        e.setHeader("`{}` resolves but is not exported from `{}` and `{}` is not imported",
                                    litSymbol.show(ctx), pkg.show(ctx), pkg.show(ctx));
                        addExportInfo(ctx, e, litSymbol, definesBehavior);
                        addImportExportAutocorrect(ctx, e, move(importAutocorrect), move(exportAutocorrect));
                    }
                } else if (!isExported && testImportInProd) {
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::UsedTestOnlyName)) {
                        e.setHeader("`{}` resolves but is not exported from `{}` and `{}` is `{}`ed",
                                    litSymbol.show(ctx), pkg.show(ctx), pkg.show(ctx), "test_import");
                        addExportInfo(ctx, e, litSymbol, definesBehavior);
                        addImportExportAutocorrect(ctx, e, move(importAutocorrect), move(exportAutocorrect));
                    }
                } else if (!isExported && testUnitImportInHelper) {
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::UsedTestOnlyName)) {
                        e.setHeader("`{}` resolves but is not exported from `{}` and `{}` is `{}`ed for only {} files",
                                    litSymbol.show(ctx), pkg.show(ctx), pkg.show(ctx), "test_import", ".test.rb");
                        e.addErrorNote("This is because this `{}` is declared with `{}`, which means the constant can "
                                       "only be used in `{}` files.",
                                       "test_import", "only: 'test_rb'", ".test.rb");
                        addExportInfo(ctx, e, litSymbol, definesBehavior);
                        addImportExportAutocorrect(ctx, e, move(importAutocorrect), move(exportAutocorrect));
                    }
                } else if (!isExported) {
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::UsedPackagePrivateName)) {
                        e.setHeader("`{}` resolves but is not exported from `{}`", litSymbol.show(ctx), pkg.show(ctx));
                        addExportInfo(ctx, e, litSymbol, definesBehavior);

                        addImportExportAutocorrect(ctx, e, move(importAutocorrect), move(exportAutocorrect));
                    }
                } else if (!wasImported) {
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::MissingImport)) {
                        e.setHeader("`{}` resolves but its package is not imported", lit.symbol().show(ctx));
                        e.addErrorLine(pkg.declLoc(), "Exported from package here");
                        addImportExportAutocorrect(ctx, e, move(importAutocorrect), move(exportAutocorrect));
                    }
                } else if (testImportInProd) {
                    ENFORCE(!isTestImport);
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::UsedTestOnlyName)) {
                        e.setHeader("Used `{}` constant `{}` in non-test file", "test_import", litSymbol.show(ctx));
                        e.addErrorLine(pkg.declLoc(), "Defined here");
                        addImportExportAutocorrect(ctx, e, move(importAutocorrect), move(exportAutocorrect));
                    }
                } else if (testUnitImportInHelper) {
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::UsedTestOnlyName)) {
                        e.setHeader("The `{}` constant `{}` can only be used in `{}` files", "test_import",
                                    litSymbol.show(ctx), ".test.rb");
                        e.addErrorLine(pkg.declLoc(), "Defined here");
                        e.addErrorNote("This is because this `{}` is declared with `{}`, which means the constant can "
                                       "only be used in `{}` files.",
                                       "test_import", "only: 'test_rb'", ".test.rb");
                        addImportExportAutocorrect(ctx, e, move(importAutocorrect), move(exportAutocorrect));
                    }
                } else {
                    ENFORCE(false);
                }
            } else {
                packageReferences[otherPackage].causesModularityError = true;
                // TODO(neil): Provide actionable advice and/or link to a doc that would help the user resolve these
                // layering/strict_dependencies issues.
                core::ErrorClass error =
                    causesCycle ? core::errors::Packager::StrictDependenciesViolation
                                : (layeringViolation ? core::errors::Packager::LayeringViolation
                                                     : core::errors::Packager::StrictDependenciesViolation);
                if (auto e = ctx.beginError(lit.loc(), error)) {
                    vector<string> reasons;
                    if (causesCycle) {
                        reasons.emplace_back(core::ErrorColors::format(
                            "importing its package would put `{}` into a cycle", this->package.show(ctx)));
                        auto currentStrictDepsLevel =
                            fmt::format("strict_dependencies '{}'",
                                        core::packages::strictDependenciesLevelToString(strictDepsLevel));
                        e.addErrorLine(core::Loc(this->package.file, this->package.locs.strictDependenciesLevel),
                                       "`{}` is `{}`, which disallows cycles", this->package.show(ctx),
                                       currentStrictDepsLevel);
                        ENFORCE(path.has_value(),
                                "Path from pkg to this->package should always exist if causesCycle is true");
                        e.addErrorNote("Path from `{}` to `{}`:\n{}", pkg.show(ctx), this->package.show(ctx),
                                       path.value());
                    }

                    if (layeringViolation) {
                        reasons.emplace_back("importing its package would cause a layering violation");
                        ENFORCE(pkg.layer.exists(), "causesLayeringViolation should return false if layer is not set");
                        ENFORCE(this->package.layer.exists(),
                                "causesLayeringViolation should return false if layer is not set");
                        e.addErrorLine(core::Loc(pkg.file, pkg.locs.layer),
                                       "Package `{}` must be at most layer `{}` (to match package `{}`) but is "
                                       "currently layer `{}`",
                                       pkg.show(ctx), this->package.layer.show(ctx), this->package.show(ctx),
                                       pkg.layer.show(ctx));
                    }

                    if (strictDependenciesTooLow) {
                        reasons.emplace_back(
                            core::ErrorColors::format("its `{}` is not strict enough", "strict_dependencies"));
                        ENFORCE(importStrictDepsLevel != core::packages::StrictDependenciesLevel::None,
                                "strictDependenciesTooLow should be false if strict_dependencies level is not set");
                        auto requiredStrictDepsLevel = fmt::format("strict_dependencies '{}'",
                                                                   core::packages::strictDependenciesLevelToString(
                                                                       this->package.minimumStrictDependenciesLevel()));
                        auto currentStrictDepsLevel =
                            fmt::format("strict_dependencies '{}'",
                                        core::packages::strictDependenciesLevelToString(importStrictDepsLevel));
                        e.addErrorLine(core::Loc(pkg.file, pkg.locs.strictDependenciesLevel),
                                       "`{}` must be at least `{}` but is currently `{}`", pkg.show(ctx),
                                       requiredStrictDepsLevel, currentStrictDepsLevel);
                    }

                    ENFORCE(!reasons.empty(), "At least one reason should be present");
                    string reason;
                    if (reasons.size() == 1) {
                        reason = reasons[0];
                    } else if (reasons.size() == 2) {
                        reason = fmt::format("{}, and {}", reasons[0], reasons[1]);
                    } else if (reasons.size() == 3) {
                        reason = fmt::format("{}, {}, and {}", reasons[0], reasons[1], reasons[2]);
                    } else {
                        ENFORCE(false, "At most three reasons should be present");
                    }
                    e.setHeader("`{}` cannot be referenced here because {}", lit.symbol().show(ctx), reason);
                    if (!isExported) {
                        e.addErrorNote("`{}` is not exported", lit.symbol().show(ctx));
                    } else if (!wasImported) {
                        e.addErrorNote("`{}`'s package is not imported", lit.symbol().show(ctx));
                    } else if (testImportInProd || testUnitImportInHelper) {
                        e.addErrorNote("`{}`'s package is imported as `{}`", lit.symbol().show(ctx), "test_import");
                    } else {
                        ENFORCE(false);
                    }
                }
            }
        }
    }

    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (original.kind == ast::ClassDef::Kind::Class && !original.ancestors.empty()) {
            auto &superClass = original.ancestors[0];
            ast::TreeWalk::apply(ctx, *this, superClass);
        }
    }

    static vector<ast::ParsedFile> run(core::GlobalState &nonConstGs, WorkerPool &workers,
                                       vector<ast::ParsedFile> files) {
        const core::GlobalState &gs = nonConstGs;
        auto resultq = std::make_shared<BlockingBoundedQueue<std::optional<std::pair<
            core::FileRef, UnorderedMap<core::packages::MangledName, core::packages::PackageReferenceInfo>>>>>(
            files.size());
        Timer timeit(gs.tracer(), "visibility_checker.check_visibility");
        Parallel::iterate(workers, "VisibilityChecker", absl::MakeSpan(files), [&gs, resultq](ast::ParsedFile &f) {
            if (!f.file.data(gs).isPackage(gs)) {
                auto pkgName = gs.packageDB().getPackageNameForFile(f.file);
                if (pkgName.exists()) {
                    core::Context ctx{gs, core::Symbols::root(), f.file};
                    VisibilityCheckerPass pass{ctx, gs.packageDB().getPackageInfo(pkgName)};
                    ast::TreeWalk::apply(ctx, pass, f.tree);
                    resultq->push(
                        std::optional<std::pair<core::FileRef, UnorderedMap<core::packages::MangledName,
                                                                            core::packages::PackageReferenceInfo>>>(
                            {f.file, std::move(pass.packageReferences)}),
                        1);
                } else {
                    resultq->push(std::nullopt, 1);
                }
            } else {
                resultq->push(std::nullopt, 1);
            }
        });

        // TODO: Parallel::iterate uses multiplexJobWait, so this loop won't start consuming from resultq until every
        // file is visited, but that's not necessary, we can start consuming them before that. Make the above use
        // multiplexJob.
        std::optional<
            std::pair<core::FileRef, UnorderedMap<core::packages::MangledName, core::packages::PackageReferenceInfo>>>
            threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem() && threadResult.has_value()) {
                auto file = threadResult.value().first;
                auto pkgName = gs.packageDB().getPackageNameForFile(file);
                if (!pkgName.exists()) {
                    continue;
                }
                auto nonConstPackageInfo = nonConstGs.packageDB().getPackageInfoNonConst(pkgName);
                nonConstPackageInfo->untrackPackageReferencesFor(file);
                for (auto [p, packageReferenceInfo] : threadResult.value().second) {
                    nonConstPackageInfo->trackPackageReference(file, p, packageReferenceInfo);
                }
            }
        }

        return files;
    }
};
} // namespace

vector<ast::ParsedFile> VisibilityChecker::run(core::GlobalState &gs, WorkerPool &workers,
                                               vector<ast::ParsedFile> files) {
    Timer timeit(gs.tracer(), "visibility_checker.run");

    {
        Timer timeit(gs.tracer(), "visibility_checker.propagate_visibility");
        for (auto &f : files) {
            PropagateVisibility::run(gs, f);
        }
    }

    return VisibilityCheckerPass::run(gs, workers, std::move(files));
}

} // namespace sorbet::packager
