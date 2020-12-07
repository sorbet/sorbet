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

enum class Tag : u1 {
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
    Cast,
    TAbsurd,
};

// A mapping from instruction type to its corresponding tag.
template <typename T> struct InsnToTag;

class InsnPtr;
class Instruction;
template <class To> To *cast_instruction(Instruction *what);

// When adding a new subtype, see if you need to add it to fillInBlockArguments
class Instruction {
public:
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
    bool isSynthetic = false;

protected:
    Instruction(Tag tag) : tag(tag) {}
    ~Instruction() = default;

private:
    void deleteTagged();

    friend InsnPtr;
    template <class To>
    friend To *cast_instruction(Instruction *);
    const Tag tag;
};

template <class To> To *cast_instruction(Instruction *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Instruction *&, To *>::value,
                  "Ill Formed To, has to be a subclass of Instruction");
    if (what == nullptr) {
        return nullptr;
    }
    if (what->tag != InsnToTag<To>::value) {
        return nullptr;
    }
    return static_cast<To*>(what);
}

template <class To> bool isa_instruction(Instruction *what) {
    return cast_instruction<To>(what) != nullptr;
}

#define INSN(name)                              \
    class name;                                 \
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
CheckSize(Alias, 16, 8);

INSN(SolveConstraint) : public Instruction {
public:
    LocalRef send;
    std::shared_ptr<core::SendAndBlockLink> link;
    SolveConstraint(const std::shared_ptr<core::SendAndBlockLink> &link, LocalRef send) : Instruction(Tag::SolveConstraint), send(send), link(link){};
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(SolveConstraint, 24, 8);

INSN(Send) : public Instruction {
public:
    bool isPrivateOk;
    u2 numPosArgs;
    core::NameRef fun;
    VariableUseSite recv;
    core::LocOffsets receiverLoc;
    InlinedVector<VariableUseSite, 2> args;
    InlinedVector<core::LocOffsets, 2> argLocs;
    std::shared_ptr<core::SendAndBlockLink> link;

    Send(LocalRef recv, core::NameRef fun, core::LocOffsets receiverLoc, u2 numPosArgs,
         const InlinedVector<LocalRef, 2> &args, InlinedVector<core::LocOffsets, 2> argLocs, bool isPrivateOk = false,
         const std::shared_ptr<core::SendAndBlockLink> &link = nullptr);

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
CheckSize(Return, 40, 8);

INSN(BlockReturn) : public Instruction {
public:
    std::shared_ptr<core::SendAndBlockLink> link;
    VariableUseSite what;

    BlockReturn(std::shared_ptr<core::SendAndBlockLink> link, LocalRef what);
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(BlockReturn, 48, 8);

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
CheckSize(Literal, 24, 8);

INSN(GetCurrentException) : public Instruction {
public:
    GetCurrentException() : Instruction(Tag::GetCurrentException) {
        categoryCounterInc("cfg", "GetCurrentException");
    };
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(GetCurrentException, 8, 8);

INSN(LoadArg) : public Instruction {
public:
    u2 argId;
    core::MethodRef method;

    LoadArg(core::MethodRef method, u2 argId) : Instruction(Tag::LoadArg), argId(argId), method(method) {
        categoryCounterInc("cfg", "loadarg");
    };

    const core::ArgInfo &argument(const core::GlobalState &gs) const;
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(LoadArg, 8, 8);

INSN(ArgPresent) : public Instruction {
public:
    u2 argId;
    core::MethodRef method;

    ArgPresent(core::MethodRef method, u2 argId) : Instruction(Tag::ArgPresent), argId(argId), method(method) {
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

    LoadYieldParams(const std::shared_ptr<core::SendAndBlockLink> &link) : Instruction(Tag::LoadYieldParams), link(link) {
        categoryCounterInc("cfg", "loadarg");
    };
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(LoadYieldParams, 24, 8);

INSN(YieldParamPresent) : public Instruction {
public:
    u2 argId;

    YieldParamPresent(u2 argId) : argId{argId} {
        categoryCounterInc("cfg", "argpresent");
    };
    virtual std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    virtual std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(YieldParamPresent, 16, 8);

INSN(YieldLoadArg) : public Instruction {
public:
    core::ArgInfo::ArgFlags flags;
    u2 argId;
    VariableUseSite yieldParam;

    YieldLoadArg(u2 argId, core::ArgInfo::ArgFlags flags, LocalRef yieldParam)
        : flags(flags), argId(argId), yieldParam(yieldParam) {
        categoryCounterInc("cfg", "yieldloadarg");
    }
    virtual std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    virtual std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(YieldLoadArg, 40, 8);

INSN(Cast) : public Instruction {
public:
    core::NameRef cast;
    VariableUseSite value;
    core::TypePtr type;

    Cast(LocalRef value, const core::TypePtr &type, core::NameRef cast) : Instruction(Tag::Cast), cast(cast), value(value), type(type) {}

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(Cast, 48, 8);

INSN(TAbsurd) : public Instruction {
public:
    VariableUseSite what;

    TAbsurd(LocalRef what) : Instruction(Tag::TAbsurd), what(what) {
        categoryCounterInc("cfg", "tabsurd");
    }

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg, int tabs = 0) const;
};
CheckSize(TAbsurd, 32, 8);

class InsnPtr final {
    Instruction *ptr;

    void reset(Instruction *i) noexcept {
        if (ptr) {
            ptr->deleteTagged();
        }
        ptr = i;
    }
    Instruction *release() noexcept {
        auto *i = ptr;
        ptr = nullptr;
        return i;
    }

public:
    // Required for typecase
    template <class To> static bool isa(const InsnPtr &insn);
    template <class To> static const To &cast(const InsnPtr &insn);
    template <class To> static To &cast(InsnPtr &insn) {
        return const_cast<To &>(cast<To>(static_cast<const InsnPtr &>(insn)));
    }

    InsnPtr() : InsnPtr(nullptr) {}
    InsnPtr(Instruction *i) : ptr(i) {}
    ~InsnPtr() {
        if (ptr != nullptr) {
            ptr->deleteTagged();
        }
    }

    InsnPtr(const InsnPtr &) = delete;
    InsnPtr &operator=(const InsnPtr &) = delete;

    InsnPtr(InsnPtr &&other) noexcept {
        ptr = other.release();
    }
    InsnPtr &operator=(InsnPtr &&other) noexcept {
        if (*this == other) {
            return *this;
        }

        reset(other.release());
        return *this;
    }

    Instruction *operator->() const noexcept {
        return get();
    }
    Instruction *get() const noexcept {
        return ptr;
    }

    explicit operator bool() const noexcept {
        return get() != nullptr;
    }

    bool operator==(const InsnPtr &other) const noexcept {
        return get() == other.get();
    }
    bool operator!=(const InsnPtr &other) const noexcept {
        return get() != other.get();
    }

    Tag tag() const noexcept {
        return get()->tag;
    }
};

template <class To> inline bool InsnPtr::isa(const InsnPtr &what) {
    return isa_instruction<To>(what.get());
}
template <> inline bool InsnPtr::isa<InsnPtr>(const InsnPtr &what) {
    return true;
}

template <class To> inline const To &InsnPtr::cast(const InsnPtr &what) {
    return *cast_instruction<To>(what.get());
}
template <> inline const InsnPtr &InsnPtr::cast(const InsnPtr &what) {
    return what;
}

template <typename T, class... Args>
InsnPtr make_insn(Args&& ...arg) {
    return InsnPtr(new T(std::forward<Args>(arg)...));
}

} // namespace sorbet::cfg

#endif // SORBET_CFG_H
