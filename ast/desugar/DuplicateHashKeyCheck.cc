#include "ast/desugar/DuplicateHashKeyCheck.h"
#include "core/KwPair.h"
#include "core/errors/desugar.h"

namespace sorbet::ast::desugar {

void DuplicateHashKeyCheck::check(const ExpressionPtr &key) {
    auto lit = ast::cast_tree<ast::Literal>(key);
    if (lit == nullptr || !lit->isName()) {
        return;
    }
    auto nameRef = lit->asName();

    auto &table = lit->isSymbol() ? hashKeySymbols : hashKeyStrings;
    if (!table.contains(nameRef)) {
        table[nameRef] = key.loc();
    } else {
        if (auto e = ctx.beginIndexerError(key.loc(), core::errors::Desugar::DuplicatedHashKeys)) {
            core::LocOffsets originalLoc = table[nameRef];

            e.setHeader("Hash key `{}` is duplicated", nameRef.toString(ctx.state));
            e.addErrorLine(ctx.locAt(originalLoc), "First occurrence of `{}` hash key", nameRef.toString(ctx.state));
        }
    }
}

void DuplicateHashKeyCheck::checkSendArgs(const core::MutableContext ctx, int numPosArgs,
                                          absl::Span<const ExpressionPtr> args) {
    DuplicateHashKeyCheck duplicateKeyCheck{ctx};

    args.remove_prefix(numPosArgs);
    // Drop any kwsplat arg.
    if ((args.size() % 2) == 1) {
        args.remove_suffix(1);
    }

    for (auto [key, _value] : core::KwPairSpan<const ExpressionPtr>{args}) {
        duplicateKeyCheck.check(key);
    }
}

} // namespace sorbet::ast::desugar
