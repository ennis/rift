#include <hudtext.hpp>
#include <cstring>
#include <effect.hpp>
#include <transform.hpp>

namespace
{
	// TODO auto-generation of parameter block
	struct TextParams
	{
		glm::mat4 transform;
		glm::vec4 fillColor;
		glm::vec4 outlineColor;
	}; 
	
	struct GlyphVertex
	{
		float x, y;
		float tx, ty;
	};
}

HUDTextRenderer::HUDTextRenderer() 
{
	layout = InputLayout::create(1, { Attribute{ ElementFormat::Float4 } });
	auto effect = gl4::Effect::loadFromFile("resources/shaders/text.glsl");
	RasterizerDesc rs;
	rs.fillMode = PolygonFillMode::Fill;
	DepthStencilDesc ds;
	ds.depthTestEnable = false;
	ds.depthWriteEnable = false;
	BlendDesc om{};
	shader = effect->compileShader({}, rs, ds, om);
}

void HUDTextRenderer::renderText(
	CommandBuffer &cmdBuf,
	util::string_ref str,
	const Font &font,
	glm::vec2 viewPos,
	glm::vec2 viewportSize,
	const glm::vec4 &color,
	const glm::vec4 &outlineColor)
{
	auto len = str.size();
	if (len > kMaxGlyphsPerCall) 
		len = kMaxGlyphsPerCall;

	auto &vb_stream = Renderer::allocTransientBuffer(BufferUsage::VertexBuffer, sizeof(GlyphVertex) * len * 4);
	auto &ib_stream = Renderer::allocTransientBuffer(BufferUsage::IndexBuffer, sizeof(uint16_t) * len * 6);
	auto &cb_stream = Renderer::allocTransientBuffer(BufferUsage::ConstantBuffer, sizeof(TextParams));
	auto vbuf = vb_stream.map_as<GlyphVertex>();
	auto ibuf = ib_stream.map_as<uint16_t>();

	auto &metrics=font.getMetrics();

	int x=0,y=0;
	for (unsigned i = 0; i < len; ++i) 
	{
		const Font::Glyph *g = nullptr;
		// TODO: real utf-8
		if (!font.getGlyph(static_cast<char32_t>(str[i]),g)) {
			WARNING<<"No glyph for character "<<str[i];
		}
		int curx=x+g->xOffset;
		int cury=y+g->yOffset;
		int botx=curx+g->width;
		int boty=cury+g->height;
		vbuf[i*4].x=float(curx); // A
		vbuf[i*4].y=float(cury);
		vbuf[i*4].tx=float(g->x)/metrics.scaleW;
		vbuf[i*4].ty=float(g->y)/metrics.scaleH;
		vbuf[i*4+1].x=float(botx); // B
		vbuf[i*4+1].y=float(cury);
		vbuf[i*4+1].tx=float(g->x+g->width)/metrics.scaleW;
		vbuf[i*4+1].ty=float(g->y)/metrics.scaleH;
		vbuf[i*4+2].x=float(curx); // C
		vbuf[i*4+2].y=float(boty);
		vbuf[i*4+2].tx=float(g->x)/metrics.scaleW;
		vbuf[i*4+2].ty=float(g->y+g->height)/metrics.scaleH;
		vbuf[i*4+3].x=float(botx); // D
		vbuf[i*4+3].y=float(boty);
		vbuf[i*4+3].tx=float(g->x+g->width)/metrics.scaleW;
		vbuf[i*4+3].ty=float(g->y+g->height)/metrics.scaleH;
		ibuf[i*6]=i*4;
		ibuf[i*6+1]=i*4+1;
		ibuf[i*6+2]=i*4+3;
		ibuf[i*6+3]=i*4;
		ibuf[i*6+4]=i*4+3;
		ibuf[i*6+5]=i*4+2;
		x+=g->xAdvance;
	}

	auto p = cb_stream.map_as<TextParams>();
	p->transform = glm::ortho(
		-viewPos.x, 
		viewportSize.x - viewPos.x,
		viewportSize.y - viewPos.y,
		-viewPos.y);
	p->fillColor = color;
	p->outlineColor = outlineColor;

	cmdBuf.setShader(shader.get());
	cmdBuf.setVertexBuffers({ &vb_stream }, *layout);
	cmdBuf.setConstantBuffers({ &cb_stream });
	cmdBuf.setTextures({ &font.getTexture() }, { Renderer::getSampler_LinearClamp() });
	cmdBuf.drawIndexed(PrimitiveType::Triangle, ib_stream, 0, 0, len * 6, 0, 1);
}
