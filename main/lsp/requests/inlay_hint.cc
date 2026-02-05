#include "main/lsp/requests/inlay_hint.h"
#include "ast/treemap/treemap.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

// Checks if source text represents a .new call (e.g., "Processor.new", "User.new(args)")
bool isNewCallSource(string_view source) {
    // Look for ".new" pattern - handles Foo.new, Foo.new(), Foo.new(args)
    auto pos = source.find(".new");
    if (pos == string_view::npos) {
        return false;
    }
    // Make sure it's ".new" at the end or followed by ( or whitespace
    auto afterNew = pos + 4;
    if (afterNew >= source.size()) {
        return true; // ".new" at end
    }
    char next = source[afterNew];
    return next == '(' || next == ' ' || next == '\n' || next == '\t';
}

// Collects locations of local variable assignments within a given range
class AssignmentCollector {
    core::Loc rangeLoc;
    vector<core::Loc> &assignmentLocs;
    string_view source;

public:
    AssignmentCollector(core::Loc rangeLoc, vector<core::Loc> &assignmentLocs, string_view source)
        : rangeLoc(rangeLoc), assignmentLocs(assignmentLocs), source(source) {}

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

        // Skip obvious instantiations like `processor = Processor.new`
        // Check the source text since the AST may be desugared
        auto assignSource = source.substr(assignLoc.beginPos(), assignLoc.endPos() - assignLoc.beginPos());
        if (isNewCallSource(assignSource)) {
            return;
        }

        if (localLoc.beginPos() >= rangeLoc.beginPos() && localLoc.endPos() <= rangeLoc.endPos()) {
            assignmentLocs.push_back(localLoc);
        }
    }
};

// Information about a method call for parameter hints
struct SendInfo {
    core::Loc funLoc;                  // Location of the method name (for querying)
    vector<core::Loc> argLocs;         // Locations of each positional argument
    vector<string_view> argTexts;      // Source text of each argument
};

// Collects method call locations within a given range for parameter hints
class SendCollector {
    core::Loc rangeLoc;
    vector<SendInfo> &sendInfos;
    string_view source;

public:
    SendCollector(core::Loc rangeLoc, vector<SendInfo> &sendInfos, string_view source)
        : rangeLoc(rangeLoc), sendInfos(sendInfos), source(source) {}

    void postTransformSend(core::Context ctx, const ast::Send &send) {
        auto funLoc = ctx.locAt(send.funLoc);
        if (!funLoc.exists()) {
            return;
        }

        // Only process sends within the range
        if (funLoc.beginPos() < rangeLoc.beginPos() || funLoc.endPos() > rangeLoc.endPos()) {
            return;
        }

        // Skip sends with no positional arguments
        auto numPosArgs = send.numPosArgs();
        if (numPosArgs == 0) {
            return;
        }

        SendInfo info;
        info.funLoc = funLoc;

        // Collect each positional argument location and source text
        for (auto i = 0; i < numPosArgs; i++) {
            auto &arg = send.getPosArg(i);
            auto argLoc = ctx.locAt(arg.loc());
            if (argLoc.exists()) {
                info.argLocs.push_back(argLoc);
                // Extract source text for the argument
                auto argText = source.substr(argLoc.beginPos(), argLoc.endPos() - argLoc.beginPos());
                info.argTexts.push_back(argText);
            }
        }

        if (!info.argLocs.empty()) {
            sendInfos.push_back(move(info));
        }
    }
};

} // namespace

InlayHintTask::InlayHintTask(const LSPConfiguration &config, MessageId id, unique_ptr<InlayHintParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentInlayHint), params(move(params)) {}

unique_ptr<ResponseMessage> InlayHintTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentInlayHint);

    const auto &inlayTypeHints = config.getClientConfig().inlayTypeHints;

    // Return empty if inlay hints are disabled
    if (inlayTypeHints == "off") {
        response->result = vector<unique_ptr<InlayHint>>();
        return response;
    }

    const core::GlobalState &gs = typechecker.state();
    const auto &uri = params->textDocument->uri;
    auto fref = config.uri2FileRef(gs, uri);

    if (!fref.exists()) {
        response->result = vector<unique_ptr<InlayHint>>();
        return response;
    }

    // Only show inlay hints for files with typed: true or higher (typed: false doesn't have reliable type info)
    if (fref.data(gs).strictLevel < core::StrictLevel::True) {
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

    auto source = fref.data(gs).source();

    // Collect all local variable assignment locations in the range
    vector<core::Loc> assignmentLocs;
    AssignmentCollector assignCollector(rangeLoc, assignmentLocs, source);
    core::Context ctx(gs, core::Symbols::root(), fref);
    ast::ConstTreeWalk::apply(ctx, assignCollector, localVarTree);

    // Also collect method calls for parameter hints
    vector<SendInfo> sendInfos;
    SendCollector sendCollector(rangeLoc, sendInfos, source);
    ast::ConstTreeWalk::apply(ctx, sendCollector, localVarTree);

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

                auto typeStr = retType.show(gs);
                unique_ptr<Position> hintPos;
                string label;
                bool paddingLeft = false;
                bool paddingRight = false;

                if (inlayTypeHints == "before_var") {
                    // Position at start of variable, label is "TypeName "
                    hintPos = make_unique<Position>(details.first.line - 1, details.first.column - 1);
                    label = typeStr + " ";
                    paddingRight = false;
                } else if (inlayTypeHints == "after_var") {
                    // Position at end of variable name, label is ": TypeName"
                    hintPos = make_unique<Position>(details.second.line - 1, details.second.column - 1);
                    label = ": " + typeStr;
                    paddingLeft = false;
                    paddingRight = true;
                } else if (inlayTypeHints == "RBS") {
                    // Position at end of line, label is " #: TypeName"
                    auto lineNum = details.first.line;
                    auto lineContent = fref.data(gs).getLine(lineNum);
                    // Find length excluding trailing newline
                    auto lineLen = lineContent.length();
                    if (lineLen > 0 && lineContent[lineLen - 1] == '\n') {
                        lineLen--;
                    }
                    hintPos = make_unique<Position>(lineNum - 1, lineLen);
                    label = " #: " + typeStr;
                    paddingLeft = false;
                    paddingRight = false;
                } else {
                    // Unknown style, skip
                    continue;
                }

                auto hint = make_unique<InlayHint>(move(hintPos), label);
                hint->kind = InlayHintKind::Type;
                hint->paddingLeft = paddingLeft;
                hint->paddingRight = paddingRight;

                hints.push_back(move(hint));
                break;
            }
        }
    }

    // Generate parameter name hints for method calls
    for (const auto &sendInfo : sendInfos) {
        // Query at the method name location to get the SendResponse
        auto details = sendInfo.funLoc.toDetails(gs);
        auto pos = make_unique<Position>(details.first.line - 1, details.first.column - 1);

        auto queryResult = LSPQuery::byLoc(config, typechecker, uri, *pos, LSPMethod::TextDocumentInlayHint, false);

        if (queryResult.error || queryResult.responses.empty()) {
            continue;
        }

        // Find a SendResponse in the query results
        for (const auto &resp : queryResult.responses) {
            if (auto sendResp = resp->isSend()) {
                auto method = sendResp->dispatchResult->main.method;
                if (!method.exists()) {
                    continue;
                }

                const auto &params = method.data(gs)->parameters;
                auto numPosArgs = min(sendInfo.argLocs.size(), params.size());

                for (size_t i = 0; i < numPosArgs; i++) {
                    const auto &param = params[i];
                    // Skip block and keyword-only parameters
                    if (param.flags.isBlock || param.flags.isKeyword) {
                        continue;
                    }

                    auto paramName = string(param.parameterName(gs));
                    auto argText = sendInfo.argTexts[i];

                    // Skip if argument text exactly matches parameter name
                    if (argText == paramName) {
                        continue;
                    }

                    // Create a parameter hint before the argument
                    auto argDetails = sendInfo.argLocs[i].toDetails(gs);
                    auto hintPos = make_unique<Position>(argDetails.first.line - 1, argDetails.first.column - 1);
                    auto label = paramName + ":";

                    auto hint = make_unique<InlayHint>(move(hintPos), label);
                    hint->kind = InlayHintKind::Parameter;
                    hint->paddingLeft = false;
                    hint->paddingRight = true;

                    hints.push_back(move(hint));
                }
                break;
            }
        }
    }

    response->result = move(hints);
    return response;
}

} // namespace sorbet::realmain::lsp
