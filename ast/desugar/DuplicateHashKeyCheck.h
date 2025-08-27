#ifndef SORBET_AST_DUPLICATE_HASH_KEY_CHECK_H
#define SORBET_AST_DUPLICATE_HASH_KEY_CHECK_H

#include "ast/Trees.h"
#include "common/common.h"
#include "core/errors/desugar.h"

namespace sorbet::ast::desugar {

class DuplicateHashKeyCheck {
    const core::MutableContext ctx;
    const core::GlobalState &gs;
    UnorderedMap<core::NameRef, core::LocOffsets> hashKeySymbols;
    UnorderedMap<core::NameRef, core::LocOffsets> hashKeyStrings;

public:
    DuplicateHashKeyCheck(const core::MutableContext &ctx)
        : ctx{ctx}, gs{ctx.state}, hashKeySymbols(), hashKeyStrings() {}

    void check(const ExpressionPtr &key) {
        auto lit = ast::cast_tree<ast::Literal>(key);
        if (lit == nullptr) {
            return;
        }

        auto isSymbol = lit->isSymbol();
        if (!lit || !lit->isName()) {
            return;
        }
        auto nameRef = lit->asName();

        if (isSymbol && !hashKeySymbols.contains(nameRef)) {
            hashKeySymbols[nameRef] = key.loc();
        } else if (!isSymbol && !hashKeyStrings.contains(nameRef)) {
            hashKeyStrings[nameRef] = key.loc();
        } else {
            if (auto e = ctx.beginIndexerError(key.loc(), core::errors::Desugar::DuplicatedHashKeys)) {
                core::LocOffsets originalLoc;
                if (isSymbol) {
                    originalLoc = hashKeySymbols[nameRef];
                } else {
                    originalLoc = hashKeyStrings[nameRef];
                }

                e.setHeader("Hash key `{}` is duplicated", nameRef.toString(gs));
                e.addErrorLine(ctx.locAt(originalLoc), "First occurrence of `{}` hash key", nameRef.toString(gs));
            }
        }
    }

    void reset() {
        hashKeySymbols.clear();
        hashKeyStrings.clear();
    }

    // This is only used with Send::ARGS_store and Array::ELEMS_store
    template <typename T> static void checkSendArgs(const core::MutableContext ctx, int numPosArgs, const T &args) {
        DuplicateHashKeyCheck duplicateKeyCheck{ctx};

        // increment by two so that a keyword args splat gets skipped.
        for (int i = numPosArgs; i < args.size(); i += 2) {
            duplicateKeyCheck.check(args[i]);
        }
    }
};

} // namespace sorbet::ast::desugar

#endif // SORBET_AST_DUPLICATE_HASH_KEY_CHECK_H
