#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H

#include <string>
#include <string_view>
#include <vector>

namespace sorbet::realmain::lsp::watchman {

std::string buildSubscribeCommand(std::string_view root, std::string_view subscriptionName,
                                  const std::vector<std::string> &extensions, std::string_view watchmanNamespace);

// Covers the window a replacement subscription does not. A `since` on the subscribe itself is not enough: against
// watchman 20240624 it produced no initial results for a file that had changed two seconds earlier, where a `query`
// with the same clock did report it. Selects the same files as `buildSubscribeCommand`.
std::string buildCatchUpQueryCommand(std::string_view root, const std::vector<std::string> &extensions,
                                     std::string_view watchmanNamespace, std::string_view sinceClock);

} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
