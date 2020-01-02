#ifndef RUBY_TYPER_PIPELINE_H
#define RUBY_TYPER_PIPELINE_H
#include "ProgressIndicator.h"
#include "ast/ast.h"
#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/NameHash.h"
#include "main/options/options.h"

namespace sorbet::realmain::pipeline {
ast::ParsedFile indexOne(const options::Options &opts, core::GlobalState &lgs, core::FileRef file,
                         std::unique_ptr<KeyValueStore> &kvstore);

std::pair<ast::ParsedFile, std::vector<std::shared_ptr<core::File>>>
indexOneWithPlugins(const options::Options &opts, core::GlobalState &lgs, core::FileRef file,
                    std::unique_ptr<KeyValueStore> &kvstore);

std::vector<core::FileRef> reserveFiles(std::unique_ptr<core::GlobalState> &gs, const std::vector<std::string> &files);

std::vector<ast::ParsedFile> index(std::unique_ptr<core::GlobalState> &gs, std::vector<core::FileRef> files,
                                   const options::Options &opts, WorkerPool &workers,
                                   std::unique_ptr<KeyValueStore> &kvstore);

ast::ParsedFilesOrCancelled resolve(std::unique_ptr<core::GlobalState> &gs, std::vector<ast::ParsedFile> what,
                                    const options::Options &opts, WorkerPool &workers, bool skipConfigatron = false);

std::vector<ast::ParsedFile> incrementalResolve(core::GlobalState &gs, std::vector<ast::ParsedFile> what,
                                                const options::Options &opts);

std::vector<ast::ParsedFile> name(core::GlobalState &gs, std::vector<ast::ParsedFile> what,
                                  const options::Options &opts, bool skipConfigatron = false);

// Note: `epoch` parameter is only used in LSP mode when preemptible is true. Do not use it elsewhere.
// TODO(jvilk): Make this interface cleaner.
ast::ParsedFilesOrCancelled typecheck(std::unique_ptr<core::GlobalState> &gs, std::vector<ast::ParsedFile> what,
                                      const options::Options &opts, WorkerPool &workers, bool cancelable = false,
                                      bool preemptible = false, u4 epoch = 0);

ast::ParsedFile typecheckOne(core::Context ctx, ast::ParsedFile resolved, const options::Options &opts);

core::FileHash computeFileHash(std::shared_ptr<core::File> forWhat, spdlog::logger &logger);

core::StrictLevel decideStrictLevel(const core::GlobalState &gs, const core::FileRef file,
                                    const options::Options &opts);

} // namespace sorbet::realmain::pipeline
#endif // RUBY_TYPER_PIPELINE_H
