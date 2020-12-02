#ifndef SORBET_NAMES_H
#define SORBET_NAMES_H

#include "common/common.h"
#include <string>
#include <vector>

#include "core/NameRef.h"
#include "core/Names_gen.h"

namespace sorbet::core {
class GlobalState;
namespace serialize {
class SerializerImpl;
}

class UTF8Name final {
    friend class GlobalState;
    friend class serialize::SerializerImpl;
    UTF8Name(std::string_view utf8);

public:
    const std::string_view utf8;

    unsigned int hash(const GlobalState &gs) const;
    UTF8Name deepCopy(const GlobalState &to) const;
};
CheckSize(UTF8Name, 16, 8);

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

class UniqueName final {
    friend class GlobalState;
    friend class serialize::SerializerImpl;
    UniqueName(NameRef original, u4 num, UniqueNameKind uniqueNameKind);

public:
    const NameRef original;
    const u4 num;
    const UniqueNameKind uniqueNameKind;

    unsigned int hash(const GlobalState &gs) const;
    UniqueName deepCopy(const GlobalState &to) const;
};

CheckSize(UniqueName, 12, 4);

class ConstantName final {
    friend class GlobalState;
    friend class serialize::SerializerImpl;
    ConstantName(NameRef original);

public:
    const NameRef original;

    unsigned int hash(const GlobalState &gs) const;
    ConstantName deepCopy(const GlobalState &to) const;
};
CheckSize(ConstantName, 4, 4);

} // namespace sorbet::core

#endif // SORBET_NAMES_H
