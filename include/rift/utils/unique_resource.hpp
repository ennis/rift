#ifndef UNIQUE_RESOURCE_HPP
#define UNIQUE_RESOURCE_HPP

namespace util
{

template <typename T>
struct null_deleter
{
	void operator()(T) {}
};

template <
	typename T, 
	typename Deleter = null_deleter<T>,
	T Null = reinterpret_cast<T>(nullptr)
	>
class unique_resource
{
public:
	unique_resource() = default;
	unique_resource(T res) : resource(res)
	{}
	unique_resource(const unique_resource<T, Deleter, Null> &) = delete;
	unique_resource &operator=(const unique_resource<T, Deleter, Null> &) = delete;
	unique_resource(unique_resource<T, Deleter, Null> &&rhs) : resource(rhs.resource), deleter(rhs.deleter)
	{}
	unique_resource &operator=(unique_resource<T, Deleter, Null> &&rhs) 
	{
		reset();
		resource = rhs.resource;
		deleter = rhs.deleter;
		return *this;
	}

	void reset()
	{
		if (resource != Null)
			deleter(resource);
	}

	T get() const {
		return resource;
	}

private:
	T resource = Null;
	Deleter deleter = Deleter();
};

template <
	typename T,
	typename Deleter = null_deleter<T>,
	T Null = reinterpret_cast<T>(nullptr)
	>
unique_resource<T, Deleter, Null> 
make_unique_resource(T res) 
{
	return unique_resource<T, Deleter, Null>(res);
}

}
 
#endif /* end of include guard: UNIQUE_RESOURCE_HPP */