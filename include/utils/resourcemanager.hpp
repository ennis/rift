#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <common.hpp>
#include <unordered_map>
#include <resource.hpp>
#include <log.hpp>
#include <string>

class ResourceManagerBase;

class ResourceLoader
{
public:
	virtual CResourceBase *load(std::string key) = 0;
	virtual void destroy(std::string const &key, CResourceBase *resource) = 0;
};


struct resource_block
{
	resource_block(ResourceManagerBase &manager) : mManager(manager)
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
		if (mData && mData->getRefCount()) {
			WARNING << "resource_block deleted with non-zero reference counts (strong refs=" << mData->getRefCount() << ')';
		}
	}

	//
	// Manager
	ResourceManagerBase &mManager;

	//
	// pointer to resource
	CResourceBase *mData = nullptr;

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
	// Number of weak references to this resource 
	// Prevents the resource from being unloaded
	int mHandleCount = 0;
};

template <typename T>
class ResourceManager;

template <typename T> 
struct Handle
{
	friend class ResourceManager<T>;

	Handle() : mControlBlock(nullptr), mGeneration(0)
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

	T* get()
	{
		mControlBlock->mData->addRef();
		return static_cast<T*>(mControlBlock->mData);
	}

private:
	resource_block *mControlBlock;
	int mGeneration;
};

class ResourceManagerBase
{
public:
	~ResourceManagerBase()
	{
		unloadAll();
	}

	resource_block *doLoad(std::string key);
	void doUnload(resource_block *handle);
	resource_block *doRegister(std::string key, CResourceBase* resource);
	void setLoader(std::unique_ptr<ResourceLoader> loader);
	void printResources();
	void unloadAll();

private:

	//
	// resource loader
	std::unique_ptr<ResourceLoader> mLoader;

	//
	// resource blocks
	std::unordered_map<std::string, std::unique_ptr<resource_block> > resourceMap;

	typedef std::unordered_map<std::string, std::unique_ptr<resource_block> >::iterator map_iterator;
};

template <typename T>
class ResourceManager : public ResourceManagerBase
{
public:
	Handle<T> load(std::string key) {
		return Handle<T>(doLoad(key));
	}

	void unload(Handle<T> handle) {
		doUnload(handle.mControlBlock);
	}
};

#endif