#ifndef SORBET_LSP_CALL_SITES_H
#define SORBET_LSP_CALL_SITES_H

#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPTypechecker.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

class AbstractRenamer {
public:
    class UniqueSymbolQueue {
    public:
        bool tryEnqueue(core::SymbolRef s);
        core::SymbolRef pop();

    private:
        std::deque<core::SymbolRef> symbols;
        UnorderedSet<core::SymbolRef> set;
    };

    AbstractRenamer(const core::GlobalState &gs, const sorbet::realmain::lsp::LSPConfiguration &config,
                    const std::string oldName, const std::string newName)
        : gs(gs), config(config), oldName(oldName), newName(newName), invalid(false){};

    virtual ~AbstractRenamer() = default;
    virtual void rename(std::unique_ptr<core::lsp::QueryResponse> &response, const core::SymbolRef originalSymbol) = 0;
    std::optional<std::vector<std::unique_ptr<TextDocumentEdit>>> buildTextDocumentEdits();
    std::variant<JSONNullObject, std::unique_ptr<WorkspaceEdit>> buildWorkspaceEdit();
    virtual void addSymbol(const core::SymbolRef) = 0;

    void getRenameEdits(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol, std::string newName);

    bool getInvalid();
    std::string getError();
    std::shared_ptr<UniqueSymbolQueue> getQueue();

protected:
    const core::GlobalState &gs;
    const LSPConfiguration &config;
    std::string oldName;
    std::string newName;
    UnorderedMap<core::Loc, std::string> edits;
    bool invalid;
    std::shared_ptr<UniqueSymbolQueue> symbolQueue = std::make_shared<UniqueSymbolQueue>();
    std::string error;

    static void addSubclassRelatedMethods(const core::GlobalState &gs, core::MethodRef symbol,
                                          std::shared_ptr<UniqueSymbolQueue> methods);

    static void addDispatchRelatedMethods(const core::GlobalState &gs, const core::DispatchResult *dispatchResult,
                                          std::shared_ptr<UniqueSymbolQueue> methods);
};

} // namespace sorbet::realmain::lsp

#endif // SORBET_LSP_CALL_SITES_H
