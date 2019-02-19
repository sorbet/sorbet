#ifndef SORBET_CUSTOM_REPLACE_H
#define SORBET_CUSTOM_REPLACE_H

#include "ast/Trees.h"
#include "common/common.h"
#include "dsl/custom/Matcher.h"
#include <optional>

namespace sorbet::dsl::custom {
class TreeTemplate;
class CustomReplace final {
public:
    ~CustomReplace();
    CustomReplace(CustomReplace &&);
    CustomReplace(CustomReplace &) = delete;

    static const std::string specSchema;
    static std::optional<CustomReplace> parseDefinition(core::GlobalState &gs, std::string_view jsonDefinition);
    std::vector<std::unique_ptr<ast::Expression>> matchAndReplace(core::GlobalState &gs, ast::Expression *matchee);

private:
    CustomReplace() = default;
    std::unique_ptr<Matcher> matcher;
    std::unique_ptr<TreeTemplate> treeTemplate;
};
} // namespace sorbet::dsl::custom

#endif // SORBET_CUSTOM_REPLACE_H
