# SorbetLLVM Internals

> 🚧 This doc is WIP. Add or change it as you see fit. 🚧

Plugin injector:

1. DSL passes
2. IREmission (typed cfg to LLVM IR)
   IREmission (typed cfg to LLVM IR)
   IREmission (typed cfg to LLVM IR)
   (accumulate multiple methods' CFGs into one llvm::Module)
3. ObjectFileEmission (LLVM IR to shared object `*.so`)

Ruby methods and Ruby blocks are similar in many ways from the compiler's
perspective (they both end up as llvm::Functions that can be called directly).

Key data structures:

TypecheckThreadState
  llvm::Module
CompilerState
IREmitterContext
Aliases

