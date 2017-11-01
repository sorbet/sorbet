#ifndef SRUBY_NAMES_H
#define SRUBY_NAMES_H

#include "common/common.h"
#include <string>
#include <vector>

namespace ruby_typer {
namespace ast {
class GlobalState;
class Name;
enum NameKind : u1 {
    UTF8 = 1,
    UNIQUE = 2,
};

CheckSize(NameKind, 1, 1);

inline int _NameKind2Id_UTF8(NameKind nm) {
    DEBUG_ONLY(Error::check(nm == UTF8));
    return 1;
}

inline int _NameKind2Id_UNIQUE(NameKind nm) {
    DEBUG_ONLY(Error::check(nm == UNIQUE));
    return 2;
}

class NameRef {
public:
    friend GlobalState;
    friend Name;

    NameRef() : _id(-1){};

    NameRef(unsigned int id) : _id(id) {}

    NameRef(const NameRef &nm) = default;

    NameRef(NameRef &&nm) = default;

    NameRef &operator=(const NameRef &rhs) = default;

    bool operator==(const NameRef &rhs) const {
        return _id == rhs._id;
    }

    bool operator!=(const NameRef &rhs) const {
        return !(rhs == *this);
    }

    inline int id() const {
        return _id;
    }

    Name &name(GlobalState &ctx) const;

    inline bool exists() const {
        return _id != 0;
    }

    inline NameRef addEq() const {
        return NameRef(2);
    }

    std::string toString(GlobalState &ctx) const;

public:
    int _id;
};

CheckSize(NameRef, 4, 4);

struct UTF8Desc {
    const char *from;
    int to;

    friend std::ostream &operator<<(std::ostream &os, const UTF8Desc &dt) {
        os.write(dt.from, dt.to);
        return os;
    }

    UTF8Desc(const char *from, int to) : from(from), to(to) {}
    UTF8Desc(const std::string &str) : from(str.c_str()), to(str.size()) {}

    inline bool operator==(const UTF8Desc &rhs) const {
        return (to == rhs.to) && ((from == rhs.from) || !strncmp(from, rhs.from, to));
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

    std::string toString() const {
        return std::string(from, to);
    }
};

struct RawName {
    UTF8Desc utf8;
};
CheckSize(RawName, 16, 8);

enum UniqueNameKind : u2 { Parser, Desugar, CFG, Singleton };

struct UniqueName {
    NameRef original;
    UniqueNameKind uniqueNameKind;
    u2 num;
};

CheckSize(UniqueName, 8, 4)

    class Names {
public:
    static inline NameRef initialize() {
        return NameRef(1);
    }
    static inline NameRef andAnd() {
        return NameRef(2);
    }
    static inline NameRef orOr() {
        return NameRef(3);
    }

    static inline NameRef to_s() {
        return NameRef(4);
    }

    static inline NameRef concat() {
        return NameRef(5);
    }

    static inline NameRef call() {
        return NameRef(6);
    }

    // !
    static inline NameRef bang() {
        return NameRef(7);
    }

    // []
    static inline NameRef squareBrackets() {
        return NameRef(8);
    }

    // []=
    static inline NameRef squareBracketsEq() {
        return NameRef(9);
    }

    // @+
    static inline NameRef unaryPlus() {
        return NameRef(10);
    }

    // @-
    static inline NameRef unaryMinus() {
        return NameRef(11);
    }

    // *
    static inline NameRef star() {
        return NameRef(12);
    }

    // **
    static inline NameRef starStar() {
        return NameRef(13);
    }

    // used in CFG for temporary
    static inline NameRef whileTemp() {
        return NameRef(14);
    }

    // used in CFG for temporary
    static inline NameRef ifTemp() {
        return NameRef(15);
    }

    // used in CFG for temporary
    static inline NameRef returnTemp() {
        return NameRef(16);
    }

    // used in CFG for temporary
    static inline NameRef statTemp() {
        return NameRef(17);
    }

    // used in CFG for temporary
    static inline NameRef assignTemp() {
        return NameRef(18);
    }

    // used in CFG for temporary
    static inline NameRef returnMethodTemp() {
        return NameRef(19);
    }

    // used in CFG for block return value temporaries
    static inline NameRef blockReturnTemp() {
        return NameRef(20);
    }

    // include
    static inline NameRef include() {
        return NameRef(21);
    }

    // __FILE__
    static inline NameRef currentFile() {
        return NameRef(22);
    }

    // merge
    static inline NameRef merge() {
        return NameRef(23);
    }

    // used in CFG for temporary
    static inline NameRef selfMethodTemp() {
        return NameRef(24);
    }

    // used in resolver to find signatures
    static inline NameRef standardMethod() {
        return NameRef(25);
    }

    // used in resolver to find signatures
    static inline NameRef returns() {
        return NameRef(26);
    }

    // used in resolver to find signatures
    static inline NameRef all() {
        return NameRef(27);
    }

    // used in resolver to find signatures
    static inline NameRef any() {
        return NameRef(28);
    }

    // used in resolver to find signatures
    static inline NameRef nilable() {
        return NameRef(29);
    }

    // The next two names are used as keys in SymbolInfo::members to store
    // pointers up and down the singleton-class hierarchy. If A's singleton
    // class is B, then A will have a `singletonClass` entry in its members
    // table which references B, and B will have an `attachedClass` entry
    // pointing at A.
    //
    // The "attached class" terminology is borrowed from MRI, which refers
    // to the unique instance attached to a singleton class as the "attached
    // object"
    static inline NameRef singletonClass() {
        return NameRef(30);
    }

    static inline NameRef attachedClass() {
        return NameRef(31);
    }
};

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

    std::string toString(GlobalState &ctx) const;

private:
    unsigned int hash(GlobalState &ctx) const;

public:
    static unsigned int hashNames(std::vector<NameRef> &lhs, GlobalState &ctx);
};

CheckSize(Name, 24, 8);
} // namespace ast
} // namespace ruby_typer

template <> struct std::hash<ruby_typer::ast::NameRef> {
    size_t operator()(const ruby_typer::ast::NameRef &x) const {
        return x._id;
    }
};

template <> struct std::equal_to<ruby_typer::ast::NameRef> {
    constexpr bool operator()(const ruby_typer::ast::NameRef &lhs, const ruby_typer::ast::NameRef &rhs) const {
        return lhs._id == rhs._id;
    }
};

#endif // SRUBY_NAMES_H
