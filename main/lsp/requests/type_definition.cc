#include "main/lsp/requests/type_definition.h"
#include "common/typecase.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
vector<core::Loc> locsForType(const core::GlobalState &gs, const core::TypePtr &type) {
    vector<core::Loc> result;
    if (type.isUntyped()) {
        return result;
    }
    typecase(
        type,
        [&](const core::ClassType &t) {
            for (auto loc : t.symbol.data(gs)->locs()) {
                result.emplace_back(loc);
            }
        },
        [&](const core::AppliedType &t) {
            for (auto loc : t.klass.data(gs)->locs()) {
                result.emplace_back(loc);
            }
        },
        [&](const core::OrType &t) {
            for (auto loc : locsForType(gs, t.left)) {
                result.emplace_back(loc);
            }
            for (auto loc : locsForType(gs, t.right)) {
                result.emplace_back(loc);
            }
        },
        [&](const core::AndType &t) {
            for (auto loc : locsForType(gs, t.left)) {
                result.emplace_back(loc);
            }
            for (auto loc : locsForType(gs, t.right)) {
                result.emplace_back(loc);
            }
        },
        [&](const core::SelfTypeParam &s) {
            for (auto loc : s.definition.locs(gs)) {
                result.emplace_back(loc);
            }
        },
        [&](const core::NewSelfType &s) {
            // TODO(jez) Probably want a test for this
            for (auto loc : locsForType(gs, s.upperBound)) {
                result.emplace_back(loc);
            }
        },
        [&](const core::LambdaParam &l) {
            for (auto loc : l.definition.data(gs)->locs()) {
                result.emplace_back(loc);
            }
        },
        [&](const core::ShapeType &_) {
            // nothing
        },
        [&](const core::TupleType &_) {
            // nothing
        },
        [&](const core::MetaType &_) {
            // nothing
        },
        [&](const core::AliasType &a) {
            ENFORCE(false, "Please add a test case for this test, and delete this enforce.");
            for (auto loc : a.symbol.locs(gs)) {
                result.emplace_back(loc);
            }
        },
        [&](const core::TypeVar &s) {
            ENFORCE(false, "Please add a test case for this test, and delete this enforce.");
        },
        [&](const core::TypePtr &t) {
            if (core::is_proxy_type(type)) {
                auto type = t.underlying(gs);
                result = locsForType(gs, type);
                return;
            }
            Exception::raise("Unhandled case in textDocument/typeDefinition: {}", core::TypePtr::tagToString(t.tag()));
        });
    return result;
}
} // namespace

TypeDefinitionTask::TypeDefinitionTask(const LSPConfiguration &config, MessageId id,
                                       unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentTypeDefinition), params(move(params)) {}

unique_ptr<ResponseMessage> TypeDefinitionTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentTypeDefinition);
    const core::GlobalState &gs = typechecker.state();
    const auto &uri = params->textDocument->uri;
    auto result =
        LSPQuery::byLoc(config, typechecker, uri, *params->position, LSPMethod::TextDocumentTypeDefinition, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    auto fref = config.uri2FileRef(gs, uri);
    // LSPQuery::byLoc reports an error if the file or loc don't exist
    auto queryLoc = params->position->toLoc(gs, fref).value();

    auto &queryResponses = result.responses;
    vector<unique_ptr<Location>> locations;
    if (!queryResponses.empty()) {
        const bool fileIsTyped = fref.data(gs).strictLevel >= core::StrictLevel::True;
        auto resp = skipLiteralIfMethodDef(gs, queryResponses);

        // Only support go-to-type-definition on constants and fields in untyped files.
        if (resp->isConstant() || resp->isField() || (fileIsTyped && (resp->isIdent() || resp->isLiteral()))) {
            for (auto loc : locsForType(gs, resp->getRetType())) {
                addLocIfExists(gs, locations, loc);
            }
        } else if (fileIsTyped && resp->isKeywordArg()) {
            // We only store one loc for a ParamInfo, which disguises the full set of locs if there are multiple.
            for (auto loc : locsForType(gs, resp->getRetType())) {
                addLocIfExists(gs, locations, loc);
            }

            // Loop over all (in case of multiple dispatch targets)
            for (const auto &resp : queryResponses) {
                if (resp == nullptr || !resp->isKeywordArg()) {
                    continue;
                }

                for (auto loc : locsForType(gs, resp->getRetType())) {
                    addLocIfExists(gs, locations, loc);
                }
            }
        } else if (fileIsTyped && resp->isSend()) {
            auto sendResp = resp->isSend();
            // Don't want to show hover results if we're hovering over, e.g., the arguments, and there's nothing there.
            if (sendResp->funLoc().exists() && sendResp->funLoc().contains(queryLoc)) {
                for (auto loc : locsForType(gs, sendResp->dispatchResult->returnType)) {
                    addLocIfExists(gs, locations, loc);
                }
            }
        }
    }
    response->result = move(locations);
    return response;
}
} // namespace sorbet::realmain::lsp
