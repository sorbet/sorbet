#include "cfg/LocalRef.h"
#include "cfg/CFG.h"

using namespace std;

namespace sorbet::cfg {
core::LocalVariable LocalRef::data(const CFG &cfg) const {
    ENFORCE(cfg.localVariables.size() > this->_id);
    // Note: It's OK to call `.data()` of a variable that doesn't exist; it returns `LocalVariable::noVariable`.
    return cfg.localVariables[this->_id];
}

int LocalRef::minLoops(const CFG &cfg) const {
    ENFORCE(cfg.minLoops.size() > this->_id);
    ENFORCE(this->exists());
    return cfg.minLoops[this->_id];
}

int LocalRef::maxLoopWrite(const CFG &cfg) const {
    ENFORCE(cfg.maxLoopWrite.size() > this->_id);
    ENFORCE(this->exists());
    return cfg.maxLoopWrite[this->_id];
}

bool LocalRef::exists() const {
    return this->_id > 0;
}

bool LocalRef::isAliasForGlobal(const core::GlobalState &gs, const CFG &cfg) const {
    return this->data(cfg).isAliasForGlobal(gs);
}

bool LocalRef::isSyntheticTemporary(const CFG &cfg) const {
    return this->data(cfg).isSyntheticTemporary();
}

string LocalRef::toString(const core::GlobalState &gs, const CFG &cfg) const {
    return this->data(cfg).toString(gs);
}

string LocalRef::showRaw(const core::GlobalState &gs, const CFG &cfg) const {
    return this->data(cfg).showRaw(gs);
}

bool LocalRef::operator==(const LocalRef &rhs) const {
    return this->_id == rhs._id;
}

bool LocalRef::operator!=(const LocalRef &rhs) const {
    return this->_id != rhs._id;
}

// Note: The special ID values below are validated in CFG.cc in the CFG constructor.

LocalRef LocalRef::noVariable() {
    return LocalRef(0);
}

LocalRef LocalRef::blockCall() {
    return LocalRef(1);
}

LocalRef LocalRef::selfVariable() {
    return LocalRef(2);
}

LocalRef LocalRef::unconditional() {
    return LocalRef(3);
}

LocalRef LocalRef::finalReturn() {
    return LocalRef(4);
}
} // namespace sorbet::cfg
