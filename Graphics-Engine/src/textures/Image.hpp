#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <string>
#include "../vulkan/device.hpp"
#include "../vulkan/Buffer.hpp"

class Image
{
public:
	explicit Image(const Vk::Device& device, const std::string& path, const VkCommandPool commandPool, int32_t format = STBI_rgb_alpha);
	~Image() noexcept;

	Image(const Image&) = delete;
	Image(Image&&) = default;
	Image& operator=(const Image&) = delete;

private:
	void init(const std::string& path, const VkCommandPool commandPool, int32_t format);
	std::tuple<Vk::Buffer, int32_t, int32_t> loadImage(const std::string& path, int32_t format);
	void createVkImage(int32_t width, int32_t height);
	void allocateMemory();
	void transferLayout(const VkCommandPool commandPool, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyFromBuffer(const VkCommandPool commandPool, const Vk::Buffer& buffer, int32_t width, int32_t height);

private:
	const Vk::Device& device;
	VkImage image;
	VkDeviceMemory imageMemory;
	VkDeviceSize imageSize;
};

