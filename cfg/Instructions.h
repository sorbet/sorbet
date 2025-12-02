#ifndef SORBET_INSTRUCTIONS_H
#define SORBET_INSTRUCTIONS_H

#include "cfg/LocalRef.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/LocalVariable.h"
#include "core/NameRef.h"
#include "core/TrailingObjects.h"
#include "core/Types.h"
#include "core/UntaggedPtr.h"
#include "core/ZippedPair.h"
#include <climits>
#include <memory>

namespace sorbet::cfg {
class VariableUseSite final {
public:
    LocalRef variable;
    core::TypePtr type;
    VariableUseSite() = default;
    VariableUseSite(LocalRef local) : variable(local){};
    VariableUseSite(LocalRef local, core::TypePtr type) : variable(local), type(std::move(type)){};
    VariableUseSite(const VariableUseSite &) = delete;
    const VariableUseSite &operator=(const VariableUseSite &rhs) = delete;
    VariableUseSite(VariableUseSite &&) = default;
    VariableUseSite &operator=(VariableUseSite &&rhs) = default;
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};

// TODO: convert it to implicitly numbered instead of explicitly bound
// implicitly numbered: result of every instruction can be uniquely referenced
// by its position in a linear array.

enum class Tag : uint8_t {
    Ident = 1,
    Alias,
    SolveConstraint,
    Send,
    Return,
    BlockReturn,
    LoadSelf,
    Literal,
    GetCurrentException,
    LoadArg,
    ArgPresent,
    LoadYieldParams,
    YieldParamPresent,
    YieldLoadArg,
    Cast,
    TAbsurd,
    KeepAlive,
};

// A mapping from instruction type to its corresponding tag.
template <typename T> struct InsnToTag;

class InstructionPtr;
class Instruction;
class Send;

// When adding a new subtype, see if you need to add it to fillInBlockArguments
class Instruction {
protected:
    Instruction() = default;
    ~Instruction() = default;

private:
    friend InstructionPtr;
};

class InstructionPtr final {
    using tagged_storage = uint64_t;

    static constexpr tagged_storage TAG_MASK = 0xff;
    static constexpr tagged_storage FLAG_MASK = 0xff00;
    static constexpr tagged_storage SYNTHETIC_FLAG = 0x0100;
    static_assert((TAG_MASK & FLAG_MASK) == 0, "no bits should be shared between tags and flags");
    static constexpr tagged_storage PTR_MASK = ~(FLAG_MASK | TAG_MASK);

    tagged_storage ptr;

    template <typename T, typename... Args> friend InstructionPtr make_insn(Args &&...);
    friend Send;

    static tagged_storage tagPtr(Tag tag, void *i) {
        auto val = static_cast<tagged_storage>(tag);
        auto maskedPtr = reinterpret_cast<tagged_storage>(i) << 16;

        return maskedPtr | val;
    }

    static void deleteTagged(Tag tag, void *ptr) noexcept;

    InstructionPtr(Tag tag, Instruction *i) : ptr(tagPtr(tag, i)) {}

    void resetTagged(tagged_storage i) noexcept {
        Tag tagVal;
        void *saved = nullptr;

        if (ptr != 0) {
            tagVal = tag();
            saved = get();
        }

        ptr = i;

        if (saved != nullptr) {
            deleteTagged(tagVal, saved);
        }
    }
    tagged_storage releaseTagged() noexcept {
        auto i = ptr;
        ptr = 0;
        return i;
    }

    uint16_t tagMaybeZero() const noexcept {
        return ptr & TAG_MASK;
    }

public:
    // Required for typecase
    template <class To> static bool isa(const InstructionPtr &insn);
    template <class To> static const To &cast(const InstructionPtr &insn);
    template <class To> static To &cast(InstructionPtr &insn) {
        return const_cast<To &>(cast<To>(static_cast<const InstructionPtr &>(insn)));
    }

    constexpr InstructionPtr() noexcept : ptr(0) {}
    constexpr InstructionPtr(std::nullptr_t) : ptr(0) {}
    ~InstructionPtr() {
        if (ptr != 0) {
            deleteTagged(tag(), get());
        }
    }

    InstructionPtr(const InstructionPtr &) = delete;
    InstructionPtr &operator=(const InstructionPtr &) = delete;

    InstructionPtr(InstructionPtr &&other) noexcept {
        ptr = other.releaseTagged();
    }
    InstructionPtr &operator=(InstructionPtr &&other) noexcept {
        if (*this == other) {
            return *this;
        }

        resetTagged(other.releaseTagged());
        return *this;
    }

    Instruction *operator->() const noexcept {
        return get();
    }
    Instruction *get() const noexcept {
        auto val = ptr & PTR_MASK;
        return reinterpret_cast<Instruction *>(val >> 16);
    }

    explicit operator bool() const noexcept {
        return ptr != 0;
    }

    bool operator==(const InstructionPtr &other) const noexcept {
        return ptr == other.ptr;
    }
    bool operator==(std::nullptr_t) const noexcept {
        return ptr == 0;
    }
    bool operator!=(const InstructionPtr &other) const noexcept {
        return ptr != other.ptr;
    }
    bool operator!=(std::nullptr_t) const noexcept {
        return ptr != 0;
    }

    Tag tag() const noexcept {
        ENFORCE(ptr != 0);

        return static_cast<Tag>(ptr & TAG_MASK);
    }

    bool isSynthetic() const noexcept {
        return (this->ptr & SYNTHETIC_FLAG) != 0;
    }

    template <class To> core::UntaggedPtr<To> as_instruction() {
        bool isValid = tagMaybeZero() == uint16_t(InsnToTag<To>::value);
        auto *ptr = isValid ? reinterpret_cast<To *>(get()) : nullptr;
        return core::UntaggedPtr<To>(ptr, isValid);
    }

    template <class To> core::UntaggedPtr<const To> as_instruction() const {
        bool isValid = tagMaybeZero() == uint16_t(InsnToTag<To>::value);
        auto *ptr = isValid ? reinterpret_cast<const To *>(get()) : nullptr;
        return core::UntaggedPtr<const To>(ptr, isValid);
    }

    void setSynthetic() noexcept {
        this->ptr |= SYNTHETIC_FLAG;
    }

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};

#define INSN(name)                              \
    class name;                                 \
    template <> struct InsnToTag<name> {        \
        static constexpr Tag value = Tag::name; \
    };                                          \
    class __attribute__((aligned(8))) name final

INSN(Ident) : public Instruction {
public:
    LocalRef what;

    Ident(LocalRef what);
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(Ident, 8, 8);

INSN(Alias) : public Instruction {
public:
    core::SymbolRef what;
    core::NameRef name;

    Alias(core::SymbolRef what, core::NameRef name = core::NameRef::noName());

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(Alias, 8, 8);

INSN(SolveConstraint) : public Instruction {
public:
    LocalRef send;
    std::shared_ptr<core::SendAndBlockLink> link;
    SolveConstraint(std::shared_ptr<core::SendAndBlockLink> link, LocalRef send) : send(send), link(std::move(link)) {
        categoryCounterInc("cfg", "solveconstraint");
    };
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(SolveConstraint, 24, 8);

INSN(Send) : public Instruction, private core::TrailingObjects<Send, LocalRef, core::TypePtr, core::LocOffsets> {
    friend core::TrailingObjects<Send, LocalRef, core::TypePtr, core::LocOffsets>;
    using Parent = core::TrailingObjects<Send, LocalRef, core::TypePtr, core::LocOffsets>;

    template <typename T> absl::Span<T> span() {
        return absl::MakeSpan(getTrailingObjects<T>(), numArgs);
    }
    template <typename T> absl::Span<const T> span() const {
        return absl::MakeSpan(getTrailingObjects<T>(), numArgs);
    }

    Send(LocalRef recv, core::LocOffsets receiverLoc, core::NameRef fun, core::LocOffsets funLoc, uint16_t numPosArgs,
         bool isPrivateOk, size_t numArgs);

public:
    bool isPrivateOk;
    uint16_t numPosArgs;
    core::NameRef fun;
    VariableUseSite recv;
    core::LocOffsets funLoc;
    core::LocOffsets receiverLoc;
    size_t numArgs;
    std::shared_ptr<core::SendAndBlockLink> link;

    // We only need this for the first two sets of trailing types, but it's
    // defined identically for all three types and it's convenient to have it
    // for defining `span`, above.
    template <typename T> size_t numTrailingObjects(OverloadToken<T>) const {
        return numArgs;
    }

    class SendInitializer {
        // We want to hide the details of the uninitialized memory for the trailing
        // fields of `Send` from the client, and this class is how we're going to
        // do it: it is effectively a wrapper around a pointer that knows how to
        // use placement new for assignments.
        template <typename T> class SlotInit {
            // Note that these point to _uninitialized_ memory.
            T *current;
            T *end;

        public:
            SlotInit(absl::Span<T> span) : current(span.begin()), end(span.end()) {}

        public:
            SlotInit &operator=(const T &value) {
                new (current) T(value);
                return *this;
            }

            // Not necessary, but it makes the code look a little more idiomatic,
            // letting you write something like:
            //
            // *slot++ = val;
            //
            // Just like you would with a pointer.
            SlotInit &operator*() {
                return *this;
            }

            SlotInit &operator++() {
                ENFORCE(current != end);
                ENFORCE(current < end);
                ++current;
                return *this;
            }

            SlotInit operator++(int) {
                SlotInit copy(*this);
                ++*this;
                return copy;
            }
        };

        Send *snd;

    public:
        SendInitializer(Send *snd);

        SlotInit<LocalRef> refs;
        SlotInit<core::LocOffsets> locs;

        InstructionPtr asInsnPtr() && {
            return InstructionPtr(InsnToTag<Send>::value, snd);
        }
    };

    static SendInitializer make(LocalRef recv, core::LocOffsets receiverLoc, core::NameRef fun, core::LocOffsets funLoc,
                                uint16_t numPosArgs, bool isPrivateOk, size_t numArgs);

    absl::Span<LocalRef> argRefs() {
        return span<LocalRef>();
    }
    absl::Span<const LocalRef> argRefs() const {
        return span<LocalRef>();
    }
    absl::Span<core::TypePtr> argTypes() {
        return span<core::TypePtr>();
    }
    absl::Span<const core::TypePtr> argTypes() const {
        return span<core::TypePtr>();
    }
    absl::Span<core::LocOffsets> argLocs() {
        return span<core::LocOffsets>();
    }

    core::ZippedPairSpan<LocalRef, core::TypePtr> argSpan() {
        return core::ZippedPairSpan<LocalRef, core::TypePtr>{argRefs(), argTypes()};
    }
    core::ZippedPairSpan<const LocalRef, const core::TypePtr> argSpan() const {
        return core::ZippedPairSpan<const LocalRef, const core::TypePtr>{argRefs(), argTypes()};
    }

    ~Send();

    core::LocOffsets locWithoutBlock(core::LocOffsets bindLoc);

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(Send, 120, 8);

INSN(Return) : public Instruction {
public:
    VariableUseSite what;
    core::LocOffsets whatLoc;

    Return(LocalRef what, core::LocOffsets whatLoc);
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(Return, 24, 8);

INSN(BlockReturn) : public Instruction {
public:
    std::shared_ptr<core::SendAndBlockLink> link;
    VariableUseSite what;

    BlockReturn(std::shared_ptr<core::SendAndBlockLink> link, LocalRef what);
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(BlockReturn, 32, 8);

INSN(LoadSelf) : public Instruction {
public:
    LocalRef fallback;
    std::shared_ptr<core::SendAndBlockLink> link;
    LoadSelf(std::shared_ptr<core::SendAndBlockLink> link, LocalRef fallback);
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(LoadSelf, 24, 8);

INSN(Literal) : public Instruction {
public:
    core::TypePtr value;

    Literal(const core::TypePtr &value);
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(Literal, 8, 8);

INSN(GetCurrentException) : public Instruction {
public:
    GetCurrentException() {
        categoryCounterInc("cfg", "GetCurrentException");
    };
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(GetCurrentException, 8, 8);

INSN(LoadArg) : public Instruction {
public:
    uint16_t argId;
    core::MethodRef method;

    LoadArg(core::MethodRef method, uint16_t argId) : argId(argId), method(method) {
        categoryCounterInc("cfg", "loadarg");
    };

    const core::ParamInfo &argument(const core::GlobalState &gs) const;
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(LoadArg, 8, 8);

INSN(ArgPresent) : public Instruction {
public:
    uint16_t argId;
    core::MethodRef method;

    ArgPresent(core::MethodRef method, uint16_t argId) : argId(argId), method(method) {
        categoryCounterInc("cfg", "argpresent");
    }

    const core::ParamInfo &argument(const core::GlobalState &gs) const;
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(ArgPresent, 8, 8);

INSN(LoadYieldParams) : public Instruction {
public:
    std::shared_ptr<core::SendAndBlockLink> link;

    LoadYieldParams(std::shared_ptr<core::SendAndBlockLink> link) : link(std::move(link)) {
        categoryCounterInc("cfg", "loadyieldparams");
    };
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(LoadYieldParams, 16, 8);

INSN(YieldParamPresent) : public Instruction {
public:
    uint16_t argId;

    YieldParamPresent(uint16_t argId) : argId{argId} {
        categoryCounterInc("cfg", "yieldparampresent");
    };
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(YieldParamPresent, 8, 8);

INSN(YieldLoadArg) : public Instruction {
public:
    core::ParamInfo::Flags flags;
    uint16_t argId;
    VariableUseSite yieldParam;

    YieldLoadArg(uint16_t argId, core::ParamInfo::Flags flags, LocalRef yieldParam)
        : flags(flags), argId(argId), yieldParam(yieldParam) {
        categoryCounterInc("cfg", "yieldloadarg");
    }
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(YieldLoadArg, 24, 8);

INSN(Cast) : public Instruction {
public:
    core::NameRef cast;
    VariableUseSite value;
    core::LocOffsets valueLoc;
    core::TypePtr type;

    Cast(LocalRef value, core::LocOffsets valueLoc, const core::TypePtr &type, core::NameRef cast)
        : cast(cast), value(value), valueLoc(valueLoc), type(type) {
        categoryCounterInc("cfg", "cast");
    }

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(Cast, 40, 8);

INSN(TAbsurd) : public Instruction {
public:
    VariableUseSite what;

    TAbsurd(LocalRef what) : what(what) {
        categoryCounterInc("cfg", "tabsurd");
    }

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(TAbsurd, 16, 8);

INSN(KeepAlive) : public Instruction {
public:
    LocalRef what;

    KeepAlive(LocalRef what) : what(what) {
        categoryCounterInc("cfg", "volatileread");
    }

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(KeepAlive, 8, 8);

template <class To> bool isa_instruction(const InstructionPtr &what) {
    return bool(what.as_instruction<To>());
}

template <class To> core::UntaggedPtr<To> cast_instruction(InstructionPtr &what) {
    static_assert(!std::is_pointer_v<To>, "To has to be a pointer");
    static_assert(std::is_assignable_v<Instruction *&, To *>, "Ill Formed To, has to be a subclass of Instruction");
    return what.as_instruction<To>();
}

template <class To> core::UntaggedPtr<const To> cast_instruction(const InstructionPtr &what) {
    static_assert(!std::is_pointer_v<To>, "To has to be a pointer");
    static_assert(std::is_assignable_v<Instruction *&, To *>, "Ill Formed To, has to be a subclass of Instruction");
    return what.as_instruction<To>();
}

template <class To> inline bool InstructionPtr::isa(const InstructionPtr &what) {
    return bool(what.as_instruction<To>());
}
template <> inline bool InstructionPtr::isa<InstructionPtr>(const InstructionPtr &what) {
    return true;
}

template <class To> inline const To &InstructionPtr::cast(const InstructionPtr &what) {
    return *cast_instruction<To>(what);
}
template <> inline const InstructionPtr &InstructionPtr::cast(const InstructionPtr &what) {
    return what;
}

template <typename T, class... Args> InstructionPtr make_insn(Args &&...arg) {
    return InstructionPtr(InsnToTag<T>::value, new T(std::forward<Args>(arg)...));
}

} // namespace sorbet::cfg

#endif // SORBET_CFG_H
