#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <common.hpp>
#include <unordered_map>
#include <log.hpp>
#include <string>
#include <resource.hpp>

class ResourceManager
{
public:
	~ResourceManager()
	{
	}

	// returns nullptr if not found
	template <typename T, typename LoadCallback>
	T *load(std::string key, LoadCallback loadCallback)
	{
		//return static_cast<T*>(loadImpl(key, loader));
		// try to insert a resource block
		auto ins = resourceMap.insert(std::pair<std::string, CResourceBase*>(key, nullptr));

		auto &res = ins.first->second;

		// not yet loaded
		if (ins.second) {
			res = loadCallback(key);
			//res->load();
			res->setName(key);
		}

		return static_cast<T*>(res);
	}

	void printResources();

	static ResourceManager &getInstance() {
		return sInstance;
	}

private:
	//
	// list of resource blocks
	std::unordered_map<std::string, Resource*> resourceMap;

	typedef std::unordered_map<std::string, Resource*>::iterator map_iterator;

	// resource manager instance
	static ResourceManager sInstance;
};

std::string unique_key();


#endif