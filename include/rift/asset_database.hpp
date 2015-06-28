#ifndef ASSET_DATABASE_HPP
#define ASSET_DATABASE_HPP

#include <asset.hpp>
#include <unordered_map>

class AssetDatabase
{
public:
	using AssetID = std::string;
	using AssetPtr = std::shared_ptr<Asset>;

	template <typename AssetType, typename LoadFn>
	AssetType *loadAsset(AssetID assetID, LoadFn loadFn)
	{
		auto ins = assetMap.insert(std::pair<AssetID, AssetPtr>(assetID, nullptr));
		auto &res = ins.first->second;
		// not yet loaded
		if (ins.second) {
			res = std::move(loadFn());
		}
		return static_cast<AssetType*>(res.get());
	}

private:
	std::unordered_map<std::string, std::string> assetIdToPath;
	std::unordered_map<std::string, AssetPtr> assetMap;
};
 
#endif /* end of include guard: ASSET_DATABASE_HPP */