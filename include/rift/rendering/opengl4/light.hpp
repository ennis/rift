#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <rendering/opengl4/opengl4.hpp>

namespace gl4
{
	enum class LightMode
	{
		Directional,
		Spot,
		Point
	};

	struct Light
	{
		LightMode mode;
		glm::vec3 intensity;
	};
}
 
#endif /* end of include guard: LIGHT_HPP */
