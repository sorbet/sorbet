#include "dsl/Plugin.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "dsl/helpers.h"
#include "dsl/util.h"
#include "parser/parser.h"
#include "absl/strings/str_replace.h"

using namespace std;

string exec(string cmd);

namespace sorbet::dsl {

vector<unique_ptr<ast::Expression>> Plugin::replaceDSL(core::MutableContext ctx, unique_ptr<ast::ClassDef> &classDef, ast::Send *send) {
    vector<unique_ptr<ast::Expression>> stats;

    auto funName = send->fun.toString(ctx.state);
    auto command = ctx.state.findDslPlugin(funName);

    if (command) {
        auto className = classDef->name->loc.source(ctx.state);
        auto sendSource = send->loc.source(ctx.state);
        auto cmd = fmt::format(
            "{} --class \"{}\" --method \"{}\" \"{}\"",
            *command,
            absl::StrReplaceAll(className, {{"\"", "\\\""}}),
            absl::StrReplaceAll(funName, {{"\"", "\\\""}}),
            absl::StrReplaceAll(sendSource, {{"\"", "\\\""}})
        );
        auto output = exec(cmd);
        core::FileRef file = ctx.state.enterFile(string(send->loc.file().data(ctx.state).path()), output);
        auto nodes = parser::Parser::run(ctx.state, file);
        auto ast = ast::desugar::node2Tree(ctx, move(nodes));
        stats.emplace_back(move(ast));
    }

    return stats;
}

}; // namespace sorbet::dsl
