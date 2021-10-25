#ifndef SORBET_COMPILER_RUBYSTACKARGS_H
#define SORBET_COMPILER_RUBYSTACKARGS_H

#include "compiler/Core/ForwardDeclarations.h"
#include "compiler/IREmitter/CallCacheFlags.h"

#include <string_view>
#include <vector>

namespace sorbet::compiler {
class CompilerState;

struct RubyStackArgs {
    RubyStackArgs(std::vector<llvm::Value *> stack, std::vector<std::string_view> keywords, CallCacheFlags flags)
        : stack{std::move(stack)}, keywords{std::move(keywords)}, flags(flags) {}

    std::vector<llvm::Value *> stack;
    std::vector<std::string_view> keywords;
    CallCacheFlags flags;
};

} // namespace sorbet::compiler

#endif
