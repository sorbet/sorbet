#include "Types.h"
#include "absl/algorithm/container.h"

using namespace std;
namespace sorbet {
namespace core {

// This sorts the underlying `origins`
vector<ErrorLine> TypeAndOrigins::origins2Explanations(Context ctx) const {
    vector<ErrorLine> result;
    auto compare = [](Loc left, Loc right) {
        if (left.file() != right.file()) {
            return left.file().id() < right.file().id();
        }
        if (left.beginPos() != right.beginPos()) {
            return left.beginPos() < right.beginPos();
        }
        if (left.endPos() != right.endPos()) {
            return left.endPos() < right.endPos();
        }
        return false;
    };
    auto sortedOrigins = origins;
    absl::c_sort(sortedOrigins, compare);
    Loc last;
    for (auto o : sortedOrigins) {
        if (o == last) {
            continue;
        }
        last = o;
        result.emplace_back(o, "");
    }
    return result;
}

TypeAndOrigins::~TypeAndOrigins() {
    histogramInc("TypeAndOrigins.origins.size", origins.size());
}

} // namespace core
} // namespace sorbet
