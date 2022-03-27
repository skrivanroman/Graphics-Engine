#pragma once

#include "Renderable.hpp"

namespace Vk
{
	class Cube : public Renderable
	{
	public:
		Cube(const Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, 
			const VkCommandPool commandPool);
		~Cube() noexcept;

		Cube(Cube&&) = default;

		void draw(VkCommandBuffer commandBuffer, const VkPipelineLayout pipelineLayout) const override;

		static Cube createCube(const Device& device, const glm::vec3& dimensions, const glm::vec3& position, const glm::vec3& color,
			const VkCommandPool commandPool);
	private:
		glm::vec3 dimensions, color;
	};
}
