//
// Created by Dmitry Petrashko on 10/5/17.
//

#include "Context.h"

namespace sruby {
namespace ast {

SymbolRef ContextBase::synthesizeClass(UTF8Desc name) {
    auto nameId = enterNameUTF8(name);
    auto symId = getTopLevelClassSymbol(nameId);
    symId.info(*this, true).setCompleted();
    return symId;
}

static const char *init = "initialize";
static UTF8Desc init_DESC{(char *)init, (int)std::strlen(init)};

static const char *no_symbol_str = "<none>";
static UTF8Desc no_symbol_DESC{(char *)no_symbol_str, (int)std::strlen(no_symbol_str)};

static const char *top_str = "<top>";
static UTF8Desc top_DESC{(char *)top_str, (int)std::strlen(top_str)};

static const char *bottom_str = "<bottom>";
static UTF8Desc bottom_DESC{(char *)bottom_str, (int)std::strlen(bottom_str)};

ContextBase::ContextBase(spdlog::logger &logger) : logger(logger) {
    max_name_count = 262144;   // 6MB
    max_symbol_count = 524288; // 32MB

    symbols = (SymbolInfo *)malloc(max_symbol_count * sizeof(SymbolInfo));
    names = (Name *)malloc(max_name_count * sizeof(Name));
    names_by_hash_size = 2 * max_name_count;
    names_by_hash = (std::pair<unsigned int, unsigned int> *)calloc(names_by_hash_size,
                                                                    sizeof(std::pair<unsigned int, unsigned int>));
    symbols_used = 0;
    DEBUG_ONLY(Error::check((names_by_hash_size & (names_by_hash_size - 1)) == 0));

    names_used = 1; // first name is used in hashes to indicate empty cell
    // Second name is always <init>, see SymbolInfo::isConstructor
    auto init_id = enterNameUTF8(init_DESC);
    DEBUG_ONLY(Error::check(init_id._id == 1));

    SymbolRef no_symbol_id = synthesizeClass(no_symbol_DESC);
    SymbolRef top_id = synthesizeClass(top_DESC); // BasicObject
    SymbolRef bottom_id = synthesizeClass(bottom_DESC);
    //  SymbolRef int_id = synthesizeClass(int_DESC);
    //  SymbolRef long_id = synthesizeClass(long_DESC);
    //  SymbolRef float_id = synthesizeClass(float_DESC);
    //  SymbolRef double_id = synthesizeClass(double_DESC);
    //  SymbolRef char_id = synthesizeClass(char_DESC);
    //  SymbolRef short_id = synthesizeClass(short_DESC);
    //  SymbolRef bool_id = synthesizeClass(bool_DESC);
    //  SymbolRef byte_id = synthesizeClass(byte_DESC);
    //  SymbolRef void_id = synthesizeClass(void_DESC);
    //  SymbolRef top_id = synthesizeClass(TOP_DESC);
    //  SymbolRef null_id = synthesizeClass(null_DESC);

    Error::check(no_symbol_id == noSymbol());
    Error::check(top_id == defn_top());
    Error::check(bottom_id == denf_bottom());
    //  Error::check(int_id == defn_int());
    //  Error::check(long_id == defn_long());
    //  Error::check(float_id == defn_float());
    //  Error::check(double_id == defn_double());
    //  Error::check(char_id == defn_char());
    //  Error::check(short_id == defn_short());
    //  Error::check(bool_id == defn_bool());
    //  Error::check(byte_id == defn_byte());
    //  Error::check(void_id == defn_void());
    //  Error::check(top_id == defn_top());
    //  Error::check(null_id == defn_null());
    /* 0: <none>
     * 1: <top>
     * 2: <bottom>
     * 2: int();
     * 3: long();
     * 4: float();
     * 5: double();
     * 6: char();
     * 7: short();
     * 8: bool();
     * 9: byte();
     * 10: void();
     * 11: TOP;
     * 12: null
     */
    Error::check(symbols_used == 3);
}

ContextBase::~ContextBase() {
    for (int i = 0; i < names_used; i++) {
        names[i].~Name();
    }
    for (int i = 0; i < symbols_used; i++) {
        symbols[i].~SymbolInfo();
    }

    free(names);
    free(names_by_hash);
    free(symbols);
    names = nullptr;
    symbols = nullptr;
    names_by_hash = nullptr;
}
} // namespace ast
} // namespace sruby