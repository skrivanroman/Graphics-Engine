#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <array>
#include "Device.hpp"

namespace Vk 
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;

		static std::array<VkVertexInputBindingDescription, 1> getBindingDescriptions();
		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
	};

	//sharing mode exlusive for now
	class Buffer
	{
	public:
		explicit Buffer(const Device& device, const std::vector<Vertex>& vertices, const VkCommandPool commandPool);
		explicit Buffer(const Device& device, const std::vector<uint32_t>& indices, const VkCommandPool commandPool);
		explicit Buffer(const Device& device, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags memoryProperties);
		~Buffer() noexcept;

		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = default;
		Buffer& operator=(const Buffer&) = delete;

		void bind(const VkCommandBuffer commandBuffer) const;
		void copyBuffer(VkBuffer destinationBuffer, const VkCommandPool commandPool) const;
		const uint32_t getVertexCount() const noexcept;
		VkDeviceSize getDeviceSize() const noexcept;
		VkBuffer getBuffer() const noexcept;
		const VkDeviceMemory getMemory() const noexcept;

	private:
		void initVertexBuffer(const std::vector<Vertex>& vertices, const VkCommandPool commandPool);
		void initIndexBuffer(const std::vector<uint32_t>& indices, const VkCommandPool commmandPool);
		void allocateBuffer(VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags memoryProperties);

	private:
		const Device& device;
		const uint32_t vertexCount;
		VkBuffer buffer;
		VkDeviceSize size;
		VkDeviceMemory bufferMemory;
	};
}

