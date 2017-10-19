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

class Instruction {
public:
    virtual ~Instruction() = default;
    virtual std::string toString(ast::Context ctx) = 0;
};

class Ident : public Instruction {
    ast::SymbolRef what;

public:
    Ident(const ast::SymbolRef &what);
    virtual std::string toString(ast::Context ctx);
};

class Send : public Instruction {
public:
    ast::SymbolRef recv;
    ast::NameRef fun;
    std::vector<ast::SymbolRef> args;
    virtual std::string toString(ast::Context ctx);
};

class LoadArg : public Instruction {
    int argId;
    ast::SymbolRef tpe;
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

class Block : public Instruction {
public:
    ast::SymbolRef method;

    Block(ast::SymbolRef method) : method(method){};
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
    int args = 0;
    std::vector<Binding> exprs;
    BlockExit bexit;

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
    static std::unique_ptr<CFG> buildFor(ast::Context ctx, ast::MethodDef &md);
    inline BasicBlock *entry() {
        return basicBlocks[0].get();
    }

    BasicBlock *deadBlock() {
        return basicBlocks[1].get();
    };

    std::string toString(ast::Context ctx);

private:
    CFG();
    BasicBlock *walk(ast::Context ctx, ast::Statement *what, BasicBlock *current, CFG &inWhat, ast::SymbolRef target);
    BasicBlock *freshBlock();
};

} // namespace cfg
} // namespace ruby_typer

#endif // SRUBY_CFG_H
