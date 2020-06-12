#include <string_view>

#include "absl/strings/str_split.h"
#include "common/formatting.h"
#include "resolver/CorrectTypeAlias.h"

using namespace std;

namespace sorbet::resolver {
std::string getIndent(core::Context ctx, const core::Loc loc) {
    auto [_, indentLen] = loc.findStartOfLine(ctx);
    return string(indentLen, ' ');
}

std::string indented(const std::string &s) {
    vector<string_view> lines = absl::StrSplit(s, '\n');
    return fmt::format("{}", fmt::map_join(lines, "\n", [](auto line) -> string { return fmt::format("  {}", line); }));
}

void CorrectTypeAlias::eagerToLazy(core::Context ctx, core::ErrorBuilder &e, ast::Send *send) {
    if (send->args.size() != 1) {
        return;
    }
    auto *arg = send->args[0].get();
    auto *hash = ast::cast_tree<ast::Hash>(send->args[0]);
    // Insert extra {}'s when a hash literal does not have them.
    // Example: `T.type_alias(a: Integer,  b: String)`
    bool wrapHash = hash != nullptr && core::Loc(ctx.file, hash->loc).source(ctx)[0] != '{';
    auto [start, end] = core::Loc(ctx.file, send->loc).position(ctx);
    if (start.line == end.line) {
        if (wrapHash) {
            e.replaceWith("Convert to lazy type alias", core::Loc(ctx.file, send->loc), "T.type_alias {{{{{}}}}}",
                          core::Loc(ctx.file, arg->loc).source(ctx));
        } else {
            e.replaceWith("Convert to lazy type alias", core::Loc(ctx.file, send->loc), "T.type_alias {{{}}}",
                          core::Loc(ctx.file, arg->loc).source(ctx));
        }
    } else {
        auto loc = core::Loc(ctx.file, arg->loc);
        core::Loc endLoc(loc.file(), loc.endPos(), loc.endPos());
        string argIndent = getIndent(ctx, endLoc);
        string argSrc = fmt::format("{}{}", argIndent, core::Loc(ctx.file, arg->loc).source(ctx));
        if (wrapHash) {
            argSrc = fmt::format("{}{{\n{}\n{}}}", argIndent, indented(argSrc), argIndent);
        }
        if (core::Loc(ctx.file, send->loc).position(ctx).second.line == endLoc.position(ctx).second.line) {
            argSrc = indented(argSrc);
        }
        e.replaceWith("Convert to lazy type alias", core::Loc(ctx.file, send->loc), "T.type_alias do\n{}\n{}end",
                      argSrc, getIndent(ctx, core::Loc(ctx.file, send->loc)));
    }
}

} // namespace sorbet::resolver
