#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Vk
{
	class Camera
	{
	public:
		Camera(const glm::vec3& position, const glm::vec3& target, float aspectRatio, float fieldOfView,
			float near = 0.1f, float far = 100.0f
		);
		
		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;
	
		void update();
		const glm::mat4& getViewProjection() const noexcept;

	private:
		glm::vec3 position, target, up;
		float aspectRatio, fieldOfView, near, far;
		glm::mat4 viewProjection;
	};
}
