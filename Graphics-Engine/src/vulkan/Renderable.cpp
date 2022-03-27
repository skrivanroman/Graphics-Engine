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
		glm::mat4 model{ 1.0f };
		model = glm::translate(model, position);
		model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, scale);
		return model;
	}

}
