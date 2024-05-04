#ifndef SORBET_GLOBAL_STATE_H
#define SORBET_GLOBAL_STATE_H
#include "absl/synchronization/mutex.h"

#include "common/StableStringStorage.h"
#include "core/Error.h"
#include "core/ErrorQueueMessage.h"
#include "core/Files.h"
#include "core/Loc.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TrackUntyped.h"
#include "core/lsp/Query.h"
#include "core/packages/PackageDB.h"
#include "core/packages/PackageInfo.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include <memory>

namespace sorbet::core {

class NameRef;
class ClassOrModule;
class SymbolRef;
class ClassOrModuleRef;
class MethodRef;
class FieldRef;
class TypeArgumentRef;
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
    friend TypeArgumentRef;
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
                int globalStateId);

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
                           uint32_t typeArgumentsSize, uint32_t typeMembersSize, uint32_t utf8NameSize,
                           uint32_t constantNameSize, uint32_t uniqueNameSize);

    GlobalState(const GlobalState &) = delete;
    GlobalState(GlobalState &&) = delete;

    ~GlobalState() = default;

    ClassOrModuleRef enterClassSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    TypeMemberRef enterTypeMember(Loc loc, ClassOrModuleRef owner, NameRef name, Variance variance);
    TypeArgumentRef enterTypeArgument(Loc loc, MethodRef owner, NameRef name, Variance variance);
    MethodRef enterMethodSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    MethodRef enterNewMethodOverload(Loc loc, MethodRef original, core::NameRef originalName, uint32_t num,
                                     const std::vector<bool> &argsToKeep);
    FieldRef enterFieldSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    FieldRef enterStaticFieldSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    ArgInfo &enterMethodArgumentSymbol(Loc loc, MethodRef owner, NameRef name);

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
    FileRef enterFile(const std::shared_ptr<File> &file);
    FileRef enterNewFileAt(const std::shared_ptr<File> &file, FileRef id);
    FileRef reserveFileRef(std::string path);
    void replaceFile(FileRef whatFile, const std::shared_ptr<File> &withWhat);
    static std::unique_ptr<GlobalState> markFileAsTombStone(std::unique_ptr<GlobalState>, FileRef fref);
    FileRef findFileByPath(std::string_view path) const;

    const packages::PackageDB &packageDB() const;
    void setPackagerOptions(const std::vector<std::string> &extraPackageFilesDirectoryUnderscorePrefixes,
                            const std::vector<std::string> &extraPackageFilesDirectorySlashPrefixes,
                            const std::vector<std::string> &packageSkipRBIExportEnforcementDirs,
                            const std::vector<std::string> &skipImportVisibilityCheckFor, std::string errorHint);
    packages::UnfreezePackages unfreezePackages();

    NameRef nextMangledName(ClassOrModuleRef owner, NameRef origName);
    void mangleRenameMethod(MethodRef what, NameRef origName);
    void mangleRenameForOverload(MethodRef what, NameRef origName);
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
    unsigned int typeArgumentsUsed() const;
    unsigned int typeMembersUsed() const;
    unsigned int filesUsed() const;
    unsigned int symbolsUsedTotal() const;

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

    int totalErrors() const;
    bool wasModified() const;

    int globalStateId;
    bool silenceErrors = false;
    bool autocorrect = false;
    bool didYouMean = true;
    TrackUntyped trackUntyped = TrackUntyped::Nowhere;
    bool printingFileTable = false;

    // We have a lot of internal names of form `<something>` that's chosen with `<` and `>` as you can't make
    // this into a valid ruby identifier without suffering.
    // We want to make sure we don't round-trip through strings for those names.
    //
    // If this attribute is set to `true`, all strings will be checked for `<` and `>` characters in them.
    bool ensureCleanStrings = false;

    // So we can know whether we're running in autogen mode.
    // Right now this is only used to turn certain Rewriter passes on or off.
    // Think very hard before looking at this value in namer / resolver!
    // (hint: probably you want to find an alternate solution)
    bool runningUnderAutogen = false;
    bool censorForSnapshotTests = false;

    std::optional<int> sleepInSlowPathSeconds = std::nullopt;

    std::unique_ptr<GlobalState> deepCopy(bool keepId = false) const;
    mutable std::shared_ptr<ErrorQueue> errorQueue;

    // Copy the file table and other parts of GlobalState that are required for the indexing pass.
    // NOTE: this very intentionally will not copy the symbol or name tables. The symbol tables aren't used or populated
    // during indexing, and the name tables will only be written to.
    std::unique_ptr<GlobalState> copyForIndex() const;

    // Merge the contents of one file table into this GlobalState. This is used during the index pass to make sure that
    // changes made to the file table in worker threads are propagated back to the main GlobalState.
    void mergeFileTable(const core::GlobalState &gs);

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

    std::unique_ptr<LocalSymbolTableHashes> hash() const;
    const std::vector<std::shared_ptr<File>> &getFiles() const;

    // Contains a string to be used as the base of the error URL.
    // The error code is appended to this string.
    std::string errorUrlBase;

    // If 'true', print error sections
    bool includeErrorSections = true;

    // If 'true', enforce use of Ruby 3.0-style keyword args.
    bool ruby3KeywordArgs = false;

    // If 'true', attempt to typecheck calls to `super` as often as possible.
    // Some calls to `super` are not type checked due to incomplete/imperfect information.
    bool typedSuper = true;

    std::vector<std::string> suppressPayloadSuperclassRedefinitionFor;

    // When present, this indicates that single-package rbi generation is being performed, and contains metadata about
    // the packages that are imported by the one whose interface is being generated.
    std::optional<packages::ImportInfo> singlePackageImports;

    void ignoreErrorClassForSuggestTyped(int code);
    void suppressErrorClass(int code);
    void onlyShowErrorClass(int code);

    std::optional<std::string> suggestUnsafe;

    std::vector<std::unique_ptr<pipeline::semantic_extension::SemanticExtension>> semanticExtensions;

    bool requiresAncestorEnabled = false;

    bool shouldReportErrorOn(Loc loc, ErrorClass what) const;

    void clearErrorCacheForFile(core::FileRef fref,
                                std::function<bool(const std::unique_ptr<core::ErrorQueueMessage> &)> predicate);
    UnorderedMap<core::FileRef, std::vector<std::unique_ptr<core::ErrorQueueMessage>>> errors;

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
    UnorderedMap<std::string, FileRef> fileRefByPath;
    std::vector<ClassOrModule> classAndModules;
    std::vector<Method> methods;
    std::vector<Field> fields;
    std::vector<TypeParameter> typeMembers;
    std::vector<TypeParameter> typeArguments;
    struct Bucket {
        unsigned int hash;
        uint32_t rawId;
    };
    std::vector<Bucket> namesByHash;
    std::vector<std::shared_ptr<File>> files;
    UnorderedSet<int> ignoredForSuggestTypedErrorClasses;
    UnorderedSet<int> suppressedErrorClasses;
    UnorderedSet<int> onlyErrorClasses;
    bool wasModified_ = false;

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

    void expandNames(uint32_t utf8NameSize, uint32_t constantNameSize, uint32_t uniqueNameSize);
    void moveNames(Bucket *from, Bucket *to, unsigned int szFrom, unsigned int szTo);

    ClassOrModuleRef synthesizeClass(NameRef nameID, uint32_t superclass = Symbols::todo().id(), bool isModule = false);

    SymbolRef lookupSymbolWithKind(ClassOrModuleRef owner, NameRef name, SymbolRef::Kind kind,
                                   SymbolRef defaultReturnValue, bool ignoreKind = false) const;

    void mangleRenameMethodInternal(MethodRef what, NameRef origName, UniqueNameKind kind);

    std::string toStringWithOptions(bool showFull, bool showRaw) const;
};
// CheckSize(GlobalState, 152, 8);
// Historically commented out because size of unordered_map was different between different versions of stdlib

} // namespace sorbet::core

#endif // SORBET_GLOBAL_STATE_H
