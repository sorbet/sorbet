#ifndef SRUBY_SYMBOLS_H
#define SRUBY_SYMBOLS_H

#include "Names.h"
#include "common/common.h"
#include <memory>
#include <vector>

namespace ruby_typer {
namespace ast {
class Symbol;
class GlobalState;
class Type;

class SymbolRef {
    friend class GlobalState;

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

    bool inline exists() const {
        return _id;
    }

    inline SymbolRef orElse(SymbolRef other) const {
        if (exists())
            return *this;
        else
            return other;
    }

    bool isSynthetic() const;

    /** Not printed when showing name table */
    bool isHidden() const;

    bool isPlaceHolder() const;

    bool isPrimitive() const;

    Symbol &info(GlobalState &ctx, bool allowNone = false) const;

    bool operator==(const SymbolRef &rhs) const;

    bool operator!=(const SymbolRef &rhs) const;

    bool operator!() {
        return !_id;
    }

    std::string toString(GlobalState &ctx, int tabs = 0) const;

    u4 _id;
};

CheckSize(SymbolRef, 4, 4);

class Symbol {
public:
    Symbol(const Symbol &) = delete;
    Symbol() = default;
    Symbol(Symbol &&) noexcept = default;

    bool isConstructor(GlobalState &ctx) const;

    SymbolRef owner;
    /* isClass,   IsArray,  isField, isMethod
     * IsFromJar, IsFromFile
     * */
    u4 flags;
    // TODO: make into tiny
    std::vector<SymbolRef> argumentsOrMixins;

    inline std::vector<SymbolRef> &arguments() {
        Error::check(!isClass());
        return argumentsOrMixins;
    }

    bool derivesFrom(GlobalState &ctx, SymbolRef sym);

    SymbolRef ref(GlobalState &ctx);

    inline std::vector<SymbolRef> &mixins(GlobalState &ctx) {
        Error::check(isClass());
        ensureCompleted(ctx);
        return argumentsOrMixins;
    }

    SymbolRef parent_;
    std::shared_ptr<Type> resultType;

    inline SymbolRef parent(GlobalState &ctx) {
        Error::check(isClass());
        ensureCompleted(ctx);
        return parent_;
    }

    SymbolRef ref(GlobalState &ctx) const;

    inline bool isClass() const {
        return (flags & 0x8000) != 0;
    }

    inline bool isArray() const {
        return (flags & 0x4000) != 0;
    }

    inline bool isField() const {
        return (flags & 0x2000) != 0;
    }

    inline bool isMethod() const {
        return (flags & 0x1000) != 0;
    }

    inline bool isCompleted() const {
        return (flags & 0x0C00) == 0x0C00;
    }

    inline bool isCompletingFromJar() const {
        return (flags & 0x0C00) == 0x0800;
    }

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

    SymbolRef findMember(NameRef name);
    std::string fullName(GlobalState &ctx) const;

    //    std::vector<Tree> implementation; // TODO: make into small vector too
    NameRef name; // todo: move out? it should not matter but it's important for
    // name resolution
    std::vector<std::pair<NameRef, SymbolRef>> members; // TODO: replace with https://github.com/greg7mdp/sparsepp &
    // optimize for absence
private:
    void ensureCompleted(GlobalState &ctx);
};

CheckSize(Symbol, 64, 8);
} // namespace ast
} // namespace ruby_typer

namespace std {
template <> struct hash<ruby_typer::ast::SymbolRef> {
    std::size_t operator()(const ruby_typer::ast::SymbolRef k) const {
        return k._id;
    }
};
} // namespace std
#endif // SRUBY_SYMBOLS_H
