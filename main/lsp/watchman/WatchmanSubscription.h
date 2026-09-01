#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H

#include <string>
#include <string_view>
#include <vector>

namespace sorbet::realmain::lsp::watchman {

// Builds the `subscribe` command Sorbet writes to the watchman CLI's stdin.
std::string buildSubscribeCommand(std::string_view root, std::string_view subscriptionName,
                                  const std::vector<std::string> &extensions, std::string_view watchmanNamespace);

// Builds the `query` command that covers the window a replacement subscription does not.
//
// A subscription reports what changes after watchman creates it. It does accept a `since`, which watchman documents as
// limiting the initial results to the changes after that clock, and which feeds exactly one evaluation before the
// subscription advances to a clock of its own. Sorbet does not rely on it: against watchman 20240624, subscribing with
// a `since` produced no initial results at all for a file that had changed two seconds earlier, and the first
// unilateral response then reported only later changes. (Subscription evaluations force `sync_timeout` to 0, unlike a
// query, which is the likeliest reason, though that is not confirmed.) A `query` with the same `since` does report that
// file, so that is how Sorbet asks for what it missed.
//
// Matches `buildSubscribeCommand`'s expression exactly, so the two cannot disagree about which files Sorbet cares
// about.
std::string buildCatchUpQueryCommand(std::string_view root, const std::vector<std::string> &extensions,
                                     std::string_view watchmanNamespace, std::string_view sinceClock);

} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANSUBSCRIPTION_H
