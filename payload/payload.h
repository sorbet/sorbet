#ifndef RUBY_TYPER_PAYLOAD_H
#define RUBY_TYPER_PAYLOAD_H
#include "common/kvstore/KeyValueStore.h"
#include "core/GlobalState.h"
#include "main/options/options.h"
#include "spdlog/spdlog.h"

namespace sorbet::payload {

constexpr std::string_view GLOBAL_STATE_KEY = "GlobalState";

void createInitialGlobalState(std::unique_ptr<core::GlobalState> &gs, const realmain::options::Options &options,
                              const std::unique_ptr<const OwnedKeyValueStore> &kvstore);

} // namespace sorbet::payload
#endif // RUBY_TYPER_PAYLOAD_H
