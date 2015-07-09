#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

namespace util
{
	// static ring buffer implementation
	// N >= 1
	template <typename T, std::size_t N>
	class ring_buffer
	{
		typedef T value_type;
		typedef const T* pointer;
		typedef const T& reference;
		typedef const T& const_reference;
		typedef const T* const_iterator;
		typedef T* iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		using storage_type = typename std::aligned_storage<sizeof(T)>::type;


		void push_back(const T& t) {
			assert(length < N);
			new (storage_ptr(ring_next())) T(t);
		}

		void push_back(T&& t) {
			assert(length < N);
			new (storage_ptr(ring_next())) T(std::move(t));
		}

		size_t ring_next()
		{
			return (++start) % N;
		}


	private:
		size_t start;
		size_t stop;
		size_t length;
		storage_type v[N];
	};
}

 
#endif /* end of include guard: RING_BUFFER_HPP */