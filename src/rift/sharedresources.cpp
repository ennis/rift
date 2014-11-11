#include <sharedresources.hpp>
#include <log.hpp>

//=============================================================================
VertexLayout *GVertexLayout_V3F = nullptr;
VertexLayout *GVertexLayout_V2F = nullptr;
VertexBuffer *GScreenQuad = nullptr;

//=============================================================================
void initSharedResources(Renderer &renderer)
{
	static const VertexElement elem_v2f[1] = {
		VertexElement(0, 0, 0, 2*sizeof(float), ElementFormat::Float2)
	};
	GVertexLayout_V2F = renderer.createVertexLayout(1, elem_v2f);

	static const VertexElement elem_v3f[1] = {
		VertexElement(0, 0, 0, 3*sizeof(float), ElementFormat::Float3)
	};
	GVertexLayout_V3F = renderer.createVertexLayout(1, elem_v3f);

	static const float screen_quad[8] = {
		-1.0f,  1.0f, 
		-1.0f, -1.0f, 
		 1.0f,  1.0f,
		 1.0f, -1.0f
	};
	GScreenQuad = renderer.createVertexBuffer(2*sizeof(float), 4, ResourceUsage::Static, screen_quad);
}

//=============================================================================
void releaseSharedResources()
{
	GVertexLayout_V2F->release();
	GVertexLayout_V3F->release();
	GScreenQuad->release();
}
