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
    bool wrapHash = false;

    if (send->hasKwArgs()) {
        if (send->numPosArgs != 0) {
            return;
        }
        wrapHash = true;
    } else {
        if (send->numPosArgs != 1) {
            return;
        }
    }

    auto *front = send->args.front().get();
    auto *back = send->args.back().get();
    core::Loc argsLoc{ctx.file, front->loc.join(back->loc)};

    auto [start, end] = core::Loc(ctx.file, send->loc).position(ctx);

    if (start.line == end.line) {
        if (wrapHash) {
            e.replaceWith("Convert to lazy type alias", core::Loc(ctx.file, send->loc), "T.type_alias {{{{{}}}}}",
                          argsLoc.source(ctx));
        } else {
            e.replaceWith("Convert to lazy type alias", core::Loc(ctx.file, send->loc), "T.type_alias {{{}}}",
                          argsLoc.source(ctx));
        }
    } else {
        core::Loc endLoc(argsLoc.file(), argsLoc.endPos(), argsLoc.endPos());
        string argIndent = getIndent(ctx, endLoc);
        string argSrc = fmt::format("{}{}", argIndent, argsLoc.source(ctx));
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
