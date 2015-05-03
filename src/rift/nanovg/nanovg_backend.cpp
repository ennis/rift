#include <nanovg/nanovg_backend.hpp>
#include <nanovg/nanovg.h>
#include <log.hpp>
#include <gl4/effect.hpp>

namespace nvg { namespace detail {

	struct FillShaderParams
	{
		glm::vec2 viewSize;
		glm::mat3 scissorMat;
		glm::mat3 paintMat;
		glm::vec4 innerCol;
		glm::vec4 outerCol;
		glm::vec2 scissorExt;
		glm::vec2 scissorScale;
		glm::vec2 extent;
		float radius;
		float feather;
		float strokeMult;
		float strokeThr;
		int texType;
		int type;
	};

	int Renderer::renderCreate()
	{
		LOG << "renderCreate";
		auto effect = gl4::Effect::loadFromFile("resources/shaders/nanovg/nanovg.glsl");

		StencilOpDesc frontOp = StencilOpDesc{ StencilOp::Keep, StencilOp::Keep, StencilOp::Increment, StencilFunc::Always };
		StencilOpDesc backOp = StencilOpDesc{ StencilOp::Keep, StencilOp::Keep, StencilOp::Decrement, StencilFunc::Always };
		StencilOpDesc aaOp = StencilOpDesc{ StencilOp::Keep, StencilOp::Keep, StencilOp::Keep, StencilFunc::Equal };
		StencilOpDesc fillOp = StencilOpDesc{ StencilOp::Zero, StencilOp::Zero, StencilOp::Zero, StencilFunc::NotEqual };

		// stencil write pass
		DepthStencilDesc ds;
		ds.depthTestEnable = false;
		ds.depthWriteEnable = false;
		ds.stencilTestEnable = true;
		ds.stencilMask = 0xFF;
		ds.stencilOpFront = frontOp;
		ds.stencilOpBack = backOp;
		stencilPass = effect->compileShader({}, RasterizerDesc{}, ds);

		// AA pass
		ds.stencilOpFront = aaOp;
		ds.stencilOpBack = aaOp;
		aaPass = effect->compileShader({}, RasterizerDesc{}, ds);

		// Stencil Fill pass
		ds.stencilOpFront = fillOp;
		ds.stencilOpBack = fillOp;
		fillPass = effect->compileShader({}, RasterizerDesc{}, ds);

		// default pass
		ds.stencilTestEnable = false;
		defaultPass = effect->compileShader({}, RasterizerDesc{}, ds);

		inputLayout = InputLayout::create(1, { Attribute{ ElementFormat::Float2 }, Attribute{ ElementFormat::Float2 } });
		return 1;
	}

	int Renderer::renderCreateTexture(int type, int w, int h, int imageFlags, const unsigned char* data)
	{
		LOG << "renderCreateTexture " << type << ' ' << w << ' ' << h << ' ' << imageFlags;
		return 1;
	}

	int Renderer::renderDeleteTexture(int image)
	{
		LOG << "renderDeleteTexture " << image;
		return 1;
	}

	int Renderer::renderUpdateTexture(int image, int x, int y, int w, int h, const unsigned char* data)
	{
		LOG << "renderUpdateTexture " << image << ' ' << x << ' ' << y << ' ' << w << ' ' << h;
		return 1;
	}

	int Renderer::renderGetTextureSize(int image, int* w, int* h)
	{
		LOG << "renderGetTextureSize " << image;
		return 1;
	}

	void Renderer::renderViewport(int width, int height)
	{
		LOG << "renderViewport " << width << ' ' << height;
	}

	void Renderer::renderCancel()
	{
		LOG << "renderCancel";
	}

	void Renderer::renderFlush()
	{
		LOG << "renderFlush";
	}

	void Renderer::renderFill(
		NVGpaint* paint, 
		NVGscissor* scissor, 
		float fringe, 
		const float* bounds, 
		const NVGpath* paths, 
		int npaths)
	{
		LOG << "renderFill";

		for (int i = 0; i < npaths; ++i)
		{
			auto p = paths[i];
			
		}
	}

	void Renderer::renderStroke(
		NVGpaint* paint, 
		NVGscissor* scissor, 
		float fringe, 
		float strokeWidth, 
		const NVGpath* paths, 
		int npaths)
	{
		LOG << "renderStroke";
	}

	void Renderer::renderTriangles(
		NVGpaint* paint, 
		NVGscissor* scissor, 
		const NVGvertex* verts, 
		int nverts)
	{
		LOG << "renderTriangles";
	}

	void Renderer::renderDelete()
	{
		LOG << "renderDelete";
	}

	namespace
	{
		int renderCreate(void* uptr)
		{
			return static_cast<Renderer*>(uptr)->renderCreate();
		}
		int renderCreateTexture(void* uptr, int type, int w, int h, int imageFlags, const unsigned char* data)
		{
			return static_cast<Renderer*>(uptr)->renderCreateTexture(type, w, h, imageFlags, data);
		}
		int renderDeleteTexture(void* uptr, int image)
		{
			return static_cast<Renderer*>(uptr)->renderDeleteTexture(image);
		}
		int renderUpdateTexture(void* uptr, int image, int x, int y, int w, int h, const unsigned char* data)
		{
			return static_cast<Renderer*>(uptr)->renderUpdateTexture(image, x, y, w, h, data);
		}
		int renderGetTextureSize(void* uptr, int image, int* w, int* h)
		{
			return static_cast<Renderer*>(uptr)->renderGetTextureSize(image, w, h);
		}
		void renderViewport(void* uptr, int width, int height)
		{
			return static_cast<Renderer*>(uptr)->renderViewport(width, height);
		}
		void renderCancel(void* uptr)
		{
			return static_cast<Renderer*>(uptr)->renderCancel();
		}
		void renderFlush(void* uptr)
		{
			return static_cast<Renderer*>(uptr)->renderFlush();
		}
		void renderFill(void* uptr, NVGpaint* paint, NVGscissor* scissor, float fringe, const float* bounds, const NVGpath* paths, int npaths)
		{
			return static_cast<Renderer*>(uptr)->renderFill(paint, scissor, fringe, bounds, paths, npaths);
		}
		void renderStroke(void* uptr, NVGpaint* paint, NVGscissor* scissor, float fringe, float strokeWidth, const NVGpath* paths, int npaths)
		{
			return static_cast<Renderer*>(uptr)->renderStroke(paint, scissor, fringe, strokeWidth, paths, npaths);
		}
		void renderTriangles(void* uptr, NVGpaint* paint, NVGscissor* scissor, const NVGvertex* verts, int nverts)
		{
			return static_cast<Renderer*>(uptr)->renderTriangles(paint, scissor, verts, nverts);
		}
		void renderDelete(void* uptr)
		{
			delete static_cast<Renderer*>(uptr);
		}
	}

	NVGcontext* createContext()
	{
		NVGparams params;
		auto renderer = new Renderer();

		memset(&params, 0, sizeof(params));
		params.renderCreate = renderCreate;
		params.renderCreateTexture = renderCreateTexture;
		params.renderDeleteTexture = renderDeleteTexture;
		params.renderUpdateTexture = renderUpdateTexture;
		params.renderGetTextureSize = renderGetTextureSize;
		params.renderViewport = renderViewport;
		params.renderCancel = renderCancel;
		params.renderFlush = renderFlush;
		params.renderFill = renderFill;
		params.renderStroke = renderStroke;
		params.renderTriangles = renderTriangles;
		params.renderDelete = renderDelete;
		params.userPtr = renderer;
		params.edgeAntiAlias = 1;

		NVGcontext* ctx = nvgCreateInternal(&params);
		if (!ctx)
		{
			// 'renderer' is freed by nvgDeleteInternal.
			return nullptr; 
		}

		return ctx;
	}

}}