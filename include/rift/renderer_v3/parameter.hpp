#ifndef PARAMETER_HPP
#define PARAMETER_HPP

#include <renderer_common.hpp>

// represents an effect parameter
template <typename Backend>
class ParameterBase : public RendererObject<typename Backend::ParameterImpl>
{
public:
	ParameterBase() = default;
	ParameterBase(const ParameterBase &) = delete;
	ParameterBase(Impl impl_) : RendererObject<typename Backend::ParameterImpl>(impl_)
	{}
	ParameterBase &operator=(const ParameterBase &) = delete;
	ParameterBase(ParameterBase &&rhs) {
		*this = std::move(rhs);
	}
	ParameterBase &operator=(ParameterBase &&rhs) {
		std::swap(impl, rhs.impl);
		return *this;
	}
	~ParameterBase() {
		Backend::getInstance().deleteEffectParameter(impl);
	}

	int getSize() const {
		// TODO
	}
};


template <typename Backend>
class TextureParameterBase : public RendererObject<typename Backend::TextureParameterImpl>
{
public:
	TextureParameterBase();
	TextureParameterBase(const TextureParameterBase &) = delete;
	TextureParameterBase(Impl impl_) : impl(impl_)
	{}
	TextureParameterBase &operator=(const TextureParameterBase &) = delete;
	TextureParameterBase(TextureParameterBase &&rhs) {
		*this = std::move(rhs);
	}
	TextureParameterBase &operator=(TextureParameterBase &&rhs) {
		std::swap(impl, rhs.impl);
		return *this;
	}
	~TextureParameterBase() {
		Backend::getInstance().deleteEffectTextureParameter(impl);
	}
};


#endif /* end of include guard: PARAMETER_HPP */