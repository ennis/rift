#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include <scene.hpp>
#include <entity.hpp>
#include <renderer2.hpp>
#include <renderqueue.hpp>

enum RenderableFlags
{
	RF_ShadowReceiver = (1 << 0),
	RF_ShadowCaster = (1 << 1),
	RF_Sky = (1 << 2),
	RF_Opaque = (1 << 3),
	RF_Transparent = (1 << 4),
	// Static: no need to call render more than once
	RF_Static = (1 << 5),
	RF_Terrain = (1 << 6)
};


// Renderable objects
// These objects have a render method
class Renderable : public IComponent<CID_Renderable>
{
public:
	Renderable(uint64_t flags_ = 0) : flags(flags_)
	{}

	virtual void render(
			RenderQueue &renderQueue, 
			const SceneData &data) = 0;

	uint64_t getFlags() const
	{
		return flags;
	}

protected:
	uint64_t flags;
	// could add an ID here, for submission caching
	std::vector<int> cachedSubmissions;
};

#endif