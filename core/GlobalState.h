#ifndef SRUBY_GLOBAL_STATE_H
#define SRUBY_GLOBAL_STATE_H
#include "Errors.h"
#include "Files.h"
#include "Hashing.h"
#include "Loc.h"
#include "Names.h"
#include "Symbols.h"
#include "spdlog/spdlog.h"
#include <memory>

namespace ruby_typer {
namespace core {

class Name;
class NameRef;
class Symbol;
class SymbolRef;
class GlobalSubstitution;

namespace serialize {
class GlobalStateSerializer;
}

class GlobalState final {
    friend Name;
    friend NameRef;
    friend Symbol;
    friend SymbolRef;
    friend File;
    friend FileRef;
    friend GlobalSubstitution;
    friend serialize::GlobalStateSerializer;
    friend class UnfreezeNameTable;
    friend class UnfreezeSymbolTable;
    friend class UnfreezeFileTable;

public:
    GlobalState(spdlog::logger &logger);
    void initEmpty();

    GlobalState(const GlobalState &) = delete;
    GlobalState(GlobalState &&) = delete;

    ~GlobalState();

    SymbolRef enterClassSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterTypeMember(Loc loc, SymbolRef owner, NameRef name, Variance variance);
    SymbolRef enterTypeArgument(Loc loc, SymbolRef owner, NameRef name, Variance variance);
    SymbolRef enterMethodSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterNewMethodOverload(Loc loc, SymbolRef original, u2 num);
    SymbolRef enterFieldSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterStaticFieldSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterMethodArgumentSymbol(Loc loc, SymbolRef owner, NameRef name);
    LocalVariable enterLocalSymbol(SymbolRef owner, NameRef name);

    LocalVariable newTemporary(NameRef name, SymbolRef owner);

    NameRef enterNameUTF8(absl::string_view nm);

    NameRef freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num);

    NameRef enterNameConstant(NameRef original);
    NameRef enterNameConstant(absl::string_view original);

    FileRef enterFile(absl::string_view path, absl::string_view source);
    FileRef enterFileAt(absl::string_view path, absl::string_view source, int id);
    FileRef enterFile(std::shared_ptr<File> file);
    FileRef enterFileAt(std::shared_ptr<File> file, int id);

    unsigned int namesUsed();

    unsigned int symbolsUsed();
    unsigned int filesUsed() const;

    void sanityCheck() const;

    std::string toString(bool showHidden = false);
    std::string showAnnotatedSource(FileRef file);
    enum AnnotationPos { BEFORE, AFTER };
    struct Annotation {
        Loc loc;
        std::string str;
        AnnotationPos pos;
        Annotation(Loc loc, std::string str, AnnotationPos pos) : loc(loc), str(str), pos(pos){};
    };
    void addAnnotation(Loc loc, std::string str, AnnotationPos pos = AnnotationPos::BEFORE) {
        annotations.emplace_back(loc, str, pos);
    }

    bool hadCriticalError() {
        return errors.hadCritical;
    };

    template <typename... Args> void error(Loc loc, ErrorClass what, const std::string &msg, const Args &... args) {
        std::string formatted = fmt::format(msg, args...);
        _error(std::make_unique<BasicError>(BasicError(loc, what, formatted)));
    }
    void error(ComplexError error) {
        _error(std::make_unique<ComplexError>(error));
    }

    int totalErrors();
    void flushErrors();
    std::vector<std::unique_ptr<BasicError>> drainErrors();
    std::vector<std::pair<int, int>> errorHistogram() {
        return this->errors.histogram;
    }

    spdlog::logger &logger;

    static SymbolRef noSymbol() {
        return SymbolRef(nullptr, 0);
    }
    static SymbolRef defn_top() {
        return SymbolRef(nullptr, 1);
    }
    static SymbolRef defn_bottom() {
        return SymbolRef(nullptr, 2);
    }

    static SymbolRef defn_root() {
        return SymbolRef(nullptr, 3);
    }

    static SymbolRef defn_nil() {
        return SymbolRef(nullptr, 4);
    }

    static SymbolRef defn_todo() {
        return SymbolRef(nullptr, 5);
    }

    static SymbolRef defn_Object() {
        return SymbolRef(nullptr, 6);
    }

    static SymbolRef defn_junk() {
        return SymbolRef(nullptr, 7);
    }

    static SymbolRef defn_Integer() {
        return SymbolRef(nullptr, 8);
    }

    static SymbolRef defn_Float() {
        return SymbolRef(nullptr, 9);
    }

    static SymbolRef defn_String() {
        return SymbolRef(nullptr, 10);
    }

    static SymbolRef defn_Symbol() {
        return SymbolRef(nullptr, 11);
    }

    static SymbolRef defn_Array() {
        return SymbolRef(nullptr, 12);
    }

    static SymbolRef defn_Hash() {
        return SymbolRef(nullptr, 13);
    }

    static SymbolRef defn_TrueClass() {
        return SymbolRef(nullptr, 14);
    }

    static SymbolRef defn_FalseClass() {
        return SymbolRef(nullptr, 15);
    }

    static SymbolRef defn_NilClass() {
        return SymbolRef(nullptr, 16);
    }

    static SymbolRef defn_untyped() {
        return SymbolRef(nullptr, 17);
    }

    static SymbolRef defn_Opus() {
        return SymbolRef(nullptr, 18);
    }

    static SymbolRef defn_T() {
        return SymbolRef(nullptr, 19);
    }

    static SymbolRef defn_Class() {
        return SymbolRef(nullptr, 20);
    }

    static SymbolRef defn_BasicObject() {
        return SymbolRef(nullptr, 21);
    }

    static SymbolRef defn_Kernel() {
        return SymbolRef(nullptr, 22);
    }

    static SymbolRef defn_Range() {
        return SymbolRef(nullptr, 23);
    }

    static SymbolRef defn_Regexp() {
        return SymbolRef(nullptr, 24);
    }

    static SymbolRef defn_Magic() {
        return SymbolRef(nullptr, 25);
    }

    static SymbolRef defn_Module() {
        return SymbolRef(nullptr, 26);
    }

    static SymbolRef defn_StandardError() {
        return SymbolRef(nullptr, 27);
    }

    static SymbolRef defn_Complex() {
        return SymbolRef(nullptr, 28);
    }

    static SymbolRef defn_Rational() {
        return SymbolRef(nullptr, 29);
    }

    static SymbolRef defn_T_Array() {
        return SymbolRef(nullptr, 30);
    }

    static SymbolRef defn_T_Hash() {
        return SymbolRef(nullptr, 31);
    }

    static SymbolRef defn_T_Proc() {
        return SymbolRef(nullptr, 32);
    }

    static SymbolRef defn_Proc() {
        return SymbolRef(nullptr, 33);
    }

    static SymbolRef defn_T_any() {
        return SymbolRef(nullptr, 34);
    }

    static SymbolRef defn_T_all() {
        return SymbolRef(nullptr, 35);
    }

    static SymbolRef defn_T_untyped() {
        return SymbolRef(nullptr, 36);
    }

    static SymbolRef defn_T_nilable() {
        return SymbolRef(nullptr, 37);
    }

    static SymbolRef defn_Enumerable() {
        return SymbolRef(nullptr, 38);
    }

    static SymbolRef defn_Set() {
        return SymbolRef(nullptr, 39);
    }

    static SymbolRef defn_Struct() {
        return SymbolRef(nullptr, 40);
    }

    static SymbolRef defn_File() {
        return SymbolRef(nullptr, 41);
    }

    // Keep as last and update to match the last entry
    static SymbolRef defn_last_synthetic_sym() {
        return SymbolRef(nullptr, MAX_SYNTHETIC_SYMBOLS - 1);
    }

    int globalStateId;

    std::unique_ptr<GlobalState> deepCopy() const;

private:
    static constexpr int MAX_SYNTHETIC_SYMBOLS = 100;
    static constexpr int STRINGS_PAGE_SIZE = 4096;
    std::vector<std::shared_ptr<std::vector<char>>> strings;
    absl::string_view enterString(absl::string_view nm);
    u2 strings_last_page_used = STRINGS_PAGE_SIZE + 1;
    std::vector<Name> names;
    std::vector<Symbol> symbols;
    std::vector<std::pair<unsigned int, unsigned int>> names_by_hash;
    std::vector<std::shared_ptr<File>> files;
    std::vector<Annotation> annotations;
    struct {
        std::vector<std::unique_ptr<BasicError>> buffer;
        std::vector<std::pair<int, int>> histogram;
        bool hadCritical = false;
    } errors;

    bool freezeSymbolTable();
    bool freezeNameTable();
    bool freezeFileTable();
    bool unfreezeSymbolTable();
    bool unfreezeNameTable();
    bool unfreezeFileTable();
    bool nameTableFrozen = false;
    bool symbolTableFrozen = false;
    bool fileTableFrozen = false;

    void _error(std::unique_ptr<BasicError> error);

    void expandNames();

    void expandSymbols();

    void complete(SymbolRef id, Symbol &currentInfo);

    SymbolRef synthesizeClass(absl::string_view name, u4 superclass = core::GlobalState::defn_todo()._id,
                              bool isModule = false);
    SymbolRef enterSymbol(Loc loc, SymbolRef owner, NameRef name, u4 flags);

    SymbolRef getTopLevelClassSymbol(NameRef name);
};
// CheckSize(GlobalState, 152, 8);
// Historically commented out because size of unordered_map was different between different versions of stdlib

} // namespace core
} // namespace ruby_typer

#endif // SRUBY_GLOBAL_STATE_H
