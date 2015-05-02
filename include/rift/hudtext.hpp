#ifndef HUDTEXT_HPP
#define HUDTEXT_HPP

#include <gl4/renderer.hpp>
#include <font.hpp>
#include <string_ref.hpp>

class HUDTextRenderer
{
public:
	HUDTextRenderer();
	
	void renderText(
		CommandBuffer &cmdBuf,
		util::string_ref str,
		const Font &font,
		glm::vec2 viewPos,
		glm::vec2 viewportSize,
		const glm::vec4 &color,
		const glm::vec4 &outlineColor);

private:
	static constexpr auto kMaxGlyphsPerCall = 32768u;

	InputLayout::Ptr layout;
	Shader::Ptr shader;
};

#endif