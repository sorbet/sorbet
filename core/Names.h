#ifndef SORBET_NAMES_H
#define SORBET_NAMES_H

#include "common/common.h"
#include <string>
#include <vector>

#include "core/NameRef.h"
#include "core/Names_gen.h"

namespace sorbet::core {
class GlobalState;
class Name;
enum class NameKind : u1 {
    UTF8 = 1,
    UNIQUE = 2,
    CONSTANT = 3,
};

CheckSize(NameKind, 1, 1);

inline int _NameKind2Id_UTF8(NameKind nm) {
    ENFORCE(nm == NameKind::UTF8);
    return 1;
}

inline int _NameKind2Id_UNIQUE(NameKind nm) {
    ENFORCE(nm == NameKind::UNIQUE);
    return 2;
}

inline int _NameKind2Id_CONSTANT(NameKind nm) {
    ENFORCE(nm == NameKind::CONSTANT);
    return 3;
}

struct RawName final {
    std::string_view utf8;
};
CheckSize(RawName, 16, 8);

enum class UniqueNameKind : u1 {
    Parser,
    Desugar,
    Namer,
    MangleRename,
    Singleton,
    Overload,
    TypeVarName,
    PositionalArg,        // num >=0 - normal args, -1 ==> restarg, -2 => kwrest arg
    MangledKeywordArg,    // only used when we have duplicated keyword arguments
    ResolverMissingClass, // used by resolver when we want to enter a stub class into a static field. see
                          // test/resolver/stub_missing_class_alias.rb
    TEnum,
};

struct UniqueName final {
    NameRef original;
    u4 num;
    UniqueNameKind uniqueNameKind;
};

CheckSize(UniqueName, 12, 4);

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

    // Convenience method, because enums need to be special cased in more places than other kinds of
    // unique names, and everyone always forget to unwrap the first layer (NameKind::CONSTANT)
    // before checking for UniqueNameKind::TEnum.
    bool isTEnumName(const GlobalState &gs) const;

    std::string showRaw(const GlobalState &gs) const;
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
} // namespace sorbet::core

#endif // SORBET_NAMES_H
