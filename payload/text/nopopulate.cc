#include "payload/text/text.h"
using namespace std;

namespace sorbet::rbi {
void populateRBIsInto(core::GlobalState &gs) {
    Exception::raise("Should never call populateRBIsInto with nopopulate.cc");
}

} // namespace sorbet::rbi
