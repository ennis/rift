#ifndef SKY_HPP
#define SKY_HPP

#include <renderer.hpp>
#include <renderable.hpp>
#include <mesh.hpp>

class Sky
{
public:
	Sky(Renderer &renderer);
	virtual ~Sky();

	void setTimeOfDay(float hour);

	void render(RenderContext const &renderContext);

protected:
	Renderer &mRenderer;
	float mTimeOfDay;
	// unique_ptr OR resource_ptr
	Shader *mSkyShader;
	Mesh mSkybox;
};

#endif