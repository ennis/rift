#include <immediatecontext.hpp>
#include <cassert>
#include <effect.hpp>

ImmediateContext &ImmediateContext::addVertex(Vertex const &vertex)
{
	assert(mVertices.size() < mMaxNumVertices);
	mVertices.push_back(vertex);
	return *this;
}

ImmediateContext &ImmediateContext::clear()
{
	mVertices.clear();
	return *this;
}

void ImmediateContext::render(RenderContext const &rc)
{
	const unsigned int nv = static_cast<unsigned int>(mVertices.size());
	mRenderer->updateBuffer(mVB, 0, nv * sizeof(Vertex), mVertices.data());
	mRenderer->setShader(mShader);
	mRenderer->setVertexBuffer(0, mVB);
	mRenderer->setVertexLayout(mLayout);
	mRenderer->setConstantBuffer(0, rc.perFrameShaderParameters);
	mRenderer->draw(mPrimitiveType, 0, nv);
}

ImmediateContext::ImmediateContext(
	Renderer &renderer,
	Shader *shader,
	VertexLayout *layout,
	int maxNumVertices,
	PrimitiveType primitiveType) :
mRenderer(&renderer),
mShader(shader),
mLayout(layout),
mMaxNumVertices(maxNumVertices),
mPrimitiveType(primitiveType)
{
	mVB = mRenderer->createVertexBuffer(
		sizeof(Vertex), 
		maxNumVertices, 
		ResourceUsage::Dynamic, 
		nullptr);
}

ImmediateContextFactory::ImmediateContextFactory(Renderer &renderer) : 
mRenderer(&renderer)
{
	VertexElement2 elements[2] = {
		VertexElement2(0, ElementFormat::Float4),
		VertexElement2(0, ElementFormat::Float4)
	};
	mLayout = mRenderer->createVertexLayout2(2, elements);
	mShader = mRenderer->createShader(
		loadShaderSource("resources/shaders/immediate/vert.glsl").c_str(),
		loadShaderSource("resources/shaders/immediate/frag.glsl").c_str());
}

ImmediateContextFactory::~ImmediateContextFactory()
{
	// TODO release resources
}

ImmediateContext *ImmediateContextFactory::create(int maxNumVertices, PrimitiveType primitiveType)
{
	return new ImmediateContext(
		*mRenderer, 
		mShader,
		mLayout,
		maxNumVertices,
		primitiveType);
}
