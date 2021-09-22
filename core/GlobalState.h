#ifndef SORBET_GLOBAL_STATE_H
#define SORBET_GLOBAL_STATE_H
#include "absl/synchronization/mutex.h"

#include "core/Error.h"
#include "core/Files.h"
#include "core/Loc.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/lsp/Query.h"
#include "core/packages/PackageInfo.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include <memory>

namespace sorbet::packager {
class PackagerImpl;
} // namespace sorbet::packager

namespace sorbet::core {

class NameRef;
class Symbol;
class SymbolRef;
class ClassOrModuleRef;
class MethodRef;
class FieldRef;
class TypeArgumentRef;
class TypeMemberRef;
class GlobalSubstitution;
class ErrorQueue;
struct GlobalStateHash;

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
    friend Symbol;
    friend SymbolRef;
    friend ClassOrModuleRef;
    friend MethodRef;
    friend TypeMemberRef;
    friend TypeArgumentRef;
    friend FieldRef;
    friend File;
    friend FileRef;
    friend GlobalSubstitution;
    friend ErrorBuilder;
    friend serialize::Serializer;
    friend serialize::SerializerImpl;
    friend class UnfreezeNameTable;
    friend class UnfreezeSymbolTable;
    friend class UnfreezeFileTable;
    friend struct NameRefDebugCheck;
    friend class sorbet::packager::PackagerImpl;

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

    // Expand symbol and name tables to the given lengths. Does nothing if the value is <= current capacity.
    void preallocateTables(u4 classAndModulesSize, u4 methodsSize, u4 fieldsSize, u4 typeArgumentsSize,
                           u4 typeMembersSize, u4 utf8NameSize, u4 constantNameSize, u4 uniqueNameSize);

    GlobalState(const GlobalState &) = delete;
    GlobalState(GlobalState &&) = delete;

    ~GlobalState() = default;

    ClassOrModuleRef enterClassSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    TypeMemberRef enterTypeMember(Loc loc, ClassOrModuleRef owner, NameRef name, Variance variance);
    TypeArgumentRef enterTypeArgument(Loc loc, MethodRef owner, NameRef name, Variance variance);
    MethodRef enterMethodSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    MethodRef enterNewMethodOverload(Loc loc, MethodRef original, core::NameRef originalName, u4 num,
                                     const std::vector<bool> &argsToKeep);
    FieldRef enterFieldSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    FieldRef enterStaticFieldSymbol(Loc loc, ClassOrModuleRef owner, NameRef name);
    ArgInfo &enterMethodArgumentSymbol(Loc loc, MethodRef owner, NameRef name);

    SymbolRef lookupSymbol(SymbolRef owner, NameRef name) const {
        return lookupSymbolWithFlags(owner, name, 0, Symbols::noSymbol());
    }
    TypeMemberRef lookupTypeMemberSymbol(ClassOrModuleRef owner, NameRef name) const {
        return lookupSymbolWithFlags(owner, name, Symbol::Flags::TYPE_MEMBER, Symbols::noTypeMember())
            .asTypeMemberRef();
    }
    ClassOrModuleRef lookupClassSymbol(ClassOrModuleRef owner, NameRef name) const {
        return lookupSymbolWithFlags(owner, name, Symbol::Flags::CLASS_OR_MODULE, Symbols::noClassOrModule())
            .asClassOrModuleRef();
    }
    MethodRef lookupMethodSymbol(ClassOrModuleRef owner, NameRef name) const {
        return lookupSymbolWithFlags(owner, name, Symbol::Flags::METHOD, Symbols::noMethod()).asMethodRef();
    }
    MethodRef lookupMethodSymbolWithHash(ClassOrModuleRef owner, NameRef name, const std::vector<u4> &methodHash) const;
    FieldRef lookupStaticFieldSymbol(ClassOrModuleRef owner, NameRef name) const {
        return lookupSymbolWithFlags(owner, name, Symbol::Flags::STATIC_FIELD, Symbols::noField()).asFieldRef();
    }
    FieldRef lookupFieldSymbol(ClassOrModuleRef owner, NameRef name) const {
        return lookupSymbolWithFlags(owner, name, Symbol::Flags::FIELD, Symbols::noField()).asFieldRef();
    }
    SymbolRef findRenamedSymbol(SymbolRef owner, SymbolRef name) const;

    MethodRef staticInitForFile(Loc loc);
    MethodRef staticInitForClass(ClassOrModuleRef klass, Loc loc);

    MethodRef lookupStaticInitForFile(Loc loc) const;
    MethodRef lookupStaticInitForClass(ClassOrModuleRef klass) const;

    NameRef enterNameUTF8(std::string_view nm);
    NameRef lookupNameUTF8(std::string_view nm) const;

    NameRef lookupNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u4 num) const;
    NameRef freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u4 num);

    NameRef enterNameConstant(NameRef original);
    NameRef enterNameConstant(std::string_view original);
    NameRef lookupNameConstant(NameRef original) const;
    NameRef lookupNameConstant(std::string_view original) const;

    FileRef enterFile(std::string_view path, std::string_view source);
    FileRef enterFile(const std::shared_ptr<File> &file);
    FileRef enterNewFileAt(const std::shared_ptr<File> &file, FileRef id);
    FileRef reserveFileRef(std::string path);
    static std::unique_ptr<GlobalState> replaceFile(std::unique_ptr<GlobalState> inWhat, FileRef whatFile,
                                                    const std::shared_ptr<File> &withWhat);
    static std::unique_ptr<GlobalState> markFileAsTombStone(std::unique_ptr<GlobalState>, FileRef fref);
    FileRef findFileByPath(std::string_view path) const;

    NameRef enterPackage(std::unique_ptr<packages::PackageInfo> pkg);
    NameRef lookupPackage(NameRef pkgMangledName) const;

    void mangleRenameSymbol(SymbolRef what, NameRef origName);
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

    bool sleepInSlowPath = false;

    std::unique_ptr<GlobalState> deepCopy(bool keepId = false) const;
    mutable std::shared_ptr<ErrorQueue> errorQueue;

    // Contains a path prefix that should be stripped from all printed paths.
    std::string pathPrefix;
    // Returns a string_view of the given path with the path prefix removed.
    std::string_view getPrintablePath(std::string_view path) const;

    // Contains a location / symbol / variable reference that various Sorbet passes are looking for.
    // See ErrorQueue#queryResponse
    lsp::Query lspQuery;

    // Stores a UUID that uniquely identifies this GlobalState in kvstore.
    u4 kvstoreUuid = 0;

    FlowId creation; // used to track flow of global states

    // Indicates the number of times LSP has run the type checker with this global state.
    // Used to ensure GlobalState is in the correct state to process requests.
    unsigned int lspTypecheckCount = 0;
    // [LSP] Manages typechecking epochs and cancelation.
    std::shared_ptr<lsp::TypecheckEpochManager> epochManager;

    void trace(std::string_view msg) const;

    std::unique_ptr<GlobalStateHash> hash() const;
    const std::vector<std::shared_ptr<File>> &getFiles() const;

    // Contains a string to be used as the base of the error URL.
    // The error code is appended to this string.
    std::string errorUrlBase;

    // If 'true', print error sections
    bool includeErrorSections = true;

    // If 'true', enforce use of Ruby 3.0-style keyword args.
    bool ruby3KeywordArgs = false;

    void ignoreErrorClassForSuggestTyped(int code);
    void suppressErrorClass(int code);
    void onlyShowErrorClass(int code);

    std::optional<std::string> suggestUnsafe;

    std::vector<std::unique_ptr<pipeline::semantic_extension::SemanticExtension>> semanticExtensions;

    bool requiresAncestorEnabled = false;

private:
    bool shouldReportErrorOn(Loc loc, ErrorClass what) const;
    struct DeepCloneHistoryEntry {
        int globalStateId;
        unsigned int lastUTF8NameKnownByParentGlobalState;
        unsigned int lastConstantNameKnownByParentGlobalState;
        unsigned int lastUniqueNameKnownByParentGlobalState;
    };
    std::vector<DeepCloneHistoryEntry> deepCloneHistory;

    static constexpr int STRINGS_PAGE_SIZE = 4096;
    std::vector<std::shared_ptr<std::vector<char>>> strings;
    std::string_view enterString(std::string_view nm);
    u2 stringsLastPageUsed = STRINGS_PAGE_SIZE + 1;
    std::vector<UTF8Name> utf8Names;
    std::vector<ConstantName> constantNames;
    std::vector<UniqueName> uniqueNames;
    UnorderedMap<std::string, FileRef> fileRefByPath;
    std::vector<Symbol> classAndModules;
    std::vector<Symbol> methods;
    std::vector<Symbol> fields;
    std::vector<Symbol> typeMembers;
    std::vector<Symbol> typeArguments;
    std::vector<std::pair<unsigned int, u4>> namesByHash;
    std::vector<std::shared_ptr<File>> files;
    UnorderedSet<int> ignoredForSuggestTypedErrorClasses;
    UnorderedSet<int> suppressedErrorClasses;
    UnorderedSet<int> onlyErrorClasses;
    bool wasModified_ = false;

    UnorderedMap<NameRef, std::unique_ptr<packages::PackageInfo>> packages;
    UnorderedMap<std::string, NameRef> packagesByPathPrefix;

    bool freezeSymbolTable();
    bool freezeNameTable();
    bool freezeFileTable();
    bool unfreezeSymbolTable();
    bool unfreezeNameTable();
    bool unfreezeFileTable();
    bool nameTableFrozen = true;
    bool symbolTableFrozen = true;
    bool fileTableFrozen = true;

    void expandNames(u4 utf8NameSize, u4 constantNameSize, u4 uniqueNameSize);

    ClassOrModuleRef synthesizeClass(NameRef nameID, u4 superclass = Symbols::todo().id(), bool isModule = false);

    SymbolRef lookupSymbolWithFlags(SymbolRef owner, NameRef name, u4 flags, SymbolRef defaultReturnValue) const;

    std::string toStringWithOptions(bool showFull, bool showRaw) const;
};
// CheckSize(GlobalState, 152, 8);
// Historically commented out because size of unordered_map was different between different versions of stdlib

} // namespace sorbet::core

#endif // SORBET_GLOBAL_STATE_H
