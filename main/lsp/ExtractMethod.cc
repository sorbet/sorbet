#include "main/lsp/ExtractMethod.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace extract_method {

vector<unique_ptr<TextDocumentEdit>> getExtractMethodEdits(LSPTypecheckerDelegate &typechecker,
                                                           const LSPConfiguration &config,
                                                           const core::Loc selectionLoc) {
    return {};
}

} // namespace extract_method

} // namespace sorbet::realmain::lsp
