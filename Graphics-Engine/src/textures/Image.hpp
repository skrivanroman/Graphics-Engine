#pragma once

#include <stb/stb_image.h>
#include <string>
#include "../vulkan/device.hpp"
#include "../vulkan/Buffer.hpp"
#include "../vulkan/Renderable.hpp"

class Image : public Vk::Renderable
{
public:
	explicit Image(const Vk::Device& device, const std::string& path, const glm::vec2& dimensions, const VkCommandPool commandPool, 
		int32_t format = STBI_rgb_alpha
	);
	~Image() noexcept;

	Image(const Image&) = delete;
	Image(Image&&) = default;
	Image& operator=(const Image&) = delete;

	const VkImageView getImageView() const noexcept;
	const VkSampler getSampler() const noexcept;
	void draw(VkCommandBuffer commandBuffer, const VkPipelineLayout pipelineLayout, const Vk::Camera& camera) const override;
	
private:
	void init(const std::string& path, const VkCommandPool commandPool, int32_t format);
	std::tuple<std::unique_ptr<Vk::Buffer>, int32_t, int32_t> loadImage(const std::string& path, int32_t format);
	void createVkImage(int32_t width, int32_t height);
	void allocateMemory();
	void transferLayout(const VkCommandPool commandPool, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyFromBuffer(const VkCommandPool commandPool, const std::unique_ptr<Vk::Buffer>& buffer, int32_t width, int32_t height);
	void createVkImageView(const Vk::Device& device);
	void createVkImageSampler();
	void createBuffers(const VkCommandPool commandPool);

private:
	const Vk::Device& device;
	glm::vec2 dimensions;
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;
	VkDeviceSize imageSize;
	VkSampler imageSampler;
};

