#ifndef SRUBY_NAMES_H
#define SRUBY_NAMES_H

#include "common/common.h"
#include <string>
#include <vector>

namespace ruby_typer {
namespace core {
class GlobalState;
class Name;
enum NameKind : u1 {
    UTF8 = 1,
    UNIQUE = 2,
    CONSTANT = 3,
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

inline int _NameKind2Id_CONSTANT(NameKind nm) {
    DEBUG_ONLY(Error::check(nm == CONSTANT));
    return 3;
}

class NameRef final {
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

    Name &name(GlobalState &gs) const;

    inline bool exists() const {
        return _id != 0;
    }

    bool isBlockClashSafe(GlobalState &gs) const;

    NameRef addEq(GlobalState &gs) const;

    std::string toString(GlobalState &gs) const;

public:
    int _id;
};

CheckSize(NameRef, 4, 4);

struct UTF8Desc final {
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

struct RawName final {
    UTF8Desc utf8;
};
CheckSize(RawName, 16, 8);

enum UniqueNameKind : u2 {
    Parser,
    Desugar,
    Namer,
    CFG,
    NestedScope, // used by freshName to make sure blocks local variables do not collapse into method variables
    Singleton,
};

struct UniqueName final {
    NameRef original;
    UniqueNameKind uniqueNameKind;
    u2 num;
};

CheckSize(UniqueName, 8, 4);

struct ConstantName final {
    NameRef original;
};

class Names final {
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

    // Used to generate temporary names for entering blocks into the symbol
    // table.
    static inline NameRef blockTemp() {
        return NameRef(32);
    }

    // used in resolver to find signatures
    static inline NameRef declareVariables() {
        return NameRef(33);
    }

    static inline NameRef new_() {
        return NameRef(34);
    };

    // Used to generate temporary names for destructuring arguments ala proc do
    //  |(x,y)|; end
    static inline NameRef destructureArg() {
        return NameRef(35);
    }

    // &
    static inline NameRef ampersand() {
        return NameRef(36);
    }

    static inline NameRef blockCall() {
        return NameRef(37);
    }

    static inline NameRef lambda() {
        return NameRef(38);
    }

    // nil?
    static inline NameRef nil_p() {
        return NameRef(39);
    }

    // super calls
    static inline NameRef super() {
        return NameRef(40);
    }

    // the empty string
    static inline NameRef empty() {
        return NameRef(41);
    }

    // ===
    static inline NameRef tripleEq() {
        return NameRef(42);
    }
};

class Name final {
public:
    NameKind kind;

private:
    unsigned char UNUSED(_fill[3]);

public:
    union { // todo: can discriminate this union through the pointer to Name
        // itself using lower bits
        RawName raw;
        UniqueName unique;
        ConstantName cnst;
    };

    Name() noexcept {};

    Name(Name &&other) noexcept = default;

    Name(const Name &other) = delete;

    ~Name() noexcept;

    bool operator==(const Name &rhs) const;

    bool operator!=(const Name &rhs) const;

    std::string toString(GlobalState &gs) const;

private:
    unsigned int hash(GlobalState &gs) const;

public:
    static unsigned int hashNames(std::vector<NameRef> &lhs, GlobalState &gs);
};

CheckSize(Name, 24, 8);
} // namespace core
} // namespace ruby_typer

template <> struct std::hash<ruby_typer::core::NameRef> {
    size_t operator()(const ruby_typer::core::NameRef &x) const {
        return x._id;
    }
};

template <> struct std::equal_to<ruby_typer::core::NameRef> {
    constexpr bool operator()(const ruby_typer::core::NameRef &lhs, const ruby_typer::core::NameRef &rhs) const {
        return lhs._id == rhs._id;
    }
};

#endif // SRUBY_NAMES_H
