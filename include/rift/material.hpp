#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <renderer.hpp>

//=============================================================================
struct TextureState
{
	Texture *texture;
	TextureFilter minFilter;
	TextureFilter maxFilter;
	TextureAddressMode addressModeU;
	TextureAddressMode addressModeV;
	TextureAddressMode addressModeW;
};

//=============================================================================
class MaterialInstance : public Resource
{
public:
	friend class Material;

	virtual ~MaterialInstance();

	void size() const;
	void update(int offset, int size, const void *data);
	// TODO setNamedConstant
	void setTextureState(int slot, TextureState &state);

	// Material *mat = Material::loadFromFile(renderer, ...)
	// model.setMaterial(mat->createInstance().float3("", ...))
	//
	//
	//
	//

private:
	MaterialInstance(Material *material, int instanceID);

	Material *mMaterial;
	int mInstanceID;
	// FIXME max 16 tex units?
	TextureState mTextureState[16];
	// misc render states
	RenderState mRenderState;
};

//=============================================================================
// Code
// inputs
// outputs
// constants
// samplers
// entry point
// input type
// output type

//=============================================================================
// vertex shader
// texture shader
// lighting shader
class Material : public Resource
{
public:
	friend class MaterialInstance;

	Material();
	~Material();

	void setVertexShaderNode(ShaderNode *vertexShaderNode);
	MaterialInstance *createMaterialInstance();

	Shader *getShader() const {
		return mShader;
	}

	ConstantBuffer *getPerMaterialBuffer() const;
	void bindPerInstanceBuffer(Renderer &renderer, int instanceID, int targetSlot);

protected:
	void updateInstance(int instanceID, int offset, int size, const void *data);

	// Shader
	Shader *mShader;
	// FIXME shaders for multiple configurations
	// constant buffer for material parameters
	// FIXME may need multiple constant buffers (max 256 entries of 256 bytes)
	ConstantBuffer *mPerInstanceBuffer;
	ConstantBuffer *mPerMaterialBuffer;
};

#endif