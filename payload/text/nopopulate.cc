#include "payload/text/text.h"
using namespace std;

namespace sorbet::rbi {
void populateRBIsInto(unique_ptr<core::GlobalState> &gs) {
    Exception::raise("Should never call populateRBIsInto with nopopulate.cc");
}

} // namespace sorbet::rbi
