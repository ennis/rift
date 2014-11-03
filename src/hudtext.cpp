#include <hudtext.hpp>

HUDTextRenderer::HUDTextRenderer(Renderer &renderer): mRenderer(&renderer)
{
	init();
}

HUDTextRenderer::~HUDTextRenderer()
{
	// TODO release resources
}

void HUDTextRenderer::init()
{
	mVB = mRenderer->createVertexBuffer(
		4*sizeof(uint16_t), /*elemSize*/ 
		kMaxNumGlyphs*4, /*numVertices*/
		ResourceUsage::Dynamic, 
		nullptr);
	mIB = mRenderer->createIndexBuffer(
		2*sizeof(uint16_t),
		kMaxNumGlyphs*6, 
		ResourceUsage::Dynamic, 
		nullptr);
	mShader = mRenderer->createShader(
		loadShaderSource("resources/shaders/text/vert.glsl").c_str(),
		loadShaderSource("resources/shaders/text/frag.glsl").c_str());
	VertexElement layout[] = {
		VertexElement(0, 0, 0, 4*sizeof(int16_t), ElementFormat::Sint16x2),
		VertexElement(1, 0, 2*sizeof(uint16_t), 4*sizeof(uint16_t), ElementFormat::Unorm16x2),
	};
	mLayout = mRenderer->createVertexLayout(2, layout);
}

void HUDTextRenderer::renderString(
	RenderContext &renderContext,
	const char *str,
	Font *font,
	glm::ivec2 viewPos, 
	glm::vec4 const &color,
	glm::vec4 const &outlineColor)
{
	int len = strlen(str);
	// TODO do not truncate
	if (len > kMaxNumGlyphs) len = kMaxNumGlyphs;
	struct {
		int16_t x, y;
		uint16_t tx, ty;
	} vbuf[kMaxNumGlyphs*4];
	uint16_t ibuf[kMaxNumGlyphs*6];
	auto &metrics = font->metrics();
	int x = 0, y = 0;
	for (int i = 0; i < len; ++i) {
		Font::Glyph const *g = nullptr;
		if (!font->getGlyph(char32_t(str[i]), g)) {
			WARNING << "No glyph for character " << str[i];
		}
		// Triangles
		// A-B
		// | |
		// C-D
		// ABD-ADC
		int curx = x + g->xOffset;
		int cury = y + g->yOffset;
		int botx = curx + g->width;
		int boty = cury + g->height;
		vbuf[i*4].x = curx; // A
		vbuf[i*4].y = cury;
		vbuf[i*4].tx = g->x * 65535 / metrics.scaleW;
		vbuf[i*4].ty = g->y * 65535 / metrics.scaleH;
		vbuf[i*4+1].x = botx; // B
		vbuf[i*4+1].y = cury;
		vbuf[i*4+1].tx = (g->x + g->width) * 65535 / metrics.scaleW;
		vbuf[i*4+1].ty = g->y * 65535 / metrics.scaleH;
		vbuf[i*4+2].x = curx; // C
		vbuf[i*4+2].y = boty;
		vbuf[i*4+2].tx = g->x * 65535 / metrics.scaleW;
		vbuf[i*4+2].ty = (g->y + g->height) * 65535 / metrics.scaleH;
		vbuf[i*4+3].x = botx; // D
		vbuf[i*4+3].y = boty;
		vbuf[i*4+3].tx = (g->x + g->width) * 65535 / metrics.scaleW;
		vbuf[i*4+3].ty = (g->y + g->height) * 65535 / metrics.scaleH;
		ibuf[i*6  ] = i*4; 
		ibuf[i*6+1] = i*4+1; 
		ibuf[i*6+2] = i*4+3; 
		ibuf[i*6+3] = i*4; 
		ibuf[i*6+4] = i*4+3; 
		ibuf[i*6+5] = i*4+2;
		x += g->xAdvance;
	}
	mRenderer->updateBuffer(mVB, 0, len*4*4*sizeof(uint16_t), vbuf);
	mRenderer->updateBuffer(mIB, 0, len*6*sizeof(uint16_t), ibuf);
	mRenderer->setShader(mShader);
	mRenderer->setNamedConstantInt2("uViewSize", 
		glm::ivec2(
			renderContext.pfsp.viewportSize.x, 
			renderContext.pfsp.viewportSize.y));
	// FIXME assume that all glyphs are on the same texture
	// (they should be, really)
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	mRenderer->setTexture(0, font->getTexture(0));
	mRenderer->setNamedConstantFloat4("uFillColor", color);
	mRenderer->setNamedConstantFloat4("uOutlineColor", outlineColor);
	mRenderer->setVertexBuffer(0, mVB);
	mRenderer->setIndexBuffer(mIB);
	mRenderer->setVertexLayout(mLayout);
	mRenderer->drawIndexed(PrimitiveType::Triangle, 0, len * 4, 0, len * 6);
	glDepthMask(GL_TRUE);
}

