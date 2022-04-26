#include <glm/gtc/matrix_transform.hpp>
#include "Camera.hpp"

namespace Vk
{
	Camera::Camera(const glm::vec3& position, const glm::vec3& target, float aspectRatio, float fieldOfView, float near, float far)
		:position(position), target(target), up({ 0.0f, 1.0f, 0.0f }), aspectRatio(aspectRatio), fieldOfView(fieldOfView), 
		near(near), far(far)
	{
		update();
	}

	void Camera::update()
	{
		viewProjection = glm::perspective(fieldOfView, aspectRatio, near, far) * glm::lookAt(position, target, up);
	}

	void Camera::move(const glm::vec3& change) noexcept
	{
		position += change;
	}

	void Camera::moveTarget(const glm::vec3& change) noexcept
	{
		target += change;
	}

	const glm::mat4& Camera::getViewProjection() const  noexcept
	{
		return viewProjection;
	}
}
