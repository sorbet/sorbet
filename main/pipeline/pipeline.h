#ifndef RUBY_TYPER_PIPELINE_H
#define RUBY_TYPER_PIPELINE_H
#include "ProgressIndicator.h"
#include "ast/ast.h"
#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "main/options/options.h"

namespace sorbet::core::lsp {
class PreemptionTaskManager;
}

namespace sorbet::realmain::pipeline {
ast::ParsedFile indexOne(const options::Options &opts, core::GlobalState &lgs, core::FileRef file,
                         const std::unique_ptr<OwnedKeyValueStore> &kvstore);

std::pair<ast::ParsedFile, std::vector<std::shared_ptr<core::File>>>
indexOneWithPlugins(const options::Options &opts, core::GlobalState &lgs, core::FileRef file,
                    const std::unique_ptr<OwnedKeyValueStore> &kvstore);

std::vector<core::FileRef> reserveFiles(std::unique_ptr<core::GlobalState> &gs, const std::vector<std::string> &files);

std::vector<ast::ParsedFile> index(std::unique_ptr<core::GlobalState> &gs, std::vector<core::FileRef> files,
                                   const options::Options &opts, WorkerPool &workers,
                                   const std::unique_ptr<OwnedKeyValueStore> &kvstore);

ast::ParsedFilesOrCancelled resolve(std::unique_ptr<core::GlobalState> &gs, std::vector<ast::ParsedFile> what,
                                    const options::Options &opts, WorkerPool &workers, bool skipConfigatron = false);

std::vector<ast::ParsedFile> incrementalResolve(core::GlobalState &gs, std::vector<ast::ParsedFile> what,
                                                const options::Options &opts);

std::vector<ast::ParsedFile> name(core::GlobalState &gs, std::vector<ast::ParsedFile> what,
                                  const options::Options &opts, bool skipConfigatron = false);

// Note: `cancelable` and `preemption task manager` are only applicable to LSP.
ast::ParsedFilesOrCancelled
typecheck(std::unique_ptr<core::GlobalState> &gs, std::vector<ast::ParsedFile> what, const options::Options &opts,
          WorkerPool &workers, bool cancelable = false,
          std::optional<std::shared_ptr<core::lsp::PreemptionTaskManager>> preemptionManager = std::nullopt);

ast::ParsedFile typecheckOne(core::Context ctx, ast::ParsedFile resolved, const options::Options &opts);

// Computes file hashes for the given files, and stores them in the files. If supplied, attempts to retrieve hashes from
// the key-value store. Returns 'true' if it had to compute any file hashes.
bool computeFileHashes(const std::vector<std::shared_ptr<core::File>> files, spdlog::logger &logger,
                       WorkerPool &workers, const std::unique_ptr<OwnedKeyValueStore> &kvstore);

core::StrictLevel decideStrictLevel(const core::GlobalState &gs, const core::FileRef file,
                                    const options::Options &opts);

} // namespace sorbet::realmain::pipeline
#endif // RUBY_TYPER_PIPELINE_H
