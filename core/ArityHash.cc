#include "core/ArityHash.h"

using namespace std;

namespace sorbet::core {

ArityHash::ArityHash(uint32_t _hashValue) {
    if (_hashValue == UNDEFINED) {
        this->_hashValue = UNDEFINED_COLLISION_AVOID;
    } else if (_hashValue == ALIAS_METHOD) {
        this->_hashValue = ALIAS_METHOD_COLLISION_AVOID;
    } else {
        this->_hashValue = _hashValue;
    }
}

ArityHash ArityHash::aliasMethodHash() {
    ArityHash result;
    result._hashValue = ALIAS_METHOD;
    return result;
}

} // namespace sorbet::core
