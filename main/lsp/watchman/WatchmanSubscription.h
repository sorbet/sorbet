#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

namespace sorbet::realmain::lsp::watchman {

// The JSON `subscribe` command sent to `watchman -j -p`.
std::string buildSubscribeCommand(std::string_view root, std::string_view subscriptionName,
                                  const std::vector<std::string> &extensions, std::string_view watchmanNamespace);

// A one-shot JSON `query` for the files (matching the same expression as the subscription) that changed after
// `since`. Unlike a subscription's initial results, a query synchronizes with the file system before answering, so
// it also reports changes that watchman was still digesting when it was asked. `is_fresh_instance` in the answer
// means the clock belongs to a different watchman instance and the delta is unavailable.
std::string buildChangesSinceQuery(std::string_view root, const std::vector<std::string> &extensions,
                                   std::string_view watchmanNamespace, std::string_view since);

// How many times in a row a dead watchman CLI is respawned before Sorbet gives up and exits.
constexpr int MAX_WATCHMAN_RESTARTS = 5;

// The pause before the `attempt`th respawn (1-based): 1s, 2s, 4s, 8s, then 8s.
std::chrono::milliseconds watchmanRestartDelay(int attempt);

} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
