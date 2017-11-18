#ifndef SRUBY_GLOBAL_STATE_H
#define SRUBY_GLOBAL_STATE_H

#include "Files.h"
#include "Hashing.h"
#include "Loc.h"
#include "Names.h"
#include "Reporter.h"
#include "Symbols.h"
#include "spdlog/spdlog.h"

namespace ruby_typer {
namespace core {

class Name;
class NameRef;
class Symbol;
class SymbolRef;
struct UTF8Desc;

class GlobalState final {
    friend Name;
    friend NameRef;
    friend Symbol;
    friend SymbolRef;
    friend File;
    friend FileRef;

public:
    GlobalState(spdlog::logger &logger);

    GlobalState(const GlobalState &) = delete;

    GlobalState(GlobalState &&) = delete;

    ~GlobalState();

    SymbolRef fillPreregistedSym(SymbolRef which, SymbolRef owner, NameRef name); // need to be implemented from scratch

    SymbolRef prePregisterSym();

    SymbolRef enterClassSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterMethodSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterFieldSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterStaticFieldSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterMethodArgumentSymbol(Loc loc, SymbolRef owner, NameRef name);
    LocalVariable enterLocalSymbol(SymbolRef owner, NameRef name);

    LocalVariable newTemporary(UniqueNameKind kind, NameRef name, SymbolRef owner);

    NameRef enterNameUTF8(UTF8Desc nm);

    NameRef freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num = 0);

    NameRef enterNameConstant(NameRef original);
    NameRef enterNameConstant(UTF8Desc original);

    FileRef enterFile(UTF8Desc path, UTF8Desc source);

    int indexClassOrJar(const char *name);

    unsigned int namesUsed();

    unsigned int symbolsUsed();
    unsigned int filesUsed();

    unsigned int symbolCapacity();

    void sanityCheck() const;

    std::string toString();

    spdlog::logger &logger;
    Reporter errors;

    static constexpr SymbolRef noSymbol() {
        return SymbolRef(0);
    }
    static constexpr SymbolRef defn_top() {
        return SymbolRef(1);
    }
    static constexpr SymbolRef defn_bottom() {
        return SymbolRef(2);
    }

    static constexpr SymbolRef defn_root() {
        return SymbolRef(3);
    }

    static constexpr SymbolRef defn_nil() {
        return SymbolRef(4);
    }

    static constexpr SymbolRef defn_todo() {
        return SymbolRef(5);
    }

    static constexpr SymbolRef defn_object() {
        return SymbolRef(6);
    }

    static constexpr SymbolRef defn_junk() {
        return SymbolRef(7);
    }

    static constexpr SymbolRef defn_Integer() {
        return SymbolRef(8);
    }

    static constexpr SymbolRef defn_Float() {
        return SymbolRef(9);
    }

    static constexpr SymbolRef defn_String() {
        return SymbolRef(10);
    }

    static constexpr SymbolRef defn_Symbol() {
        return SymbolRef(11);
    }

    static constexpr SymbolRef defn_Array() {
        return SymbolRef(12);
    }

    static constexpr SymbolRef defn_Hash() {
        return SymbolRef(13);
    }

    static constexpr SymbolRef defn_TrueClass() {
        return SymbolRef(14);
    }

    static constexpr SymbolRef defn_FalseClass() {
        return SymbolRef(15);
    }

    static constexpr SymbolRef defn_NilClass() {
        return SymbolRef(16);
    }

    static constexpr SymbolRef defn_untyped() {
        return SymbolRef(17);
    }

    static constexpr SymbolRef defn_Opus() {
        return SymbolRef(18);
    }

    static constexpr SymbolRef defn_Opus_Types() {
        return SymbolRef(19);
    }

    static constexpr SymbolRef defn_Class() {
        return SymbolRef(20);
    }

    static constexpr SymbolRef defn_Basic_Object() {
        return SymbolRef(21);
    }

    static constexpr SymbolRef defn_Kernel() {
        return SymbolRef(22);
    }

    static constexpr SymbolRef defn_emptyHash() {
        return SymbolRef(23);
    }

    static constexpr SymbolRef defn_Range() {
        return SymbolRef(24);
    }

    // Keep as last and update to match the last entry
    static constexpr SymbolRef defn_last_synthetic_sym() {
        return SymbolRef(MAX_SYNTHETIC_SYMBOLS - 1);
    }

private:
    static constexpr int MAX_SYNTHETIC_SYMBOLS = 100;
    static constexpr int STRINGS_PAGE_SIZE = 4096;
    std::vector<std::unique_ptr<std::vector<char>>> strings;
    u2 strings_last_page_used = STRINGS_PAGE_SIZE;
    std::vector<Name> names;
    std::vector<Symbol> symbols;
    unsigned int max_zips_count;
    unsigned int zips_used;
    unsigned int max_files_count;
    unsigned int files_used;
    std::vector<std::pair<unsigned int, unsigned int>> names_by_hash;
    std::vector<File> files;

    void expandNames();

    void expandSymbols();

    void complete(SymbolRef id, Symbol &currentInfo);

    SymbolRef synthesizeClass(UTF8Desc name);

    SymbolRef enterSymbol(Loc loc, SymbolRef owner, NameRef name, u4 flags);

    SymbolRef getTopLevelClassSymbol(NameRef name);

    u2 freshNameId = 0;
};
// CheckSize(GlobalState, 152, 8);
// Historically commented out because size of unordered_map was different between different versions of stdlib

} // namespace core
} // namespace ruby_typer

#endif // SRUBY_GLOBAL_STATE_H
