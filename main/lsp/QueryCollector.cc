#include "main/lsp/QueryCollector.h"

using namespace std;
namespace sorbet::realmain::lsp {
namespace {
// In the case of location ties, determines which query response takes priority in the vector constructed by
// flushErrors. Larger values means greater specificity.
uint16_t getQueryResponseTypeSpecificity(const core::lsp::QueryResponse &q) {
    if (q.isEdit()) {
        // Only reported for autocomplete, and should take precedence over anything else reported
        return 8;
    } else if (q.isMethodDef()) {
        return 7;
    } else if (q.isSend()) {
        return 6;
    } else if (q.isField()) {
        return 5;
    } else if (q.isIdent()) {
        return 4;
    } else if (q.isConstant()) {
        return 3;
    } else if (q.isLiteral()) {
        return 2;
    } else {
        return 1;
    }
}
} // namespace
void QueryCollector::flushErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                                 vector<unique_ptr<core::ErrorQueueMessage>> errors) {
    for (auto &error : errors) {
        if (error->kind == core::ErrorQueueMessage::Kind::QueryResponse) {
            queryResponses.emplace_back(move(error->queryResponse));
        }
    }
}

vector<unique_ptr<core::lsp::QueryResponse>> QueryCollector::drainQueryResponses() {
    stable_sort(queryResponses.begin(), queryResponses.end(), [](auto &left, auto &right) -> bool {
        /* we want the most precise information to go first. Normally, they are computed in this order by
        construction, but threading artifact might reorder them, thus we'd like to sort them */
        auto leftTermLoc = left->getLoc();
        auto rightTermLoc = right->getLoc();
        auto leftLength = leftTermLoc.endPos() - leftTermLoc.beginPos();
        auto rightLength = rightTermLoc.endPos() - rightTermLoc.beginPos();
        if (leftLength != rightLength) {
            return leftLength < rightLength;
        }
        if (leftTermLoc.beginPos() != rightTermLoc.beginPos()) {
            return leftTermLoc.beginPos() < rightTermLoc.beginPos();
        }
        if (leftTermLoc.endPos() != rightTermLoc.endPos()) {
            return leftTermLoc.endPos() < rightTermLoc.endPos();
        }
        // Locations tie! Tiebreak with the expected specificity of the response.
        return getQueryResponseTypeSpecificity(*left) > getQueryResponseTypeSpecificity(*right);
    });

    return move(queryResponses);
};
} // namespace sorbet::realmain::lsp
