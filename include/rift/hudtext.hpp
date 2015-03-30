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
	
	void renderString(
		util::string_ref str,
		const Font &font,
		glm::vec2 viewPos,
		const glm::vec4 &color,
		const glm::vec4 &outlineColor,
		RenderQueue &renderQueue,
		const SceneData &sceneData,
		const ConstantBuffer &sceneDataCB);

private:
	static constexpr auto kMaxNumGlyphs = 128u;

	// TODO auto-generation of parameter struct
	struct Params
	{
		glm::mat4 transform;
		glm::vec4 fillColor;
		glm::vec4 outlineColor;
	};

	int frame = 0;
	GLsync sync;
	GLsync sync2;
	Mesh::Ptr mesh;
	Shader::Ptr shader;
	ParameterBlock::Ptr pb;
	ConstantBuffer::Ptr cbParams;
};

#endif