#include "main/lsp/QueryCollector.h"

using namespace std;
namespace sorbet::realmain::lsp {
namespace {
// In the case of location ties, determines which query response takes priority in the vector constructed by
// flushErrors. Larger values means greater specificity.
uint16_t getQueryResponseTypeSpecificity(const core::lsp::QueryResponse &q) {
    // TODO(jez) C++26: Convert this to variant::visit instance method
    return visit(
        [](auto &&res) -> uint16_t {
            using T = decay_t<decltype(res)>;
            if constexpr (is_same_v<T, core::lsp::EditResponse>) {
                // Only reported for autocomplete, and should take precedence over anything else reported
                return 9;
            } else if constexpr (is_same_v<T, core::lsp::MethodDefResponse>) {
                return 8;
            } else if constexpr (is_same_v<T, core::lsp::SendResponse>) {
                return 7;
            } else if constexpr (is_same_v<T, core::lsp::FieldResponse>) {
                return 6;
            } else if constexpr (is_same_v<T, core::lsp::IdentResponse>) {
                return 5;
            } else if constexpr (is_same_v<T, core::lsp::ConstantResponse>) {
                return 4;
            } else if constexpr (is_same_v<T, core::lsp::KeywordArgResponse>) {
                return 3;
            } else if constexpr (is_same_v<T, core::lsp::LiteralResponse>) {
                return 2;
            } else if constexpr (is_same_v<T, core::lsp::ClassDefResponse>) {
                return 1;
            } else {
                static_assert(always_false_v<T>, "Should never happen, as the above checks should be exhaustive.");
            }
        },
        q.asResponse());
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
        auto leftLength = leftTermLoc.length();
        auto rightLength = rightTermLoc.length();
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
