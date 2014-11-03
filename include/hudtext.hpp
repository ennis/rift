#ifndef HUDTEXT_HPP
#define HUDTEXT_HPP

#include <renderer.hpp>
#include <font.hpp>
#include <renderable.hpp>

class HUDTextRenderer
{
public:
	HUDTextRenderer(Renderer &renderer);
	~HUDTextRenderer();
	void renderString(
		RenderContext &renderContext,
		const char *str,
		Font *font,
		glm::ivec2 viewPos,
		glm::vec4 const &color = glm::vec4(1.0f),
		glm::vec4 const &outlineColor = glm::vec4(0.0f));
private:
	static const unsigned int kMaxNumGlyphs = 128;
	void init();
	Renderer *mRenderer;	// borrowed ref
	Shader *mShader;
	VertexBuffer *mVB;
	IndexBuffer *mIB;
	VertexLayout *mLayout;
};

#endif