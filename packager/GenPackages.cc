#include "packager/GenPackages.h"

using namespace std;

namespace sorbet::packager {

void GenPackages::run(core::GlobalState &gs) {
    Timer timeit(gs.tracer(), "gen_packages.run");
}

} // namespace sorbet::packager
