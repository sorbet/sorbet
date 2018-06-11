#include "Types.h"

#include <algorithm> // std::sort

namespace sorbet {
namespace core {

// This sorts the underlying `origins`
std::vector<ErrorLine> TypeAndOrigins::origins2Explanations(core::Context ctx) {
    std::vector<ErrorLine> result;
    auto compare = [](Loc &left, Loc &right) {
        if (left.file != right.file) {
            return left.file.id() < right.file.id();
        }
        if (left.begin_pos != right.begin_pos) {
            return left.begin_pos < right.begin_pos;
        }
        if (left.end_pos != right.end_pos) {
            return left.end_pos < right.end_pos;
        }
        return false;
    };
    std::sort(origins.begin(), origins.end(), compare);
    core::Loc last;
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
