#ifndef INPUT_HPP
#define INPUT_HPP

#include <GLFW/glfw3.h>

// Polls keyboards, mouse and joysticks for key states
// Handles mapping between keys and actions
namespace input
{
	// key: GLFW identifier
	int getKeyState(int key);
	// button: GLFW mouse button identifier
	int getMouseButtonState(int button);

	enum Axis : int
	{
		Axis_X = 0, 
		Axis_Y,
		Axis_Z,
		Axis_RotZ,
		Axis_HatX,
		Axis_HatY
	};

	float getAxis(int axisId);
}

#endif /* end of include guard: INPUT_HPP */