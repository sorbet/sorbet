#include "core/LocalVariable.h"
#include "core/Names.h"

template class std::vector<sorbet::core::LocalVariable>;

using namespace std;

namespace sorbet::core {
bool LocalVariable::exists() const {
    return _name._id > 0;
}

bool LocalVariable::isSyntheticTemporary(const GlobalState &gs) const {
    if (_name.data(gs)->kind == NameKind::UNIQUE) {
        return true;
    }
    if (unique == 0) {
        return false;
    }
    return _name == Names::whileTemp() || _name == Names::ifTemp() || _name == Names::returnTemp() ||
           _name == Names::statTemp() || _name == Names::assignTemp() || _name == Names::returnMethodTemp() ||
           _name == Names::blockReturnTemp() || _name == Names::nextTemp() || _name == Names::selfMethodTemp() ||
           _name == Names::hashTemp() || _name == Names::arrayTemp() || _name == Names::rescueTemp() ||
           _name == Names::rescueStartTemp() || _name == Names::rescueEndTemp() || _name == Names::gotoDeadTemp() ||
           _name == Names::isaCheckTemp() || _name == Names::throwAwayTemp() || _name == Names::castTemp() ||
           _name == Names::finalReturn();
}

bool LocalVariable::isAliasForGlobal(const GlobalState &gs) const {
    return _name == Names::cfgAlias();
}

bool LocalVariable::operator==(const LocalVariable &rhs) const {
    return this->_name == rhs._name && this->unique == rhs.unique;
}

bool LocalVariable::operator!=(const LocalVariable &rhs) const {
    return !this->operator==(rhs);
}

string LocalVariable::toString(const GlobalState &gs) const {
    if (unique == 0) {
        return this->_name.toString(gs);
    }
    return this->_name.toString(gs) + "$" + to_string(this->unique);
}

} // namespace sorbet::core
