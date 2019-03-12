#ifndef SORBET_PLUGIN_PLUGINS_H
#define SORBET_PLUGIN_PLUGINS_H

#include "common/common.h"
#include "core/GlobalState.h"

namespace sorbet::plugin {

class Plugins final {
public:
    static void dumpPluginGeneratedFiles(const core::GlobalState &gs);
    Plugins() = delete;
};

} // namespace sorbet::plugin

#endif // SORBET_PLUGIN_PLUGINS_H
