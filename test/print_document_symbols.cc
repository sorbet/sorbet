#include "common/FileSystem.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/lsp.h"
#include "main/lsp/wrapper.h"
#include "test/helpers/lsp.h"
#include <iostream>
#include <memory>

namespace sorbet::realmain::lsp {
using namespace std;

namespace {
constexpr string_view uriPrefix = "file:///"sv;

// Given a path to a *.rb or *.rbupdate file, returns the set of edits to apply to Sorbet before requesting document
// symbols along with the URI of the document.
pair<string, vector<string>> findEditsToApply(string_view filePath) {
    const auto idx = filePath.rfind('.');
    if (idx == string_view::npos) {
        Exception::raise("Invalid file path: {}", filePath);
    }
    string_view extension = filePath.substr(idx);
    OSFileSystem fs;
    vector<string> fileContents;
    string uri;
    fileContents.push_back(fs.readFile(filePath));
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
    return make_pair(uri, move(fileContents));
}

int printDocumentSymbols(string_view filePath) {
    LSPWrapper lspWrapper("", false);
    lspWrapper.enableAllExperimentalFeatures();
    int nextId = 1;
    int fileId = 1;
    auto [fileUri, fileEdits] = findEditsToApply(filePath);
    test::initializeLSP("", uriPrefix, lspWrapper, nextId);
    {
        // Initialize empty file.
        auto params =
            make_unique<DidOpenTextDocumentParams>(make_unique<TextDocumentItem>(fileUri, "ruby", fileId++, ""));
        auto notif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(params));
        // Discard responses.
        lspWrapper.getLSPResponsesFor(make_unique<LSPMessage>(move(notif)));
    }

    for (auto &fileEdit : fileEdits) {
        vector<unique_ptr<TextDocumentContentChangeEvent>> edits;
        edits.push_back(make_unique<TextDocumentContentChangeEvent>(fileEdit));
        auto params = make_unique<DidChangeTextDocumentParams>(
            make_unique<VersionedTextDocumentIdentifier>(fileUri, static_cast<double>(fileId++)), move(edits));
        auto notif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidChange, move(params));
        lspWrapper.getLSPResponsesFor(make_unique<LSPMessage>(move(notif)));
    }

    auto docSymbolParams = make_unique<DocumentSymbolParams>(make_unique<TextDocumentIdentifier>(fileUri));
    auto req =
        make_unique<RequestMessage>("2.0", nextId++, LSPMethod::TextDocumentDocumentSymbol, move(docSymbolParams));
    // Make documentSymbol request.
    auto responses = lspWrapper.getLSPResponsesFor(make_unique<LSPMessage>(move(req)));

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
    if (argc != 2) {
        std::cout << "Usage: print_document_symbols path/to/file.rb\n";
        return 1;
    }

    return sorbet::test::printDocumentSymbols(argv[1]);
}
