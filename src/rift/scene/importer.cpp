#include <scene/scene.hpp>
#include <boost/filesystem.hpp>
#include <utils/binary_io.hpp>
#include <rendering/opengl4/material.hpp>
#include <asset_database.hpp>

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

	std::string getAssetPath(boost::filesystem::path sceneDir, std::string relpath)
	{
		auto fullpath = sceneDir / relpath;
		return fullpath.string();
	}

	using AssetMap = std::unordered_map<std::string, std::string>;

	gl4::Material *loadMaterialAsset(AssetMap& assetMap, const std::string &id, AssetDatabase& assetDb, gl4::GraphicsContext& gc)
	{
		auto path = assetMap.at(id);
		return assetDb.loadAsset<gl4::Material>(path, [&](){
			auto fileIn = std::ifstream(path, std::ios::binary);
			auto bin = util::BinaryReader(fileIn);
			std::string shaderid;
			bin >> shaderid;
			auto shaderPath = "resources/shaders/default.glsl";
			auto shader = assetDb.loadAsset<gl4::Shader>(shaderPath, [&]() {
				return gl4::Shader::loadFromFile(shaderPath);
			});
			auto ptr = std::make_unique<gl4::Material>();
			ptr->shader = shader;
			std::string texname;
			std::string texid;
			while (bin >> texname)
			{
				bin >> texid;
				texid = assetMap.at(texid);
				auto tex = assetDb.loadAsset<gl4::Texture2D>(texid, [&]() {
					LOG << "Material: loading " << texid << "...";
					Image img = Image::loadFromFile(texid.c_str());
					return gl4::Texture2D::createFromImage(img);
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

void Scene::loadFromFile(const char *path)
{
	namespace fs = boost::filesystem;
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

	std::ifstream fileIn(path, std::ios::binary);
	util::BinaryReader bin(fileIn);

	uint8_t version;
	bin >> util::read8(version);
	assert(version == 1);

	uint16_t c;
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
			while (c != CC_End)
			{
				// game object
				if (c == CC_Transform)
				{
					// parent entity
					int parent_id;
					bin >> parent_id;
					auto t = getTransform(id);
					bin >> t->position.x;
					bin >> t->position.y;
					bin >> t->position.z;
					bin >> t->rotation.x;
					bin >> t->rotation.y;
					bin >> t->rotation.z;
					bin >> t->rotation.w;
					bin >> t->scaling.x;
					bin >> t->scaling.y;
					bin >> t->scaling.z;
				}
				else if (c == CC_Mesh)
				{
					std::string relpath;
					bin >> relpath;

					auto mesh = assetDb.loadAsset<gl4::Mesh>(asset_map[relpath], [&](){
						LOG << "Scene: loading mesh: " << relpath << "...";
						std::ifstream fileIn(asset_map[relpath], std::ios::binary);
						MeshData data;
						data.loadFromStream(fileIn);
						return gl4::Mesh::create(graphicsContext, data);
					});

					// load materials
					unsigned nmat;
					bin >> nmat;
					std::vector<gl4::Material*> mats(nmat);
					for (auto i = 0u; i < nmat; ++i)
					{
						std::string matid;
						bin >> matid;
						mats[i] = loadMaterialAsset(asset_map, matid, assetDb, graphicsContext);
					}
					if (mats[0]->diffuseMap == nullptr)
					{
						mats[0] = defaultMaterial.get();
					}
					createMeshNode(id, *mesh, *mats[0]);
				}
				bin >> util::read16(c);
			}
		}
	}
}
