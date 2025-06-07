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

// For each __package.rb file, traverse the resolved tree and apply the visibility annotations to the symbols.
class PropagateVisibility final {
    const core::packages::PackageInfo &package;

    bool definedByThisPackage(const core::GlobalState &gs, core::ClassOrModuleRef sym) {
        auto pkg = gs.packageDB().getPackageNameForFile(sym.data(gs)->loc().file());
        return this->package.mangledName() == pkg;
    }

    void recursiveExportSymbol(core::GlobalState &gs, bool firstSymbol, core::ClassOrModuleRef klass) {
        // We only mark symbols from this package. However, there's a
        // tough case where non-behavior-defining "namespace-like"
        // constants might get attributed to other packages (since we
        // don't have a canonical location, so we use the first place
        // we see them... which might be in a subpackage) and
        // therefore this might stop too soon. That's why we only stop
        // recursing if the thing is actually behavior-defining.
        if (!this->definedByThisPackage(gs, klass) && klass.data(gs)->flags.isBehaviorDefining) {
            return;
        }

        klass.data(gs)->flags.isExported = true;

        for (const auto &[name, child] : klass.data(gs)->members()) {
            if (name == core::Names::attached()) {
                // There is a cycle between a class and its singleton, and this avoids infinite recursion.
                continue;
            }
            if (child.isClassOrModule()) {
                recursiveExportSymbol(gs, false, child.asClassOrModuleRef());
            } else if (child.isFieldOrStaticField()) {
                child.asFieldRef().data(gs)->flags.isExported = true;
            }
        }
    }

    void exportParentNamespace(core::GlobalState &gs, core::ClassOrModuleRef owner) {
        // Implicitly export parent namespace (symbol owner) until we hit the root of the package.
        // NOTE that we make an exception for namespaces that define behavior: these CANNOT get exported implicitly,
        // as that violates the private-by-default paradigm.
        while (owner.exists() && !owner.data(gs)->flags.isExported && !owner.data(gs)->flags.isBehaviorDefining &&
               this->definedByThisPackage(gs, owner)) {
            owner.data(gs)->flags.isExported = true;
            owner = owner.data(gs)->owner;
        }
    }

    // Lookup the package name on the given root symbol, and mark the final symbol as exported.
    void exportRoot(core::GlobalState &gs, core::ClassOrModuleRef sym) {
        // For a package named `A::B`, the ClassDef that we see in this pass is for a symbol named
        // `<PackageSpecRegistry>::A::B`. In order to make the name `A::B` visible to packages that have imported
        // `A::B`, we explicitly lookup and export them here. This is a design decision inherited from the previous
        // packages implementation, and we could remove it after migrating Stripe's codebase to not depend on package
        // names being exported by default.
        for (auto name : this->package.fullName()) {
            auto next = sym.data(gs)->findMember(gs, name);
            if (!next.exists() || !next.isClassOrModule()) {
                sym = core::Symbols::noClassOrModule();
                return;
            }

            sym = next.asClassOrModuleRef();
        }

        sym.data(gs)->flags.isExported = true;
    }

    // Mark both `::A::B` and `::Test::A::B` as exported (given package `A::B`).
    // This will terminate the recursive exporting in `exportParentNamespace`, as it stops when it
    // hits either the root or an exported symbol.
    void exportPackageRoots(core::GlobalState &gs) {
        this->exportRoot(gs, core::Symbols::root());

        auto test = core::Symbols::root().data(gs)->findMember(gs, core::Names::Constants::Test());
        if (test.exists() && test.isClassOrModule()) {
            this->exportRoot(gs, test.asClassOrModuleRef());
        }
    }

    // While processing the ClassDef for the package, which will be named something like `<PackageSpecRegistry>::A::B`,
    // we also check that the symbols `A::B` and `Test::A::B` have locations whose package matches the one we're
    // processing. If they don't match, we add locs to ensure that those symbols are associated with this package.
    //
    // The reason for this step is that it's currently allowed to refer to the name of the package outside of the
    // context of the package spec, even if it doesn't explicitly export its top-level name. So in the case above, there
    // would be no `export A::B` line in the package spec. However, if there is a package that is defined in the child
    // namespace of `A::B`, and it at some point has a declaration of the form `module A::B`, the file that contains
    // that declaration will be marked as owning `A::B`, thus breaking the invariant that the file can be used to
    // determine the package that owns a symbol. So, to avoid this case we ensure that the symbols that correspond to
    // the package name are always owned by the package that defines them.
    void setPackageLocs(core::MutableContext ctx, core::LocOffsets loc, core::ClassOrModuleRef sym) {
        vector<core::NameRef> names;

        while (sym.exists() && sym != core::Symbols::PackageSpecRegistry()) {
            // The symbol isn't a package name if it's defined outside of the package registry.
            if (sym == core::Symbols::root()) {
                return;
            }

            names.emplace_back(sym.data(ctx)->name);
            sym = sym.data(ctx)->owner;
        }

        absl::c_reverse(names);

        auto &db = ctx.state.packageDB();

        {
            auto packageSym = core::Symbols::root();
            for (auto name : names) {
                auto member = packageSym.data(ctx)->findMember(ctx, name);
                if (!member.exists() || !member.isClassOrModule()) {
                    packageSym = core::Symbols::noClassOrModule();
                    break;
                }

                packageSym = member.asClassOrModuleRef();
            }

            if (packageSym.exists()) {
                auto file = packageSym.data(ctx)->loc().file();
                if (db.getPackageNameForFile(file) != this->package.mangledName()) {
                    packageSym.data(ctx)->addLoc(ctx, ctx.locAt(loc));
                }
            }
        }

        {
            auto testSym = core::Symbols::root();
            auto member = testSym.data(ctx)->findMember(ctx, core::Names::Constants::Test());
            if (!member.exists() || !member.isClassOrModule()) {
                return;
            }

            testSym = member.asClassOrModuleRef();
            for (auto name : names) {
                auto member = testSym.data(ctx)->findMember(ctx, name);
                if (!member.exists() || !member.isClassOrModule()) {
                    testSym = core::Symbols::noClassOrModule();
                    break;
                }

                testSym = member.asClassOrModuleRef();
            }

            if (testSym.exists()) {
                auto file = testSym.data(ctx)->loc().file();
                if (db.getPackageNameForFile(file) != this->package.mangledName()) {
                    testSym.data(ctx)->addLoc(ctx, ctx.locAt(loc));
                }
            }
        }
    }

    bool ignoreRBIExportEnforcement(core::MutableContext &ctx, core::FileRef file) {
        const auto path = file.data(ctx).path();

        return absl::c_any_of(ctx.state.packageDB().skipRBIExportEnforcementDirs(),
                              [&](const string &dir) { return absl::StartsWith(path, dir); });
    }

    // Checks that the package that a symbol is defined in can be exported from the package we're currently checking.
    void checkExportPackage(core::MutableContext &ctx, core::LocOffsets loc, core::SymbolRef sym) {
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

        auto definingFile = sym.loc(ctx).file();
        auto symPackage = ctx.state.packageDB().getPackageNameForFile(definingFile);
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

    PropagateVisibility(const core::packages::PackageInfo &package) : package{package} {}

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
        if (lit == nullptr || lit->symbol() == core::Symbols::StubModule()) {
            // We don't raise an explicit error here, as this is one of two cases:
            //   1. Export is given a non-constant argument
            //   2. The argument failed to resolve
            // In both cases, errors will be raised by previous passes.
            return;
        }

        auto litSymbol = lit->symbol();
        if (litSymbol.isClassOrModule()) {
            auto sym = litSymbol.asClassOrModuleRef();
            checkExportPackage(ctx, send.loc, litSymbol);
            recursiveExportSymbol(ctx, true, sym);

            // When exporting a symbol, we also export its parent namespace. This is a bit of a hack, and it would be
            // great to remove this, but this was the behavior of the previous packager implementation.
            exportParentNamespace(ctx, sym.data(ctx)->owner);
        } else if (litSymbol.isFieldOrStaticField()) {
            auto sym = litSymbol.asFieldRef();
            checkExportPackage(ctx, send.loc, litSymbol);
            sym.data(ctx)->flags.isExported = true;

            // When exporting a field, we also export its parent namespace. This is a bit of a hack, and it would be
            // great to remove this, but this was the behavior of the previous packager implementation.
            exportParentNamespace(ctx, sym.data(ctx)->owner);
        } else {
            string_view kind = ""sv;
            switch (litSymbol.kind()) {
                case core::SymbolRef::Kind::ClassOrModule:
                case core::SymbolRef::Kind::FieldOrStaticField:
                    ENFORCE(false, "ClassOrModule and FieldOrStaticField marked not exportable");
                    break;

                case core::SymbolRef::Kind::Method:
                    kind = "type argument"sv;
                    break;

                case core::SymbolRef::Kind::TypeArgument:
                    kind = "type argument"sv;
                    break;

                case core::SymbolRef::Kind::TypeMember:
                    kind = "type member"sv;
                    break;
            }

            if (auto e = ctx.beginError(send.loc, core::errors::Packager::InvalidExport)) {
                e.setHeader("Only classes, modules, or constants may be exported");
                e.addErrorLine(litSymbol.loc(ctx), "Defined here");
                e.addErrorNote("`{}` is a `{}`", litSymbol.show(ctx), kind);
            }
        }
    }

    void preTransformClassDef(core::MutableContext ctx, const ast::ClassDef &original) {
        if (original.symbol == core::Symbols::root()) {
            return;
        }

        ENFORCE(original.symbol != core::Symbols::todo());
        setPackageLocs(ctx, original.name.loc(), original.symbol);
    }

    static void run(core::GlobalState &gs, ast::ParsedFile &f) {
        if (!f.file.data(gs).isPackage(gs)) {
            return;
        }

        auto pkgName = gs.packageDB().getPackageNameForFile(f.file);
        if (!pkgName.exists()) {
            return;
        }

        const auto &package = gs.packageDB().getPackageInfo(pkgName);
        ENFORCE(package.exists(), "Package is associated with a file, but doesn't exist");

        PropagateVisibility pass{package};

        pass.exportPackageRoots(gs);

        core::MutableContext ctx{gs, core::Symbols::root(), f.file};
        ast::ConstTreeWalk::apply(ctx, pass, f.tree);

        // if we used `export_all`, then there were no `export`
        // directives in the previous pass; we should instead export
        // the package root
        if (package.exportAll()) {
            // we check if these exist because if no constants were
            // defined in the package then we might not have actually
            // ever created the relevant namespaces
            auto pkgRoot = package.getPackageScope(gs);
            if (pkgRoot.exists()) {
                pass.recursiveExportSymbol(gs, true, pkgRoot);
            }

            auto pkgTestRoot = package.getPackageTestScope(gs);
            if (pkgTestRoot.exists()) {
                pass.recursiveExportSymbol(gs, true, pkgTestRoot);
            }
        }
    }
};

class VisibilityCheckerPass final {
    void addPackagedFalseNote(core::Context ctx, core::ErrorBuilder &e) {
        if (!ctx.file.data(ctx).isPackaged()) {
            e.addErrorNote("A `{}` file is allowed to define constants outside of the package's "
                           "namespace,\n    "
                           "but must still respect its enclosing package's imports.",
                           "# packaged: false");
        }
    }

    void addExportInfo(core::Context ctx, core::ErrorBuilder &e, core::SymbolRef litSymbol) {
        auto definedHereLoc = litSymbol.loc(ctx);
        if (definedHereLoc.file().data(ctx).isRBI()) {
            e.addErrorSection(
                core::ErrorSection(core::ErrorColors::format("Consider marking this RBI file `{}` if it is meant to "
                                                             "declare unpackaged constants",
                                                             "# packaged: false"),
                                   {core::ErrorLine(definedHereLoc, "")}));
        } else {
            e.addErrorLine(definedHereLoc, "Defined here");
        }
    }

    core::AutocorrectSuggestion combineImportExportAutocorrect(core::AutocorrectSuggestion &importAutocorrect,
                                                               core::AutocorrectSuggestion &exportAutocorrect) {
        auto combinedTitle = fmt::format("{} and {}", importAutocorrect.title, exportAutocorrect.title);
        importAutocorrect.edits.insert(importAutocorrect.edits.end(), exportAutocorrect.edits.begin(),
                                       exportAutocorrect.edits.end());
        core::AutocorrectSuggestion combinedAutocorrect(combinedTitle, importAutocorrect.edits);
        return importAutocorrect;
    }

public:
    const core::packages::PackageInfo &package;
    const bool insideTestFile;

    VisibilityCheckerPass(core::Context ctx, const core::packages::PackageInfo &package)
        : package{package}, insideTestFile{ctx.file.data(ctx).isPackagedTest()} {}

    void postTransformConstantLit(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &lit = ast::cast_tree_nonnull<ast::ConstantLit>(tree);
        auto litSymbol = lit.symbol();
        if (!litSymbol.isClassOrModule() && !litSymbol.isFieldOrStaticField()) {
            return;
        }

        auto loc = litSymbol.loc(ctx);

        auto otherFile = loc.file();
        if (!otherFile.exists() || !otherFile.data(ctx).isPackaged()) {
            return;
        }

        // If the imported symbol comes from the test namespace, we must also be in the test namespace.
        if (otherFile.data(ctx).isPackagedTest() && !this->insideTestFile) {
            if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::UsedTestOnlyName)) {
                e.setHeader("`{}` is defined in a test namespace and cannot be referenced in a non-test file",
                            litSymbol.show(ctx));
            }
            return;
        }

        auto &db = ctx.state.packageDB();

        // no need to check visibility for these cases
        auto otherPackage = db.getPackageNameForFile(otherFile);
        if (!otherPackage.exists() || this->package.mangledName() == otherPackage) {
            return;
        }

        bool isExported = false;
        if (litSymbol.isClassOrModule()) {
            isExported = litSymbol.asClassOrModuleRef().data(ctx)->flags.isExported;
        } else if (litSymbol.isFieldOrStaticField()) {
            isExported = litSymbol.asFieldRef().data(ctx)->flags.isExported;
        }
        isExported = isExported || db.allowRelaxedPackagerChecksFor(this->package.mangledName());
        auto currentImportType = this->package.importsPackage(otherPackage);
        auto wasImported = currentImportType.has_value();
        auto importedAsTest =
            wasImported && currentImportType.value() == core::packages::ImportType::Test && !this->insideTestFile;
        if (!wasImported || importedAsTest || !isExported) {
            auto &pkg = ctx.state.packageDB().getPackageInfo(otherPackage);
            bool isTestImport = otherFile.data(ctx).isPackagedTest() || this->insideTestFile;
            auto strictDepsLevel = this->package.strictDependenciesLevel();
            auto importStrictDepsLevel = pkg.strictDependenciesLevel();
            bool layeringViolation = false;
            bool strictDependenciesTooLow = false;
            bool causesCycle = false;
            optional<string> path;
            if (!isTestImport && db.enforceLayering()) {
                layeringViolation = strictDepsLevel.has_value() &&
                                    strictDepsLevel.value().first != core::packages::StrictDependenciesLevel::False &&
                                    this->package.causesLayeringViolation(db, pkg);
                strictDependenciesTooLow =
                    importStrictDepsLevel.has_value() &&
                    importStrictDepsLevel.value().first < this->package.minimumStrictDependenciesLevel();
                // If there's a path from the imported packaged to this package, then adding the import will close
                // the loop and cause a cycle.
                path = pkg.pathTo(ctx, this->package.mangledName());
                causesCycle = strictDepsLevel.has_value() &&
                              strictDepsLevel.value().first >= core::packages::StrictDependenciesLevel::LayeredDag &&
                              path.has_value();
            }
            if (!causesCycle && !layeringViolation && !strictDependenciesTooLow) {
                std::optional<core::AutocorrectSuggestion> importAutocorrect;
                if (!wasImported || importedAsTest) {
                    if (auto exp = this->package.addImport(ctx, pkg, isTestImport)) {
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
                    if (auto exp = pkg.addExport(ctx, symToExport)) {
                        exportAutocorrect.emplace(exp.value());
                    }
                }

                if (!isExported && !wasImported) {
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::MissingImport)) {
                        e.setHeader("`{}` resolves but is not exported from `{}` and `{}` is not imported",
                                    litSymbol.show(ctx), pkg.show(ctx), pkg.show(ctx));
                        addExportInfo(ctx, e, litSymbol);
                        if (importAutocorrect.has_value() && exportAutocorrect.has_value()) {
                            core::AutocorrectSuggestion combinedAutocorrect =
                                combineImportExportAutocorrect(importAutocorrect.value(), exportAutocorrect.value());
                            e.addAutocorrect(std::move(combinedAutocorrect));
                            if (!db.errorHint().empty()) {
                                e.addErrorNote("{}", db.errorHint());
                            }
                        } else {
                            ENFORCE(false);
                        }
                        addPackagedFalseNote(ctx, e);
                    }
                } else if (!isExported && importedAsTest) {
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::UsedTestOnlyName)) {
                        e.setHeader("`{}` resolves but is not exported from `{}` and `{}` is `{}`ed",
                                    litSymbol.show(ctx), pkg.show(ctx), pkg.show(ctx), "test_import");
                        addExportInfo(ctx, e, litSymbol);
                        if (importAutocorrect.has_value() && exportAutocorrect.has_value()) {
                            core::AutocorrectSuggestion combinedAutocorrect =
                                combineImportExportAutocorrect(importAutocorrect.value(), exportAutocorrect.value());
                            e.addAutocorrect(std::move(combinedAutocorrect));
                            if (!db.errorHint().empty()) {
                                e.addErrorNote("{}", db.errorHint());
                            }
                        } else {
                            ENFORCE(false);
                        }
                    }
                } else if (!isExported) {
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::UsedPackagePrivateName)) {
                        e.setHeader("`{}` resolves but is not exported from `{}`", litSymbol.show(ctx), pkg.show(ctx));
                        addExportInfo(ctx, e, litSymbol);

                        if (exportAutocorrect.has_value()) {
                            e.addAutocorrect(std::move(exportAutocorrect.value()));
                            if (!db.errorHint().empty()) {
                                e.addErrorNote("{}", db.errorHint());
                            }
                        }
                    }
                } else if (!wasImported) {
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::MissingImport)) {
                        e.setHeader("`{}` resolves but its package is not imported", lit.symbol().show(ctx));
                        e.addErrorLine(pkg.declLoc(), "Exported from package here");
                        if (importAutocorrect.has_value()) {
                            e.addAutocorrect(std::move(importAutocorrect.value()));
                            if (!db.errorHint().empty()) {
                                e.addErrorNote("{}", db.errorHint());
                            }
                        }
                        addPackagedFalseNote(ctx, e);
                    }
                } else if (importedAsTest) {
                    ENFORCE(!isTestImport);
                    if (auto e = ctx.beginError(lit.loc(), core::errors::Packager::UsedTestOnlyName)) {
                        e.setHeader("Used `{}` constant `{}` in non-test file", "test_import", litSymbol.show(ctx));
                        e.addErrorLine(pkg.declLoc(), "Defined here");
                        if (importAutocorrect.has_value()) {
                            e.addAutocorrect(std::move(importAutocorrect.value()));
                            if (!db.errorHint().empty()) {
                                e.addErrorNote("{}", db.errorHint());
                            }
                        }
                    }
                } else {
                    ENFORCE(false);
                }
            } else {
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
                                        core::packages::strictDependenciesLevelToString(strictDepsLevel.value().first));
                        e.addErrorLine(core::Loc(this->package.declLoc().file(), strictDepsLevel.value().second),
                                       "`{}` is `{}`, which disallows cycles", this->package.show(ctx),
                                       currentStrictDepsLevel);
                        ENFORCE(path.has_value(),
                                "Path from pkg to this->package should always exist if causesCycle is true");
                        e.addErrorNote("Path from `{}` to `{}`:\n{}", pkg.show(ctx), this->package.show(ctx),
                                       path.value());
                    }

                    if (layeringViolation) {
                        reasons.emplace_back("importing its package would cause a layering violation");
                        ENFORCE(pkg.layer().has_value(),
                                "causesLayeringViolation should return false if layer is not set");
                        ENFORCE(this->package.layer().has_value(),
                                "causesLayeringViolation should return false if layer is not set");
                        e.addErrorLine(core::Loc(pkg.declLoc().file(), pkg.layer().value().second),
                                       "Package `{}` must be at most layer `{}` (to match package `{}`) but is "
                                       "currently layer `{}`",
                                       pkg.show(ctx), this->package.layer().value().first.show(ctx),
                                       this->package.show(ctx), pkg.layer().value().first.show(ctx));
                    }

                    if (strictDependenciesTooLow) {
                        reasons.emplace_back(
                            core::ErrorColors::format("its `{}` is not strict enough", "strict_dependencies"));
                        ENFORCE(importStrictDepsLevel.has_value(),
                                "strictDependenciesTooLow should be false if strict_dependencies level is not set");
                        auto requiredStrictDepsLevel = fmt::format("strict_dependencies '{}'",
                                                                   core::packages::strictDependenciesLevelToString(
                                                                       this->package.minimumStrictDependenciesLevel()));
                        auto currentStrictDepsLevel = fmt::format(
                            "strict_dependencies '{}'",
                            core::packages::strictDependenciesLevelToString(importStrictDepsLevel.value().first));
                        e.addErrorLine(core::Loc(pkg.declLoc().file(), importStrictDepsLevel.value().second),
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
                    } else if (importedAsTest) {
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

    static vector<ast::ParsedFile> run(const core::GlobalState &gs, WorkerPool &workers,
                                       vector<ast::ParsedFile> files) {
        Timer timeit(gs.tracer(), "visibility_checker.check_visibility");
        Parallel::iterate(workers, "VisibilityChecker", absl::MakeSpan(files), [&gs](ast::ParsedFile &f) {
            if (!f.file.data(gs).isPackage(gs)) {
                auto pkgName = gs.packageDB().getPackageNameForFile(f.file);
                if (pkgName.exists()) {
                    core::Context ctx{gs, core::Symbols::root(), f.file};
                    VisibilityCheckerPass pass{ctx, gs.packageDB().getPackageInfo(pkgName)};
                    ast::TreeWalk::apply(ctx, pass, f.tree);
                }
            }
        });

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
