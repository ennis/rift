#include <rendering/scene_renderer.hpp>
#include <rendering/opengl4.hpp>
#include <cstring>
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

void SceneRenderer::drawTextVBO(Buffer *vb, Buffer *ib, unsigned len, const Font &font, glm::ivec2 pos, glm::vec4 fill, glm::vec4 outline)
{
	auto cb = graphicsContext.createTransientBuffer(gl::UNIFORM_BUFFER, sizeof(TextParams));
	auto p = (TextParams*)cb->ptr;
	p->transform = glm::ortho(
		-float(pos.x), 
		float(viewportSize.x) - float(pos.x),
		float(viewportSize.y) - float(pos.y),
		-float(pos.y));
	p->fillColor = fill;
	p->outlineColor = outline;
	gl::UseProgram(textProgram);
	gl::Disable(gl::CULL_FACE);
	gl::Disable(gl::DEPTH_TEST);
	bindVertexBuffers({ vb }, textVao);
	bindBuffersRangeHelper(0, { cb });
	GLuint samplers[] = {graphicsContext.getSamplerLinearClamp()};
	GLuint textures[] = {font.getTexture().getGL()};
	gl::BindTextures(0, 1, textures);
	gl::BindSamplers(0, 1, samplers);
	drawIndexed(gl::TRIANGLES, *ib, 0, 0, len * 6, 0, 1);
	gl::Enable(gl::DEPTH_TEST);
}

void SceneRenderer::drawTextShadow(glm::ivec2 pos, const char *str)
{
	Buffer *vb, *ib;
	unsigned len;
	makeTextVBO(*defaultFont, str, vb, ib, len);
	drawTextVBO(vb, ib, len, *defaultFont, pos + glm::ivec2(2, 2), glm::vec4(0.0, 0.0, 0.0, 1.0), glm::vec4(0.0, 0.0, 0.0, 0.0));
	drawTextVBO(vb, ib, len, *defaultFont, pos,                    glm::vec4(1.0, 1.0, 1.0, 1.0), glm::vec4(0.0, 0.0, 0.0, 0.0));
}

void SceneRenderer::makeTextVBO(Font &font, const char *str, Buffer *&vb, Buffer *& ib, unsigned &len)
{
	len = strlen(str);
	vb = graphicsContext.createTransientBuffer(gl::ARRAY_BUFFER, sizeof(GlyphVertex) * len * 4);
	ib = graphicsContext.createTransientBuffer(gl::ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * len * 6);
	auto vbuf = (GlyphVertex*)vb->ptr;
	auto ibuf = (uint16_t*)ib->ptr;
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
}

