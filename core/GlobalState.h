#ifndef SRUBY_GLOBAL_STATE_H
#define SRUBY_GLOBAL_STATE_H
#include "Files.h"
#include "Hashing.h"
#include "Loc.h"
#include "Names.h"
#include "Reporter.h"
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

    GlobalState(const GlobalState &) = delete;
    GlobalState(GlobalState &&) = default;

    ~GlobalState();

    SymbolRef enterClassSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterMethodSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterFieldSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterStaticFieldSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterMethodArgumentSymbol(Loc loc, SymbolRef owner, NameRef name);
    LocalVariable enterLocalSymbol(SymbolRef owner, NameRef name);

    LocalVariable newTemporary(UniqueNameKind kind, NameRef name, SymbolRef owner);

    NameRef enterNameUTF8(absl::string_view nm);

    NameRef freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num);

    NameRef enterNameConstant(NameRef original);
    NameRef enterNameConstant(absl::string_view original);

    FileRef enterFile(absl::string_view path, absl::string_view source);
    FileRef enterFile(std::shared_ptr<File> file);

    unsigned int namesUsed();

    unsigned int symbolsUsed();
    unsigned int filesUsed();

    void sanityCheck();

    std::string toString(bool showHidden = false);

    spdlog::logger &logger;
    Reporter errors;

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

    static SymbolRef defn_Opus_Types() {
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

    // Keep as last and update to match the last entry
    static SymbolRef defn_last_synthetic_sym() {
        return SymbolRef(nullptr, MAX_SYNTHETIC_SYMBOLS - 1);
    }

    int globalStateId;

    GlobalState deepCopy();

private:
    static constexpr int MAX_SYNTHETIC_SYMBOLS = 100;
    static constexpr int STRINGS_PAGE_SIZE = 4096;
    std::vector<std::shared_ptr<std::vector<char>>> strings;
    absl::string_view enterString(absl::string_view nm);
    u2 strings_last_page_used = STRINGS_PAGE_SIZE;
    std::vector<Name> names;
    std::vector<Symbol> symbols;
    std::vector<std::pair<unsigned int, unsigned int>> names_by_hash;
    std::vector<std::shared_ptr<File>> files;

    bool freezeSymbolTable();
    bool freezeNameTable();
    bool freezeFileTable();
    bool unfreezeSymbolTable();
    bool unfreezeNameTable();
    bool unfreezeFileTable();
    bool nameTableFrozen = false;
    bool symbolTableFrozen = false;
    bool fileTableFrozen = false;

    void expandNames();

    void expandSymbols();

    void complete(SymbolRef id, Symbol &currentInfo);

    SymbolRef synthesizeClass(absl::string_view name, int superclass = core::GlobalState::defn_todo()._id);

    SymbolRef enterSymbol(Loc loc, SymbolRef owner, NameRef name, u4 flags);

    SymbolRef getTopLevelClassSymbol(NameRef name);
};
// CheckSize(GlobalState, 152, 8);
// Historically commented out because size of unordered_map was different between different versions of stdlib

} // namespace core
} // namespace ruby_typer

#endif // SRUBY_GLOBAL_STATE_H
