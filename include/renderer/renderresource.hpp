#ifndef RENDERRESOURCE_HPP
#define RENDERRESOURCE_HPP

#include <resource.hpp>

class CRenderResource : public CResourceBase
{
public:
	// delete the resource
	virtual void deleteResource() = 0;
};

#endif