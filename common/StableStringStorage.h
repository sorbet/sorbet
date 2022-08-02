#ifndef SORBET_STABLE_STRING_STORAGE_H
#define SORBET_STABLE_STRING_STORAGE_H

#include <string>
#include <utility>
#include <vector>

namespace sorbet {

// Allocator adaptor that interposes construct() calls to
// convert value initialization into default initialization.
template <typename T, typename A = std::allocator<T>> class default_init_allocator : public A {
    typedef std::allocator_traits<A> a_t;

public:
    template <typename U> struct rebind {
        using other = default_init_allocator<U, typename a_t::template rebind_alloc<U>>;
    };

    using A::A;

    template <typename U> void construct(U *ptr) noexcept(std::is_nothrow_default_constructible<U>::value) {
        ::new (static_cast<void *>(ptr)) U;
    }
    template <typename U, typename... Args> void construct(U *ptr, Args &&...args) {
        a_t::construct(static_cast<A &>(*this), ptr, std::forward<Args>(args)...);
    }
};

template <size_t PageSize = 4096> class StableStringStorage {
    using Buffer = std::vector<char, default_init_allocator<char>>;
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
    std::vector<std::shared_ptr<Buffer>> strings;
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
        auto &inserted = strings.emplace_back(std::make_unique<Buffer>(str.size()));
        from = inserted->data();
        if (strings.size() > 1) {
            // last page wasn't full, keep it in the end
            swap(*(strings.end() - 1), *(strings.end() - 2));

            // NOTE: we do not update `currentPagePosition` here because it refers to the offset into the last page,
            // which is swapped in by the line above.
        } else {
            // Insert a new empty page at the end to enforce the invariant that inserting a huge string will always
            // leave a page that can be written to at the end of the table.
            strings.emplace_back(std::make_unique<Buffer>(PageSize));
            currentPagePosition = 0;
        }
    } else {
        if (currentPagePosition + str.size() > PageSize) {
            strings.emplace_back(std::make_unique<Buffer>(PageSize));
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
