#include "main/lsp/requests/rename.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/AbstractRewriter.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/json_types.h"
#include <stdio.h>
using namespace std;

namespace sorbet::realmain::lsp {

namespace {

bool isValidRenameLocation(const core::SymbolRef &symbol, const core::GlobalState &gs,
                           unique_ptr<ResponseMessage> &response) {
    auto locs = symbol.locs(gs);
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
                            filetype, symbol.name(gs).show(gs), loc.filePosToString(gs));
            response->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, error);
            return false;
        }
    }
    return true;
}

class LocalRenamer : public AbstractRewriter {
public:
    LocalRenamer(const core::GlobalState &gs, const LSPConfiguration &config, const string newName,
                 vector<core::Loc> localUsages)
        : AbstractRewriter(gs, config), newName(newName), localUsages(localUsages) {
        // If the name is the same as before or empty, return an error. VS Code already prevents this on its own, but
        // other IDEs might not
        if (newName.empty()) {
            invalid = true;
            error = "The new name must not be empty.";
            return;
        }
    }

    ~LocalRenamer() {}
    void rename(unique_ptr<core::lsp::QueryResponse> &response) override {
        if (invalid) {
            return;
        }

        string source;

        for (auto &localUsage : localUsages) {
            source = localUsage.source(gs).value();
            edits[localUsage] = source[source.length() - 1] == ':' ? newName + ":" : newName;
        }
    }

private:
    string newName;
    vector<core::Loc> localUsages;
};

class MethodRenamer : public AbstractRewriter {
    string oldName;
    string newName;

public:
    MethodRenamer(const core::GlobalState &gs, const LSPConfiguration &config,
                  core::lsp::Query::Symbol::STORAGE &&symbols, const string oldName, const string newName)
        : AbstractRewriter(gs, config), oldName(oldName), newName(newName) {
        const vector<string> invalidNames = {"initialize", "call"};
        for (auto name : invalidNames) {
            if (oldName == name) {
                invalid = true;
                error = fmt::format("The `{}` method cannot be renamed.", oldName);
                return;
            }
        }
        // block any method not starting with /[a-zA-Z0-9_]+/. This blocks operator overloads.
        if (!isalnum(oldName[0]) && oldName[0] != '_') {
            error = fmt::format("The `{}` method cannot be renamed.", oldName);
            invalid = true;
        }

        for (const auto sym : symbols) {
            addSubclassRelatedMethods(gs, sym.asMethodRef(), getQueue());
        }
    }

    ~MethodRenamer() {}
    void rename(unique_ptr<core::lsp::QueryResponse> &response) override {
        if (invalid) {
            return;
        }
        auto loc = response->getLoc();

        // If we're renaming the exact same place twice, silently ignore it. We reach this condition when we find the
        // same method send through multiple definitions (e.g. in the case of union types)
        if (edits.contains(loc)) {
            return;
        }

        auto source = loc.source(gs);
        if (!source.has_value()) {
            return;
        }
        string newsrc;
        if (auto sendResp = response->isSend()) {
            newsrc = replaceMethodNameInSend(string(source.value()), sendResp);
        } else if (response->isMethodDef()) {
            newsrc = replaceMethodNameInDef(string(source.value()));
        } else {
            ENFORCE(0, "Unexpected query response type while renaming method");
            return;
        }
        edits[loc] = newsrc;
    }

private:
    string replaceMethodNameInSend(string source, const core::lsp::SendResponse *sendResp) {
        // find the method in the send expression
        auto methodNameLoc = sendResp->getMethodNameLoc(gs);
        if (!methodNameLoc) {
            // If there are locations we don't know how to rename, fail the entire rename operation
            invalid = true;
            if (methodNameLoc->file().exists()) {
                auto path = methodNameLoc->file().data(gs).path();
                error = fmt::format("Failed to rename `{}` method call at {}:{}", oldName, path,
                                    methodNameLoc->toDetails(gs).first.line);
            }
            return "";
        }
        // TODO(jez) Use Loc::adjust here?
        string::size_type methodNameOffset = methodNameLoc->beginPos() - sendResp->termLocOffsets.beginPos();
        auto newsrc = replaceAt(source, methodNameOffset);
        return newsrc;
    }

    string replaceMethodNameInDef(string def) {
        const vector<string> unsupportedDefPrefixes{"attr_reader", "attr_accessor", "attr_writer"};
        for (auto u : unsupportedDefPrefixes) {
            if (absl::StartsWith(def, u)) {
                invalid = true; // ensures the entire rename operation is blocked, not just this particular location
                error = fmt::format("Sorbet does not support renaming `{}`s", u);
                return def;
            }
        }

        // We can't do a simple string search because of cases like:
        //   `def foo(foobar)`
        //   `def de`
        //   `attr_reader :att`
        // Since we don't have more precise info about where the name is, we do this:
        //   1. if the definition starts with "def" (or another supported prefix) then skip over that
        //   2. and then change the first instance of oldName to newName
        string::size_type offset = 0;
        const vector<string> prefixes{"def"};
        for (auto prefix : prefixes) {
            if (absl::StartsWith(def, prefix)) {
                offset = prefix.length();
                break;
            }
        }
        auto pos = def.find(oldName, offset);
        if (pos == string::npos) {
            ENFORCE(0, "Method name should appear in definition");
            return def;
        }
        return replaceAt(def, pos);
    }
    string replaceAt(string_view input, string::size_type pos) {
        auto suffixOffset = pos + oldName.length();
        return absl::StrCat(input.substr(0, pos), newName, input.substr(suffixOffset, input.length() - suffixOffset));
    }
}; // MethodRenamer

class ConstRenamer : public AbstractRewriter {
    string newName;

public:
    ConstRenamer(const core::GlobalState &gs, const LSPConfiguration &config, core::SymbolRef symbol,
                 const string newName)
        : AbstractRewriter(gs, config), newName(newName) {
        if (!symbol.isMethod()) {
            getQueue()->tryEnqueue(symbol);
        }
    }

    ~ConstRenamer() {}

    void rename(unique_ptr<core::lsp::QueryResponse> &response) override {
        auto loc = response->getLoc();
        auto source = loc.source(gs);
        if (!source.has_value()) {
            return;
        }
        vector<string> strs = absl::StrSplit(source.value(), "::");
        strs[strs.size() - 1] = string(newName);
        auto newsrc = absl::StrJoin(strs, "::");
        edits[loc] = newsrc;
    }
};

class FieldRenamer : public AbstractRewriter {
    string newName;

public:
    FieldRenamer(const core::GlobalState &gs, const LSPConfiguration &config, core::FieldRef field,
                 const string newName)
        : AbstractRewriter(gs, config), newName(newName) {
        getQueue()->tryEnqueue(field);
    }

    ~FieldRenamer() {}

    void rename(unique_ptr<core::lsp::QueryResponse> &response) override {
        auto loc = response->getLoc();
        auto source = loc.source(gs);
        if (!source.has_value() || source.value().empty()) {
            return;
        }

        string newsrc;

        // If the source includes `@` in the first character, then we're renaming the instance variable and need to make
        // sure to include it. Otherwise, we're renaming an attr_reader/writer/accessor and need to make sure to not
        // include it. This allows users to not care about whether they need to add the `@` themselves.
        if (source.value()[0] == '@' && newName[0] != '@') {
            newsrc = "@" + newName;
        } else if (source.value()[0] != '@' && newName[0] == '@') {
            newsrc = newName.substr(1, newName.length() - 1);
        } else {
            newsrc = newName;
        }

        edits[loc] = newsrc;
    }
};

void enrichResponse(unique_ptr<ResponseMessage> &responseMsg, AbstractRewriter &renamer) {
    responseMsg->result = renamer.buildWorkspaceEdit();
    if (renamer.getInvalid()) {
        responseMsg->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, renamer.getError());
    }
}

bool canRenameMethod(const core::GlobalState &gs, core::MethodRef method) {
    if (!method.exists()) {
        return false;
    }

    auto data = method.data(gs);
    auto loc = data->loc();
    // TODO(jez) We can probably ~trivially support overloaded methods now
    return loc.exists() && !loc.file().data(gs).isStdlib() && !data->flags.isOverloaded &&
           !data->name.isOverloadName(gs);
}

unique_ptr<AbstractRewriter> makeRenamer(const core::GlobalState &gs,
                                         const sorbet::realmain::lsp::LSPConfiguration &config, core::SymbolRef symbol,
                                         const string newName) {
    auto loc = symbol.loc(gs);
    if (!loc.exists() || loc.file().data(gs).isStdlib()) {
        return nullptr;
    }

    ENFORCE(!symbol.isMethod(), "Need to make one separately, because renaming methods involves multiple symbols");
    if (symbol.isField(gs)) {
        return make_unique<FieldRenamer>(gs, config, symbol.asFieldRef(), newName);
    } else {
        return make_unique<ConstRenamer>(gs, config, symbol, newName);
    }
}

} // namespace
RenameTask::RenameTask(const LSPConfiguration &config, MessageId id, unique_ptr<RenameParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentRename), params(move(params)) {}

unique_ptr<ResponseMessage> RenameTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    const core::GlobalState &gs = typechecker.state();

    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentRename);

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.rename");

    if (params->newName.empty()) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, "No new name provided for rename request.");
        return response;
    }

    ShowOperation op(config, ShowOperation::Kind::Rename);

    auto result = LSPQuery::byLoc(config, typechecker, params->textDocument->uri, *params->position,
                                  LSPMethod::TextDocumentRename);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    // An explicit null indicates that we don't support this request (or that nothing was at the location).
    // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
    response->result = variant<JSONNullObject, unique_ptr<WorkspaceEdit>>(JSONNullObject());
    auto &queryResponses = result.responses;
    if (queryResponses.empty()) {
        return response;
    }

    auto resp = skipLiteralIfMethodDef(gs, queryResponses);
    if (auto constResp = resp->isConstant()) {
        // Sanity check the text.
        if (islower(params->newName[0])) {
            response->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                                         "Constant names must begin with an uppercase letter.");
            return response;
        }
        if (isValidRenameLocation(constResp->symbolBeforeDealias, gs, response)) {
            if (auto renamer = makeRenamer(gs, config, constResp->symbolBeforeDealias, params->newName)) {
                renamer->getEdits(typechecker);
                enrichResponse(response, *renamer);
            }
        }
    } else if (auto defResp = resp->isMethodDef()) {
        if (isValidRenameLocation(defResp->symbol, gs, response) && canRenameMethod(gs, defResp->symbol)) {
            auto symbols = core::lsp::Query::Symbol::STORAGE{1, defResp->symbol};
            auto renamer =
                MethodRenamer{gs, config, move(symbols), defResp->symbol.data(gs)->name.show(gs), params->newName};
            renamer.getEdits(typechecker);
            enrichResponse(response, renamer);
        }
    } else if (auto sendResp = resp->isSend()) {
        auto symbols = core::lsp::Query::Symbol::STORAGE{};
        for (auto start = sendResp->dispatchResult.get(); start != nullptr; start = start->secondary.get()) {
            auto method = start->main.method;
            if (!canRenameMethod(gs, method)) {
                continue;
            }

            symbols.emplace_back(method);
            // This could be a `prop` or `attr_*`, which has multiple associated symbols.
            addOtherAccessorSymbols(gs, method, symbols);
        }

        if (!symbols.empty()) {
            auto renamer = MethodRenamer{gs, config, move(symbols), sendResp->callerSideName.show(gs), params->newName};
            renamer.getEdits(typechecker);
            enrichResponse(response, renamer);
        }
    } else if (auto identResp = resp->isIdent()) {
        if (identResp->enclosingMethod.exists()) {
            auto references =
                typechecker.query(core::lsp::Query::createVarQuery(identResp->enclosingMethod,
                                                                   identResp->enclosingMethodLoc, identResp->variable),
                                  {identResp->termLoc.file()});
            vector<core::Loc> locations;

            for (auto &reference : references.responses) {
                locations.emplace_back(reference->getLoc());
            }

            unique_ptr<AbstractRewriter> renamer = make_unique<LocalRenamer>(gs, config, params->newName, locations);
            renamer->rename(resp);
            enrichResponse(response, *renamer);
        }
    } else if (auto fieldResp = resp->isField()) {
        if (auto renamer = makeRenamer(gs, config, fieldResp->symbol, params->newName)) {
            renamer->getEdits(typechecker);
            enrichResponse(response, *renamer);
        }
    }

    return response;
}

} // namespace sorbet::realmain::lsp
