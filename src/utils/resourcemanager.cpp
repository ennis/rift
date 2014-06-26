#include <resourcemanager.hpp>
#include <log.hpp>

resource_block *ResourceManagerBase::doLoad(std::string key)
{
	// try to insert a resource block
	auto ins = resourceMap.insert(std::pair<std::string, std::unique_ptr<resource_block> >(key, nullptr));

	auto &block = ins.first->second;

	// not yet loaded
	if (ins.second || ins.first->second->mIsLoaded == false) 
	{
		if (ins.second) {
			block = std::unique_ptr<resource_block>(new resource_block(*this));
		}

		CResourceBase *ptr = mLoader->load(key);
		block->mKey = key;
		block->mData = ptr;
		block->mIsLoaded = true;
	}

	return block.get();
}

void ResourceManagerBase::doUnload(resource_block *block)
{
	if (block->mIsLoaded) {
		auto &key = block->mKey;
		block->check_ref_counts();
		mLoader->destroy(key, block->mData);
		block->mIsLoaded = false;
		block->mData = nullptr;
		//resourceMap.erase(key);
		LOG << "unloaded " << key;
	}
}

void ResourceManagerBase::unloadAll()
{
	for (auto &e : resourceMap) {
		doUnload(e.second.get());
	}
}

void ResourceManagerBase::setLoader(std::unique_ptr<ResourceLoader> loader)
{
	mLoader = std::move(loader);
}

void ResourceManagerBase::printResources()
{
	LOG << "Resource map:";
	LOG << "-------------";

	for (auto &e : resourceMap) {
		if (e.second->mIsLoaded) {
			LOG << e.first << " -> " << e.second->mData << " (strong refs=" << e.second->mData->getRefCount() << ")";
		}
		else {
			LOG << e.first << " (unloaded)";
		}
	}
}
