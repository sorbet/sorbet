#ifndef SORBET_COMPILER_FORWARD_DECLARATIONS_H
#define SORBET_COMPILER_FORWARD_DECLARATIONS_H

namespace llvm {
class AllocaInst;
class BasicBlock;
class DIBuilder;
class DICompileUnit;
class Function;
class FunctionType;
class IRBuilderBase;
class LLVMContext;
class Module;
class StructType;
class Type;
class Value;
} // namespace llvm

namespace sorbet::core {
class GlobalState;
class Loc;
class MutableContext;
class SymbolRef;
class MethodRef;
class NameRef;
} // namespace sorbet::core

namespace sorbet::ast {
class ClassDef;
class MethodDef;
} // namespace sorbet::ast

namespace sorbet::cfg {
class CFG;
class Send;
} // namespace sorbet::cfg

#endif
