#include "main/lsp/requests/code_action_resolve.h"
#include "core/insert_method/insert_method.h"
#include "main/lsp/ConvertToSingletonClassMethod.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/MoveMethod.h"
#include "main/lsp/ShowOperation.h"

using namespace std;
namespace sorbet::realmain::lsp {

namespace {

bool allowedCodeActionKind(optional<CodeActionKind> codeActionKind) {
    if (!codeActionKind.has_value()) {
        return false;
    }

    switch (*codeActionKind) {
        case CodeActionKind::Refactor:
        case CodeActionKind::RefactorExtract:
        case CodeActionKind::RefactorRewrite:
            return true;
        default:
            return false;
    }
}

} // namespace

CodeActionResolveTask::CodeActionResolveTask(const LSPConfiguration &config, MessageId id,
                                             unique_ptr<CodeAction> params)
    : LSPRequestTask(config, move(id), LSPMethod::CodeActionResolve), params(move(params)) {}

unique_ptr<ResponseMessage> CodeActionResolveTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::CodeActionResolve);
    if (!allowedCodeActionKind(params->kind) || !params->data.has_value()) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, "Invalid `codeAction/resolve` request");
        return response;
    }

    auto &gs = typechecker.state();

    if (auto &maybeInsertOverride = params->data.value()->insertOverride) {
        auto &insertOverride = maybeInsertOverride.value();
        if (insertOverride->method <= 0 || insertOverride->method >= gs.methodsUsed()) {
            response->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                                         "Invalid method ID in code action resolve request");
            return response;
        }
        auto method = core::MethodRef::fromRaw(insertOverride->method);
        if (insertOverride->inWhere <= 0 || insertOverride->inWhere >= gs.methodsUsed()) {
            response->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                                         "Invalid class-or-module ID in code action resolve request");
            return response;
        }
        auto inWhere = core::ClassOrModuleRef::fromRaw(insertOverride->inWhere);
        auto fref = config.uri2FileRef(gs, insertOverride->textDocument->uri);
        auto termLoc = insertOverride->termLocation->toLoc(gs, fref);
        auto declLoc = insertOverride->declLocation->toLoc(gs, fref);
        if (!termLoc.has_value() || !declLoc.has_value()) {
            response->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                                         "Failed to convert locations when inserting override methods");
            return response;
        }
        auto toInsert = vector<core::MethodRef>{1, method};
        auto edits =
            core::insert_method::run(gs, toInsert, inWhere, declLoc.value(), termLoc.value().copyEndWithZeroLength());
        auto action = move(params);
        action->edit = make_unique<WorkspaceEdit>();
        action->edit.value()->documentChanges = autocorrect2DocumentEdits(config, gs, edits);
        response->result = move(action);
        return response;
    }

    const auto &actualParams = params->data.value()->params;

    const auto queryResult = LSPQuery::byLoc(config, typechecker, actualParams->textDocument->uri,
                                             *actualParams->range->start, LSPMethod::CodeActionResolve, false);

    if (queryResult.error != nullptr) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, "Invalid `codeAction/resolve` request");
        return response;
    }

    ShowOperation op(config, ShowOperation::Kind::MoveMethod);

    for (const auto &resp : queryResult.responses) {
        const auto *def = resp->isMethodDef();
        if (def == nullptr) {
            continue;
        }

        unique_ptr<CodeAction> action;
        if (def->symbol.data(gs)->owner.data(gs)->isSingletonClass(gs)) {
            action = make_unique<CodeAction>("Move method to a new module");
            action->kind = CodeActionKind::RefactorExtract;
            auto workspaceEdit = make_unique<WorkspaceEdit>();
            workspaceEdit->documentChanges = getMoveMethodEdits(typechecker, config, *def);
            action->edit = move(workspaceEdit);
        } else {
            action = make_unique<CodeAction>("Convert to singleton class method (best effort)");
            action->kind = CodeActionKind::RefactorRewrite;
            auto workspaceEdit = make_unique<WorkspaceEdit>();
            auto edits = convertToSingletonClassMethod(typechecker, config, *def);
            workspaceEdit->documentChanges = move(edits);
            action->edit = move(workspaceEdit);
        }
        response->result = move(action);
    }

    if (response->result == nullopt) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, "Invalid `codeAction/resolve` request");
        return response;
    }

    return response;
}

} // namespace sorbet::realmain::lsp
