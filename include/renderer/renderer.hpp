#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <common.hpp>
#include <transform.hpp>
#include <string>
#include <vector>

#include <camera.hpp>
#include <resourcemanager.hpp>
#include <renderresource.hpp>


// fwd decls
struct CTexture2D;
struct CTextureCubeMap;
struct CMesh;
struct CMeshBuffer;
struct CModel;
struct CMaterial;

typedef Handle<CTexture2D> CTexture2DRef;
typedef Handle<CTextureCubeMap> CTextureCubeMapRef;
typedef Handle<CMesh> CMeshRef;
typedef Handle<CMeshBuffer> CMeshBufferRef;
typedef Handle<CModel> CModelRef;
typedef Handle<CMaterial> CMaterialRef;

struct Texture2DDesc;
struct TextureCubeMapDesc;
struct MeshBufferInit;

struct RenderData
{
	glm::mat4 mViewMatrix;
	glm::mat4 mProjMatrix;
	glm::ivec2 mViewportSize;
};

// 
class CRendererImplBase
{
public:
	virtual void initialize() = 0;
	
	virtual CTexture2D *createTexture2D(Texture2DDesc &desc, const void *initialData) = 0;
	virtual CTextureCubeMap *createTextureCubeMap(TextureCubeMapDesc &desc, const void *initialData[6]) = 0;

	virtual CMeshBuffer *createMeshBuffer(MeshBufferInit &init) = 0;

	// TODO parameters into a struct
	virtual void submit(CMeshBuffer *meshBuffer, Transform &transform, CMaterial *material) = 0;
	virtual void render(RenderData &renderData) = 0;

	virtual void setClearColor(glm::vec4 const &color) = 0;
	virtual void setClearDepth(float color) = 0;
};

//
// unloader for render resources
/*class RenderResourceLoader : public ResourceLoader
{
public:
	void *load(std::string key) {
		// unimplemented
		throw std::logic_error("RenderResourceLoader::load");
	}

	void destroy(std::string const &key, void *resource) {
		CRenderResource *rr = static_cast<CRenderResource*>(resource);
		rr->destroy();
	}
};

template <typename T>
static inline Handle<T> addRenderResource(T *ptr, std::string const &name)
{
	static_assert(std::is_base_of<CRenderResource, T>::value, "T must be a subclass of CRenderResource");
	return ResourceManager::getInstance().add<T>(name, ptr, std::unique_ptr<ResourceLoader>(new RenderResourceLoader));
}*/

class CRenderer
{
public:
	void initialize(std::unique_ptr<CRendererImplBase> impl);

	CTexture2D *createTexture2D(Texture2DDesc &desc, void const *initialData);
	CTextureCubeMap *createTextureCubeMap(TextureCubeMapDesc &desc, void const *initialData[6]);

	CMesh *createMesh(MeshBufferInit &init);
	CModel *createModel();

	void render(CMesh *mesh, Transform &transform);
	void render(CModel *model, Transform &transform);

	void render();

	//
	// Camera
	void setCamera(Camera &camera);

	//
	// dessin immédiat
	void drawText(const char *text, Transform const &transform);
	void drawLine(glm::vec2 const &begin, glm::vec2 const &end);

	CRendererImplBase &getImpl();
	static CRenderer &getInstance() {
		assert(sInstance);
		return *sInstance;
	}

protected:
	static CRenderer *sInstance;
	std::unique_ptr<CRendererImplBase> mImpl;
	RenderData mRenderData;
};

#endif