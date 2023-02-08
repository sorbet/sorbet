#ifndef SORBET_CONVERT_TO_SINGLETON_CLASS_METHOD_H
#define SORBET_CONVERT_TO_SINGLETON_CLASS_METHOD_H

#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPTypechecker.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

std::vector<std::unique_ptr<TextDocumentEdit>>
convertToSingletonClassMethod(LSPTypecheckerDelegate &typechecker, const LSPConfiguration &config,
                              const core::lsp::MethodDefResponse &definition);

} // namespace sorbet::realmain::lsp

#endif
