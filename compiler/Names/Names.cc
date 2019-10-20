#include "compiler/Names/Names.h"
#include "core/GlobalState.h"

namespace sorbet::compiler {

void Names::init(core::GlobalState &gs) {
    Names::sorbet_defineTopLevelModule = gs.enterNameUTF8("<sorbet_defineTopLevelModule>");
    Names::sorbet_defineNestedModule = gs.enterNameUTF8("<sorbet_defineNestedModule>");
    Names::sorbet_defineTopClassOrModule = gs.enterNameUTF8("<sorbet_defineTopClassOrModule>");
    Names::sorbet_defineNestedClass = gs.enterNameUTF8("<sorbet_defineNestedClass>");
    Names::sorbet_defineMethod = gs.enterNameUTF8("<sorbet_defineMethod>");
    Names::sorbet_defineMethodSingleton = gs.enterNameUTF8("<sorbet_defineMethodSingleton>");
}

core::NameRef Names::sorbet_defineTopLevelModule;
core::NameRef Names::sorbet_defineNestedModule;
core::NameRef Names::sorbet_defineTopClassOrModule;
core::NameRef Names::sorbet_defineNestedClass;
core::NameRef Names::sorbet_defineMethod;
core::NameRef Names::sorbet_defineMethodSingleton;

} // namespace sorbet::compiler
