#ifndef POOL_HPP
#define POOL_HPP

#include <vector>

template <typename T>
class Pool
{
public:
	Pool(int capacity = 10) : store(capacity)
	{}

	void init()
	{
		firstFree = 0;
		for (int i = 0; i < store.size() - 1; ++i) {
			store[i].nextFree = i + 1;
		}
		(*store.rbegin()).nextFree = -1;
	}

	template <typename... Args>
	std::pair<T&, int> allocate(Args&&... args)
	{
		Elem &e = store[firstFree];
		int r = firstFree;
		firstFree = e.nextFree;
		return std::pair<T&, int>(e.obj, r);
	}

private:
	union Elem {
		T obj;
		int nextFree;
	};

	int firstFree;

	std::vector<Elem> store;
};

#endif