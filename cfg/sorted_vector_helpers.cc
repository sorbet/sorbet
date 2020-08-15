#include "cfg/sorted_vector_helpers.h"

using namespace std;

namespace sorbet::cfg {
void SortedVectorHelpers::setDifferenceInplace(vector<int> &data, const vector<int> &toRemove) {
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

void SortedVectorHelpers::setUnionInplace(vector<int> &into, const vector<int> &from) {
    // A decent proportion of unions are just copies.
    if (into.empty()) {
        into = from;
        return;
    }

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
            // Rather than insert in the middle of the vector, break out of loop and do a set union.
            vector<int> merged(into.begin(), intoIt);
            merged.reserve(max(into.size(), from.size()));
            set_union(intoIt, into.end(), fromIt, from.end(), back_inserter(merged));
            into = move(merged);
            return;
        }
    }
    while (fromIt != from.end()) {
        // These are all missing from into and are > than anything in into.
        into.emplace_back(*fromIt);
        fromIt++;
    }
}
} // namespace sorbet::cfg
