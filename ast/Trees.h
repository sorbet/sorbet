#ifndef SRUBY_TREES_H
#define SRUBY_TREES_H

#include "../common/common.h"
#include "common/common.h"
#include "spdlog/spdlog.h"
#include <vector>

namespace sruby {
namespace ast {
class ContextBase;

class Name;

class NameRef;

class SymbolRef;

class SymbolInfo;

class Arena;

enum NameKind : u1 {
  UTF8 = 1,
  UNIQUE = 2,
};

CheckSize(NameKind, 1, 1)

    inline int _NameKind2Id_UTF8(NameKind nm) {
  DEBUG_ONLY(Error::check(nm == UTF8));
  return 1;
}

inline int _NameKind2Id_UNIQUE(NameKind nm) {
  DEBUG_ONLY(Error::check(nm == UNIQUE));
  return 2;
}

class SymbolRef {
  friend class ContextBase;

public:
  constexpr SymbolRef(u4 _id) : _id(_id){};

  SymbolRef() : _id(0){};

  unsigned inline int classId() const {
    DEBUG_ONLY(Error::check((_id & 3) == 3, "not a classId"));
    return _id >> 2;
  }

  unsigned inline int defId() const {
    DEBUG_ONLY(Error::check((_id & 3) == 2, "not a defId"));
    return _id >> 2;
  }

  unsigned inline int valId() const {
    DEBUG_ONLY(Error::check((_id & 3) == 1, "not a valId"));
    return _id >> 2;
  }

  unsigned inline int typeId() const {
    DEBUG_ONLY(Error::check((_id & 3) == 0, "not a valId"));
    return _id >> 2;
  }

  bool inline exists() const { return _id; }

  bool isSynthetic() const;

  bool isPrimitive() const;

  SymbolInfo &info(ContextBase &ctx, bool lazy = false) const;

  bool operator==(const SymbolRef &rhs) const;

  bool operator!=(const SymbolRef &rhs) const;

  bool operator!() { return !_id; }

private:
  u4 x;
};

CheckSize(SymbolRef, 4, 4)

    struct ConstantRef {
  unsigned int operator()() const { return _id; }

  ConstantRef(const ConstantRef &) = default;

  ConstantRef(const unsigned int _id) : _id(_id) {}

private:
  unsigned int _id; // todo: encode strings too
};

CheckSize(ConstantRef, 4, 4);

class NameRef {
public:
  friend ContextBase;
  friend Name;

  NameRef() : _id(-1){};

  NameRef(unsigned int id) : _id(id) {}

  NameRef(const NameRef &nm) = default;

  NameRef(NameRef &&nm) = default;

  NameRef &operator=(const NameRef &rhs) = default;

  bool operator==(const NameRef &rhs) const { return _id == rhs._id; }

  bool operator!=(const NameRef &rhs) const { return !(rhs == *this); }

  std::string show(ContextBase &ctx) const;

  inline int id() const { return _id; }

  Name &name(ContextBase &ctx) const;

  inline bool exists() const { return _id != 0; }

public:
  int _id;
};

CheckSize(NameRef, 4, 4);
} // namespace ast
} // namespace sruby
template <> struct std::hash<sruby::ast::NameRef> {
  size_t operator()(const sruby::ast::NameRef &x) const { return x._id; }
};

template <> struct std::equal_to<sruby::ast::NameRef> {
  constexpr bool operator()(const sruby::ast::NameRef &lhs,
                            const sruby::ast::NameRef &rhs) const {
    return lhs._id == rhs._id;
  }
};

namespace sruby {
namespace ast {

struct UTF8Desc {
  const char *from;
  int to;

  friend std::ostream &operator<<(std::ostream &os, const UTF8Desc &dt) {
    os.write(dt.from, dt.to);
    return os;
  }

  inline bool operator==(const UTF8Desc &rhs) const {
    return (to == rhs.to) &&
           ((from == rhs.from) || !strncmp(from, rhs.from, to));
  }

  inline bool operator!=(const UTF8Desc &rhs) const {
    return !this->operator==(rhs);
  }

  inline bool operator!=(const char *rhs) const {
    return !this->operator==(rhs);
  }

  inline bool operator==(const char *rhs) const {
    return rhs && (strlen(rhs) == to) && !strncmp(from, rhs, to);
  }

  std::string toString() const { return std::string(from, to); }
};

struct RawName {
  UTF8Desc utf8;
};
CheckSize(RawName, 16, 8);

struct UniqueName {
  NameRef original;
  u2 num;
};

CheckSize(UniqueName, 8, 4)

    class Name {
public:
  NameKind kind;

private:
  unsigned char UNUSED(_fill[3]);

public:
  union { // todo: can discriminate this union through the pointer to Name
    // itself using lower bits
    RawName raw;
    UniqueName unique;
  };

  Name() noexcept {};

  Name(Name &&other) noexcept = default;

  Name(const Name &other) = delete;

  ~Name() noexcept;

  bool operator==(const Name &rhs) const;

  bool operator!=(const Name &rhs) const;

  UTF8Desc printTo(Arena &, ContextBase &ctx) const;

private:
  unsigned int hash(ContextBase &ctx) const;

  int length(ContextBase &ctx) const;

  char *printTo(char *, ContextBase &ctx) const;
};

CheckSize(Name, 24, 8);

class SymbolInfo {
public:
  bool isConstructor(ContextBase &ctx) const;

  SymbolRef owner;
  /* isClass,   IsArray,  isField, isMethod
   * IsFromJar, IsFromFile
   * */
  u4 flags;
  // this contains the tree that only defines the type. TODO: make into tiny
  // vector
  // for classes it contains constructor signature
  // for method - full method type. it is not used to refer to fields. No
  // fields. Yet.
  //    std::vector<Tree> definition;
  std::vector<SymbolRef> argumentsOrInterfaces;

  inline std::vector<SymbolRef> &arguments() {
    Error::check(!isClass());
    return argumentsOrInterfaces;
  }

  inline std::vector<SymbolRef> &interfaces(ContextBase &ctx) {
    Error::check(isClass());
    ensureCompleted(ctx);
    return argumentsOrInterfaces;
  }

  SymbolRef resultOrParentOrLoader;

  inline SymbolRef result() const {
    Error::check(!isClass());
    return resultOrParentOrLoader;
  }

  inline SymbolRef parent(ContextBase &ctx) {
    Error::check(isClass());
    ensureCompleted(ctx);
    return resultOrParentOrLoader;
  }

  SymbolRef ref(ContextBase &ctx) const;

  inline bool isClass() const { return (flags & 0x8000) != 0; }

  inline bool isArray() const { return (flags & 0x4000) != 0; }

  inline bool isField() const { return (flags & 0x2000) != 0; }

  inline bool isMethod() const { return (flags & 0x1000) != 0; }

  inline bool isCompleted() const { return (flags & 0x0C00) == 0x0C00; }

  inline bool isCompletingFromJar() const { return (flags & 0x0C00) == 0x0800; }

  inline bool isCompletingFromFile() const {
    return (flags & 0x0C00) == 0x0400;
  }

  inline void setClass() {
    DEBUG_ONLY(Error::check(!isArray() && !isField() && !isMethod()));
    flags = flags | 0x8000;
  }

  inline void setArray() {
    DEBUG_ONLY(Error::check(!isClass() && !isField() && !isMethod()));
    flags = flags | 0x4000;
  }

  inline void setField() {
    DEBUG_ONLY(Error::check(!isClass() && !isArray() && !isMethod()));
    flags = flags | 0x2000;
  }

  inline void setMethod() {
    DEBUG_ONLY(Error::check(!isClass() && !isArray() && !isField()));
    flags = flags | 0x1000;
  }

  inline void setCompleted() {
    DEBUG_ONLY(Error::check(!isArray()));
    flags = flags | 0x0C00;
  }

  void setCompletingFromFile(unsigned int idx) {
    DEBUG_ONLY(Error::check(!isCompleted()));
    DEBUG_ONLY(Error::check(!isArray()));
    flags = flags | 0x0400;
    resultOrParentOrLoader = idx;
  }

  //    std::vector<Tree> implementation; // TODO: make into small vector too
  NameRef name; // todo: move out? it should not matter but it's important for
  // name resolution
  std::vector<std::pair<NameRef, SymbolRef>>
      members; // TODO: replace with https://github.com/greg7mdp/sparsepp &
               // optimize for absence
private:
  void ensureCompleted(ContextBase &ctx);
};

CheckSize(SymbolInfo, 64, 8);

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

  ConstantRef addConstant(long bits);

  SymbolRef fillPreregistedSym(SymbolRef which, SymbolRef owner, NameRef name);

  SymbolRef prePregisterSym();

  SymbolRef enterSymbol(SymbolRef owner, NameRef name, SymbolRef result,
                        std::vector<SymbolRef> &args, bool isMethod,
                        bool isCompletion);

  SymbolRef getTopLevelClassSymbol(NameRef name, u1 dims = 0);

  SymbolRef newInnerClass(SymbolRef owner, NameRef name);

  NameRef enterNameUTF8(UTF8Desc nm);

  NameRef enterNameUnique(NameRef separator, u2 num, NameKind kind,
                          std::vector<NameRef> &&originals);

  NameRef enterNameSigned(NameRef orig, NameRef result,
                          std::vector<NameRef> &&args);

  int indexClassOrJar(const char *name);

  unsigned int namesUsed();

  unsigned int symbolsUsed();

  unsigned int symbolCapacity();

  unsigned int constantsUsed();

  void sanityCheck() const;

  spdlog::logger &logger;

  SymbolRef noSymbol() { return SymbolRef(0); }

  SymbolRef defn_javaLangString();

  static constexpr SymbolRef defn_javaLangObject() { return SymbolRef(1); }

  static constexpr SymbolRef defn_int() { return SymbolRef(2); }

  static constexpr SymbolRef defn_long() { return SymbolRef(3); }

  static constexpr SymbolRef defn_float() { return SymbolRef(4); }

  static constexpr SymbolRef defn_double() { return SymbolRef(5); }

  static constexpr SymbolRef defn_char() { return SymbolRef(6); }

  static constexpr SymbolRef defn_short() { return SymbolRef(7); }

  static constexpr SymbolRef defn_bool() { return SymbolRef(8); }

  static constexpr SymbolRef defn_byte() { return SymbolRef(9); }

  static constexpr SymbolRef defn_void() { return SymbolRef(10); }

  static constexpr SymbolRef defn_top() { return SymbolRef(11); }

  static constexpr SymbolRef defn_null() { return SymbolRef(12); }

private:
  std::vector<long> constants;
  static constexpr int STRINGS_PAGE_SIZE = 4096;
  std::vector<std::unique_ptr<char>> strings;
  u2 strings_last_page_used = STRINGS_PAGE_SIZE;
  Name *names;
  SymbolInfo *symbols;
  unsigned int max_symbol_count;
  unsigned int symbols_used;
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

} // namespace ast
} // namespace sruby

#endif // SRUBY_TREES_H
