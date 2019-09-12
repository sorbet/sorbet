#ifndef SORBET_AST_ARG_PARSING_H
#define SORBET_AST_ARG_PARSING_H
#include "ast/ast.h"
namespace sorbet::ast {
struct ParsedArg {
    core::Loc loc;
    core::LocalVariable local;
    std::unique_ptr<ast::Expression> default_;
    bool keyword = false;
    bool block = false;
    bool repeated = false;
    bool shadow = false;
};
class ArgParsing {
public:
    static ParsedArg parseArg(core::Context ctx, std::unique_ptr<ast::Reference> arg);
    static std::vector<ParsedArg> parseArgs(core::Context ctx, ast::MethodDef::ARGS_store &args);
    static std::vector<u4> hashArgs(core::Context ctx, std::vector<ParsedArg> &args);
};
}; // namespace sorbet::ast

#endif
