#ifndef EFFECT_HPP
#define EFFECT_HPP

#include <renderer_common.hpp>
#include <parameter.hpp>

template <typename Backend>
class EffectBase : public RendererObject<typename Backend::EffectImpl>
{
public:
	// nullable
	EffectBase() = default;
	// ctor
	EffectBase(Impl impl_) : public RendererObject<typename Backend::EffectImpl>(impl_)
	{}
	// noncopyable
	EffectBase(const EffectBase<Backend> &) = delete;
	EffectBase<Backend> &operator=(const EffectBase<Backend> &) = delete;
	// moveable
	EffectBase(EffectBase<Backend> &&rhs) {
		*this = std::move(rhs);
	}
	EffectBase &operator=(EffectBase<Backend> &&rhs) {
		std::swap(impl, rhs.impl);
		return *this;
	}
	~EffectBase() {
		Backend::getInstance().deleteEffect(impl);
	}

	EffectBase(
		const char *combinedShaderSource,
		const char *includePath,
		RasterizerDesc rasterizerState,
		DepthStencilDesc depthStencilState
		) : RendererObject<typename Backend::EffectImpl>(
			Backend::getInstance().createEffect(
				combinedShaderSource, 
				includePath, 
				rasterizerState, 
				depthStencilState))
	{
	}

	// TODO parameter by names?
	ParameterBase<Backend> getParameter(const char *name) {
		return ParameterBase<Backend>(Backend::getInstance().createEffectParameter(impl, name));
	}
};

#endif /* end of include guard: EFFECT_HPP */