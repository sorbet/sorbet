#ifndef SORBET_NAMER_NAMER_H
#define SORBET_NAMER_NAMER_H
#include "ast/ast.h"
#include "core/FileHash.h"
#include <memory>

namespace sorbet {
class WorkerPool;
}

namespace sorbet::namer {

class Namer final {
public:
    static ast::ParsedFilesOrCancelled
    symbolizeTreesBestEffort(const core::GlobalState &gs, std::vector<ast::ParsedFile> trees, WorkerPool &workers);

    // Note: foundMethodHashes is an optional out parameter.
    //
    // Setting it to a non-nullptr requests that Namer compute a fingerprint of the FoundDefinitions
    // it found while running. (Thus, it's usually nullptr except when pipeline::resolve is called
    // for the purpose of computing a FileHash.)
    static ast::ParsedFilesOrCancelled run(core::GlobalState &gs, std::vector<ast::ParsedFile> trees,
                                           WorkerPool &workers, core::FoundMethodHashes *foundMethodHashesOut);

    Namer() = delete;
};

} // namespace sorbet::namer

#endif
