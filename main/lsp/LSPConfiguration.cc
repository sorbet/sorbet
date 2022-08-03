#include "main/lsp/LSPConfiguration.h"
#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "common/FileOps.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
constexpr string_view sorbetScheme = "sorbet:"sv;
constexpr string_view httpsScheme = "https"sv;
} // namespace

namespace {

string getRootPath(const shared_ptr<LSPOutput> &output, const options::Options &opts,
                   const shared_ptr<spdlog::logger> &logger) {
    if (opts.rawInputDirNames.size() != 1) {
        auto msg =
            fmt::format("Sorbet's language server requires a single input directory. However, {} are configured: [{}]",
                        opts.rawInputDirNames.size(), absl::StrJoin(opts.rawInputDirNames, ", "));
        logger->error(msg);
        auto params = make_unique<ShowMessageParams>(MessageType::Error, msg);
        output->write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::WindowShowMessage, move(params))));
        throw EarlyReturnWithCode(1);
    }
    return opts.rawInputDirNames.at(0);
}

MarkupKind getPreferredMarkupKind(vector<MarkupKind> formats) {
    if (absl::c_find(formats, MarkupKind::Markdown) != formats.end()) {
        return MarkupKind::Markdown;
    } else {
        return MarkupKind::Plaintext;
    }
}
} // namespace

LSPConfiguration::LSPConfiguration(const options::Options &opts, const shared_ptr<LSPOutput> &output,
                                   const shared_ptr<spdlog::logger> &logger, bool disableFastPath)
    : initialized(atomic<bool>(false)), opts(opts), output(output), logger(logger), disableFastPath(disableFastPath),
      rootPath(getRootPath(output, opts, logger)) {}

void LSPConfiguration::assertHasClientConfig() const {
    if (!clientConfig) {
        Exception::raise("clientConfig is not initialized.");
    }
}

LSPClientConfiguration::LSPClientConfiguration(const InitializeParams &params) {
    // Note: Default values for fields are set in class definition.
    if (auto rootUriString = get_if<string>(&params.rootUri)) {
        if (absl::EndsWith(*rootUriString, "/")) {
            rootUri = rootUriString->substr(0, rootUriString->length() - 1);
        } else {
            rootUri = *rootUriString;
        }
    }

    if (params.capabilities->textDocument.has_value()) {
        auto &textDocument = params.capabilities->textDocument.value();
        if (textDocument->completion.has_value()) {
            auto &completion = textDocument->completion.value();
            if (completion->completionItem.has_value()) {
                auto &completionItem = completion->completionItem.value();
                clientCompletionItemSnippetSupport = completionItem->snippetSupport.value_or(false);
                if (completionItem->documentationFormat != nullopt) {
                    clientCompletionItemMarkupKind =
                        getPreferredMarkupKind(completionItem->documentationFormat.value());
                }
            }
        }
        if (textDocument->hover.has_value()) {
            auto &hover = textDocument->hover.value();
            if (hover->contentFormat.has_value()) {
                auto &contentFormat = hover->contentFormat.value();
                clientHoverMarkupKind = getPreferredMarkupKind(contentFormat);
            }
        }
        if (textDocument->codeAction.has_value()) {
            auto &codeAction = textDocument->codeAction.value();
            if (codeAction->dataSupport.has_value()) {
                clientCodeActionDataSupport = codeAction->dataSupport.value_or(false);
            }

            if (codeAction->resolveSupport.has_value()) {
                auto &properties = codeAction->resolveSupport.value()->properties;
                clientCodeActionResolveEditSupport =
                    absl::c_any_of(properties, [](auto prop) { return prop == "edit"; });
            }
        }
    }

    if (params.initializationOptions) {
        auto &initOptions = *params.initializationOptions;
        enableOperationNotifications = initOptions->supportsOperationNotifications.value_or(false);
        enableTypecheckInfo = initOptions->enableTypecheckInfo.value_or(false);
        enableSorbetURIs = initOptions->supportsSorbetURIs.value_or(false);
    }
}

void LSPConfiguration::setClientConfig(const shared_ptr<const LSPClientConfiguration> &clientConfig) {
    if (this->clientConfig) {
        Exception::raise("Cannot call setClientConfig twice in one session!");
    }
    this->clientConfig = clientConfig;
}

string LSPConfiguration::localName2Remote(string_view filePath) const {
    ENFORCE(absl::StartsWith(filePath, rootPath));
    assertHasClientConfig();
    string_view relativeUri = filePath.substr(rootPath.length());
    if (relativeUri.at(0) == '/') {
        relativeUri = relativeUri.substr(1);
    }

    // Special case: Root uri is '' (happens in Monaco)
    if (clientConfig->rootUri.length() == 0) {
        return string(relativeUri);
    }

    // Use a sorbet: URI if the file is not present on the client AND the client supports sorbet: URIs
    if (clientConfig->enableSorbetURIs &&
        FileOps::isFileIgnored(rootPath, filePath, opts.lspDirsMissingFromClient, {})) {
        return absl::StrCat(sorbetScheme, relativeUri);
    }
    return absl::StrCat(clientConfig->rootUri, "/", relativeUri);
}

string urlDecode(string_view uri) {
    vector<pair<const absl::string_view, string>> replacements;

    for (size_t pos = uri.find('%'); pos != string::npos; pos = uri.find('%', pos + 1)) {
        // "%3a"
        auto from = uri.substr(pos, 3);
        // add replacement only if % is actually followed by exactly 2 hex digits
        if (from.size() == 3 && isxdigit(from[1]) && isxdigit(from[2])) {
            auto to = absl::HexStringToBytes(from.substr(1));
            replacements.push_back({from, to});
        }
    }

    return absl::StrReplaceAll(uri, replacements);
}

string LSPConfiguration::remoteName2Local(string_view encodedUri) const {
    assertHasClientConfig();

    // VS Code URLencodes the file path, so we should decode first
    string uri = urlDecode(encodedUri);

    if (!isUriInWorkspace(uri) && !isSorbetUri(uri)) {
        logger->error("Unrecognized URI received from client: {}", uri);
        return string(uri);
    }

    const bool isSorbetURI = this->isSorbetUri(uri);
    const string_view root = isSorbetURI ? sorbetScheme : clientConfig->rootUri;
    string::iterator start = uri.begin() + root.length();
    if (*start == '/') {
        ++start;
    }

    string path = string(start, uri.end());

    const bool isHttps = isSorbetURI && absl::StartsWith(path, httpsScheme) && path.length() > httpsScheme.length() &&
                         path[httpsScheme.length()] == ':';
    if (isHttps) {
        return path;
    } else if (rootPath.length() > 0) {
        return absl::StrCat(rootPath, "/", path);
    } else {
        // Special case: Folder is '' (current directory)
        return path;
    }
}

core::FileRef LSPConfiguration::uri2FileRef(const core::GlobalState &gs, string_view uri) const {
    assertHasClientConfig();
    if (!isUriInWorkspace(uri) && !isSorbetUri(uri)) {
        return core::FileRef();
    }
    auto needle = remoteName2Local(uri);
    return gs.findFileByPath(needle);
}

string LSPConfiguration::fileRef2Uri(const core::GlobalState &gs, core::FileRef file) const {
    assertHasClientConfig();
    string uri;
    if (!file.exists()) {
        uri = "???";
    } else {
        auto &messageFile = file.data(gs);
        if (messageFile.isPayload()) {
            if (clientConfig->enableSorbetURIs) {
                uri = absl::StrCat(sorbetScheme, messageFile.path());
            } else {
                uri = string(messageFile.path());
            }
        } else {
            uri = localName2Remote(messageFile.path());
        }
    }
    return uri;
}

unique_ptr<Location> LSPConfiguration::loc2Location(const core::GlobalState &gs, core::Loc loc) const {
    assertHasClientConfig();
    auto range = Range::fromLoc(gs, loc);
    if (range == nullptr) {
        return nullptr;
    }
    string uri = fileRef2Uri(gs, loc.file());
    if (loc.file().exists() && loc.file().data(gs).isPayload() && !clientConfig->enableSorbetURIs) {
        // This is hacky because VSCode appends #4,3 (or whatever the position is of the
        // error) to the uri before it shows it in the UI since this is the format that
        // VSCode uses to denote which location to jump to. However, if you append #L4
        // to the end of the uri, this will work on github (it will ignore the #4,3)
        //
        // As an example, in VSCode, on hover you might see
        //
        // string.rbi(18,7): Method `+` has specified type of argument `arg0` as `String`
        //
        // When you click on the link, in the browser it appears as
        // https://git.corp.stripe.com/stripe-internal/ruby-typer/tree/master/rbi/core/string.rbi#L18%2318,7
        // but shows you the same thing as
        // https://git.corp.stripe.com/stripe-internal/ruby-typer/tree/master/rbi/core/string.rbi#L18
        uri = fmt::format("{}#L{}", uri, loc.position(gs).first.line);
    }
    return make_unique<Location>(uri, std::move(range));
}

vector<string> LSPConfiguration::frefsToPaths(const core::GlobalState &gs, const vector<core::FileRef> &refs) const {
    vector<string> paths;
    paths.resize(refs.size());
    std::transform(refs.begin(), refs.end(), paths.begin(),
                   [&gs](const auto &ref) -> string { return string(ref.data(gs).path()); });
    return paths;
}

bool LSPConfiguration::isFileIgnored(string_view filePath) const {
    return FileOps::isFileIgnored(rootPath, filePath, opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns);
}

bool LSPConfiguration::isSorbetUri(string_view uri) const {
    assertHasClientConfig();
    return clientConfig->enableSorbetURIs && absl::StartsWith(uri, sorbetScheme);
}

bool LSPConfiguration::isUriInWorkspace(string_view uri) const {
    assertHasClientConfig();
    return absl::StartsWith(uri, clientConfig->rootUri);
}

void LSPConfiguration::markInitialized() {
    initialized.store(true);
}

bool LSPConfiguration::isInitialized() const {
    return initialized.load();
}

const LSPClientConfiguration &LSPConfiguration::getClientConfig() const {
    assertHasClientConfig();
    return *clientConfig;
}

} // namespace sorbet::realmain::lsp
