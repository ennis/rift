#ifndef RESOURCE_LOADER_HPP
#define RESOURCE_LOADER_HPP

#include <resource_manager.hpp>
#include <rendering/opengl4/opengl4.hpp>
#include <rendering/opengl4/graphics_context.hpp>
#include <rendering/opengl4/mesh_renderer.hpp>
#include <image.hpp>
#include <log.hpp>

struct Texture2DLoader
{
	using pointer = gl4::Texture2D::Ptr;
	using key_type = std::string;
	using resource_type = gl4::Texture2D;

	gl4::Texture2D::Ptr load(const std::string &path)
	{
		LOG << "Loading texture: " << path << "...";
		Image img = Image::loadFromFile(path.data());
		return gl4::Texture2D::createFromImage(img);
	}
};

struct TextureCubeMapLoader
{
	using pointer = gl4::TextureCubeMap::Ptr;
	using key_type = std::string;
	using resource_type = gl4::TextureCubeMap;

	gl4::TextureCubeMap::Ptr load(const std::string &path)
	{
		LOG << "Loading cube map texture: " << path << "...";
		Image img = Image::loadFromFile(path.data());
		return gl4::TextureCubeMap::createFromImage(img);
	}
};

struct MeshLoader
{
	using pointer = gl4::Mesh::Ptr;
	using key_type = std::string;
	using resource_type = gl4::Mesh;

	gl4::Mesh::Ptr load(const std::string &path, gl4::GraphicsContext &context)
	{
		LOG << "Loading mesh: " << path << "...";
		std::ifstream fileIn(path.data(), std::ios::binary);
		MeshData data;
		data.loadFromStream(fileIn);
		return gl4::Mesh::create(context, data);
	}
};

struct ShaderLoader
{
	using pointer = gl4::Shader::Ptr;
	using key_type = std::string;
	using resource_type = gl4::Shader;

	gl4::Shader::Ptr load(const std::string &path, gl4::GraphicsContext &context)
	{
		return gl4::Shader::loadFromFile("resources/shaders/default.glsl");
	}
};


struct ResourceLoader
{
	gl4::Texture2D *loadTexture(const char *path)
	{
		return textures.load(std::string(path));
	}

	gl4::TextureCubeMap *loadCubeMap(const char *path)
	{
		return cubeMaps.load(std::string(path));
	}

	gl4::Mesh *loadMesh(const char *path, gl4::GraphicsContext &gc)
	{
 		return meshes.load(std::string(path), gc);
	}

	gl4::Shader *loadShader(const char *path, gl4::GraphicsContext &gc)
	{
		return shaders.load(std::string(path), gc);
	}

	util::resource_manager<Texture2DLoader> textures;
	util::resource_manager<TextureCubeMapLoader> cubeMaps;
	util::resource_manager<MeshLoader> meshes;
	util::resource_manager<ShaderLoader> shaders;

	//util::resource_manager<SkinnedMeshLoader> skinnedMeshes;
};

 
#endif /* end of include guard: RESOURCE_LOADER_HPP */