#ifndef COLORS_HPP
#define COLORS_HPP

#include <common.hpp>

namespace Color {
	constexpr glm::vec4 Black(0.0, 0.0, 0.0, 1.0); 
	constexpr glm::vec4 White(1.0, 1.0, 1.0, 1.0);
	constexpr glm::vec4 Red(1.0, 0.0, 0.0, 1.0);
	constexpr glm::vec4 Green(0.0, 1.0, 0.0, 1.0);
	constexpr glm::vec4 Blue(0.0, 0.0, 1.0, 1.0);
	constexpr glm::vec4 Yellow(1.0, 1.0, 0.0, 1.0);
	constexpr glm::vec4 Cyan(0.0, 1.0, 1.0, 1.0);
	constexpr glm::vec4 Magenta(1.0, 0.0, 1.0, 1.0);
	constexpr glm::vec4 Grey(0.5, 0.5, 0.5, 1.0);
}
 
#endif /* end of include guard: COLORS_HPP */