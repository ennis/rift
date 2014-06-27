#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <renderresource.hpp>
#include <texture.hpp>


struct MaterialDesc
{
	CTexture2DRef diffuseMap;
	CTexture2DRef specularMap;
	CTexture2DRef normalMap;
	CTexture2DRef roughnessMap;
};

struct CMaterial : public CRenderResource
{
	virtual ~CMaterial() = 0
	{}

	MaterialDesc mDesc;
};

#endif