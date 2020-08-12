#include "cfg/sorted_vector_helpers.h"

using namespace std;

namespace sorbet::cfg {
void removeFrom(vector<int> &data, const vector<int> &toRemove) {
    auto dataIt = data.begin();
    auto removeIt = toRemove.begin();
    while (dataIt != data.end() && removeIt != toRemove.end()) {
        const int datum = *dataIt;
        const int remove = *removeIt;
        if (datum == remove) {
            dataIt = data.erase(dataIt);
        } else if (datum < remove) {
            dataIt++;
        } else {
            removeIt++;
        }
    }
}

void setMerge(vector<int> &into, const vector<int> &from) {
    auto intoIt = into.begin();
    auto fromIt = from.begin();

    while (intoIt != into.end() && fromIt != from.end()) {
        const auto intoEl = *intoIt;
        const auto fromEl = *fromIt;
        if (intoEl == fromEl) {
            // Present in both sets.
            fromIt++;
            intoIt++;
        } else if (intoEl < fromEl) {
            // Present in into, not in from
            intoIt++;
        } else {
            // intoEl > fromEl
            // Present in from, not in into.
            // Insert at intoIt just before intoEl
            intoIt = into.insert(intoIt, fromEl);
            // intoIt now points to an entry containing fromEl; we can skip past it.
            intoIt++;
            fromIt++;
        }
    }
    while (fromIt != from.end()) {
        // These are all missing from into and are > than anything in into.
        into.emplace_back(*fromIt);
        fromIt++;
    }
}
} // namespace sorbet::cfg