#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H

#include <string>
#include <string_view>
#include <vector>

namespace sorbet::realmain::lsp::watchman {

std::string buildSubscribeCommand(std::string_view root, std::string_view subscriptionName,
                                  const std::vector<std::string> &extensions, std::string_view watchmanNamespace,
                                  const std::vector<std::string> &deferStates);

} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
