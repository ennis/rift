#include <freecameracontrol.hpp>
#include <engine.hpp>
#include <common.hpp>

void FreeCameraController::updateViewport(Camera *camera, glm::vec2 windowSize)
{
	// setAspectRatio
	camera->setAspectRatio(windowSize.x / windowSize.y);
}

void FreeCameraController::init()
{
	mCamera = getEntity()->getComponent<Camera>();
}

void FreeCameraController::update(float dt)
{
	using namespace glm;

	auto windowHandle = Engine::instance().getWindow().getHandle();
	auto &transform = getEntity()->getTransform();

	int width, height;
	glfwGetWindowSize(windowHandle, &width, &height);
	updateViewport(mCamera, glm::vec2(float(width), float(height)));

	// handle mouse input
	double cursorX, cursorY;
	glfwGetCursorPos(windowHandle, &cursorX, &cursorY);
	float dx = (float(cursorX) - mLastMousePos.x) / 100.f;
	float dy = (float(cursorY) - mLastMousePos.y) / 100.f;

	if (abs(dx) > abs(dy)) {
		dy = 0;
	}
	else {
		dx = 0;
	}

	// TODO utiliser l'input manager
	bool mouseRight = glfwGetMouseButton(windowHandle, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
	bool mouseLeft = glfwGetMouseButton(windowHandle, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS;

	if ((mouseRight || mouseLeft) && (dx != 0 || dy != 0)) 
	{
		float orbit_dist = distance(transform.position, vec3(0, 0, 0));

		mPhi += dy;
		mTheta += dx;
		vec3 up = vec3(0, 1, 0);
		vec3 right = angleAxis(mTheta, up) * vec3(1, 0, 0);
		transform.rotation = angleAxis(mPhi, right) * angleAxis(mTheta, up);

		if (mouseLeft) {
			// orbit mode
			transform.position = transform.rotation * vec3(0, 0, -orbit_dist);
		}
	}

	mLastMousePos.x = float(cursorX);
	mLastMousePos.y = float(cursorY);

	vec3 front = transform.rotation * Camera::Front;
	vec3 right = transform.rotation * Camera::Right;

	// handle keyboard input
	int keyW = glfwGetKey(windowHandle, GLFW_KEY_Z);
	int keyA = glfwGetKey(windowHandle, GLFW_KEY_Q);
	int keyS = glfwGetKey(windowHandle, GLFW_KEY_S);
	int keyD = glfwGetKey(windowHandle, GLFW_KEY_D);

	if (keyW == GLFW_PRESS) {
		transform.position += 100 * dt * front;
	}
	else if (keyS == GLFW_PRESS) {
		transform.position -= 100 * dt * front;
	}
	if (keyA == GLFW_PRESS) {
		transform.position += 100 * dt * right;
	} 
	else if (keyD == GLFW_PRESS) {
		transform.position -= 100 * dt * right;
	}

}