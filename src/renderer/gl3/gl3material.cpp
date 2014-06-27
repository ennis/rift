#include <gl3material.hpp>

CGL3Material::CGL3Material(MaterialDesc &desc)
{
	mDesc = desc;
}

void CGL3Material::destroy()
{
	delete this;
}