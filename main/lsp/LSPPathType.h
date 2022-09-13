
#ifndef SORBET_LSP_PATH_TYPE_H
#define SORBET_LSP_PATH_TYPE_H

namespace sorbet::realmain::lsp {

/**
 * The enum declares a quality of service for a given change
 */
enum class PathType {
    // Full typecheck
    Slow,

    // Incremental typecheck
    Fast,
};
}; // namespace sorbet::realmain::lsp

#endif
