#include <resourcemanager.hpp>
#include <log.hpp>

void ResourceManager::printResources()
{
	LOG << "Resource map:";
	LOG << "-------------";

	for (auto &e : resourceMap) {
		if (e.second->mIsLoaded) {
			LOG << e.first << " -> " << e.second << " (strong refs=" << e.second->getRefCount() << ")";
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