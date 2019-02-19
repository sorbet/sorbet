#ifndef SORBET_TREE_MATCHER_H
#define SORBET_TREE_MATCHER_H

#include "ast/Trees.h"
#include "common/common.h"

namespace sorbet::dsl::custom {

class Capture {
public:
    enum class LiteralType { String, Symbol, Constant, Nil, True, False };
    int captureIndex;
    LiteralType variant;
    core::NameRef name;
    Capture(int captureIndex, LiteralType variant, core::NameRef name);
};

class Matcher {
public:
    using MatchResult = std::pair<std::vector<Capture>, bool>;
    virtual ~Matcher() = default;
    virtual MatchResult match(ast::Expression *matchee, core::GlobalState &ctx) = 0;
};

class NameMatcher {
public:
    int captureIndex = 0;
    std::string expected;
    NameMatcher(const NameMatcher &) = default;
    NameMatcher(NameMatcher &&) = default;
    NameMatcher(std::string &&expected);
    Matcher::MatchResult match(core::NameRef ref, core::GlobalState &ctx);
};

// ####PRE-PUSH: do we still need this?
class GroupMatcher {
public:
    static constexpr int EXPECTED_SEQUENCE_LENGTH = 2;
    using MatchGroup_store = InlinedVector<std::unique_ptr<Matcher>, EXPECTED_SEQUENCE_LENGTH>;
    using ExpressionIterator = InlinedVector<std::unique_ptr<ast::Expression>, 1>::iterator;

    MatchGroup_store expectedSequence;

    GroupMatcher() = delete;
    GroupMatcher(MatchGroup_store &&expectedSequence);
    Matcher::MatchResult match(ExpressionIterator start, ExpressionIterator end, core::GlobalState &ctx);
};

class WildCardMatcher : public Matcher {
public:
    WildCardMatcher() = default;
    virtual MatchResult match(ast::Expression *matchee, core::GlobalState &ctx) override;
};

class SelfMatcher : public Matcher {
public:
    virtual MatchResult match(ast::Expression *matchee, core::GlobalState &ctx) override;
};

class ConstantMatcher : public Matcher {
public:
    static constexpr int EXPECTED_RHS_COUNT = 4;
    using Components_store = InlinedVector<std::string, EXPECTED_RHS_COUNT>;

    Components_store componentsRightToLeft;
    bool wildcardComponents = false;
    int captureIndex = 0;

    ConstantMatcher(Components_store &&componentsRightToLeft);
    virtual MatchResult match(ast::Expression *matchee, core::GlobalState &ctx) override;
};

class SendMatcher : public Matcher {
public:
    NameMatcher calleeMatcher;
    std::unique_ptr<Matcher> recvMatcher;
    std::unique_ptr<GroupMatcher> argMatcher;
    std::unique_ptr<Matcher> blockMatcher;

    SendMatcher(NameMatcher &&calleeMatcher, std::unique_ptr<Matcher> recvMatcher,
                std::unique_ptr<GroupMatcher> argMatcher, std::unique_ptr<Matcher> blockMatcher);
    virtual MatchResult match(ast::Expression *matchee, core::GlobalState &ctx) override;
};

class LiteralMatcher : public Matcher {
public:
    std::string expectedContent;
    Capture::LiteralType expectedType;
    int captureIndex = 0;
    bool wildcardContent = false;

    LiteralMatcher(Capture::LiteralType expectedType);
    LiteralMatcher(Capture::LiteralType expectedType, std::string &&expectedContent);

    virtual MatchResult match(ast::Expression *matchee, core::GlobalState &ctx) override;
};
} // namespace sorbet::dsl::custom

#endif // SORBET_TREE_MATCHER_H
