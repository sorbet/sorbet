#ifndef RUBY_TYPER_PAYLOAD_H
#define RUBY_TYPER_PAYLOAD_H
#include "common/kvstore/KeyValueStore.h"
#include "core/GlobalState.h"
#include "main/options/options.h"
#include "spdlog/spdlog.h"

namespace sorbet::payload {

void createInitialGlobalState(std::unique_ptr<core::GlobalState> &gs, std::shared_ptr<spdlog::logger> &logger,
                              const realmain::options::Options &options, std::unique_ptr<KeyValueStore> &kvstore);
void retainGlobalState(std::unique_ptr<core::GlobalState> &gs, std::shared_ptr<spdlog::logger> &logger,
                       const realmain::options::Options &options, std::unique_ptr<KeyValueStore> &kvstore);

} // namespace sorbet::payload
#endif // RUBY_TYPER_PAYLOAD_H
