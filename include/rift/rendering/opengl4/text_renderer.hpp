#ifndef TEXT_RENDERER_HPP
#define TEXT_RENDERER_HPP

#include <rendering/opengl4/graphics_context.hpp>
#include <font.hpp>
#include <string_ref.hpp>

namespace gl4
{
	class TextRenderer
	{
	public:
		TextRenderer(gl4::GraphicsContext &graphicsContext);
		
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

		gl4::GraphicsContext &context;
		InputLayout::Ptr layout;
		PipelineState::Ptr textPS;
	};
}

#endif