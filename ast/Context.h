#ifndef SRUBY_CONTEXT_H
#define SRUBY_CONTEXT_H

#include "Names.h"
#include "Symbols.h"
#include "common/common.h"
#include "spdlog/spdlog.h"
#include <vector>

namespace sruby {
namespace ast {
class Name;
class NameRef;
class SymbolInfo;
class SymbolRef;
struct UTF8Desc;

class ContextBase {
    friend Name;
    friend NameRef;
    friend SymbolInfo;
    friend SymbolRef;

public:
    ContextBase(spdlog::logger &logger);

    ContextBase(const ContextBase &) = delete;

    ContextBase(ContextBase &&) = delete;

    ~ContextBase();

    SymbolRef fillPreregistedSym(SymbolRef which, SymbolRef owner, NameRef name); // need to be implemented from scratch

    SymbolRef prePregisterSym();

    SymbolRef enterSymbol(SymbolRef owner, NameRef name, SymbolRef result, std::vector<SymbolRef> &args, bool isMethod);

    /** Creates a new top-level class if not exists already */
    SymbolRef getTopLevelClassSymbol(NameRef name);

    SymbolRef newInnerClass(SymbolRef owner, NameRef name); // Needs to be implemented from scratch

    NameRef enterNameUTF8(UTF8Desc nm);

    NameRef enterNameUnique(NameRef separator, u2 num, NameKind kind, NameRef original);

    int indexClassOrJar(const char *name);

    unsigned int namesUsed();

    unsigned int symbolsUsed();

    unsigned int symbolCapacity();

    void sanityCheck() const;

    spdlog::logger &logger;

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

private:
    static constexpr int STRINGS_PAGE_SIZE = 4096;
    std::vector<std::unique_ptr<std::vector<char>>> strings;
    u2 strings_last_page_used = STRINGS_PAGE_SIZE;
    Name *names;
    std::vector<SymbolInfo> symbols;
    unsigned int max_zips_count;
    unsigned int zips_used;
    unsigned int max_files_count;
    unsigned int files_used;
    std::pair<unsigned int, unsigned int> *names_by_hash;
    unsigned int max_name_count;
    unsigned int names_by_hash_size;
    unsigned int names_used;
    std::unordered_map<NameRef, SymbolRef> classes;

    void expandNames();

    void expandSymbols();

    void complete(SymbolRef id, SymbolInfo &currentInfo);

    SymbolRef synthesizeClass(UTF8Desc name);
};
// CheckSize(ContextBase, 152, 8);
// Historically commented out because size of unordered_map was different between different versions of stdlib

class Context {
public:
    ContextBase &state;
    SymbolRef owner;
    operator ContextBase &() {
        return state;
    }

    Context(ContextBase &state, SymbolRef owner) : state(state), owner(owner) {}
    Context(const Context &other) : state(other.state), owner(other.owner) {}

    Context withOwner(SymbolRef sym) {
        auto r = Context(*this);
        r.owner = sym;
        return r;
    }
};

} // namespace ast
} // namespace sruby

#endif // SRUBY_CONTEXT_H
