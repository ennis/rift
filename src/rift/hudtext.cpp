#include <hudtext.hpp>
#include <cstring>
#include <effect.hpp>
#include <transform.hpp>

HUDTextRenderer::HUDTextRenderer() 
{
	layout = InputLayout::create({ Attribute{ ElementFormat::Float4 } });
	vb = Buffer::create(kMaxNumGlyphs * 4 * 4 * sizeof(float), ResourceUsage::Dynamic, BufferUsage::VertexBuffer, nullptr);
	ib = Buffer::create(kMaxNumGlyphs * 6 * sizeof(uint16_t), ResourceUsage::Dynamic, BufferUsage::IndexBuffer, nullptr);
	auto effect = gl4::Effect::loadFromFile("resources/shaders/text.glsl");
	shader = effect->compileShader();
	pb = ParameterBlock::create(*shader);
	cbParams = ConstantBuffer::create(sizeof(Params), nullptr);
	pb->setConstantBuffer(0, *cbParams);
}

void HUDTextRenderer::renderString(
	util::string_ref str,
	const Font &font,
	glm::vec2 viewPos,
	const glm::vec4 &color,
	const glm::vec4 &outlineColor,
	RenderQueue &renderQueue,
	const SceneData &sceneData,
	const ConstantBuffer &sceneDataCB)
{
	auto len = str.size();
	// TODO do not truncate
	if (len>kMaxNumGlyphs) len=kMaxNumGlyphs;
	struct {
		float x,y;
		float tx,ty;
	} vbuf[kMaxNumGlyphs*4];
	uint16_t ibuf[kMaxNumGlyphs*6];
	auto &metrics=font.getMetrics();
	int x=0,y=0;
	for (unsigned int i=0; i<len; ++i) {
		Font::Glyph const *g=nullptr;
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
	vb->update(0, len * 4 * sizeof(float) * 4, vbuf);
	ib->update(0, len * 6 * sizeof(uint16_t), ibuf);

	Params p;
	p.transform = glm::ortho(0.f, sceneData.viewportSize.x, sceneData.viewportSize.y, 0.f);
	p.fillColor = color;
	p.outlineColor = outlineColor;
	cbParams->update(0, sizeof(p), &p);

	gl::Enable(gl::BLEND);
	gl::DepthMask(gl::FALSE_);
	gl::BlendEquationSeparate(gl::FUNC_ADD, gl::FUNC_ADD);
	gl::BlendFuncSeparate(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA, gl::ONE, gl::ZERO);

	pb->setTextureParameter(0, &font.getTexture(), SamplerDesc{});
	renderQueue.draw2(*vb, *ib, *layout, Submesh{ PrimitiveType::Triangle, 0, 0, len*4, len * 6 }, *shader, *pb, 0);

	gl::DepthMask(gl::TRUE_);
}

