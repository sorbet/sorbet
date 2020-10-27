#ifndef SORBET_TYPEPTR_H
#define SORBET_TYPEPTR_H
#include "common/common.h"
#include <memory>

namespace sorbet::core {
// using TypePtr = std::shared_ptr<Type>;
class Type;
class TypePtr {
    std::shared_ptr<Type> store;
    TypePtr(std::shared_ptr<Type> &&store);

public:
    TypePtr() = default;
    TypePtr(TypePtr &&other) = default;
    TypePtr(const TypePtr &other) = default;
    TypePtr &operator=(TypePtr &&other) = default;
    TypePtr &operator=(const TypePtr &other) = default;
    explicit TypePtr(Type *ptr) : store(ptr) {}
    TypePtr(std::nullptr_t n) : store(nullptr) {}
    operator bool() const {
        return (bool)store;
    }
    Type *get() const {
        return store.get();
    }
    Type *operator->() const {
        return get();
    }
    Type &operator*() const {
        return *get();
    }
    bool operator!=(const TypePtr &other) const {
        return store != other.store;
    }
    bool operator==(const TypePtr &other) const {
        return store == other.store;
    }
    bool operator!=(std::nullptr_t n) const {
        return store != nullptr;
    }
    bool operator==(std::nullptr_t n) const {
        return store == nullptr;
    }

    template <class T, class... Args> friend TypePtr make_type(Args &&... args);
};
CheckSize(TypePtr, 16, 8);
} // namespace sorbet::core

#endif
