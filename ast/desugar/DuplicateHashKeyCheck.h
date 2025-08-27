#ifndef SORBET_AST_DUPLICATE_HASH_KEY_CHECK_H
#define SORBET_AST_DUPLICATE_HASH_KEY_CHECK_H

#include "ast/Trees.h"
#include "common/common.h"
#include "core/errors/desugar.h"

namespace sorbet::ast::desugar {

class DuplicateHashKeyCheck {
    const core::MutableContext ctx;
    UnorderedMap<core::NameRef, core::LocOffsets> hashKeySymbols;
    UnorderedMap<core::NameRef, core::LocOffsets> hashKeyStrings;

public:
    DuplicateHashKeyCheck(const core::MutableContext &ctx) : ctx{ctx}, hashKeySymbols(), hashKeyStrings() {}

    void check(const ExpressionPtr &key) {
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
                e.addErrorLine(ctx.locAt(originalLoc), "First occurrence of `{}` hash key",
                               nameRef.toString(ctx.state));
            }
        }
    }

    // This is only used with Send::ARGS_store and Array::ELEMS_store
    static void checkSendArgs(const core::MutableContext ctx, int numPosArgs, absl::Span<const ExpressionPtr> args) {
        DuplicateHashKeyCheck duplicateKeyCheck{ctx};

        // increment by two so that a keyword args splat gets skipped.
        for (int i = numPosArgs; i < args.size(); i += 2) {
            duplicateKeyCheck.check(args[i]);
        }
    }
};

} // namespace sorbet::ast::desugar

#endif // SORBET_AST_DUPLICATE_HASH_KEY_CHECK_H
