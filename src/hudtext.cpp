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
		4*sizeof(float), /*elemSize*/ 
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
		VertexElement(0, 0, 0, 4*sizeof(float), ElementFormat::Float4)
	};
	mLayout = mRenderer->createVertexLayout(1, layout);
}

void HUDTextRenderer::renderString(
	RenderContext &renderContext,
	const char *str,
	Font *font,
	glm::vec2 viewPos, 
	glm::vec4 const &color,
	glm::vec4 const &outlineColor)
{
	auto len = strlen(str);
	// TODO do not truncate
	if (len > kMaxNumGlyphs) len = kMaxNumGlyphs;
	struct {
		float x, y;
		float tx, ty;
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
		vbuf[i*4].x = float(curx); // A
		vbuf[i*4].y = float(cury);
		vbuf[i*4].tx = float(g->x) / metrics.scaleW;
		vbuf[i*4].ty = float(g->y) / metrics.scaleH;
		vbuf[i*4+1].x = float(botx); // B
		vbuf[i*4+1].y = float(cury);
		vbuf[i*4+1].tx = float(g->x + g->width) / metrics.scaleW;
		vbuf[i*4+1].ty = float(g->y) / metrics.scaleH;
		vbuf[i*4+2].x = float(curx); // C
		vbuf[i*4+2].y = float(boty);
		vbuf[i*4+2].tx = float(g->x) / metrics.scaleW;
		vbuf[i*4+2].ty = float(g->y + g->height) / metrics.scaleH;
		vbuf[i*4+3].x = float(botx); // D
		vbuf[i*4+3].y = float(boty);
		vbuf[i*4+3].tx = float(g->x + g->width) / metrics.scaleW;
		vbuf[i*4+3].ty = float(g->y + g->height) / metrics.scaleH;
		ibuf[i*6  ] = i*4; 
		ibuf[i*6+1] = i*4+1; 
		ibuf[i*6+2] = i*4+3; 
		ibuf[i*6+3] = i*4; 
		ibuf[i*6+4] = i*4+3; 
		ibuf[i*6+5] = i*4+2;
		x += g->xAdvance;
	}
	mRenderer->updateBuffer(mVB,0,len*4*4*sizeof(float),vbuf);
	mRenderer->updateBuffer(mIB,0,len*6*sizeof(uint16_t),ibuf);
	mRenderer->setShader(mShader);
	auto transform = glm::ortho(0.f,renderContext.pfsp.viewportSize.x,renderContext.pfsp.viewportSize.y,0.f) *
		Transform().move(glm::vec3(viewPos,0.0f)).toMatrix();
	mRenderer->setNamedConstantMatrix4("uTransform", transform);
	// FIXME assume that all glyphs are on the same texture
	// (they should be, really)
	// TODO blend states
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ZERO);
	mRenderer->setTexture(0, font->getTexture(0));
	mRenderer->setNamedConstantFloat4("uFillColor",color);
	mRenderer->setNamedConstantFloat4("uOutlineColor",outlineColor);
	mRenderer->setVertexBuffer(0,mVB);
	mRenderer->setIndexBuffer(mIB);
	mRenderer->setVertexLayout(mLayout);
	mRenderer->drawIndexed(PrimitiveType::Triangle,0,len*4,0,len*6);
	glDepthMask(GL_TRUE);
}

