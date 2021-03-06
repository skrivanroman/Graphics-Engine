#include "Image.hpp"
#include "../utils/assert.hpp"
#include "../vulkan/Pipeline.hpp"

Image::Image(const Vk::Device& device, const std::string& path, const glm::vec2& dimensions, 
	const VkCommandPool commandPool, int32_t format
)
	:device(device), dimensions(dimensions)
{
	init(path, commandPool, format);
}

Image::~Image() noexcept
{
	vkDestroySampler(device.getLogicalDevice(), imageSampler, nullptr);
	vkDestroyImageView(device.getLogicalDevice(), imageView, nullptr);
	vkDestroyImage(device.getLogicalDevice(), image, nullptr);
	vkFreeMemory(device.getLogicalDevice(), imageMemory, nullptr);
}

const VkImageView Image::getImageView() const noexcept
{
	return imageView;
}

const VkSampler Image::getSampler() const noexcept
{
	return imageSampler;
}

void Image::draw(VkCommandBuffer commandBuffer, const VkPipelineLayout pipelineLayout, const Vk::Camera& camera) const
{
		VkBuffer rawVertexBuffer = vertexBuffer->getBuffer();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &rawVertexBuffer, &offset);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		Vk::PushConstant push{};
		push.model = transform.getModel();
		push.viewProjection = camera.getViewProjection();

		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Vk::PushConstant), &push);

        vkCmdDrawIndexed(commandBuffer, indexBuffer->getVertexCount(), 1, 0, 0, 0);

		//transform.rotation.y = transform.rotation.y + 0.001;
		transform.scale.x += 0.001;
		if (transform.scale.x > 2.5f)
			transform.scale.x = 1;
}

void Image::init(const std::string& path, const VkCommandPool commandPool, int32_t format)
{
	auto [buffer, width, height] = loadImage(path, format);
	createVkImage(width, height);
    allocateMemory();

	transferLayout(commandPool, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyFromBuffer(commandPool, buffer, width, height);
	transferLayout(commandPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	createVkImageView(device);
	createVkImageSampler();

	createBuffers(commandPool);
}

std::tuple<std::unique_ptr<Vk::Buffer>, int32_t, int32_t> Image::loadImage(const std::string& path, int32_t format)
{
	int32_t width, height, channels;
	stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, format);

	assert(pixels != nullptr, "cant load image");

	imageSize = static_cast<VkDeviceSize>(width) * height * 4;
	auto transferBuffer = std::make_unique<Vk::Buffer>(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	transferBuffer->setData(pixels, imageSize);

	stbi_image_free(pixels);

	return std::make_tuple(std::move(transferBuffer), width, height);
}

void Image::createVkImage(int32_t width, int32_t height)
{
	VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    assert(vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &image) == VK_SUCCESS, "cant create image");
}

void Image::allocateMemory()
{
    VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device.getLogicalDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = device.getMemoryTypeIdx(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	assert(vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &imageMemory) == VK_SUCCESS, "cant allocate image memory");

	vkBindImageMemory(device.getLogicalDevice(), image, imageMemory, 0);
}

void Image::transferLayout(const VkCommandPool commandPool, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = device.beginCommandBuffer(commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} 
	else
	{
		assert(false, "unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	device.endCommandBuffer(commandBuffer, commandPool);
}

void Image::copyFromBuffer(const VkCommandPool commandPool, const std::unique_ptr<Vk::Buffer>& buffer, int32_t width, int32_t height)
{
	VkCommandBuffer commandBuffer = device.beginCommandBuffer(commandPool);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer->getBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	device.endCommandBuffer(commandBuffer, commandPool);
}

void Image::createVkImageView(const Vk::Device& device)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	assert(vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &imageView) == VK_SUCCESS, "cant create image view");
}

void Image::createVkImageSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	assert(vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &imageSampler) == VK_SUCCESS, "cant create image sampler");
}

void Image::createBuffers(const VkCommandPool commandPool)
{
	std::vector<Vk::Vertex> vertices(4, { glm::vec3{0.0f}, glm::vec3{0.0f} });
	vertices[0].position.x = -dimensions.x / 2.0f;
	vertices[0].position.y = -dimensions.y / 2.0f;
	vertices[0].texCord = glm::vec2{ 1.0f, 0.0f };
	vertices[1].position.x = dimensions.x / 2.0f;
	vertices[1].position.y = -dimensions.y / 2.0f;
	vertices[1].texCord = glm::vec2{ 0.0f };
	vertices[2].position.x = dimensions.x / 2.0f;
	vertices[2].position.y = dimensions.y / 2.0f;
	vertices[2].texCord = glm::vec2{ 0.0f, 1.0f };
	vertices[3].position.x = -dimensions.x / 2.0f;
	vertices[3].position.y = dimensions.y / 2.0f;
	vertices[3].texCord = glm::vec2{ 1.0f };
	
	std::vector<uint32_t> indices = {
		0, 1, 2, 2, 3, 0
	};

	vertexBuffer = std::make_unique<Vk::Buffer>(device, vertices, commandPool);
	indexBuffer = std::make_unique<Vk::Buffer>(device, indices, commandPool);
}
