#ifndef SORBET_LSP_CALL_SITES_H
#define SORBET_LSP_CALL_SITES_H

#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {
class UniqueSymbolQueue {
public:
    bool tryEnqueue(core::SymbolRef s);
    core::SymbolRef pop();

private:
    std::deque<core::SymbolRef> symbols;
    UnorderedSet<core::SymbolRef> set;
};

class Renamer {
public:
    Renamer(const core::GlobalState &gs, const sorbet::realmain::lsp::LSPConfiguration &config,
            const std::string oldName, const std::string newName)
        : gs(gs), config(config), oldName(oldName), newName(newName), invalid(false){};

    virtual ~Renamer() = default;
    virtual void rename(std::unique_ptr<core::lsp::QueryResponse> &response) = 0;
    std::variant<JSONNullObject, std::unique_ptr<WorkspaceEdit>> buildEdit();
    virtual void addSymbol(const core::SymbolRef) = 0;

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
};

core::ClassOrModuleRef findRootClassWithMethod(const core::GlobalState &gs, core::ClassOrModuleRef klass,
                                               core::NameRef methodName);

void addSubclassRelatedMethods(const core::GlobalState &gs, core::MethodRef symbol,
                               std::shared_ptr<UniqueSymbolQueue> methods);

void addDispatchRelatedMethods(const core::GlobalState &gs, const core::DispatchResult *dispatchResult,
                               std::shared_ptr<UniqueSymbolQueue> methods);

} // namespace sorbet::realmain::lsp

#endif // SORBET_LSP_CALL_SITES_H
