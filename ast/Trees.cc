
#include "Trees.h"

namespace sruby {
namespace ast {

sruby::ast::Name::~Name() noexcept {
  if (kind == NameKind::UNIQUE)
    unique.~UniqueName();
}

static constexpr unsigned int HASH_MULT = 65599; // sdbm
static constexpr unsigned int HASH_MULT2 = 31;   // for names

inline unsigned int mix(unsigned int acc, unsigned int nw) {
  return nw + (acc << 6) + (acc << 16) - acc; // HASH_MULT in faster version
}

unsigned int _hash(UTF8Desc utf8) {
  // TODO: replace with http://www.sanmayce.com/Fastest_Hash/, see
  // https://www.strchr.com/hash_functions and
  // https://github.com/rurban/smhasher
  auto *end = utf8.from + utf8.to;
  unsigned int res = 0;
  auto it = utf8.from;
#pragma clang loop unroll(enable)
  for (; it != end; it++) {
    res = mix(res, *it - '!'); // "!" is the first printable letter in ASCII.
    // This will help Latin1 but may harm utf8 multibyte
  }
  return res * HASH_MULT2 + _NameKind2Id_UTF8(UTF8);
}

unsigned int _hash_mix_unique(unsigned int hash1, NameKind nk,
                              unsigned int hash2, unsigned int hash3) {
  return mix(mix(hash2, hash1), hash3) * HASH_MULT2 + _NameKind2Id_UNIQUE(nk);
}

SymbolRef ContextBase::synthesizeClass(UTF8Desc name) {
  auto nameId = enterNameUTF8(name);
  auto symId = getTopLevelClassSymbol(nameId);
  symId.info(*this, true).setCompleted();
  return symId;
}

static const char *init = "initialize";
static UTF8Desc init_DESC{(char *)init, std::strlen(init)};

ContextBase::ContextBase(spdlog::logger &logger) : logger(logger) {
  max_name_count = 262144;   // 6MB
  max_symbol_count = 524288; // 32MB

  symbols = (SymbolInfo *)malloc(max_symbol_count * sizeof(SymbolInfo));
  constants.reserve(512);
  names = (Name *)malloc(max_name_count * sizeof(Name));
  names_by_hash_size = 2 * max_name_count;
  names_by_hash = (std::pair<unsigned int, unsigned int> *)calloc(
      names_by_hash_size, sizeof(std::pair<unsigned int, unsigned int>));
  symbols_used = 0;
  DEBUG_ONLY(
      Error::check((names_by_hash_size & (names_by_hash_size - 1)) == 0));

  names_used = 1; // first name is used in hashes to indicate empty cell
  // Second name is always <init>, see SymbolInfo::isConstructor
  auto init_id = enterNameUTF8(init_DESC);
  DEBUG_ONLY(Error::check(init_id._id == 1));

  SymbolRef no_symbol_id = synthesizeClass(no_symbol_DESC);
  SymbolRef object_id = synthesizeClass(JLObject_DESC);
  SymbolRef int_id = synthesizeClass(int_DESC);
  SymbolRef long_id = synthesizeClass(long_DESC);
  SymbolRef float_id = synthesizeClass(float_DESC);
  SymbolRef double_id = synthesizeClass(double_DESC);
  SymbolRef char_id = synthesizeClass(char_DESC);
  SymbolRef short_id = synthesizeClass(short_DESC);
  SymbolRef bool_id = synthesizeClass(bool_DESC);
  SymbolRef byte_id = synthesizeClass(byte_DESC);
  SymbolRef void_id = synthesizeClass(void_DESC);
  SymbolRef top_id = synthesizeClass(TOP_DESC);
  SymbolRef null_id = synthesizeClass(null_DESC);

  Error::check(no_symbol_id == noSymbol());
  Error::check(object_id == defn_javaLangObject());
  Error::check(int_id == defn_int());
  Error::check(long_id == defn_long());
  Error::check(float_id == defn_float());
  Error::check(double_id == defn_double());
  Error::check(char_id == defn_char());
  Error::check(short_id == defn_short());
  Error::check(bool_id == defn_bool());
  Error::check(byte_id == defn_byte());
  Error::check(void_id == defn_void());
  Error::check(top_id == defn_top());
  Error::check(null_id == defn_null());
  /* 0: NoSymbol
   * 1: Object
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
  Error::check(symbols_used == 13);
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
