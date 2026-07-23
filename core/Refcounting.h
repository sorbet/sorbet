#ifndef SORBET_REFCOUNTING_H
#define SORBET_REFCOUNTING_H

#include <atomic>
#include <cstdint>
#include <utility>

namespace sorbet::core {

class Refcountable {
    std::atomic<uint32_t> counter{0};

public:
    void addref() {
        this->counter.fetch_add(1);
    }

    uint32_t release() {
        // fetch_sub returns value prior to subtract
        return this->counter.fetch_sub(1) - 1;
    }

    // You typically should not need to call this; it is mostly for tests to
    // verify that things manipulating `Refcountable` are getting the counting
    // logic correct.
    bool hasMultipleRefs() const {
        return this->counter.load() > 1;
    }
};

// CRTP base for classes that want to be used with RefPtr<T>.
template <typename T> class RefCounted : public Refcountable {
public:
    void addref() {
        Refcountable::addref();
    }

    void release() {
        uint32_t remaining = Refcountable::release();
        if (remaining == 0) {
            delete static_cast<T *>(this);
        }
    }

protected:
    ~RefCounted() = default;
};

// A reference-counting smart pointer.
//
// T must inherit from RefCounted<T>.  Instances of T should only be created with
// `MakeRefPtr`.
template <typename T> class RefPtr {
    T *ptr_ = nullptr;

    void assign(T *p) {
        if (p) {
            p->addref();
        }
        T *old = ptr_;
        ptr_ = p;
        if (old) {
            old->release();
        }
    }

public:
    RefPtr() = default;

    explicit RefPtr(T *p) : ptr_(p) {
        if (ptr_) {
            ptr_->addref();
        }
    }

    RefPtr(const RefPtr &other) : ptr_(other.ptr_) {
        if (ptr_) {
            ptr_->addref();
        }
    }

    RefPtr(RefPtr &&other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    ~RefPtr() {
        if (ptr_) {
            ptr_->release();
        }
    }

    RefPtr &operator=(const RefPtr &other) {
        if (ptr_ != other.ptr_) {
            assign(other.ptr_);
        }
        return *this;
    }

    RefPtr &operator=(RefPtr &&other) noexcept {
        if (ptr_ != other.ptr_) {
            if (ptr_) {
                ptr_->release();
            }
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    RefPtr &operator=(T *p) {
        assign(p);
        return *this;
    }

    RefPtr &operator=(std::nullptr_t) {
        if (ptr_) {
            ptr_->release();
            ptr_ = nullptr;
        }
        return *this;
    }

    T *get() const noexcept {
        return ptr_;
    }
    T *operator->() const noexcept {
        return ptr_;
    }
    T &operator*() const noexcept {
        return *ptr_;
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    bool operator<=>(const RefPtr &other) const noexcept = default;
    bool operator==(std::nullptr_t) const noexcept {
        return ptr_ == nullptr;
    }
    bool operator!=(std::nullptr_t) const noexcept {
        return ptr_ != nullptr;
    }
};

template <typename T, typename... Args> RefPtr<T> MakeRefPtr(Args &&...args) {
    return RefPtr<T>(new T(std::forward<Args>(args)...));
}

} // namespace sorbet::core

#endif
