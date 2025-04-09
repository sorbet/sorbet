#ifndef RUBY_TYPER_PAYLOAD_H
#define RUBY_TYPER_PAYLOAD_H
#include "common/kvstore/KeyValueStore.h"
#include "core/GlobalState.h"
#include "main/options/options.h"
#include "spdlog/spdlog.h"

namespace sorbet::payload {

void createInitialGlobalState(core::GlobalState &gs, const realmain::options::Options &options,
                              const std::unique_ptr<const OwnedKeyValueStore> &kvstore);

// Create a copy of `gs` that has the same string storage, name table, and file table, but otherwise has its state set
// as though it was initialized by `createInitialGlobalState`.
std::unique_ptr<core::GlobalState> copyForSlowPath(const core::GlobalState &gs,
                                                   const realmain::options::Options &options);

} // namespace sorbet::payload
#endif // RUBY_TYPER_PAYLOAD_H
