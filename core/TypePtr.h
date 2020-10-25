#ifndef SORBET_TYPEPTR_H
#define SORBET_TYPEPTR_H
#include "common/common.h"
#include "core/NameRef.h"
#include "core/SymbolRef.h"
#include <memory>

namespace sorbet::core {
class TypeConstraint;
struct DispatchResult;
struct DispatchArgs;

class TypePtr final {
public:
    // We store tagged pointers as 64-bit values.
    using tagged_storage = u8;

    // Type -> GroundType, ProxyType, LambdaParam,
    //         SelfTypeParam, AliasType, SelfType, TypeVar,
    //         AppliedType

    // GroundType -> ClassType, OrType, AndType
    //     ClassType -> BlamedUntyped, UnresolvedClassType, UnresolvedAppliedType

    // ProxyType -> LiteralType, ShapeType, TupleType, MetaType,

    // isa_class_type, isa_ground_type, isa_...

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

    // A mapping from tag value to whether or not the indicated type can be stored inline in a TypePtr.
    template <typename T> struct TypeToIsInline;

private:
    std::atomic<u4> *counter;
    tagged_storage ptr;

    static constexpr tagged_storage TAG_MASK = 0xffff000000000007;

    static constexpr tagged_storage PTR_MASK = ~TAG_MASK;

    static tagged_storage _tag(Tag tag, tagged_storage value) {
        auto val = static_cast<tagged_storage>(tag);
        if (val >= 8) {
            // Store the tag in the upper 16 bits of the pointer, as it won't fit in the lower three bits.
            val <<= 48;
        }

        auto maskedValue = value & PTR_MASK;

        return maskedValue | val;
    }

    static tagged_storage tagValue(Tag tag, u4 value) {
        return _tag(tag, (static_cast<tagged_storage>(value) << 3));
    }

    static tagged_storage tagPtr(Tag tag, void *expr) {
        return _tag(tag, reinterpret_cast<tagged_storage>(expr));
    }

    explicit TypePtr(Tag tag, std::atomic<u4> *counter, void *expr) : counter(counter), ptr(tagPtr(tag, expr)) {
        if (counter != nullptr) {
            counter->fetch_add(1);
        }
    }

    template <class T>
    explicit TypePtr(Tag tag, T type, bool) : counter(nullptr), ptr(tagValue(tag, type.toTypePtrValue())) {
        ENFORCE_NO_TIMER(TypeToIsInline<T>::value);
    }

    static void deleteTagged(Tag tag, void *ptr) noexcept;

    u4 untagValue() const noexcept {
        auto val = (ptr & PTR_MASK) >> 3;
        return static_cast<u4>(val);
    }

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

    void *get() const {
        auto val = ptr & PTR_MASK;
        if constexpr (sizeof(void *) == 4) {
            return reinterpret_cast<void *>(val);
        } else {
            // sign extension for the upper 16 bits
            return reinterpret_cast<void *>((val << 16) >> 16);
        }
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

    bool isUntyped() const;

    bool isNilClass() const;

    bool isBottom() const;

    // Used in subtyping.cc to order types.
    int typeKind() const;

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
    template <class To> friend To const *cast_type_const(const TypePtr &what);
    template <class To> friend To *cast_type(TypePtr &what);
    template <class To> friend To cast_inline_type_nonnull(const TypePtr &what);
    template <class T, class... Args> friend TypePtr make_inline_type(Args &&... args);
};
CheckSize(TypePtr, 16, 8);
} // namespace sorbet::core

#endif
