#include <material.hpp>

void Material::setup(Renderer &renderer) const
{
	// setup effect
	auto cs = mEffect->compileShader(renderer);
	cs->setup(renderer);

	for (auto &p : mParameters)
	{
		auto &pp = p.second;
		switch (pp.type)
		{
		case EffectParameterType::Float:
			renderer.setNamedConstantFloat(pp.name.c_str(), pp.u.f32[0]);
			break;
		case EffectParameterType::Float2:
			renderer.setNamedConstantFloat2(pp.name.c_str(), 
				glm::make_vec2(pp.u.f32.data()));
			break;
		case EffectParameterType::Float3:
			renderer.setNamedConstantFloat3(pp.name.c_str(), 
				glm::make_vec3(pp.u.f32.data()));
			break;
		case EffectParameterType::Float4:
			renderer.setNamedConstantFloat4(pp.name.c_str(), 
				glm::make_vec4(pp.u.f32.data()));
			break;
		case EffectParameterType::Int:
			renderer.setNamedConstantInt(pp.name.c_str(), pp.u.i32[0]);
			break;
		case EffectParameterType::Int2:
			renderer.setNamedConstantInt2(pp.name.c_str(), glm::ivec2(pp.u.i32[0], pp.u.i32[1]));
			break;
		case EffectParameterType::Int3:
			//renderer.setNamedConstantInt3(pp.name.c_str(), glm::ivec3(pp.u.i32[0], pp.u.i32[1], pp.u.i32[2]));
			break;
		case EffectParameterType::Int4:
			//renderer.setNamedConstantInt4(pp.name.c_str(), glm::ivec4(pp.u.i32[0], pp.u.i32[1], pp.u.i32[2], pp.u.i32[3]));
			break;
		case EffectParameterType::Float3x4:
			// TODO
			break;
		case EffectParameterType::Float4x4:
			renderer.setNamedConstantMatrix4(pp.name.c_str(), glm::make_mat4(pp.u.f32.data()));
			break;
		default:
			break;
		}
	}

	for (int i = 0; i < 16; ++i)
	{
		if (mSamplers[i])
			renderer.setTexture(i, mSamplers[i]);
	}
}