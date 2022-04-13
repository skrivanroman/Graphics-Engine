#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <array>
#include <optional>

class KeyboardMouse
{
public:
	KeyboardMouse(float moveSensitivity, float lookSensitivity);
	KeyboardMouse(const std::array<float, 6>& moveSensitivity, float lookSensitivity);

	std::optional<glm::vec3> getUpdate(GLFWwindow* window, float deltaTime, float yaw) const noexcept;

private:
	std::array<float, 6> moveSensitivity;
	float lookSensitivity;
};

