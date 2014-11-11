#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include <type_traits>
#include <string>

enum class DeletionPolicy
{
	Delete,		// delete when last reference released (automatic memory management)
	KeepAlive	// do not delete
};

// probably good to make a wrapper class around that
class Resource
{
public:
	friend class ResourceManager;

	Resource() = default;

	virtual ~Resource()
	{}

	// ensures the resource is loaded before use
	void load();

	void addRef();
	void release();
	
	int getRefCount() const {
		return mRefCount;
	}

	void setDeletionPolicy(DeletionPolicy policy) {
		mDeletionPolicy = policy;
	}

	void setName(std::string name)
	{
		mKey = name;
	}

	std::string const &getName() const
	{
		return mKey;
	}

protected:
	virtual void destroy();

	DeletionPolicy mDeletionPolicy = DeletionPolicy::Delete;
	int mRefCount = 1;

	//
	// resource key (path)
	std::string mKey;

	//
	// is it dynamic (generated)?
	bool mIsDynamic = false;

	//
	// is the resource still loaded
	bool mIsLoaded = false;

	//
	// generation (incremented each time the resource is reloaded)
	int mGeneration = 0;
};

// automatic pointer
template <typename T>
class resource_ptr
{
public:
	static_assert(std::is_base_of<Resource, T>::value, "resource_ptr: T must be a subclass of Resource");

	resource_ptr(T *ptr) : mPtr(ptr) {
		mPtr->addRef();
	}

	~resource_ptr() {
		mPtr->release();
	}

	resource_ptr(resource_ptr &&moved) {
		reset();
		mPtr = moved.mPtr;
	}

	resource_ptr(resource_ptr const &ref) {
		*this = ref;
	}

	resource_ptr &operator=(resource_ptr const &rhs) {
		reset();
		mPtr = rhs.mPtr;
		mPtr->addRef();
		return *this;
	}

	void reset() {
		if (!empty()) {
			mPtr->release();
		}
		mPtr = nullptr;
	}

	bool empty() const {
		return mPtr == nullptr;
	}

private:
	T *mPtr = nullptr;
};

#endif