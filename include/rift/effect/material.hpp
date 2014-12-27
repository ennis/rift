#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <renderer.hpp>
#include <effect.hpp>
#include <renderable.hpp>

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
// 
// classe Material
// Gère la technique de rendu d'un objet
// En d'autres termes, la classe Material contient une partie de l'état du pipeline
// graphique: les shaders à utiliser et les 'render states' (alpha blending, depth test, culling, etc.)
//
// Les Materials ont une référence vers un 'Effect' qui contient les shaders
// et possèdent un dictionnaire de paramètres qui correspond aux paramètres de l'Effect
// Normalement, il n'y a qu'une seule copie d'un Material pour chaque objet.
// Les effects peuvent cependant être partagés entre plusieurs Materials.
//
// Pour utiliser un Material, il suffit d'appeller Material::setup(Renderer &renderer)
// 
class Material
{
public:
	Material() : mEffect(nullptr)
	{
		mSamplers.fill(nullptr);
	}

	Material(Effect &effect) : mEffect(&effect)
	{
		mSamplers.fill(nullptr);
	}

	Effect *getEffect() const 
	{
		return mEffect;
	}

	void setEffect(Effect &effect)
	{
		mEffect = &effect;
	}

	void setParameter(const char *name, float value)
	{
		auto &ent = addParameter(name, EffectParameterType::Float);
		ent.u.f32[0] = value;
	}

	void setParameter(const char *name, glm::vec2 value)
	{
		auto &ent = addParameter(name, EffectParameterType::Float2);
		ent.u.f32[0] = value.x;
		ent.u.f32[1] = value.y;
	}

	void setParameter(const char *name, glm::vec3 value)
	{
		auto &ent = addParameter(name, EffectParameterType::Float3);
		ent.u.f32[0] = value.x;
		ent.u.f32[1] = value.y;
		ent.u.f32[2] = value.z;
	}

	void setParameter(const char *name, glm::vec4 value)
	{
		auto &ent = addParameter(name, EffectParameterType::Float4);
		ent.u.f32[0] = value.x;
		ent.u.f32[1] = value.y;
		ent.u.f32[2] = value.z;
		ent.u.f32[3] = value.w;
	}

	void setParameter(const char *name, int value)
	{
		auto &ent = addParameter(name, EffectParameterType::Int);
		ent.u.i32[0] = value;
	}

	void setParameter(const char *name, glm::ivec2 value)
	{
		auto &ent = addParameter(name, EffectParameterType::Int2);
		ent.u.i32[0] = value[0];
		ent.u.i32[1] = value[1];
	}

	void setParameter(const char *name, glm::ivec3 value)
	{
		auto &ent = addParameter(name, EffectParameterType::Int3);
		ent.u.i32[0] = value[0];
		ent.u.i32[1] = value[1];
		ent.u.i32[2] = value[2];
	}

	void setParameter(const char *name, glm::ivec4 value)
	{
		auto &ent = addParameter(name, EffectParameterType::Int4);
		ent.u.i32[0] = value[0];
		ent.u.i32[1] = value[1];
		ent.u.i32[2] = value[2];
		ent.u.i32[3] = value[3];
	}

	void setParameter(const char *name, glm::mat3x4 value)
	{
		auto &ent = addParameter(name, EffectParameterType::Float3x4);
		auto ptr = glm::value_ptr(value);
		for (int i = 0; i < 12; ++i) {
			ent.u.f32[i] = ptr[i];
		}
	}

	void setParameter(const char *name, glm::mat4 value)
	{
		auto &ent = addParameter(name, EffectParameterType::Float4x4);
		auto ptr = glm::value_ptr(value);
		for (int i = 0; i < 16; ++i) {
			ent.u.f32[i] = ptr[i];
		}
	}

	void setTexture(int textureUnit, Texture *texture)
	{
		assert(0 <= textureUnit && textureUnit < 16);
		mSamplers[textureUnit] = texture;
	}

	// for each parameter, call setNamedConstantXXX
	// for each sampler, call setTexture
	void setup(Renderer &renderer) const;

private:
	struct Parameter
	{
		Parameter(EffectParameterType type_) : type(type_)
		{}

		~Parameter()
		{}

		std::string name;
		EffectParameterType type;

		union U {
			// integer values
			std::array<int32_t, 16> i32;
			// float values
			std::array<float, 16> f32;
		} u;
	};

	Parameter &addParameter(const char *name, EffectParameterType type)
	{
		auto ent = mParameters.insert(std::make_pair(std::string(name), Parameter(type)));
		assert(ent.first->second.type == type);
		return ent.first->second;
	}

	// associated effect
	Effect *mEffect;
	// parameter dictionary
	std::unordered_map<std::string, Parameter> mParameters;
	// textures
	std::array<Texture*, 16> mSamplers/* = { { nullptr } }*/;
};


#endif