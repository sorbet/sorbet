#include "core/ArityHash.h"

using namespace std;

namespace sorbet::core {

ArityHash::ArityHash(uint32_t _hashValue) {
    if (_hashValue == UNDEFINED) {
        this->_hashValue = UNDEFINED_COLLISION_AVOID;
    } else {
        this->_hashValue = _hashValue;
    }
}

} // namespace sorbet::core
