#ifndef SRUBY_GLOBAL_STATE_H
#define SRUBY_GLOBAL_STATE_H
#include "Errors.h"
#include "Files.h"
#include "Hashing.h"
#include "Loc.h"
#include "Names.h"
#include "Symbols.h"
#include <memory>
#include <mutex>

namespace ruby_typer {
namespace core {

class Name;
class NameRef;
class Symbol;
class SymbolRef;
class GlobalSubstitution;
class ErrorRegion;
struct ErrorQueue;

namespace serialize {
class Serializer;
}

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
    friend class UnfreezeNameTable;
    friend class UnfreezeSymbolTable;
    friend class UnfreezeFileTable;

public:
    GlobalState(std::shared_ptr<ErrorQueue> errorQueue);
    void initEmpty();

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

    LocalVariable newTemporary(NameRef name, SymbolRef owner) const;

    NameRef enterNameUTF8(absl::string_view nm);

    NameRef getNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num) const;
    NameRef freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num);

    NameRef enterNameConstant(NameRef original);
    NameRef enterNameConstant(absl::string_view original);

    FileRef enterFile(absl::string_view path, absl::string_view source);
    FileRef enterFileAt(absl::string_view path, absl::string_view source, int id);
    FileRef enterFile(std::shared_ptr<File> file);
    FileRef enterNewFileAt(std::shared_ptr<File> file, int id);

    void mangleRenameSymbol(SymbolRef what, NameRef origName, UniqueNameKind kind);

    unsigned int namesUsed() const;

    unsigned int symbolsUsed() const;
    unsigned int filesUsed() const;

    void sanityCheck() const;

    std::string toString(bool showHidden = false) const;
    std::string showAnnotatedSource(FileRef file) const;
    enum AnnotationPos { BEFORE, AFTER };
    struct Annotation {
        Loc loc;
        std::string str;
        AnnotationPos pos;
        Annotation(Loc loc, std::string str, AnnotationPos pos) : loc(loc), str(str), pos(pos){};
    };
    void addAnnotation(Loc loc, std::string str, AnnotationPos pos = AnnotationPos::BEFORE) const;

    bool hadCriticalError() const;

    ErrorBuilder beginError(Loc loc, ErrorClass what) const;

    int totalErrors() const;
    void flushErrors();
    bool wasModified() const;

    int globalStateId;
    bool silenceErrors = false;

    std::unique_ptr<GlobalState> deepCopy(bool keepId = false) const;
    mutable std::shared_ptr<ErrorQueue> errorQueue;

    void trace(const std::string &msg) const;

private:
    bool shouldReportErrorOn(Loc loc, ErrorClass what) const;
    static constexpr int STRINGS_PAGE_SIZE = 4096;
    std::vector<std::shared_ptr<std::vector<char>>> strings;
    absl::string_view enterString(absl::string_view nm);
    u2 strings_last_page_used = STRINGS_PAGE_SIZE + 1;
    std::vector<Name> names;
    std::vector<Symbol> symbols;
    std::vector<std::pair<unsigned int, unsigned int>> names_by_hash;
    std::vector<std::shared_ptr<File>> files;
    bool wasModified_ = false;

    mutable std::mutex annotations_mtx;
    mutable std::vector<Annotation> annotations;

    bool freezeSymbolTable();
    bool freezeNameTable();
    bool freezeFileTable();
    bool unfreezeSymbolTable();
    bool unfreezeNameTable();
    bool unfreezeFileTable();
    bool nameTableFrozen = false;
    bool symbolTableFrozen = false;
    bool fileTableFrozen = false;

    void _error(std::unique_ptr<BasicError> error) const;

    void expandNames();

    SymbolRef synthesizeClass(absl::string_view name, u4 superclass = core::Symbols::todo()._id, bool isModule = false);
    SymbolRef enterSymbol(Loc loc, SymbolRef owner, NameRef name, u4 flags);

    SymbolRef getTopLevelClassSymbol(NameRef name);
};
// CheckSize(GlobalState, 152, 8);
// Historically commented out because size of unordered_map was different between different versions of stdlib

} // namespace core
} // namespace ruby_typer

#endif // SRUBY_GLOBAL_STATE_H
