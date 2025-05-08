#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "absl/algorithm/container.h"
#include "common/sort/sort.h"
#include "main/autogen/data/msgpack.h"

using namespace std;
namespace sorbet::autogen {

void validateAttrMap(const std::map<int, vector<string>> &hugeMap, const std::map<int, vector<string>> &liteMap) {
    vector<int> hugeKeys;
    vector<int> liteKeys;

    absl::c_transform(hugeMap, std::back_inserter(hugeKeys), [](const auto &e) { return e.first; });
    absl::c_transform(liteMap, std::back_inserter(liteKeys), [](const auto &e) { return e.first; });

    fast_sort(hugeKeys);
    fast_sort(liteKeys);

    vector<int> keyDiff;
    absl::c_set_symmetric_difference(hugeKeys, liteKeys, std::back_inserter(keyDiff));
    CHECK(keyDiff.empty());

    for (auto &version : hugeKeys) {
        auto hugeAttrsIt = hugeMap.find(version);
        auto liteAttrsIt = liteMap.find(version);
        REQUIRE_NE(hugeAttrsIt, hugeMap.end());
        REQUIRE_NE(liteAttrsIt, liteMap.end());

        auto &hugeAttrs = hugeAttrsIt->second;
        auto &liteAttrs = liteAttrsIt->second;

        // The lite attributes should occur in the same order as those in the
        // regular version.
        vector<vector<string>::const_iterator> hugeAttrsPtrs;
        absl::c_transform(liteAttrs, std::back_inserter(hugeAttrsPtrs), [&hugeAttrs](const auto &e) {
            auto it = absl::c_find(hugeAttrs, e);
            CHECK_NE(it, hugeAttrs.end());
            return it;
        });

        CHECK(absl::c_is_sorted(hugeAttrsPtrs));
    }
}

TEST_SUITE("Autogen") {
    TEST_CASE("msgpack defs") {
        validateAttrMap(MsgpackWriterFull::defAttrMap, MsgpackWriterLite::defAttrMap);
    }

    TEST_CASE("msgpack refs") {
        validateAttrMap(MsgpackWriterFull::refAttrMap, MsgpackWriterLite::refAttrMap);
    }

    TEST_CASE("msgpack parsed files") {
        validateAttrMap(MsgpackWriterFull::parsedFileAttrMap, MsgpackWriterLite::parsedFileAttrMap);
    }
}

} // namespace sorbet::autogen
