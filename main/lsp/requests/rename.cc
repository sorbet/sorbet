#include "main/lsp/requests/rename.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"
#include <iostream>
#include <stdio.h>

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
bool isValidRenameLocation(const core::SymbolRef &symbol, const core::GlobalState &gs,
                           unique_ptr<ResponseMessage> &response) {
    auto locs = symbol.data(gs)->locs();
    string filetype;
    for (auto loc : locs) {
        if (loc.file().data(gs).isRBI()) {
            filetype = ".rbi";
        } else if (loc.file().data(gs).isPayload()) {
            filetype = "payload";
        }

        if (!filetype.empty()) {
            auto error =
                fmt::format("Renaming constants defined in {} files is not supported; symbol {} is defined at {}",
                            filetype, symbol.data(gs)->name.show(gs), loc.filePosToString(gs));
            response->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, error);
            return false;
        }
    }
    return true;
}

// Replaces originalName with newName in the last dotted component if the string is of the form x.y.z; otherwise does a
// simple ReplaceAll.
std::string replaceLastDotted(std::string_view input, std::string_view originalName, std::string_view newName) {
    if (absl::StrContains(input, ".")) {
        std::vector<std::string> dotted = absl::StrSplit(input, ".");
        // TODO: check for exactly one match
        dotted[dotted.size() - 1] = absl::StrReplaceAll(dotted[dotted.size() - 1], {{originalName, newName}});
        return absl::StrJoin(dotted, ".");
    }
    return absl::StrReplaceAll(input, {{originalName, newName}});
}

// returns true if s is subclass of root; also updates isSubclass, visited, and subclasses vectors
bool updateSubclass(const core::GlobalState &gs, core::SymbolRef root, core::SymbolRef s, vector<bool> &isSubclass,
                    vector<bool> &visited, vector<core::SymbolRef> &subclasses) {
    if (visited[s.rawId()] == true) {
        return isSubclass[s.rawId()];
    }
    visited[s.rawId()] = true;
    if (s.rawId() == root.rawId()) {
        subclasses.push_back(s);
        isSubclass[s.rawId()] = true;
        return true;
    }
    auto super = s.data(gs)->superClass();
    if (super.exists()) {
        if (updateSubclass(gs, root, super, isSubclass, visited, subclasses)) {
            subclasses.push_back(s);
            isSubclass[super.rawId()] = true;
            return true;
        }
    }
    isSubclass[s.rawId()] = false;
    return false;
}

// returns all subclasses of root (including root)
vector<core::SymbolRef> getSubclasses(LSPTypecheckerDelegate &typechecker, core::SymbolRef root) {
    const core::GlobalState &gs = typechecker.state();
    vector<bool> isSubclass(gs.classAndModulesUsed());
    vector<bool> visited(gs.classAndModulesUsed());
    vector<core::SymbolRef> subclasses;
    for (u4 i = 1; i < gs.classAndModulesUsed(); ++i) {
        auto s = core::SymbolRef(&gs, core::SymbolRef::Kind::ClassOrModule, i);
        updateSubclass(gs, root, s, isSubclass, visited, subclasses);
    }
    return subclasses;
}

} // namespace

variant<JSONNullObject, unique_ptr<WorkspaceEdit>>
RenameTask::getRenameEdits(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol, std::string_view newName) {
    const core::GlobalState &gs = typechecker.state();
    auto originalName = symbol.data(gs)->name.show(gs);
    auto we = make_unique<WorkspaceEdit>();

    cout << "originalName/newName: " << originalName << "/" << newName << "\n";

    // check for this symbol as part of a class hierarchy: follow superClass() links till we find the root; then find
    // the full tree; then look for methods with the same name as ours; then find all references to all those methods.
    // Rename all defs and references and we're done.

    auto symbolClass = symbol.data(gs)->enclosingClass(gs);

    // find root
    auto root = symbolClass;
    while (true) {
        cout << "Path to root: " << root.data(gs)->show(gs) << "\n";
        auto tmp = root.data(gs)->superClass();
        // TODO is there a better way to do this check for the base object class?
        if (!tmp.exists() || tmp.data(gs)->show(gs) == "Object") {
            break;
        }
        root = tmp;
    }
    cout << "Root: " << root.data(gs)->show(gs) << "\n";

    // and the tree
    auto subclasses = getSubclasses(typechecker, root);

    // find the target method definition in each subclass
    vector<core::SymbolRef> methods;
    for (auto c : subclasses) {
        cout << "class: " << c.data(gs)->show(gs) << "\n";
        auto classSymbol = c.data(gs);
        auto member = classSymbol->findMember(gs, symbol.data(gs)->name);
        if (member.exists()) {
            cout << "method: " << member.data(gs)->show(gs) << "\n";
            methods.push_back(member);
        }
    }

    UnorderedMap<string, vector<unique_ptr<TextEdit>>> edits;

    // find all references to each symbol
    // TODO is it possible to do one query with many symbols, and would that be faster?
    for (auto method : methods) {
        vector<unique_ptr<Location>> references = getReferencesToSymbol(typechecker, method);

        for (auto &location : references) {
            // Get text at location.
            auto fref = config.uri2FileRef(gs, location->uri);
            if (fref.data(gs).isPayload()) {
                // We don't support renaming things in payload files.
                return JSONNullObject();
            }

            auto loc = location->range->toLoc(gs, fref);
            auto source = loc.source(gs);

            if (absl::StrContains(source, "::")) {
                std::vector<std::string> strs = absl::StrSplit(source, "::");

                strs[strs.size() - 1] = replaceLastDotted(strs[strs.size() - 1], originalName, newName);

                auto newsrc = absl::StrJoin(strs, "::");
                cout << "Replace: " << source << "/" << newsrc << "\n";
                edits[location->uri].push_back(make_unique<TextEdit>(move(location->range), newsrc));
            } else {
                auto newsrc = replaceLastDotted(source, originalName, newName);
                cout << "Replace: " << source << "/" << newsrc << "\n";
                edits[location->uri].push_back(make_unique<TextEdit>(move(location->range), newsrc));
            }
        }
    }
    vector<unique_ptr<TextDocumentEdit>> textDocEdits;
    for (auto &item : edits) {
        // TODO: Version.
        textDocEdits.push_back(make_unique<TextDocumentEdit>(
            make_unique<VersionedTextDocumentIdentifier>(item.first, JSONNullObject()), move(item.second)));
    }
    we->documentChanges = move(textDocEdits);

    return we;
}

RenameTask::RenameTask(const LSPConfiguration &config, MessageId id, unique_ptr<RenameParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentRename), params(move(params)) {}

unique_ptr<ResponseMessage> RenameTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    const core::GlobalState &gs = typechecker.state();

    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentRename);
    if (!config.opts.lspRenameEnabled) {
        response->error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidRequest, "The `Rename` LSP feature is experimental and disabled by default.");
        return response;
    }

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.rename");

    if (params->newName.empty()) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, "No new name provided for rename request.");
        return response;
    }

    ShowOperation op(config, ShowOperation::Kind::Rename);

    auto result = queryByLoc(typechecker, params->textDocument->uri, *params->position, LSPMethod::TextDocumentRename);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
    } else {
        // An explicit null indicates that we don't support this request (or that nothing was at the location).
        // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
        response->result = variant<JSONNullObject, unique_ptr<WorkspaceEdit>>(JSONNullObject());
        auto &queryResponses = result.responses;
        if (!queryResponses.empty()) {
            auto resp = move(queryResponses[0]);
            // Only supports rename requests from constants and class definitions.
            if (auto constResp = resp->isConstant()) {
                // Sanity check the text.
                if (islower(params->newName[0])) {
                    response->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                                                 "Constant names must begin with an uppercase letter.");
                    return response;
                }
                if (isValidRenameLocation(constResp->symbol, gs, response)) {
                    response->result = getRenameEdits(typechecker, constResp->symbol, params->newName);
                }
            } else if (auto defResp = resp->isDefinition()) {
                if (defResp->symbol.data(gs)->isClassOrModule() || defResp->symbol.data(gs)->isMethod()) {
                    if (isValidRenameLocation(defResp->symbol, gs, response)) {
                        response->result = getRenameEdits(typechecker, defResp->symbol, params->newName);
                    }
                }
            } else if (auto sendResp = resp->isSend()) {
                auto method = sendResp->dispatchResult->main.method;
                response->result = getRenameEdits(typechecker, method, params->newName);
            }
        }
    }

    return response;
}

} // namespace sorbet::realmain::lsp
