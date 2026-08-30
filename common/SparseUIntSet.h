#ifndef SORBET_COMMON_SPARSE_UINTSET_H
#define SORBET_COMMON_SPARSE_UINTSET_H

#include "common/common.h"

namespace sorbet {

// A set of small integers with the interface of `UIntSet`, storing only the 32-bit words of the bitset that are not
// zero, in order of their index. The CFG builder's liveness passes keep several sets per basic block over all the
// locals of a method; in a method with thousands of locals and blocks (a test file, whose `it` blocks are all inlined
// into one method) each block mentions a handful of locals, so the bitsets are large and almost entirely zero, and
// walking them dominated building the CFG. Set operations here cost in proportion to the words present in either set
// rather than to the universe. For dense sets a bitset is the better representation; the caller picks.
class SparseUIntSet final {
    struct Word {
        uint32_t index;
        uint32_t bits;
    };
    // Sorted by `index`; no `bits` is zero.
    InlinedVector<Word, 4> words;

    // The position of the word with `index`, or the position to insert it at.
    size_t positionOf(uint32_t index) const {
        size_t lo = 0;
        size_t hi = words.size();
        while (lo < hi) {
            auto mid = (lo + hi) / 2;
            if (words[mid].index < index) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return lo;
    }

    // Combines `other` into this set word by word, keeping the words of this set (in place): `combine(ours, theirs)`
    // gives the new bits for a word of this set, with `theirs` zero when `other` lacks the word. Zero results are
    // dropped. For unions, which may add words, see `add`.
    template <class Combine> void combineInPlace(const SparseUIntSet &other, Combine combine) {
        size_t write = 0;
        size_t j = 0;
        for (size_t i = 0; i < words.size(); i++) {
            while (j < other.words.size() && other.words[j].index < words[i].index) {
                j++;
            }
            uint32_t theirs =
                (j < other.words.size() && other.words[j].index == words[i].index) ? other.words[j].bits : 0;
            auto bits = combine(words[i].bits, theirs);
            if (bits != 0) {
                words[write++] = Word{words[i].index, bits};
            }
        }
        words.resize(write);
    }

public:
    // `capacity` (the largest item plus one) is what `UIntSet` takes; a sparse set needs no sizing.
    explicit SparseUIntSet(uint32_t capacity) {}

    void clear() {
        words.clear();
    }

    void add(uint32_t item) {
        uint32_t index = item >> 5;
        uint32_t mask = 1u << (item & 0x1F);
        auto pos = positionOf(index);
        if (pos < words.size() && words[pos].index == index) {
            words[pos].bits |= mask;
        } else {
            words.insert(words.begin() + pos, Word{index, mask});
        }
    }

    void remove(uint32_t item) {
        uint32_t index = item >> 5;
        uint32_t mask = 1u << (item & 0x1F);
        auto pos = positionOf(index);
        if (pos < words.size() && words[pos].index == index) {
            words[pos].bits &= ~mask;
            if (words[pos].bits == 0) {
                words.erase(words.begin() + pos);
            }
        }
    }

    bool contains(uint32_t item) const {
        uint32_t index = item >> 5;
        auto pos = positionOf(index);
        return pos < words.size() && words[pos].index == index && (words[pos].bits & (1u << (item & 0x1F))) != 0;
    }

    // Calls `each` with every item, in ascending order.
    template <typename F> void forEach(F each) const {
        for (auto word : words) {
            uint32_t base = word.index * 32;
            auto bits = word.bits;
            while (bits != 0) {
                uint32_t bit = __builtin_ctz(bits);
                each(base + bit);
                bits &= bits - 1;
            }
        }
    }

    // Adds the items of `set`.
    void add(const SparseUIntSet &set) {
        if (set.words.empty()) {
            return;
        }
        // Words of `set` that this set lacks have to be inserted; when there are none (the usual case once a fixed
        // point iteration has settled) the union is in place.
        size_t missing = 0;
        {
            size_t i = 0;
            for (auto &word : set.words) {
                while (i < words.size() && words[i].index < word.index) {
                    i++;
                }
                if (i == words.size() || words[i].index != word.index) {
                    missing++;
                }
            }
        }
        if (missing == 0) {
            size_t j = 0;
            for (auto &word : words) {
                while (j < set.words.size() && set.words[j].index < word.index) {
                    j++;
                }
                if (j < set.words.size() && set.words[j].index == word.index) {
                    word.bits |= set.words[j].bits;
                }
            }
            return;
        }
        InlinedVector<Word, 4> result;
        result.reserve(words.size() + missing);
        size_t i = 0;
        size_t j = 0;
        while (i < words.size() || j < set.words.size()) {
            if (j == set.words.size() || (i < words.size() && words[i].index < set.words[j].index)) {
                result.push_back(words[i++]);
            } else if (i == words.size() || set.words[j].index < words[i].index) {
                result.push_back(set.words[j++]);
            } else {
                result.push_back(Word{words[i].index, words[i].bits | set.words[j].bits});
                i++;
                j++;
            }
        }
        words = std::move(result);
    }

    // Adds the items of `a` and `b`.
    void add(const SparseUIntSet &a, const SparseUIntSet &b) {
        add(a);
        add(b);
    }

    // Removes the items of `set`.
    void remove(const SparseUIntSet &set) {
        if (set.words.empty() || words.empty()) {
            return;
        }
        combineInPlace(set, [](uint32_t ours, uint32_t theirs) { return ours & ~theirs; });
    }

    // Keeps only the items also in `set`.
    void intersect(const SparseUIntSet &set) {
        if (set.words.empty()) {
            words.clear();
            return;
        }
        combineInPlace(set, [](uint32_t ours, uint32_t theirs) { return ours & theirs; });
    }

    // Replaces the contents with the union of `a` and `b`.
    void overwriteWithUnion(const SparseUIntSet &a, const SparseUIntSet &b) {
        words = a.words;
        add(b);
    }

    // Calls `each(index, bits)` for every word present, in index order.
    template <typename F> void forEachWord(F each) const {
        for (auto word : words) {
            each(word.index, word.bits);
        }
    }

    // Keeps only the items also in the set whose word at `index` is `wordAt(index)` (a dense set).
    template <typename WordAt> void intersectWords(WordAt wordAt) {
        size_t write = 0;
        for (auto word : words) {
            auto bits = word.bits & wordAt(word.index);
            if (bits != 0) {
                words[write++] = Word{word.index, bits};
            }
        }
        words.resize(write);
    }

    bool empty() const {
        return words.empty();
    }

    size_t size() const {
        size_t count = 0;
        for (auto word : words) {
            count += __builtin_popcount(word.bits);
        }
        return count;
    }
};

} // namespace sorbet

#endif
