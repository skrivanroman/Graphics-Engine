#include "KeyboardMouse.hpp"
#include "../utils/Logger.hpp"

KeyboardMouse::KeyboardMouse(float moveSensitivity, float lookSensitivity)
	:lookSensitivity(lookSensitivity)
{
	for (auto& sensitivity : this->moveSensitivity)
		sensitivity = moveSensitivity;
}

KeyboardMouse::KeyboardMouse(const std::array<float, 6>& moveSensitivity, float lookSensitivity)
	:moveSensitivity(moveSensitivity), lookSensitivity(lookSensitivity)
{
}

std::optional<glm::vec3> KeyboardMouse::getUpdate(GLFWwindow* window, float deltaTime, float yaw) const noexcept
{
	const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
	const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
	const glm::vec3 upDir{0.f, -1.f, 0.f};

	glm::vec3 moveDir{0.f};
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += forwardDir;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= forwardDir;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += rightDir;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= rightDir;
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) moveDir += upDir;
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) moveDir -= upDir;

	if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) 
	  return  moveSensitivity[0] * deltaTime * glm::normalize(moveDir);

	return {};
}
