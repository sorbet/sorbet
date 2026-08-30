#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H

#include <string>
#include <string_view>
#include <vector>

namespace sorbet::realmain::lsp::watchman {

// Renders the `subscribe` command Sorbet sends to the watchman CLI: files under `root` (optionally restricted to
// `watchmanNamespace`) whose suffix is one of `extensions`, reported by name. Each entry of `deferStates` is a watchman
// state; while any of them is asserted on the root, watchman holds this subscription's notifications and delivers them
// in one batch when the state is left.
std::string buildSubscribeCommand(std::string_view root, std::string_view subscriptionName,
                                  const std::vector<std::string> &extensions, std::string_view watchmanNamespace,
                                  const std::vector<std::string> &deferStates);

} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
