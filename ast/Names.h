#ifndef SRUBY_NAMES_H
#define SRUBY_NAMES_H

#include "common/common.h"
#include <string>

namespace sruby {
namespace ast {
class ContextBase;
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
    friend ContextBase;
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

    std::string show(ContextBase &ctx) const;

    inline int id() const {
        return _id;
    }

    Name &name(ContextBase &ctx) const;

    inline bool exists() const {
        return _id != 0;
    }

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

    std::string toString(ContextBase &ctx) const;

private:
    unsigned int hash(ContextBase &ctx) const;
};

CheckSize(Name, 24, 8);
} // namespace ast
} // namespace sruby

template <> struct std::hash<sruby::ast::NameRef> {
    size_t operator()(const sruby::ast::NameRef &x) const {
        return x._id;
    }
};

template <> struct std::equal_to<sruby::ast::NameRef> {
    constexpr bool operator()(const sruby::ast::NameRef &lhs, const sruby::ast::NameRef &rhs) const {
        return lhs._id == rhs._id;
    }
};

#endif // SRUBY_NAMES_H
