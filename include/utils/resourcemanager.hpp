#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <common.hpp>
#include <unordered_map>
#include <log.hpp>
#include <string>

class ResourceLoader
{
public:
	virtual void *load(std::string key) = 0;
	virtual void destroy(std::string const &key, void *resource) = 0;
};

struct resource_block
{
	resource_block()
	{}

	~resource_block()
	{
		check_ref_counts();
	}

	//
	// unload
	void unload()
	{
		check_ref_counts();
		mData = nullptr;
		mIsLoaded = false;
		mHandleCount = 0;
	}

	//
	// check reference counts
	void check_ref_counts()
	{
		if (mStrongRefs) {
			WARNING << "non-zero reference counts for resource " << mKey << " (strong refs=" << mStrongRefs << ')';
		}
	}

	//
	// Loader
	std::unique_ptr<ResourceLoader> mLoader = nullptr;

	//
	// pointer to resource
	void *mData = nullptr;

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

	//
	// Number of strong refs
	int mStrongRefs = 0;

	//
	// Number of weak references to this resource 
	// Prevents the resource from being unloaded
	int mHandleCount = 0;
};

template <typename T> 
struct Handle
{
	friend class ResourceManager;

	Handle() : mControlBlock(nullptr), mGeneration(0)
	{}

	Handle(nullptr_t) :
		mControlBlock(nullptr),
		mGeneration(0)
	{}

	Handle(resource_block *controlBlock) :
		mControlBlock(controlBlock), 
		mGeneration(controlBlock->mGeneration)
	{}

	~Handle()
	{}

	bool isLoaded() const 
	{
		return mControlBlock->mIsLoaded;
	}

	bool reloaded()
	{
		bool b = mGeneration < mControlBlock->mGeneration;
		mGeneration = mControlBlock->mGeneration;
		return b;
	}

	T *get() 
	{
		if (!mControlBlock) return nullptr;
		return static_cast<T*>(mControlBlock->mData);
	}

	T *operator->()
	{
		return get();
	}

	template <typename U>
	operator Handle<U>() {
		static_assert(std::is_base_of<U, T>::value, "invalid handle cast");
		return Handle<U>(mControlBlock);
	}

	resource_block *mControlBlock;
	int mGeneration;
};

template <typename U, typename T>
Handle<U> static_handle_cast(Handle<T> handle)
{
	// TODO static assert here
	return Handle<U>(handle.mControlBlock);
}


class ResourceManager
{
public:
	~ResourceManager()
	{
		unloadAll();
	}

	template <typename T>
	Handle<T> load(std::string key, std::unique_ptr<ResourceLoader> loader)
	{
		return Handle<T>(loadImpl(key, std::move(loader)));
	}

	template <typename T>
	void unload(Handle<T> handle)
	{
		unloadImpl(handle.mControlBlock);
	}

	template <typename T>
	Handle<T> add(std::string key, void *data, std::unique_ptr<ResourceLoader> loader = nullptr)
	{
		return Handle<T>(addImpl(key, data, std::move(loader)));
	}

	void printResources();
	void unloadAll();

	static ResourceManager &getInstance() {
		return sInstance;
	}

private:
	resource_block *loadImpl(std::string key, std::unique_ptr<ResourceLoader> loader);
	void unloadImpl(resource_block *handle);
	resource_block *addImpl(std::string key, void* resource, std::unique_ptr<ResourceLoader> loader);

	//
	// list of resource blocks
	std::unordered_map<std::string, std::unique_ptr<resource_block> > resourceMap;

	typedef std::unordered_map<std::string, std::unique_ptr<resource_block> >::iterator map_iterator;

	// resource manager instance
	static ResourceManager sInstance;
};

template <typename T>
class DefaultResourceLoader : public ResourceLoader
{
public:
	void *load(std::string key) {
		throw std::logic_error("DefaultResourceLoader::load");
	}

	void destroy(std::string const &key, void *resource) {
		delete static_cast<T>(resource);
	}

private:
};

std::string unique_key();

template <typename T>
Handle<T> make_handle(
	T *data, 
	std::unique_ptr<ResourceLoader> loader = std::unique_ptr<DefaultResourceLoader<T> > (new DefaultResourceLoader<T>()), 
	std::string name = unique_key())
{
	return ResourceManager::getInstance().add<T>(name, data, std::move(loader));
}

#endif