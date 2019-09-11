#ifndef RUBY_TYPER_LSPCONFIGURATION_H
#define RUBY_TYPER_LSPCONFIGURATION_H

#include "main/lsp/json_types.h"
#include "main/options/options.h"

namespace sorbet::realmain::lsp {

/**
 * The language server's configuration information.
 * Note: Some information is only populated during initialization.
 */
class LSPConfiguration {
public:
    // The following properties are configured when the language server is created.

    /** Command line options. */
    const options::Options &opts;
    std::shared_ptr<spdlog::logger> logger;
    /** If true, LSPLoop will skip configatron during type checking */
    const bool skipConfigatron;
    /** If true, all queries will hit the slow path. */
    const bool disableFastPath;
    /** File system root of LSP client workspace. May be empty if it is the current working directory. */
    const std::string rootPath;

    // The following properties are configured during initialization.

    /**
     * Set to true once the server is initialized.
     * TODO(jvilk): Use to raise server not initialized errors.
     */
    bool initialized = false;
    /** Root of LSP client workspace. May be different than rootPath. At Stripe, editing happens locally but Sorbet
     * runs on a remote server. */
    std::string rootUri = "";
    /**
     * If true, then LSP will send the client notifications at the start and end of slow operations.
     * We don't want to send these notifications to clients that don't know what to do with them,
     * so this boolean gets set when the client sends the `initialize` request with
     * `params.initializationOptions.supportsOperationNotifications` set to `true`.
     */
    bool enableOperationNotifications = false;
    /**
     * If true, then Sorbet will use sorbet: URIs for files that are not stored on disk (e.g., payload files).
     */
    bool enableSorbetURIs = false;
    /** If true, then LSP sends metadata to the client every time it typechecks files. Used in tests. */
    bool enableTypecheckInfo = false;
    /**
     * Whether or not the active client has support for snippets in CompletionItems.
     * Note: There is a generated ClientCapabilities class, but it is cumbersome to work with as most fields are
     * optional.
     */
    bool clientCompletionItemSnippetSupport = false;
    /** What hover markup should we send to the client? */
    MarkupKind clientHoverMarkupKind = MarkupKind::Plaintext;
    /** What completion item markup should we send to the client? */
    MarkupKind clientCompletionItemMarkupKind = MarkupKind::Plaintext;

    LSPConfiguration(const options::Options &opts, const std::shared_ptr<spdlog::logger> &logger,
                     bool skipConfigatron = false, bool disableFastPath = false);

    void configure(const InitializeParams &initializeParams);

    core::FileRef uri2FileRef(const core::GlobalState &gs, std::string_view uri) const;
    std::string fileRef2Uri(const core::GlobalState &gs, const core::FileRef) const;
    std::string remoteName2Local(std::string_view uri) const;
    std::string localName2Remote(std::string_view filePath) const;
    core::Loc lspPos2Loc(core::FileRef fref, const Position &pos, const core::GlobalState &gs) const;
    // returns nullptr if this loc doesn't exist
    std::unique_ptr<Location> loc2Location(const core::GlobalState &gs, core::Loc loc) const;
    bool isFileIgnored(std::string_view filePath) const;
};
} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSPCONFIGURATION_H
