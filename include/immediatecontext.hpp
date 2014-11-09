#ifndef IMMEDIATECONTEXT_HPP
#define IMMEDIATECONTEXT_HPP

#include <renderer.hpp>
#include <renderable.hpp>
#include <vector>

struct Vertex
{
	Vertex() = default;
	Vertex(glm::vec3 const &position_, glm::vec4 const &color_) :
	position(glm::vec4(position_, 1.0)),
	color(color_)
	{}

	glm::vec4 position = glm::vec4(0, 0, 0, 1);
	glm::vec4 color = glm::vec4(1, 1, 1, 1);
};

// A more convenient vertex buffer
class ImmediateContext
{
	friend class ImmediateContextFactory;
public:
	ImmediateContext &addVertex(Vertex const &vertex);
	ImmediateContext &clear();
	void render(RenderContext const &rc);
private:
	ImmediateContext(
		Renderer &renderer,
		Shader *shader,
		VertexLayout *layout,
		int maxNumVertices,
		PrimitiveType primitiveType);
	Renderer *mRenderer; // borrowed
	Shader *mShader;	// borrowed
	VertexLayout *mLayout;
	int mMaxNumVertices;
	PrimitiveType mPrimitiveType;
	std::vector<Vertex> mVertices;
	VertexBuffer *mVB;	// owned
};

class ImmediateContextFactory
{
public:
	ImmediateContextFactory(Renderer &renderer);
	~ImmediateContextFactory();
	ImmediateContext *create(int maxNumVertices, PrimitiveType primitiveType);
private:
	Renderer *mRenderer;
	VertexLayout *mLayout;
	Shader *mShader;
};

#endif /* end of include guard: IMMEDIATECONTEXT_HPP */