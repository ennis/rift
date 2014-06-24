#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include <renderresource.hpp>
#include <transform.hpp>

class CRenderable : public CRenderResource
{
public:
	virtual void render(Transform &transform) = 0;
};


#endif