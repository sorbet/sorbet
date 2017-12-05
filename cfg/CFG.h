#ifndef SRUBY_CFG_H
#define SRUBY_CFG_H

#include "core/core.h"
#include <memory>
#include <unordered_map>

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
        categoryCounterInc("CFG", "StringLit");
    };
    virtual std::string toString(core::Context ctx);
};

class SymbolLit final : public Instruction {
public:
    core::NameRef value;

    SymbolLit(core::NameRef value) : value(value) {
        categoryCounterInc("CFG", "SymbolLit");
    };
    virtual std::string toString(core::Context ctx);
};

class NotSupported final : public Instruction {
public:
    std::string why;

    NotSupported(std::string why) : why(why) {
        categoryCounterInc("CFG", "NotSupported");
    };
    virtual std::string toString(core::Context ctx);
};

class BoolLit final : public Instruction {
public:
    bool value;

    BoolLit(bool value) : value(value) {
        categoryCounterInc("CFG", "BoolLit");
    };
    virtual std::string toString(core::Context ctx);
};

class ArraySplat final : public Instruction {
public:
    core::NameRef arg;

    ArraySplat(core::NameRef arg) : arg(arg) {
        categoryCounterInc("CFG", "ArraySplat");
    };
    virtual std::string toString(core::Context ctx);
};

class HashSplat final : public Instruction {
public:
    core::NameRef arg;

    HashSplat(core::NameRef arg) : arg(arg) {
        categoryCounterInc("CFG", "HashSplat");
    };
    virtual std::string toString(core::Context ctx);
};

class Self final : public Instruction {
public:
    core::SymbolRef klass;

    Self(core::SymbolRef klass) : klass(klass) {
        categoryCounterInc("CFG", "Self");
    };
    virtual std::string toString(core::Context ctx);
};

class LoadArg final : public Instruction {
public:
    core::LocalVariable receiver;
    core::NameRef method;
    u4 arg;

    LoadArg(core::LocalVariable receiver, core::NameRef method, u4 arg) : receiver(receiver), method(method), arg(arg) {
        categoryCounterInc("CFG", "LoadArg");
    };
    virtual std::string toString(core::Context ctx);
};

class BasicBlock;

class BlockExit final {
public:
    core::LocalVariable cond;
    BasicBlock *thenb;
    BasicBlock *elseb;
    core::Loc loc;
};

class Binding final {
public:
    core::LocalVariable bind;
    core::Loc loc;

    std::unique_ptr<Instruction> value;
    std::shared_ptr<core::Type> tpe;

    Binding(core::LocalVariable bind, core::Loc loc, std::unique_ptr<Instruction> value);
    Binding(Binding &&other) = default;
    Binding() = default;

    Binding &operator=(Binding &&) = default;
};

class BasicBlock final {
public:
    std::vector<core::LocalVariable> args;
    int id = 0;
    int flags = 0;
    int outerLoops = 0;
    std::vector<Binding> exprs;
    BlockExit bexit;
    std::vector<BasicBlock *> backEdges;
    core::Loc loc;
    BasicBlock() {
        counterInc("BasicBlocks");
    };

    std::string toString(core::Context ctx);
};

class CFGContext;

class CFG final {
    friend class CFGBuilder;
    /**
     * CFG owns all the BasicBlocks, and then they have raw unmanaged pointers to and between each other,
     * because they all have lifetime identical with each other and the CFG.
     */
public:
    core::SymbolRef symbol;
    int maxBasicBlockId = 0;
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks;
    /** Blocks in topoligical sort. All parent blocks are earlier than child blocks
     *
     * The name here goes from using forwards or backwards edges as dependencies in topological sort.
     * This in indeed kind-a reverse to what order would node be in: for topological sort with forward edges
     * the entry point is going to be the last node in sorted array.
     */
    std::vector<BasicBlock *> forwardsTopoSort;
    std::vector<BasicBlock *> backwardsTopoSort;
    inline BasicBlock *entry() {
        return basicBlocks[0].get();
    }

    BasicBlock *deadBlock() {
        return basicBlocks[1].get();
    };

    std::string toString(core::Context ctx);

    static int FORWARD_TOPO_SORT_VISITED;
    static int BACKWARD_TOPO_SORT_VISITED;
    static int LOOP_HEADER;
    std::unordered_map<core::LocalVariable, int> minLoops;
    std::unordered_map<core::LocalVariable, int> maxLoopWrite;

private:
    CFG();
    BasicBlock *freshBlock(int outerLoops, core::Loc loc, BasicBlock *from);
};

} // namespace cfg
} // namespace ruby_typer

#endif // SRUBY_CFG_H
