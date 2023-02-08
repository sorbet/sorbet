#include "main/lsp/ConvertToSingletonClassMethod.h"
#include "main/lsp/AbstractRewriter.h"
using namespace std;

namespace sorbet::realmain::lsp {

namespace {

class MethodCallSiteRewriter : public AbstractRewriter {
public:
    MethodCallSiteRenamer(const core::GlobalState &gs, const LSPConfiguration &config) {
        //
    }
};

} // namespace

vector<unique_ptr<TextDocumentEdit>> convertToSingletonClassMethod(LSPTypecheckerDelegate &typechecker,
                                                                   const LSPConfiguration &config,
                                                                   const core::lsp::MethodDefResponse &definition) {
    vector<unique_ptr<TextDocumentEdit>> res;
    auto &gs = typechecker.state();

    // TODO(jez) moveMethod → insert `self.` into the method def (or wrap in `class << self`)
    auto edits = moveMethod(typechecker, config, definition, newModuleName.value());

    auto renamer = make_shared<MethodCallSiteRewriter>(gs, config);
    renamer->getRenameEdits(typechecker, definition.symbol);
    auto callSiteEdits = renamer->buildTextDocumentEdits();

    if (callSiteEdits.has_value()) {
        for (auto &edit : callSiteEdits.value()) {
            res.emplace_back(move(edit));
        }
    }
    auto docEdit =
        make_unique<TextDocumentEdit>(make_unique<VersionedTextDocumentIdentifier>(
                                          config.fileRef2Uri(gs, definition.termLoc.file()), JSONNullObject()),
                                      move(edits));

    res.emplace_back(move(docEdit));
    return res;
}
} // namespace sorbet::realmain::lsp
