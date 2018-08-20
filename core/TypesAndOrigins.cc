#include "Types.h"
#include "absl/algorithm/container.h"

using namespace std;
namespace sorbet {
namespace core {

// This sorts the underlying `origins`
vector<ErrorLine> TypeAndOrigins::origins2Explanations(Context ctx) {
    vector<ErrorLine> result;
    auto compare = [](Loc &left, Loc &right) {
        if (left.file != right.file) {
            return left.file.id() < right.file.id();
        }
        if (left.beginPos != right.beginPos) {
            return left.beginPos < right.beginPos;
        }
        if (left.endPos != right.endPos) {
            return left.endPos < right.endPos;
        }
        return false;
    };
    absl::c_sort(origins, compare);
    Loc last;
    for (auto o : origins) {
        if (o == last) {
            continue;
        }
        last = o;
        result.emplace_back(o, "");
    }
    return result;
}

} // namespace core
} // namespace sorbet
