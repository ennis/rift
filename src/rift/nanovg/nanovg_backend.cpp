#include <nanovg/nanovg_backend.hpp>
#include <nanovg/nanovg.h>
#include <log.hpp>
#include <gl4/shadercompiler.hpp>

namespace nvg { namespace backend {

	struct FillShaderParams
	{
		glm::mat3x4 scissorMat;
		glm::mat3x4 paintMat;
		glm::vec4 innerCol;
		glm::vec4 outerCol;
		glm::vec2 scissorExt;
		glm::vec2 scissorScale;
		glm::vec2 extent;
		glm::vec2 viewSize;
		float radius;
		float feather;
		float strokeMult;
		float strokeThr;
		int texType;
		int type;
	};

	namespace
	{
		int maxVertCount(const NVGpath* paths, int npaths)
		{
			int i, count = 0;
			for (i = 0; i < npaths; i++) {
				count += paths[i].nfill;
				count += paths[i].nstroke;
			}
			return count;
		}
	}

	int Renderer::renderCreate()
	{
		LOG << "renderCreate";
		auto src = gl4::loadShaderSource("resources/shaders/nanovg/nanovg.glsl");
		auto vs = gl4::compileShader(src.c_str(), "", ShaderStage::VertexShader, {});
		auto ps = gl4::compileShader(src.c_str(), "", ShaderStage::PixelShader, {});

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
		stencilPassPS = PipelineState::create(vs.get(), nullptr, ps.get(), RasterizerDesc{}, ds, BlendDesc{});

		// AA pass
		ds.stencilOpFront = aaOp;
		ds.stencilOpBack = aaOp;
		aaPassPS = PipelineState::create(vs.get(), nullptr, ps.get(), RasterizerDesc{}, ds, BlendDesc{});

		// Stencil Fill pass
		ds.stencilOpFront = fillOp;
		ds.stencilOpBack = fillOp;
		fillPassPS = PipelineState::create(vs.get(), nullptr, ps.get(), RasterizerDesc{}, ds, BlendDesc{});

		// default pass
		ds.stencilTestEnable = false;
		defaultPassPS = PipelineState::create(vs.get(), nullptr, ps.get(), RasterizerDesc{}, ds, BlendDesc{});

		layout = InputLayout::create(1, { Attribute{ ElementFormat::Float2 }, Attribute{ ElementFormat::Float2 } });
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
		//LOG << "renderViewport " << width << ' ' << height;
		viewportWidth = width;
		viewportHeight = height;
	}

	void Renderer::renderCancel()
	{
		LOG << "renderCancel";
	}

	void Renderer::renderFlush()
	{
		::Renderer::execute(cmdBuf);
		cmdBuf = {};
		//LOG << "renderFlush";
	}

	void Renderer::renderFill(
		NVGpaint* paint, 
		NVGscissor* scissor, 
		float fringe, 
		const float* bounds, 
		const NVGpath* paths, 
		int npaths)
	{
		//LOG << "renderFill";
		// allocate space for the vertices
		auto maxv = 0u;
		for (auto i = 0u; i < npaths; ++i)
		{
			maxv += paths[i].nfill * 2 - 3;
		}
		auto &buf = ::Renderer::allocTransientBuffer(BufferUsage::VertexBuffer, sizeof(Vertex) * maxv, nullptr);
		auto vptr = buf.map_as<Vertex>();
		unsigned voffset = 0;
		auto &cb = ::Renderer::allocTransientBuffer(BufferUsage::ConstantBuffer, sizeof(FillShaderParams), nullptr);
		auto cbPtr = cb.map_as<FillShaderParams>();
		cbPtr->type = 2;
		cbPtr->viewSize = glm::vec2(viewportWidth, viewportHeight);
		cmdBuf.setScreenRenderTarget();
		cmdBuf.setVertexBuffers({ &buf }, *layout);
		cmdBuf.setConstantBuffers({ &cb });
		cmdBuf.setPipelineState(stencilPassPS.get());
		for (int i = 0; i < npaths; ++i)
		{
			auto p = paths[i];
			auto start = voffset;
			// convert fan to triangle strip
			for (int v = 1; v < p.nfill; ++v, ++voffset)
			{
				vptr[voffset].x = p.fill[v].x;
				vptr[voffset].y = p.fill[v].y;
				vptr[voffset].u = p.fill[v].u;
				vptr[voffset].v = p.fill[v].v;
				if (v != p.nfill) {
					++voffset;
					vptr[voffset].x = p.fill[0].x;
					vptr[voffset].y = p.fill[0].y;
					vptr[voffset].u = p.fill[0].u;
					vptr[voffset].v = p.fill[0].v;
				}
			}
			cmdBuf.draw(PrimitiveType::TriangleStrip, start, voffset - start, 0, 1);
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