#ifndef GRAPHICS_CONTEXT_HPP
#define GRAPHICS_CONTEXT_HPP

#include <rendering/opengl4/opengl4.hpp>

namespace gl4
{
	class GraphicsContext
	{
	public:
		void initialize();
		void beginFrame();
		void execute(CommandBuffer &cmdBuf);
		void endFrame(); 
		void tearDown();

		Sampler *getSampler_LinearClamp();
		Sampler *getSampler_NearestClamp();
		Sampler *getSampler_LinearRepeat();
		Sampler *getSampler_NearestRepeat();

		Buffer::Ptr allocBuffer(BufferUsage bufferUsage, size_t size, const void *initialData = nullptr);
		Buffer *allocTransientBuffer(BufferUsage bufferUsage, size_t size, const void *initialData = nullptr);

		template <typename T>
		TTransientBuffer<T> allocTransientBuffer()
		{
			return TTransientBuffer<T> { allocTransientBuffer(BufferUsage::ConstantBuffer, sizeof(T)) };
		}

		unsigned getFrameCounter()
		{
			return frame_counter;
		}

	protected:
		Sampler::Ptr sampler_LinearClamp;
		Sampler::Ptr sampler_NearestClamp;
		Sampler::Ptr sampler_LinearRepeat;
		Sampler::Ptr sampler_NearestRepeat;	
		GLuint dummy_vao;
		unsigned frame_counter;	
	};
}

 
#endif /* end of include guard: GRAPHICS_CONTEXT_HPP */