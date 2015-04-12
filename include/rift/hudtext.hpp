#ifndef HUDTEXT_HPP
#define HUDTEXT_HPP

#include <gl4/renderer.hpp>
#include <font.hpp>
#include <scene.hpp>
#include <string_ref.hpp>

class HUDTextRenderer
{
public:
	HUDTextRenderer();
	
	void renderText(
		SceneRenderContext& context,
		util::string_ref str,
		const Font& font,
		glm::vec2 viewPos,
		const glm::vec4& color,
		const glm::vec4& outlineColor);

	void fence(SceneRenderContext& context);

private:
	static constexpr auto kMaxGlyphsPerFrame = 1024 * 1024u;
	static constexpr auto kMaxCallsPerFrame = 1024;

	Stream::Ptr vb_stream;
	Stream::Ptr ib_stream;
	InputLayout::Ptr layout;
	Shader::Ptr shader;
	Stream::Ptr cb_stream;
};

#endif