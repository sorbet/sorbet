#ifndef SORBET_INSTRUCTIONS_H
#define SORBET_INSTRUCTIONS_H

#include "cfg/LocalRef.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/LocalVariable.h"
#include "core/NameRef.h"
#include "core/Types.h"
#include <climits>
#include <memory>

namespace sorbet::cfg {
class VariableUseSite final {
public:
    LocalRef variable;
    core::TypePtr type;
    VariableUseSite() = default;
    VariableUseSite(LocalRef local) : variable(local){};
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
};

// A mapping from instruction type to its corresponding tag.
template <typename T> struct InsnToTag;

class InstructionPtr;
class Instruction;

// When adding a new subtype, see if you need to add it to fillInBlockArguments
class Instruction {
protected:
    Instruction() = default;
    ~Instruction() = default;

private:
    friend InstructionPtr;
};

#define INSN(name)                                                                  \
    class name;                                                                     \
    template <> struct InsnToTag<name> { static constexpr Tag value = Tag::name; }; \
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
    SolveConstraint(const std::shared_ptr<core::SendAndBlockLink> &link, LocalRef send) : send(send), link(link) {
        categoryCounterInc("cfg", "solveconstraint");
    };
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(SolveConstraint, 24, 8);

INSN(Send) : public Instruction {
public:
    bool isPrivateOk;
    uint16_t numPosArgs;
    core::NameRef fun;
    VariableUseSite recv;
    core::LocOffsets funLoc;
    core::LocOffsets receiverLoc;
    InlinedVector<VariableUseSite, 2> args;
    InlinedVector<core::LocOffsets, 2> argLocs;
    std::shared_ptr<core::SendAndBlockLink> link;

    Send(LocalRef recv, core::LocOffsets receiverLoc, core::NameRef fun, core::LocOffsets funLoc, uint16_t numPosArgs,
         const InlinedVector<LocalRef, 2> &args, InlinedVector<core::LocOffsets, 2> argLocs, bool isPrivateOk = false,
         const std::shared_ptr<core::SendAndBlockLink> &link = nullptr);

    core::LocOffsets locWithoutBlock(core::LocOffsets bindLoc);

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(Send, 144, 8);

INSN(Return) : public Instruction {
public:
    VariableUseSite what;
    core::LocOffsets whatLoc;

    Return(LocalRef what, core::LocOffsets whatLoc);
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(Return, 32, 8);

INSN(BlockReturn) : public Instruction {
public:
    std::shared_ptr<core::SendAndBlockLink> link;
    VariableUseSite what;

    BlockReturn(std::shared_ptr<core::SendAndBlockLink> link, LocalRef what);
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(BlockReturn, 40, 8);

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
CheckSize(Literal, 16, 8);

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

    const core::ArgInfo &argument(const core::GlobalState &gs) const;
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

    const core::ArgInfo &argument(const core::GlobalState &gs) const;
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(ArgPresent, 8, 8);

INSN(LoadYieldParams) : public Instruction {
public:
    std::shared_ptr<core::SendAndBlockLink> link;

    LoadYieldParams(const std::shared_ptr<core::SendAndBlockLink> &link) : link(link) {
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
    core::ArgInfo::ArgFlags flags;
    uint16_t argId;
    VariableUseSite yieldParam;

    YieldLoadArg(uint16_t argId, core::ArgInfo::ArgFlags flags, LocalRef yieldParam)
        : flags(flags), argId(argId), yieldParam(yieldParam) {
        categoryCounterInc("cfg", "yieldloadarg");
    }
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(YieldLoadArg, 32, 8);

INSN(Cast) : public Instruction {
public:
    core::NameRef cast;
    VariableUseSite value;
    core::TypePtr type;

    Cast(LocalRef value, const core::TypePtr &type, core::NameRef cast) : cast(cast), value(value), type(type) {
        categoryCounterInc("cfg", "cast");
    }

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(Cast, 48, 8);

INSN(TAbsurd) : public Instruction {
public:
    VariableUseSite what;

    TAbsurd(LocalRef what) : what(what) {
        categoryCounterInc("cfg", "tabsurd");
    }

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(TAbsurd, 24, 8);

class InstructionPtr final {
    using tagged_storage = uint64_t;

    static constexpr tagged_storage TAG_MASK = 0xff;
    static constexpr tagged_storage FLAG_MASK = 0xff00;
    static constexpr tagged_storage SYNTHETIC_FLAG = 0x0100;
    static_assert((TAG_MASK & FLAG_MASK) == 0, "no bits should be shared between tags and flags");
    static constexpr tagged_storage PTR_MASK = ~(FLAG_MASK | TAG_MASK);

    tagged_storage ptr;

    template <typename T, typename... Args> friend InstructionPtr make_insn(Args &&...);

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
        return get() != nullptr;
    }

    bool operator==(const InstructionPtr &other) const noexcept {
        return get() == other.get();
    }
    bool operator!=(const InstructionPtr &other) const noexcept {
        return get() != other.get();
    }

    Tag tag() const noexcept {
        ENFORCE(ptr != 0);

        return static_cast<Tag>(ptr & TAG_MASK);
    }

    bool isSynthetic() const noexcept {
        return (this->ptr & SYNTHETIC_FLAG) != 0;
    }

    void setSynthetic() noexcept {
        this->ptr |= SYNTHETIC_FLAG;
    }

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};

template <class To> bool isa_instruction(const InstructionPtr &what) {
    return what != nullptr && what.tag() == InsnToTag<To>::value;
}

template <class To> To *cast_instruction(InstructionPtr &what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Instruction *&, To *>::value,
                  "Ill Formed To, has to be a subclass of Instruction");
    if (isa_instruction<To>(what)) {
        return reinterpret_cast<To *>(what.get());
    }
    return nullptr;
}

template <class To> const To *cast_instruction(const InstructionPtr &what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Instruction *&, To *>::value,
                  "Ill Formed To, has to be a subclass of Instruction");
    if (isa_instruction<To>(what)) {
        return reinterpret_cast<const To *>(what.get());
    }
    return nullptr;
}

template <class To> inline bool InstructionPtr::isa(const InstructionPtr &what) {
    return isa_instruction<To>(what);
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
