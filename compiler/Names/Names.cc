#include "compiler/Names/Names.h"
#include "core/GlobalState.h"

namespace sorbet::compiler {

void Names::init(core::GlobalState &gs) {
    Names::sorbet_defineTopClassOrModule = gs.enterNameUTF8("<sorbet_defineTopClassOrModule>");
    Names::sorbet_defineMethod = gs.enterNameUTF8("<sorbet_defineMethod>");
    Names::sorbet_defineMethodSingleton = gs.enterNameUTF8("<sorbet_defineMethodSingleton>");
}

core::NameRef Names::sorbet_defineTopClassOrModule;
core::NameRef Names::sorbet_defineMethod;
core::NameRef Names::sorbet_defineMethodSingleton;

} // namespace sorbet::compiler
