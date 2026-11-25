#include "cfg/LinkRef.h"
#include "cfg/CFG.h"

using namespace std;

namespace sorbet::cfg {

shared_ptr<core::SendAndBlockLink> &LinkRef::data(CFG &cfg) {
    ENFORCE(_id < cfg.links.size());
    return cfg.links[_id];
}

const shared_ptr<core::SendAndBlockLink> &LinkRef::data(const CFG &cfg) const {
    ENFORCE(_id < cfg.links.size());
    return cfg.links[_id];
}

} // namespace sorbet::cfg
