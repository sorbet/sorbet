#include "core/packages/PackageInfo.h"
#include "core/Loc.h"
#include "core/NameRef.h"

using namespace std;

namespace sorbet::core::packages {
bool PackageInfo::exists() const {
    return mangledName().exists();
}

PackageInfo::~PackageInfo() {
    // see https://eli.thegreenplace.net/2010/11/13/pure-virtual-destructors-in-c
}
} // namespace sorbet::core::packages
