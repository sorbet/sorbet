#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

namespace sorbet::realmain::lsp::watchman {

std::string buildSubscribeCommand(std::string_view root, std::string_view subscriptionName,
                                  const std::vector<std::string> &extensions, std::string_view watchmanNamespace);

// Matches the same files as the subscription, but synchronizes with the file system before answering.
std::string buildChangesSinceQuery(std::string_view root, const std::vector<std::string> &extensions,
                                   std::string_view watchmanNamespace, std::string_view since);

constexpr int MAX_WATCHMAN_RESTARTS = 5;

// 1-based, so the schedule is 1s, 2s, 4s, 8s, then 8s.
std::chrono::milliseconds watchmanRestartDelay(int attempt);

} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
