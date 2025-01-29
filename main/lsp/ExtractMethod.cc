#include "main/lsp/ExtractMethod.h"
#include "absl/strings/escaping.h"
#include "ast/treemap/treemap.h"
#include "common/sort/sort.h"

using namespace std;

namespace sorbet::realmain::lsp {

vector<unique_ptr<TextDocumentEdit>> MethodExtractor::getExtractEdits(const LSPTypecheckerDelegate &typechecker,
                                                                      const LSPConfiguration &config) {
    return {};
}

} // namespace sorbet::realmain::lsp
