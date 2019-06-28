#ifndef SORBET_PLUGIN_SUBPROCESS_TEXT_H
#define SORBET_PLUGIN_SUBPROCESS_TEXT_H
#include "ast/ast.h"

namespace sorbet::plugin {

class SubprocessTextPlugin final {
public:
    static std::pair<std::unique_ptr<ast::Expression>, std::vector<std::shared_ptr<core::File>>>
    run(core::Context ctx, std::unique_ptr<ast::Expression> tree);
    // Plugin generated files are not stored on disk and have special paths
    static std::string pluginFilePath(std::string_view sourceFilePath, u4 invocationId);

    SubprocessTextPlugin() = delete;
};

} // namespace sorbet::plugin

#endif // SORBET_PLUGIN_SUBPROCESS_TEXT_H
