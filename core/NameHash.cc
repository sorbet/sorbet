#include "core/NameHash.h"
#include "core/GlobalState.h"
#include "core/Hashing.h"
#include "core/Names.h"
using namespace std;
namespace sorbet::core {
u4 incZero(u4 a) {
    return a == 0 ? 1 : a;
};
NameHash::NameHash(const GlobalState &gs, const NameData &nm) : _hashValue(incZero(_hash(nm->shortName(gs)))){};
NameHash::NameHash(const GlobalState &gs, const Name &nm) : _hashValue(incZero(_hash(nm.shortName(gs)))){};

} // namespace sorbet::core
