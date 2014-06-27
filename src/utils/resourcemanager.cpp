#include <resourcemanager.hpp>
#include <log.hpp>

resource_block *ResourceManager::loadImpl(std::string key, std::unique_ptr<ResourceLoader> loader)
{
	// try to insert a resource block
	auto ins = resourceMap.insert(std::pair<std::string, std::unique_ptr<resource_block> >(key, nullptr));

	auto &block = ins.first->second;

	// not yet loaded
	if (ins.second || ins.first->second->mIsLoaded == false) 
	{
		if (ins.second) {
			block = std::unique_ptr<resource_block>(new resource_block());
		}

		block->mLoader = std::move(loader);
		block->mKey = key;
		block->mData = block->mLoader->load(key);
		block->mIsLoaded = true;
		block->mIsDynamic = false;
	}

	return block.get();
}


resource_block *ResourceManager::addImpl(std::string key, void *data, std::unique_ptr<ResourceLoader> loader)
{
	// try to insert a resource block
	if (key.size() == 0) {
		key = unique_key();
	}

	auto ins = resourceMap.insert(std::pair<std::string, std::unique_ptr<resource_block> >(key, nullptr));

	auto &block = ins.first->second;

	// not yet loaded
	if (ins.second || ins.first->second->mIsLoaded == false)
	{
		if (ins.second) {
			block = std::unique_ptr<resource_block>(new resource_block());
		}

		block->mLoader = std::move(loader);
		block->mKey = key;
		// do not load
		block->mData = data;
		block->mIsLoaded = true;
		block->mIsDynamic = true;
	}

	return block.get();
}


void ResourceManager::unloadImpl(resource_block *block)
{
	if (block->mIsLoaded) {
		auto &key = block->mKey;
		block->check_ref_counts();
		block->mIsLoaded = false;
		if (block->mLoader)
			block->mLoader->destroy(block->mKey, block->mData);
		// lost forever
		block->mData = nullptr;
		//resourceMap.erase(key);
		LOG << "unloaded " << key;
	}
}

void ResourceManager::unloadAll()
{
	for (auto &e : resourceMap) {
		unloadImpl(e.second.get());
	}
}

void ResourceManager::printResources()
{
	LOG << "Resource map:";
	LOG << "-------------";

	for (auto &e : resourceMap) {
		if (e.second->mIsLoaded) {
			LOG << e.first << " -> " << e.second->mData << " (strong refs=" << e.second->mStrongRefs << ")";
		}
		else {
			LOG << e.first << " (unloaded)";
		}
	}
}

std::string unique_key()
{
	static int counter = 0;
	return "<" + std::to_string(counter++) + ">";
}

ResourceManager ResourceManager::sInstance;