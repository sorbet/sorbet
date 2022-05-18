#include "core/FileHash.h"
#include "common/sort.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/hashing/hashing.h"

using namespace std;
namespace sorbet::core {
uint32_t incZero(uint32_t a) {
    return a == 0 ? 1 : a;
};
NameHash::NameHash(const GlobalState &gs, NameRef nm) : _hashValue(incZero(_hash(nm.shortName(gs)))){};

void NameHash::sortAndDedupe(std::vector<core::NameHash> &hashes) {
    fast_sort(hashes);
    hashes.resize(std::distance(hashes.begin(), std::unique(hashes.begin(), hashes.end())));
}

FileHash::FileHash(LocalSymbolTableHashes &&localSymbolTableHashes, UsageHash &&usages)
    : localSymbolTableHashes(move(localSymbolTableHashes)), usages(move(usages)) {}

} // namespace sorbet::core
