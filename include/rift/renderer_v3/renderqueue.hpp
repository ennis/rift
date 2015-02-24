#ifndef RENDERQUEUE_HPP
#define RENDERQUEUE_HPP

#include <renderer_common.hpp>
#include <parameter_block.hpp>

template <typename Backend>
class RenderQueueBase : public RendererObject<typename Backend::RenderQueueImpl>
{
public:
	// nullable
	RenderQueueBase() = default;
	// ctor
	RenderQueueBase(Impl impl_) : RendererObject<typename Backend::RenderQueueImpl>(impl_)
	{}
	// noncopyable
	RenderQueueBase(const RenderQueueBase<Backend> &) = delete;
	RenderQueueBase<Backend> &operator=(const RenderQueueBase<Backend> &) = delete;
	// moveable
	RenderQueueBase(RenderQueueBase<Backend> &&rhs) {
		*this = std::move(rhs);
	}
	RenderQueueBase &operator=(RenderQueueBase<Backend> &&rhs) {
		std::swap(impl, rhs.impl);
		return *this;
	}
	~RenderQueueBase() {
		Backend::getInstance().deleteRenderQueue(impl);
	}

	void draw(
		const MeshBase<Backend> &mesh,
		int submeshIndex,
		const EffectBase<Backend> &effect,
		const ParameterBlockBase<Backend> &parameterBlock,
		uint64_t sortHint
		)
	{
		Backend::getInstance().draw(
			impl,
			mesh.getImpl(),
			submeshIndex,
			effect.getImpl(),
			parameterBlock.getImpl(),
			sortHint);
	}

	void clear()
	{
		Backend::getInstance().clearRenderQueue(impl);
	}
};

 
#endif /* end of include guard: RENDERQUEUE_HPP */