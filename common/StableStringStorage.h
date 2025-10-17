#ifndef SORBET_STABLE_STRING_STORAGE_H
#define SORBET_STABLE_STRING_STORAGE_H

#include <string>
#include <utility>
#include <vector>

namespace sorbet {

template <size_t PageSize = 4096> class StableStringStorage {
    std::shared_ptr<char> &addPage(size_t size) {
        auto page = std::unique_ptr<char[]>(new char[size]);
        return strings.emplace_back(page.release(), page.get_deleter());
    }

public:
    StableStringStorage() = default;

    // We override the copy constructor and assignment because we need to ensure
    // that `currentPagePosition` in the copy forces a separate page to be allocated.
    // The pages may be shared between `StableStringStorage` instances, but only
    // one instance is ever permitted to write to a single page.
    StableStringStorage(const StableStringStorage &rhs);
    StableStringStorage &operator=(const StableStringStorage &rhs);

    bool empty() const {
        return strings.empty();
    }
    std::string_view enterString(std::string_view str);

private:
    std::vector<std::shared_ptr<char>> strings;
    size_t currentPagePosition = PageSize + 1;
};

template <size_t PageSize>
StableStringStorage<PageSize>::StableStringStorage(const StableStringStorage<PageSize> &rhs)
    : strings(rhs.string), currentPagePosition(PageSize + 1) {}

template <size_t PageSize>
StableStringStorage<PageSize> &StableStringStorage<PageSize>::operator=(const StableStringStorage<PageSize> &rhs) {
    this->strings = rhs.strings;
    this->currentPagePosition = PageSize + 1;
    return *this;
}

template <size_t PageSize> std::string_view StableStringStorage<PageSize>::enterString(std::string_view str) {
    char *from = nullptr;
    if (str.size() > PageSize) {
        auto &inserted = addPage(str.size());
        from = inserted.get();
        if (strings.size() > 1) {
            // last page wasn't full, keep it in the end
            swap(*(strings.end() - 1), *(strings.end() - 2));

            // NOTE: we do not update `currentPagePosition` here because it refers to the offset into the last page,
            // which is swapped in by the line above.
        } else {
            // Insert a new empty page at the end to enforce the invariant that inserting a huge string will always
            // leave a page that can be written to at the end of the table.
            addPage(PageSize);
            currentPagePosition = 0;
        }
    } else {
        if (currentPagePosition + str.size() > PageSize) {
            addPage(PageSize);
            currentPagePosition = 0;
        }
        from = strings.back().get() + currentPagePosition;
        currentPagePosition += str.size();
    }

    memcpy(from, str.data(), str.size());
    return std::string_view(from, str.size());
}

} // namespace sorbet

#endif // SORBET_STABLE_STRING_STORAGE_H
