#include "core/FileHash.h"
#include "common/sort.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/hashing/hashing.h"

using namespace std;
namespace sorbet::core {
namespace {

// We use 0 to signal whether the hash is defined (see isDefined in the header)
uint32_t incZero(uint32_t a) {
    return a == 0 ? 1 : a;
};

uint32_t hashFullNameRef(const GlobalState &gs, NameRef nm) {
    uint32_t result;
    auto kind = nm.kind();

    switch (kind) {
        case NameKind::UTF8: {
            result = _hash(nm.dataUtf8(gs)->utf8);
            break;
        }
        case NameKind::CONSTANT: {
            result = hashFullNameRef(gs, nm.dataCnst(gs)->original);
            break;
        }
        case NameKind::UNIQUE: {
            auto data = nm.dataUnique(gs);
            auto hashedUniqueNameKind = static_cast<underlying_type<UniqueNameKind>::type>(data->uniqueNameKind);
            result = mix(mix(hashFullNameRef(gs, data->original), data->num), hashedUniqueNameKind);
            break;
        }
    }

    auto hashedKind = static_cast<underlying_type<NameKind>::type>(kind);
    return mix(result, hashedKind);
}

} // namespace

ShortNameHash::ShortNameHash(const GlobalState &gs, NameRef nm) : _hashValue(incZero(_hash(nm.shortName(gs)))){};

void ShortNameHash::sortAndDedupe(std::vector<core::ShortNameHash> &hashes) {
    fast_sort(hashes);
    hashes.resize(std::distance(hashes.begin(), std::unique(hashes.begin(), hashes.end())));
}

FullNameHash::FullNameHash(const GlobalState &gs, NameRef nm) : _hashValue(incZero(hashFullNameRef(gs, nm))) {}

void FullNameHash::sortAndDedupe(std::vector<core::FullNameHash> &hashes) {
    fast_sort(hashes);
    hashes.resize(std::distance(hashes.begin(), std::unique(hashes.begin(), hashes.end())));
}

FileHash::FileHash(LocalSymbolTableHashes &&localSymbolTableHashes, UsageHash &&usages)
    : localSymbolTableHashes(move(localSymbolTableHashes)), usages(move(usages)) {}

} // namespace sorbet::core
