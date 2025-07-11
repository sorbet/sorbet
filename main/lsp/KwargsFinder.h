#ifndef RUBY_TYPER_LSP_KWARGSFINDER_H
#define RUBY_TYPER_LSP_KWARGSFINDER_H

#include "core/Loc.h"
#include <vector>

namespace sorbet::core {
class GlobalState;
class NameRef;
} // namespace sorbet::core

namespace sorbet::ast {
struct ParsedFile;
}

namespace sorbet::realmain::lsp {

class KwargsFinder {
public:
    // Find all of the kwargs mentioned by the send whose method name is located at `funLoc`.
    static std::vector<core::NameRef> findKwargs(const core::GlobalState &gs, const ast::ParsedFile &ast,
                                                 core::LocOffsets funLoc);
};

} // namespace sorbet::realmain::lsp

#endif
