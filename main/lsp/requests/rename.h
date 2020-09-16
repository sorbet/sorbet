#ifndef RUBY_TYPER_LSP_REQUESTS_RENAME_H
#define RUBY_TYPER_LSP_REQUESTS_RENAME_H

#include "main/lsp/LSPTask.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {
class RenameParams;
class RenameTask final : public LSPRequestTask {
    std::unique_ptr<RenameParams> params;
<<<<<<< HEAD
    absl::variant<JSONNullObject, std::unique_ptr<WorkspaceEdit>>
    getRenameEdits(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol, std::string_view newName);
=======
    std::unique_ptr<WorkspaceEdit> getRenameEdits(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol,
                                                  std::string_view newName);
>>>>>>> c3b20a977... Rename constants

public:
    RenameTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<RenameParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
