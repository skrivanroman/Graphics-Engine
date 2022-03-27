#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "../Window.hpp"
#include "Device.hpp"

namespace Vk 
{

	struct SwapChainSupport {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	#define NO_TIMEOUT UINT64_MAX

	class SwapChain
	{
	public:
		explicit SwapChain(const Device& device, const Window& window);
		~SwapChain();

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;

		static SwapChainSupport querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
		void recreateSwapChain(const VkRenderPass);
		VkExtent2D getExtent() const;
		VkFormat getImageFormat() const;
		const std::vector<VkImageView>& getImageViews() const;
		VkResult acquireNextImage(VkSemaphore semaphore, uint32_t* imageIndex) const;
		void presentImage(uint32_t imageIndex, VkSemaphore* waitSemaphores) const;
		VkSwapchainKHR getSwapChain() const;
		const std::vector<VkFramebuffer>& getFrameBuffers() const;
		void createFrameBuffers(const VkRenderPass renderPass);
		uint32_t getImageCount() const noexcept;

	private:
		void init(bool reacreation);
		void destroySwapChain(VkSwapchainKHR oldSwapChain);
		void createSwapChain();
		void createImages();
		void createDepthImages();
		VkSurfaceFormatKHR pickFormat(std::vector<VkSurfaceFormatKHR>& formats) const;
		VkPresentModeKHR pickPresentMode(std::vector<VkPresentModeKHR>& presentModes) const;
		VkExtent2D pickSwapExtent2D(VkSurfaceCapabilitiesKHR& capabilities, const Window& window) const;
		VkFormat findDepthFormat() const;

	private:
		VkSwapchainKHR swapChain;
		const Device& device;
		const Window& window;
		std::vector<VkImage> images;
		std::vector<VkImage> depthImages;
		VkFormat imageFormat;
		VkExtent2D imageExtent;
		std::vector<VkImageView> imageViews;
		std::vector<VkImageView> depthImageViews;
		std::vector<VkDeviceMemory> depthImageMemory;
		std::vector<VkFramebuffer> frameBuffers;
	};
}
