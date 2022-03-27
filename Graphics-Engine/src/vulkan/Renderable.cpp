#include "Renderable.hpp"
#include "../utils/Logger.hpp"

namespace Vk
{
	Renderable::Renderable(const Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, 
		const VkCommandPool commandPool)
		:vertexBuffer(std::make_unique<Buffer>(device, vertices, commandPool)), indexBuffer(std::make_unique<Buffer>(device, indices, commandPool))
	{

	}

	Renderable::~Renderable() noexcept
	{

	}

	const Buffer& Renderable::getVertexBuffer() const noexcept
	{
		return *vertexBuffer;
	}

	const Buffer& Renderable::getIndexBuffer() const noexcept
	{
		return *indexBuffer;
	}

	glm::mat4 Transform::getModel() const noexcept
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		return glm::mat4 {
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{position.x, position.y, position.z, 1.0f}
		};
	}

}
