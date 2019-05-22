#ifndef SORBET_PLUGIN_PLUGINS_H
#define SORBET_PLUGIN_PLUGINS_H

#include "common/common.h"
#include "core/GlobalState.h"
#include "main/options/options.h"

namespace sorbet::plugin {

class Plugins final {
public:
    static void dumpPluginGeneratedFiles(const core::GlobalState &gs, const realmain::options::PrinterConfig &pc);
    Plugins() = delete;
};

} // namespace sorbet::plugin

#endif // SORBET_PLUGIN_PLUGINS_H
