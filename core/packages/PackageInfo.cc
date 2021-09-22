#include "core/packages/PackageInfo.h"

namespace sorbet::core::packages {
PackageInfo::~PackageInfo() {
    // see https://eli.thegreenplace.net/2010/11/13/pure-virtual-destructors-in-c
}
} // namespace sorbet::core::packages
