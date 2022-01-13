#ifndef RUBY_PARSER_POOL_HH
#define RUBY_PARSER_POOL_HH

#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>

template<typename T, std::size_t N>
class pool {
public:
	pool() : _slab(new(slab)) {}

	template <typename... Args>
	T *alloc(Args&&... args) {
		if (_slab->is_full()) {
			push_slab();
		}
		return _slab->alloc(std::forward<Args>(args)...);
	}

	~pool() {
		delete _slab;
		for (auto &p: _history) {
			delete p;
		}
	}

protected:
	class slab {
		typename std::aligned_storage<sizeof(T), alignof(T)>::type data[N];
		std::size_t _size = 0;

	public:
		inline bool is_full() const {
			return _size >= N;
		}

		template<typename ...Args>
		T *alloc(Args&&... args)
		{
			assert(!is_full());
			T *p = reinterpret_cast<T*>(data+_size);
			new(p) T(std::forward<Args>(args)...);
			++_size;
			return p;
		}

		~slab() {
			for (std::size_t pos = 0; pos < _size; ++pos) {
				reinterpret_cast<T*>(data+pos)->~T();
			}
		}
	};

	using slab_t = slab*;

	std::vector<slab *> _history;
	slab *_slab;

	void push_slab() {
		slab *newb = new(slab);
		_history.emplace_back(_slab);
		_slab = newb;
	}
};

#endif
