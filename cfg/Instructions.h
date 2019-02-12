#ifndef SORBET_INSTRUCTIONS_H
#define SORBET_INSTRUCTIONS_H

#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/LocalVariable.h"
#include "core/NameRef.h"
#include "core/Types.h"
#include <climits>
#include <memory>

namespace sorbet::cfg {

class VariableUseSite {
public:
    core::LocalVariable variable;
    core::TypePtr type;
    VariableUseSite() = default;
    VariableUseSite(core::LocalVariable local) : variable(local){};
    VariableUseSite(const VariableUseSite &) = delete;
    const VariableUseSite &operator=(const VariableUseSite &rhs) = delete;
    VariableUseSite(VariableUseSite &&) = default;
    VariableUseSite &operator=(VariableUseSite &&rhs) = default;
    std::string toString(core::Context ctx) const;
};

// TODO: convert it to implicitly numbered instead of explicitly bound
// implicitly numbered: result of every instruction can be uniquely referenced
// by its position in a linear array.

// When adding a new subtype, see if you need to add it to fillInBlockArguments
class Instruction {
public:
    virtual ~Instruction() = default;
    virtual std::string toString(core::Context ctx) = 0;
    Instruction() = default;
    bool isSynthetic = false;
};

template <class To> To *cast_instruction(Instruction *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Instruction *&, To *>::value,
                  "Ill Formed To, has to be a subclass of Instruction");
    return fast_cast<Instruction, To>(what);
}

template <class To> bool isa_instruction(Instruction *what) {
    return cast_instruction<To>(what) != nullptr;
}

class Ident final : public Instruction {
public:
    core::LocalVariable what;

    Ident(core::LocalVariable what);
    virtual std::string toString(core::Context ctx);
};
CheckSize(Ident, 24, 8);

class Alias final : public Instruction {
public:
    core::SymbolRef what;

    Alias(core::SymbolRef what);

    virtual std::string toString(core::Context ctx);
};
CheckSize(Alias, 16, 8);

class SolveConstraint final : public Instruction {
public:
    std::shared_ptr<core::SendAndBlockLink> link;
    SolveConstraint(const std::shared_ptr<core::SendAndBlockLink> &link) : link(link){};
    virtual std::string toString(core::Context ctx);
};
CheckSize(SolveConstraint, 32, 8);

class Send final : public Instruction {
public:
    VariableUseSite recv;
    core::NameRef fun;
    core::Loc receiverLoc;
    InlinedVector<VariableUseSite, 2> args;
    InlinedVector<core::Loc, 2> argLocs;
    std::shared_ptr<core::SendAndBlockLink> link;

    Send(core::LocalVariable recv, core::NameRef fun, core::Loc receiverLoc,
         const InlinedVector<core::LocalVariable, 2> &args, const InlinedVector<core::Loc, 2> &argLocs,
         const std::shared_ptr<core::SendAndBlockLink> &link = nullptr);

    virtual std::string toString(core::Context ctx);
};
CheckSize(Send, 152, 8);

class Return final : public Instruction {
public:
    VariableUseSite what;

    Return(core::LocalVariable what);
    virtual std::string toString(core::Context ctx);
};
CheckSize(Return, 40, 8);

class BlockReturn final : public Instruction {
public:
    std::shared_ptr<core::SendAndBlockLink> link;
    VariableUseSite what;

    BlockReturn(const std::shared_ptr<core::SendAndBlockLink> &link, core::LocalVariable what);
    virtual std::string toString(core::Context ctx);
};
CheckSize(BlockReturn, 56, 8);

class Literal final : public Instruction {
public:
    core::TypePtr value;

    Literal(const core::TypePtr &value);
    virtual std::string toString(core::Context ctx);
};
CheckSize(Literal, 32, 8);

class Unanalyzable : public Instruction {
public:
    Unanalyzable() {
        categoryCounterInc("cfg", "unanalyzable");
    };
    virtual std::string toString(core::Context ctx);
};
CheckSize(Unanalyzable, 16, 8);

class NotSupported final : public Unanalyzable {
public:
    std::string why;

    NotSupported(std::string_view why) : why(why) {
        categoryCounterInc("cfg", "notsupported");
    };
    virtual std::string toString(core::Context ctx);
};
CheckSize(NotSupported, 40, 8);

class Self final : public Instruction {
public:
    core::SymbolRef klass;

    Self(core::SymbolRef klass) : klass(klass) {
        categoryCounterInc("cfg", "self");
    };
    virtual std::string toString(core::Context ctx);
};
CheckSize(Self, 16, 8);

class LoadArg final : public Instruction {
public:
    core::SymbolRef arg;

    LoadArg(core::SymbolRef arg) : arg(arg) {
        categoryCounterInc("cfg", "loadarg");
    };
    virtual std::string toString(core::Context ctx);
};
CheckSize(LoadArg, 16, 8);

class LoadYieldParams final : public Instruction {
public:
    std::shared_ptr<core::SendAndBlockLink> link;
    core::SymbolRef block;

    LoadYieldParams(const std::shared_ptr<core::SendAndBlockLink> &link, core::SymbolRef blk) : link(link), block(blk) {
        categoryCounterInc("cfg", "loadarg");
    };
    virtual std::string toString(core::Context ctx);
};
CheckSize(LoadYieldParams, 40, 8);

class Cast final : public Instruction {
public:
    VariableUseSite value;
    core::TypePtr type;
    core::NameRef cast;

    Cast(core::LocalVariable value, const core::TypePtr &type, core::NameRef cast)
        : value(value), type(type), cast(cast) {}

    virtual std::string toString(core::Context ctx);
};
CheckSize(Cast, 64, 8);

class DebugEnvironment final : public Instruction {
public:
    std::string str;
    core::GlobalState::AnnotationPos pos;

    DebugEnvironment(core::GlobalState::AnnotationPos pos);
    virtual std::string toString(core::Context ctx);
};
CheckSize(DebugEnvironment, 48, 8);

} // namespace sorbet::cfg

#endif // SORBET_CFG_H
