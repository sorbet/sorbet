#ifndef SORBET_NAMES_H
#define SORBET_NAMES_H

#include "common/common.h"
#include <string>
#include <vector>

#include "core/NameRef.h"

namespace sorbet {
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
    ENFORCE(nm == UTF8);
    return 1;
}

inline int _NameKind2Id_UNIQUE(NameKind nm) {
    ENFORCE(nm == UNIQUE);
    return 2;
}

inline int _NameKind2Id_CONSTANT(NameKind nm) {
    ENFORCE(nm == CONSTANT);
    return 3;
}

struct RawName final {
    std::string_view utf8;
};
CheckSize(RawName, 16, 8);

enum UniqueNameKind : u2 {
    Parser,
    Desugar,
    Namer,
    Singleton,
    Overload,
    TypeVarName,
    ResolverMissingClass // used by resolver when we want to enter a stub class into a static field. see
                         // test/resolver/stub_missing_class_alias.rb
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

class Name final {
public:
    friend GlobalState;

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
    bool isClassName(const GlobalState &gs) const;

    std::string toString(const GlobalState &gs) const;
    std::string show(const GlobalState &gs) const;
    std::string_view shortName(const GlobalState &gs) const;
    void sanityCheck(const GlobalState &gs) const;
    NameRef ref(const GlobalState &gs) const;

    Name deepCopy(const GlobalState &to) const;

private:
    unsigned int hash(const GlobalState &gs) const;
};

CheckSize(Name, 24, 8);
} // namespace core
} // namespace sorbet

#endif // SORBET_NAMES_H
