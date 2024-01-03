#ifndef RUBY_TYPER_LSPCONFIGURATION_H
#define RUBY_TYPER_LSPCONFIGURATION_H

#include "core/Loc.h"
#include "main/lsp/json_enums.h"
#include "main/options/options.h"

namespace sorbet::realmain::lsp {

class LSPOutput;
class InitializeParams;
class SorbetInitializationOptions;
class Position;
class Location;

/**
 * Client options sent during initialization.
 */
class LSPClientConfiguration final {
public:
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

    /** Where LSP should output an information diagnostic for untyped values */
    core::TrackUntyped enableHighlightUntyped = core::TrackUntyped::Nowhere;

    /** If false, nudges in `typed: false` files are disabled */
    bool enableTypedFalseCompletionNudges = true;

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
    /** If true, then LSP client can resolve CodeAction's `edit` property lazily */
    bool clientCodeActionResolveEditSupport = false;
    /** If true, then LSP client can pass additional data between codeAction and codeAction/resolve requests */
    bool clientCodeActionDataSupport = false;

    LSPClientConfiguration(const InitializeParams &initializeParams);

    static core::TrackUntyped parseEnableHighlightUntyped(const SorbetInitializationOptions &options,
                                                          core::TrackUntyped defaultIfUnset);
};

/**
 * The language server's configuration information.
 *
 * Shared among multiple threads, e.g.:
 * - Preprocessor thread
 * - Watchman thread
 * - Typechecking thread
 *
 * Only the preprocessor thread has mutable access to LSPConfiguration, which it uses to update clientConfig and the
 * initialized boolean. All other threads have read-only access.
 */
class LSPConfiguration {
    // Raw access to clientConfig is restricted to avoid race conditions. `getClientConfig` safely mediates read-only
    // access to this object. Object is `const` to avoid mutations post-initialization.
    // clientConfig is set by the LSPPreprocessor.
    std::shared_ptr<const LSPClientConfiguration> clientConfig;
    // If 'true', then the client<->server initialization handshake has *completed*. Set by the LSPPreprocessor.
    std::atomic<bool> initialized;
    // Throws if clientConfig is not set. Must be called before accessing `clientConfig` in LSPConfiguration methods.
    void assertHasClientConfig() const;
    // Does the given URI correspond to a `sorbet:` URI?
    bool isSorbetUri(std::string_view uri) const;

public:
    // The following properties are configured when the language server is created.

    /** Command line / startup options. */
    const options::Options &opts;
    const std::shared_ptr<LSPOutput> output;
    const std::shared_ptr<spdlog::logger> logger;
    /** If true, all queries will hit the slow path. */
    const bool disableFastPath;
    /** File system root of LSP client workspace. May be empty if it is the current working directory. */
    const std::string rootPath;

    // The following properties are configured during initialization.

    LSPConfiguration(const options::Options &opts, const std::shared_ptr<LSPOutput> &output,
                     const std::shared_ptr<spdlog::logger> &logger, bool disableFastPath = false);

    // Note: These two methods should only be called from the LSPPreprocessor thread, which is the only place that
    // should have mutable access to LSPConfiguration.
    void setClientConfig(const std::shared_ptr<const LSPClientConfiguration> &clientConfig);
    void markInitialized();

    /**
     * If 'true', then the initialization routine is complete.
     */
    bool isInitialized() const;

    const LSPClientConfiguration &getClientConfig() const;
    core::FileRef uri2FileRef(const core::GlobalState &gs, std::string_view uri) const;
    std::string fileRef2Uri(const core::GlobalState &gs, const core::FileRef) const;
    std::vector<std::string> frefsToPaths(const core::GlobalState &gs, const std::vector<core::FileRef> &refs) const;
    std::string remoteName2Local(std::string_view uri) const;
    std::string localName2Remote(std::string_view filePath) const;
    // returns nullptr if this loc doesn't exist
    std::unique_ptr<Location> loc2Location(const core::GlobalState &gs, core::Loc loc) const;
    bool isFileIgnored(std::string_view filePath) const;

    /**
     * Returns 'true' if the URI corresponds to a path within the active workspace.
     * Note: Does *not* return 'true' for URIs corresponding to sorbet: URIs. While these are valid URIs, they are not
     * actually within the workspace.
     */
    bool isUriInWorkspace(std::string_view uri) const;
};
} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSPCONFIGURATION_H
