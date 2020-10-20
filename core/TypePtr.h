#ifndef SORBET_TYPEPTR_H
#define SORBET_TYPEPTR_H
#include "common/common.h"
#include <memory>

namespace sorbet::core {
class Type;
class TypePtr final {
public:
    // We store tagged pointers as 64-bit values.
    using tagged_storage = u8;

    enum class Tag {
        ClassType = 1,
        LambdaParam,
        SelfTypeParam,
        AliasType,
        SelfType,
        LiteralType,
        TypeVar,
        OrType,
        AndType,
        ShapeType,
        TupleType,
        AppliedType,
        MetaType,
        BlamedUntyped,
        UnresolvedClassType,
        UnresolvedAppliedType,
    };

    // A mapping from type to its corresponding tag.
    template <typename T> struct TypeToTag;

    // A mapping from tag value to the type it represents.
    template <Tag T> struct TagToType;

private:
    std::atomic<u4> *counter;
    tagged_storage ptr;

    static constexpr tagged_storage TAG_MASK = 0xffff000000000007;

    static constexpr tagged_storage PTR_MASK = ~TAG_MASK;

    static tagged_storage tagPtr(Tag tag, void *expr) {
        auto val = static_cast<tagged_storage>(tag);
        if (val >= 8) {
            // Store the tag in the upper 16 bits of the pointer, as it won't fit in the lower three bits.
            val <<= 48;
        }

        auto maskedPtr = reinterpret_cast<tagged_storage>(expr) & PTR_MASK;

        return maskedPtr | val;
    }

    explicit TypePtr(Tag tag, std::atomic<u4> *counter, void *expr) : counter(counter), ptr(tagPtr(tag, expr)) {
        if (counter != nullptr) {
            counter->fetch_add(1);
        }
    }
    static void deleteTagged(Tag tag, void *ptr) noexcept;

    // A version of release that doesn't mask the tag bits
    tagged_storage releaseTagged() noexcept {
        auto saved = ptr;
        ptr = 0;
        return saved;
    }

    std::atomic<u4> *releaseCounter() noexcept {
        auto saved = counter;
        counter = nullptr;
        return saved;
    }

    void handleDelete() noexcept {
        if (counter != nullptr) {
            // fetch_sub returns value prior to subtract
            const u4 counterVal = counter->fetch_sub(1) - 1;
            if (counterVal == 0) {
                deleteTagged(tag(), get());
                delete counter;
            }
        }
    }

public:
    // Default: noSymbol class type.
    constexpr TypePtr() noexcept : counter(nullptr), ptr(0) {}

    TypePtr(std::nullptr_t) noexcept : TypePtr() {}

    TypePtr(TypePtr &&other) : counter(other.releaseCounter()), ptr(other.releaseTagged()){};

    TypePtr(const TypePtr &other) : counter(other.counter), ptr(other.ptr) {
        if (counter != nullptr) {
            counter->fetch_add(1);
        }
    };

    ~TypePtr() {
        handleDelete();
    }

    TypePtr &operator=(TypePtr &&other) {
        handleDelete();
        counter = other.releaseCounter();
        ptr = other.releaseTagged();
        return *this;
    };

    TypePtr &operator=(const TypePtr &other) {
        if (*this == other) {
            return *this;
        }

        handleDelete();
        counter = other.counter;
        ptr = other.ptr;
        if (counter != nullptr) {
            counter->fetch_add(1);
        }
        return *this;
    };

    explicit TypePtr(Tag tag, void *expr) : TypePtr(tag, new std::atomic<u4>(), expr) {}

    operator bool() const {
        return (bool)ptr;
    }

    Tag tag() const noexcept {
        ENFORCE_NO_TIMER(ptr != 0);

        auto value = reinterpret_cast<tagged_storage>(ptr) & TAG_MASK;
        if (value <= 7) {
            return static_cast<Tag>(value);
        } else {
            return static_cast<Tag>(value >> 48);
        }
    }

    Type *get() const {
        auto val = ptr & PTR_MASK;
        if constexpr (sizeof(void *) == 4) {
            return reinterpret_cast<Type *>(val);
        } else {
            // sign extension for the upper 16 bits
            return reinterpret_cast<Type *>((val << 16) >> 16);
        }
    }
    Type *operator->() const {
        return get();
    }
    Type &operator*() const {
        return *get();
    }
    bool operator!=(const TypePtr &other) const {
        return ptr != other.ptr;
    }
    bool operator==(const TypePtr &other) const {
        return ptr == other.ptr;
    }
    bool operator!=(std::nullptr_t n) const {
        return ptr != 0;
    }
    bool operator==(std::nullptr_t n) const {
        return ptr == 0;
    }

    template <class T, class... Args> friend TypePtr make_type(Args &&... args);
};
CheckSize(TypePtr, 16, 8);
} // namespace sorbet::core

#endif
