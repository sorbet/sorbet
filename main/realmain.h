#ifndef RUBY_TYPER_REAL_MAIN_H
#define RUBY_TYPER_REAL_MAIN_H
#include "common/Timer.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/GlobalState.h"
#include "main/options/options.h"
#include "spdlog/spdlog.h"

namespace sorbet::realmain {
int realmain(int argc, char *argv[]);

void createInitialGlobalState(std::unique_ptr<core::GlobalState> &gs, std::shared_ptr<spdlog::logger> &logger,
                              const options::Options &options, std::unique_ptr<KeyValueStore> &kvstore);

extern std::shared_ptr<spdlog::logger> logger;
} // namespace sorbet::realmain
#endif // RUBY_TYPER_REAL_MAIN_H
