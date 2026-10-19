#include "packager/VisibilityChecker.h"
#include "absl/algorithm/container.h"
#include "absl/strings/match.h"
#include "absl/synchronization/blocking_counter.h"
#include "ast/treemap/treemap.h"
#include "common/concurrency/Parallel.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
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

// Which of a package's namespaces to recompute export bits for. A package that doesn't use `test!` packages defines
// its application and its `Test::` namespace in different strata in package-directed mode, and processing one must
// not change the other's symbols, which might belong to an earlier stratum.
//
// TODO(jez) Delete this once all packages use `test!` packages.
enum class Namespace {
    Application,
    Test,
};

class PropagateVisibility final {
    enum class Mode {
        SetExportBits,
        ReportErrors,
    };

    core::packages::PackageInfo &package;
    const Mode mode;

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
                              core::LocOffsets currentExportLineLoc, core::SymbolRef sym) {
        // Only report duplicate errors if there's an entry in the explicitlyExported map. We don't add to the
        // explicitlyExported map when marking a parent namespace as exported (in exportParentNamespace).
        auto isExplicitlyExported = this->explicitlyExported.find(sym) != this->explicitlyExported.end();
        if (isExplicitlyExported) {
            // If we're not at the top, then the current export is more general.
            // Report the error on the line that previously exported this symbol.
            auto atTop = sym == currentExportLineSym;
            auto errLoc = atTop ? currentExportLineLoc : this->explicitlyExported[sym].firstExportedAt;

            if (duplicateExports.find(errLoc) == duplicateExports.end()) {
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

                if (mode == Mode::ReportErrors) {
                    checkDuplicateExport(ctx, currentExportLineSym, currentExportLineLoc, sym);
                } else {
                    klassData->flags.isExported = setExportedTo;
                }

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

                if (mode == Mode::ReportErrors) {
                    checkDuplicateExport(ctx, currentExportLineSym, currentExportLineLoc, sym);
                } else {
                    fieldData->flags.isExported = setExportedTo;
                }
                break;
            }

            case core::SymbolRef::Kind::TypeMember:
            case core::SymbolRef::Kind::Method:
            case core::SymbolRef::Kind::TypeParameter:
                break;
        }
    }

    void recursiveCheckDuplicateExports(core::MutableContext ctx, bool setExportedTo,
                                        core::SymbolRef currentExportLineSym, core::LocOffsets currentExportLineLoc,
                                        core::SymbolRef sym) {
        // Stop recursing at package boundary
        if (this->package.mangledName() != sym.enclosingClass(ctx).data(ctx)->package) {
            return;
        }

        switch (sym.kind()) {
            case core::SymbolRef::Kind::ClassOrModule: {
                auto klassData = sym.asClassOrModuleRef().data(ctx);

                if (mode == Mode::ReportErrors) {
                    checkDuplicateExport(ctx, currentExportLineSym, currentExportLineLoc, sym);
                } else {
                    klassData->flags.isExported = setExportedTo;
                }

                // This `members` call does not have a stable order--we recover a determinstic order
                // by sorting duplicate errors at the end of PropagateVisibility
                for (const auto &[name, child] : klassData->members()) {
                    if (name == core::Names::attached()) {
                        // There is a cycle between a class and its singleton, and this avoids infinite recursion.
                        continue;
                    }

                    recursiveCheckDuplicateExports(ctx, setExportedTo, currentExportLineSym, currentExportLineLoc,
                                                   child);
                }
                break;
            }

            case core::SymbolRef::Kind::FieldOrStaticField: {
                auto fieldData = sym.asFieldRef().data(ctx);
                if (!fieldData->flags.isStaticField) {
                    break;
                }

                if (mode == Mode::ReportErrors) {
                    checkDuplicateExport(ctx, currentExportLineSym, currentExportLineLoc, sym);
                } else {
                    fieldData->flags.isExported = setExportedTo;
                }
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
        if (!this->package.usesTestPackages && testNamespace.exists() && testNamespace.isClassOrModule()) {
            testScope = getScopeForPackage(gs, parts, testNamespace.asClassOrModuleRef());
        }

        // TODO(trevor): we can remove the returned test scope after switching to test packages.
        return {nonTestScope, testScope};
    }

    void unsetAllExportedInPackage(core::MutableContext ctx, Namespace ns) {
        auto [nonTestScope, testScope] = getScopesForPackage(ctx);

        auto setExportedTo = false;

        // loc is never used in `recursiveSetIsExported` if `setExportedTo` is false, so just say "none"
        auto currentExportLineLoc = core::LocOffsets::none();
        if (ns == Namespace::Application && nonTestScope.exists()) {
            recursiveSetIsExported(ctx, setExportedTo, nonTestScope, currentExportLineLoc, nonTestScope);
        }
        if (ns == Namespace::Test && testScope.exists()) {
            recursiveSetIsExported(ctx, setExportedTo, testScope, currentExportLineLoc, testScope);
        }

        // Shouldn't have been touched, because currentExportLineLoc was none, but let's just clear it to be safe.
        explicitlyExported.clear();
    }

    bool ignoreRBIExportEnforcement(const core::GlobalState &gs, core::FileRef file) {
        const auto path = file.data(gs).path();

        return absl::c_any_of(gs.packageDB().skipRBIExportEnforcementDirs(),
                              [&](const string &dir) { return absl::StartsWith(path, dir); });
    }

    // Returns true if all the locs that define a symbol come from RBI files.
    bool onlyInRBI(const core::GlobalState &gs, core::SymbolRef sym) {
        return absl::c_all_of(sym.locs(gs), [&gs, this](const core::Loc &loc) {
            return loc.file().data(gs).isRBI() && !ignoreRBIExportEnforcement(gs, loc.file());
        });
    }

    // Checks that the package that a symbol is defined in can be exported from the package we're currently checking.
    void checkExportPackage(core::MutableContext ctx, core::LocOffsets loc, core::SymbolRef sym) {
        ENFORCE(!sym.locs(ctx).empty()); // Can't be empty

        bool shouldDeleteExport = false;
        if (onlyInRBI(ctx, sym)) {
            if (auto e = ctx.beginError(loc, core::errors::Packager::InvalidExport)) {
                e.setHeader("Cannot export `{}` because it is only defined in an RBI file", sym.show(ctx));
                e.addErrorLine(sym.loc(ctx), "Defined here");
            }
        }

        auto symPackage = sym.enclosingClass(ctx).data(ctx)->package;
        if (symPackage != this->package.mangledName()) {
            if (ctx.state.packageDB().genPackagesMode() == core::packages::GenPackagesMode::Disabled) {
                if (auto e = ctx.beginError(loc, core::errors::Packager::InvalidExport)) {
                    e.setHeader("Cannot export `{}` because it is owned by another package", sym.show(ctx));
                    e.addErrorLine(sym.loc(ctx), "Defined here");
                    // TODO(neil): should there be an autocorrect to delete this export here?
                }
            } else {
                shouldDeleteExport = true;
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
            if (ctx.state.packageDB().genPackagesMode() == core::packages::GenPackagesMode::Disabled) {
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
            } else {
                shouldDeleteExport = true;
            }
        }
        if (shouldDeleteExport) {
            ENFORCE(ctx.state.packageDB().genPackagesMode() != core::packages::GenPackagesMode::Disabled);
            this->package.exportsToDelete_.emplace_back(loc);
        }
    }

    vector<string> computeRecursiveExports(const core::GlobalState &gs, core::ClassOrModuleRef klass) {
        vector<core::ClassOrModuleRef> work{klass};
        vector<string> lines;

        while (!work.empty()) {
            auto sym = work.back();
            work.pop_back();

            for (auto [name, member] : sym.data(gs)->members()) {
                // We only export classes, modules, or fields.
                if (member.isClassOrModule()) {
                    auto klass = member.asClassOrModuleRef();

                    if (klass.data(gs)->package != this->package.mangledName()) {
                        continue;
                    }

                    if (klass.data(gs)->isSingletonClass(gs)) {
                        continue;
                    }

                    // If we encounter another namespace with no real declaration, continue exporting
                    // its members.
                    if (!klass.data(gs)->isDeclaredInPackage()) {
                        work.push_back(klass);
                        continue;
                    }
                } else if (member.isFieldOrStaticField()) {
                    auto memberPkg = member.enclosingClass(gs).data(gs)->package;
                    if (memberPkg != this->package.mangledName()) {
                        continue;
                    }
                } else {
                    continue;
                }

                // We can't export symbols that are only defined in RBI files
                if (onlyInRBI(gs, member)) {
                    continue;
                }

                lines.emplace_back(fmt::format("export {}", member.show(gs)));
            }
        }

        fast_sort(lines);

        return lines;
    }

    void applyExport(core::MutableContext ctx, core::SymbolRef sym, core::LocOffsets loc) {
        auto setExportedTo = true;
        if (mode == Mode::ReportErrors) {
            recursiveCheckDuplicateExports(ctx, setExportedTo, sym, loc, sym);
        } else {
            recursiveSetIsExported(ctx, setExportedTo, sym, loc, sym);
        }

        if (mode == Mode::SetExportBits) {
            // When exporting a symbol, we also export its parent namespace. This is a bit of a hack, and it would
            // be great to remove this, but this was the behavior of the previous packager implementation.
            exportParentNamespace(ctx, sym.owner(ctx).asClassOrModuleRef());
        }
    }

    PropagateVisibility(core::packages::PackageInfo &package, Mode mode) : package{package}, mode{mode} {}

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

        if (lit->symbol() == core::Symbols::StubModule()) {
            // Don't attempt to export a symbol that doesn't exist. Resolver reported an error already.
            return;
        }

        // Skip any `export` constants which only resolved through a class alias.
        // When exporting something, we shouldn't allow people to indirect or obscure their exports.
        auto scope = lit;
        while (scope->original() != nullptr) {
            scope = ast::cast_tree<ast::ConstantLit>(scope->original()->scope);
            if (scope == nullptr) {
                break;
            }
            auto prefix = scope->symbol();
            if (prefix.isClassAlias(ctx)) {
                if (ctx.state.packageDB().genPackagesMode() == core::packages::GenPackagesMode::Disabled) {
                    if (auto e = ctx.beginError(scope->loc(), core::errors::Packager::InvalidExport)) {
                        e.setHeader("Cannot export through class alias `{}`", prefix.show(ctx));
                        e.addErrorLine(prefix.loc(ctx), "Alias defined here");
                        e.addErrorNote("Export `{}` directly instead", lit->symbol().show(ctx));
                    }
                } else {
                    // The generator will add a direct export for the target if another package uses it.
                    this->package.exportsToDelete_.emplace_back(send.loc);
                }
                return;
            }
        }

        auto sym = lit->symbol();

        string_view kind;
        switch (sym.kind()) {
            case core::SymbolRef::Kind::ClassOrModule: {
                auto klass = sym.asClassOrModuleRef();
                auto klassData = klass.data(ctx);

                // The symbol being exported must have an actual declaration, being part of a path that's present for
                // other declarations isn't sufficient.
                if (!klassData->isDeclaredInPackage()) {
                    if (auto e = ctx.beginError(send.loc, core::errors::Packager::InvalidExport)) {
                        e.setHeader("Constant `{}` lacks a declaration in this package and cannot be exported",
                                    sym.show(ctx));

                        auto lines = computeRecursiveExports(ctx, klass);
                        e.replaceWith(lines.empty() ? "Remove this export" : "Export all child symbols",
                                      ctx.locAt(send.loc), "{}",
                                      fmt::map_join(lines, "\n  ", [](string_view line) { return line; }));
                    }
                }

                checkExportPackage(ctx, send.loc, sym);
                applyExport(ctx, sym, send.loc);
                return;
            }

            case core::SymbolRef::Kind::FieldOrStaticField: {
                checkExportPackage(ctx, send.loc, sym);
                applyExport(ctx, sym, send.loc);
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

    // A resolved `__package.rb` is only used to report errors in its exports.
    static void reportErrors(core::GlobalState &gs, const ast::ParsedFile &f) {
        ENFORCE(f.file.data(gs).isPackage(gs));

        auto pkgName = gs.packageDB().getPackageNameForFile(f.file);
        if (!pkgName.exists()) {
            return;
        }

        auto package = gs.packageDB().getPackageInfoNonConst(pkgName);
        ENFORCE(package->exists(), "Package is associated with a file, but doesn't exist");

        // TODO(jez) Get this to be a `core::Context` now.
        core::MutableContext ctx{gs, core::Symbols::root(), f.file};
        PropagateVisibility pass{*package, Mode::ReportErrors};
        ast::ConstTreeWalk::apply(ctx, pass, f.tree);

        auto exportAll = package->locs.exportAll;
        if (exportAll.exists() && !package->exports_.empty()) {
            if (auto e = ctx.beginError(exportAll, core::errors::Packager::ExportConflict)) {
                e.setHeader("Package `{}` declares `{}` and therefore should not use explicit exports",
                            package->mangledName().owner.show(ctx), "export_all!");

                auto edits = vector<core::AutocorrectSuggestion::Edit>{};
                for (const auto &exp : package->exports_) {
                    auto replaceLoc = ctx.locAt(exp.loc);
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

    // Recompute the export bits of the package's symbols from its `exports_`.
    //
    // This reports no errors: `reportErrors` reports them, from the package's resolved `__package.rb`
    // (which might not happen until the test SCC for this package runs).
    static void setExportBits(core::GlobalState &gs, core::packages::MangledName name, Namespace ns) {
        auto package = gs.packageDB().getPackageInfoNonConst(name);
        ENFORCE(package != nullptr);
        if (package->usesTestPackages) {
            // A `test!` package is a separate package, so each package has only one namespace.
            ns = Namespace::Application;
        }

        core::MutableContext ctx{gs, core::Symbols::root(), package->fullLoc().file()};
        // TODO(jez) This doesn't actually walk the tree anymore, but it still constructs
        // PropagateVisibility because that's where all the helpers that set export bits are.
        PropagateVisibility pass{*package, Mode::SetExportBits};

        pass.unsetAllExportedInPackage(ctx, ns);

        for (const auto &exp : package->exports_) {
            // Exports can't go through class aliases, so an export of a `Test::` constant only exports symbols in
            // the `Test::` namespace, and vice versa.
            auto isTestExport =
                !package->usesTestPackages && exp.reversedPath.back() == core::packages::PackageDB::TEST_NAMESPACE;
            if (isTestExport != (ns == Namespace::Test)) {
                continue;
            }

            // For the same reason, we don't need to dealias any prefix.
            core::SymbolRef sym = core::Symbols::root();
            for (auto it = exp.reversedPath.rbegin(); it != exp.reversedPath.rend() && sym.exists(); ++it) {
                sym = sym.isClassOrModule() ? sym.asClassOrModuleRef().data(gs)->findMemberNoDealias(*it)
                                            : core::Symbols::noSymbol();
            }

            if (sym.exists() && (sym.isClassOrModule() || sym.isStaticField(gs))) {
                pass.applyExport(ctx, sym, exp.loc);
            }
        }
    }
};

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

public:
    const core::packages::PackageInfo &package;
    UnorderedMap<core::packages::MangledName, core::packages::PackageReferenceInfo> referencedPackages;
    UnorderedSet<core::SymbolRef> referencedSymbols;

    // We only want to validate visibility for usages of constants, not definitions.
    // postTransformConstantLit does not discriminate, so we have to remember whether a given
    // ConstantLit was a definition.
    UnorderedSet<const void *> constantAssignmentDefinitions;

    VisibilityCheckerPass(core::Context ctx, const core::packages::PackageInfo &package) : package{package} {}

    void preTransformAssign(core::Context ctx, const ast::Assign &asgn) {
        auto lhs = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (lhs != nullptr) {
            constantAssignmentDefinitions.insert(lhs.get());
        }
    }

    void postTransformAssign(core::Context ctx, const ast::Assign &asgn) {
        auto lhs = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (lhs != nullptr) {
            constantAssignmentDefinitions.erase(lhs.get());
        }
    }

    void postTransformConstantLit(core::Context ctx, const ast::ConstantLit &lit) {
        if (constantAssignmentDefinitions.contains(&lit)) {
            return;
        }

        auto litSymbol = lit.symbol();
        if (!litSymbol.isClassOrModule() && !litSymbol.isFieldOrStaticField()) {
            return;
        }
        if (litSymbol == core::Symbols::todo()) {
            // The symbol will be `<todo sym>` when traversing to a ClassDef which omits a superclass.
            // TODO(jez) Should we change put the real superclass in the tree once GlobalPass resolves it?
            return;
        }

        // NOTE: this only tracks the information required for computing what symbols needed to be exported, and not for
        // find all references. For example, if the current symbol is A::B::C::D, then only A::B::C::D will be added to
        // symbolsReferenced, and not A, A::B, A::B::C.
        // TODO(neil): we should also track A, A::B, A::B::C, so that we can use this for find all references too.
        referencedSymbols.insert(litSymbol);

        auto &db = ctx.state.packageDB();

        // no need to check visibility for these cases
        auto otherPackage = litSymbol.enclosingClass(ctx).data(ctx)->package;
        if (!otherPackage.exists() || this->package.mangledName() == otherPackage) {
            return;
        }

        auto importError = this->package.checkReferenceAgainstImports(ctx, lit.loc(), otherPackage);
        referencedPackages[otherPackage] = importError.value_or(core::packages::PackageReferenceInfo{});
        if (importError.has_value()) {
            // An error was reported already
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
        if (this->package.usesTestPackages && !isExported) {
            auto *import = this->package.importsPackage(otherPackage);
            ENFORCE(import != nullptr, "If it wasn't imported, we should not be dealing with exports");
            isExported = isExported || import->usesInternals;
        }

        if (!isExported) {
            if (db.genPackagesMode() != core::packages::GenPackagesMode::Disabled) {
                return;
            }

            bool definesBehavior =
                !litSymbol.isClassOrModule() || litSymbol.asClassOrModuleRef().data(ctx)->flags.isBehaviorDefining;
            std::optional<core::AutocorrectSuggestion> exportAutocorrect;
            if (definesBehavior) {
                auto symToExport = litSymbol;
                auto enumClass = getEnumClassForEnumValue(ctx.state, symToExport);
                if (enumClass.exists()) {
                    symToExport = enumClass;
                }
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
            if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::UsedPackagePrivateName)) {
                e.setHeader("`{}` resolves but is not exported from `{}`", litSymbol.show(ctx), pkg.show(ctx));
                addExportInfo(ctx, e, litSymbol, definesBehavior);

                auto hasAutocorrect = exportAutocorrect.has_value();
                e.maybeAddAutocorrect(move(exportAutocorrect));
                if (hasAutocorrect && !db.errorHint().empty()) {
                    e.addErrorNote("{}", db.errorHint());
                }
            }
        }
    }

    void preTransformClassDef(core::Context ctx, const ast::ClassDef &original) {
        if (original.kind == ast::ClassDef::Kind::Class && !original.ancestors.empty()) {
            auto &superClass = original.ancestors[0];
            ast::ConstTreeWalk::apply(ctx, *this, superClass);
        }
    }

    static void run(core::GlobalState &nonConstGs, WorkerPool &workers, absl::Span<const ast::ParsedFile> filesSpan) {
        const core::GlobalState &gs = nonConstGs;
        core::packages::PackageDB &nonConstPackageDB = nonConstGs.packageDB();
        struct ThreadResult {
            core::FileRef file;
            UnorderedMap<core::packages::MangledName, core::packages::PackageReferenceInfo> referencedPackages;
            UnorderedSet<core::SymbolRef> referencedSymbols;
        };
        auto resultq = std::make_shared<BlockingBoundedQueue<std::optional<ThreadResult>>>(filesSpan.size());
        Timer timeit(gs.tracer(), "visibility_checker.check_visibility");
        auto taskq = std::make_shared<ConcurrentBoundedQueue<size_t>>(filesSpan.size());
        for (size_t i = 0; i < filesSpan.size(); ++i) {
            taskq->push(i, 1);
        }

        // N.B.: `workers.size()` can be `0` when threads are disabled, which would result in undefined behavior for
        // `BlockingCounter`.
        absl::BlockingCounter barrier(std::max(workers.size(), 1));
        workers.multiplexJob("VisibilityChecker", [taskq, &filesSpan, &gs, resultq, &barrier]() {
            size_t idx;
            for (auto result = taskq->try_pop(idx); !result.done(); result = taskq->try_pop(idx)) {
                if (!result.gotItem()) {
                    continue;
                }
                auto &f = filesSpan[idx];
                if (f.file.data(gs).isPackage(gs)) {
                    resultq->push(std::nullopt, 1);
                    continue;
                }
                auto pkgName = gs.packageDB().getPackageNameForFile(f.file);
                if (!pkgName.exists()) {
                    resultq->push(std::nullopt, 1);
                    continue;
                }
                core::Context ctx{gs, core::Symbols::root(), f.file};
                VisibilityCheckerPass pass{ctx, gs.packageDB().getPackageInfo(pkgName)};
                ast::ConstTreeWalk::apply(ctx, pass, f.tree);
                resultq->push(
                    ThreadResult{f.file, std::move(pass.referencedPackages), std::move(pass.referencedSymbols)}, 1);
            }
            barrier.DecrementCount();
        });

        std::optional<ThreadResult> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem() && threadResult.has_value()) {
                auto &file = threadResult->file;
                auto pkgName = gs.packageDB().getPackageNameForFile(file);
                if (!pkgName.exists()) {
                    continue;
                }

                auto nonConstPackageInfo = nonConstPackageDB.getPackageInfoNonConst(pkgName);
                vector<pair<core::packages::MangledName, core::packages::PackageReferenceInfo>> references;
                auto &referencedPackages = threadResult->referencedPackages;
                for (auto &[packageName, packageReferenceInfo] : referencedPackages) {
                    references.emplace_back(make_pair(packageName, packageReferenceInfo));
                }
                if (gs.packageDB().genPackagesMode() == core::packages::GenPackagesMode::Disabled) {
                    // Resolver has already recorded references that it rejected at package boundaries. Merge the
                    // successfully resolved references found here into the same per-file entry.
                    for (auto &[packageName, packageReferenceInfo] : references) {
                        nonConstPackageInfo->trackPackageReference(file, packageName, packageReferenceInfo);
                    }
                } else {
                    // Resolver package checking is disabled in gen-packages mode, so this pass owns the complete set.
                    nonConstPackageInfo->trackPackageReferences(file, references);
                }

                auto &referencedSymbols = threadResult->referencedSymbols;
                nonConstGs.setSymbolsReferencedByFile(file, referencedSymbols);
            }
        }
        barrier.Wait();
    }
};
} // namespace

void VisibilityChecker::propagateExports(core::GlobalState &gs, absl::Span<const ast::ParsedFile> files) {
    Timer timeit(gs.tracer(), "visibility_checker.propagate_visibility");
    // Recompute the bits of the namespaces whose files are being processed: these are the only symbols that this
    // stratum (or fast path) can have entered or changed.
    UnorderedSet<pair<core::packages::MangledName, Namespace>> recomputed;
    for (auto &f : files) {
        auto &file = f.file.data(gs);
        if (file.isPackage(gs)) {
            // Doesn't depend on export bits, only on the exports in the package's `__package.rb`.
            PropagateVisibility::reportErrors(gs, f);
        } else {
            auto pkgName = gs.packageDB().getPackageNameForFile(f.file);
            if (!pkgName.exists()) {
                continue;
            }

            // Track which packages we've already setExportBits for, because we only have to do this
            // once per unique package in the set of edited files. (And we can't just scan files
            // looking for `__package.rb` files, because the application stratum of a legacy
            // application+test package won't place the `__package.rb` file in that stratum, but
            // still needs to export the application constants.)
            auto ns = file.isPackagedTest() ? Namespace::Test : Namespace::Application;
            auto [_it, seenForFirstTime] = recomputed.emplace(pkgName, ns);
            if (seenForFirstTime) {
                PropagateVisibility::setExportBits(gs, pkgName, ns);
            }
        }
    }
}

void VisibilityChecker::checkReferences(core::GlobalState &gs, WorkerPool &workers,
                                        absl::Span<const ast::ParsedFile> files) {
    VisibilityCheckerPass::run(gs, workers, files);
}

} // namespace sorbet::packager
