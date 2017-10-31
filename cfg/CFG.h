#ifndef SRUBY_CFG_H
#define SRUBY_CFG_H

#include "../ast/Symbols.h"
#include "../ast/Trees.h"
#include "ast/ast.h"
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
    virtual std::string toString(ast::Context ctx) = 0;
};

class Ident : public Instruction {
public:
    ast::SymbolRef what;

    Ident(const ast::SymbolRef &what);
    virtual std::string toString(ast::Context ctx);
};

class Send : public Instruction {
public:
    ast::SymbolRef recv;
    ast::NameRef fun;
    std::vector<ast::SymbolRef> args;

    Send(ast::SymbolRef recv, ast::NameRef fun, std::vector<ast::SymbolRef> &args);

    virtual std::string toString(ast::Context ctx);
};

class Return : public Instruction {
public:
    ast::SymbolRef what;

    Return(const ast::SymbolRef &what);
    virtual std::string toString(ast::Context ctx);
};

class New : public Instruction {
public:
    ast::SymbolRef klass;
    std::vector<ast::SymbolRef> args;

    New(const ast::SymbolRef &klass, std::vector<ast::SymbolRef> &args);
    virtual std::string toString(ast::Context ctx);
};

class Super : public Instruction {
public:
    std::vector<ast::SymbolRef> args;

    Super(std::vector<ast::SymbolRef> &args);
    virtual std::string toString(ast::Context ctx);
};

class NamedArg : public Instruction {
public:
    ast::NameRef name;
    ast::SymbolRef value;
};

class FloatLit : public Instruction {
public:
    float value;

    FloatLit(float value);
    virtual std::string toString(ast::Context ctx);
};

class IntLit : public Instruction {
public:
    int value;

    IntLit(int value);
    virtual std::string toString(ast::Context ctx);
};

class StringLit : public Instruction {
public:
    ast::NameRef value;

    StringLit(ast::NameRef value) : value(value){};
    virtual std::string toString(ast::Context ctx);
};

class NotSupported : public Instruction {
public:
    std::string why;

    NotSupported(std::string why) : why(why){};
    virtual std::string toString(ast::Context ctx);
};

class BoolLit : public Instruction {
public:
    bool value;

    BoolLit(bool value) : value(value){};
    virtual std::string toString(ast::Context ctx);
};

class ConstantLit : public Instruction {
    ast::NameRef cnst;

public:
    ConstantLit(ast::NameRef cnst) : cnst(cnst) {}
    virtual std::string toString(ast::Context ctx);
};

class ArraySplat : public Instruction {
public:
    ast::NameRef arg;

    ArraySplat(ast::NameRef arg) : arg(arg){};
};

class HashSplat : public Instruction {
public:
    ast::NameRef arg;

    HashSplat(ast::NameRef arg) : arg(arg){};
};

class Nil : public Instruction {
public:
    Nil(){};
    virtual std::string toString(ast::Context ctx);
};

class Self : public Instruction {
public:
    ast::SymbolRef klass;

    Self(ast::SymbolRef klass) : klass(klass){};
    virtual std::string toString(ast::Context ctx);
};

class LoadArg : public Instruction {
public:
    ast::SymbolRef receiver;
    ast::NameRef method;
    u4 arg;

    LoadArg(ast::SymbolRef receiver, ast::NameRef method, u4 arg) : receiver(receiver), method(method), arg(arg){};
    virtual std::string toString(ast::Context ctx);
};

class BasicBlock;

class BlockExit {
public:
    ast::SymbolRef cond;
    BasicBlock *thenb;
    BasicBlock *elseb;
};

class Binding {
public:
    /**
     * This is inefficient as it pollutes global symbol table and in the future (when we start working on perfromance)
     * we should consider using references to instructions similarly to how LuaJIT does it.
     */
    ast::SymbolRef bind;

    std::unique_ptr<Instruction> value;

    Binding(const ast::SymbolRef &bind, std::unique_ptr<Instruction> value);
    Binding(Binding &&other) = default;
    Binding() = default;
};

class BasicBlock {
public:
    std::vector<ast::SymbolRef> args;
    int id = 0;
    int flags = 0;
    int outerLoops = 0;
    std::vector<Binding> exprs;
    BlockExit bexit;
    std::vector<BasicBlock *> backEdges;

    BasicBlock(){};
    std::string toString(ast::Context ctx);
};

class CFG {
    /**
     * CFG owns all the BasicBlocks, and then they have raw unmanaged pointers to and between each other,
     * because they all have lifetime identical with each other and the CFG.
     */
public:
    ast::SymbolRef symbol;
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks;
    /** Blocks in topoligical sort. All parent blocks are earlier than child blocks
     *
     * The name here goes from using forwards or backwards edges as dependencies in topological sort.
     * This in indeed kind-a reverse to what order would node be in: for topological sort with forward edges
     * the entry point is going to be the last node in sorted array.
     */
    std::vector<BasicBlock *> forwardsTopoSort;
    std::vector<BasicBlock *> backwardsTopoSort;
    static std::unique_ptr<CFG> buildFor(ast::Context ctx, ast::MethodDef &md);
    inline BasicBlock *entry() {
        return basicBlocks[0].get();
    }

    BasicBlock *deadBlock() {
        return basicBlocks[1].get();
    };

    std::string toString(ast::Context ctx);

    static inline int FORWARD_TOPO_SORT_VISITED = 1 << 0;
    static inline int BACKWARD_TOPO_SORT_VISITED = 1 << 1;

private:
    CFG();
    BasicBlock *walk(ast::Context ctx, ast::Statement *what, BasicBlock *current, CFG &inWhat, ast::SymbolRef target,
                     int loops);
    BasicBlock *freshBlock(int outerLoops);
    void fillInTopoSorts(ast::Context ctx);
    void fillInBlockArguments(ast::Context ctx);
    int topoSortFwd(std::vector<BasicBlock *> &target, int nextFree, BasicBlock *currentBB);
    int topoSortBwd(std::vector<BasicBlock *> &target, int nextFree, BasicBlock *currentBB);
};

} // namespace cfg
} // namespace ruby_typer

#endif // SRUBY_CFG_H
