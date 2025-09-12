#ifndef SORBET_AST_ARG_PARSING_H
#define SORBET_AST_ARG_PARSING_H
#include "ast/ast.h"
#include "core/ArityHash.h"
namespace sorbet::ast {
class ArgParsing {
public:
    static std::vector<core::ParsedParam> parseParams(const ast::MethodDef::PARAMS_store &params);
    static core::ArityHash hashParams(core::Context ctx, const std::vector<core::ParsedParam> &params);
    // Returns the default argument value for the given parameter, or nullptr if not specified. Mutates param.
    static ExpressionPtr getDefault(const core::ParsedParam &parsedParam, ExpressionPtr param);
};
}; // namespace sorbet::ast

#endif
