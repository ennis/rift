#ifndef HUDTEXT_HPP
#define HUDTEXT_HPP

#include <renderer.hpp>
#include <font.hpp>
#include <renderable.hpp>
#include <mesh.hpp>

class HUDTextRenderer
{
public:
	HUDTextRenderer() = default;
	HUDTextRenderer(Renderer &renderer);
	
	~HUDTextRenderer();
	
	void renderString(
		RenderContext &renderContext,
		const char *str,
		Font *font,
		glm::vec2 viewPos,
		glm::vec4 const &color = glm::vec4(1.0f),
		glm::vec4 const &outlineColor = glm::vec4(0.0f));

private:
	static const unsigned int kMaxNumGlyphs = 128;
	void init();
	Renderer *mRenderer = nullptr;	// borrowed ref
	Shader *mShader = nullptr;
	Mesh mMesh;
};

#endif