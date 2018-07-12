#ifndef RUBY_TYPER_PIPELINE_H
#define RUBY_TYPER_PIPELINE_H
#include "ProgressIndicator.h"
#include "ast/ast.h"
#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "main/options/options.h"

namespace sorbet {
namespace realmain {
namespace pipeline {
constexpr std::chrono::milliseconds PROGRESS_REFRESH_TIME_MILLIS = ProgressIndicator::REPORTING_INTERVAL();

std::unique_ptr<ast::Expression> indexOne(const options::Options &opts, core::GlobalState &lgs, core::FileRef file,
                                          std::unique_ptr<KeyValueStore> &kvstore,
                                          std::shared_ptr<spdlog::logger> logger);

std::vector<std::unique_ptr<ast::Expression>>
index(std::unique_ptr<core::GlobalState> &gs, std::vector<std::string> frs, std::vector<core::FileRef> mainThreadFiles,
      const options::Options &opts, WorkerPool &workers, std::unique_ptr<KeyValueStore> &kvstore,
      std::shared_ptr<spdlog::logger> logger);

std::vector<std::unique_ptr<ast::Expression>> resolve(core::GlobalState &gs,
                                                      std::vector<std::unique_ptr<ast::Expression>> what,
                                                      const options::Options &opts,
                                                      std::shared_ptr<spdlog::logger> logger);

void typecheck(std::unique_ptr<core::GlobalState> &gs, std::vector<std::unique_ptr<ast::Expression>> what,
               const options::Options &opts, WorkerPool &workers, std::shared_ptr<spdlog::logger> logger);

std::unique_ptr<ast::Expression> typecheckOne(core::Context ctx, std::unique_ptr<ast::Expression> resolved,
                                              const options::Options &opts, std::shared_ptr<spdlog::logger> logger);
} // namespace pipeline
} // namespace realmain
} // namespace sorbet
#endif // RUBY_TYPER_PIPELINE_H
