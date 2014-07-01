#ifndef RENDERRESOURCE_HPP
#define RENDERRESOURCE_HPP


class CRenderResource
{
public:
	// delete the resource
	virtual void destroy() = 0;
};

#endif