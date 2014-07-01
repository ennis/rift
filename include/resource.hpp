#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include <type_traits>

enum class DeletionPolicy
{
	Delete,		// delete when last reference released (automatic memory management)
	KeepAlive	// do not delete
};

class CResourceBase
{
public:
	CResourceBase() = default;
	virtual ~CResourceBase()
	{}

	void addRef();
	void release();
	
	int getRefCount() const {
		return mRefCount;
	}

	void setDeletionPolicy(DeletionPolicy policy) {
		mDeletionPolicy = policy;
	}

protected:
	virtual void deleteResource();

	DeletionPolicy mDeletionPolicy = DeletionPolicy::Delete;
	int mRefCount = 0;
};

// automatic pointer
template <typename T>
class resource_ptr
{
public:
	static_assert(std::is_base_of<CResourceBase, T>::value, "resource_ptr: T must be a subclass of CResourceBase");

	resource_ptr(CResourceBase *ptr) : mPtr(ptr) {
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