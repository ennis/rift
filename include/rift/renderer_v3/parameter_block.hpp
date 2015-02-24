#ifndef PARAMETER_BLOCK_HPP
#define PARAMETER_BLOCK_HPP

#include <renderer_common.hpp>
#include <effect.hpp>
#include <parameter.hpp>
#include <constant_buffer.hpp>

template <typename Backend>
class ParameterBlockBase : 
	public RendererObject<typename Backend::ParameterBlockImpl>
{
public:
	// nullable
	ParameterBlockBase() = default;
	// ctor
	// noncopyable
	ParameterBlockBase(const ParameterBlockBase<Backend> &) = delete;
	ParameterBlockBase<Backend> &operator=(const ParameterBlockBase<Backend> &) = delete;
	// moveable
	ParameterBlockBase(ParameterBlockBase<Backend> &&rhs) {
		*this = std::move(rhs);
	}
	ParameterBlockBase &operator=(ParameterBlockBase<Backend> &&rhs) {
		std::swap(impl, rhs.impl);
		return *this;
	}

	~ParameterBlockBase() {
		Backend::getInstance().deleteParameterBlock(impl);
	}

	ParameterBlockBase(EffectBase<Backend> &effect) :
		RendererObject<typename Backend::ParameterBlockImpl>(Backend::getInstance().createParameterBlock(effect.getImpl()))
	{
	}


	void setConstantBuffer(
		const ParameterBase<Backend> &param,
		const ConstantBufferBase<Backend> &constantBuffer
		)
	{
		Backend::getInstance().setConstantBuffer(
			impl,
			param.getImpl(),
			constantBuffer.getImpl());
	}

	/*void setTextureParameter(
		const TextureParameterBase<Backend> &param,
		const Texture2DBase<Backend> *texture,
		const SamplerDesc &samplerDesc
		)
	{
		Backend::getInstance().setTextureParameter(
			impl,
			param.getImpl(),
			texture.getImpl());
	}*/
};

 
#endif /* end of include guard: PARAMETER_BLOCK_HPP */