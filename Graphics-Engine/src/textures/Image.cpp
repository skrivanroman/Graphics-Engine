#include "Image.hpp"
#include "../utils/assert.hpp"

Image::Image(const Vk::Device& device, const std::string& path, const VkCommandPool commandPool, int32_t format)
	:device(device)
{
	init(path, commandPool, format);
}

Image::~Image() noexcept
{
	vkDestroyImage(device.getLogicalDevice(), image, nullptr);
	vkFreeMemory(device.getLogicalDevice(), imageMemory, nullptr);
}

void Image::init(const std::string& path, const VkCommandPool commandPool, int32_t format)
{
	auto [buffer, width, height] = loadImage(path, format);
	createVkImage(width, height);
    allocateMemory();

	transferLayout(commandPool, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyFromBuffer(commandPool, buffer, width, height);
	transferLayout(commandPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

std::tuple<Vk::Buffer, int32_t, int32_t> Image::loadImage(const std::string& path, int32_t format)
{
	int32_t width, height, channels;
	stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, format);

	assert(pixels != nullptr, "cant load image");

	imageSize = static_cast<VkDeviceSize>(width) * height * 4;
	Vk::Buffer transferBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	transferBuffer.setData(pixels, imageSize);

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

void Image::copyFromBuffer(const VkCommandPool commandPool, const Vk::Buffer& buffer, int32_t width, int32_t height)
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

	vkCmdCopyBufferToImage(commandBuffer, buffer.getBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	device.endCommandBuffer(commandBuffer, commandPool);
}
