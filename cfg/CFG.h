#ifndef SRUBY_CFG_H
#define SRUBY_CFG_H

#include "../ast/Trees.h"
#include "ast/ast.h"
#include "core/core.h"
#include "parser/parser.h"
#include <memory>

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

class Ident : public Instruction {
public:
    core::SymbolRef what;

    Ident(core::SymbolRef what);
    virtual std::string toString(core::Context ctx);
};

class Send : public Instruction {
public:
    core::SymbolRef recv;
    core::NameRef fun;
    std::vector<core::SymbolRef> args;

    Send(core::SymbolRef recv, core::NameRef fun, std::vector<core::SymbolRef> &args);

    virtual std::string toString(core::Context ctx);
};

class Return : public Instruction {
public:
    core::SymbolRef what;

    Return(core::SymbolRef what);
    virtual std::string toString(core::Context ctx);
};

class New : public Instruction {
public:
    core::SymbolRef klass;
    std::vector<core::SymbolRef> args;

    New(core::SymbolRef klass, std::vector<core::SymbolRef> &args);
    virtual std::string toString(core::Context ctx);
};

class Super : public Instruction {
public:
    std::vector<core::SymbolRef> args;

    Super(std::vector<core::SymbolRef> &args);
    virtual std::string toString(core::Context ctx);
};

class NamedArg : public Instruction {
public:
    core::NameRef name;
    core::SymbolRef value;
};

class FloatLit : public Instruction {
public:
    float value;

    FloatLit(float value);
    virtual std::string toString(core::Context ctx);
};

class IntLit : public Instruction {
public:
    int value;

    IntLit(int value);
    virtual std::string toString(core::Context ctx);
};

class StringLit : public Instruction {
public:
    core::NameRef value;

    StringLit(core::NameRef value) : value(value){};
    virtual std::string toString(core::Context ctx);
};

class NotSupported : public Instruction {
public:
    std::string why;

    NotSupported(std::string why) : why(why){};
    virtual std::string toString(core::Context ctx);
};

class BoolLit : public Instruction {
public:
    bool value;

    BoolLit(bool value) : value(value){};
    virtual std::string toString(core::Context ctx);
};

class ArraySplat : public Instruction {
public:
    core::NameRef arg;

    ArraySplat(core::NameRef arg) : arg(arg){};
};

class HashSplat : public Instruction {
public:
    core::NameRef arg;

    HashSplat(core::NameRef arg) : arg(arg){};
};

class Nil : public Instruction {
public:
    Nil(){};
    virtual std::string toString(core::Context ctx);
};

class Self : public Instruction {
public:
    core::SymbolRef klass;

    Self(core::SymbolRef klass) : klass(klass){};
    virtual std::string toString(core::Context ctx);
};

class LoadArg : public Instruction {
public:
    core::SymbolRef receiver;
    core::NameRef method;
    u4 arg;

    LoadArg(core::SymbolRef receiver, core::NameRef method, u4 arg) : receiver(receiver), method(method), arg(arg){};
    virtual std::string toString(core::Context ctx);
};

class BasicBlock;

class BlockExit {
public:
    core::SymbolRef cond;
    BasicBlock *thenb;
    BasicBlock *elseb;
};

class Binding {
public:
    /**
     * This is inefficient as it pollutes global symbol table and in the future (when we start working on perfromance)
     * we should consider using references to instructions similarly to how LuaJIT does it.
     */
    core::SymbolRef bind;
    core::Loc loc;

    std::unique_ptr<Instruction> value;
    std::shared_ptr<core::Type> tpe;

    Binding(core::SymbolRef bind, core::Loc loc, std::unique_ptr<Instruction> value);
    Binding(Binding &&other) = default;
    Binding() = default;

    Binding &operator=(Binding &&) = default;
};

class BasicBlock {
public:
    std::vector<core::SymbolRef> args;
    int id = 0;
    int flags = 0;
    int outerLoops = 0;
    std::vector<Binding> exprs;
    BlockExit bexit;
    std::vector<BasicBlock *> backEdges;

    BasicBlock(){};
    std::string toString(core::Context ctx);
};

class CFG {
    /**
     * CFG owns all the BasicBlocks, and then they have raw unmanaged pointers to and between each other,
     * because they all have lifetime identical with each other and the CFG.
     */
public:
    core::SymbolRef symbol;
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks;
    /** Blocks in topoligical sort. All parent blocks are earlier than child blocks
     *
     * The name here goes from using forwards or backwards edges as dependencies in topological sort.
     * This in indeed kind-a reverse to what order would node be in: for topological sort with forward edges
     * the entry point is going to be the last node in sorted array.
     */
    std::vector<BasicBlock *> forwardsTopoSort;
    std::vector<BasicBlock *> backwardsTopoSort;
    static std::unique_ptr<CFG> buildFor(core::Context ctx, ast::MethodDef &md);
    inline BasicBlock *entry() {
        return basicBlocks[0].get();
    }

    BasicBlock *deadBlock() {
        return basicBlocks[1].get();
    };

    std::string toString(core::Context ctx);

    static int FORWARD_TOPO_SORT_VISITED;
    static int BACKWARD_TOPO_SORT_VISITED;

private:
    CFG();
    BasicBlock *walk(core::Context ctx, ast::Expression *what, BasicBlock *current, CFG &inWhat, core::SymbolRef target,
                     int loops);
    BasicBlock *freshBlock(int outerLoops);
    void fillInTopoSorts(core::Context ctx);
    void dealias(core::Context ctx);
    void fillInBlockArguments(core::Context ctx);
    int topoSortFwd(std::vector<BasicBlock *> &target, int nextFree, BasicBlock *currentBB);
    int topoSortBwd(std::vector<BasicBlock *> &target, int nextFree, BasicBlock *currentBB);
};

} // namespace cfg
} // namespace ruby_typer

#endif // SRUBY_CFG_H
