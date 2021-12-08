#ifndef SORBET_AST_ARG_PARSING_H
#define SORBET_AST_ARG_PARSING_H
#include "ast/ast.h"
namespace sorbet::ast {
struct ParsedArg {
    core::LocOffsets loc;
    core::LocalVariable local;
    core::ArgInfo::ArgFlags flags;
};
class ArgParsing {
public:
    static std::vector<ParsedArg> parseArgs(const ast::MethodDef::ARGS_store &args);
    static std::vector<uint32_t> hashArgs(core::Context ctx, const std::vector<ParsedArg> &args);
    // Returns the default argument value for the given argument, or nullptr if not specified. Mutates arg.
    static ExpressionPtr getDefault(const ParsedArg &parsedArg, ExpressionPtr arg);
};
}; // namespace sorbet::ast

#endif
