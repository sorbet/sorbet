#include "compiler/Names/Names.h"
#include "core/GlobalState.h"

namespace sorbet::compiler {

core::NameRef Names::defineTopClassOrModule(const core::GlobalState &gs) {
    auto ret = gs.lookupNameUTF8("<defineTopClassOrModule>");
    ENFORCE(ret.exists(), "Did you forget to call Names::init");
    return ret;
}
core::NameRef Names::defineMethod(const core::GlobalState &gs) {
    auto ret = gs.lookupNameUTF8("<defineMethod>");
    ENFORCE(ret.exists(), "Did you forget to call Names::init");
    return ret;
}
core::NameRef Names::defineMethodSingleton(const core::GlobalState &gs) {
    auto ret = gs.lookupNameUTF8("<defineMethodSingleton>");
    ENFORCE(ret.exists(), "Did you forget to call Names::init");
    return ret;
}
core::NameRef Names::returnValue(const core::GlobalState &gs) {
    auto ret = gs.lookupNameUTF8("<returnValue>");
    ENFORCE(ret.exists(), "Did you forget to call Names::init");
    return ret;
}

void Names::init(core::GlobalState &gs) {
    gs.enterNameUTF8("<defineTopClassOrModule>");
    gs.enterNameUTF8("<defineMethod>");
    gs.enterNameUTF8("<defineMethodSingleton>");
    gs.enterNameUTF8("<returnValue>");
}

} // namespace sorbet::compiler
