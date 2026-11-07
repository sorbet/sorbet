#ifndef RUBY_TYPER_LSP_SHOWOPERATION_H
#define RUBY_TYPER_LSP_SHOWOPERATION_H

#include <optional>
#include <string>
#include <string_view>

namespace sorbet::realmain::lsp {
class LSPOutput;
class LSPConfiguration;

/**
 * Object that uses the RAII pattern to notify the client when a *slow* operation
 * starts and ends. Is used to provide user feedback in the status line of VS Code.
 *
 * Clients that opt into `sorbet/showOperation` (like Sorbet's VS Code extension) get those custom notifications.
 * Other clients that advertise `window.workDoneProgress` get standard LSP `$/progress` notifications instead.
 */
class ShowOperation final {
    const LSPConfiguration &config;
    const std::string operationName;
    const std::string description;
    // Set when this operation is being reported with `$/progress`.
    std::optional<std::string> progressToken;

public:
    enum class Kind {
        Indexing = 1,
        SlowPathBlocking,
        SlowPathNonBlocking,
        FastPath,
        References,
        SymbolSearch,
        Rename,
        MoveMethod,
    };

    // Copy and move construction and assignment is explicitly deleted here
    // because the destructor of ShowOperation sends LSP notifications.
    // Allowing copies or moves will often involve making temporaries, and
    // those temporaries will have their destructors run, causing notifications
    // to be sent unexpectedly. There's a reasonable argument for implementing
    // move assignment and a move constructor, but we would need to modify
    // ShowOperation to know that it had been moved out of, and for our uses
    // it's not worth the effort.
    ShowOperation(const ShowOperation &other) = delete;
    ShowOperation &operator=(const ShowOperation &other) = delete;
    ShowOperation(ShowOperation &&other) = delete;
    ShowOperation &operator=(ShowOperation &&other) = delete;

    ShowOperation(const LSPConfiguration &config, Kind kind);
    ~ShowOperation();

    // Prefix of the IDs Sorbet uses for the `window/workDoneProgress/create` requests it sends to the client.
    static constexpr std::string_view WORK_DONE_PROGRESS_CREATE_ID_PREFIX = "sorbet/workDoneProgress/create/";
};
} // namespace sorbet::realmain::lsp

#endif
