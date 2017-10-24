#ifndef SRUBY_SYMBOLS_H
#define SRUBY_SYMBOLS_H

#include "Names.h"
#include "common/common.h"
#include <vector>

namespace ruby_typer {
namespace ast {
class SymbolInfo;
class ContextBase;

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

    bool inline exists() const {
        return _id;
    }

    bool isSynthetic() const;

    bool isPlaceHolder() const;

    bool isPrimitive() const;

    SymbolInfo &info(ContextBase &ctx, bool allowNone = false) const;

    bool operator==(const SymbolRef &rhs) const;

    bool operator!=(const SymbolRef &rhs) const;

    bool operator!() {
        return !_id;
    }

    std::string toString(ContextBase &ctx, int tabs = 0) const;

private:
    u4 _id;
};

CheckSize(SymbolRef, 4, 4);

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
    std::vector<SymbolRef> argumentsOrMixins;

    inline std::vector<SymbolRef> &arguments() {
        Error::check(!isClass());
        return argumentsOrMixins;
    }

    inline std::vector<SymbolRef> &mixins(ContextBase &ctx) {
        Error::check(isClass());
        ensureCompleted(ctx);
        return argumentsOrMixins;
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

    void setCompletingFromFile(unsigned int idx) {
        DEBUG_ONLY(Error::check(!isCompleted()));
        DEBUG_ONLY(Error::check(!isArray()));
        flags = flags | 0x0400;
        resultOrParentOrLoader = idx;
    }

    //    std::vector<Tree> implementation; // TODO: make into small vector too
    NameRef name; // todo: move out? it should not matter but it's important for
    // name resolution
    std::vector<std::pair<NameRef, SymbolRef>> members; // TODO: replace with https://github.com/greg7mdp/sparsepp &
    // optimize for absence
private:
    void ensureCompleted(ContextBase &ctx);
};

CheckSize(SymbolInfo, 64, 8);
} // namespace ast
} // namespace ruby_typer

#endif // SRUBY_SYMBOLS_H
