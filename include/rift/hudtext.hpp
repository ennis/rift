#ifndef HUDTEXT_HPP
#define HUDTEXT_HPP

#include <gl4/renderer.hpp>
#include <font.hpp>
#include <scene.hpp>

class HUDTextRenderer
{
public:
	HUDTextRenderer();
	~HUDTextRenderer();
	
	void renderString(
		const SceneData &sceneData,
		const char *str,
		const Font &font,
		glm::vec2 viewPos,
		glm::vec4 const &color = glm::vec4(1.0f),
		glm::vec4 const &outlineColor = glm::vec4(0.0f));

private:
	static const int kMaxNumGlyphs = 128;
	void init();
	Shader::Ptr shader;
	Buffer::Ptr vb;
};

#endif