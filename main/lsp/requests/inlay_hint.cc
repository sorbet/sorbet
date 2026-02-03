#include "main/lsp/requests/inlay_hint.h"
#include "ast/treemap/treemap.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

// Collects locations of local variable assignments within a given range
class AssignmentCollector {
    core::Loc rangeLoc;
    vector<core::Loc> &assignmentLocs;

public:
    AssignmentCollector(core::Loc rangeLoc, vector<core::Loc> &assignmentLocs)
        : rangeLoc(rangeLoc), assignmentLocs(assignmentLocs) {}

    void postTransformAssign(core::Context ctx, const ast::Assign &assign) {
        auto local = ast::cast_tree<ast::Local>(assign.lhs);
        if (local == nullptr) {
            return;
        }

        auto assignLoc = ctx.locAt(assign.loc);
        auto localLoc = ctx.locAt(local->loc);

        if (!assignLoc.exists() || !localLoc.exists()) {
            return;
        }

        if (localLoc.beginPos() >= rangeLoc.beginPos() && localLoc.endPos() <= rangeLoc.endPos()) {
            assignmentLocs.push_back(localLoc);
        }
    }
};

} // namespace

InlayHintTask::InlayHintTask(const LSPConfiguration &config, MessageId id, unique_ptr<InlayHintParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentInlayHint), params(move(params)) {}

unique_ptr<ResponseMessage> InlayHintTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentInlayHint);

    const core::GlobalState &gs = typechecker.state();
    const auto &uri = params->textDocument->uri;
    auto fref = config.uri2FileRef(gs, uri);

    if (!fref.exists()) {
        response->result = vector<unique_ptr<InlayHint>>();
        return response;
    }

    // Only show inlay hints for files with some typing (at least typed: false)
    if (fref.data(gs).strictLevel < core::StrictLevel::False) {
        response->result = vector<unique_ptr<InlayHint>>();
        return response;
    }

    vector<unique_ptr<InlayHint>> hints;

    // Get the requested range
    auto startLoc = params->range->start->toLoc(gs, fref);
    auto endLoc = params->range->end->toLoc(gs, fref);

    if (!startLoc.has_value() || !endLoc.has_value()) {
        response->result = move(hints);
        return response;
    }

    // Create a range loc that spans from start to end
    core::Loc rangeLoc(fref, startLoc->beginPos(), endLoc->endPos());

    // Get the local var trees (preserves concrete syntax for finding assignments)
    auto localVarTree = typechecker.getLocalVarTrees(fref);
    if (localVarTree == nullptr) {
        response->result = move(hints);
        return response;
    }

    // Collect all local variable assignment locations in the range
    vector<core::Loc> assignmentLocs;
    AssignmentCollector collector(rangeLoc, assignmentLocs);
    core::Context ctx(gs, core::Symbols::root(), fref);
    ast::ConstTreeWalk::apply(ctx, collector, localVarTree);

    // For each assignment, query the type and create an inlay hint
    for (const auto &loc : assignmentLocs) {
        // Convert loc to Position for the query (LSP is 0-indexed, Sorbet is 1-indexed)
        auto details = loc.toDetails(gs);
        auto pos = make_unique<Position>(details.first.line - 1, details.first.column - 1);

        auto queryResult = LSPQuery::byLoc(config, typechecker, uri, *pos, LSPMethod::TextDocumentInlayHint, false);

        if (queryResult.error || queryResult.responses.empty()) {
            continue;
        }

        // Find an IdentResponse in the query results
        for (const auto &resp : queryResult.responses) {
            if (auto identResp = resp->isIdent()) {
                auto retType = identResp->retType.type;
                if (retType == nullptr || retType.isUntyped()) {
                    continue;
                }

                // Create the hint position at the end of the variable name (0-indexed)
                auto hintPos = make_unique<Position>(details.second.line - 1, details.second.column - 1);

                // Create the inlay hint with just the type (client handles formatting)
                auto hint = make_unique<InlayHint>(move(hintPos), retType.show(gs));
                hint->kind = InlayHintKind::Type;
                hint->paddingLeft = false;
                hint->paddingRight = true;

                hints.push_back(move(hint));
                break;
            }
        }
    }

    response->result = move(hints);
    return response;
}

} // namespace sorbet::realmain::lsp
