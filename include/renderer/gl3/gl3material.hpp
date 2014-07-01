#ifndef GL3MATERIAL_HPP
#define GL3MATERIAL_HPP

#include <material.hpp>

struct CGL3Material : public CMaterial
{
	CGL3Material(MaterialDesc &desc);
	
	void destroy() override;
};

#endif