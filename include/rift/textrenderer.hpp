#ifndef TEXTRENDERER_HPP
#define TEXTRENDERER_HPP

#include <gl4/renderer.hpp>
#include <font.hpp>
#include <string_ref.hpp>

class TextRenderer
{
public:
	TextRenderer();
	
	void render(
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
	PipelineState::Ptr textPS;
};

#endif