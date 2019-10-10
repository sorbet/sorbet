#include "compiler/Names/Names.h"
#include "core/GlobalState.h"

namespace sorbet::compiler {

void Names::init(core::GlobalState &gs) {
    Names::registerClass = gs.enterNameUTF8("<registerClass>");
    Names::registerMethod = gs.enterNameUTF8("<registerMethod>");
}

core::NameRef Names::registerClass;
core::NameRef Names::registerMethod;

} // namespace sorbet::compiler
