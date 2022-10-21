#ifndef SORBET_TYPEPTR_H
#define SORBET_TYPEPTR_H
#include "absl/types/span.h"
#include "common/common.h"
#include "core/NameRef.h"
#include "core/Polarity.h"
#include "core/ShowOptions.h"
#include "core/SymbolRef.h"
#include <memory>

namespace sorbet::core {
class TypeConstraint;
struct DispatchResult;
struct DispatchArgs;

class NonRefcounted {};

class Refcounted {
    friend class TypePtrTestHelper;
    std::atomic<uint32_t> counter{0};

public:
    void addref() {
        this->counter.fetch_add(1);
    }

    uint32_t release() {
        // fetch_sub returns value prior to subtract
        return this->counter.fetch_sub(1) - 1;
    }
};

class TypePtr final {
    template <class To> static To &const_cast_type(const To &what) {
        return const_cast<To &>(what);
    }
    template <class To> static To const_cast_type(To &&what) {
        return std::move(what);
    }

public:
    // We store tagged pointers as 64-bit values.
    using tagged_storage = uint64_t;

    enum class Tag {
        ClassType = 1,
        LambdaParam,
        SelfTypeParam,
        AliasType,
        SelfType,
        IntegerLiteralType,
        FloatLiteralType,
        NamedLiteralType,
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

    // Disallowing this and requiring people to cast to `const TypePtr &` is simplier
    // than creating a parallel `TypeToCastType` for non-const argument types.
    template <class To> static auto cast(TypePtr &what) = delete;

    static std::string tagToString(Tag tag);

private:
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

    static tagged_storage tagValue(Tag tag, uint64_t inlinedValue) {
        // Ensure the upper bits aren't getting used for anything exciting.
        ENFORCE_NO_TIMER((inlinedValue >> 48) == 0);
        auto val = tagToMask(tag);

        // Store value into val.  Put it in the same place as pointer values.
        val |= static_cast<tagged_storage>(inlinedValue) << 16;

        // Asserts that tag isn't using the bit which we use to indicate that value is _not_ inlined.
        ENFORCE((val & NOT_INLINED_MASK) == 0);

        return val;
    }

    static tagged_storage tagPtr(Tag tag, Refcounted *expr) {
        auto val = tagToMask(tag);

        auto maskedPtr = reinterpret_cast<tagged_storage>(expr) << 16;

        return maskedPtr | val | NOT_INLINED_MASK;
    }

    // Inlined TypePtr constructor
    TypePtr(Tag tag, uint64_t inlinedValue) noexcept : store(tagValue(tag, inlinedValue)) {}

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

    void handleDelete() noexcept {
        if (containsPtr()) {
            auto *ptr = this->get();
            const uint32_t counterVal = ptr->release();
            if (counterVal == 0) {
                deleteTagged(tag(), ptr);
            }
        }
    }

    void _sanityCheck(const GlobalState &gs) const;

    uint64_t inlinedValue() const {
        ENFORCE_NO_TIMER(!containsPtr());
        auto val = store >> 16;
        return val;
    }

    Refcounted *get() const {
        auto val = store & PTR_MASK;
        return reinterpret_cast<Refcounted *>(val >> 16);
    }

public:
    constexpr TypePtr() noexcept : store(0) {}

    TypePtr(std::nullptr_t) noexcept : TypePtr() {}

    TypePtr(TypePtr &&other) noexcept {
        store = other.releaseTagged();
    }

    TypePtr(const TypePtr &other) noexcept : store(other.store) {
        if (other.containsPtr()) {
            this->get()->addref();
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
        store = other.releaseTagged();
        return *this;
    };

    TypePtr &operator=(const TypePtr &other) noexcept {
        if (*this == other) {
            return *this;
        }

        handleDelete();
        if (other.containsPtr()) {
            other.get()->addref();
        }
        store = other.store;
        return *this;
    };

    explicit TypePtr(Tag tag, Refcounted *expr) noexcept : store(tagPtr(tag, expr)) {
        expr->addref();
    }

    operator bool() const {
        return (bool)store;
    }

    Tag tag() const noexcept {
        ENFORCE_NO_TIMER(store != 0);

        auto value = reinterpret_cast<tagged_storage>(store) & TAG_MASK;
        return static_cast<Tag>(value);
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

    bool isTop() const;

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

    // Converts a type like this:
    //   T.proc.params(arg0: Integer, arg1: Integer).void
    // into this:
    //   [Integer, Integer]
    // for use with LoadYieldParams
    TypePtr getCallArguments(const GlobalState &gs, NameRef name) const;

    TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const;

    TypePtr _replaceSelfType(const GlobalState &gs, const TypePtr &receiver) const;

    TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) const;

    TypePtr _instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                         const std::vector<TypePtr> &targs) const;

    // If this TypePtr `is_proxy_type`, returns its underlying type.
    TypePtr underlying(const GlobalState &gs) const;

    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string toString(const GlobalState &gs) const {
        return toStringWithTabs(gs);
    }

    // User visible type. Should exactly match what the user can write.
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    // Like show, but can include extra info. Does not necessarily match what the user can type.
    std::string showWithMoreInfo(const GlobalState &gs) const;

    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;

    uint32_t hash(const GlobalState &gs) const;

    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;

    template <class T, class... Args> friend TypePtr make_type(Args &&...args);
    template <class To> friend To const *cast_type(const TypePtr &what);
    template <class To>
    friend typename TypeToCastType<To, TypeToIsInlined<To>::value>::type cast_type_nonnull(const TypePtr &what);
    friend class TypePtrTestHelper;
};
CheckSize(TypePtr, 8, 8);
} // namespace sorbet::core

#endif
