#ifndef SRUBY_INSTRUCTIONS_H
#define SRUBY_INSTRUCTIONS_H

#include "core/core.h"
#include <climits>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace ruby_typer {
namespace cfg {

// TODO: convert it to implicitly numbered instead of explicitly bound
// implicitly numbered: result of every instruction can be uniquely referenced
// by its position in a linear array.

// When adding a new subtype, see if you need to add it to fillInBlockArguments
class Instruction {
public:
    virtual ~Instruction() = default;
    virtual std::string toString(core::Context ctx) = 0;
    Instruction() = default;
};

template <class To> To *cast_instruction(Instruction *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Instruction *&, To *>::value,
                  "Ill Formed To, has to be a subclass of Instruction");
    return fast_cast<Instruction, To>(what);
}

class Ident final : public Instruction {
public:
    core::LocalVariable what;

    Ident(core::LocalVariable what);
    virtual std::string toString(core::Context ctx);
};

class Alias final : public Instruction {
public:
    core::SymbolRef what;

    Alias(core::SymbolRef what);

    virtual std::string toString(core::Context ctx);
};

class Send final : public Instruction {
public:
    core::LocalVariable recv;
    core::NameRef fun;
    std::vector<core::LocalVariable> args;

    Send(core::LocalVariable recv, core::NameRef fun, std::vector<core::LocalVariable> &args);

    virtual std::string toString(core::Context ctx);
};

class Return final : public Instruction {
public:
    core::LocalVariable what;

    Return(core::LocalVariable what);
    virtual std::string toString(core::Context ctx);
};

class NamedArg : public Instruction {
public:
    core::NameRef name;
    core::LocalVariable value;
};

class FloatLit final : public Instruction {
public:
    double value;

    FloatLit(double value);
    virtual std::string toString(core::Context ctx);
};

class IntLit final : public Instruction {
public:
    int64_t value;

    IntLit(int64_t value);
    virtual std::string toString(core::Context ctx);
};

class StringLit final : public Instruction {
public:
    core::NameRef value;

    StringLit(core::NameRef value) : value(value) {
        categoryCounterInc("cfg", "stringlit");
    };
    virtual std::string toString(core::Context ctx);
};

class SymbolLit final : public Instruction {
public:
    core::NameRef value;

    SymbolLit(core::NameRef value) : value(value) {
        categoryCounterInc("cfg", "symbollit");
    };
    virtual std::string toString(core::Context ctx);
};

class Unanalyzable : public Instruction {
public:
    Unanalyzable() {
        categoryCounterInc("cfg", "Unanalyzable");
    };
    virtual std::string toString(core::Context ctx);
};

class NotSupported final : public Unanalyzable {
public:
    std::string why;

    NotSupported(std::string why) : why(why) {
        categoryCounterInc("cfg", "notsupported");
    };
    virtual std::string toString(core::Context ctx);
};

class BoolLit final : public Instruction {
public:
    bool value;

    BoolLit(bool value) : value(value) {
        categoryCounterInc("cfg", "boollit");
    };
    virtual std::string toString(core::Context ctx);
};

class ArraySplat final : public Instruction {
public:
    core::NameRef arg;

    ArraySplat(core::NameRef arg) : arg(arg) {
        categoryCounterInc("cfg", "arraysplat");
    };
    virtual std::string toString(core::Context ctx);
};

class HashSplat final : public Instruction {
public:
    core::NameRef arg;

    HashSplat(core::NameRef arg) : arg(arg) {
        categoryCounterInc("cfg", "hashsplat");
    };
    virtual std::string toString(core::Context ctx);
};

class Self final : public Instruction {
public:
    core::SymbolRef klass;

    Self(core::SymbolRef klass) : klass(klass) {
        categoryCounterInc("cfg", "self");
    };
    virtual std::string toString(core::Context ctx);
};

class LoadArg final : public Instruction {
public:
    core::LocalVariable receiver;
    core::NameRef method;
    u4 arg;

    LoadArg(core::LocalVariable receiver, core::NameRef method, u4 arg) : receiver(receiver), method(method), arg(arg) {
        categoryCounterInc("cfg", "loadarg");
    };
    virtual std::string toString(core::Context ctx);
};

class Cast final : public Instruction {
public:
    core::LocalVariable value;
    std::shared_ptr<core::Type> type;
    bool assertType;

    Cast(core::LocalVariable value, std::shared_ptr<core::Type> type, bool assertType)
        : value(value), type(type), assertType(assertType) {}

    virtual std::string toString(core::Context ctx);
};

} // namespace cfg
} // namespace ruby_typer

#endif // SRUBY_CFG_H
