#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
void LSPLoop::symbolRef2DocumentSymbolWalkMembers(const core::GlobalState &gs, core::SymbolRef sym,
                                                  core::FileRef filter, rapidjson::Value &out) {
    for (auto mem : sym.data(gs)->membersStableOrderSlow(gs)) {
        if (mem.first != core::Names::attached() && mem.first != core::Names::singleton()) {
            bool foundThisFile = false;
            for (auto loc : mem.second.data(gs)->locs()) {
                foundThisFile = foundThisFile || loc.file() == filter;
            }
            if (!foundThisFile) {
                continue;
            }
            auto inner = LSPLoop::symbolRef2DocumentSymbol(gs, mem.second, filter);
            if (inner) {
                out.PushBack(*inner, alloc);
            }
        }
    }
}

unique_ptr<core::GlobalState> LSPLoop::handleTextDocumentDocumentSymbol(unique_ptr<core::GlobalState> gs,
                                                                        rapidjson::Value &result,
                                                                        rapidjson::Document &d) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.documentSymbol");
    result.SetArray();
    auto uri = string_view(d["params"]["textDocument"]["uri"].GetString(),
                           d["params"]["textDocument"]["uri"].GetStringLength());
    auto fref = uri2FileRef(uri);
    auto finalGs = move(gs);
    for (u4 idx = 1; idx < finalGs->symbolsUsed(); idx++) {
        core::SymbolRef ref(finalGs.get(), idx);
        if (!hideSymbol(*finalGs, ref) && (ref.data(*finalGs)->owner.data(*finalGs)->loc().file() != fref ||
                                           ref.data(*finalGs)->owner == core::Symbols::root())) {
            for (auto definitionLocation : ref.data(*finalGs)->locs()) {
                if (definitionLocation.file() == fref) {
                    auto data = symbolRef2DocumentSymbol(*finalGs, ref, fref);
                    if (data) {
                        result.PushBack(move(*data), alloc);
                        break;
                    }
                }
            }
        }
    }
    sendResult(d, result);
    return finalGs;
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

unique_ptr<rapidjson::Value> LSPLoop::symbolRef2DocumentSymbol(const core::GlobalState &gs, core::SymbolRef symRef,
                                                               core::FileRef filter) {
    if (!symRef.exists()) {
        return nullptr;
    }
    auto sym = symRef.data(gs);
    if (!sym->loc().file().exists() || hideSymbol(gs, symRef)) {
        return nullptr;
    }
    rapidjson::Value result;
    result.SetObject();
    string prefix = "";
    if (sym->owner.exists() && sym->owner.data(gs)->isClass() && sym->owner.data(gs)->attachedClass(gs).exists()) {
        prefix = "self.";
    }
    result.AddMember("name", prefix + sym->name.show(gs), alloc);
    if (sym->isMethod()) {
        result.AddMember("detail", methodDetail(gs, symRef, nullptr, nullptr, nullptr), alloc);
    } else {
        // Currently released version of VSCode has a bug that requires this non-optional field to be present
        result.AddMember("detail", "", alloc);
    }
    result.AddMember("kind", symbolRef2SymbolKind(gs, symRef), alloc);
    result.AddMember("range", loc2Range(gs, sym->loc()),
                     alloc); // TODO: this range should cover body. Currently it doesn't.
    result.AddMember("selectionRange", loc2Range(gs, sym->loc()), alloc);

    rapidjson::Value children;
    children.SetArray();
    symbolRef2DocumentSymbolWalkMembers(gs, symRef, filter, children);
    if (sym->isClass()) {
        auto singleton = sym->lookupSingletonClass(gs);
        if (singleton.exists()) {
            symbolRef2DocumentSymbolWalkMembers(gs, singleton, filter, children);
        }
    }
    result.AddMember("children", children, alloc);

    return make_unique<rapidjson::Value>(move(result));
}

} // namespace sorbet::realmain::lsp