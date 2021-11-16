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
    template <class To> static To &const_cast_type(const To &what) {
        return const_cast<To &>(what);
    }
    template <class To> static To const_cast_type(To &&what) {
        return std::move(what);
    }

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

    // A mapping from type to whether or not that type is inlined into TypePtr.
    template <typename T> struct TypeToIsInlined;

    // A mapping from type to the type returned by `cast_type_nonnull`.
    template <typename T, bool isInlined> struct TypeToCastType {};
    template <typename T> struct TypeToCastType<T, true> { using type = T; };
    template <typename T> struct TypeToCastType<T, false> { using type = const T &; };

    // Required for typecase.
    template <class To> static bool isa(const TypePtr &what);

    template <class To> static typename TypeToCastType<To, TypeToIsInlined<To>::value>::type cast(const TypePtr &what);
    // We disallow casting on temporary values because the lifetime of the returned value is
    // tied to the temporary, but it is possible for the temporary to be destroyed at the end
    // of the current statement, leading to use-after-free bugs.
    template <class To>
    static typename TypeToCastType<To, TypeToIsInlined<To>::value>::type cast(TypePtr &&what) = delete;

    template <class To> static auto cast(TypePtr &what) {
        return const_cast_type<To>(cast<To>(static_cast<const TypePtr &>(what)));
    }

    static std::string tagToString(Tag tag);

private:
    union {
        // If containsPtr()
        std::atomic<u4> *counter;
        // If !containsPtr()
        u8 value;
    };
    tagged_storage store;

    // We use a 0 to indicate not inlined so that nullptr (which has counter value 0) is naturally viewed as
    // 'inlined'.
    static constexpr tagged_storage NOT_INLINED_MASK = 0x0100;
    // We use a 8-bit tag mask so that tags can be efficiently extracted via
    // movzx on x86.
    static constexpr tagged_storage TAG_MASK = 0x00FF;

    static constexpr tagged_storage PTR_MASK = ~(NOT_INLINED_MASK | TAG_MASK);

    static tagged_storage tagToMask(Tag tag) {
        // Store the tag in the lower bits of the pointer, regardless of size.
        return static_cast<tagged_storage>(tag);
    }

    static tagged_storage tagValue(Tag tag, u4 inlinedValue) {
        auto val = tagToMask(tag);

        // Store value into val.  It doesn't much matter where we put it in
        // the upper 48 bits, but we put it in the uppermost 32 bits to
        // ensure that retrieving it requires only a shift and no masking.
        val |= static_cast<tagged_storage>(inlinedValue) << 32;

        // Asserts that tag isn't using the bit which we use to indicate that value is _not_ inlined.
        ENFORCE((val & NOT_INLINED_MASK) == 0);

        return val;
    }

    static tagged_storage tagPtr(Tag tag, void *expr) {
        auto val = tagToMask(tag);

        auto maskedPtr = reinterpret_cast<tagged_storage>(expr) << 16;

        return maskedPtr | val | NOT_INLINED_MASK;
    }

    TypePtr(Tag tag, std::atomic<u4> *counter, void *expr) : counter(counter), store(tagPtr(tag, expr)) {
        ENFORCE_NO_TIMER(counter != nullptr);
        counter->fetch_add(1);
    }

    // Inlined TypePtr constructor
    TypePtr(Tag tag, u4 value1, u8 value2) : value(value2), store(tagValue(tag, value1)) {}

    static void deleteTagged(Tag tag, void *ptr) noexcept;

    bool containsPtr() const noexcept {
        return (store & NOT_INLINED_MASK) > 0;
    }

    // A version of release that doesn't mask the tag bits
    tagged_storage releaseTagged() noexcept {
        auto saved = store;
        store = 0;
        return saved;
    }

    std::atomic<u4> *releaseCounter() noexcept {
        ENFORCE_NO_TIMER(containsPtr());
        auto saved = counter;
        counter = nullptr;
        return saved;
    }

    u8 releaseValue() noexcept {
        ENFORCE_NO_TIMER(!containsPtr());
        auto saved = value;
        value = 0;
        return saved;
    }

    void handleDelete() noexcept {
        if (containsPtr()) {
            // fetch_sub returns value prior to subtract
            const u4 counterVal = counter->fetch_sub(1) - 1;
            if (counterVal == 0) {
                deleteTagged(tag(), get());
                delete counter;
            }
        }
    }

    void _sanityCheck(const GlobalState &gs) const;

    u4 inlinedValue() const {
        ENFORCE_NO_TIMER(!containsPtr());
        auto val = store >> 32;
        return static_cast<u4>(val);
    }

    void *get() const {
        auto val = store & PTR_MASK;
        return reinterpret_cast<void *>(val >> 16);
    }

public:
    constexpr TypePtr() noexcept : value(0), store(0) {}

    TypePtr(std::nullptr_t) noexcept : TypePtr() {}

    TypePtr(TypePtr &&other) noexcept {
        if (other.containsPtr()) {
            counter = other.releaseCounter();
            ENFORCE_NO_TIMER(counter != nullptr);
        } else {
            value = other.releaseValue();
        }
        // Has to happen last to avoid releaseCounter() triggering an ENFORCE.
        store = other.releaseTagged();
    }

    TypePtr(const TypePtr &other) noexcept : store(other.store) {
        if (other.containsPtr()) {
            counter = other.counter;
            ENFORCE_NO_TIMER(counter != nullptr);
            counter->fetch_add(1);
        } else {
            value = other.value;
        }
    };

    ~TypePtr() {
        handleDelete();
    }

    TypePtr &operator=(TypePtr &&other) noexcept {
        if (*this == other) {
            return *this;
        }

        handleDelete();
        if (other.containsPtr()) {
            counter = other.releaseCounter();
        } else {
            value = other.releaseValue();
        }
        store = other.releaseTagged();
        return *this;
    };

    TypePtr &operator=(const TypePtr &other) noexcept {
        if (*this == other) {
            return *this;
        }

        handleDelete();

        if (other.containsPtr()) {
            counter = other.counter;
            if (counter != nullptr) {
                counter->fetch_add(1);
            }
        } else {
            value = other.value;
        }
        store = other.store;
        return *this;
    };

    explicit TypePtr(Tag tag, void *expr) : TypePtr(tag, new std::atomic<u4>(), expr) {}

    operator bool() const {
        return (bool)store;
    }

    Tag tag() const noexcept {
        ENFORCE_NO_TIMER(store != 0);

        auto value = reinterpret_cast<tagged_storage>(store) & TAG_MASK;
        return static_cast<Tag>(value);
    }

    bool operator!=(const TypePtr &other) const {
        // There's a lot going on in this line.
        // * If store == other.store, both `this` and `other` have the same value of `containsPtr()`.
        // * If store == other.store and both contain a pointer, there's no need to compare `counter`; they point to the
        // same Type object.
        // * If store == other.store and both do not contain a pointer, then we need to compare the inlined values.
        return store != other.store || (!containsPtr() && value != other.value);
    }
    bool operator==(const TypePtr &other) const {
        // Inverse of !=
        return store == other.store && (containsPtr() || value == other.value);
    }
    bool operator!=(std::nullptr_t n) const {
        return store != 0;
    }
    bool operator==(std::nullptr_t n) const {
        return store == 0;
    }

    bool isUntyped() const;

    bool isNilClass() const;

    bool isTrueOrFalseClass() const;

    bool isClass(ClassOrModuleRef cls) const;

    bool isBottom() const;

    // Used in subtyping.cc to order types.
    int kind() const;

    std::string typeName() const;

    bool isFullyDefined() const;

    bool hasUntyped() const;

    void sanityCheck(const GlobalState &gs) const {
        if constexpr (!debug_mode)
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

    // If this TypePtr `is_proxy_type`, returns its underlying type.
    TypePtr underlying(const GlobalState &gs) const;

    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string toString(const GlobalState &gs) const {
        return toStringWithTabs(gs);
    }

    // User visible type. Should exactly match what the user can write.
    std::string show(const GlobalState &gs) const;
    // Like show, but can include extra info. Does not necessarily match what the user can type.
    std::string showWithMoreInfo(const GlobalState &gs) const;

    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;

    u4 hash(const GlobalState &gs) const;

    DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) const;

    template <class T, class... Args> friend TypePtr make_type(Args &&...args);
    template <class To> friend To const *cast_type(const TypePtr &what);
    template <class To>
    friend typename TypeToCastType<To, TypeToIsInlined<To>::value>::type cast_type_nonnull(const TypePtr &what);
    friend class TypePtrTestHelper;
};
CheckSize(TypePtr, 16, 8);
} // namespace sorbet::core

#endif
