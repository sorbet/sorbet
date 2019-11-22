#include "compiler/Names/Names.h"
#include "core/GlobalState.h"

namespace sorbet::compiler {

core::NameRef Names::defineTopClassOrModule(const core::GlobalState &gs) {
    auto ret = gs.lookupNameUTF8("<sorbet_defineTopClassOrModule>");
    ENFORCE(ret.exists(), "Did you forget to call Names::init");
    return ret;
}
core::NameRef Names::defineMethod(const core::GlobalState &gs) {
    auto ret = gs.lookupNameUTF8("<sorbet_defineMethod>");
    ENFORCE(ret.exists(), "Did you forget to call Names::init");
    return ret;
}
core::NameRef Names::defineMethodSingleton(const core::GlobalState &gs) {
    auto ret = gs.lookupNameUTF8("<sorbet_defineMethodSingleton>");
    ENFORCE(ret.exists(), "Did you forget to call Names::init");
    return ret;
}

void Names::init(core::GlobalState &gs) {
    gs.enterNameUTF8("<sorbet_defineTopClassOrModule>");
    gs.enterNameUTF8("<sorbet_defineMethod>");
    gs.enterNameUTF8("<sorbet_defineMethodSingleton>");
}

} // namespace sorbet::compiler
