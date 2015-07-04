#include <input.hpp>
#include <application.hpp>

#include <algorithm>

namespace input
{
	// key: GLFW identifier
	int getKeyState(int key)
	{
		auto app = GetApplication();
		return glfwGetKey(app->getWindow(), key);
	}

	// button: GLFW mouse button identifier
	int getMouseButtonState(int button)
	{
		auto app = GetApplication();
		return glfwGetMouseButton(app->getWindow(), button);
	}

	// Actually ZQSD
	float getAxisWASD(int axisId)
	{
		auto app = GetApplication();
		auto win = app->getWindow();
		switch (axisId)
		{
		case Axis_X:
			return 
				((glfwGetKey(app->getWindow(), GLFW_KEY_D) == GLFW_PRESS) ? 1.0f : 0.0f) + 
				((glfwGetKey(app->getWindow(), GLFW_KEY_Q) == GLFW_PRESS) ? -1.0f : 0.0f);
		case Axis_Y:
			return 
				((glfwGetKey(app->getWindow(), GLFW_KEY_Z) == GLFW_PRESS) ? 1.0f : 0.0f) + 
				((glfwGetKey(app->getWindow(), GLFW_KEY_S) == GLFW_PRESS) ? -1.0f : 0.0f);
		default:
			return 0.0f;
		}
	}

	float getAxisJoystick(int axisId)
	{
		int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
		int axesCount;
		auto axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
		if (axisId >= axesCount) 
			return 0.0f;
		return axes[axisId];
	}

	float getAxis(int axisId)
	{
		// TODO do not sum values
		return getAxisWASD(axisId) + getAxisJoystick(axisId);
	}

}