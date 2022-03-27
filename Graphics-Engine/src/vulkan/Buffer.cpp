#pragma optimize( "", off )
#include "Buffer.hpp"
#include "../utils/assert.hpp"

namespace Vk
{
	Buffer::Buffer(const Device& device, const std::vector<Vertex>& vertices, const VkCommandPool commandPool)
		:device(device), vertexCount(static_cast<uint32_t>(vertices.size())), buffer(VK_NULL_HANDLE),
		size(sizeof(Vertex) * vertices.size()), bufferMemory(VK_NULL_HANDLE)
	{
		initVertexBuffer(vertices, commandPool);
	}

	Buffer::Buffer(const Device& device, const std::vector<uint32_t>& indices, const VkCommandPool commandPool)
		:device(device), vertexCount(static_cast<uint32_t>(indices.size())), buffer(VK_NULL_HANDLE),
		size(sizeof(indices[0]) * indices.size()), bufferMemory(VK_NULL_HANDLE)
	{
		initIndexBuffer(indices, commandPool);
	}

	Buffer::Buffer(const Device& device, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags memoryProterties)
		:device(device), vertexCount(0), buffer(VK_NULL_HANDLE), size(size), bufferMemory(VK_NULL_HANDLE)
	{
		allocateBuffer(bufferUsage, memoryProterties);
	}
	#pragma optimize( "", on )

	Buffer::~Buffer() noexcept
	{
		vkDestroyBuffer(device.getLogicalDevice(), buffer, nullptr);
		vkFreeMemory(device.getLogicalDevice(), bufferMemory, nullptr);
	}

	void Buffer::bind(const VkCommandBuffer commandBuffer) const
	{
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);
	}

	void Buffer::copyBuffer(VkBuffer destinationBuffer, const VkCommandPool commandPool) const
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device.getLogicalDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; 
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, buffer, destinationBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, nullptr);
		vkQueueWaitIdle(device.getGraphicsQueue());

		vkFreeCommandBuffers(device.getLogicalDevice(), commandPool, 1, &commandBuffer);
	}

	const uint32_t Buffer::getVertexCount() const noexcept
	{
		return vertexCount;
	}

	VkDeviceSize Buffer::getDeviceSize() const noexcept
	{
		return size;
	}

	VkBuffer Buffer::getBuffer() const noexcept
	{
		return buffer;
	}

	const VkDeviceMemory Buffer::getMemory() const noexcept
	{
		return bufferMemory;
	}
	
	void Buffer::initVertexBuffer(const std::vector<Vertex>& vertices, const VkCommandPool commandPool)
	{
		Buffer transferBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* data;
		vkMapMemory(device.getLogicalDevice(), transferBuffer.getMemory(), 0, size, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(size));
		vkUnmapMemory(device.getLogicalDevice(), transferBuffer.getMemory());

		allocateBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		transferBuffer.copyBuffer(buffer, commandPool);
	}

	void Buffer::initIndexBuffer(const std::vector<uint32_t>& indices, const VkCommandPool commandPool)
	{
		Buffer transferBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* data;
		vkMapMemory(device.getLogicalDevice(), transferBuffer.getMemory(), 0, size, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(size));
		vkUnmapMemory(device.getLogicalDevice(), transferBuffer.getMemory());

		allocateBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		transferBuffer.copyBuffer(buffer, commandPool);
	}

	void Buffer::allocateBuffer(VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags memoryProperties)
	{
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = size;
		createInfo.usage = bufferUsage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; 
		assert(vkCreateBuffer(device.getLogicalDevice(), &createInfo, nullptr, &buffer) == VK_SUCCESS, "cant create buffer");

		VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device.getLogicalDevice(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.getMemoryTypeIdx(memRequirements.memoryTypeBits, memoryProperties);

		assert(vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &bufferMemory) == VK_SUCCESS, "cant allocate buffer");

        vkBindBufferMemory(device.getLogicalDevice(), buffer, bufferMemory, 0);
	}

	std::array<VkVertexInputBindingDescription, 1> Vertex::getBindingDescriptions()
	{
		std::array<VkVertexInputBindingDescription, 1> bindingDescriptions{};
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
}
