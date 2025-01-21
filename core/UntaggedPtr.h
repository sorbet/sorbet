#ifndef SORBET_UNTAGGEDPTR_H
#define SORBET_UNTAGGEDPTR_H

namespace sorbet::core {

template <class To> struct UntaggedPtr {
private:
    To *value;
    bool isValid;

public:
    explicit UntaggedPtr(To *p, bool valid) noexcept : value(p), isValid(valid) {}

    explicit operator bool() const noexcept {
        return isValid;
    }

    To *get() const noexcept {
        return value;
    }

    To *operator->() const noexcept {
        return value;
    }

    operator To *() const noexcept {
        return value;
    }

    bool operator==(std::nullptr_t) const noexcept {
        return value == nullptr;
    }

    bool operator!=(std::nullptr_t) const noexcept {
        return value != nullptr;
    }
};

} // namespace sorbet::core

#endif // SORBET_UNTAGGEDPTR_H
