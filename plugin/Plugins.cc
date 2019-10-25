#include "plugin/Plugins.h"

#include "common/sort.h"
using namespace std;

namespace sorbet::plugin {

void Plugins::dumpPluginGeneratedFiles(const core::GlobalState &gs, const realmain::options::PrinterConfig &pc) {
    vector<core::FileRef> refs;
    for (int i = 1; i < gs.filesUsed(); i++) {
        if (core::FileRef(i).data(gs).pluginGenerated) {
            refs.emplace_back(i);
        }
    }

    fast_sort(refs,
              [&gs](const core::FileRef &a, const core::FileRef &b) { return a.data(gs).path() < b.data(gs).path(); });

    for (auto ref : refs) {
        const core::File &file = ref.data(gs);
        pc.fmt("# Path: \"{}\":\n", file.path());
        pc.fmt("{}\n", file.source());
    }
}

} // namespace sorbet::plugin
