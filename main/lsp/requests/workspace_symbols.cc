#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

/**
 * Represents information about programming constructs like variables, classes,
 * interfaces etc.
 *
 *        interface SymbolInformation {
 *                 // The name of this symbol.
 *                name: string;
 *                 // The kind of this symbol.
 *                kind: number;
 *
 *                 // Indicates if this symbol is deprecated.
 *                deprecated?: boolean;
 *
 *                 *
 *                 * The location of this symbol. The location's range is used by a tool
 *                 * to reveal the location in the editor. If the symbol is selected in the
 *                 * tool the range's start information is used to position the cursor. So
 *                 * the range usually spans more then the actual symbol's name and does
 *                 * normally include things like visibility modifiers.
 *                 *
 *                 * The range doesn't have to denote a node range in the sense of a abstract
 *                 * syntax tree. It can therefore not be used to re-construct a hierarchy of
 *                 * the symbols.
 *                 *
 *                location: Location;
 *
 *                 *
 *                 * The name of the symbol containing this symbol. This information is for
 *                 * user interface purposes (e.g. to render a qualifier in the user interface
 *                 * if necessary). It can't be used to re-infer a hierarchy for the document
 *                 * symbols.
 *
 *                containerName?: string;
 *        }
 **/
unique_ptr<rapidjson::Value> LSPLoop::symbolRef2SymbolInformation(core::SymbolRef symRef) {
    auto sym = symRef.data(*finalGs);
    if (!sym->loc().file().exists() || hideSymbol(symRef)) {
        return nullptr;
    }
    rapidjson::Value result;
    result.SetObject();
    result.AddMember("name", sym->name.show(*finalGs), alloc);
    result.AddMember("location", loc2Location(sym->loc()), alloc);
    result.AddMember("containerName", sym->owner.data(*finalGs)->fullName(*finalGs), alloc);
    result.AddMember("kind", symbolRef2SymbolKind(symRef), alloc);

    return make_unique<rapidjson::Value>(move(result));
}

void LSPLoop::handleWorkspaceSymbols(rapidjson::Value &result, rapidjson::Document &d) {
    prodCategoryCounterInc("lsp.requests.processed", "workspace.symbols");
    result.SetArray();
    string searchString = d["params"]["query"].GetString();

    for (u4 idx = 1; idx < finalGs->symbolsUsed(); idx++) {
        core::SymbolRef ref(finalGs.get(), idx);
        if (hasSimilarName(*finalGs, ref.data(*finalGs)->name, searchString)) {
            auto data = symbolRef2SymbolInformation(ref);
            if (data) {
                result.PushBack(move(*data), alloc);
            }
        }
    }
    sendResult(d, result);
}
} // namespace sorbet::realmain::lsp