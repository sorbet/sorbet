//
// Created by Dmitry Petrashko on 10/5/17.
//

#include "Context.h"
#include "Hashing.h"

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

NameRef ContextBase::enterNameUTF8(UTF8Desc nm) {
    const auto hs = _hash(nm);
    unsigned int hashTableSize = names_by_hash_size;
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probe_count = 1;

    while (names_by_hash[bucketId].second) {
        auto &bucket = names_by_hash[bucketId];
        if (bucket.first == hs) {
            auto name_id = bucket.second;
            auto &nm2 = names[name_id];
            if (nm2.kind == NameKind::UTF8 && nm2.raw.utf8 == nm)
                return name_id;
        }
        bucketId = (bucketId + probe_count) & mask;
        probe_count++;
    }
    DEBUG_ONLY(if (probe_count == hashTableSize) { Error::raise("Full table?"); });

    if (names_used == max_name_count) {
        expandNames();
        hashTableSize = names_by_hash_size;
        mask = hashTableSize - 1;
        bucketId = hs & mask; // look for place in the new size
        probe_count = 1;
        while (names_by_hash[bucketId].second != 0) {
            bucketId = (bucketId + probe_count) & mask;
            probe_count++;
        }
    }

    auto &bucket = names_by_hash[bucketId];
    bucket.first = hs;
    bucket.second = names_used;

    auto idx = names_used++;
    Error::check(nm.to < STRINGS_PAGE_SIZE);

    if (strings_last_page_used + nm.to > STRINGS_PAGE_SIZE) {
        strings.push_back(std::unique_ptr<char>((char *)malloc(STRINGS_PAGE_SIZE)));
        // printf("Wasted %i space\n", STRINGS_PAGE_SIZE - strings_last_page_used);
        strings_last_page_used = 0;
    }
    char *from = strings.back().get() + strings_last_page_used;

    memcpy(from, nm.from, nm.to);
    names[idx].kind = NameKind::UTF8;
    names[idx].raw.utf8.from = from;
    names[idx].raw.utf8.to = nm.to;
    strings_last_page_used += nm.to;

    return idx;
}

NameRef ContextBase::enterNameUnique(NameRef separator, u2 num, NameKind kind, NameRef original) {
    Error::check(separator.id() < names_used);
    const auto hs = _hash_mix_unique(separator.id(), UNIQUE, num, original.id());
    unsigned int hashTableSize = names_by_hash_size;
    unsigned int mask = hashTableSize - 1;
    auto bucketId = hs & mask;
    unsigned int probe_count = 1;

    while (names_by_hash[bucketId].second != 0 && probe_count < hashTableSize) {
        auto &bucket = names_by_hash[bucketId];
        if (bucket.first == hs) {
            auto &nm2 = names[bucket.second];
            if (nm2.kind == UNIQUE && nm2.unique.separator == separator && nm2.unique.num == num &&
                nm2.unique.original == original)
                return bucket.second;
        }
        bucketId = (bucketId + probe_count) & mask;
        probe_count++;
    }
    if (probe_count == hashTableSize) {
        Error::raise("Full table?");
    }

    if (names_used == max_name_count) {
        expandNames();
        hashTableSize = names_by_hash_size;
        mask = hashTableSize - 1;

        bucketId = hs & mask; // look for place in the new size
        probe_count = 1;
        while (names_by_hash[bucketId].second != 0) {
            bucketId = (bucketId + probe_count) & mask;
            probe_count++;
        }
    }

    auto &bucket = names_by_hash[bucketId];
    bucket.first = hs;
    bucket.second = names_used;

    auto idx = names_used++;

    names[idx].kind = UNIQUE;
    names[idx].unique.num = num;
    names[idx].unique.separator = separator;
    names[idx].unique.original = original;
    return idx;
}

SymbolRef ContextBase::getTopLevelClassSymbol(NameRef name) {
    auto &current = classes[name];
    if (current.exists()) {
        return current; // return fast
    }

    if (symbols_used == max_symbol_count)
        expandSymbols();
    current = symbols_used++;
    auto &info = current.info(*this, true); // allowing noSymbol is needed because this enters noSymbol.
    new (&info) SymbolInfo();
    info.name = name;
    info.flags = 0;
    info.setClass();
    return current;
}

} // namespace ast
} // namespace sruby