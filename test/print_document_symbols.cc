#include "common/FileSystem.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/lsp.h"
#include "main/lsp/wrapper.h"
#include "test/helpers/lsp.h"
#include <iostream>
#include <memory>

namespace sorbet::realmain::lsp {
using namespace std;
int printDocumentSymbols(string_view filePath) {
    LSPWrapper lspWrapper("", false);
    lspWrapper.enableAllExperimentalFeatures();
    int nextId = 1;
    string uriPrefix = "file:///";
    test::initializeLSP("", uriPrefix, lspWrapper, nextId);
    string fileUri = uriPrefix + string(filePath);
    {
        OSFileSystem fs;
        auto params = make_unique<DidOpenTextDocumentParams>(
            make_unique<TextDocumentItem>(fileUri, "ruby", 1, fs.readFile(filePath)));
        auto notif = make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(params));
        // Discard responses; it's OK if the file has errors.
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
} // namespace sorbet::realmain::lsp

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: print_document_symbols path/to/file.rb\n";
        return 1;
    }

    return sorbet::realmain::lsp::printDocumentSymbols(argv[1]);
}
