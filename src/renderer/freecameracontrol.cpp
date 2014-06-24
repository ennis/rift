#include <freecameracontrol.hpp>
#include <game.hpp>
#include <common.hpp>

void FreeCameraControl::updateViewport(Camera &camera, glm::vec2 windowSize)
{
	camera.aspectRatio = windowSize.x / windowSize.y;
}

void FreeCameraControl::update(Camera &camera, Transform &transform, float dt)
{
	using namespace glm;

	auto window = Game::instance().window();

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	updateViewport(camera, glm::vec2(static_cast<float>(width), static_cast<float>(height)));

	// handle mouse input
	double cursorX, cursorY;
	glfwGetCursorPos(window, &cursorX, &cursorY);
	float dx = (float(cursorX) - mLastMousePos.x) / 100.f;
	float dy = (float(cursorY) - mLastMousePos.y) / 100.f;

	if (abs(dx) > abs(dy)) {
		dy = 0;
	}
	else {
		dx = 0;
	}

	// TODO utiliser l'input manager
	bool mouseRight = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
	bool mouseLeft = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS;

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

	mLastMousePos.x = static_cast<float>(cursorX);
	mLastMousePos.y = static_cast<float>(cursorY);

	vec3 front = transform.rotation * Camera::Front;
	vec3 right = transform.rotation * Camera::Right;

	// handle keyboard input
	int keyW = glfwGetKey(window, GLFW_KEY_Z);
	int keyA = glfwGetKey(window, GLFW_KEY_Q);
	int keyS = glfwGetKey(window, GLFW_KEY_S);
	int keyD = glfwGetKey(window, GLFW_KEY_D);

	if (keyW == GLFW_PRESS) {
		transform.position += dt * front;
	}
	else if (keyS == GLFW_PRESS) {
		transform.position -= dt * front;
	}
	if (keyA == GLFW_PRESS) {
		transform.position += dt * right;
	} 
	else if (keyD == GLFW_PRESS) {
		transform.position -= dt * right;
	}

	camera.updateViewMatrix(transform);
	camera.updateProjectionMatrix(transform);
}