#ifndef SORBET_STABLE_STRING_STORAGE_H
#define SORBET_STABLE_STRING_STORAGE_H

#include <string>
#include <vector>
#include <utility>

namespace sorbet {

template <size_t PageSize = 4096>
class StableStringStorage {
public:
    StableStringStorage() = default;

    bool empty() const { return strings.empty(); }
    std::string_view enterString(std::string_view str);

private:
    std::vector<std::shared_ptr<std::vector<char>>> strings;
    size_t currentPagePosition = PageSize + 1;
};

template <size_t PageSize>
std::string_view StableStringStorage<PageSize>::enterString(std::string_view str) {
    char *from = nullptr;
    if (str.size() > PageSize) {
        auto &inserted = strings.emplace_back(std::make_unique<std::vector<char>>(str.size()));
        from = inserted->data();
        if (strings.size() > 1) {
            // last page wasn't full, keep it in the end
            swap(*(strings.end() - 1), *(strings.end() - 2));

            // NOTE: we do not update `currentPagePosition` here because it refers to the offset into the last page,
            // which is swapped in by the line above.
        } else {
            // Insert a new empty page at the end to enforce the invariant that inserting a huge string will always
            // leave a page that can be written to at the end of the table.
            strings.emplace_back(std::make_unique<std::vector<char>>(PageSize));
            currentPagePosition = 0;
        }
    } else {
        if (currentPagePosition + str.size() > PageSize) {
            strings.emplace_back(std::make_unique<std::vector<char>>(PageSize));
            currentPagePosition = 0;
        }
        from = strings.back()->data() + currentPagePosition;
        currentPagePosition += str.size();
    }

    memcpy(from, str.data(), str.size());
    return std::string_view(from, str.size());
}

} // namespace sorbet

#endif // SORBET_STABLE_STRING_STORAGE_H
