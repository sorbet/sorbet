#ifndef SORBET_GLOBAL_STATE_H
#define SORBET_GLOBAL_STATE_H
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/Files.h"
#include "core/Hashing.h"
#include "core/Loc.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include <memory>
#include <mutex>

namespace sorbet::core {

class Name;
class NameRef;
class Symbol;
class SymbolRef;
class GlobalSubstitution;
class ErrorRegion;
class ErrorQueue;

namespace serialize {
class Serializer;
class SerializerImpl;
} // namespace serialize

class GlobalState final {
    friend Name;
    friend NameRef;
    friend Symbol;
    friend SymbolRef;
    friend File;
    friend FileRef;
    friend GlobalSubstitution;
    friend ErrorRegion;
    friend ErrorBuilder;
    friend serialize::Serializer;
    friend serialize::SerializerImpl;
    friend class UnfreezeNameTable;
    friend class UnfreezeSymbolTable;
    friend class UnfreezeFileTable;

public:
    GlobalState(const std::shared_ptr<ErrorQueue> &errorQueue);

    void initEmpty();
    void installIntrinsics();

    // Expand tables to use approximate `kb` KiB of memory. Can be used prior to
    // operation to avoid table resizes.
    void reserveMemory(u4 kb);

    GlobalState(const GlobalState &) = delete;
    GlobalState(GlobalState &&) = delete;

    ~GlobalState() = default;

    SymbolRef enterClassSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterTypeMember(Loc loc, SymbolRef owner, NameRef name, Variance variance);
    SymbolRef enterTypeArgument(Loc loc, SymbolRef owner, NameRef name, Variance variance);
    SymbolRef enterMethodSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterNewMethodOverload(Loc loc, SymbolRef original, u2 num);
    SymbolRef enterFieldSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterStaticFieldSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterMethodArgumentSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef staticInitForFile(FileRef file);

    NameRef enterNameUTF8(std::string_view nm);

    NameRef getNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num) const;
    NameRef freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num);

    NameRef enterNameConstant(NameRef original);
    NameRef enterNameConstant(std::string_view original);

    FileRef enterFile(std::string_view path, std::string_view source);
    FileRef enterFileAt(std::string_view path, std::string_view source, FileRef id);
    FileRef enterFile(const std::shared_ptr<File> &file);
    FileRef enterNewFileAt(const std::shared_ptr<File> &file, FileRef id);
    FileRef reserveFileRef(std::string path);
    static std::unique_ptr<GlobalState> replaceFile(std::unique_ptr<GlobalState> inWhat, FileRef whatFile,
                                                    const std::shared_ptr<File> &withWhat);
    static std::unique_ptr<GlobalState> markFileAsTombStone(std::unique_ptr<GlobalState>, FileRef fref);
    FileRef findFileByPath(std::string_view path);

    void mangleRenameSymbol(SymbolRef what, NameRef origName, UniqueNameKind kind);

    unsigned int namesUsed() const;

    unsigned int symbolsUsed() const;
    unsigned int filesUsed() const;

    void sanityCheck() const;
    void markAsPayload();

    std::string toString(bool showHidden = false) const;
    std::string showAnnotatedSource(FileRef file) const;
    enum AnnotationPos { BEFORE, AFTER };
    struct Annotation {
        Loc loc;
        std::string str;
        AnnotationPos pos;
        u4 blockId; // used to define order in case `loc`s are the same
    };
    void addAnnotation(Loc loc, std::string str, u4 blockId, AnnotationPos pos) const;

    bool hadCriticalError() const;

    ErrorBuilder beginError(Loc loc, ErrorClass what) const;
    void _error(std::unique_ptr<Error> error) const;

    int totalErrors() const;
    bool wasModified() const;

    int globalStateId;
    int parentGlobalStateId;
    int lastNameKnownByParentGlobalState;
    bool silenceErrors = false;
    bool autocorrect = false;
    bool suggestGarbageType = false;

    std::unique_ptr<GlobalState> deepCopy(bool keepId = false) const;
    mutable std::shared_ptr<ErrorQueue> errorQueue;

    // When set, contains a location at which the typechecker should generate information
    // See ErrorQueue#queryResponse
    Loc lspInfoQueryLoc;
    SymbolRef lspQuerySymbol;

    void trace(std::string_view msg) const;

    static constexpr int HASH_STATE_INVALID = 0;
    unsigned int hash() const;
    std::vector<std::shared_ptr<File>> getFiles() const;

    void suppressErrorClass(ErrorClass err);

private:
    bool shouldReportErrorOn(Loc loc, ErrorClass what) const;
    static constexpr int STRINGS_PAGE_SIZE = 4096;
    std::vector<std::shared_ptr<std::vector<char>>> strings;
    std::string_view enterString(std::string_view nm);
    u2 strings_last_page_used = STRINGS_PAGE_SIZE + 1;
    std::vector<Name> names;
    UnorderedMap<std::string, FileRef> fileRefByPath;
    std::vector<Symbol> symbols;
    std::vector<std::pair<unsigned int, unsigned int>> names_by_hash;
    std::vector<std::shared_ptr<File>> files;
    UnorderedSet<ErrorClass> suppressed_error_classes;
    bool wasModified_ = false;

    mutable std::mutex annotations_mtx;
    mutable std::vector<Annotation> annotations;

    bool freezeSymbolTable();
    bool freezeNameTable();
    bool freezeFileTable();
    bool unfreezeSymbolTable();
    bool unfreezeNameTable();
    bool unfreezeFileTable();
    bool nameTableFrozen = true;
    bool symbolTableFrozen = true;
    bool fileTableFrozen = true;

    void expandNames(int growBy = 2);

    SymbolRef synthesizeClass(std::string_view name, u4 superclass = Symbols::todo()._id, bool isModule = false);
    SymbolRef enterSymbol(Loc loc, SymbolRef owner, NameRef name, u4 flags);

    SymbolRef getTopLevelClassSymbol(NameRef name);
};
// CheckSize(GlobalState, 152, 8);
// Historically commented out because size of unordered_map was different between different versions of stdlib

} // namespace sorbet::core

#endif // SORBET_GLOBAL_STATE_H
