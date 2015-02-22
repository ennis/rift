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
	EffectBase(Impl impl_) : impl(impl_)
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
		) : impl(
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

private:
	Impl impl;
};

#endif /* end of include guard: EFFECT_HPP */