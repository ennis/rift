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
		RenderQueue &renderQueue,
		util::string_ref str,
		const Font &font,
		glm::vec2 viewPos,
		glm::vec2 viewportSize,
		const glm::vec4 &color,
		const glm::vec4 &outlineColor);

	void fence(RenderQueue &renderQueue);

private:
	static constexpr auto kMaxGlyphsPerFrame = 32768u;
	static constexpr auto kMaxCallsPerFrame = 1024u;

	Stream::Ptr vb_stream;
	Stream::Ptr ib_stream;
	InputLayout::Ptr layout;
	Shader::Ptr shader;
	Stream::Ptr cb_stream;
};

#endif