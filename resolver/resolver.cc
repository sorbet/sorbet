#include "core/errors/resolver.h"
#include "ast/Helpers.h"
#include "ast/Trees.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/sort.h"
#include "core/Error.h"
#include "core/Names.h"
#include "core/StrictLevel.h"
#include "core/core.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "resolver/CorrectTypeAlias.h"
#include "resolver/SuggestPackage.h"
#include "resolver/resolver.h"
#include "resolver/type_syntax.h"

#include "absl/algorithm/container.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "common/Timer.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "core/Symbols.h"
#include <utility>
#include <vector>

using namespace std;

namespace sorbet::resolver {
namespace {

/*
 * Note: There are multiple separate tree walks defined in this file, the main
 * ones being:
 *
 * - ResolveConstantsWalk
 * - ResolveSignaturesWalk
 *
 * There are also other important parts of resolver found elsewhere in the
 * resolver/ package (GlobalPass, type_syntax). Below we describe
 * ResolveConstantsWalk, which is particularly sophisticated.
 *
 *                                - - - - -
 *
 * Ruby supports resolving constants via ancestors--superclasses and mixins.
 * Since superclass and mixins are themselves constant references, we thus may
 * not be able to resolve certain constants until after we've resolved others.
 *
 * To solve this, we collect any failed resolutions in a number of TODO lists,
 * and iterate over them to a fixed point (namely, either all constants
 * resolve, or no new constants resolve and we stub out any that remain).
 * In practice this fixed point computation terminates after 3 or fewer passes
 * on most real codebases.
 *
 * The four TODO lists that this loop maintains are:
 *
 *  - constants to be resolved
 *  - ancestors to be filled that require constants to be resolved
 *  - class aliases (class aliases know the symbol they alias to)
 *  - type aliases (type aliases know the fully parsed type of their RHS, and
 *    thus require their RHS to be resolved)
 *
 * Successful resolutions are removed from the lists, and then we loop again.
 * We track all these lists separately for the dual reasons that
 *
 * 1. Upon successful resolution, we need to do additional work (mutating the
 *    symbol table to reflect the new ancestors) and
 * 2. Resolving those constants potentially renders additional constants
 *    resolvable, and so if any resolution succeeds, we need to keep looping in
 *    the outer loop.
 *
 * We also track failure in the item, in order to distinguish a thing left in the
 * list because we simply haven't succeeded yet and a thing left in the list
 * because we have actively found a failure. For example, we might know that a
 * given constant is unresolvable by Sorbet because it was qualified under
 * not-a-constant: we mark this kind of job `resolution_failed`. The reason for
 * this is unresolved constants are set to noSymbol, and once constant resolution
 * has truly finished, we want to know which remaining failed jobs we need to set
 * to a sensible default value. (Setting a conceptually failed job to untyped()
 * before we've completed this loop can occasionally cause other jobs to
 * non-deterministically half-resolve in the presence of multiple errors---c.f.
 * issue #1126 on Github---so we mark jobs as failed rather than reporting an
 * error and "resolving" them as untyped during the loop.)
 *
 * After the above passes:
 *
 * - ast::UnresolvedConstantLit nodes (constants that have a NameRef) are
 *   replaced with ast::ConstantLit nodes (constants that have a SymbolRef).
 * - Every constant SymbolRef has enough to completely understand it's own
 *   place in the ancestor hierarchy.
 * - Every type alias symbol carries with it the type it should be treated as.
 *
 * The resolveConstants method is the best place to start if you want to browse
 * the fixed point loop at a high level.
 */

class ResolveConstantsWalk {
    friend class ResolveSanityCheckWalk;

private:
    struct Nesting {
        const shared_ptr<Nesting> parent;
        const core::SymbolRef scope;

        Nesting(shared_ptr<Nesting> parent, core::SymbolRef scope) : parent(std::move(parent)), scope(scope) {}
    };
    shared_ptr<Nesting> nesting_;

    struct ResolutionItem {
        shared_ptr<Nesting> scope;
        ast::ConstantLit *out;
        bool resolutionFailed = false;
        bool possibleGenericType = false;

        ResolutionItem() = default;
        ResolutionItem(const shared_ptr<Nesting> &scope, ast::ConstantLit *lit) : scope(scope), out(lit) {}
        ResolutionItem(ResolutionItem &&rhs) noexcept = default;
        ResolutionItem &operator=(ResolutionItem &&rhs) noexcept = default;

        ResolutionItem(const ResolutionItem &rhs) = delete;
        const ResolutionItem &operator=(const ResolutionItem &rhs) = delete;
    };

    template <class T> struct ResolveItems {
        core::FileRef file;
        vector<T> items;

        ResolveItems(core::FileRef file, vector<T> &&items) : file(file), items(move(items)){};
    };

    struct AncestorResolutionItem {
        ast::ConstantLit *ancestor;
        core::ClassOrModuleRef klass;

        bool isSuperclass; // true if superclass, false for mixin
        bool isInclude;    // true if include, false if extend
        std::optional<uint16_t> mixinIndex;

        AncestorResolutionItem() = default;
        AncestorResolutionItem(AncestorResolutionItem &&rhs) noexcept = default;
        AncestorResolutionItem &operator=(AncestorResolutionItem &&rhs) noexcept = default;

        AncestorResolutionItem(const AncestorResolutionItem &rhs) = delete;
        const AncestorResolutionItem &operator=(const AncestorResolutionItem &rhs) = delete;
    };

    struct ClassAliasResolutionItem {
        core::SymbolRef lhs;
        ast::ConstantLit *rhs;

        ClassAliasResolutionItem(core::SymbolRef lhs, ast::ConstantLit *rhs) : lhs(lhs), rhs(rhs) {}

        ClassAliasResolutionItem() = default;
        ClassAliasResolutionItem(ClassAliasResolutionItem &&) noexcept = default;
        ClassAliasResolutionItem &operator=(ClassAliasResolutionItem &&rhs) noexcept = default;

        ClassAliasResolutionItem(const ClassAliasResolutionItem &) = delete;
        const ClassAliasResolutionItem &operator=(const ClassAliasResolutionItem &) = delete;
    };

    struct TypeAliasResolutionItem {
        core::SymbolRef lhs;
        core::FileRef file;
        ast::ExpressionPtr *rhs;

        TypeAliasResolutionItem(core::SymbolRef lhs, core::FileRef file, ast::ExpressionPtr *rhs)
            : lhs(lhs), file(file), rhs(rhs) {}

        TypeAliasResolutionItem(TypeAliasResolutionItem &&) noexcept = default;
        TypeAliasResolutionItem &operator=(TypeAliasResolutionItem &&rhs) noexcept = default;

        TypeAliasResolutionItem(const TypeAliasResolutionItem &) = delete;
        const TypeAliasResolutionItem &operator=(const TypeAliasResolutionItem &) = delete;
    };

    struct ClassMethodsResolutionItem {
        core::FileRef file;
        core::SymbolRef owner;
        ast::Send *send;

        ClassMethodsResolutionItem(core::FileRef file, core::SymbolRef owner, ast::Send *send)
            : file(file), owner(owner), send(send) {}

        ClassMethodsResolutionItem(ClassMethodsResolutionItem &&) noexcept = default;
        ClassMethodsResolutionItem &operator=(ClassMethodsResolutionItem &&rhs) noexcept = default;

        ClassMethodsResolutionItem(const ClassMethodsResolutionItem &) = delete;
        const ClassMethodsResolutionItem &operator=(const ClassMethodsResolutionItem &) = delete;
    };

    struct RequireAncestorResolutionItem {
        core::FileRef file;
        core::ClassOrModuleRef owner;
        ast::Send *send;

        RequireAncestorResolutionItem(core::FileRef file, core::ClassOrModuleRef owner, ast::Send *send)
            : file(file), owner(owner), send(send) {}

        RequireAncestorResolutionItem(RequireAncestorResolutionItem &&) noexcept = default;
        RequireAncestorResolutionItem &operator=(RequireAncestorResolutionItem &&rhs) noexcept = default;

        RequireAncestorResolutionItem(const RequireAncestorResolutionItem &) = delete;
        const RequireAncestorResolutionItem &operator=(const RequireAncestorResolutionItem &) = delete;
    };

    vector<ResolutionItem> todo_;
    vector<AncestorResolutionItem> todoAncestors_;
    vector<ClassAliasResolutionItem> todoClassAliases_;
    vector<TypeAliasResolutionItem> todoTypeAliases_;
    vector<ClassMethodsResolutionItem> todoClassMethods_;
    vector<RequireAncestorResolutionItem> todoRequiredAncestors_;

    static core::SymbolRef resolveLhs(core::Context ctx, const shared_ptr<Nesting> &nesting, core::NameRef name) {
        Nesting *scope = nesting.get();
        while (scope != nullptr) {
            if (scope->scope.isClassOrModule()) {
                auto lookup = scope->scope.asClassOrModuleRef().data(ctx)->findMember(ctx, name);
                if (lookup.exists()) {
                    return lookup;
                }
            }
            scope = scope->parent.get();
        }
        return nesting->scope.asClassOrModuleRef().data(ctx)->findMemberTransitive(ctx, name);
    }

    static bool isAlreadyResolved(core::Context ctx, const ast::ConstantLit &original) {
        auto sym = original.symbol;
        if (!sym.exists()) {
            return false;
        }
        if (sym.isTypeAlias(ctx)) {
            return sym.asFieldRef().data(ctx)->resultType != nullptr;
        }
        return true;
    }

    class ResolutionChecker {
    public:
        bool seenUnresolved = false;

        ast::ExpressionPtr postTransformConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
            auto &original = ast::cast_tree_nonnull<ast::ConstantLit>(tree);
            seenUnresolved |= !isAlreadyResolved(ctx, original);
            return tree;
        };
    };

    static bool isFullyResolved(core::Context ctx, const ast::ExpressionPtr &expression) {
        ResolutionChecker checker;
        ast::ExpressionPtr dummy(expression.getTagged());
        dummy = ast::TreeMap::apply(ctx, checker, std::move(dummy));
        ENFORCE(dummy == expression);
        dummy.release();
        return !checker.seenUnresolved;
    }

    static core::SymbolRef resolveConstant(core::Context ctx, const shared_ptr<Nesting> &nesting,
                                           const ast::UnresolvedConstantLit &c, bool &resolutionFailed) {
        if (ast::isa_tree<ast::EmptyTree>(c.scope)) {
            core::SymbolRef result = resolveLhs(ctx, nesting, c.cnst);
            return result;
        }
        if (auto *id = ast::cast_tree<ast::ConstantLit>(c.scope)) {
            auto sym = id->symbol;
            if (sym.exists() && sym.isTypeAlias(ctx) && !resolutionFailed) {
                if (auto e = ctx.beginError(c.loc, core::errors::Resolver::ConstantInTypeAlias)) {
                    e.setHeader("Resolving constants through type aliases is not supported");
                }
                resolutionFailed = true;
                return core::Symbols::noSymbol();
            }
            if (!sym.exists()) {
                return core::Symbols::noSymbol();
            }
            core::SymbolRef resolved = id->symbol.dealias(ctx);
            core::SymbolRef result;
            if (resolved.isClassOrModule()) {
                result = resolved.asClassOrModuleRef().data(ctx)->findMember(ctx, c.cnst);
            }

            // Private constants are allowed to be resolved, when there is no scope set (the scope is checked above),
            // otherwise we should error out. Private constant references _are not_ enforced inside RBI files.
            if (result.exists() &&
                ((result.isClassOrModule() && result.asClassOrModuleRef().data(ctx)->flags.isPrivate) ||
                 (result.isStaticField(ctx) && result.asFieldRef().data(ctx)->flags.isStaticFieldPrivate)) &&
                !ctx.file.data(ctx).isRBI()) {
                if (auto e = ctx.beginError(c.loc, core::errors::Resolver::PrivateConstantReferenced)) {
                    e.setHeader("Non-private reference to private constant `{}` referenced", result.show(ctx));
                }
            }
            return result;
        }

        if (!resolutionFailed) {
            if (auto e = ctx.beginError(c.loc, core::errors::Resolver::DynamicConstant)) {
                e.setHeader("Dynamic constant references are unsupported");
            }
        }
        resolutionFailed = true;
        return core::Symbols::noSymbol();
    }

    static const int MAX_SUGGESTION_COUNT = 10;

    struct PackageStub {
        core::NameRef packageId;
        vector<core::NameRef> fullName;

        PackageStub(const core::packages::PackageInfo &info)
            : packageId{info.mangledName()}, fullName{info.fullName()} {}

        bool couldDefineChildNamespace(const core::GlobalState &gs, const std::vector<core::NameRef> &prefix,
                                       const std::vector<ast::ConstantLit *> &suffix) const {
            ENFORCE(!prefix.empty());

            auto start = prefix.begin();
            if (*start == core::Names::Constants::Test()) {
                ++start;
            }
            auto prefixSize = std::distance(start, prefix.end());
            if (prefixSize == 0) {
                return false;
            }

            // The reasoning is as follows: the prefix is derived from a nesting scope paired with the symbol being
            // resolved. The nesting scopes could only define a package in the parent namespace, while this check is
            // only applied to packages that are known to not occupy that part of the namespace.
            if (this->fullName.size() <= prefixSize) {
                return false;
            }

            if (!std::equal(start, prefix.end(), this->fullName.begin())) {
                return false;
            }

            auto it = this->fullName.begin() + prefixSize;
            for (auto *cnst : suffix) {
                if (it == this->fullName.end()) {
                    return true;
                }

                auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(cnst->original);
                if (original.cnst != *it) {
                    return false;
                }

                ++it;
            }

            return true;
        }
    };

    struct ParentPackageStub {
        PackageStub stub;
        vector<core::NameRef> exports;

        // NOTE: these are the public-facing exports of the package that start with the `Test::` special prefix.  They
        // are not the names exported to the implicit test package via `export_for_test`.
        vector<core::NameRef> testExports;

        ParentPackageStub(const core::packages::PackageInfo &info) : stub{info} {
            auto prefixLen = this->stub.fullName.size();
            auto testPrefixLen = prefixLen + 1;

            for (auto &path : info.exports()) {
                // We only need the unique part of the export's name. It's safe to assume that the exports are
                // populated, as the only case we allow a full re-export of the module is for leaf modules, and we
                // already know this not a leaf.
                if (path.front() == core::Names::Constants::Test()) {
                    this->testExports.emplace_back(path[testPrefixLen]);
                } else {
                    this->exports.emplace_back(path[prefixLen]);
                }
            }
        }

        // Check that the candidate name is one of the top-level exported names from a parent package.
        bool exportsSymbol(bool inTestNamespace, core::NameRef candidate) const {
            auto &exportList = inTestNamespace ? this->testExports : this->exports;
            return absl::c_find(exportList, candidate) != exportList.end();
        }
    };

    struct ImportStubs {
        std::vector<ParentPackageStub> parents;
        std::vector<PackageStub> imports;

        static ImportStubs make(core::GlobalState &gs) {
            ImportStubs stubs;

            auto &db = gs.packageDB();

            for (auto parent : gs.singlePackageImports->parentImports) {
                auto &info = db.getPackageInfo(parent);
                stubs.parents.emplace_back(ParentPackageStub{info});
            }

            for (auto parent : gs.singlePackageImports->regularImports) {
                auto &info = db.getPackageInfo(parent);
                stubs.imports.emplace_back(PackageStub{info});
            }

            return stubs;
        }

        // Determine if a package with the same name as `scope` is known to export the name `cnst`.
        bool packageExportsConstant(const std::vector<core::NameRef> &scope, core::NameRef cnst) const {
            ENFORCE(!scope.empty());

            auto start = scope.begin();
            if (*start == core::Names::Constants::Test()) {
                ++start;
            }

            auto scopeSize = std::distance(start, scope.end());
            ENFORCE(scopeSize > 0);

            const auto parent = absl::c_find_if(this->parents, [start, scopeSize, &scope](auto &p) {
                return scopeSize == p.stub.fullName.size() && std::equal(start, scope.end(), p.stub.fullName.begin());
            });

            bool inTestNamespace = start != scope.begin();
            return parent != this->parents.end() && parent->exportsSymbol(inTestNamespace, cnst);
        }

        // Determine if a package is known to have a prefix that is a combination of the name defined by `scope`, and
        // some prefix of the suffix vector provided.
        bool fromChildNamespace(const core::GlobalState &gs, const std::vector<core::NameRef> &prefix,
                                const std::vector<ast::ConstantLit *> &suffix) const {
            return absl::c_any_of(this->imports, [&gs, &prefix, &suffix](auto &stub) {
                return stub.couldDefineChildNamespace(gs, prefix, suffix);
            });
        }
    };

    static void ensureTGenericMixin(core::GlobalState &gs, core::ClassOrModuleRef klass) {
        auto &mixins = klass.data(gs)->mixins();
        if (absl::c_find(mixins, core::Symbols::T_Generic()) == mixins.end()) {
            mixins.emplace_back(core::Symbols::T_Generic());
        }
    }

    static core::ClassOrModuleRef stubConstant(core::MutableContext ctx, core::ClassOrModuleRef owner,
                                               ast::ConstantLit *out, bool possibleGenericType) {
        auto symbol = ctx.state.enterClassSymbol(ctx.locAt(out->loc), owner,
                                                 ast::cast_tree<ast::UnresolvedConstantLit>(out->original)->cnst);

        auto data = symbol.data(ctx);
        // force a singleton into existence
        auto singletonClass = data->singletonClass(ctx);
        if (possibleGenericType) {
            ensureTGenericMixin(ctx, singletonClass);
        }

        out->symbol = symbol;
        return symbol;
    }

    static void stubConstantSuffix(core::MutableContext ctx, core::ClassOrModuleRef owner,
                                   std::vector<ast::ConstantLit *> suffix, bool possibleGenericType) {
        if (suffix.empty()) {
            return;
        }

        auto last = suffix.end() - 1;
        for (auto it = suffix.begin(); it != last; ++it) {
            owner = stubConstant(ctx, owner, *it, false);
        }
        stubConstant(ctx, owner, *last, possibleGenericType);
    }

    // Turn a symbol into a vector of `NameRefs`. Returns true if a non-empty result vector was populated, and false if
    // the scope was root, or one of the owning symbols in the hierarchy doesn't exist.
    static bool scopeToNames(core::GlobalState &gs, core::SymbolRef sym, std::vector<core::NameRef> &res) {
        res.clear();
        while (sym.exists() && sym != core::Symbols::root()) {
            if (!sym.isClassOrModule()) {
                res.clear();
                return false;
            }

            auto cls = sym.asClassOrModuleRef();
            res.emplace_back(cls.data(gs)->name);
            sym = cls.data(gs)->owner;
        }

        // Explicitly consider an empty top-level scope as one to skip. This arises when the symbol passed in is
        // `core::Symbols::root()`, which will always be the top-most parent for the `Nesting` linked list present for
        // the `ResolutionItem` being stubbed. As this doesn't correspond to a prefix of the current package's
        // namespace, we return false to signal that this scope doesn't need to be considered as either of the
        // parent/child package special cases.
        if (res.empty()) {
            return false;
        }

        absl::c_reverse(res);

        return true;
    }

    // Determine if a parent package exports a constant named `base`. If it does, return the symbol that already exists
    // for that nesting scope.
    static core::ClassOrModuleRef parentPackageOwner(core::MutableContext ctx, const ImportStubs &importStubs,
                                                     const Nesting *scope, ast::ConstantLit *base) {
        std::vector<core::NameRef> prefix;
        prefix.reserve(5);

        for (auto *cursor = scope; cursor != nullptr; cursor = cursor->parent.get()) {
            // If this is expensive, we could cache it in stubForRbiGeneration below
            if (!scopeToNames(ctx, cursor->scope, prefix)) {
                continue;
            }

            auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(base->original);
            if (importStubs.packageExportsConstant(prefix, original.cnst)) {
                ENFORCE(cursor->scope.isClassOrModule());
                return cursor->scope.asClassOrModuleRef();
            }
        }

        return core::ClassOrModuleRef{};
    }

    // Determine if a child class shares a name in suffix, potentially rooted in any of the nesting scopes.
    static core::ClassOrModuleRef childPackageOwner(core::MutableContext ctx, const ImportStubs &importStubs,
                                                    const Nesting *scope, std::vector<ast::ConstantLit *> suffix) {
        std::vector<core::NameRef> prefix;
        prefix.reserve(5);

        for (auto *cursor = scope; cursor != nullptr; cursor = cursor->parent.get()) {
            // If this is expensive, we could cache it in stubForRbiGeneration below
            if (!scopeToNames(ctx, cursor->scope, prefix)) {
                continue;
            }

            if (importStubs.fromChildNamespace(ctx, prefix, suffix)) {
                ENFORCE(cursor->scope.isClassOrModule());
                return cursor->scope.asClassOrModuleRef();
            }
        }

        return core::ClassOrModuleRef{};
    }

    static void stubForRbiGeneration(core::MutableContext ctx, const ImportStubs &importStubs, const Nesting *scope,
                                     ast::ConstantLit *out, bool possibleGenericType) {
        std::vector<ast::ConstantLit *> suffix;
        {
            auto *cursor = out;
            bool isRootReference = false;
            while (cursor != nullptr) {
                auto *original = ast::cast_tree<ast::UnresolvedConstantLit>(cursor->original);
                if (original == nullptr) {
                    isRootReference = cursor->symbol == core::Symbols::root();
                    break;
                }

                suffix.emplace_back(cursor);
                cursor = ast::cast_tree<ast::ConstantLit>(original->scope);
            }
            absl::c_reverse(suffix);

            // If the constant looks like `::Foo::Bar`, we don't need to apply the heuristics below as it's known to be
            // defined at the root.
            if (isRootReference) {
                stubConstantSuffix(ctx, core::Symbols::root(), suffix, possibleGenericType);
                return;
            }
        }

        // If the constant doesn't resolve to something that overlaps with this package's namespace, it will be defined
        // at the root scope.
        auto owner = core::Symbols::root();

        // First, determine if we're already in the context of a parent package by crawling up the nesting scopes.
        auto parentPackage = parentPackageOwner(ctx, importStubs, scope, suffix.front());
        if (parentPackage.exists()) {
            owner = parentPackage;
        } else {
            // If we're not in a parent package, check the name suffix to determine if we're inside of a child package.
            auto childPackage = childPackageOwner(ctx, importStubs, scope, suffix);
            if (childPackage.exists()) {
                owner = childPackage;
            }
        }

        stubConstantSuffix(ctx, owner, suffix, possibleGenericType);
    }

    // We have failed to resolve the constant. We'll need to report the error and stub it so that we can proceed
    template <typename StateType>
    static void constantResolutionFailed(StateType &gs, core::FileRef file, ResolutionItem &job,
                                         const ImportStubs &importStubs, int &suggestionCount) {
        static_assert(is_same_v<remove_const_t<StateType>, core::GlobalState>);
        constexpr bool isMutableStateType = !is_const_v<StateType>;

        auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(job.out->original);
        core::Context ctx(gs, core::Symbols::root(), file);

        bool singlePackageRbiGeneration = ctx.state.singlePackageImports.has_value();

        auto resolved = resolveConstant(ctx.withOwner(job.scope->scope), job.scope, original, job.resolutionFailed);
        if (resolved.exists() && resolved.isTypeAlias(ctx)) {
            auto resolvedField = resolved.asFieldRef();
            if (resolvedField.data(ctx)->resultType == nullptr) {
                if (singlePackageRbiGeneration) {
                    if constexpr (isMutableStateType) {
                        job.out->symbol.setResultType(gs, core::make_type<core::ClassType>(core::Symbols::todo()));
                    } else {
                        ENFORCE(false, "Was not expecting non-mutating resolver and single package RBI generation");
                    }
                } else {
                    // This is actually a use-site error, but we limit ourselves to emitting it once by checking
                    // resultType
                    auto loc = resolvedField.data(ctx)->loc();
                    if (auto e = ctx.state.beginError(loc, core::errors::Resolver::RecursiveTypeAlias)) {
                        e.setHeader("Unable to resolve right hand side of type alias `{}`", resolved.show(ctx));
                        e.addErrorLine(ctx.locAt(job.out->original.loc()), "Type alias used here");
                    }

                    if constexpr (isMutableStateType) {
                        resolvedField.data(gs)->resultType = core::Types::untyped(ctx, resolved);
                    } else {
                        // Welp. We'll just report extra use-site errors possibly. But we don't care
                        // about errors for the best-effort / non-mutating mode anyways.
                    }
                }
            }
            job.out->symbol = resolved;
            return;
        }
        if (job.resolutionFailed) {
            // we only set this when a job has failed for other reasons and we've already reported an error, and
            // continuining on will only redundantly report that we can't resolve the constant, so bail early here
            job.out->symbol = core::Symbols::untyped();
            return;
        }

        // When generating rbis in single-package mode, we may need to invent a symbol at this point
        if (singlePackageRbiGeneration) {
            if constexpr (isMutableStateType) {
                core::MutableContext ctx(gs, job.scope->scope, file);
                stubForRbiGeneration(ctx, importStubs, job.scope.get(), job.out, job.possibleGenericType);
            } else {
                ENFORCE(false, "Was not expecting non-mutating resolver and single package RBI generation");
            }
            return;
        }

        ENFORCE(!resolved.exists());
        ENFORCE(!job.out->symbol.exists());

        job.out->symbol = core::Symbols::StubModule();

        bool alreadyReported = false;
        job.out->resolutionScopes = make_unique<ast::ConstantLit::ResolutionScopes>();
        if (auto *id = ast::cast_tree<ast::ConstantLit>(original.scope)) {
            auto originalScope = id->symbol.dealias(ctx);
            if (originalScope == core::Symbols::StubModule()) {
                // If we were trying to resolve some literal like C::D but `C` itself was already stubbed,
                // no need to also report that `D` is missing.
                alreadyReported = true;
                job.out->resolutionScopes->emplace_back(core::Symbols::noSymbol());
            } else {
                // We were trying to resolve a constant literal that had an explicit scope.
                // Since Sorbet doesn't combine ancestor resolution and explicit scope resolution,
                // we just put a single entry in the resolutionScopes list.
                job.out->resolutionScopes->emplace_back(originalScope);
            }
        } else {
            auto nesting = job.scope;
            while (true) {
                job.out->resolutionScopes->emplace_back(nesting->scope);
                if (nesting->parent == nullptr) {
                    break;
                }

                nesting = nesting->parent;
            }
        }

        ENFORCE(!job.out->resolutionScopes->empty());
        ENFORCE(job.scope->scope != core::Symbols::StubModule());

        // This name is an artifact of parser recovery--no need to leak the parser implementation to the user,
        // because an error will have already been reported.
        auto constantNameMissing = original.cnst == core::Names::Constants::ConstantNameMissing();
        // If a package exports a name that does not exist only one error should appear at the
        // export site. Ignore resolution failures in the aliases/modules created by packaging to
        // avoid this resulting in duplicate errors.
        if (!constantNameMissing && !alreadyReported && !isPackagerMaterializedConstantRef(ctx, job)) {
            if (auto e = ctx.beginError(job.out->original.loc(), core::errors::Resolver::StubConstant)) {
                e.setHeader("Unable to resolve constant `{}`", original.cnst.show(ctx));

                auto suggestScope = job.out->resolutionScopes->front();
                if (suggestionCount < MAX_SUGGESTION_COUNT && suggestScope.exists() && suggestScope.isClassOrModule()) {
                    suggestionCount++;
                    bool madePackageSuggestions =
                        SuggestPackage::tryPackageCorrections(ctx, e, *job.out->resolutionScopes, original);
                    if (madePackageSuggestions) {
                        return;
                    }
                    auto suggested =
                        suggestScope.asClassOrModuleRef().data(ctx)->findMemberFuzzyMatch(ctx, original.cnst);
                    if (suggested.size() > 3) {
                        suggested.resize(3);
                    }
                    if (!suggested.empty()) {
                        for (auto suggestion : suggested) {
                            const auto replacement = suggestion.symbol.show(ctx);
                            e.didYouMean(replacement, ctx.locAt(job.out->loc));
                            e.addErrorLine(suggestion.symbol.loc(ctx), "`{}` defined here", replacement);
                        }
                    }
                }
            }
        }
    }

    // Is this constant materialized by the packager. This specifically excludes the PackageSpec
    // class body itself.
    static bool isPackagerMaterializedConstantRef(core::Context ctx, const ResolutionItem &job) {
        if (!ctx.file.data(ctx).isPackage()) {
            return false;
        }
        auto nesting = job.scope;
        while (nesting != nullptr) {
            if (nesting->scope == core::Symbols::PackageRegistry() || nesting->scope == core::Symbols::PackageTests()) {
                return true;
            }
            nesting = nesting->parent;
        }
        return false;
    }

    static bool resolveJob(core::Context ctx, ResolutionItem &job) {
        if (isAlreadyResolved(ctx, *job.out)) {
            if (job.possibleGenericType) {
                return false;
            }
            return true;
        }
        auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(job.out->original);
        auto resolved = resolveConstant(ctx.withOwner(job.scope->scope), job.scope, original, job.resolutionFailed);
        if (!resolved.exists()) {
            return false;
        }
        if (resolved.isTypeAlias(ctx)) {
            auto resolvedField = resolved.asFieldRef();
            if (resolvedField.data(ctx)->resultType != nullptr) {
                job.out->symbol = resolved;
                return true;
            }
            return false;
        }

        job.out->symbol = resolved;
        return true;
    }

    static bool resolveConstants(const core::GlobalState &gs, vector<ResolveItems<ResolutionItem>> &jobs,
                                 WorkerPool &workers) {
        if (jobs.empty()) {
            return false;
        }
        auto outputq =
            make_shared<BlockingBoundedQueue<pair<uint32_t, vector<ResolveItems<ResolutionItem>>>>>(jobs.size());
        auto inputq = make_shared<ConcurrentBoundedQueue<ResolveItems<ResolutionItem>>>(jobs.size());
        for (auto &job : jobs) {
            inputq->push(move(job), 1);
        }
        jobs.clear();

        workers.multiplexJob("resolveConstantsWorker", [inputq, outputq, &gs]() {
            vector<ResolveItems<ResolutionItem>> leftover;
            ResolveItems<ResolutionItem> job(core::FileRef(), {});
            uint32_t processed = 0;
            uint32_t retries = 0;
            for (auto result = inputq->try_pop(job); !result.done(); result = inputq->try_pop(job)) {
                if (result.gotItem()) {
                    processed++;
                    core::Context ictx(gs, core::Symbols::root(), job.file);
                    auto origSize = job.items.size();
                    auto fileIt = remove_if(job.items.begin(), job.items.end(),
                                            [&](ResolutionItem &item) -> bool { return resolveJob(ictx, item); });
                    job.items.erase(fileIt, job.items.end());
                    retries += origSize - job.items.size();
                    if (!job.items.empty()) {
                        leftover.emplace_back(move(job));
                    }
                }
            }
            if (processed > 0) {
                auto pair = make_pair(retries, move(leftover));
                outputq->push(move(pair), processed);
            }
        });

        uint32_t retries = 0;
        pair<uint32_t, vector<ResolveItems<ResolutionItem>>> threadResult;
        for (auto result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem()) {
                retries += threadResult.first;
                jobs.insert(jobs.end(), make_move_iterator(threadResult.second.begin()),
                            make_move_iterator(threadResult.second.end()));
            }
        }
        categoryCounterAdd("resolve.constants.nonancestor", "retry", retries);
        return retries > 0;
    }

    static bool resolveTypeAliasJob(core::MutableContext ctx, TypeAliasResolutionItem &job) {
        core::TypeMemberRef enclosingTypeMember;
        core::ClassOrModuleRef enclosingClass = job.lhs.enclosingClass(ctx);
        while (enclosingClass != core::Symbols::root()) {
            auto typeMembers = enclosingClass.data(ctx)->typeMembers();
            if (!typeMembers.empty()) {
                enclosingTypeMember = typeMembers[0];
                break;
            }
            enclosingClass = enclosingClass.data(ctx)->owner;
        }
        auto &rhs = *job.rhs;
        if (enclosingTypeMember.exists()) {
            if (auto e = ctx.beginError(rhs.loc(), core::errors::Resolver::TypeAliasInGenericClass)) {
                e.setHeader("Type aliases are not allowed in generic classes");
                e.addErrorLine(enclosingTypeMember.data(ctx)->loc(), "Here is enclosing generic member");
            }
            job.lhs.setResultType(ctx, core::Types::untyped(ctx, job.lhs));
            return true;
        }
        if (isFullyResolved(ctx, rhs)) {
            // this todo will be resolved during ResolveTypeMembersAndFieldsWalk below
            job.lhs.setResultType(ctx, core::make_type<core::ClassType>(core::Symbols::todo()));
            return true;
        }

        return false;
    }

    static bool resolveClassAliasJob(core::MutableContext ctx, ClassAliasResolutionItem &it) {
        auto rhsSym = it.rhs->symbol;
        if (!rhsSym.exists()) {
            return false;
        }

        if (rhsSym.isTypeAlias(ctx)) {
            if (auto e = ctx.beginError(it.rhs->loc, core::errors::Resolver::ReassignsTypeAlias)) {
                if (ctx.file.data(ctx).isPackage()) {
                    // In --stripe-packages mode, this error surfaces when type aliases
                    // are exported by a package. TODO (aadi-stripe, 12/30/2021) update docs for
                    // error 5034 as part of any larger --stripe-packages documentation effort.
                    e.setHeader("Exporting a type alias is not allowed in package specifications");
                } else {
                    e.setHeader("Reassigning a type alias is not allowed");
                }
                e.addErrorLine(rhsSym.loc(ctx), "Originally defined here");
                auto rhsLoc = ctx.locAt(it.rhs->loc);
                if (rhsLoc.exists()) {
                    e.replaceWith("Declare as type alias", rhsLoc, "T.type_alias {{{}}}", rhsLoc.source(ctx).value());
                }
            }
            it.lhs.setResultType(ctx, core::Types::untypedUntracked());
            return true;
        }

        if (rhsSym.dealias(ctx) != it.lhs) {
            it.lhs.setResultType(ctx, core::make_type<core::AliasType>(rhsSym));
        } else {
            if (auto e = ctx.state.beginError(it.lhs.loc(ctx), core::errors::Resolver::RecursiveClassAlias)) {
                e.setHeader("Class alias aliases to itself");
            }
            it.lhs.setResultType(ctx, core::Types::untypedUntracked());
        }
        return true;
    }

    static void saveAncestorTypeForHashing(core::MutableContext ctx, const AncestorResolutionItem &item) {
        // For LSP, create a synthetic method <unresolved-ancestors> that has a return type containing a type
        // for every ancestor. When this return type changes, LSP takes the slow path (see
        // Symbol::methodShapeHash()).
        auto unresolvedPath = item.ancestor->fullUnresolvedPath(ctx);
        if (!unresolvedPath.has_value()) {
            return;
        }

        auto ancestorType =
            core::make_type<core::UnresolvedClassType>(unresolvedPath->first, move(unresolvedPath->second));

        auto uaSym = ctx.state.enterMethodSymbol(core::Loc::none(), item.klass, core::Names::unresolvedAncestors());

        // Add a fake block argument so that this method symbol passes sanity checks
        auto &arg = ctx.state.enterMethodArgumentSymbol(core::Loc::none(), uaSym, core::Names::blkArg());
        arg.flags.isBlock = true;

        core::TypePtr resultType = uaSym.data(ctx)->resultType;
        if (!resultType) {
            uaSym.data(ctx)->resultType = core::make_type<core::TupleType>(vector<core::TypePtr>{ancestorType});
        } else if (auto tt = core::cast_type<core::TupleType>(resultType)) {
            tt->elems.push_back(ancestorType);
        } else {
            ENFORCE(false);
        }
    }

    static core::ClassOrModuleRef stubSymbolForAncestor(const AncestorResolutionItem &item) {
        if (item.isSuperclass) {
            return core::Symbols::StubSuperClass();
        }
        return core::Symbols::StubMixin();
    }

    static bool resolveAncestorJob(core::MutableContext ctx, AncestorResolutionItem &job, bool lastRun) {
        auto ancestorSym = job.ancestor->symbol;
        if (!ancestorSym.exists()) {
            if (!lastRun && !job.isSuperclass && !job.mixinIndex.has_value()) {
                // This is an include or extend. Add a placeholder to fill in later to preserve
                // ordering of mixins, unless an index is already set.
                job.mixinIndex = job.klass.data(ctx)->addMixinPlaceholder(ctx);
            }
            return false;
        }

        core::ClassOrModuleRef resolvedClass;
        {
            core::SymbolRef resolved;
            if (ancestorSym.isTypeAlias(ctx)) {
                if (!lastRun) {
                    return false;
                }
                if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::DynamicSuperclass)) {
                    e.setHeader("Superclasses and mixins may not be type aliases");
                }
                resolved = stubSymbolForAncestor(job);
            } else {
                resolved = ancestorSym.dealias(ctx);
            }

            if (!resolved.isClassOrModule()) {
                if (!lastRun) {
                    if (!job.isSuperclass && !job.mixinIndex.has_value()) {
                        // This is an include or extend. Add a placeholder to fill in later to preserve
                        // ordering of mixins.
                        job.mixinIndex = job.klass.data(ctx)->addMixinPlaceholder(ctx);
                    }
                    return false;
                }
                if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::DynamicSuperclass)) {
                    e.setHeader("Superclasses and mixins may only use class aliases like `{}`", "A = Integer");
                }
                resolved = stubSymbolForAncestor(job);
            }
            resolvedClass = resolved.asClassOrModuleRef();
        }

        if (resolvedClass == job.klass) {
            if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::CircularDependency)) {
                e.setHeader("Circular dependency: `{}` is a parent of itself", job.klass.show(ctx));
                e.addErrorLine(resolvedClass.data(ctx)->loc(), "Class definition");
            }
            resolvedClass = stubSymbolForAncestor(job);
        } else if (resolvedClass.data(ctx)->derivesFrom(ctx, job.klass)) {
            if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::CircularDependency)) {
                e.setHeader("Circular dependency: `{}` and `{}` are declared as parents of each other",
                            job.klass.show(ctx), resolvedClass.show(ctx));
                e.addErrorLine(job.klass.data(ctx)->loc(), "One definition");
                e.addErrorLine(resolvedClass.data(ctx)->loc(), "Other definition");
            }
            resolvedClass = stubSymbolForAncestor(job);
        }

        bool ancestorPresent = true;
        if (job.isSuperclass) {
            if (resolvedClass == core::Symbols::todo()) {
                // No superclass specified
                ancestorPresent = false;
            } else if (!job.klass.data(ctx)->superClass().exists() ||
                       job.klass.data(ctx)->superClass() == core::Symbols::todo() ||
                       job.klass.data(ctx)->superClass() == resolvedClass) {
                job.klass.data(ctx)->setSuperClass(resolvedClass);
            } else {
                if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::RedefinitionOfParents)) {
                    e.setHeader("Parent of class `{}` redefined from `{}` to `{}`", job.klass.show(ctx),
                                job.klass.data(ctx)->superClass().show(ctx), resolvedClass.show(ctx));
                }
            }
        } else {
            if (!job.klass.data(ctx)->addMixin(ctx, resolvedClass, job.mixinIndex)) {
                if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::IncludesNonModule)) {
                    e.setHeader("Only modules can be `{}`d, but `{}` is a class", job.isInclude ? "include" : "extend",
                                resolvedClass.show(ctx));
                    e.addErrorLine(resolvedClass.data(ctx)->loc(), "`{}` defined as a class here",
                                   resolvedClass.show(ctx));
                }
            }
        }

        if (ancestorPresent) {
            saveAncestorTypeForHashing(ctx, job);
        }
        return true;
    }

    template <typename StateType>
    static void resolveClassMethodsJob(StateType &gs, const ClassMethodsResolutionItem &todo) {
        static_assert(is_same_v<remove_const_t<StateType>, core::GlobalState>);
        constexpr bool isMutableStateType = !is_const_v<StateType>;

        auto owner = todo.owner;
        auto send = todo.send;
        if (!owner.isClassOrModule() || !owner.asClassOrModuleRef().data(gs)->isModule()) {
            if (auto e =
                    gs.beginError(core::Loc(todo.file, send->loc), core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("`{}` can only be declared inside a module, not a class", send->fun.show(gs));
            }
            return;
        }

        auto ownerKlass = owner.asClassOrModuleRef();
        const auto numPosArgs = send->numPosArgs();
        if (numPosArgs == 0) {
            // The arity mismatch error will be emitted later by infer.
            return;
        }

        auto encounteredError = false;
        for (auto i = 0; i < numPosArgs; ++i) {
            auto &arg = send->getPosArg(i);
            if (arg.isSelfReference()) {
                auto recv = ast::cast_tree<ast::ConstantLit>(send->recv);
                if (recv != nullptr && recv->symbol == core::Symbols::Magic()) {
                    // This is the first argument of a Magic.mixes_in_class_methods() call
                    continue;
                }
            }

            auto *id = ast::cast_tree<ast::ConstantLit>(arg);

            if (id == nullptr || !id->symbol.exists()) {
                if (auto e = gs.beginError(core::Loc(todo.file, send->loc),
                                           core::errors::Resolver::InvalidMixinDeclaration)) {
                    e.setHeader("Argument to `{}` must be statically resolvable to a module", send->fun.show(gs));
                }
                continue;
            }
            if (!id->symbol.isClassOrModule()) {
                if (auto e =
                        gs.beginError(core::Loc(todo.file, id->loc), core::errors::Resolver::InvalidMixinDeclaration)) {
                    e.setHeader("Argument to `{}` must be statically resolvable to a module", send->fun.show(gs));
                }
                continue;
            }
            if (id->symbol.asClassOrModuleRef().data(gs)->isClass()) {
                if (auto e =
                        gs.beginError(core::Loc(todo.file, id->loc), core::errors::Resolver::InvalidMixinDeclaration)) {
                    e.setHeader("`{}` is a class, not a module; Only modules may be mixins", id->symbol.show(gs));
                }
                encounteredError = true;
            }
            if (id->symbol == owner) {
                if (auto e =
                        gs.beginError(core::Loc(todo.file, id->loc), core::errors::Resolver::InvalidMixinDeclaration)) {
                    e.setHeader("Must not pass your self to `{}`", send->fun.show(gs));
                }
                encounteredError = true;
            }
            if (encounteredError) {
                continue;
            }

            if constexpr (isMutableStateType) {
                // Get the fake property holding the mixes
                auto mixMethod = ownerKlass.data(gs)->findMethod(gs, core::Names::mixedInClassMethods());
                if (!mixMethod.exists()) {
                    // We never stored a mixin in this symbol
                    // Create a the fake property that will hold the mixed in modules
                    mixMethod = gs.enterMethodSymbol(core::Loc{todo.file, send->loc}, ownerKlass,
                                                     core::Names::mixedInClassMethods());
                    mixMethod.data(gs)->resultType = core::make_type<core::TupleType>(vector<core::TypePtr>{});

                    // Create a dummy block argument to satisfy sanitycheck during GlobalState::expandNames
                    auto &arg = gs.enterMethodArgumentSymbol(core::Loc::none(), mixMethod, core::Names::blkArg());
                    arg.flags.isBlock = true;
                }

                auto type = core::make_type<core::ClassType>(id->symbol.asClassOrModuleRef());
                auto &elems = (core::cast_type<core::TupleType>(mixMethod.data(gs)->resultType))->elems;
                // Make sure we are not adding existing symbols to our tuple
                if (absl::c_find(elems, type) == elems.end()) {
                    elems.emplace_back(type);
                }
            }
        }
    }

    template <typename StateType>
    static void resolveRequiredAncestorsJob(StateType &gs, const RequireAncestorResolutionItem &todo) {
        static_assert(is_same_v<remove_const_t<StateType>, core::GlobalState>);
        constexpr bool isMutableStateType = !is_const_v<StateType>;

        auto owner = todo.owner;
        auto send = todo.send;
        auto loc = core::Loc(todo.file, send->loc);

        if (!owner.data(gs)->isModule() && !owner.data(gs)->flags.isAbstract) {
            if (auto e = gs.beginError(loc, core::errors::Resolver::InvalidRequiredAncestor)) {
                e.setHeader("`{}` can only be declared inside a module or an abstract class", send->fun.show(gs));
            }
            return;
        }

        auto *block = send->block();

        if (send->numPosArgs() > 0) {
            if (auto e = gs.beginError(loc, core::errors::Resolver::InvalidRequiredAncestor)) {
                e.setHeader("`{}` only accepts a block", send->fun.show(gs));
                e.addErrorNote("Use {} to auto-correct using the new syntax",
                               "--isolate-error-code 5062 -a --typed true");

                if (block != nullptr) {
                    return;
                }

                string replacement = "";
                int indent = core::Loc::offset2Pos(todo.file.data(gs), send->loc.beginPos()).column - 1;
                int index = 1;
                const auto numPosArgs = send->numPosArgs();
                for (auto i = 0; i < numPosArgs; ++i) {
                    auto &arg = send->getPosArg(i);
                    auto argLoc = core::Loc(todo.file, arg.loc());
                    replacement += fmt::format("{:{}}{} {{ {} }}{}", "", index == 1 ? 0 : indent, send->fun.show(gs),
                                               argLoc.source(gs).value(), index < numPosArgs ? "\n" : "");
                    index += 1;
                }
                e.addAutocorrect(
                    core::AutocorrectSuggestion{fmt::format("Replace `{}` with `{}`", send->fun.show(gs), replacement),
                                                {core::AutocorrectSuggestion::Edit{loc, replacement}}});
            }
            return;
        }

        if (block == nullptr) {
            return; // The sig mismatch error will be emitted later by infer.
        }

        ENFORCE(block->body);

        auto blockLoc = core::Loc(todo.file, block->body.loc());
        auto *id = ast::cast_tree<ast::ConstantLit>(block->body);

        if (id == nullptr || !id->symbol.exists() || !id->symbol.isClassOrModule()) {
            if (auto e = gs.beginError(blockLoc, core::errors::Resolver::InvalidRequiredAncestor)) {
                e.setHeader("Argument to `{}` must be statically resolvable to a class or a module",
                            send->fun.show(gs));
            }
            return;
        }

        if (id->symbol == owner) {
            if (auto e = gs.beginError(blockLoc, core::errors::Resolver::InvalidRequiredAncestor)) {
                e.setHeader("Must not pass yourself to `{}`", send->fun.show(gs));
            }
            return;
        }

        if constexpr (isMutableStateType) {
            owner.data(gs)->recordRequiredAncestor(gs, id->symbol.asClassOrModuleRef(), blockLoc);
        }
    }

    static void tryRegisterSealedSubclass(core::MutableContext ctx, AncestorResolutionItem &job) {
        ENFORCE(job.ancestor->symbol.exists(), "Ancestor must exist, or we can't check whether it's sealed.");
        auto ancestorSym = job.ancestor->symbol.dealias(ctx).asClassOrModuleRef();

        if (!ancestorSym.data(ctx)->flags.isSealed) {
            return;
        }
        Timer timeit(ctx.state.tracer(), "resolver.registerSealedSubclass");

        ancestorSym.data(ctx)->recordSealedSubclass(ctx, job.klass);
    }

    void transformAncestor(core::Context ctx, core::ClassOrModuleRef klass, ast::ExpressionPtr &ancestor,
                           bool isInclude, bool isSuperclass = false) {
        if (auto *constScope = ast::cast_tree<ast::UnresolvedConstantLit>(ancestor)) {
            auto scopeTmp = nesting_;
            if (isSuperclass) {
                nesting_ = nesting_->parent;
            }
            ancestor = postTransformUnresolvedConstantLit(ctx, std::move(ancestor));
            nesting_ = scopeTmp;
        }
        AncestorResolutionItem job;
        job.klass = klass;
        job.isSuperclass = isSuperclass;
        job.isInclude = isInclude;

        if (auto *cnst = ast::cast_tree<ast::ConstantLit>(ancestor)) {
            auto sym = cnst->symbol;
            if (sym.exists() && sym.isTypeAlias(ctx)) {
                if (auto e = ctx.beginError(cnst->loc, core::errors::Resolver::DynamicSuperclass)) {
                    e.setHeader("Superclasses and mixins may not be type aliases");
                }
                return;
            }
            ENFORCE(sym.exists() ||
                    ast::isa_tree<ast::ConstantLit>(
                        ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(cnst->original).scope) ||
                    ast::isa_tree<ast::EmptyTree>(
                        ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(cnst->original).scope));
            if (isSuperclass && sym == core::Symbols::todo()) {
                // This is the case where the superclass is empty, for example: `class A; end`
                return;
            }
            job.ancestor = cnst;
        } else if (ancestor.isSelfReference()) {
            auto loc = ancestor.loc();
            auto enclosingClass = ctx.owner.enclosingClass(ctx);
            auto nw = ast::MK::UnresolvedConstant(loc, std::move(ancestor), enclosingClass.data(ctx)->name);
            auto out = ast::make_expression<ast::ConstantLit>(loc, enclosingClass, std::move(nw));
            job.ancestor = ast::cast_tree<ast::ConstantLit>(out);
            ancestor = std::move(out);
        } else if (ast::isa_tree<ast::EmptyTree>(ancestor)) {
            return;
        } else {
            ENFORCE(false, "Namer should have not allowed this");
        }

        todoAncestors_.emplace_back(std::move(job));
    }

    ast::ExpressionPtr walkUnresolvedConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
        if (auto *c = ast::cast_tree<ast::UnresolvedConstantLit>(tree)) {
            c->scope = walkUnresolvedConstantLit(ctx, std::move(c->scope));
            auto loc = c->loc;
            auto out = ast::make_expression<ast::ConstantLit>(loc, core::Symbols::noSymbol(), std::move(tree));
            ResolutionItem job{nesting_, ast::cast_tree<ast::ConstantLit>(out)};
            if (resolveJob(ctx, job)) {
                categoryCounterInc("resolve.constants.nonancestor", "firstpass");
            } else {
                todo_.emplace_back(std::move(job));
            }
            return out;
        }
        if (ast::isa_tree<ast::EmptyTree>(tree) || ast::isa_tree<ast::ConstantLit>(tree)) {
            return tree;
        }

        // Uncommon case. Will result in "Dynamic constant references are not allowed" eventually.
        // Still want to do our best to recover (for e.g., LSP queries)
        return ast::TreeMap::apply(ctx, *this, std::move(tree));
    }

public:
    ResolveConstantsWalk() : nesting_(nullptr) {}

    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        nesting_ = make_unique<Nesting>(std::move(nesting_), ast::cast_tree_nonnull<ast::ClassDef>(tree).symbol);
        return tree;
    }

    ast::ExpressionPtr postTransformUnresolvedConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
        return walkUnresolvedConstantLit(ctx, std::move(tree));
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        auto klass = original.symbol;

        bool isInclude = true;
        for (auto &ancst : original.ancestors) {
            bool isSuperclass = (original.kind == ast::ClassDef::Kind::Class && &ancst == &original.ancestors.front() &&
                                 !klass.data(ctx)->isSingletonClass(ctx));
            transformAncestor(isSuperclass ? ctx : ctx.withOwner(klass), klass, ancst, isInclude, isSuperclass);
        }

        auto singleton = klass.data(ctx)->lookupSingletonClass(ctx);
        isInclude = false;
        for (auto &ancst : original.singletonAncestors) {
            ENFORCE(singleton.exists());
            transformAncestor(ctx.withOwner(klass), singleton, ancst, isInclude);
        }

        // Check for ambiguous definitions.
        if (checkAmbiguousDefinition(ctx)) {
            const auto ambigDef = findAnyDefinitionAmbiguousWithCurrent(ctx);
            if (ambigDef.exists()) {
                if (auto e = ctx.beginError(original.loc, core::errors::Resolver::AmbiguousDefinitionError)) {
                    e.setHeader("Class definition is ambiguous");
                    e.addErrorLine(ambigDef.loc(ctx), "Alternate definition {} here",
                                   ambigDef.showFullNameWithoutPackagePrefix(ctx));
                }
            }
        }

        nesting_ = nesting_->parent;
        return tree;
    }

    const bool checkAmbiguousDefinition(core::Context ctx) {
        if (ctx.state.runningUnderAutogen) {
            // no need to check in autogen
            return false;
        }

        if (ctx.file.data(ctx).isPackage()) {
            // no need to check package files
            return false;
        }

        if (nesting_->scope == core::Symbols::root()) {
            // no need to check <root> def itself
            return false;
        }

        // If current definition is owned by package registry/test, this means it is a package-mangled module.
        const core::SymbolRef curOwner = nesting_->scope.owner(ctx);
        if (curOwner == core::Symbols::PackageTests() || curOwner == core::Symbols::PackageRegistry()) {
            return false;
        }

        // If current parent is owned by package registry/test, this means we are at package-top-level (and we skip).
        // If we didn't skip here, we'd end up checking symbols like "Test", "Opus", etc.
        //
        // We have a small set of these leading package namespace symbols at Stripe, and we don't expect these to run
        // into ambiguous definition issues. Skipping checking these saves on significant overhead.
        const core::SymbolRef curParentOwner = nesting_->parent->scope.owner(ctx);
        if (curParentOwner == core::Symbols::PackageTests() || curParentOwner == core::Symbols::PackageRegistry()) {
            return false;
        }

        // Can't be ambiguous if current definition is single-part, don't check in this case.
        if (curOwner == nesting_->parent->scope) {
            return false;
        }

        return true;
    }

    const core::SymbolRef findAnyDefinitionAmbiguousWithCurrent(core::Context ctx) {
        const core::SymbolRef defaultSymbol;
        auto curSym = nesting_->scope;
        auto curNesting = nesting_->parent;
        if (curNesting == nullptr || curNesting->scope == core::Symbols::root()) {
            // can't be ambiguous if nested directly under root scope
            return defaultSymbol;
        }

        // find preceding symbol for current def
        core::SymbolRef lastSym;
        while (curSym != curNesting->scope && curSym.exists() && curSym != core::Symbols::root()) {
            lastSym = curSym;
            curSym = curSym.owner(ctx);
        }

        if (!curSym.exists() || (curSym.exists() && curSym == core::Symbols::root())) {
            // preceding symbol cannot be ambiguous in these cases
            return defaultSymbol;
        }
        auto precedingSymForCurDef = lastSym;

        if (!precedingSymForCurDef.isClassOrModule()) {
            // only proceed if preceding symbol is a class or module
            return defaultSymbol;
        }

        const auto &precedingSymKlass = precedingSymForCurDef.asClassOrModuleRef().data(ctx);
        if (!precedingSymKlass->isClassOrModuleUndeclared()) {
            // Not a filler def, but a real def
            return defaultSymbol;
        }

        // preceding symbol for current def is a filler
        core::NameRef filler = precedingSymForCurDef.name(ctx);

        // Look for filler name in all nestings above current nesting.
        curNesting = curNesting->parent;
        while (curNesting != nullptr) {
            if (curNesting->scope.isClassOrModule()) {
                auto scopeSym = curNesting->scope.asClassOrModuleRef().data(ctx);
                const auto ambigDef = scopeSym->findMember(ctx, filler);
                if (ambigDef.exists()) {
                    // Filler name found! Definition is ambiguous.
                    return ambigDef;
                }
            }

            curNesting = curNesting->parent;
        }

        return defaultSymbol;
    }

    ast::ExpressionPtr postTransformAssign(core::Context ctx, ast::ExpressionPtr tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);

        auto *id = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (id == nullptr || !id->symbol.isStaticField(ctx)) {
            return tree;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn.rhs);
        if (send != nullptr && send->fun == core::Names::typeAlias()) {
            if (!send->hasBlock()) {
                // if we have an invalid (i.e. nullary) call to TypeAlias, then we'll treat it as a type alias for
                // Untyped and report an error here: otherwise, we end up in a state at the end of constant resolution
                // that won't match our expected invariants (and in fact will fail our sanity checks)
                // auto temporaryUntyped = ast::MK::Untyped(asgn->lhs.get()->loc);
                auto temporaryUntyped = ast::MK::Block0(asgn.lhs.loc(), ast::MK::Untyped(asgn.lhs.loc()));
                send->setBlock(std::move(temporaryUntyped));

                // because we're synthesizing a fake "untyped" here and actually adding it to the AST, we won't report
                // an arity mismatch for `T.untyped` in the future, so report the arity mismatch now
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeAlias)) {
                    e.setHeader("No block given to `{}`", "T.type_alias");
                    CorrectTypeAlias::eagerToLazy(ctx, e, send);
                }
            }
            auto *block = send->block();
            this->todoTypeAliases_.emplace_back(id->symbol, ctx.file, &block->body);

            // We also enter a ResolutionItem for the lhs of a type alias so even if the type alias isn't used,
            // we'll still emit a warning when the rhs of a type alias doesn't resolve.
            this->todo_.emplace_back(nesting_, id);
            return tree;
        }

        auto *rhs = ast::cast_tree<ast::ConstantLit>(asgn.rhs);
        if (rhs == nullptr) {
            return tree;
        }

        // TODO(perf) currently, by construction the last item in resolve todo list is the one this alias depends on
        // We may be able to get some perf by using this
        this->todoClassAliases_.emplace_back(id->symbol, rhs);
        return tree;
    }

    ast::ExpressionPtr postTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);
        if (send.recv.isSelfReference()) {
            if (send.fun == core::Names::mixesInClassMethods()) {
                this->todoClassMethods_.emplace_back(ctx.file, ctx.owner, &send);
            } else if (send.fun == core::Names::requiresAncestor()) {
                if (ctx.state.requiresAncestorEnabled) {
                    this->todoRequiredAncestors_.emplace_back(ctx.file, ctx.owner.asClassOrModuleRef(), &send);
                }
            }
        } else {
            auto recvAsConstantLit = ast::cast_tree<ast::ConstantLit>(send.recv);
            if (recvAsConstantLit != nullptr && recvAsConstantLit->symbol == core::Symbols::Magic() &&
                send.fun == core::Names::mixesInClassMethods()) {
                this->todoClassMethods_.emplace_back(ctx.file, ctx.owner, &send);
            } else if (recvAsConstantLit != nullptr && send.fun == core::Names::squareBrackets() &&
                       ctx.state.singlePackageImports.has_value()) {
                // In single-package RBI generation mode, we treat Constant[...] as
                // possible generic types.
                ResolutionItem job{nesting_, recvAsConstantLit};
                job.possibleGenericType = true;
                if (!resolveJob(ctx, job)) {
                    todo_.emplace_back(std::move(job));
                }
            }
        }
        return tree;
    }

    static bool compareFiles(const core::GlobalState &gs, core::FileRef lhs, core::FileRef rhs) {
        core::StrictLevel left = core::StrictLevel::Strong;
        core::StrictLevel right = core::StrictLevel::Strong;

        if (lhs.exists()) {
            left = lhs.data(gs).strictLevel;
        }
        if (rhs.exists()) {
            right = rhs.data(gs).strictLevel;
        }
        if (left != right) {
            return right < left;
        }
        return lhs < rhs;
    }

    static bool compareLocOffsets(core::LocOffsets lhs, core::LocOffsets rhs) {
        if (lhs.beginPos() != rhs.beginPos()) {
            return lhs.beginPos() < rhs.beginPos();
        }
        return lhs.endPos() < rhs.endPos();
    }

    static int constantDepth(ast::ConstantLit *exp) {
        int depth = 0;
        ast::ConstantLit *scope = exp;
        while (scope->original && (scope = ast::cast_tree<ast::ConstantLit>(
                                       ast::cast_tree<ast::UnresolvedConstantLit>(scope->original)->scope))) {
            depth += 1;
        }
        return depth;
    }

    struct ResolveWalkResult {
        vector<ResolveItems<ResolutionItem>> todo_;
        vector<ResolveItems<AncestorResolutionItem>> todoAncestors_;
        vector<ResolveItems<ClassAliasResolutionItem>> todoClassAliases_;
        vector<ResolveItems<TypeAliasResolutionItem>> todoTypeAliases_;
        vector<ResolveItems<ClassMethodsResolutionItem>> todoClassMethods_;
        vector<ResolveItems<RequireAncestorResolutionItem>> todoRequiredAncestors_;
        vector<ast::ParsedFile> trees;
    };

    template <typename StateType>
    static vector<ast::ParsedFile> resolveConstants(StateType &gs, vector<ast::ParsedFile> trees, WorkerPool &workers) {
        static_assert(is_same_v<remove_const_t<StateType>, core::GlobalState>);
        constexpr bool isMutableStateType = !is_const_v<StateType>;

        Timer timeit(gs.tracer(), "resolver.resolve_constants");
        const core::GlobalState &igs = gs;
        auto resultq = make_shared<BlockingBoundedQueue<ResolveWalkResult>>(trees.size());
        auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
        for (auto &tree : trees) {
            fileq->push(move(tree), 1);
        }

        workers.multiplexJob("resolveConstantsWalk", [&igs, fileq, resultq]() {
            Timer timeit(igs.tracer(), "ResolveConstantsWorker");
            ResolveConstantsWalk constants;
            ResolveWalkResult walkResult;
            vector<ast::ParsedFile> partiallyResolvedTrees;
            ast::ParsedFile job;
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    core::Context ictx(igs, core::Symbols::root(), job.file);
                    job.tree = ast::TreeMap::apply(ictx, constants, std::move(job.tree));
                    partiallyResolvedTrees.emplace_back(move(job));
                    if (!constants.todo_.empty()) {
                        walkResult.todo_.emplace_back(job.file, move(constants.todo_));
                    }
                    if (!constants.todoAncestors_.empty()) {
                        walkResult.todoAncestors_.emplace_back(job.file, move(constants.todoAncestors_));
                    }
                    if (!constants.todoClassAliases_.empty()) {
                        walkResult.todoClassAliases_.emplace_back(job.file, move(constants.todoClassAliases_));
                    }
                    if (!constants.todoTypeAliases_.empty()) {
                        walkResult.todoTypeAliases_.emplace_back(job.file, move(constants.todoTypeAliases_));
                    }
                    if (!constants.todoClassMethods_.empty()) {
                        walkResult.todoClassMethods_.emplace_back(job.file, move(constants.todoClassMethods_));
                    }
                    if (!constants.todoRequiredAncestors_.empty()) {
                        walkResult.todoRequiredAncestors_.emplace_back(job.file,
                                                                       move(constants.todoRequiredAncestors_));
                    }
                }
            }
            if (!partiallyResolvedTrees.empty()) {
                walkResult.trees = move(partiallyResolvedTrees);
                auto computedTreesCount = walkResult.trees.size();
                resultq->push(move(walkResult), computedTreesCount);
            }
        });
        trees.clear();
        vector<ResolveItems<ResolutionItem>> todo;
        vector<ResolveItems<AncestorResolutionItem>> todoAncestors;
        vector<ResolveItems<ClassAliasResolutionItem>> todoClassAliases;
        vector<ResolveItems<TypeAliasResolutionItem>> todoTypeAliases;
        vector<ResolveItems<ClassMethodsResolutionItem>> todoClassMethods;
        vector<ResolveItems<RequireAncestorResolutionItem>> todoRequiredAncestors;

        {
            ResolveWalkResult threadResult;
            for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
                 !result.done();
                 result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
                if (result.gotItem()) {
                    todo.insert(todo.end(), make_move_iterator(threadResult.todo_.begin()),
                                make_move_iterator(threadResult.todo_.end()));
                    todoAncestors.insert(todoAncestors.end(), make_move_iterator(threadResult.todoAncestors_.begin()),
                                         make_move_iterator(threadResult.todoAncestors_.end()));
                    todoClassAliases.insert(todoClassAliases.end(),
                                            make_move_iterator(threadResult.todoClassAliases_.begin()),
                                            make_move_iterator(threadResult.todoClassAliases_.end()));
                    todoTypeAliases.insert(todoTypeAliases.end(),
                                           make_move_iterator(threadResult.todoTypeAliases_.begin()),
                                           make_move_iterator(threadResult.todoTypeAliases_.end()));
                    todoClassMethods.insert(todoClassMethods.end(),
                                            make_move_iterator(threadResult.todoClassMethods_.begin()),
                                            make_move_iterator(threadResult.todoClassMethods_.end()));
                    todoRequiredAncestors.insert(todoRequiredAncestors.end(),
                                                 make_move_iterator(threadResult.todoRequiredAncestors_.begin()),
                                                 make_move_iterator(threadResult.todoRequiredAncestors_.end()));
                    trees.insert(trees.end(), make_move_iterator(threadResult.trees.begin()),
                                 make_move_iterator(threadResult.trees.end()));
                }
            }
        }

        // Note: `todo` does not need to be sorted. There are no ordering effects on error production.

        fast_sort(todoAncestors, [](const auto &lhs, const auto &rhs) -> bool { return lhs.file < rhs.file; });
        fast_sort(todoClassAliases, [](const auto &lhs, const auto &rhs) -> bool { return lhs.file < rhs.file; });
        fast_sort(todoTypeAliases, [](const auto &lhs, const auto &rhs) -> bool { return lhs.file < rhs.file; });
        fast_sort(todoClassMethods, [](const auto &lhs, const auto &rhs) -> bool { return lhs.file < rhs.file; });
        fast_sort(todoRequiredAncestors, [](const auto &lhs, const auto &rhs) -> bool { return lhs.file < rhs.file; });

        ENFORCE(todoRequiredAncestors.empty() || gs.requiresAncestorEnabled);
        fast_sort(trees, [](const auto &lhs, const auto &rhs) -> bool { return lhs.file < rhs.file; });

        Timer timeit1(gs.tracer(), "resolver.resolve_constants.fixed_point");

        bool progress = true;
        bool first = true; // we need to run at least once to force class aliases and type aliases

        // If the constant didn't immediately resolve during the initial treewalk, and we're not
        // allowed to mutate GlobalState, it will never resolve. Let's just skip to the error phase.
        if constexpr (isMutableStateType) {
            while (progress && (first || !todo.empty() || !todoAncestors.empty())) {
                first = false;
                counterInc("resolve.constants.retries");
                {
                    Timer timeit(gs.tracer(), "resolver.resolve_constants.fixed_point.ancestors");
                    // This is an optimization. The order should not matter semantically
                    // We try to resolve most ancestors second because this makes us much more likely to resolve
                    // everything else.
                    long retries = 0;
                    auto f = [&gs, &retries](ResolveItems<AncestorResolutionItem> &job) -> bool {
                        core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                        const auto origSize = job.items.size();
                        auto g = [&](AncestorResolutionItem &item) -> bool {
                            auto resolved = resolveAncestorJob(ctx, item, false);
                            if (resolved) {
                                tryRegisterSealedSubclass(ctx, item);
                            }
                            return resolved;
                        };
                        auto fileIt = remove_if(job.items.begin(), job.items.end(), std::move(g));
                        job.items.erase(fileIt, job.items.end());
                        retries += origSize - job.items.size();
                        return job.items.empty();
                    };
                    auto it = remove_if(todoAncestors.begin(), todoAncestors.end(), std::move(f));
                    todoAncestors.erase(it, todoAncestors.end());
                    categoryCounterAdd("resolve.constants.ancestor", "retry", retries);
                    progress = retries > 0;
                }
                {
                    Timer timeit(gs.tracer(), "resolver.resolve_constants.fixed_point.constants");
                    bool resolvedSomeConstants = resolveConstants(gs, todo, workers);
                    progress = progress || resolvedSomeConstants;
                }
                {
                    Timer timeit(gs.tracer(), "resolver.resolve_constants.fixed_point.class_aliases");
                    // This is an optimization. The order should not matter semantically
                    // This is done as a "pre-step" because the first iteration of this effectively ran in TreeMap.
                    // every item in todoClassAliases implicitly depends on an item in item in todo
                    // there would be no point in running the todoClassAliases step before todo

                    long retries = 0;
                    auto f = [&gs, &retries](ResolveItems<ClassAliasResolutionItem> &job) -> bool {
                        core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                        auto origSize = job.items.size();
                        auto g = [&](ClassAliasResolutionItem &item) -> bool {
                            return resolveClassAliasJob(ctx, item);
                        };
                        auto fileIt = remove_if(job.items.begin(), job.items.end(), std::move(g));
                        job.items.erase(fileIt, job.items.end());
                        retries += origSize - job.items.size();
                        return job.items.empty();
                    };
                    auto it = remove_if(todoClassAliases.begin(), todoClassAliases.end(), std::move(f));
                    todoClassAliases.erase(it, todoClassAliases.end());
                    categoryCounterAdd("resolve.constants.aliases", "retry", retries);
                    progress = progress || retries > 0;
                }
                {
                    Timer timeit(gs.tracer(), "resolver.resolve_constants.fixed_point.type_aliases");
                    long retries = 0;
                    auto f = [&gs, &retries](ResolveItems<TypeAliasResolutionItem> &job) -> bool {
                        core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                        auto origSize = job.items.size();
                        auto g = [&](TypeAliasResolutionItem &item) -> bool { return resolveTypeAliasJob(ctx, item); };
                        auto fileIt = remove_if(job.items.begin(), job.items.end(), std::move(g));
                        job.items.erase(fileIt, job.items.end());
                        retries += origSize - job.items.size();
                        return job.items.empty();
                    };
                    auto it = remove_if(todoTypeAliases.begin(), todoTypeAliases.end(), std::move(f));
                    todoTypeAliases.erase(it, todoTypeAliases.end());
                    categoryCounterAdd("resolve.constants.typealiases", "retry", retries);
                    progress = progress || retries > 0;
                }
            }
        }

        {
            Timer timeit(gs.tracer(), "resolver.mixes_in_class_methods");
            for (auto &job : todoClassMethods) {
                for (auto &todo : job.items) {
                    resolveClassMethodsJob(gs, todo);
                }
            }
            todoClassMethods.clear();
        }

        {
            Timer timeit(gs.tracer(), "resolver.requires_ancestor");
            for (auto &job : todoRequiredAncestors) {
                for (auto &todo : job.items) {
                    resolveRequiredAncestorsJob(gs, todo);
                }
            }
            todoRequiredAncestors.clear();
        }

        // We can no longer resolve new constants. All the code below reports errors

        categoryCounterAdd("resolve.constants.nonancestor", "failure", todo.size());
        categoryCounterAdd("resolve.constants.ancestor", "failure", todoAncestors.size());

        /*
         * Sort errors so we choose a deterministic error to report for each
         * missing constant:
         *
         * - Visit the strictest files first. If we were to report an error in
         *     an untyped file it would get suppressed, even if the same error
         *     also appeared in a typed file.
         *
         * - Break ties within strictness levels by file ID. We populate file
         *     IDs in the order we are given files on the command-line, so this
         *     means users see the error on the first file they provided.
         *
         * - Within a file, report the first occurrence.
         */
        fast_sort(todo,
                  [&gs](const auto &lhs, const auto &rhs) -> bool { return compareFiles(gs, lhs.file, rhs.file); });

        for (auto &todos : todo) {
            fast_sort(todos.items, [](const ResolutionItem &lhs, const ResolutionItem &rhs) -> bool {
                if (lhs.out->loc == rhs.out->loc) {
                    return constantDepth(lhs.out) < constantDepth(rhs.out);
                }
                return compareLocOffsets(lhs.out->loc, rhs.out->loc);
            });
        }

        fast_sort(todoAncestors,
                  [&gs](const auto &lhs, const auto &rhs) -> bool { return compareFiles(gs, lhs.file, rhs.file); });

        for (auto &todos : todoAncestors) {
            fast_sort(todos.items, [](const AncestorResolutionItem &lhs, const AncestorResolutionItem &rhs) -> bool {
                if (lhs.ancestor->loc == rhs.ancestor->loc) {
                    return constantDepth(lhs.ancestor) < constantDepth(rhs.ancestor);
                }
                return compareLocOffsets(lhs.ancestor->loc, rhs.ancestor->loc);
            });
        }

        // Note that this is missing alias stubbing, thus resolveJob needs to be able to handle missing aliases.

        {
            Timer timeit(gs.tracer(), "resolver.resolve_constants.errors");

            // Initialize the stubbed parent namespaces if we're generating an interface for a single package
            ImportStubs importStubs;
            bool singlePackageRbiGeneration = gs.singlePackageImports.has_value();
            if (singlePackageRbiGeneration) {
                if constexpr (isMutableStateType) {
                    importStubs = ImportStubs::make(gs);
                } else {
                    ENFORCE(false, "Was not expecting non-mutating resolver and single package RBI generation");
                }
            }

            // Only give suggestions for the first several constants, because fuzzy suggestions are expensive.
            int suggestionCount = 0;
            for (auto &job : todo) {
                for (auto &item : job.items) {
                    constantResolutionFailed(gs, job.file, item, importStubs, suggestionCount);
                }
            }

            if (singlePackageRbiGeneration) {
                if constexpr (isMutableStateType) {
                    for (auto &job : todoClassAliases) {
                        core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                        for (auto &item : job.items) {
                            resolveClassAliasJob(ctx, item);
                        }
                    }

                } else {
                    ENFORCE(false, "Was not expecting non-mutating resolver and single package RBI generation");
                }
            }

            if constexpr (isMutableStateType) {
                // Only purpose of resolveAncestorJob is to mutate the symbol table, not the tree, so
                // in non-mutating resolver mode we don't have to do anything (because we don't care
                // about errors).
                for (auto &job : todoAncestors) {
                    core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                    for (auto &item : job.items) {
                        auto resolved = resolveAncestorJob(ctx, item, true);
                        if (!resolved) {
                            resolved = resolveAncestorJob(ctx, item, true);
                            ENFORCE(resolved);
                        }
                    }
                }
            }
        }

        return trees;
    }
};

class ResolveTypeMembersAndFieldsWalk {
    // A type_member, type_template, or T.type_alias that needs to have types
    // resolved.
    struct ResolveAssignItem {
        // The owner at the time the assignment was encountered.
        core::SymbolRef owner;

        // The symbol being populated, either a type alias or an individual
        // type_member.
        core::SymbolRef lhs;

        // The type_member, type_template, or type_alias of the RHS.
        ast::Send *rhs;

        // The symbols that this alias depends on that have type members.
        vector<core::SymbolRef> dependencies;

        core::FileRef file;
    };

    struct ResolveAttachedClassItem {
        core::SymbolRef owner;
        core::ClassOrModuleRef klass;
        core::FileRef file;
    };

    struct ResolveCastItem {
        core::FileRef file;
        core::SymbolRef owner;
        ast::ExpressionPtr *typeArg;
        ast::Cast *cast;
    };

    struct ResolveFieldItem {
        core::FileRef file;
        core::SymbolRef owner;
        ast::UnresolvedIdent *ident;
        ast::Cast *cast;
        bool atTopLevel = false;
    };

    struct ResolveSimpleStaticFieldItem {
        core::FieldRef sym;
        core::TypePtr resultType;
    };

    struct ResolveStaticFieldItem {
        core::FileRef file;
        core::FieldRef sym;
        ast::Assign *asgn;
    };

    struct ResolveMethodAliasItem {
        core::FileRef file;
        core::ClassOrModuleRef owner;
        core::LocOffsets loc;
        core::LocOffsets toNameLoc;
        core::NameRef toName;
        core::NameRef fromName;
    };

    struct ResolveTypeMembersAndFieldsResult {
        vector<ast::ParsedFile> files;
        vector<ResolveAssignItem> todoAssigns;
        vector<ResolveAttachedClassItem> todoAttachedClassItems;
        vector<core::SymbolRef> todoUntypedResultTypes;
        vector<ResolveCastItem> todoResolveCastItems;
        vector<ResolveFieldItem> todoResolveFieldItems;
        vector<ResolveStaticFieldItem> todoResolveStaticFieldItems;
        vector<ResolveSimpleStaticFieldItem> todoResolveSimpleStaticFieldItems;
        vector<ResolveMethodAliasItem> todoMethodAliasItems;
    };

    vector<ResolveAssignItem> todoAssigns_;
    vector<ResolveAttachedClassItem> todoAttachedClassItems_;
    vector<core::SymbolRef> todoUntypedResultTypes_;
    vector<ResolveCastItem> todoResolveCastItems_;
    vector<ResolveFieldItem> todoResolveFieldItems_;
    vector<ResolveStaticFieldItem> todoResolveStaticFieldItems_;
    vector<ResolveSimpleStaticFieldItem> todoResolveSimpleStaticFieldItems_;
    vector<ResolveMethodAliasItem> todoMethodAliasItems_;

    // State for tracking type usage inside of a type alias or type member
    // definition
    bool trackDependencies_ = false;
    vector<bool> classOfDepth_;
    vector<core::SymbolRef> dependencies_;
    std::vector<int> nestedBlockCounts;

    void extendClassOfDepth(ast::Send &send) {
        if (trackDependencies_) {
            classOfDepth_.emplace_back(isT(send.recv) && send.fun == core::Names::classOf());
        }
    }

    static bool isT(const ast::ExpressionPtr &expr) {
        auto *tMod = ast::cast_tree<ast::ConstantLit>(expr);
        return tMod && tMod->symbol == core::Symbols::T();
    }

    static bool isTodo(const core::TypePtr &type) {
        return core::isa_type<core::ClassType>(type) &&
               core::cast_type_nonnull<core::ClassType>(type).symbol == core::Symbols::todo();
    }

    static bool isLHSResolved(core::Context ctx, core::SymbolRef sym) {
        if (sym.isTypeMember()) {
            auto *lambdaParam = core::cast_type<core::LambdaParam>(sym.resultType(ctx));
            ENFORCE(lambdaParam != nullptr);

            // both bounds are set to todo in the namer, so it's sufficient to
            // just check one here.
            return !isTodo(lambdaParam->lowerBound);
        }

        return !isTodo(sym.resultType(ctx));
    }

    static bool isGenericResolved(core::Context ctx, core::SymbolRef sym) {
        if (sym.isClassOrModule()) {
            return absl::c_all_of(sym.asClassOrModuleRef().data(ctx)->typeMembers(),
                                  [&](core::SymbolRef tm) { return isLHSResolved(ctx, tm); });
        }

        return isLHSResolved(ctx, sym);
    }

    // Resolve a cast to a simple, non-generic class type (e.g., T.let(x, ClassOrModule)). Returns `false` if
    // `ResolveCastItem` is not simple.
    [[nodiscard]] static bool tryResolveSimpleClassCastItem(core::Context ctx, ResolveCastItem &job) {
        if (!ast::isa_tree<ast::ConstantLit>(*job.typeArg)) {
            return false;
        }

        auto &lit = ast::cast_tree_nonnull<ast::ConstantLit>(*job.typeArg);
        if (!lit.symbol.isClassOrModule()) {
            return false;
        }

        auto data = lit.symbol.asClassOrModuleRef().data(ctx);

        // A class with type members is not simple.
        if (!data->typeMembers().empty()) {
            return false;
        }

        resolveCastItem(ctx, job);
        return true;
    }

    // Resolve a potentially more complex cast (e.g., may reference type member or alias).
    static void resolveCastItem(core::Context ctx, ResolveCastItem &job) {
        ParsedSig emptySig;
        auto allowSelfType = true;
        auto allowRebind = false;
        auto allowTypeMember = true;
        job.cast->type = TypeSyntax::getResultType(
            ctx, *job.typeArg, emptySig,
            TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, core::Symbols::noSymbol()});
    }

    // Attempts to resolve the type of the given field. Returns `false` if the cast is not yet resolved.
    static void resolveField(core::MutableContext ctx, ResolveFieldItem &job) {
        auto cast = job.cast;

        core::ClassOrModuleRef scope;
        auto uid = job.ident;
        if (uid->kind == ast::UnresolvedIdent::Kind::Class) {
            if (!ctx.owner.isClassOrModule()) {
                if (auto e = ctx.beginError(uid->loc, core::errors::Resolver::InvalidDeclareVariables)) {
                    e.setHeader("The class variable `{}` must be declared at class scope", uid->name.show(ctx));
                }
            }

            scope = ctx.owner.enclosingClass(ctx);
        } else {
            // we need to check nested block counts because we want all fields to be declared on top level of either
            // class or body, rather then nested in some block
            if (job.atTopLevel && ctx.owner.isClassOrModule()) {
                // Declaring a class instance variable
            } else if (job.atTopLevel && ctx.owner.name(ctx) == core::Names::initialize()) {
                // Declaring a instance variable
            } else if (ctx.owner.isMethod() &&
                       ctx.owner.asMethodRef().data(ctx)->owner.data(ctx)->isSingletonClass(ctx) &&
                       !core::Types::isSubType(ctx, core::Types::nilClass(), cast->type)) {
                // Declaring a class instance variable in a static method
                if (auto e = ctx.beginError(uid->loc, core::errors::Resolver::InvalidDeclareVariables)) {
                    e.setHeader("The singleton instance variable `{}` must be declared inside the class body or "
                                "declared nilable",
                                uid->name.show(ctx));
                }
            } else if (!core::Types::isSubType(ctx, core::Types::nilClass(), cast->type)) {
                // Inside a method; declaring a normal instance variable
                if (auto e = ctx.beginError(uid->loc, core::errors::Resolver::InvalidDeclareVariables)) {
                    e.setHeader("The instance variable `{}` must be declared inside `{}` or declared nilable",
                                uid->name.show(ctx), "initialize");
                }
            }
            scope = ctx.selfClass();
        }

        auto prior = scope.data(ctx)->findMember(ctx, uid->name);
        if (prior.exists() && prior.isFieldOrStaticField()) {
            auto priorField = prior.asFieldRef();
            if (core::Types::equiv(ctx, priorField.data(ctx)->resultType, cast->type)) {
                // We already have a symbol for this field, and it matches what we already saw, so we can short
                // circuit.
                return;
            } else {
                // We do some normalization here to ensure that the file / line we report the error on doesn't
                // depend on the order that we traverse files nor the order we traverse within a file.
                auto priorLoc = priorField.data(ctx)->loc();
                core::Loc reportOn;
                core::Loc errorLine;
                core::Loc thisLoc = core::Loc(job.file, uid->loc);
                if (thisLoc.file() == priorLoc.file()) {
                    reportOn = thisLoc.beginPos() < priorLoc.beginPos() ? thisLoc : priorLoc;
                    errorLine = thisLoc.beginPos() < priorLoc.beginPos() ? priorLoc : thisLoc;
                } else {
                    reportOn = thisLoc.file() < priorLoc.file() ? thisLoc : priorLoc;
                    errorLine = thisLoc.file() < priorLoc.file() ? priorLoc : thisLoc;
                }

                if (auto e = ctx.state.beginError(reportOn, core::errors::Resolver::DuplicateVariableDeclaration)) {
                    e.setHeader("Redeclaring variable `{}` with mismatching type", uid->name.show(ctx));
                    e.addErrorLine(errorLine, "Previous declaration is here:");
                }
                return;
            }
        }
        core::FieldRef var;

        if (uid->kind == ast::UnresolvedIdent::Kind::Class) {
            var = ctx.state.enterStaticFieldSymbol(core::Loc(job.file, uid->loc), scope, uid->name);
        } else {
            var = ctx.state.enterFieldSymbol(core::Loc(job.file, uid->loc), scope, uid->name);
        }

        var.data(ctx)->resultType = cast->type;
        return;
    }

    // Resolve the type of the rhs of a constant declaration. This logic is
    // extremely simplistic; We only handle simple literals, and explicit casts.
    //
    // We don't handle array or hash literals, because intuiting the element
    // type (once we have generics) will be nontrivial.
    [[nodiscard]] static core::TypePtr resolveConstantType(core::Context ctx, const ast::ExpressionPtr &expr) {
        core::TypePtr result;
        typecase(
            expr, [&](const ast::Literal &a) { result = core::Types::dropLiteral(ctx, a.value); },
            [&](const ast::Cast &cast) {
                if (cast.type == core::Types::todo()) {
                    return;
                }

                if (cast.cast != core::Names::let() && cast.cast != core::Names::uncheckedLet()) {
                    if (auto e = ctx.beginError(cast.loc, core::errors::Resolver::ConstantAssertType)) {
                        e.setHeader("Use `{}` to specify the type of constants", "T.let");
                    }
                }
                result = cast.type;
            },
            [&](const ast::InsSeq &outer) { result = resolveConstantType(ctx, outer.expr); },
            [&](const ast::ExpressionPtr &expr) {});
        return result;
    }

    // Tries to resolve the given static field. Returns Types::todo() if it is unable to resolve the field.
    [[nodiscard]] static core::TypePtr tryResolveStaticField(core::Context ctx, ResolveStaticFieldItem &job) {
        ENFORCE(job.sym.data(ctx)->flags.isStaticField);
        auto &asgn = job.asgn;
        auto data = job.sym.data(ctx);
        if (data->resultType == nullptr) {
            if (auto resultType = resolveConstantType(ctx, asgn->rhs)) {
                return resultType;
            }
        }
        // resultType was already set. We may be running on the incremental path. Force this field to be resolved in
        // `resolveStaticField`, which may produce an error message.
        return core::Types::todo();
    }

    [[nodiscard]] static core::TypePtr resolveStaticField(core::Context ctx, ResolveStaticFieldItem &job) {
        ENFORCE(job.sym.data(ctx)->flags.isStaticField);
        auto &asgn = job.asgn;
        auto data = job.sym.data(ctx);
        if (data->resultType == nullptr) {
            auto resultType = resolveConstantType(ctx, asgn->rhs);
            // Do not attempt to suggest types for aliases that fail to resolve in package files.
            if (resultType == nullptr && !ctx.file.data(ctx).isPackage()) {
                // Instead of emitting an error now, emit an error in infer that has a proper type suggestion
                auto rhs = move(job.asgn->rhs);
                auto loc = rhs.loc();
                if (!loc.exists()) {
                    // If the rhs happens to be an EmptyTree (e.g., `begin; end`) there will be no loc.
                    // In that case, use the assign's loc instead.
                    loc = job.asgn->loc;
                }
                job.asgn->rhs = ast::MK::Send1(loc, ast::MK::Constant(loc, core::Symbols::Magic()),
                                               core::Names::suggestType(), loc.copyWithZeroLength(), move(rhs));
            }
            return resultType;
        }

        if (!core::isa_type<core::AliasType>(data->resultType)) {
            // If we've already resolved a temporary constant, we still want to run resolveConstantType to
            // report errors (e.g. so that a stand-in untyped value won't suppress errors in subsequent
            // typechecking runs) but we only want to run this on constants that are value-level and not class
            // or type aliases. The check for isa_type<AliasType> makes sure that we skip aliases of the form `X
            // = Integer` and only run this over constant value assignments like `X = 5` or `Y = 5; X = Y`.
            //
            // NOTE that this error is meaningless in package files, and hence suppressed there.
            if (resolveConstantType(ctx, asgn->rhs) == nullptr && !ctx.file.data(ctx).isPackage()) {
                if (auto e = ctx.beginError(asgn->rhs.loc(), core::errors::Resolver::ConstantMissingTypeAnnotation)) {
                    e.setHeader("Constants must have type annotations with `{}` when specifying `{}`", "T.let",
                                "# typed: strict");
                }
            }
        }
        return data->resultType;
    }

    static void resolveTypeMember(core::MutableContext ctx, core::TypeMemberRef lhs, ast::Send *rhs,
                                  vector<bool> &resolvedAttachedClasses) {
        auto data = lhs.data(ctx);
        auto owner = data->owner.asClassOrModuleRef();

        const core::LambdaParam *parentType = nullptr;
        core::SymbolRef parentMember = core::Symbols::noSymbol();
        parentMember = owner.data(ctx)->superClass().data(ctx)->findMember(ctx, data->name);

        // check mixins if the type member doesn't exist in the parent
        if (!parentMember.exists()) {
            for (auto mixin : owner.data(ctx)->mixins()) {
                parentMember = mixin.data(ctx)->findMember(ctx, data->name);
                if (parentMember.exists()) {
                    break;
                }
            }
        }

        if (parentMember.exists()) {
            if (parentMember.isTypeMember()) {
                parentType = core::cast_type<core::LambdaParam>(parentMember.resultType(ctx));
                ENFORCE(parentType != nullptr);
            } else if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::ParentTypeBoundsMismatch)) {
                const auto parentShow = parentMember.show(ctx);
                e.setHeader("`{}` is a type member but `{}` is not a type member", lhs.show(ctx), parentShow);
                e.addErrorLine(parentMember.loc(ctx), "`{}` definition", parentShow);
            }
        }

        // Initialize the resultType to a LambdaParam with default bounds
        auto lambdaParam = core::make_type<core::LambdaParam>(lhs, core::Types::bottom(), core::Types::top());
        data->resultType = lambdaParam;
        auto *memberType = core::cast_type<core::LambdaParam>(lambdaParam);

        // When no args are supplied, this implies that the upper and lower
        // bounds of the type parameter are top and bottom.
        const auto numKwArgs = rhs->numKwArgs();
        if (numKwArgs > 0) {
            for (auto i = 0; i < numKwArgs; ++i) {
                auto &keyExpr = rhs->getKwKey(i);

                auto lit = ast::cast_tree<ast::Literal>(keyExpr);
                if (lit && lit->isSymbol(ctx)) {
                    auto &value = rhs->getKwValue(i);

                    ParsedSig emptySig;
                    auto allowSelfType = true;
                    auto allowRebind = false;
                    auto allowTypeMember = false;
                    core::TypePtr resTy = TypeSyntax::getResultType(
                        ctx, value, emptySig, TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, lhs});

                    switch (lit->asSymbol(ctx).rawId()) {
                        case core::Names::fixed().rawId():
                            memberType->lowerBound = resTy;
                            memberType->upperBound = resTy;
                            break;

                        case core::Names::lower().rawId():
                            memberType->lowerBound = resTy;
                            break;

                        case core::Names::upper().rawId():
                            memberType->upperBound = resTy;
                            break;
                    }
                }
            }
        }

        // If the parent bounds existis, validate the new bounds against
        // those of the parent.
        // NOTE: these errors could be better for cases involving
        // `fixed`.
        if (parentType != nullptr) {
            if (!core::Types::isSubType(ctx, parentType->lowerBound, memberType->lowerBound)) {
                if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::ParentTypeBoundsMismatch)) {
                    e.setHeader("parent lower bound `{}` is not a subtype of lower bound `{}`",
                                parentType->lowerBound.show(ctx), memberType->lowerBound.show(ctx));
                }
            }
            if (!core::Types::isSubType(ctx, memberType->upperBound, parentType->upperBound)) {
                if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::ParentTypeBoundsMismatch)) {
                    e.setHeader("upper bound `{}` is not a subtype of parent upper bound `{}`",
                                memberType->upperBound.show(ctx), parentType->upperBound.show(ctx));
                }
            }
        }

        // Ensure that the new lower bound is a subtype of the upper
        // bound. This will be a no-op in the case that the type member
        // is fixed.
        if (!core::Types::isSubType(ctx, memberType->lowerBound, memberType->upperBound)) {
            if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::InvalidTypeMemberBounds)) {
                e.setHeader("`{}` is not a subtype of `{}`", memberType->lowerBound.show(ctx),
                            memberType->upperBound.show(ctx));
            }
        }

        // Once the owner has had all of its type members resolved, resolve the
        // AttachedClass on its singleton.
        if (isGenericResolved(ctx, owner)) {
            resolveAttachedClass(ctx, owner, resolvedAttachedClasses);
        }
    }

    static void resolveAttachedClass(core::MutableContext ctx, core::ClassOrModuleRef sym,
                                     vector<bool> &resolvedAttachedClasses) {
        // Avoid trying to re-resolve symbols that are already resolved.
        // This avoids (relatively) expensive findMember operations.
        if (resolvedAttachedClasses[sym.id()] == true) {
            return;
        }

        resolvedAttachedClasses[sym.id()] = true;

        auto singleton = sym.data(ctx)->lookupSingletonClass(ctx);
        if (!singleton.exists()) {
            return;
        }

        // NOTE: AttachedClass will not exist on `T.untyped`, which is a problem
        // because RuntimeProfiled is used as a synonym for `T.untyped`
        // internally.
        auto attachedClass = singleton.data(ctx)->findMember(ctx, core::Names::Constants::AttachedClass());
        if (!attachedClass.exists()) {
            return;
        }
        auto attachedClassTypeMember = attachedClass.asTypeMemberRef();
        auto *lambdaParam = core::cast_type<core::LambdaParam>(attachedClassTypeMember.data(ctx)->resultType);
        ENFORCE(lambdaParam != nullptr);

        if (isTodo(lambdaParam->lowerBound)) {
            lambdaParam->upperBound = sym.data(ctx)->unsafeComputeExternalType(ctx);
            lambdaParam->lowerBound = core::Types::bottom();
        }

        // If all of the singleton members have been resolved, attempt to
        // resolve the singleton of the singleton, if it exists. This case is
        // not redundant with resolveTypeMember, as it will cover the case where
        // there are no non-AttachedClass type members defined on the singleton.
        if (isGenericResolved(ctx, singleton)) {
            // Since we've resolved the singleton's AttachedClass type member, check
            // to see if there's another singleton above that must also be resolved.
            auto parent = singleton.data(ctx)->lookupSingletonClass(ctx);
            if (parent.exists()) {
                resolveAttachedClass(ctx, singleton, resolvedAttachedClasses);
            }
        }
    }

    static void resolveTypeAlias(core::MutableContext ctx, core::SymbolRef lhs, ast::Send *rhs) {
        // this is provided by ResolveConstantsWalk
        ENFORCE(rhs->block() != nullptr);
        auto block = rhs->block();
        ENFORCE(block->body);

        auto allowSelfType = true;
        auto allowRebind = false;
        auto allowTypeMember = true;
        lhs.setResultType(ctx,
                          TypeSyntax::getResultType(ctx, block->body, ParsedSig{},
                                                    TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, lhs}));
    }

    static bool resolveJob(core::MutableContext ctx, ResolveAssignItem &job, vector<bool> &resolvedAttachedClasses) {
        ENFORCE(job.lhs.isTypeAlias(ctx) || job.lhs.isTypeMember());

        if (isLHSResolved(ctx, job.lhs)) {
            return true;
        }

        auto it = std::remove_if(job.dependencies.begin(), job.dependencies.end(), [&](core::SymbolRef dep) {
            if (isGenericResolved(ctx, dep)) {
                if (dep.isClassOrModule()) {
                    // `dep`'s dependencies are resolved. Compute its externalType here so that we can resolve this
                    // type member or type alias's type.
                    dep.asClassOrModuleRef().data(ctx)->unsafeComputeExternalType(ctx);
                }
                return true;
            }
            return false;
        });
        job.dependencies.erase(it, job.dependencies.end());
        if (!job.dependencies.empty()) {
            return false;
        }
        if (job.lhs.isTypeMember()) {
            resolveTypeMember(ctx.withOwner(job.owner), job.lhs.asTypeMemberRef(), job.rhs, resolvedAttachedClasses);
        } else {
            resolveTypeAlias(ctx.withOwner(job.owner), job.lhs, job.rhs);
        }

        return true;
    }

    static void resolveMethodAlias(core::MutableContext ctx, const ResolveMethodAliasItem &job) {
        core::MethodRef toMethod = ctx.owner.asClassOrModuleRef().data(ctx)->findMethodNoDealias(ctx, job.toName);
        if (toMethod.exists()) {
            toMethod = toMethod.data(ctx)->dealiasMethod(ctx);
        }

        if (!toMethod.exists()) {
            if (auto e = ctx.beginError(job.toNameLoc, core::errors::Resolver::BadAliasMethod)) {
                e.setHeader("Can't make method alias from `{}` to non existing method `{}`", job.fromName.show(ctx),
                            job.toName.show(ctx));
            }
            toMethod = core::Symbols::Sorbet_Private_Static_badAliasMethodStub();
        }

        core::MethodRef fromMethod = ctx.owner.asClassOrModuleRef().data(ctx)->findMethodNoDealias(ctx, job.fromName);
        if (fromMethod.exists() && fromMethod.data(ctx)->dealiasMethod(ctx) != toMethod) {
            if (auto e = ctx.beginError(job.loc, core::errors::Resolver::BadAliasMethod)) {
                auto dealiased = fromMethod.data(ctx)->dealiasMethod(ctx);
                if (fromMethod == dealiased) {
                    e.setHeader("Redefining the existing method `{}` as a method alias", fromMethod.show(ctx));
                    e.addErrorLine(fromMethod.data(ctx)->loc(), "Previous definition");
                } else {
                    e.setHeader("Redefining method alias `{}` from `{}` to `{}`", fromMethod.show(ctx),
                                dealiased.show(ctx), toMethod.show(ctx));
                    e.addErrorLine(fromMethod.data(ctx)->loc(), "Previous alias definition");
                    e.addErrorLine(dealiased.data(ctx)->loc(), "Previous alias pointed to");
                    e.addErrorLine(toMethod.data(ctx)->loc(), "Redefining alias to");
                }
            }
            return;
        }

        // No need to make an alias when they're already the same symbol.
        if (fromMethod == toMethod) {
            return;
        }

        auto alias = ctx.state.enterMethodSymbol(ctx.locAt(job.loc), job.owner, job.fromName);
        alias.data(ctx)->resultType = core::make_type<core::AliasType>(core::SymbolRef(toMethod));
    }

    // Returns `true` if `asgn` is a field declaration.
    bool handleFieldDeclaration(core::Context ctx, ast::Assign &asgn) {
        auto *uid = ast::cast_tree<ast::UnresolvedIdent>(asgn.lhs);
        if (uid == nullptr) {
            return false;
        }

        if (uid->kind != ast::UnresolvedIdent::Kind::Instance && uid->kind != ast::UnresolvedIdent::Kind::Class) {
            return false;
        }

        auto *recur = &asgn.rhs;
        while (auto *outer = ast::cast_tree<ast::InsSeq>(*recur)) {
            recur = &outer->expr;
        }

        auto *cast = ast::cast_tree<ast::Cast>(*recur);
        if (cast == nullptr) {
            return false;
        }

        if (cast->cast != core::Names::let()) {
            if (auto e = ctx.beginError(cast->loc, core::errors::Resolver::ConstantAssertType)) {
                e.setHeader("Use `{}` to specify the type of constants", "T.let");
                auto rhsLoc = ctx.locAt(asgn.rhs.loc());
                auto argSource = ctx.locAt(cast->arg.loc()).source(ctx).value();
                e.replaceWith("Replace with `T.let`", rhsLoc, "T.let({}, {})", argSource, cast->type.show(ctx));
                if (cast->cast == core::Names::cast()) {
                    e.addErrorNote("If you really want to use `{}`, assign to an intermediate variable first and then "
                                   "assign that variable to `{}`",
                                   "T.cast", asgn.lhs.toString(ctx));
                }
            }
        }

        ENFORCE_NO_TIMER(!nestedBlockCounts.empty());
        todoResolveFieldItems_.emplace_back(
            ResolveFieldItem{ctx.file, ctx.owner, uid, cast, nestedBlockCounts.back() == 0});

        return true;
    }

    static void computeExternalTypes(core::GlobalState &gs) {
        Timer timeit(gs.tracer(), "resolver.computeExternalType");
        // Ensure all symbols have `externalType` computed.
        for (uint32_t i = 1; i < gs.classAndModulesUsed(); i++) {
            core::ClassOrModuleRef(gs, i).data(gs)->unsafeComputeExternalType(gs);
        }
    }

    void validateNonForcingIsA(core::Context ctx, const ast::Send &send) {
        constexpr string_view method = "T::NonForcingConstants.non_forcing_is_a?";

        if (send.numPosArgs() != 2) {
            return;
        }

        auto numKwArgs = send.numKwArgs();
        if (numKwArgs > 1) {
            return;
        }

        auto stringLoc = send.getPosArg(1).loc();

        auto *literalNode = ast::cast_tree<ast::Literal>(send.getPosArg(1));
        if (literalNode == nullptr) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("`{}` only accepts string literals", method);
            }
            return;
        }

        if (!core::isa_type<core::LiteralType>(literalNode->value)) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("`{}` only accepts string literals", method);
            }
            return;
        }

        auto literal = core::cast_type_nonnull<core::LiteralType>(literalNode->value);
        if (literal.literalKind != core::LiteralType::LiteralTypeKind::String) {
            // Infer will report a type error
            return;
        }

        core::TypePtr packageType = nullptr;
        optional<core::LocOffsets> packageLoc;
        if (send.hasKwArgs()) {
            // this means we got the third package arg
            auto *key = ast::cast_tree<ast::Literal>(send.getKwKey(0));
            if (!key || !key->isSymbol(ctx) || key->asSymbol(ctx) != ctx.state.lookupNameUTF8("package")) {
                return;
            }

            auto *packageNode = ast::cast_tree<ast::Literal>(send.getKwValue(0));
            packageLoc = std::optional<core::LocOffsets>{send.getKwValue(0).loc()};
            if (packageNode == nullptr) {
                if (auto e = ctx.beginError(send.getKwValue(0).loc(), core::errors::Resolver::LazyResolve)) {
                    e.setHeader("`{}` only accepts string literals", method);
                }
                return;
            }

            if (!core::isa_type<core::LiteralType>(packageNode->value) ||
                core::cast_type_nonnull<core::LiteralType>(packageNode->value).literalKind !=
                    core::LiteralType::LiteralTypeKind::String) {
                // Infer will report a type error
                return;
            }
            packageType = packageNode->value;
        }
        // if we got no keyword args, then package should be null, and
        // if we got keyword args, then package should be non-null
        ENFORCE((!send.hasKwArgs() && !packageType) || (send.hasKwArgs() && packageType));

        auto name = literal.asName(ctx);
        auto shortName = name.shortName(ctx);
        if (shortName.empty()) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("The string given to `{}` must not be empty", method);
            }
            return;
        }

        // If this string _begins_ with `::`, then the first fragment will be an empty string; in multiple places
        // below, we'll check to find out whether the first part is `""` or not, which means we're testing whether
        // the string did or did not begin with `::`.
        vector<string> parts = absl::StrSplit(shortName, "::");
        if (packageType) {
            // there's a little bit of a complexity here: we want
            // `non_forcing_is_a?("C::D", package: "A::B")` to be
            // looking for, more or less, `A::B::C::D`. We want this
            // _regardless_ of whether we're in packaged mode or not,
            // in fact: in non-packaged mode, we should treat this as
            // looking for `::A::B::C::D`, and in packaged mode, we're
            // looking for `A::B::C::D` nested inside the desugared
            // `PkgRegistry::Pkg_A_B` namespace.
            if (parts.front() == "") {
                if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                    e.setHeader("The string given to `{}` should not be an absolute constant reference if a "
                                "package name is also provided",
                                method);
                }
                return;
            }
            auto package = core::cast_type_nonnull<core::LiteralType>(packageType);
            auto name = package.asName(ctx).shortName(ctx);
            vector<string> pkgParts = absl::StrSplit(name, "::");
            // add the initial empty string to mimic the leading `::`
            if (ctx.state.packageDB().empty()) {
                pkgParts.insert(pkgParts.begin(), "");
            }
            // and now add the rest of `parts` to it
            pkgParts.insert(pkgParts.end(), parts.begin(), parts.end());
            // and then treat this new vector as the parts to walk over
            parts = move(pkgParts);
            // the path down below tries to find out if `packageType`
            // is null to find out whether it should look up a package
            // or not, so if we're not in package mode set
            // `packageType` to null
            if (ctx.state.packageDB().empty()) {
                packageType = nullptr;
            }
        }

        core::SymbolRef current;
        for (auto part : parts) {
            if (!current.exists()) {
                // First iteration
                if (!packageType) {
                    if (part != "") {
                        if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                            e.setHeader("The string given to `{}` must be an absolute constant reference that "
                                        "starts with `{}`",
                                        method, "::");
                        }
                        return;
                    }
                    current = core::Symbols::root();
                    continue;
                } else {
                    auto package = core::cast_type_nonnull<core::LiteralType>(packageType);
                    auto packageName = package.asName(ctx);
                    auto mangledName = packageName.lookupMangledPrivatePackageName(ctx.state);
                    // if the mangled name doesn't exist, then this means probably there's no package named this
                    if (!mangledName.exists()) {
                        if (auto e = ctx.beginError(*packageLoc, core::errors::Resolver::LazyResolve)) {
                            e.setHeader("Unable to find package: `{}`", packageName.toString(ctx));
                        }
                        return;
                    }
                    current = core::Symbols::PackageRegistry().data(ctx)->findMember(ctx, mangledName);
                    if (!current.exists()) {
                        if (auto e = ctx.beginError(*packageLoc, core::errors::Resolver::LazyResolve)) {
                            e.setHeader("Unable to find package `{}`", packageName.toString(ctx));
                        }
                        return;
                    }
                }
            }

            auto member = ctx.state.lookupNameConstant(part);
            if (!member.exists()) {
                if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                    auto prettyCurrent = current == core::Symbols::root() ? "" : "::" + current.show(ctx);
                    auto pretty = fmt::format("{}::{}", prettyCurrent, part);
                    e.setHeader("Unable to resolve constant `{}`", pretty);
                }
                return;
            }

            core::SymbolRef newCurrent;
            if (current.isClassOrModule()) {
                newCurrent = current.asClassOrModuleRef().data(ctx)->findMember(ctx, member);
            }
            if (!newCurrent.exists()) {
                if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                    auto prettyCurrent = current == core::Symbols::root() ? "" : "::" + current.show(ctx);
                    auto pretty = fmt::format("{}::{}", prettyCurrent, part);
                    e.setHeader("Unable to resolve constant `{}`", pretty);
                }
                return;
            }
            current = newCurrent;
        }

        ENFORCE(current.exists(), "Loop invariant violated");

        if (!current.isClassOrModule()) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("The string given to `{}` must resolve to a class or module", method);
                e.addErrorLine(current.loc(ctx), "Resolved to this constant");
            }
            return;
        }
    }

    core::ClassOrModuleRef methodOwner(core::Context ctx) {
        core::ClassOrModuleRef owner = ctx.owner.enclosingClass(ctx);
        if (owner == core::Symbols::root()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        return owner;
    }

public:
    ResolveTypeMembersAndFieldsWalk() {
        nestedBlockCounts.emplace_back(0);
    }

    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        nestedBlockCounts.emplace_back(0);

        auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        // If this is a class with no type members defined, resolve attached
        // class. Otherwise, it will be resolved once all type members have been
        // resolved as well.
        if (isGenericResolved(ctx, klass.symbol)) {
            todoAttachedClassItems_.emplace_back(ResolveAttachedClassItem{ctx.owner, klass.symbol, ctx.file});
        }

        return tree;
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        nestedBlockCounts.pop_back();
        return tree;
    }

    ast::ExpressionPtr preTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
        nestedBlockCounts.emplace_back(0);
        return tree;
    }

    ast::ExpressionPtr postTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
        nestedBlockCounts.pop_back();
        return tree;
    }

    ast::ExpressionPtr preTransformBlock(core::Context ctx, ast::ExpressionPtr tree) {
        ENFORCE_NO_TIMER(!nestedBlockCounts.empty());
        nestedBlockCounts.back() += 1;
        return tree;
    }

    ast::ExpressionPtr postTransformBlock(core::Context ctx, ast::ExpressionPtr tree) {
        ENFORCE_NO_TIMER(!nestedBlockCounts.empty());
        nestedBlockCounts.back() -= 1;
        return tree;
    }

    ast::ExpressionPtr preTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);
        switch (send.fun.rawId()) {
            case core::Names::typeAlias().rawId():
            case core::Names::typeMember().rawId():
            case core::Names::typeTemplate().rawId():
                break;

            default:
                extendClassOfDepth(send);
                return tree;
        }

        if (send.fun == core::Names::typeAlias()) {
            // don't track dependencies if this is some other method named `type_alias`
            if (!isT(send.recv)) {
                extendClassOfDepth(send);
                return tree;
            }
        }

        trackDependencies_ = true;
        classOfDepth_.clear();
        dependencies_.clear();
        return tree;
    }

    ast::ExpressionPtr postTransformConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
        auto &lit = ast::cast_tree_nonnull<ast::ConstantLit>(tree);

        if (trackDependencies_) {
            core::SymbolRef symbol = lit.symbol.dealias(ctx);
            if (symbol == core::Symbols::T()) {
                return tree;
            }

            if (symbol.isClassOrModule()) {
                auto klass = symbol.asClassOrModuleRef();
                // This is the same as the implementation of T::Generic.[] in calls.cc
                // NOTE: the type members of these symbols will only be depended on during payload construction, as
                // after that their bounds will have been fully resolved.
                if (klass == core::Symbols::T_Array()) {
                    klass = core::Symbols::Array();
                } else if (klass == core::Symbols::T_Hash()) {
                    klass = core::Symbols::Hash();
                } else if (klass == core::Symbols::T_Enumerable()) {
                    klass = core::Symbols::Enumerable();
                } else if (klass == core::Symbols::T_Enumerator()) {
                    klass = core::Symbols::Enumerator();
                } else if (klass == core::Symbols::T_Enumerator_Lazy()) {
                    klass = core::Symbols::Enumerator_Lazy();
                } else if (klass == core::Symbols::T_Range()) {
                    klass = core::Symbols::Range();
                } else if (klass == core::Symbols::T_Set()) {
                    klass = core::Symbols::Set();
                }

                // crawl up uses of `T.class_of` to find the right singleton symbol.
                // This is for cases like `T.class_of(T.class_of(A))`.
                for (auto it = classOfDepth_.rbegin(); it != classOfDepth_.rend() && *it; ++it) {
                    // ignore this as a potential dependency if the singleton
                    // doesn't exist -- this is an indication that there are no type
                    // members on the singleton.
                    klass = klass.data(ctx)->lookupSingletonClass(ctx);
                    if (!klass.exists()) {
                        return tree;
                    }
                }

                if (!klass.data(ctx)->typeMembers().empty()) {
                    dependencies_.emplace_back(klass);
                }
            } else if (symbol.isTypeAlias(ctx)) {
                dependencies_.emplace_back(symbol);
            }
        }

        return tree;
    }

    ast::ExpressionPtr postTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);

        switch (send.fun.rawId()) {
            case core::Names::typeMember().rawId():
            case core::Names::typeTemplate().rawId():
            case core::Names::typeAlias().rawId():
                trackDependencies_ = false;
                break;

            default:
                if (trackDependencies_) {
                    classOfDepth_.pop_back();
                }
                break;
        }

        if (auto *id = ast::cast_tree<ast::ConstantLit>(send.recv)) {
            if (id->symbol != core::Symbols::T() && id->symbol != core::Symbols::T_NonForcingConstants()) {
                return tree;
            }

            switch (send.fun.rawId()) {
                case core::Names::let().rawId():
                case core::Names::bind().rawId():
                case core::Names::uncheckedLet().rawId():
                case core::Names::assertType().rawId():
                case core::Names::cast().rawId(): {
                    if (send.numPosArgs() < 2) {
                        return tree;
                    }

                    ResolveCastItem item;
                    item.file = ctx.file;

                    // Compute the containing class when translating the type,
                    // as there's a very good chance this has been called from a
                    // method context.
                    item.owner = ctx.owner.enclosingClass(ctx);

                    auto typeExpr = ast::MK::KeepForTypechecking(std::move(send.getPosArg(1)));
                    auto expr = std::move(send.getPosArg(0));
                    auto cast =
                        ast::make_expression<ast::Cast>(send.loc, core::Types::todo(), std::move(expr), send.fun);
                    item.cast = ast::cast_tree<ast::Cast>(cast);
                    item.typeArg = &ast::cast_tree_nonnull<ast::Send>(typeExpr).getPosArg(0);

                    // We should be able to resolve simple casts immediately.
                    if (!tryResolveSimpleClassCastItem(ctx.withOwner(item.owner), item)) {
                        todoResolveCastItems_.emplace_back(move(item));
                    }

                    return ast::MK::InsSeq1(send.loc, move(typeExpr), move(cast));
                }
                case core::Names::revealType().rawId():
                case core::Names::absurd().rawId(): {
                    // These errors do not match up with our "upper error levels are super sets
                    // of errors from lower levels" claim. This is ONLY an error in lower levels.

                    string_view doWhat;
                    if (send.fun == core::Names::revealType()) {
                        doWhat = "reveal types";
                    } else if (send.fun == core::Names::absurd()) {
                        doWhat = "check exhaustiveness";
                    } else {
                        doWhat = "resolve strings to constants";
                    }

                    auto fun = fmt::format("T.{}", send.fun.show(ctx));
                    if (ctx.file.data(ctx).strictLevel <= core::StrictLevel::False) {
                        if (auto e = ctx.beginError(send.loc, core::errors::Resolver::RevealTypeInUntypedFile)) {
                            e.setHeader("`{}` can only {} in `{}` files (or higher)", fun, doWhat, "# typed: true");
                        }
                    }
                    return tree;
                }
                case core::Names::nonForcingIsA_p().rawId():
                    validateNonForcingIsA(ctx, send);
                    return tree;
                default:
                    return tree;
            }
        } else if (send.recv.isSelfReference()) {
            if (send.fun != core::Names::aliasMethod()) {
                return tree;
            }

            const auto numPosArgs = send.numPosArgs();
            if (numPosArgs != 2) {
                return tree;
            }

            InlinedVector<core::NameRef, 2> args;
            for (auto i = 0; i < numPosArgs; ++i) {
                auto &arg = send.getPosArg(i);
                auto lit = ast::cast_tree<ast::Literal>(arg);
                if (lit == nullptr || !lit->isSymbol(ctx)) {
                    continue;
                }
                core::NameRef name = lit->asSymbol(ctx);

                args.emplace_back(name);
            }
            if (args.size() != 2) {
                return tree;
            }

            auto fromName = args[0];
            auto toName = args[1];
            auto owner = methodOwner(ctx);

            todoMethodAliasItems_.emplace_back(ResolveMethodAliasItem{
                ctx.file,
                owner,
                send.loc,
                send.getPosArg(1).loc(),
                toName,
                fromName,
            });
        }

        return tree;
    }

    ast::ExpressionPtr postTransformAssign(core::Context ctx, ast::ExpressionPtr tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);
        if (handleFieldDeclaration(ctx, asgn)) {
            return tree;
        }

        auto *id = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (id == nullptr || !id->symbol.exists()) {
            return tree;
        }

        auto sym = id->symbol;
        auto *send = ast::cast_tree<ast::Send>(asgn.rhs);
        if (send && (sym.isTypeAlias(ctx) || sym.isTypeMember())) {
            ENFORCE(!sym.isTypeMember() || send->recv.isSelfReference());

            // This is for a special case that happens with the generation of
            // reflection.rbi: it re-creates the type aliases of the payload,
            // without the knowledge that they are type aliases. The manifestation
            // of this, is that there are entries like:
            //
            // > module T
            // >   Boolean = T.let(nil, T.untyped)
            // > end
            if (sym.isTypeAlias(ctx) && send->fun == core::Names::let()) {
                todoUntypedResultTypes_.emplace_back(sym);
                return tree;
            }

            ENFORCE(send->fun == core::Names::typeAlias() || send->fun == core::Names::typeMember() ||
                    send->fun == core::Names::typeTemplate());

            auto owner = sym.owner(ctx).asClassOrModuleRef();

            dependencies_.emplace_back(owner.data(ctx)->superClass());

            for (auto mixin : owner.data(ctx)->mixins()) {
                dependencies_.emplace_back(mixin);
            }

            todoAssigns_.emplace_back(ResolveAssignItem{ctx.owner, sym, send, std::move(dependencies_), ctx.file});
        } else if (sym.isStaticField(ctx)) {
            ResolveStaticFieldItem job{ctx.file, sym.asFieldRef(), &asgn};
            auto resultType = tryResolveStaticField(ctx, job);
            if (resultType != core::Types::todo()) {
                todoResolveSimpleStaticFieldItems_.emplace_back(
                    ResolveSimpleStaticFieldItem{sym.asFieldRef(), resultType});
            } else {
                todoResolveStaticFieldItems_.emplace_back(move(job));
            }
        }

        if (send == nullptr) {
            return tree;
        }

        trackDependencies_ = false;
        dependencies_.clear();
        classOfDepth_.clear();

        return tree;
    }

    template <typename StateType>
    static vector<ast::ParsedFile> run(StateType &gs, vector<ast::ParsedFile> trees, WorkerPool &workers) {
        static_assert(is_same_v<remove_const_t<StateType>, core::GlobalState>);
        constexpr bool isConstStateType = is_const_v<StateType>;
        Timer timeit(gs.tracer(), "resolver.type_params");

        auto inputq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
        auto outputq = make_shared<BlockingBoundedQueue<ResolveTypeMembersAndFieldsResult>>(trees.size());
        for (auto &tree : trees) {
            inputq->push(move(tree), 1);
        }
        trees.clear();

        workers.multiplexJob("resolveTypeParamsWalk", [&gs, inputq, outputq]() -> void {
            Timer timeit(gs.tracer(), "resolveTypeParamsWalkWorker");
            ResolveTypeMembersAndFieldsWalk walk;
            ResolveTypeMembersAndFieldsResult output;
            ast::ParsedFile job;
            for (auto result = inputq->try_pop(job); !result.done(); result = inputq->try_pop(job)) {
                if (result.gotItem()) {
                    core::Context ctx(gs, core::Symbols::root(), job.file);
                    job.tree = ast::TreeMap::apply(ctx, walk, std::move(job.tree));
                    output.files.emplace_back(move(job));
                }
            }
            if (!output.files.empty()) {
                output.todoAssigns = move(walk.todoAssigns_);
                output.todoAttachedClassItems = move(walk.todoAttachedClassItems_);
                output.todoUntypedResultTypes = move(walk.todoUntypedResultTypes_);
                output.todoResolveCastItems = move(walk.todoResolveCastItems_);
                output.todoResolveFieldItems = move(walk.todoResolveFieldItems_);
                output.todoResolveStaticFieldItems = move(walk.todoResolveStaticFieldItems_);
                output.todoResolveSimpleStaticFieldItems = move(walk.todoResolveSimpleStaticFieldItems_);
                output.todoMethodAliasItems = move(walk.todoMethodAliasItems_);
                auto count = output.files.size();
                outputq->push(move(output), count);
            }
        });

        vector<ast::ParsedFile> combinedFiles;
        // The following items are not flattened; it'd be expensive to do so on large projects (they contain every
        // field/method alias/etc for the entire workspace!)
        vector<vector<ResolveAssignItem>> combinedTodoAssigns;
        vector<vector<ResolveAttachedClassItem>> combinedTodoAttachedClassItems;
        vector<vector<core::SymbolRef>> combinedTodoUntypedResultTypes;
        vector<vector<ResolveCastItem>> combinedTodoResolveCastItems;
        vector<vector<ResolveFieldItem>> combinedTodoResolveFieldItems;
        vector<vector<ResolveStaticFieldItem>> combinedTodoResolveStaticFieldItems;
        vector<vector<ResolveSimpleStaticFieldItem>> combinedTodoResolveSimpleStaticFieldItems;
        vector<vector<ResolveMethodAliasItem>> combinedTodoMethodAliasItems;

        {
            ResolveTypeMembersAndFieldsResult threadResult;
            for (auto result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
                 !result.done();
                 result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
                if (result.gotItem()) {
                    combinedFiles.insert(combinedFiles.end(), make_move_iterator(threadResult.files.begin()),
                                         make_move_iterator(threadResult.files.end()));
                    combinedTodoAssigns.emplace_back(move(threadResult.todoAssigns));
                    combinedTodoAttachedClassItems.emplace_back(move(threadResult.todoAttachedClassItems));
                    combinedTodoUntypedResultTypes.emplace_back(move(threadResult.todoUntypedResultTypes));
                    combinedTodoResolveCastItems.emplace_back(move(threadResult.todoResolveCastItems));
                    combinedTodoResolveFieldItems.emplace_back(move(threadResult.todoResolveFieldItems));
                    combinedTodoAttachedClassItems.emplace_back(move(threadResult.todoAttachedClassItems));
                    combinedTodoAttachedClassItems.emplace_back(move(threadResult.todoAttachedClassItems));
                    combinedTodoAttachedClassItems.emplace_back(move(threadResult.todoAttachedClassItems));
                    combinedTodoResolveStaticFieldItems.emplace_back(move(threadResult.todoResolveStaticFieldItems));
                    combinedTodoResolveSimpleStaticFieldItems.emplace_back(
                        move(threadResult.todoResolveSimpleStaticFieldItems));
                    combinedTodoMethodAliasItems.emplace_back(move(threadResult.todoMethodAliasItems));
                }
            }
        }

        // Put files into a consistent order for subsequent passes.
        fast_sort(combinedFiles, [](auto &a, auto &b) -> bool { return a.file < b.file; });

        if constexpr (!isConstStateType) {
            for (auto &threadTodo : combinedTodoUntypedResultTypes) {
                for (auto sym : threadTodo) {
                    sym.setResultType(gs, core::Types::untypedUntracked());
                }
            }
        }

        vector<bool> resolvedAttachedClasses(gs.classAndModulesUsed());
        if constexpr (!isConstStateType) {
            for (auto &threadTodo : combinedTodoAttachedClassItems) {
                for (auto &job : threadTodo) {
                    core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                    resolveAttachedClass(ctx, job.klass, resolvedAttachedClasses);
                }
            }
        }

        // Resolve simple field declarations. Required so that `type_alias` can refer to an enum value type
        // (which is a static field). This is stronger than we need (we really only need the enum types)
        // but there's no particular reason to delay here.
        if constexpr (!isConstStateType) {
            for (auto &threadTodo : combinedTodoResolveSimpleStaticFieldItems) {
                for (auto &job : threadTodo) {
                    job.sym.data(gs)->resultType = job.resultType;
                }
            }
        }

        // loop over any out-of-order type_member/type_alias references
        if constexpr (!isConstStateType) {
            bool progress = true;
            while (progress && !combinedTodoAssigns.empty()) {
                progress = false;
                auto it =
                    std::remove_if(combinedTodoAssigns.begin(), combinedTodoAssigns.end(),
                                   [&](vector<ResolveAssignItem> &threadTodos) {
                                       auto origSize = threadTodos.size();
                                       auto threadTodoIt = std::remove_if(
                                           threadTodos.begin(), threadTodos.end(), [&](ResolveAssignItem &job) -> bool {
                                               core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                                               return resolveJob(ctx, job, resolvedAttachedClasses);
                                           });
                                       threadTodos.erase(threadTodoIt, threadTodos.end());
                                       progress = progress || threadTodos.size() != origSize;
                                       return threadTodos.empty();
                                   });
                combinedTodoAssigns.erase(it, combinedTodoAssigns.end());
            }

            // If there was a step with no progress, there's a cycle in the
            // type member/alias declarations. This is handled by reporting an error
            // at `typed: false`, and marking all of the involved type
            // members/aliases as T.untyped.
            if (!combinedTodoAssigns.empty()) {
                for (auto &threadTodos : combinedTodoAssigns) {
                    for (auto &job : threadTodos) {
                        if (job.lhs.isTypeMember()) {
                            job.lhs.setResultType(gs, core::make_type<core::LambdaParam>(
                                                          job.lhs.asTypeMemberRef(), core::Types::untypedUntracked(),
                                                          core::Types::untypedUntracked()));
                        } else {
                            job.lhs.setResultType(gs, core::Types::untypedUntracked());
                        }

                        if (auto e = gs.beginError(job.lhs.loc(gs), core::errors::Resolver::TypeMemberCycle)) {
                            auto flavor = job.lhs.isTypeAlias(gs) ? "alias" : "member";
                            e.setHeader("Type {} `{}` is involved in a cycle", flavor, job.lhs.show(gs));
                        }
                    }
                }
            }
        }

        // Compute the resultType of all classes.
        if constexpr (!isConstStateType) {
            computeExternalTypes(gs);
        }

        // Resolve the remaining casts and fields.
        for (auto &threadTodos : combinedTodoResolveCastItems) {
            for (auto &job : threadTodos) {
                core::Context ctx(gs, job.owner, job.file);
                resolveCastItem(ctx, job);
            }
        }
        if constexpr (!isConstStateType) {
            for (auto &threadTodos : combinedTodoResolveFieldItems) {
                for (auto &job : threadTodos) {
                    core::MutableContext ctx(gs, job.owner, job.file);
                    resolveField(ctx, job);
                }
            }
        }
        if constexpr (!isConstStateType) {
            for (auto &threadTodos : combinedTodoResolveStaticFieldItems) {
                for (auto &job : threadTodos) {
                    core::Context ctx(gs, job.sym, job.file);
                    if (auto resultType = resolveStaticField(ctx, job)) {
                        job.sym.data(gs)->resultType = resultType;
                    }
                }
            }
        }
        if constexpr (!isConstStateType) {
            for (auto &threadTodos : combinedTodoMethodAliasItems) {
                for (auto &job : threadTodos) {
                    core::MutableContext ctx(gs, job.owner, job.file);
                    resolveMethodAlias(ctx, job);
                }
            }
        }

        return combinedFiles;
    }
};

class ResolveSignaturesWalk {
public:
    struct ResolveSignatureJob {
        core::ClassOrModuleRef owner;
        ast::MethodDef *mdef;
        core::LocOffsets loc;
        ParsedSig sig;
    };

    struct OverloadedMethodSignature {
        core::LocOffsets loc;
        ParsedSig sig;
        // N.B.: Unused if method has multiple signatures but is not overloaded.
        vector<bool> argsToKeep;
    };

    struct ResolveMultiSignatureJob {
        core::ClassOrModuleRef owner;
        ast::MethodDef *mdef;
        bool isOverloaded;
        InlinedVector<OverloadedMethodSignature, 2> sigs;
    };

    struct ResolveFileSignatures {
        core::FileRef file;
        vector<ResolveSignatureJob> sigs;
        vector<ResolveMultiSignatureJob> multiSigs;
    };

    struct ResolveSignaturesWalkResult {
        vector<ResolveFileSignatures> fileSigs;
        vector<ast::ParsedFile> trees;
    };

    vector<ResolveSignatureJob> signatureJobs;
    vector<ResolveMultiSignatureJob> multiSignatureJobs;

private:
    static ast::Local const *getArgLocal(core::Context ctx, const core::ArgInfo &argSym, const ast::MethodDef &mdef,
                                         int pos, bool isOverloaded) {
        if (!isOverloaded) {
            return ast::MK::arg2Local(mdef.args[pos]);
        }

        // we cannot rely on method and symbol arguments being aligned, as method could have more arguments.
        // we roundtrip through original symbol that is stored in mdef.
        auto internalNameToLookFor = argSym.name;
        auto originalArgIt = absl::c_find_if(mdef.symbol.data(ctx)->arguments,
                                             [&](const auto &arg) { return arg.name == internalNameToLookFor; });
        ENFORCE(originalArgIt != mdef.symbol.data(ctx)->arguments.end());
        auto realPos = originalArgIt - mdef.symbol.data(ctx)->arguments.begin();
        return ast::MK::arg2Local(mdef.args[realPos]);
    }

    static void recordMethodInfoInSig(core::Context ctx, core::MethodRef method, ParsedSig &sig,
                                      const ast::MethodDef &mdef) {
        // Later passes are going to separate the sig and the method definition.
        // Record some information in the sig call itself so that we can reassociate
        // them later.
        //
        // Note that the sig still needs to send to a method called "sig" so that
        // code completion in LSP works.  We change the receiver, below, so that
        // sigs that don't pass through here still reflect the user's intent.
        auto *send = sig.origSend;
        auto *self = ast::cast_tree<ast::Local>(send->getPosArg(0));
        if (self == nullptr) {
            return;
        }

        // We distinguish "user-written" sends by checking for self.
        // T::Sig::WithoutRuntime.sig wouldn't have any runtime effect that we need
        // to record later.
        if (self->localVariable != core::LocalVariable::selfVariable()) {
            return;
        }

        auto *cnst = ast::cast_tree<ast::ConstantLit>(send->recv);
        ENFORCE(cnst != nullptr, "sig send receiver must be a ConstantLit if we got a ParsedSig from the send");

        cnst->symbol = core::Symbols::Sorbet_Private_Static_ResolvedSig();

        send->addPosArg(mdef.flags.isSelfMethod ? ast::MK::True(send->loc) : ast::MK::False(send->loc));
        send->addPosArg(ast::MK::Symbol(send->loc, method.data(ctx)->name));
    }

    static void fillInInfoFromSig(core::MutableContext ctx, core::MethodRef method, core::LocOffsets exprLoc,
                                  ParsedSig &sig, bool isOverloaded, const ast::MethodDef &mdef) {
        ENFORCE(isOverloaded || mdef.symbol == method);
        ENFORCE(isOverloaded || method.data(ctx)->arguments.size() == mdef.args.size());

        if (!sig.seen.returns && !sig.seen.void_) {
            if (auto e = ctx.beginError(exprLoc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`: No return type specified. Specify one with .returns()", "sig");
            }
        }
        if (sig.seen.returns && sig.seen.void_) {
            if (auto e = ctx.beginError(exprLoc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`: Don't use both .returns() and .void", "sig");
            }
        }

        if (sig.seen.abstract) {
            method.data(ctx)->flags.isAbstract = true;
        }
        if (sig.seen.incompatibleOverride) {
            method.data(ctx)->flags.isIncompatibleOverride = true;
        }
        if (!sig.typeArgs.empty()) {
            method.data(ctx)->flags.isGenericMethod = true;
            for (auto &typeSpec : sig.typeArgs) {
                if (typeSpec.type) {
                    auto name = ctx.state.freshNameUnique(core::UniqueNameKind::TypeVarName, typeSpec.name, 1);
                    auto sym = ctx.state.enterTypeArgument(typeSpec.loc, method, name, core::Variance::CoVariant);
                    auto asTypeVar = core::cast_type<core::TypeVar>(typeSpec.type);
                    ENFORCE(asTypeVar != nullptr);
                    asTypeVar->sym = sym;
                    sym.data(ctx)->resultType = typeSpec.type;
                }
            }
        }
        if (sig.seen.overridable) {
            method.data(ctx)->flags.isOverridable = true;
        }
        if (sig.seen.override_) {
            method.data(ctx)->flags.isOverride = true;
        }
        if (sig.seen.final) {
            method.data(ctx)->flags.isFinal = true;
        }
        if (sig.seen.bind) {
            if (sig.bind == core::Symbols::MagicBindToAttachedClass()) {
                if (auto e = ctx.beginError(exprLoc, core::errors::Resolver::BindNonBlockParameter)) {
                    e.setHeader("Using `{}` is not permitted here", "bind");
                    e.addErrorNote("Only block arguments can use `{}`", "bind");
                }
            } else {
                method.data(ctx)->rebind = sig.bind;
            }
        }

        auto methodInfo = method.data(ctx);

        // Is this a signature for a method defined with argument forwarding syntax?
        if (methodInfo->arguments.size() >= 3) {
            // To match, the definition must have been desugared with at least 3 parameters named
            // `<fwd-args>`, `<fwd-kwargs>` and `<fwd-block>`
            auto len = methodInfo->arguments.size();
            auto l1 = getArgLocal(ctx, methodInfo->arguments[len - 3], mdef, len - 3, isOverloaded)->localVariable;
            auto l2 = getArgLocal(ctx, methodInfo->arguments[len - 2], mdef, len - 2, isOverloaded)->localVariable;
            auto l3 = getArgLocal(ctx, methodInfo->arguments[len - 1], mdef, len - 1, isOverloaded)->localVariable;
            if (l1._name == core::Names::fwdArgs() && l2._name == core::Names::fwdKwargs() &&
                l3._name == core::Names::fwdBlock()) {
                if (auto e = ctx.beginError(exprLoc, core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Unsupported `{}` for argument forwarding syntax", "sig");
                    e.addErrorLine(methodInfo->loc(), "Method declares argument forwarding here");
                    e.addErrorNote("Rewrite the method as `def {}(*args, **kwargs, &blk)` to use a signature",
                                   method.show(ctx));
                }
                return;
            }
        }

        // Get the parameters order from the signature
        vector<ParsedSig::ArgSpec> sigParams(sig.argTypes);

        vector<ast::Local const *> defParams; // Parameters order from the method declaration
        bool seenOptional = false;

        methodInfo->resultType = sig.returns;
        int i = -1;
        for (auto &arg : methodInfo->arguments) {
            ++i;
            auto local = getArgLocal(ctx, arg, mdef, i, isOverloaded);
            auto treeArgName = local->localVariable._name;
            ENFORCE(local != nullptr);

            // Check that optional keyword parameters are after all the required ones
            bool isKwd = arg.flags.isKeyword;
            bool isReq = !arg.flags.isBlock && !arg.flags.isRepeated && !arg.flags.isDefault;
            if (isKwd && !isReq) {
                seenOptional = true;
            } else if (isKwd && seenOptional && isReq) {
                if (auto e = ctx.state.beginError(arg.loc, core::errors::Resolver::BadParameterOrdering)) {
                    e.setHeader("Malformed `{}`. Required parameter `{}` must be declared before all the optional ones",
                                "sig", treeArgName.show(ctx));
                    e.addErrorLine(ctx.locAt(exprLoc), "Signature");
                }
            }

            defParams.push_back(local);

            auto spec = absl::c_find_if(sig.argTypes, [&](const auto &spec) { return spec.name == treeArgName; });
            bool isBlkArg = arg.name == core::Names::blkArg();

            if (spec != sig.argTypes.end()) {
                ENFORCE(spec->type != nullptr);

                // It would be nice to remove the restriction on more than these two specific binds, but that would
                // raise a lot more errors
                if (!isBlkArg && (spec->rebind == core::Symbols::MagicBindToAttachedClass() ||
                                  spec->rebind == core::Symbols::MagicBindToSelfType())) {
                    if (auto e = ctx.state.beginError(spec->loc, core::errors::Resolver::BindNonBlockParameter)) {
                        e.setHeader("Using `{}` is not permitted here", "bind");
                        e.addErrorNote("Only block arguments can use `{}`", "bind");
                    }
                }

                arg.type = std::move(spec->type);
                arg.loc = spec->loc;
                arg.rebind = spec->rebind;
                sig.argTypes.erase(spec);
            } else {
                if (arg.type == nullptr) {
                    arg.type = core::Types::untyped(ctx, method);
                }

                // We silence the "type not specified" error when a sig does not mention the synthesized block arg.
                if (!isOverloaded && !isBlkArg && (sig.seen.params || sig.seen.returns || sig.seen.void_)) {
                    // Only error if we have any types
                    if (auto e = ctx.state.beginError(arg.loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`. Type not specified for argument `{}`", "sig",
                                    treeArgName.show(ctx));
                        e.addErrorLine(ctx.locAt(exprLoc), "Signature");
                    }
                }
            }

            if (isOverloaded && arg.flags.isKeyword) {
                if (auto e = ctx.state.beginError(arg.loc, core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Malformed `{}`. Overloaded functions cannot have keyword arguments:  `{}`", "sig",
                                treeArgName.show(ctx));
                }
            }
        }

        for (const auto &spec : sig.argTypes) {
            if (auto e = ctx.state.beginError(spec.loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Unknown argument name `{}`", spec.name.show(ctx));
            }
        }

        // Check params ordering match between signature and definition
        if (sig.argTypes.empty()) {
            int j = 0;
            for (const auto &spec : sigParams) {
                auto *param = defParams[j];
                auto sname = spec.name;
                auto dname = param->localVariable._name;
                // TODO(jvilk): Do we need to check .show? Typically NameRef equality is equal to string equality.
                if (sname != dname && sname.show(ctx) != dname.show(ctx)) {
                    if (auto e = ctx.beginError(param->loc, core::errors::Resolver::BadParameterOrdering)) {
                        e.setHeader("Bad parameter ordering for `{}`, expected `{}` instead", dname.show(ctx),
                                    sname.show(ctx));
                        e.addErrorLine(spec.loc, "Expected index in signature:");
                    }
                }
                j++;
            }
        }

        recordMethodInfoInSig(ctx, method, sig, mdef);
    }

    // Force errors from any signatures that didn't attach to methods.
    // `lastSigs` will always be empty after this function is called.
    void processLeftoverSigs(core::Context ctx, InlinedVector<ast::Send *, 1> &lastSigs) {
        if (!lastSigs.empty()) {
            // These sigs won't have been parsed, as there was no methods to
            // attach them to -- parse them here manually to force any errors.
            for (auto sig : lastSigs) {
                auto allowSelfType = true;
                auto allowRebind = false;
                auto allowTypeMember = true;
                TypeSyntax::parseSig(
                    ctx, *sig, nullptr,
                    TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, core::Symbols::untyped()});
            }

            if (auto e = ctx.beginError(lastSigs[0]->loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`. No method def following it", "sig");
            }

            lastSigs.clear();
        }
    }

    void processClassBody(core::Context ctx, ast::ClassDef &klass) {
        InlinedVector<ast::Send *, 1> lastSigs;
        for (auto &stat : klass.rhs) {
            processStatement(ctx, stat, lastSigs);
        }

        processLeftoverSigs(ctx, lastSigs);

        auto toRemove = remove_if(klass.rhs.begin(), klass.rhs.end(),
                                  [](ast::ExpressionPtr &stat) -> bool { return stat == nullptr; });
        klass.rhs.erase(toRemove, klass.rhs.end());
    }

    void processInSeq(core::Context ctx, ast::InsSeq &seq) {
        InlinedVector<ast::Send *, 1> lastSigs;

        // Explicitly check in the contxt of the class, not <static-init>
        auto classCtx = ctx.withOwner(ctx.owner.enclosingClass(ctx));

        for (auto &stat : seq.stats) {
            processStatement(classCtx, stat, lastSigs);
        }
        if (!ast::isa_tree<ast::EmptyTree>(seq.expr)) {
            processStatement(classCtx, seq.expr, lastSigs);
        }

        processLeftoverSigs(classCtx, lastSigs);

        auto toRemove = remove_if(seq.stats.begin(), seq.stats.end(),
                                  [](ast::ExpressionPtr &stat) -> bool { return stat == nullptr; });
        seq.stats.erase(toRemove, seq.stats.end());
    }

    ParsedSig parseSig(core::Context ctx, core::ClassOrModuleRef sigOwner, const ast::Send &send,
                       ast::MethodDef &mdef) {
        auto allowSelfType = true;
        auto allowRebind = false;
        auto allowTypeMember = true;
        return TypeSyntax::parseSig(ctx.withOwner(sigOwner), send, nullptr,
                                    TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, mdef.symbol});
    }

    void processStatement(core::Context ctx, ast::ExpressionPtr &stat, InlinedVector<ast::Send *, 1> &lastSigs) {
        typecase(
            stat,

            [&](ast::Send &send) {
                if (TypeSyntax::isSig(ctx, send)) {
                    if (!lastSigs.empty()) {
                        if (!ctx.permitOverloadDefinitions(ctx.file)) {
                            if (auto e = ctx.beginError(lastSigs[0]->loc, core::errors::Resolver::OverloadNotAllowed)) {
                                e.setHeader("Unused type annotation. No method def before next annotation");
                                e.addErrorLine(ctx.locAt(send.loc), "Type annotation that will be used instead");
                            }
                        }
                    }

                    lastSigs.emplace_back(&send);
                    return;
                }

                if (send.numPosArgs() == 1 &&
                    (send.fun == core::Names::public_() || send.fun == core::Names::private_() ||
                     send.fun == core::Names::privateClassMethod() || send.fun == core::Names::protected_() ||
                     send.fun == core::Names::packagePrivateClassMethod() ||
                     send.fun == core::Names::packagePrivate())) {
                    processStatement(ctx, send.getPosArg(0), lastSigs);
                    return;
                }
            },

            [&](ast::MethodDef &mdef) {
                if (debug_mode) {
                    bool hasSig = !lastSigs.empty();
                    bool rewriten = mdef.flags.isRewriterSynthesized;
                    bool isRBI = ctx.file.data(ctx).isRBI();
                    if (hasSig) {
                        categoryCounterInc("method.sig", "true");
                    } else {
                        categoryCounterInc("method.sig", "false");
                    }
                    if (rewriten) {
                        categoryCounterInc("method.dsl", "true");
                    } else {
                        categoryCounterInc("method.dsl", "false");
                    }
                    if (isRBI) {
                        categoryCounterInc("method.rbi", "true");
                    } else {
                        categoryCounterInc("method.rbi", "false");
                    }
                    if (hasSig && !isRBI && !rewriten) {
                        counterInc("types.sig.human");
                    }
                }

                if (!lastSigs.empty()) {
                    if (ctx.file.data(ctx).originalSigil == core::StrictLevel::None &&
                        !lastSigs.front()->flags.isRewriterSynthesized) {
                        auto loc = lastSigs.front()->loc;
                        if (auto e = ctx.beginError(loc, core::errors::Resolver::SigInFileWithoutSigil)) {
                            e.setHeader("To use `{}`, this file must declare an explicit `{}` sigil (found: "
                                        "none). If you're not sure which one to use, start with `{}`",
                                        "sig", "# typed:", "# typed: false");
                        }
                    }

                    // process signatures in the context of either the current
                    // class, or the current singleton class, depending on if
                    // the current method is a self method.
                    core::ClassOrModuleRef sigOwner;
                    if (mdef.flags.isSelfMethod) {
                        sigOwner = ctx.owner.asClassOrModuleRef().data(ctx)->lookupSingletonClass(ctx);
                        // namer ensures that all method owners are defined.
                        ENFORCE_NO_TIMER(sigOwner.exists());
                    } else {
                        sigOwner = ctx.owner.asClassOrModuleRef();
                    }

                    if (lastSigs.size() == 1) {
                        auto &lastSig = lastSigs.front();
                        signatureJobs.emplace_back(ResolveSignatureJob{ctx.owner.asClassOrModuleRef(), &mdef,
                                                                       lastSig->loc,
                                                                       parseSig(ctx, sigOwner, *lastSig, mdef)});
                    } else {
                        bool isOverloaded = ctx.permitOverloadDefinitions(ctx.file);
                        InlinedVector<OverloadedMethodSignature, 2> sigs;
                        for (auto &lastSig : lastSigs) {
                            auto sig = parseSig(ctx, sigOwner, *lastSig, mdef);
                            vector<bool> argsToKeep;
                            if (isOverloaded) {
                                for (auto &argTree : mdef.args) {
                                    const auto local = ast::MK::arg2Local(argTree);
                                    auto treeArgName = local->localVariable._name;
                                    ENFORCE(local != nullptr);
                                    argsToKeep.emplace_back(absl::c_find_if(sig.argTypes, [&](auto &spec) {
                                                                return spec.name == treeArgName;
                                                            }) != sig.argTypes.end());
                                }
                            }
                            sigs.emplace_back(OverloadedMethodSignature{lastSig->loc, move(sig), move(argsToKeep)});
                        }

                        multiSignatureJobs.emplace_back(ResolveMultiSignatureJob{ctx.owner.asClassOrModuleRef(), &mdef,
                                                                                 isOverloaded, std::move(sigs)});
                    }

                    lastSigs.clear();
                } else {
                    handleAbstractMethod(ctx, mdef);
                }
            },
            [&](const ast::ClassDef &cdef) {
                // Leave in place
            },

            [&](const ast::EmptyTree &e) { stat.reset(nullptr); },

            [&](const ast::ExpressionPtr &e) {});
    }

public:
    static void resolveMultiSignatureJob(core::MutableContext ctx, ResolveMultiSignatureJob &job) {
        auto &sigs = job.sigs;
        auto &mdef = *job.mdef;
        ENFORCE_NO_TIMER(sigs.size() > 1);

        prodCounterInc("types.sig.count");

        bool isOverloaded = job.isOverloaded;
        auto originalName = mdef.symbol.data(ctx)->name;
        if (isOverloaded) {
            ctx.state.mangleRenameSymbol(mdef.symbol, originalName);
        }

        int i = -1;
        for (auto &sig : sigs) {
            i++;
            core::MethodRef overloadSym;
            if (isOverloaded) {
                overloadSym =
                    ctx.state.enterNewMethodOverload(ctx.locAt(sig.loc), mdef.symbol, originalName, i, sig.argsToKeep);
                overloadSym.data(ctx)->setMethodVisibility(mdef.symbol.data(ctx)->methodVisibility());
                if (i != sigs.size() - 1) {
                    overloadSym.data(ctx)->flags.isOverloaded = true;
                }
            } else {
                overloadSym = mdef.symbol;
            }
            fillInInfoFromSig(ctx, overloadSym, sig.loc, sig.sig, isOverloaded, mdef);
        }
        // handleAbstractMethod called elsewhere
    }
    static void resolveSignatureJob(core::MutableContext ctx, ResolveSignatureJob &job) {
        prodCounterInc("types.sig.count");
        auto &mdef = *job.mdef;
        bool isOverloaded = false;
        fillInInfoFromSig(ctx, mdef.symbol, job.loc, job.sig, isOverloaded, mdef);
        // handleAbstractMethod called elsewhere
    }

    static void handleAbstractMethod(core::Context ctx, ast::MethodDef &mdef) {
        if (mdef.symbol.data(ctx)->flags.isAbstract) {
            if (!ast::isa_tree<ast::EmptyTree>(mdef.rhs)) {
                if (auto e = ctx.beginError(mdef.rhs.loc(), core::errors::Resolver::AbstractMethodWithBody)) {
                    e.setHeader("Abstract methods must not contain any code in their body");
                    e.replaceWith("Delete the body", ctx.locAt(mdef.rhs.loc()), "");
                }

                mdef.rhs = ast::MK::EmptyTree();
            }
            if (!mdef.symbol.enclosingClass(ctx).data(ctx)->flags.isAbstract) {
                if (auto e = ctx.beginError(mdef.loc, core::errors::Resolver::AbstractMethodOutsideAbstract)) {
                    e.setHeader("Before declaring an abstract method, you must mark your class/module "
                                "as abstract using `abstract!` or `interface!`");
                }
            }

            // Rewrite the empty body of the abstract method to forward all arguments to `super`, mirroring the
            // behavior of the runtime.
            ast::Send::ARGS_store args;

            auto argIdx = -1;
            auto numPosArgs = 0;
            for (auto &arg : mdef.args) {
                ++argIdx;

                const ast::Local *local = nullptr;
                if (auto *opt = ast::cast_tree<ast::OptionalArg>(arg)) {
                    local = ast::cast_tree<ast::Local>(opt->expr);
                } else {
                    local = ast::cast_tree<ast::Local>(arg);
                }

                auto &info = mdef.symbol.data(ctx)->arguments[argIdx];
                if (info.flags.isKeyword) {
                    args.emplace_back(ast::MK::Symbol(local->loc, info.name));
                    args.emplace_back(local->deepCopy());
                } else if (info.flags.isRepeated || info.flags.isBlock) {
                    // Explicitly skip for now.
                    // Involves synthesizing a call to callWithSplat, callWithBlock, or
                    // callWithSplatAndBlock
                } else {
                    args.emplace_back(local->deepCopy());
                    ++numPosArgs;
                }
            }

            auto self = ast::MK::Self(mdef.loc);
            mdef.rhs = ast::MK::Send(mdef.loc, std::move(self), core::Names::super(), mdef.loc.copyWithZeroLength(),
                                     numPosArgs, std::move(args));
        } else if (mdef.symbol.enclosingClass(ctx).data(ctx)->flags.isInterface) {
            if (auto e = ctx.beginError(mdef.loc, core::errors::Resolver::ConcreteMethodInInterface)) {
                e.setHeader("All methods in an interface must be declared abstract");
            }
        }
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        processClassBody(ctx.withOwner(klass.symbol), klass);
        return tree;
    }

    ast::ExpressionPtr postTransformInsSeq(core::Context ctx, ast::ExpressionPtr tree) {
        processInSeq(ctx, ast::cast_tree_nonnull<ast::InsSeq>(tree));
        return tree;
    }
};

class ResolveSanityCheckWalk {
public:
    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        ENFORCE(original.symbol != core::Symbols::todo(), "These should have all been resolved: {}",
                tree.toString(ctx));
        if (original.symbol == core::Symbols::root()) {
            ENFORCE(ctx.state.lookupStaticInitForFile(ctx.file).exists());
        } else {
            ENFORCE(ctx.state.lookupStaticInitForClass(original.symbol).exists());
        }
        return tree;
    }
    ast::ExpressionPtr postTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        ENFORCE(original.symbol != core::Symbols::todoMethod(), "These should have all been resolved: {}",
                tree.toString(ctx));
        return tree;
    }
    ast::ExpressionPtr postTransformUnresolvedConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
        ENFORCE(false, "These should have all been removed: {}", tree.toString(ctx));
        return tree;
    }
    ast::ExpressionPtr postTransformUnresolvedIdent(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::UnresolvedIdent>(tree);
        ENFORCE(original.kind != ast::UnresolvedIdent::Kind::Local, "{} should have been removed by local_vars",
                tree.toString(ctx));
        return tree;
    }
    ast::ExpressionPtr postTransformConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ConstantLit>(tree);
        ENFORCE(ResolveConstantsWalk::isAlreadyResolved(ctx, original));
        return tree;
    }
};

template <typename StateType>
ast::ParsedFilesOrCancelled resolveSigs(StateType &gs, vector<ast::ParsedFile> trees, WorkerPool &workers) {
    static_assert(std::is_same_v<remove_const_t<StateType>, core::GlobalState>);
    constexpr bool isConstStateType = std::is_const_v<StateType>;

    Timer timeit(gs.tracer(), "resolver.sigs_vars_and_flatten");
    auto inputq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
    auto outputq = make_shared<BlockingBoundedQueue<ResolveSignaturesWalk::ResolveSignaturesWalkResult>>(trees.size());

    for (auto &tree : trees) {
        inputq->push(move(tree), 1);
    }

    trees.clear();

    workers.multiplexJob("resolveSignaturesWalk", [&gs, inputq, outputq]() -> void {
        ResolveSignaturesWalk walk;
        ResolveSignaturesWalk::ResolveSignaturesWalkResult output;
        ast::ParsedFile job;
        for (auto result = inputq->try_pop(job); !result.done(); result = inputq->try_pop(job)) {
            if (result.gotItem()) {
                core::Context ctx(gs, core::Symbols::root(), job.file);
                job.tree = ast::ShallowMap::apply(ctx, walk, std::move(job.tree));
                if (!walk.signatureJobs.empty() || !walk.multiSignatureJobs.empty()) {
                    output.fileSigs.emplace_back(ResolveSignaturesWalk::ResolveFileSignatures{
                        job.file, move(walk.signatureJobs), move(walk.multiSignatureJobs)});
                }
                output.trees.emplace_back(move(job));
            }
        }
        if (!output.trees.empty()) {
            auto count = output.trees.size();
            outputq->push(move(output), count);
        }
    });

    vector<ResolveSignaturesWalk::ResolveFileSignatures> combinedFileJobs;
    {
        ResolveSignaturesWalk::ResolveSignaturesWalkResult threadResult;
        for (auto result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem()) {
                trees.insert(trees.end(), make_move_iterator(threadResult.trees.begin()),
                             make_move_iterator(threadResult.trees.end()));
                combinedFileJobs.insert(combinedFileJobs.end(), make_move_iterator(threadResult.fileSigs.begin()),
                                        make_move_iterator(threadResult.fileSigs.end()));
            }
        }
    }

    // We need to define sigs in a stable order since, when there are conflicting sigs in multiple RBI files, the last
    // sig 'wins'.
    fast_sort(
        combinedFileJobs,
        [&](const ResolveSignaturesWalk::ResolveFileSignatures &left,
            const ResolveSignaturesWalk::ResolveFileSignatures &right) -> bool { return left.file < right.file; });

    {
        Timer timeit(gs.tracer(), "resolver.resolve_sigs");
        for (auto &file : combinedFileJobs) {
            for (auto &job : file.sigs) {
                if constexpr (!isConstStateType) {
                    core::MutableContext ctx(gs, job.owner, file.file);
                    ResolveSignaturesWalk::resolveSignatureJob(ctx, job);
                }
                {
                    core::Context ctx(gs, job.owner, file.file);
                    ResolveSignaturesWalk::handleAbstractMethod(ctx, *job.mdef);
                }
            }
            for (auto &job : file.multiSigs) {
                if constexpr (!isConstStateType) {
                    core::MutableContext ctx(gs, job.owner, file.file);
                    ResolveSignaturesWalk::resolveMultiSignatureJob(ctx, job);
                }
                {
                    core::Context ctx(gs, job.owner, file.file);
                    ResolveSignaturesWalk::handleAbstractMethod(ctx, *job.mdef);
                }
            }
        }
    }

    return trees;
}
void sanityCheck(const core::GlobalState &gs, vector<ast::ParsedFile> &trees) {
    if (debug_mode) {
        Timer timeit(gs.tracer(), "resolver.sanity_check");
        ResolveSanityCheckWalk sanity;
        for (auto &tree : trees) {
            core::Context ctx(gs, core::Symbols::root(), tree.file);
            tree.tree = ast::TreeMap::apply(ctx, sanity, std::move(tree.tree));
        }
    }
}

void verifyLinearizationComputed(const core::GlobalState &gs) {
    DEBUG_ONLY(for (auto i = 1; i < gs.classAndModulesUsed(); i++) {
        core::ClassOrModuleRef sym(gs, i);
        // If class is not marked as 'linearization computed', then we added a mixin to it since the last slow path.
        ENFORCE_NO_TIMER(sym.data(gs)->flags.isLinearizationComputed, "{}", sym.toString(gs));
    })
}

template <typename StateType>
ast::ParsedFilesOrCancelled runIncrementalImpl(StateType &gs, vector<ast::ParsedFile> trees) {
    static_assert(is_same_v<remove_const_t<StateType>, core::GlobalState>);

    auto workers = WorkerPool::create(0, gs.tracer());
    trees = ResolveConstantsWalk::resolveConstants(gs, std::move(trees), *workers);
    // NOTE: Linearization does not need to be recomputed as we do not mutate mixins() during incremental resolve.
    verifyLinearizationComputed(gs);
    trees = ResolveTypeMembersAndFieldsWalk::run(gs, std::move(trees), *workers);
    auto result = resolveSigs(gs, std::move(trees), *workers);
    if (!result.hasResult()) {
        return result;
    }
    sanityCheck(gs, result.result());
    // This check is FAR too slow to run on large codebases, especially with sanitizers on.
    // But it can be super useful to uncomment when debugging certain issues.
    // ctx.state.sanityCheck();

    return result;
}

} // namespace

ast::ParsedFilesOrCancelled Resolver::run(core::GlobalState &gs, vector<ast::ParsedFile> trees, WorkerPool &workers) {
    const auto &epochManager = *gs.epochManager;
    trees = ResolveConstantsWalk::resolveConstants(gs, std::move(trees), workers);
    if (epochManager.wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled::cancel(move(trees), workers);
    }
    finalizeAncestors(gs);
    if (epochManager.wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled::cancel(move(trees), workers);
    }
    finalizeSymbols(gs);
    if (epochManager.wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled::cancel(move(trees), workers);
    }
    trees = ResolveTypeMembersAndFieldsWalk::run(gs, std::move(trees), workers);
    if (epochManager.wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled::cancel(move(trees), workers);
    }

    auto result = resolveSigs(gs, std::move(trees), workers);
    if (!result.hasResult()) {
        return result;
    }
    sanityCheck(gs, result.result());

    return result;
}

ast::ParsedFilesOrCancelled Resolver::runIncremental(core::GlobalState &gs, vector<ast::ParsedFile> trees) {
    return runIncrementalImpl(gs, move(trees));
}

ast::ParsedFilesOrCancelled Resolver::runIncrementalBestEffort(const core::GlobalState &gs,
                                                               vector<ast::ParsedFile> trees) {
    return runIncrementalImpl(gs, move(trees));
}

vector<ast::ParsedFile> Resolver::runConstantResolution(core::GlobalState &gs, vector<ast::ParsedFile> trees,
                                                        WorkerPool &workers) {
    trees = ResolveConstantsWalk::resolveConstants(gs, std::move(trees), workers);
    sanityCheck(gs, trees);

    return trees;
}

} // namespace sorbet::resolver
