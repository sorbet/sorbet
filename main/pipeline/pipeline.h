#ifndef RUBY_TYPER_PIPELINE_H
#define RUBY_TYPER_PIPELINE_H
#include "ProgressIndicator.h"
#include "ast/ast.h"
#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "main/options/options.h"

namespace sorbet::realmain::pipeline {
constexpr std::chrono::milliseconds PROGRESS_REFRESH_TIME_MILLIS = ProgressIndicator::REPORTING_INTERVAL();
ast::ParsedFile indexOne(const options::Options &opts, core::GlobalState &lgs, core::FileRef file,
                         std::unique_ptr<KeyValueStore> &kvstore, std::shared_ptr<spdlog::logger> logger);

std::vector<ast::ParsedFile> index(std::unique_ptr<core::GlobalState> &gs, const std::vector<std::string> &frs,
                                   std::vector<core::FileRef> mainThreadFiles, const options::Options &opts,
                                   WorkerPool &workers, std::unique_ptr<KeyValueStore> &kvstore,
                                   std::shared_ptr<spdlog::logger> logger);

std::vector<ast::ParsedFile> resolve(core::GlobalState &gs, std::vector<ast::ParsedFile> what,
                                     const options::Options &opts, std::shared_ptr<spdlog::logger> logger,
                                     bool skipConfigatron = false);

std::vector<ast::ParsedFile> incrementalResolve(core::GlobalState &gs, std::vector<ast::ParsedFile> what,
                                                const options::Options &opts, std::shared_ptr<spdlog::logger> logger);

std::vector<ast::ParsedFile> name(core::GlobalState &gs, std::vector<ast::ParsedFile> what,
                                  const options::Options &opts, std::shared_ptr<spdlog::logger> logger,
                                  bool skipConfigatron = false);

std::vector<ast::ParsedFile> typecheck(std::unique_ptr<core::GlobalState> &gs, std::vector<ast::ParsedFile> what,
                                       const options::Options &opts, WorkerPool &workers,
                                       std::shared_ptr<spdlog::logger> logger);

ast::ParsedFile typecheckOne(core::Context ctx, ast::ParsedFile resolved, const options::Options &opts,
                             std::shared_ptr<spdlog::logger> logger);
unsigned int computeFileHash(std::shared_ptr<core::File> forWhat, std::shared_ptr<spdlog::logger> logger);

} // namespace sorbet::realmain::pipeline
#endif // RUBY_TYPER_PIPELINE_H
