#include "common/FileSystem.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/json_types.h"
#include "main/lsp/wrapper.h"

#include "absl/algorithm/container.h"
#include "absl/strings/match.h"

#include <iostream>
#include <memory>

namespace sorbet::realmain::lsp {
using namespace std;

namespace {
constexpr string_view uriPrefix = "file:///"sv;

// Given a path to a *.rb or *.rbupdate file, returns the set of edits to apply to Sorbet before requesting document
// symbols along with the URI of the document.
struct TestFile {
    string uri;
    vector<string> edits;
};

UnorderedMap<string, TestFile> loadFiles(const vector<string> &files) {
    UnorderedMap<string, TestFile> data;
    for (const auto &f : files) {
        string_view filePath{f};
        const auto idx = filePath.rfind('.');
        if (idx == string_view::npos) {
            Exception::raise("Invalid file path: {}", filePath);
        }
        string_view extension = filePath.substr(idx);
        OSFileSystem fs;
        vector<string> fileContents;
        string uri;
        fileContents.push_back(fs.readFile(string(filePath)));
        if (extension == ".rbupdate") {
            // Find basename[.]version.rbupdate
            const auto versionIdx = filePath.rfind('.', idx - 1);
            if (versionIdx == string_view::npos) {
                Exception::raise("rbupdate file is missing version number: {}", filePath);
            }
            string_view baseName = filePath.substr(0, versionIdx);
            uri = absl::StrCat(uriPrefix, baseName, ".rb");
            string_view versionString = filePath.substr(versionIdx + 1, idx - versionIdx);
            int version = stoi(string(versionString));
            for (version = version - 1; version >= 0; version--) {
                try {
                    fileContents.push_back(fs.readFile(fmt::format("{}.{}.rbupdate", baseName, version)));
                } catch (FileNotFoundException e) {
                    // Ignore.
                }
            }
            fileContents.push_back(fs.readFile(fmt::format("{}.rb", baseName)));
            reverse(fileContents.begin(), fileContents.end());
        } else {
            uri = absl::StrCat(uriPrefix, filePath);
        }
        data[filePath] = TestFile{uri, move(fileContents)};
    }
    return data;
}

int printDocumentSymbols(string_view chosenFile, const vector<string> &files) {
    auto lspWrapper = SingleThreadedLSPWrapper::create();
    lspWrapper->enableAllExperimentalFeatures();
    int nextId = 0;
    int fileId = 1;
    auto testdata = loadFiles(files);

    // Send 'initialize' message.
    {
        auto initializeParams = make_unique<InitializeParams>("", make_unique<ClientCapabilities>());
        {
            auto workspaceCapabilities = make_unique<WorkspaceClientCapabilities>();
            // The following client config options were cargo-culted from existing tests.
            // TODO(jvilk): Prune these down to only the ones we care about.
            workspaceCapabilities->applyEdit = true;
            auto workspaceEdit = make_unique<WorkspaceEditCapabilities>();
            workspaceEdit->documentChanges = true;
            workspaceCapabilities->workspaceEdit = unique_ptr<WorkspaceEditCapabilities>(std::move(workspaceEdit));
            initializeParams->capabilities->workspace = move(workspaceCapabilities);
        }
        {
            auto docCapabilities = make_unique<TextDocumentClientCapabilities>();

            auto publishDiagnostics = make_unique<PublishDiagnosticsCapabilities>();
            publishDiagnostics->relatedInformation = true;
            docCapabilities->publishDiagnostics = std::move(publishDiagnostics);

            auto documentSymbol = std::make_unique<DocumentSymbolCapabilities>();
            documentSymbol->dynamicRegistration = true;
            auto symbolKind = make_unique<SymbolKindOptions>();
            auto supportedSymbols = vector<SymbolKind>();
            for (int i = (int)SymbolKind::File; i <= (int)SymbolKind::TypeParameter; i++) {
                supportedSymbols.push_back((SymbolKind)i);
            }
            symbolKind->valueSet = move(supportedSymbols);
            documentSymbol->symbolKind = move(symbolKind);
            docCapabilities->documentSymbol = move(documentSymbol);
            initializeParams->capabilities->textDocument = move(docCapabilities);
        }

        initializeParams->trace = TraceKind::Off;

        auto message = make_unique<LSPMessage>(
            make_unique<RequestMessage>("2.0", nextId++, LSPMethod::Initialize, move(initializeParams)));
        lspWrapper->getLSPResponsesFor(move(message));
    }

    // Complete initialization handshake with an 'initialized' message.
    {
        auto initialized =
            make_unique<NotificationMessage>("2.0", LSPMethod::Initialized, make_unique<InitializedParams>());
        lspWrapper->getLSPResponsesFor(make_unique<LSPMessage>(move(initialized)));
    }

    {
        // Initialize empty files.
        for (auto &filename : files) {
            auto params = make_unique<DidOpenTextDocumentParams>(
                make_unique<TextDocumentItem>(testdata[filename].uri, "ruby", fileId++, ""));
            auto notif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(params));
            // Discard responses.
            lspWrapper->getLSPResponsesFor(make_unique<LSPMessage>(move(notif)));
        }
    }

    for (auto &filename : files) {
        auto &testfile = testdata[filename];
        for (auto &fileEdit : testfile.edits) {
            vector<unique_ptr<TextDocumentContentChangeEvent>> edits;
            edits.push_back(make_unique<TextDocumentContentChangeEvent>(fileEdit));
            auto params = make_unique<DidChangeTextDocumentParams>(
                make_unique<VersionedTextDocumentIdentifier>(testfile.uri, static_cast<double>(fileId++)), move(edits));
            auto notif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidChange, move(params));
            lspWrapper->getLSPResponsesFor(make_unique<LSPMessage>(move(notif)));
        }
    }

    auto docSymbolParams =
        make_unique<DocumentSymbolParams>(make_unique<TextDocumentIdentifier>(testdata[chosenFile].uri));
    auto req =
        make_unique<RequestMessage>("2.0", nextId++, LSPMethod::TextDocumentDocumentSymbol, move(docSymbolParams));
    // Make documentSymbol request.
    auto responses = lspWrapper->getLSPResponsesFor(make_unique<LSPMessage>(move(req)));

    if (responses.size() == 1 && responses.at(0)->isResponse()) {
        cout << responses.at(0)->toJSON(true) << "\n";
        return 0;
    } else {
        cout << "Sorbet returned invalid response for documentSymbols:\n";
        if (responses.empty()) {
            cout << "(nothing)\n";
        } else {
            for (auto &response : responses) {
                cout << response->toJSON(true) << "\n";
            }
        }
        return 1;
    }
}
} // namespace
} // namespace sorbet::realmain::lsp

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout
            << "Usage: print_document_symbols path/to/file/for/symbols.rb path/to/file1.rb path/to/file2.rb ...\n";
        return 1;
    }

    int numFiles = argc - 2;
    char **files = &argv[2];
    std::vector<std::string> filenames;
    for (int i = 0; i < numFiles; ++i) {
        filenames.emplace_back(files[i]);
    }

    int rbupdates = absl::c_count_if(filenames, [](const auto &f) { return absl::EndsWith(f, "rbupdate"); });
    if (rbupdates > 1) {
        std::cout << "multiple updated files not supported at this time\n";
        return 1;
    }

    return sorbet::realmain::lsp::printDocumentSymbols(argv[1], filenames);
}
