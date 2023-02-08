#ifndef SORBET_LSP_CALL_SITES_H
#define SORBET_LSP_CALL_SITES_H

#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPTypechecker.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {

class AbstractRewriter {
public:
    class UniqueSymbolQueue {
    public:
        bool tryEnqueue(core::SymbolRef s);
        core::SymbolRef pop();

    private:
        std::deque<core::SymbolRef> symbols;
        UnorderedSet<core::SymbolRef> set;
    };

    AbstractRewriter(const core::GlobalState &gs, const sorbet::realmain::lsp::LSPConfiguration &config)
        : gs(gs), config(config), invalid(false){};

    virtual ~AbstractRewriter() = default;
    virtual void rename(std::unique_ptr<core::lsp::QueryResponse> &response, const core::SymbolRef originalSymbol) = 0;
    std::optional<std::vector<std::unique_ptr<TextDocumentEdit>>> buildTextDocumentEdits();
    std::variant<JSONNullObject, std::unique_ptr<WorkspaceEdit>> buildWorkspaceEdit();
    virtual void addSymbol(const core::SymbolRef) = 0;

    void getEdits(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol);

    bool getInvalid();
    std::string getError();
    std::shared_ptr<UniqueSymbolQueue> getQueue();

protected:
    const core::GlobalState &gs;
    const LSPConfiguration &config;
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
