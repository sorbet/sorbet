#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
void LSPLoop::symbolRef2DocumentSymbolWalkMembers(core::SymbolRef sym, core::FileRef filter, rapidjson::Value &out) {
    for (auto mem : sym.data(*finalGs)->membersStableOrderSlow(*finalGs)) {
        if (mem.first != core::Names::attached() && mem.first != core::Names::singleton()) {
            bool foundThisFile = false;
            for (auto loc : mem.second.data(*finalGs)->locs()) {
                foundThisFile = foundThisFile || loc.file() == filter;
            }
            if (!foundThisFile) {
                continue;
            }
            auto inner = LSPLoop::symbolRef2DocumentSymbol(mem.second, filter);
            if (inner) {
                out.PushBack(*inner, alloc);
            }
        }
    }
}

void LSPLoop::handleTextDocumentDocumentSymbol(rapidjson::Value &result, rapidjson::Document &d) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.documentSymbol");
    result.SetArray();
    auto uri = string_view(d["params"]["textDocument"]["uri"].GetString(),
                           d["params"]["textDocument"]["uri"].GetStringLength());
    auto fref = uri2FileRef(uri);
    for (u4 idx = 1; idx < finalGs->symbolsUsed(); idx++) {
        core::SymbolRef ref(finalGs.get(), idx);
        if (!hideSymbol(ref) && (ref.data(*finalGs)->owner.data(*finalGs)->loc().file() != fref ||
                                 ref.data(*finalGs)->owner == core::Symbols::root())) {
            for (auto definitionLocation : ref.data(*finalGs)->locs()) {
                if (definitionLocation.file() == fref) {
                    auto data = symbolRef2DocumentSymbol(ref, fref);
                    if (data) {
                        result.PushBack(move(*data), alloc);
                        break;
                    }
                }
            }
        }
    }
    sendResult(d, result);
}

/**
 *
 * Represents programming constructs like variables, classes, interfaces etc. that appear in a document. Document
 * symbols can be hierarchical and they have two ranges: one that encloses its definition and one that points to its
 * most interesting range, e.g. the range of an identifier.
 *
 *  export class DocumentSymbol {
 *
 *       // The name of this symbol.
 *      name: string;
 *
 *       // More detail for this symbol, e.g the signature of a function. If not provided the
 *       // name is used.
 *      detail?: string;
 *
 *       // The kind of this symbol.
 *      kind: SymbolKind;
 *
 *       // Indicates if this symbol is deprecated.
 *      deprecated?: boolean;
 *
 *      //  The range enclosing this symbol not including leading/trailing whitespace but everything else
 *      //  like comments. This information is typically used to determine if the the clients cursor is
 *      //  inside the symbol to reveal in the symbol in the UI.
 *      range: Range;
 *
 *      //  The range that should be selected and revealed when this symbol is being picked, e.g the name of a function.
 *      //  Must be contained by the the `range`.
 *      selectionRange: Range;
 *
 *      //  Children of this symbol, e.g. properties of a class.
 *      children?: DocumentSymbol[];
 *  }
 */

unique_ptr<rapidjson::Value> LSPLoop::symbolRef2DocumentSymbol(core::SymbolRef symRef, core::FileRef filter) {
    if (!symRef.exists()) {
        return nullptr;
    }
    auto sym = symRef.data(*finalGs);
    if (!sym->loc().file().exists() || hideSymbol(symRef)) {
        return nullptr;
    }
    rapidjson::Value result;
    result.SetObject();
    string prefix = "";
    if (sym->owner.exists() && sym->owner.data(*finalGs)->isClass() &&
        sym->owner.data(*finalGs)->attachedClass(*finalGs).exists()) {
        prefix = "self.";
    }
    result.AddMember("name", prefix + sym->name.show(*finalGs), alloc);
    if (sym->isMethod()) {
        result.AddMember("detail", methodDetail(symRef, nullptr, nullptr, nullptr), alloc);
    } else {
        // Currently released version of VSCode has a bug that requires this non-optional field to be present
        result.AddMember("detail", "", alloc);
    }
    result.AddMember("kind", symbolRef2SymbolKind(symRef), alloc);
    result.AddMember("range", loc2Range(sym->loc()),
                     alloc); // TODO: this range should cover body. Currently it doesn't.
    result.AddMember("selectionRange", loc2Range(sym->loc()), alloc);

    rapidjson::Value children;
    children.SetArray();
    symbolRef2DocumentSymbolWalkMembers(symRef, filter, children);
    if (sym->isClass()) {
        auto singleton = sym->lookupSingletonClass(*finalGs);
        if (singleton.exists()) {
            symbolRef2DocumentSymbolWalkMembers(singleton, filter, children);
        }
    }
    result.AddMember("children", children, alloc);

    return make_unique<rapidjson::Value>(move(result));
}

} // namespace sorbet::realmain::lsp