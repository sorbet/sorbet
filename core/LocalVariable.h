#ifndef RUBY_TYPER_LOCALVARIABLE_H
#define RUBY_TYPER_LOCALVARIABLE_H

#include "core/NameRef.h"
#include "core/Names.h"

namespace sorbet::core {

class LocalVariable final {
public:
    NameRef _name;

    // NOTE: this field serves to disambiguate multiple definitions of the same
    // local name, and identifies the block scope it was defined in.
    // Additionally, this value can only be non-zero if the variable is defined
    // in the scope of a block with a non-zero scope id.
    uint32_t unique;

    LocalVariable(NameRef name, uint32_t unique) : _name(name), unique(unique) {}

    LocalVariable() = default;

    bool exists() const;

    bool isSyntheticTemporary() const;

    bool isAliasForGlobal(const GlobalState &gs) const;

    LocalVariable(const LocalVariable &) = default;

    LocalVariable(LocalVariable &&) = default;

    LocalVariable &operator=(LocalVariable &&) = default;

    LocalVariable &operator=(const LocalVariable &) = default;

    bool operator==(const LocalVariable &rhs) const;

    bool operator!=(const LocalVariable &rhs) const;

    inline bool operator<(const LocalVariable &rhs) const {
        if (this->_name.rawId() < rhs._name.rawId()) {
            return true;
        }
        if (this->_name.rawId() > rhs._name.rawId()) {
            return false;
        }
        return this->unique < rhs.unique;
    }

    static inline LocalVariable noVariable() {
        return LocalVariable(NameRef::noName(), 0);
    }

    static inline LocalVariable blockCall() {
        return LocalVariable(Names::blockCall(), 0);
    }

    std::string showRaw(const GlobalState &gs) const;
    std::string toString(const GlobalState &gs) const;

    // A unique representation of this name for exporting to external tools
    std::string exportableName(const GlobalState &gs) const;

    static inline LocalVariable selfVariable() {
        return LocalVariable(Names::selfLocal(), 0);
    }

    static inline LocalVariable unconditional() {
        return LocalVariable(Names::unconditional(), 0);
    }
};

CheckSize(LocalVariable, 8, 4);

template <typename H> H AbslHashValue(H h, const LocalVariable &m) {
    return H::combine(std::move(h), m._name, m.unique);
}
} // namespace sorbet::core
#endif // RUBY_TYPER_LOCALVARIABLE_H
