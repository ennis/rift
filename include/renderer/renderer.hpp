#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <common.hpp>
#include <transform.hpp>
#include <string>
#include <vector>

#include <mesh.hpp>
#include <texture.hpp>
#include <camera.hpp>
#include <material.hpp>
#include <model.hpp>


// fwd decls
struct CTexture;
struct CMesh;
struct CMeshBuffer;

struct TextureDesc;
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
	virtual CTexture *createTexture(TextureDesc &desc) = 0;
	virtual CMeshBuffer *createMeshBuffer(MeshBufferInit &init) = 0;
	// TODO parameters into a struct
	virtual void submit(CMeshBuffer *meshBuffer, Transform &transform, CMaterial *material) = 0;
	virtual void render(RenderData &renderData) = 0;

	virtual void setClearColor(glm::vec4 const &color) = 0;
	virtual void setClearDepth(float color) = 0;
};


class CRenderer
{
public:
	void initialize(std::unique_ptr<CRendererImplBase> impl);
	CTexture *createTexture(TextureDesc &desc);
	CMesh *createMesh(MeshBufferInit &init);
	CModel *createModel();
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