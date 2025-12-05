#ifndef SORBET_GLOBAL_STATE_H
#define SORBET_GLOBAL_STATE_H
#include "absl/synchronization/mutex.h"

#include "common/StableStringStorage.h"
#include "core/Error.h"
#include "core/Files.h"
#include "core/Loc.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TrackUntyped.h"
#include "core/hashing/hashing.h"
#include "core/lsp/DiagnosticSeverity.h"
#include "core/lsp/Query.h"
#include "core/packages/PackageDB.h"
#include "core/packages/PackageInfo.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include <memory>

namespace sorbet::core {

class GlobalState;
class NameRef;
class ClassOrModule;
class SymbolRef;
class ClassOrModuleRef;
class MethodRef;
class FieldRef;
class TypeParameterRef;
class TypeMemberRef;
class NameSubstitution;
class ErrorQueue;
struct LocalSymbolTableHashes;

namespace lsp {
class Task;
class TypecheckEpochManager;
} // namespace lsp

namespace serialize {
class Serializer;
class SerializerImpl;
} // namespace serialize

class NameHash {
    friend class serialize::SerializerImpl;

public:
    using Hash = uint32_t;

    struct Bucket {
        unsigned int hash;
        uint32_t rawId;

        Bucket() = default;
        Bucket(unsigned int hash, uint32_t rawId) : hash{hash}, rawId{rawId} {}

        bool present() const {
            return this->rawId != 0u;
        }

        // Constructs a predicate for identifying empty buckets.
        static inline auto isEmpty() {
            return [](auto name) { return false; };
        }

        // Constructs a predicate for identifying a bucket holding this UTF8 name.
        static inline auto isUtf8(const GlobalState &gs, std::string_view utf8) {
            return [&gs, utf8](auto name) { return name.kind() == NameKind::UTF8 && name.dataUtf8(gs)->utf8 == utf8; };
        }

        // Constructs a predicate for identifying a bucket holding this constant name.
        static inline auto isConstant(const GlobalState &gs, NameRef original) {
            return [&gs, original](auto name) {
                return name.kind() == NameKind::CONSTANT && name.dataCnst(gs)->original == original;
            };
        }

        // Constructs a predicate for identifying a bucket holding this unique name.
        static inline auto isUnique(const GlobalState &gs, UniqueNameKind uniqueNameKind, NameRef original,
                                    uint32_t num) {
            return [&gs, uniqueNameKind, original, num](auto name) {
                if (name.kind() != NameKind::UNIQUE) {
                    return false;
                }

                auto data = name.dataUnique(gs);
                return data->uniqueNameKind == uniqueNameKind && data->num == num && data->original == original;
            };
        }
    };

private:
    std::vector<Bucket> buckets_;

    void moveNames(Bucket *from, Bucket *to, unsigned int szFrom, unsigned int szTo);

public:
    static inline Hash hashMixUTF8(std::string_view name) {
        return _hash(name);
    }

    static inline Hash hashMixUnique(UniqueNameKind unk, unsigned int num, unsigned int rawId) {
        return mix(mix(num, static_cast<uint32_t>(unk)), rawId) * HASH_MULT2 + static_cast<uint32_t>(NameKind::UNIQUE);
    }

    static inline Hash hashMixConstant(unsigned int id) {
        return id * HASH_MULT2 + static_cast<uint32_t>(NameKind::CONSTANT);
    }

    static inline Hash hashNameRef(const GlobalState &gs, NameRef nref) {
        switch (nref.kind()) {
            case NameKind::UTF8:
                return _hash(nref.shortName(gs));
            case NameKind::CONSTANT:
                return hashMixConstant(nref.dataCnst(gs)->original.rawId());
            case NameKind::UNIQUE: {
                auto data = nref.dataUnique(gs);
                return hashMixUnique(data->uniqueNameKind, data->num, data->original.rawId());
            }
        }
    }

    bool empty() const {
        return this->buckets_.empty();
    }

    size_t size() const {
        return this->buckets_.size();
    }

    size_t capacity() const {
        return this->buckets_.capacity();
    }

    void clear() {
        this->buckets_.clear();
    }

    void resize(size_t size) {
        ENFORCE_NO_TIMER((size & (size - 1)) == 0, "namesByHashSize is not a power of 2");
        this->buckets_.resize(size);
    }

    void reserve(size_t size) {
        ENFORCE_NO_TIMER((size & (size - 1)) == 0, "namesByHashSize is not a power of 2");
        this->buckets_.reserve(size);
    }

    absl::Span<const Bucket> buckets() const {
        return absl::MakeSpan(this->buckets_);
    }

    void expandNames(uint32_t utf8NameSize, uint32_t constantNameSize, uint32_t uniqueNameSize);

    template <typename Pred> const Bucket &lookupBucket(Hash hash, Pred &&pred) const {
        unsigned int hashTableSize = buckets_.size();
        unsigned int mask = hashTableSize - 1;
        auto bucketId = hash & mask;
        unsigned int probeCount = 1;

        while (buckets_[bucketId].present()) {
            auto &bucket = buckets_[bucketId];
            if (bucket.hash == hash) {
                auto name = NameRef::fromRawUnchecked(bucket.rawId);
                if (std::invoke(pred, name)) {
                    break;
                }
            }
            bucketId = (bucketId + probeCount) & mask;
            probeCount++;
        }

        if (probeCount == hashTableSize) {
            Exception::raise("Full table?");
        }

        return buckets_[bucketId];
    }

    template <typename Pred> Bucket &lookupBucket(Hash hash, Pred &&pred) {
        return const_cast<Bucket &>(const_cast<const NameHash *>(this)->lookupBucket(hash, std::forward<Pred>(pred)));
    }
};

class FileTable {
    std::vector<std::shared_ptr<File>> files;
    UnorderedMap<std::string, FileRef> fileRefByPath;

public:
    std::shared_ptr<File> &get(size_t ix) {
        return this->files[ix];
    }

    const std::shared_ptr<File> &get(size_t ix) const {
        return this->files[ix];
    }

    void reserve(size_t size) {
        this->files.reserve(size);
    }

    size_t size() const {
        return this->files.size();
    }

    bool empty() const {
        return this->files.empty();
    }

    FileRef findFileByPath(std::string_view path) const;

    FileRef emplace(std::shared_ptr<File> &&file) {
        FileRef ref(this->files.size());
        this->fileRefByPath[file->path()] = ref;
        this->files.emplace_back(std::move(file));
        return ref;
    }

    void clear() {
        this->files.clear();
        this->fileRefByPath.clear();
    }

    void initEmpty() {
        // First file is used to indicate absence of a file
        this->files.emplace_back();
    }

    absl::Span<const std::shared_ptr<File>> span() const {
        return absl::MakeSpan(this->files);
    }
};

class GlobalState final {
    friend NameRef;
    friend ClassOrModule;
    friend Method;
    friend Field;
    friend TypeParameter;
    friend SymbolRef;
    friend ClassOrModuleRef;
    friend MethodRef;
    friend TypeMemberRef;
    friend TypeParameterRef;
    friend FieldRef;
    friend File;
    friend FileRef;
    friend NameSubstitution;
    friend ErrorBuilder;
    friend serialize::Serializer;
    friend serialize::SerializerImpl;
    friend class UnfreezeNameTable;
    friend class UnfreezeSymbolTable;
    friend class UnfreezeFileTable;
    friend struct NameRefDebugCheck;

    // Private constructor that allows a specific globalStateId. Used in `makeEmptyGlobalStateForHashing` to avoid
    // contention on the global state ID atomic.
    GlobalState(std::shared_ptr<ErrorQueue> errorQueue, std::shared_ptr<lsp::TypecheckEpochManager> epochManager,
                std::shared_ptr<FileTable> files, int globalStateId);

public:
    GlobalState(std::shared_ptr<ErrorQueue> errorQueue);
    GlobalState(std::shared_ptr<ErrorQueue> errorQueue, std::shared_ptr<lsp::TypecheckEpochManager> epochManager);

    // Creates an empty global state for hashing. Bypasses important sanity checks that are used for other types of
    // global states.
    static std::unique_ptr<GlobalState> makeEmptyGlobalStateForHashing(spdlog::logger &logger);

    // Empirically determined to be the smallest powers of two larger than the
    // values required by the payload. Enforced in payload.cc.
    static constexpr unsigned int PAYLOAD_MAX_UTF8_NAME_COUNT = 16384;
    static constexpr unsigned int PAYLOAD_MAX_CONSTANT_NAME_COUNT = 4096;
    static constexpr unsigned int PAYLOAD_MAX_UNIQUE_NAME_COUNT = 4096;
    static constexpr unsigned int PAYLOAD_MAX_CLASS_AND_MODULE_COUNT = 8192;
    static constexpr unsigned int PAYLOAD_MAX_METHOD_COUNT = 32768;
    static constexpr unsigned int PAYLOAD_MAX_FIELD_COUNT = 4096;
    static constexpr unsigned int PAYLOAD_MAX_TYPE_ARGUMENT_COUNT = 256;
    static constexpr unsigned int PAYLOAD_MAX_TYPE_MEMBER_COUNT = 4096;

    void initEmpty();
    void installIntrinsics();
    void computeLinearization();

    // Expand symbol and name tables to the given lengths. Does nothing if the value is <= current capacity.
    void preallocateTables(uint32_t classAndModulesSize, uint32_t methodsSize, uint32_t fieldsSize,
                           uint32_t typeParametersSize, uint32_t typeMembersSize, uint32_t utf8NameSize,
                           uint32_t constantNameSize, uint32_t uniqueNameSize);

    GlobalState(const GlobalState &) = delete;
    GlobalState(GlobalState &&) = delete;

    ~GlobalState() = default;

    ClassOrModuleRef enterClassSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    TypeMemberRef enterTypeMember(Loc loc, ClassOrModuleRef owner, NameRef name, Variance variance);
    TypeParameterRef enterTypeParameter(Loc loc, MethodRef owner, NameRef name, Variance variance);
    MethodRef enterMethodSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    MethodRef enterNewMethodOverload(Loc loc, MethodRef original, core::NameRef originalName, uint32_t num,
                                     const std::vector<bool> &paramsToKeep);
    FieldRef enterFieldSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    FieldRef enterStaticFieldSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    ParamInfo &enterMethodParameter(Loc loc, MethodRef owner, NameRef name);

    SymbolRef lookupSymbol(ClassOrModuleRef owner, NameRef name) const {
        return lookupSymbolWithKind(owner, name, SymbolRef::Kind::ClassOrModule, Symbols::noSymbol(),
                                    /* ignoreKind */ true);
    }
    TypeMemberRef lookupTypeMemberSymbol(ClassOrModuleRef owner, NameRef name) const {
        return lookupSymbolWithKind(owner, name, SymbolRef::Kind::TypeMember, Symbols::noTypeMember())
            .asTypeMemberRef();
    }
    ClassOrModuleRef lookupClassSymbol(ClassOrModuleRef owner, NameRef name) const {
        return lookupSymbolWithKind(owner, name, SymbolRef::Kind::ClassOrModule, Symbols::noClassOrModule())
            .asClassOrModuleRef();
    }
    MethodRef lookupMethodSymbol(ClassOrModuleRef owner, NameRef name) const {
        return lookupSymbolWithKind(owner, name, SymbolRef::Kind::Method, Symbols::noMethod()).asMethodRef();
    }
    MethodRef lookupMethodSymbolWithHash(ClassOrModuleRef owner, NameRef name, ArityHash arityHash) const;
    FieldRef lookupStaticFieldSymbol(ClassOrModuleRef owner, NameRef name) const {
        // N.B.: Fields and static fields have entirely different types of names, so this should be unambiguous.
        return lookupSymbolWithKind(owner, name, SymbolRef::Kind::FieldOrStaticField, Symbols::noField()).asFieldRef();
    }
    FieldRef lookupFieldSymbol(ClassOrModuleRef owner, NameRef name) const {
        // N.B.: Fields and static fields have entirely different types of names, so this should be unambiguous.
        return lookupSymbolWithKind(owner, name, SymbolRef::Kind::FieldOrStaticField, Symbols::noField()).asFieldRef();
    }
    SymbolRef findRenamedSymbol(ClassOrModuleRef owner, SymbolRef name) const;

    MethodRef staticInitForFile(Loc loc);
    MethodRef staticInitForClass(ClassOrModuleRef klass, Loc loc);

    MethodRef lookupStaticInitForFile(FileRef file) const;
    MethodRef lookupStaticInitForClass(ClassOrModuleRef klass, bool allowMissing = false) const;

    NameRef enterNameUTF8(std::string_view nm);
    NameRef lookupNameUTF8(std::string_view nm) const;

    NameRef lookupNameUnique(UniqueNameKind uniqueNameKind, NameRef original, uint32_t num) const;
    NameRef freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, uint32_t num);

    NameRef enterNameConstant(NameRef original);
    NameRef enterNameConstant(std::string_view original);
    NameRef lookupNameConstant(NameRef original) const;
    NameRef lookupNameConstant(std::string_view original) const;

    FileRef enterFile(std::string_view path, std::string_view source);
    FileRef enterFile(std::shared_ptr<File> file);
    FileRef reserveFileRef(std::string path);
    std::shared_ptr<File> replaceFile(FileRef whatFile, std::shared_ptr<File> withWhat);
    static std::unique_ptr<GlobalState> markFileAsTombStone(std::unique_ptr<GlobalState>, FileRef fref);
    FileRef findFileByPath(std::string_view path) const {
        return this->files->findFileByPath(path);
    }

    const packages::PackageDB &packageDB() const;
    packages::PackageDB &packageDB();
    void setPackagerOptions(const std::vector<std::string> &extraPackageFilesDirectoryUnderscorePrefixes,
                            const std::vector<std::string> &extraPackageFilesDirectorySlashDeprecatedPrefixes,
                            const std::vector<std::string> &extraPackageFilesDirectorySlashPrefixes,
                            const std::vector<std::string> &packageSkipRBIExportEnforcementDirs,
                            const std::vector<std::string> &skipImportVisibilityCheckFor,
                            const std::vector<std::string> &packagerLayers, std::string errorHint, bool genPackages);
    packages::UnfreezePackages unfreezePackages();

    NameRef nextMangledName(ClassOrModuleRef owner, NameRef origName);
    void mangleRenameMethod(MethodRef what, NameRef origName);
    // NOTE: You likely want to use mangleRenameMethod not deleteMethodSymbol, unless you know what you're doing.
    // See the comment on the implementation for more.
    void deleteMethodSymbol(MethodRef what);
    // NOTE: You likely want to use mangleRenameMethod not deleteFieldSymbol, unless you know what you're doing.
    // See the comment on the implementation for more.
    void deleteFieldSymbol(FieldRef what);
    // NOTE: You likely want to use mangleRenameSymbol not deleteTypeMemberSymbol, unless you know what you're doing.
    // See the comment on the implementation for more.
    void deleteTypeMemberSymbol(TypeMemberRef what);
    spdlog::logger &tracer() const;
    unsigned int namesUsedTotal() const;
    unsigned int utf8NamesUsed() const;
    unsigned int constantNamesUsed() const;
    unsigned int uniqueNamesUsed() const;

    unsigned int classAndModulesUsed() const;
    unsigned int methodsUsed() const;
    unsigned int fieldsUsed() const;
    unsigned int typeParametersUsed() const;
    unsigned int typeMembersUsed() const;
    unsigned int filesUsed() const;
    unsigned int symbolsUsedTotal() const;

    void sanityCheckTableSizes() const;
    void sanityCheckNames() const;
    void sanityCheck() const;
    void markAsPayload();

    // These methods are here to make it easier to print the symbol table in lldb.
    // (don't have to remember the default args)
    std::string toString() const {
        bool showFull = false;
        bool showRaw = false;
        return toStringWithOptions(showFull, showRaw);
    }
    std::string toStringFull() const {
        bool showFull = true;
        bool showRaw = false;
        return toStringWithOptions(showFull, showRaw);
    }
    std::string showRaw() const {
        bool showFull = false;
        bool showRaw = true;
        return toStringWithOptions(showFull, showRaw);
    }
    std::string showRawFull() const {
        bool showFull = true;
        bool showRaw = true;
        return toStringWithOptions(showFull, showRaw);
    }

    bool hadCriticalError() const;

    ErrorBuilder beginError(Loc loc, ErrorClass what) const;
    void _error(std::unique_ptr<Error> error) const;

    // A version of `beginError` that's specific to the index phase of the pipeline, as it will record that index errors
    // have been seen on the file associated with the loc.
    ErrorBuilder beginIndexerError(Loc loc, ErrorClass what);

    int totalErrors() const;
    bool wasNameTableModified() const;

    int globalStateId;
    bool silenceErrors = false;
    bool autocorrect = false;
    bool didYouMean = true;
    TrackUntyped trackUntyped = TrackUntyped::Nowhere;

    /**
     * The severity to use when reporting untyped highlights.
     *
     * Must be stored on GlobalState (vs LSPClientConfiguration) because LSPClientConfiguration is
     * immutable after initialization.
     *
     * The fact that we report untyped code as "diagnostic" is mostly a limitation of the LSP spec.
     * There's no real way to communicate passive information about individual regions of code
     * except by either defining "semantic tokens" (for syntax highlighting) or diagnostics.
     *
     * VS Code is special (like it always is) and makes it hard to customize how diagnostics are
     * presented. In particular, marking a diagnostic as "Information" will make it show up in the
     * Problems view, while "Hint" diagnostics will not show up there. (Hint diagnostics have their
     * own problems: it's hard to customize the style of them in the editor). But in any case, we
     * can let users choose which side of these tradeoffs they want to come down on.
     */
    lsp::DiagnosticSeverity highlightUntypedDiagnosticSeverity = lsp::DiagnosticSeverity::Information;

    bool printingFileTable = false;

    // We have a lot of internal names of form `<something>` that's chosen with `<` and `>` as you can't make
    // this into a valid ruby identifier without suffering.
    // We want to make sure we don't round-trip through strings for those names.
    //
    // If this attribute is set to `true`, all strings will be checked for `<` and `>` characters in them.
    bool ensureCleanStrings = false;
    bool censorForSnapshotTests = false;

    std::optional<int> sleepInSlowPathSeconds = std::nullopt;

    std::unique_ptr<GlobalState> deepCopyGlobalState(bool keepId = false) const;
    mutable std::shared_ptr<ErrorQueue> errorQueue;

    // Minimally copy the global state and share the file table with the original. This is meant only for use on threads
    // in the indexing pass, as the global states produced will be short-lived and the files used between threads will
    // have no overlap.
    // NOTE: this very intentionally will not copy the symbol or name tables. The symbol tables aren't used or populated
    // during indexing, and the name tables will only be written to.
    std::unique_ptr<GlobalState>
    copyForIndexThread(const bool packagerEnabled,
                       const std::vector<std::string> &extraPackageFilesDirectoryUnderscorePrefixes,
                       const std::vector<std::string> &extraPackageFilesDirectorySlashDeprecatedPrefixes,
                       const std::vector<std::string> &extraPackageFilesDirectorySlashPrefixes,
                       const std::vector<std::string> &packageSkipRBIExportEnforcementDirs,
                       const std::vector<std::string> &allowRelaxedPackagerChecksFor,
                       const std::vector<std::string> &packagerLayers, std::string errorHint, bool genPackages) const;

    // Minimally copy the global state, including the file table, to initialize the LSPTypechecker.
    // NOTE: this very intentionally will not copy the symbol or name tables. The symbol tables aren't used or populated
    // during indexing, and the name tables will only be written to.
    std::unique_ptr<GlobalState> copyForLSPTypechecker(
        const bool packagerEnabled, const std::vector<std::string> &extraPackageFilesDirectoryUnderscorePrefixes,
        const std::vector<std::string> &extraPackageFilesDirectorySlashDeprecatedPrefixes,
        const std::vector<std::string> &extraPackageFilesDirectorySlashPrefixes,
        const std::vector<std::string> &packageSkipRBIExportEnforcementDirs,
        const std::vector<std::string> &allowRelaxedPackagerChecksFor, const std::vector<std::string> &packagerLayers,
        std::string errorHint, bool genPackages) const;

    // Copy the name table, file table and other parts of GlobalState that are required to start the slow path.
    // NOTE: this very intentionally will not copy the symbol table, and the expectation is that the symbol table will
    // be overwritten by immediately deserializaing the payload over it.
    std::unique_ptr<GlobalState>
    copyForSlowPath(const std::vector<std::string> &extraPackageFilesDirectoryUnderscorePrefixes,
                    const std::vector<std::string> &extraPackageFilesDirectorySlashDeprecatedPrefixes,
                    const std::vector<std::string> &extraPackageFilesDirectorySlashPrefixes,
                    const std::vector<std::string> &packageSkipRBIExportEnforcementDirs,
                    const std::vector<std::string> &allowRelaxedPackagerChecksFor,
                    const std::vector<std::string> &packagerLayers, std::string errorHint, bool genPackages) const;

    // Contains a path prefix that should be stripped from all printed paths.
    std::string pathPrefix;
    // Returns a string_view of the given path with the path prefix removed.
    std::string_view getPrintablePath(std::string_view path) const;

    // Contains a location / symbol / variable reference that various Sorbet passes are looking for.
    // See ErrorQueue#queryResponse
    lsp::Query lspQuery;

    // Stores a UUID that uniquely identifies this GlobalState in kvstore.
    uint32_t kvstoreUuid = 0;

    FlowId creation; // used to track flow of global states

    // Indicates the number of times LSP has run the type checker with this global state.
    // Used to ensure GlobalState is in the correct state to process requests.
    unsigned int lspTypecheckCount = 0;
    // [LSP] Manages typechecking epochs and cancelation.
    std::shared_ptr<lsp::TypecheckEpochManager> epochManager;

    void trace(std::string_view msg) const;

    std::unique_ptr<LocalSymbolTableHashes> hash(uint32_t foundClassesHash) const;
    absl::Span<const std::shared_ptr<File>> getFiles() const {
        return this->files->span();
    }

    // Contains a string to be used as the base of the error URL.
    // The error code is appended to this string.
    std::string errorUrlBase;

    // If 'true', print error sections
    bool includeErrorSections = true;

    // If 'true', enforce use of Ruby 3.0-style keyword args.
    bool ruby3KeywordArgs = false;

    // If 'true', use the Prism parser.
    bool parseWithPrism = false;

    // Some options change the behavior of things that might be cached, including ASTs, the file
    // table, the name table, the symbol table, etc.
    //
    // If these options change, it's no longer safe to read things out of the cache.
    // For example, `typedSuper` controls how Ruby's `super` keyword is desugared, and the
    // result of desugar (where `typedSuper` is used) is cached.
    struct CacheSensitiveOptions {
        // If 'true', attempt to typecheck calls to `super` as often as possible.
        // Some calls to `super` are not type checked due to incomplete/imperfect information.
        bool typedSuper = true;

        // Whether to parse RBS-style type annotations and assertions out of comments
        bool rbsEnabled = false;

        // Whether to allow `requires_ancestor`, which allows modeling certain kinds of indirect
        // inheritance hierarchies, at the expense of implementation complexity and soundness
        // problems.
        bool requiresAncestorEnabled = false;

        // Whether to run the RSpec-specific portions of the Minitest rewriter, in a best-effort
        // attempt to support RSpec.
        bool rspecRewriterEnabled = false;

        // So we can know whether we're running in autogen mode.
        // Right now this is only used to turn certain Rewriter passes on or off.
        // Think very hard before looking at this value in namer / resolver!
        // (hint: probably you want to find an alternate solution)
        bool runningUnderAutogen = false;
    };
    CacheSensitiveOptions cacheSensitiveOptions;

    std::vector<std::string> suppressPayloadSuperclassRedefinitionFor;

    void ignoreErrorClassForSuggestTyped(int code);
    void suppressErrorClass(int code);
    void onlyShowErrorClass(int code);

    std::optional<std::string> suggestUnsafe;

    std::vector<std::unique_ptr<pipeline::semantic_extension::SemanticExtension>> semanticExtensions;

    bool shouldReportErrorOn(FileRef file, ErrorClass what) const;

private:
    struct DeepCloneHistoryEntry {
        int globalStateId;
        unsigned int lastUTF8NameKnownByParentGlobalState;
        unsigned int lastConstantNameKnownByParentGlobalState;
        unsigned int lastUniqueNameKnownByParentGlobalState;
    };
    std::vector<DeepCloneHistoryEntry> deepCloneHistory;

    static constexpr int STRINGS_PAGE_SIZE = 4096;
    StableStringStorage<STRINGS_PAGE_SIZE> strings;
    std::string_view enterString(std::string_view nm);
    std::vector<UTF8Name> utf8Names;
    std::vector<ConstantName> constantNames;
    std::vector<UniqueName> uniqueNames;
    std::vector<ClassOrModule> classAndModules;
    std::vector<Method> methods;
    std::vector<Field> fields;
    std::vector<TypeParameter> typeMembers;
    std::vector<TypeParameter> typeParameters;
    NameHash namesByHash;
    UnorderedSet<int> ignoredForSuggestTypedErrorClasses;
    UnorderedSet<int> suppressedErrorClasses;
    UnorderedSet<int> onlyErrorClasses;
    std::shared_ptr<FileTable> files;
    bool wasNameTableModified_ = false;

    core::packages::PackageDB packageDB_;

    bool freezeSymbolTable();
    bool freezeNameTable();
    bool freezeFileTable();
    bool unfreezeSymbolTable();
    bool unfreezeNameTable();
    bool unfreezeFileTable();
    bool nameTableFrozen = true;
    bool symbolTableFrozen = true;
    bool fileTableFrozen = true;

    // Copy options over from another GlobalState. Private, as it's only meant to be used as a helper to implement other
    // copying strategies.
    void copyOptions(const GlobalState &other);

    void expandNames(uint32_t utf8NameSize, uint32_t constantNameSize, uint32_t uniqueNameSize);

    ClassOrModuleRef synthesizeClass(NameRef nameID, uint32_t superclass = Symbols::todo().id(), bool isModule = false);

    SymbolRef lookupSymbolWithKind(ClassOrModuleRef owner, NameRef name, SymbolRef::Kind kind,
                                   SymbolRef defaultReturnValue, bool ignoreKind = false) const;

    std::string toStringWithOptions(bool showFull, bool showRaw) const;
};
// CheckSize(GlobalState, 152, 8);
// Historically commented out because size of unordered_map was different between different versions of stdlib

} // namespace sorbet::core

#endif // SORBET_GLOBAL_STATE_H
