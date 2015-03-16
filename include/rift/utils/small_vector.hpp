#ifndef SMALLVECTOR_HPP
#define SMALLVECTOR_HPP

#include <array_ref.hpp>
#include <type_traits>

namespace util
{

	template <typename T, std::size_t N>
	class small_vector
	{
	public:
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

		small_vector() {
			length = 0;
		}

		small_vector(std::initializer_list<T> init) {
			assert(init.size() <= N);
			length = init.size();
			size_type i = 0;
			for (auto &&e : init) {
				new (storage_ptr(i++)) T(std::move(e));
			}
		}

		small_vector(size_type len) : length(len)
		{
			assert(len <= N);
			for (size_type i = 0; i < len; ++i) {
				new (storage_ptr(i)) T();
			}
		}

		small_vector(std::array_ref<T> arr)
		{
			assert(arr.size() <= N);
			for (size_type i = 0; i < len; ++i) {
				new (storage_ptr(i)) T(std::move(arr[i]));
			}
		}

		small_vector(const small_vector &s) {
			length = s.length;
			for (size_type i = 0; i < length; ++i) {
				new (storage_ptr(i)) T(s[i]);
			}
		}

		small_vector(small_vector &&s) {
			*this = std::move(s);
		}

		small_vector& operator=(small_vector &&s)
		{
			length = s.length;
			clear();
			for (size_type i = 0; i < length; ++i) {
				new (storage_ptr(i)) T(std::move(s[i]));
			}
			s.clear();
			return *this;
		}

		T& operator[](size_type index) {
			return *storage_ptr(index);
		}

		const T& operator[](size_type index) const {
			return *storage_ptr(index);
		}

		size_type size() const { return length; }
		size_type max_size() const { return N; }
		iterator begin() { return storage_ptr(0); }
		iterator end() { return storage_ptr(length); }
		const_iterator begin() const { return storage_ptr(0); }
		const_iterator end() const { return storage_ptr(length); }
		const_iterator cbegin() const { return begin(); }
		const_iterator cend() const { return end(); }

		void push_back(const T& t) {
			assert(length < N);
			new (storage_ptr(length++)) T(t);
		}

		const T* data() const {
			return storage_ptr(0);
		}

		template <class InputIterator>
		void assign(InputIterator first, InputIterator last)
		{
			clear();
			size_type i = 0;
			// construct remaining objects
			while (first != last) {
				new (storage_ptr(i++)) T(std::move(*first++));
			}
			length = i;
		}

		void clear()
		{
			for (size_type i = 0; i < length; ++i) {
				*static_cast<T*>(storage_ptr(i++))->~T();
			}
			length = 0;
		}

		std::array_ref<T> slice() const {
			return std::make_array_ref(data(), length);
		}

	private:
		T* storage_ptr(size_type index) {
			return reinterpret_cast<T*>(&v[index]);
		}

		const T* storage_ptr(size_type index) const {
			return reinterpret_cast<const T*>(&v[index]);
		}

		size_type length;
		storage_type v[N];
	};

}

#endif /* end of include guard: SMALLVECTOR_HPP */