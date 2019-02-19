#include "dsl/custom/Matcher.h"
#include "core/GlobalState.h"
#include <sstream>

using namespace std;
using namespace sorbet::ast;

namespace sorbet::dsl::custom {
using MatchResult = Matcher::MatchResult;

NameMatcher::NameMatcher(std::string &&expected) : expected(expected) {}

Capture::Capture(int captureIndex, LiteralType variant, core::NameRef name)
    : captureIndex(captureIndex), variant(variant), name(name) {}

static void concatenateInto(std::vector<Capture> &dest, const std::vector<Capture> &source) {
    dest.insert(dest.end(), source.begin(), source.end());
}

MatchResult NameMatcher::match(core::NameRef ref, core::GlobalState &gs) {
    MatchResult result;
    if (ref.data(gs)->shortName(gs) == expected) {
        result.second = true;
        if (captureIndex) {
            result.first.emplace_back(captureIndex, Capture::LiteralType::Symbol, ref);
        }
    }
    return result;
}

MatchResult WildCardMatcher::match(Expression *matchee, core::GlobalState &) {
    MatchResult result;
    result.second = true;
    return result;
}

MatchResult SelfMatcher::match(Expression *matchee, core::GlobalState &) {
    MatchResult result;
    result.second = isa_tree<Self>(matchee);
    return result;
}

ConstantMatcher::ConstantMatcher(ConstantMatcher::Components_store &&componentsRightToLeft)
    : componentsRightToLeft(componentsRightToLeft) {}

MatchResult ConstantMatcher::match(Expression *matchee, core::GlobalState &ctx) {
    MatchResult result;

    bool matched = true;
    if (wildcardComponents) {
        matched = isa_tree<UnresolvedConstantLit>(matchee);
    } else {
        Expression *currentlyMatching = matchee;
        for (const auto &component : componentsRightToLeft) {
            auto constLit = cast_tree<UnresolvedConstantLit>(currentlyMatching);
            if (constLit == nullptr || constLit->cnst.data(ctx)->shortName(ctx) != component) {
                matched = false;
                break;
            }
            currentlyMatching = constLit->scope.get();
        }
        // TODO: no way to match constants that start with `::` since in that case the inner-most
        // tree is a ConstantLit (root) not an EmptyTree
        if (!isa_tree<EmptyTree>(currentlyMatching)) {
            matched = false;
        }
    }

    if (matched) {
        result.second = true;
        if (captureIndex) {
            // this format is expected by the tree template filler
            stringstream flattened("c!");
            Expression *current = matchee;
            bool first = true;
            UnresolvedConstantLit *constLit;
            while ((constLit = cast_tree<UnresolvedConstantLit>(current))) {
                if (!first) {
                    flattened << '|';
                }
                flattened << constLit->cnst.data(ctx)->shortName(ctx);
                current = constLit->scope.get();
                first = false;
            }
            string reverseConst = flattened.str();
            auto ref = static_cast<core::GlobalState &>(ctx).enterNameUTF8(
                string_view(reverseConst.c_str(), reverseConst.size()));
            result.first.emplace_back(captureIndex, Capture::LiteralType::Constant, ref);
        }
    }

    return result;
}

SendMatcher::SendMatcher(NameMatcher &&calleeMatcher, std::unique_ptr<Matcher> recvMatcher,
                         std::unique_ptr<GroupMatcher> argMatcher, std::unique_ptr<Matcher> blockMatcher)
    : calleeMatcher(std::move(calleeMatcher)), recvMatcher(std::move(recvMatcher)), argMatcher(std::move(argMatcher)),
      blockMatcher(std::move(blockMatcher)) {}

MatchResult SendMatcher::match(Expression *matchee, core::GlobalState &gs) {
    MatchResult noMatch;
    Send *send = cast_tree<Send>(matchee);
    if (!send) {
        return noMatch;
    }

    auto calleeNameResult = calleeMatcher.match(send->fun, gs);
    if (!calleeNameResult.second) {
        return noMatch;
    }
    auto recvResult = recvMatcher->match(send->recv.get(), gs);
    if (!recvResult.second) {
        return noMatch;
    }
    auto argResult = argMatcher->match(send->args.begin(), send->args.end(), gs);
    if (!argResult.second) {
        return noMatch;
    }
    auto blockResult = blockMatcher->match(send->block.get(), gs);
    if (!blockResult.second) {
        return noMatch;
    }

    MatchResult result;
    result.first = std::move(calleeNameResult.first);
    concatenateInto(result.first, recvResult.first);
    concatenateInto(result.first, argResult.first);
    concatenateInto(result.first, blockResult.first);
    result.second = true;
    return result;
}

GroupMatcher::GroupMatcher(GroupMatcher::MatchGroup_store &&expectedSequence)
    : expectedSequence(std::move(expectedSequence)) {}

Matcher::MatchResult GroupMatcher::match(ExpressionIterator start, ExpressionIterator end, core::GlobalState &gs) {
    MatchResult result, noMatch;
    if ((end - start) != expectedSequence.size()) {
        result.second = false;
        return result;
    }

    if (expectedSequence.empty() && start == end) {
        result.second = true;
        return result;
    }

    int matcherIdx = 0;
    for (auto it = start; it != end; it++, matcherIdx++) {
        auto &itMatcher = expectedSequence.at(matcherIdx);
        MatchResult itResult = itMatcher->match(it->get(), gs);
        if (itResult.second) {
            concatenateInto(result.first, itResult.first);
        } else {
            return noMatch;
        }
    }

    result.second = true;
    return result;
}

LiteralMatcher::LiteralMatcher(Capture::LiteralType expectedType) : expectedType(expectedType) {}

LiteralMatcher::LiteralMatcher(Capture::LiteralType expectedType, std::string &&expectedContent)
    : expectedContent(expectedContent), expectedType(std::move(expectedType)) {}

MatchResult LiteralMatcher::match(Expression *matchee, core::GlobalState &ctx) {
    MatchResult empty, result;
    auto lit = cast_tree<Literal>(matchee);
    if (!lit) {
        return empty;
    }

    if (expectedType == Capture::LiteralType::String) {
        result.second = lit->isString(ctx);
    } else if (expectedType == Capture::LiteralType::Symbol) {
        result.second = lit->isSymbol(ctx);
    } else if (expectedType == Capture::LiteralType::True) {
        result.second = lit->isTrue(ctx);
    } else if (expectedType == Capture::LiteralType::False) {
        result.second = lit->isFalse(ctx);
    } else if (expectedType == Capture::LiteralType::Nil) {
        result.second = lit->isNil(ctx);
    }

    if (result.second && !wildcardContent) {
        result.second = (lit->asString(ctx).data(ctx)->shortName(ctx) == expectedContent);
    }

    if (result.second && captureIndex) {
        core::NameRef ref;
        if (lit->isString(ctx)) {
            ref = lit->asString(ctx);
        } else if (lit->isSymbol(ctx)) {
            ref = lit->asSymbol(ctx);
        } else {
            ref = static_cast<core::GlobalState &>(ctx).enterNameUTF8("<unkonwn literal>"sv);
        }
        result.first.emplace_back(captureIndex, expectedType, ref);
    }
    return result;
}

} // namespace sorbet::dsl::custom
