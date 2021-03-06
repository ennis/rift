#include <scene/scene.hpp>
#include <filesystem>
#include <utils/binary_io.hpp>
#include <rendering/opengl4.hpp>
#include <asset_database.hpp>

namespace fs = std::experimental::filesystem;

namespace
{
	enum ObjectType
	{
		OT_GameObject
	};

	enum ComponentClass
	{
		CC_Transform,
		CC_Mesh,
		CC_End = 0xFFFF
	};

	std::string getAssetPath(fs::path sceneDir, std::string relpath)
	{
		auto fullpath = sceneDir / relpath;
		return fullpath.string();
	}

	using AssetMap = std::unordered_map<std::string, std::string>;

	Material *loadMaterialAsset(AssetMap& assetMap, const std::string &id, AssetDatabase& assetDb, GraphicsContext& gc)
	{
		auto path = assetMap.at(id);
		return assetDb.loadAsset<Material>(path, [&](){
			auto fileIn = std::ifstream(path, std::ios::binary);
			auto bin = util::BinaryReader(fileIn);
			std::string shaderid;
			bin >> shaderid;
			auto shaderPath = "resources/shaders/default.glsl";
			auto shader = loadShaderAsset(assetDb, gc, shaderPath);
			auto ptr = std::make_unique<Material>();
			ptr->shader = shader;
			std::string texname;
			std::string texid;
			while (bin >> texname)
			{
				bin >> texid;
				texid = assetMap.at(texid);
				auto tex = assetDb.loadAsset<Texture2D>(texid, [&]() {
					LOG << "Material: loading " << texid << "...";
					Image img = Image::loadFromFile(texid.c_str());
					return Texture2D::createFromImage(img);
				});
				if (texname == "mainTexture")
				{
					ptr->diffuseMap = tex;
				}
			}
			return std::move(ptr);
		});
	}
}

void Scene::loadFromFile(GraphicsContext &gc, const char *path)
{
	auto dir = fs::path(path).parent_path();

	AssetMap asset_map;
	
	fs::directory_iterator end_it;
	// scan assets
	for (auto dir_it = fs::directory_iterator(dir); dir_it != end_it; ++dir_it)
	{
		if (!fs::is_directory(dir_it->status()))
		{
			// check for a recognized extension
			auto ext = dir_it->path().extension();
			if (ext == ".mesh"
				|| ext == ".mat"
				// image types
				|| ext == ".jpg"
				|| ext == ".jpeg"
				|| ext == ".png"
				|| ext == ".tga"
				|| ext == ".dds"
				|| ext == ".psd")
			{
				// ok, extension recognized, add to asset map
				auto assetname = dir_it->path().stem().string();
				asset_map.insert(std::pair<std::string, std::string>(assetname, dir_it->path().string()));
				LOG << "Found asset " << assetname;
			}
		}
	}

	std::unordered_map<unsigned, EntityID> fileIdToEntity;
	std::unordered_map<EntityID, unsigned> entityToParentFileId;

	std::ifstream fileIn(path, std::ios::binary);
	util::BinaryReader bin(fileIn);

	::uint8_t version;
	bin >> util::read8(version);
	assert(version == 1);

	::uint16_t c;
	while (bin >> util::read16(c))
	{
		if (c == OT_GameObject)
		{
			auto id = createEntity();
			unsigned obj_id;
			std::string objname;
			bin >> objname;
			bin >> obj_id;
			bin >> util::read16(c);
			fileIdToEntity[obj_id] = id;
			while (c != CC_End)
			{
				// game object
				if (c == CC_Transform)
				{
					// parent entity
					unsigned parent_id;
					bin >> parent_id;
					entityToParentFileId[id] = parent_id;
					auto t = getTransformNode(id);
					bin >> t->transform.position.x;
					bin >> t->transform.position.y;
					bin >> t->transform.position.z;
					bin >> t->transform.rotation.x;
					bin >> t->transform.rotation.y;
					bin >> t->transform.rotation.z;
					bin >> t->transform.rotation.w;
					bin >> t->transform.scaling.x;
					bin >> t->transform.scaling.y;
					bin >> t->transform.scaling.z;
				}
				else if (c == CC_Mesh)
				{
					std::string relpath;
					bin >> relpath;

					auto mesh =  loadMeshAsset(assetDb, gc, asset_map[relpath]);
					// load materials
					unsigned nmat;
					bin >> nmat;
					std::vector<Material*> mats(nmat);
					for (auto i = 0u; i < nmat; ++i) {
						std::string matid;
						bin >> matid;
						mats[i] = loadMaterialAsset(asset_map, matid, assetDb, gc);
					}
					if (mats[0]->diffuseMap == nullptr)
						mats[0] = nullptr;
					createMeshNode(id, *mesh, *mats[0]);
				}
				bin >> util::read16(c);
			}
		}
	}

	// fix parent relations
	for (auto &ent : entityToParentFileId)
	{
		auto &t = transforms[ent.first];
		if (ent.second != -1)
			t.parent = fileIdToEntity[ent.second];
		else
			t.parent = -1;
	}
}
