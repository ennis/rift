#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include <entity.hpp>
#include <renderer.hpp>
#include <camera.hpp>

enum RenderableFlags
{
	RF_ShadowReceiver = (1 << 0),
	RF_ShadowCaster = (1 << 1),
	RF_Sky = (1 << 2),
	RF_Opaque = (1 << 3),
	RF_Transparent = (1 << 4),
	RF_Static = (1 << 5),
	RF_Terrain = (1 << 6)
};

enum class RenderPass
{
	Sky,
	Terrain,
	Deferred,
	Opaque,
	Transparent,
	Shadow
};

struct PerFrameShaderParameters
{
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	glm::mat4 viewProjMatrix;
	glm::vec4 eyePos;	// in world space
	glm::vec4 lightDir;
	glm::vec2 viewportSize;
};

struct RenderContext
{
	Renderer *renderer;
	Camera *camera;
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	RenderPass renderPass;
	PerFrameShaderParameters pfsp;
	ConstantBuffer *perFrameShaderParameters;
};


// Renderable objects
// These objects have a render method
class Renderable : public IComponent<CID_Renderable>
{
public:
	Renderable(Renderer &renderer) : mRenderer(renderer)
	{}

	virtual void render(RenderContext const &context) = 0;
	virtual void update(float dt) override {};

	uint64_t getFlags() const
	{
		return mFlags;
	}

	Renderer &getRenderer() const 
	{
		return mRenderer;
	}

protected:
	uint64_t mFlags;
	Renderer &mRenderer;
};

#endif