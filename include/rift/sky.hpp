#ifndef SKY_HPP
#define SKY_HPP

#include <renderer.hpp>
#include <renderable.hpp>
#include <meshbuffer.hpp>

class Sky
{
public:
	Sky();
	virtual ~Sky();

	void setTimeOfDay(float hour);

	void render(RenderContext const &renderContext);

protected:
	float mTimeOfDay;
	// unique_ptr OR resource_ptr
	Shader *mSkyShader;
	MeshBuffer mSkybox;
};

#endif