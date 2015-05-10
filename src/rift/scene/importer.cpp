#include <scene/scene.hpp>
#include <boost/filesystem.hpp>

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
}

void Scene::loadFromFile(const char *path, ResourceLoader &resources)
{
	using namespace serialization;
	auto dir = boost::filesystem::path(path).parent_path();

	std::ifstream fileIn(path, std::ios::binary);
	IArchive arcIn(fileIn);

	uint8_t version;
	arcIn >> read8(version);
	assert(version == 1);

	uint16_t c;
	while (arcIn >> read16(c))
	{
		if (c == OT_GameObject)
		{
			auto id = createEntity();
			unsigned obj_id;
			arcIn >> obj_id;
			arcIn >> read16(c);
			while (c != CC_End)
			{
				// game object
				if (c == CC_Transform)
				{
					// parent entity
					int parent_id;
					arcIn >> parent_id;
					auto t = getTransform(id);
					arcIn >> t->position.x;
					arcIn >> t->position.y;
					arcIn >> t->position.z;
					arcIn >> t->rotation.x;
					arcIn >> t->rotation.y;
					arcIn >> t->rotation.z;
					arcIn >> t->rotation.w;
					arcIn >> t->scaling.x;
					arcIn >> t->scaling.y;
					arcIn >> t->scaling.z;
				}
				else if (c == CC_Mesh)
				{
					std::string relpath;
					arcIn >> relpath;
					auto fullpath = dir / relpath;
					LOG << "Scene: loading " << fullpath.c_str();
					auto fullpath_str = fullpath.string();
					auto mesh = resources.loadMesh(fullpath_str.c_str(), graphicsContext);
					createMeshNode(id, *mesh, *defaultMaterial);
				}
				arcIn >> read16(c);
			}
		}
	}
}
