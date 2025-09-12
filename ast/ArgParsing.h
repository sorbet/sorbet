#ifndef SORBET_AST_ARG_PARSING_H
#define SORBET_AST_ARG_PARSING_H
#include "ast/ast.h"
#include "core/ArityHash.h"
namespace sorbet::ast {
class ArgParsing {
public:
    static std::vector<core::ParsedParam> parseParams(const ast::MethodDef::PARAMS_store &params);
    static core::ArityHash hashArgs(core::Context ctx, const std::vector<core::ParsedParam> &args);
    // Returns the default argument value for the given argument, or nullptr if not specified. Mutates arg.
    static ExpressionPtr getDefault(const core::ParsedParam &parsedArg, ExpressionPtr arg);
};
}; // namespace sorbet::ast

#endif
