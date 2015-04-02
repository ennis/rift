#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <common.hpp>
#include <unordered_map>
#include <log.hpp>
#include <string>
#include <string_ref.hpp>

namespace util
{
	template <typename Loader>
	class resource_manager
	{
	public:
		using key_type = typename Loader::key_type;
		using resource_type = typename Loader::resource_type;
		// must have a get() method
		using pointer = typename Loader::pointer;

		resource_manager(Loader loader_ = Loader()) :
			loader(loader_)
		{
		}

		// returns nullptr if not found
		// TODO smart pointer type?
		resource_type *load(key_type key)
		{
			// try to insert a resource block
			auto ins = resource_map.insert(std::pair<key_type, pointer>(key, nullptr));
			auto &res = ins.first->second;
			// not yet loaded
			if (ins.second) {
				res = loader.load(key);
			}
			else {
				LOG << "Already loaded: " << key;
			}
			return res.get();
		}

	private:
		//
		// resource loader
		// must have methods load(): std::unique_ptr<T> and unload(): std::unique_ptr<T>
		Loader loader;

		//
		// list of resource blocks
		std::unordered_map<key_type, pointer> resource_map;
	};

}

#endif