#ifndef SORBET_TYPEPTR_H
#define SORBET_TYPEPTR_H
#include "common/common.h"
#include "core/NameRef.h"
#include "core/SymbolRef.h"
#include <memory>

namespace sorbet::core {
class Type;
class TypeConstraint;
struct DispatchResult;
struct DispatchArgs;

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

    // Required for typecase.
    template <class To> static bool isa(const TypePtr &what);

    template <class To> static To const &cast(const TypePtr &what);

    template <class To> static To &cast(TypePtr &what) {
        return const_cast<To &>(cast<To>(static_cast<const TypePtr &>(what)));
    }

    static std::string tagToString(Tag tag);

private:
    std::atomic<u4> *counter;
    tagged_storage store;

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

    TypePtr(Tag tag, std::atomic<u4> *counter, void *expr) : counter(counter), store(tagPtr(tag, expr)) {
        if (counter != nullptr) {
            counter->fetch_add(1);
        }
    }
    static void deleteTagged(Tag tag, void *ptr) noexcept;

    // A version of release that doesn't mask the tag bits
    tagged_storage releaseTagged() noexcept {
        auto saved = store;
        store = 0;
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

    void _sanityCheck(const GlobalState &gs) const;

public:
    constexpr TypePtr() noexcept : counter(nullptr), store(0) {}

    TypePtr(std::nullptr_t) noexcept : TypePtr() {}

    TypePtr(TypePtr &&other) noexcept : counter(other.releaseCounter()), store(other.releaseTagged()){};

    TypePtr(const TypePtr &other) noexcept : counter(other.counter), store(other.store) {
        if (counter != nullptr) {
            counter->fetch_add(1);
        }
    };

    ~TypePtr() {
        handleDelete();
    }

    TypePtr &operator=(TypePtr &&other) noexcept {
        handleDelete();
        counter = other.releaseCounter();
        store = other.releaseTagged();
        return *this;
    };

    TypePtr &operator=(const TypePtr &other) noexcept {
        if (*this == other) {
            return *this;
        }

        handleDelete();
        counter = other.counter;
        store = other.store;
        if (counter != nullptr) {
            counter->fetch_add(1);
        }
        return *this;
    };

    explicit TypePtr(Tag tag, void *expr) : TypePtr(tag, new std::atomic<u4>(), expr) {}

    operator bool() const {
        return (bool)store;
    }

    Tag tag() const noexcept {
        ENFORCE_NO_TIMER(store != 0);

        auto value = reinterpret_cast<tagged_storage>(store) & TAG_MASK;
        if (value <= 7) {
            return static_cast<Tag>(value);
        } else {
            return static_cast<Tag>(value >> 48);
        }
    }

    Type *get() const {
        auto val = store & PTR_MASK;
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
        return store != other.store;
    }
    bool operator==(const TypePtr &other) const {
        return store == other.store;
    }
    bool operator!=(std::nullptr_t n) const {
        return store != 0;
    }
    bool operator==(std::nullptr_t n) const {
        return store == 0;
    }

    bool isUntyped() const;

    bool isNilClass() const;

    bool isBottom() const;

    // Used in subtyping.cc to order types.
    int kind() const;

    std::string typeName() const;

    bool isFullyDefined() const;

    bool hasUntyped() const;

    void sanityCheck(const GlobalState &gs) const {
        if (!debug_mode)
            return;
        _sanityCheck(gs);
    }

    core::SymbolRef untypedBlame() const;

    TypePtr getCallArguments(const GlobalState &gs, NameRef name) const;

    TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc) const;

    TypePtr _replaceSelfType(const GlobalState &gs, const TypePtr &receiver) const;

    TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) const;

    TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                         const std::vector<TypePtr> &targs) const;

    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string toString(const GlobalState &gs) const {
        return toStringWithTabs(gs);
    }

    // User visible type. Should exactly match what the user can write.
    std::string show(const GlobalState &gs) const;
    // Like show, but can include extra info. Does not necessarily match what the user can type.
    std::string showWithMoreInfo(const GlobalState &gs) const;

    bool derivesFrom(const GlobalState &gs, SymbolRef klass) const;

    unsigned int hash(const GlobalState &gs) const;

    DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) const;

    template <class T, class... Args> friend TypePtr make_type(Args &&... args);
    friend class TypePtrTestHelper;
};
CheckSize(TypePtr, 16, 8);
} // namespace sorbet::core

#endif
