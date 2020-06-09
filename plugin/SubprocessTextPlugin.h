#ifndef SORBET_PLUGIN_SUBPROCESS_TEXT_H
#define SORBET_PLUGIN_SUBPROCESS_TEXT_H
#include "ast/ast.h"

namespace sorbet::plugin {

class SubprocessTextPlugin final {
public:
    static std::pair<ast::TreePtr, std::vector<std::shared_ptr<core::File>>> run(core::Context ctx, ast::TreePtr tree);

    SubprocessTextPlugin() = delete;
};

} // namespace sorbet::plugin

#endif // SORBET_PLUGIN_SUBPROCESS_TEXT_H
