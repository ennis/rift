#ifndef RESOURCES_HPP
#define RESOURCES_HPP

#include <resourcemanager.hpp>
#include <gl4/renderer.hpp>
#include <image.hpp>
#include <mesh.hpp>

struct Texture2DLoader
{
	using pointer = Texture2D::Ptr;
	using key_type = std::string;
	using resource_type = Texture2D;

	Texture2D::Ptr load(const std::string &path)
	{
		Image img = Image::loadFromFile(path.data());
		return img.convertToTexture2D();
	}
};

struct TextureCubeMapLoader
{
	using pointer = TextureCubeMap::Ptr;
	using key_type = std::string;
	using resource_type = TextureCubeMap;

	TextureCubeMap::Ptr load(const std::string &path)
	{
		Image img = Image::loadFromFile(path.data());
		return img.convertToTextureCubeMap();
	}
};

struct MeshLoader
{
	using pointer = Mesh::Ptr;
	using key_type = std::string;
	using resource_type = Mesh;

	Mesh::Ptr load(const std::string &path)
	{
		std::ifstream fileIn(path.data(), std::ios::binary);
		serialization::IArchive arc(fileIn);
		return Mesh::loadFromArchive(arc);
	}
}; 

/*struct SkinnedMeshLoader
{
	using pointer = SkinnedMesh::Ptr;
	using key_type = std::string;
	using resource_type = SkinnedMesh;

	SkinnedMesh::Ptr load(const std::string &path)
	{
		std::ifstream fileIn(path.data(), std::ios::binary);
		serialization::IArchive arc(fileIn);
		return SkinnedMesh::loadFromArchive(arc);
	}
};*/


struct Resources
{
	util::resource_manager<Texture2DLoader> textures;
	util::resource_manager<TextureCubeMapLoader> cubeMaps;
	util::resource_manager<MeshLoader> meshes;
	//util::resource_manager<SkinnedMeshLoader> skinnedMeshes;
};

 
#endif /* end of include guard: RESOURCES_HPP */