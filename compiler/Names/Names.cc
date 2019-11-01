#include "compiler/Names/Names.h"
#include "core/GlobalState.h"

namespace sorbet::compiler {

core::NameRef Names::sorbet_defineTopClassOrModule(const core::GlobalState &gs) {
    auto ret = gs.lookupNameUTF8("<sorbet_defineTopClassOrModule>");
    ENFORCE(ret.exists(), "Did you forget to call Names::init");
    return ret;
}
core::NameRef Names::sorbet_defineMethod(const core::GlobalState &gs) {
    auto ret = gs.lookupNameUTF8("<sorbet_defineMethod>");
    ENFORCE(ret.exists(), "Did you forget to call Names::init");
    return ret;
}
core::NameRef Names::sorbet_defineMethodSingleton(const core::GlobalState &gs) {
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
