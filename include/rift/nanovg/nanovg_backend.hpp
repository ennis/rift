#ifndef NANOVG_BACKEND_HPP
#define NANOVG_BACKEND_HPP

#include <gl4/renderer.hpp>
#include <vector>

struct NVGcontext;
struct NVGpaint;
struct NVGscissor;
struct NVGpath;
struct NVGvertex;

namespace nvg {
	namespace backend {

		class Renderer
		{
		public:
			int renderCreate();
			int renderCreateTexture(int type, int w, int h, int imageFlags, const unsigned char* data);
			int renderDeleteTexture(int image);
			int renderUpdateTexture(int image, int x, int y, int w, int h, const unsigned char* data);
			int renderGetTextureSize(int image, int* w, int* h);
			void renderViewport(int width, int height);
			void renderCancel();
			void renderFlush();

			void renderFill(
				NVGpaint* paint,
				NVGscissor* scissor,
				float fringe,
				const float* bounds,
				const NVGpath* paths,
				int npaths);

			void renderStroke(
				NVGpaint* paint,
				NVGscissor* scissor,
				float fringe,
				float strokeWidth,
				const NVGpath* paths,
				int npaths);

			void renderTriangles(
				NVGpaint* paint,
				NVGscissor* scissor,
				const NVGvertex* verts,
				int nverts);

			void renderDelete();

		private:
			PipelineState::Ptr stencilPassPS;
			PipelineState::Ptr aaPassPS;
			PipelineState::Ptr fillPassPS;
			PipelineState::Ptr defaultPassPS;
			InputLayout::Ptr layout;
			CommandBuffer cmdBuf;
			unsigned viewportWidth, viewportHeight;

			struct Vertex
			{
				float x, y, u, v;
			};
			std::vector<Vertex> vertices;
			struct DrawCall
			{
				bool fill;
				unsigned vstart;
				unsigned nvertex;
			};
			std::vector<DrawCall> calls;
		};

		NVGcontext* createContext();

	}
}
 
#endif /* end of include guard: NANOVG_BACKEND_HPP */