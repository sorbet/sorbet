#ifndef SRUBY_CFG_H
#define SRUBY_CFG_H

#include "../ast/Symbols.h"
#include "../ast/Trees.h"
#include "ast/ast.h"
#include "parser/parser.h"
#include <memory>

namespace ruby_typer {
namespace ast {
namespace cfg {

// TODO: convert it to implicitly numbered instead of explicitly bound
// implicitly numbered: result of every instruction can be uniquely referenced
// by its position in a linear array.

class Instruction {
public:
    virtual ~Instruction() = default;
    virtual std::string toString(Context ctx) = 0;
};

class Ident : public Instruction {
    SymbolRef what;

public:
    Ident(const SymbolRef &what);
    virtual std::string toString(Context ctx);
};

class Send : public Instruction {
public:
    SymbolRef recv;
    NameRef fun;
    std::vector<SymbolRef> args;
    virtual std::string toString(Context ctx);
};

class LoadArg : public Instruction {
    int argId;
    SymbolRef tpe;
};

class Return : public Instruction {
public:
    SymbolRef what;

    Return(const SymbolRef &what);
    virtual std::string toString(Context ctx);
};

class New : public Instruction {
public:
    SymbolRef claz;
    std::vector<SymbolRef> args;

    New(const SymbolRef &claz, std::vector<SymbolRef> &args);
    virtual std::string toString(Context ctx);
};

class Super : public Instruction {
public:
    std::vector<SymbolRef> args;

    Super(std::vector<SymbolRef> &args);
    virtual std::string toString(Context ctx);
};

class NamedArg : public Instruction {
public:
    NameRef name;
    SymbolRef value;
};

class FloatLit : public Instruction {
public:
    float value;

    FloatLit(float value);
    virtual std::string toString(Context ctx);
};

class IntLit : public Instruction {
public:
    int value;

    IntLit(int value);
    virtual std::string toString(Context ctx);
};

class StringLit : public Instruction {
public:
    NameRef value;

    StringLit(NameRef value) : value(value){};
    virtual std::string toString(Context ctx);
};

class NotSupported : public Instruction {
public:
    std::string why;

    NotSupported(std::string why) : why(why){};
    virtual std::string toString(Context ctx);
};

class BoolLit : public Instruction {
public:
    bool value;

    BoolLit(bool value) : value(value){};
    virtual std::string toString(Context ctx);
};

class ConstantLit : public Instruction {
    NameRef cnst;

public:
    ConstantLit(NameRef cnst) : cnst(cnst) {}
    virtual std::string toString(Context ctx);
};

class ArraySplat : public Instruction {
public:
    NameRef arg;

    ArraySplat(NameRef arg) : arg(arg){};
};

class HashSplat : public Instruction {
public:
    NameRef arg;

    HashSplat(NameRef arg) : arg(arg){};
};

class Nil : public Instruction {
public:
    Nil(){};
    virtual std::string toString(Context ctx);
};

class Self : public Instruction {
public:
    SymbolRef claz;

    Self(SymbolRef claz) : claz(claz){};
    virtual std::string toString(Context ctx);
};

class Block : public Instruction {
public:
    SymbolRef method;

    Block(SymbolRef method) : method(method){};
};

class BasicBlock;

class BlockExit {
public:
    SymbolRef cond;
    BasicBlock *thenb;
    BasicBlock *elseb;
};

class Binding {
public:
    SymbolRef bind;
    std::unique_ptr<Instruction> value;

    Binding(const SymbolRef &bind, std::unique_ptr<Instruction> value);
    Binding(Binding &&other) = default;
    Binding() = default;
};

class BasicBlock {
public:
    int args = 0;
    std::vector<Binding> exprs;
    BlockExit bexit;

    BasicBlock(){};
    std::string toString(Context ctx);
};

class CFG {

public:
    SymbolRef symbol;
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks;
    static std::unique_ptr<CFG> buildFor(Context ctx, ast::MethodDef &md);
    inline BasicBlock *entry() {
        return basicBlocks[0].get();
    }

    BasicBlock *deadBlock() {
        return basicBlocks[1].get();
    };

    std::string toString(Context ctx);

private:
    CFG();
    BasicBlock *walk(Context ctx, ast::Statement *what, BasicBlock *current, CFG &inWhat, SymbolRef target);
    BasicBlock *freshBlock();
};

} // namespace cfg
} // namespace ast
} // namespace ruby_typer

#endif // SRUBY_CFG_H
