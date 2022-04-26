#pragma once

#include <string>
#include <memory>
#include <glm/gtx/transform.hpp>
#include "Buffer.hpp"
#include "Camera.hpp"

namespace Vk
{
	//nonoptimal rotation
	struct Transform
	{
		glm::vec3 position{ 0.0f };
		glm::vec3 scale{ 1.0f };
		glm::vec3 rotation{ 0.0f };

		glm::mat4 getModel() const noexcept;
	};

	class Renderable
	{
	public:
		virtual ~Renderable() noexcept;

		Renderable(const Renderable&) = delete;
		Renderable(Renderable&&) = default;
		Renderable& operator=(const Renderable&) = delete;

		virtual void draw(VkCommandBuffer commandBuffer, const VkPipelineLayout pipelineLayout, const Camera& camera) const = 0;
		const Buffer& getVertexBuffer() const noexcept;
		const Buffer& getIndexBuffer() const noexcept;

	public:
		mutable Transform transform;

	protected:
		Renderable(const Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
			const VkCommandPool commandPool);
		Renderable();

	protected:
		std::unique_ptr<Buffer> vertexBuffer, indexBuffer;
	};
}
