#ifndef RUBY_PARSER_POOL_HH
#define RUBY_PARSER_POOL_HH

#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>

template <typename T, std::size_t N> class pool {
public:
    pool() : _slab(new (slab)) {}

    template <typename... Args> T *alloc(Args &&...args) {
        if (_slab->is_full()) {
            push_slab();
        }
        return _slab->alloc(std::forward<Args>(args)...);
    }

    ~pool() {
        delete _slab;
        for (auto &p : _history) {
            delete p;
        }
    }

protected:
    class slab {
        alignas(T) unsigned char data[sizeof(T) * N];
        std::size_t _size = 0;

        inline T *ptr_at(size_t i) {
            return reinterpret_cast<T *>(data + sizeof(T) * i);
        }

    public:
        inline bool is_full() const {
            return _size >= N;
        }

        template <typename... Args> T *alloc(Args &&...args) {
            assert(!is_full());
            auto *p = ptr_at(_size);
            new (p) T(std::forward<Args>(args)...);
            ++_size;
            return p;
        }

        ~slab() {
            for (std::size_t pos = 0; pos < _size; ++pos) {
                ptr_at(pos)->~T();
            }
        }
    };

    using slab_t = slab *;

    std::vector<slab *> _history;
    slab *_slab;

    void push_slab() {
        slab *newb = new (slab);
        _history.emplace_back(_slab);
        _slab = newb;
    }
};

#endif
