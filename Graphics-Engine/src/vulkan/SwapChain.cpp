#include <algorithm>
#include <array>
#include "SwapChain.hpp"
#include "../utils/Logger.hpp"
#include "../utils/assert.hpp"

namespace Vk 
{

	SwapChain::SwapChain(const Device& device, const Window& window)
		: swapChain(VK_NULL_HANDLE), device(device), window(window)
	{
		init(false);
	}

	SwapChain::~SwapChain()
	{
		destroySwapChain(swapChain);
	}

	void SwapChain::init(bool recreation)
	{
		auto oldSwapChain = swapChain;
		createSwapChain();

		if (recreation)
		{
			vkDeviceWaitIdle(device.getLogicalDevice());
			destroySwapChain(oldSwapChain);
		}

		createImages();
		createDepthImages();
	}

	void SwapChain::destroySwapChain(VkSwapchainKHR oldSwapChain)
	{
		for (auto frameBuffer : frameBuffers)
			vkDestroyFramebuffer(device.getLogicalDevice(), frameBuffer, nullptr);
		for (auto imageView : imageViews)
			vkDestroyImageView(device.getLogicalDevice(), imageView, nullptr);

		for (auto depthImageView : depthImageViews)
			vkDestroyImageView(device.getLogicalDevice(), depthImageView, nullptr);
		for (auto depthImage : depthImages)
			vkDestroyImage(device.getLogicalDevice(), depthImage, nullptr);
		for (auto imageMemory : depthImageMemory)
			vkFreeMemory(device.getLogicalDevice(), imageMemory, nullptr);

		vkDestroySwapchainKHR(device.getLogicalDevice(), oldSwapChain, nullptr);
	}

	void SwapChain::createSwapChain()
	{
		SwapChainSupport swapChainSupport = SwapChain::querySwapChainSupport(device.getPhysicalDevice(), device.getSurface());

		VkSurfaceFormatKHR format = pickFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = pickPresentMode(swapChainSupport.presentModes);
		imageExtent = pickSwapExtent2D(swapChainSupport.capabilities, window);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = device.getSurface();

		if (swapChainSupport.capabilities.maxImageCount == 0)
			createInfo.minImageCount = swapChainSupport.capabilities.minImageCount + 1;
		else
			createInfo.minImageCount = std::min(swapChainSupport.capabilities.minImageCount + 1, swapChainSupport.capabilities.maxImageCount);

		imageFormat = format.format;
		createInfo.imageFormat = imageFormat;
		createInfo.imageColorSpace = format.colorSpace;
		createInfo.imageExtent = imageExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = device.getQueueFamilies(device.getPhysicalDevice());
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = swapChain;

		assert(vkCreateSwapchainKHR(device.getLogicalDevice(), &createInfo, nullptr, &swapChain) == VK_SUCCESS, "cant create swapChain");
	}

	void SwapChain::createImages()
	{
		uint32_t imageCount;
		vkGetSwapchainImagesKHR(device.getLogicalDevice(), swapChain, &imageCount, nullptr);
		images.resize(imageCount);
		vkGetSwapchainImagesKHR(device.getLogicalDevice(), swapChain, &imageCount, images.data());

		imageViews.resize(images.size());

		for (size_t i = 0; i < images.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = imageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			assert(vkCreateImageView(device.getLogicalDevice(), &createInfo, nullptr, &imageViews[i]) == VK_SUCCESS, "cant create image view");
		}

	}

	void SwapChain::createDepthImages()
	{
		VkFormat depthFormat = findDepthFormat();

		depthImages.resize(images.size());
		depthImageMemory.resize(images.size());
		depthImageViews.resize(images.size());

		for (size_t i = 0; i < depthImages.size(); ++i)
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = imageExtent.width;
			imageInfo.extent.height = imageExtent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = depthFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = 0;

			VkResult result = vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &depthImages[i]);
			assert(result == VK_SUCCESS, "cant create depth images");

			VkMemoryRequirements memoryRequirements;
			vkGetImageMemoryRequirements(device.getLogicalDevice(), depthImages[i], &memoryRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memoryRequirements.size;
			allocInfo.memoryTypeIndex = device.getMemoryTypeIdx(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			result = vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &depthImageMemory[i]);
			assert(result == VK_SUCCESS, "cant allocate depth image memory");

			result = vkBindImageMemory(device.getLogicalDevice(), depthImages[i], depthImageMemory[i], 0);
			assert(result == VK_SUCCESS, "cant bind depth image memory");

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = depthImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = depthFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			result = vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &depthImageViews[i]);
			assert(result == VK_SUCCESS, "cant create depth image view");
		}
	}

	void SwapChain::createFrameBuffers(const VkRenderPass renderPass)
	{
		frameBuffers.resize(imageViews.size());

		for (size_t i = 0; i < imageViews.size(); ++i)
		{
			VkFramebufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.renderPass = renderPass;
			
			std::array attachments = {imageViews[i], depthImageViews[i]};
			createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			createInfo.pAttachments = attachments.data();
			createInfo.width = imageExtent.width;
			createInfo.height = imageExtent.height;
			createInfo.layers = 1;

			assert(vkCreateFramebuffer(device.getLogicalDevice(), &createInfo, nullptr, &frameBuffers[i]) == VK_SUCCESS, "cant create frame buffers");
		}
	}

	uint32_t SwapChain::getImageCount() const noexcept
	{
		return static_cast<uint32_t>(images.size());
	}

	VkSurfaceFormatKHR SwapChain::pickFormat(std::vector<VkSurfaceFormatKHR>& formats) const
	{
		for (auto availableFormat : formats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}

		LOG_WARNING("Desired surface format is not available");
		return formats[0];
	}

	VkPresentModeKHR SwapChain::pickPresentMode(std::vector<VkPresentModeKHR>& presentModes) const
	{
		if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.end())
			return VK_PRESENT_MODE_MAILBOX_KHR;

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	SwapChainSupport SwapChain::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupport details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void SwapChain::recreateSwapChain(const VkRenderPass renderPass)
	{
		init(true);
		createFrameBuffers(renderPass);
	}

	VkExtent2D SwapChain::getExtent() const
	{
		return imageExtent;
	}

	VkFormat SwapChain::getImageFormat() const
	{
		return imageFormat;
	}

	const std::vector<VkImageView>& SwapChain::getImageViews() const 
	{
		return imageViews;
	}

	VkResult SwapChain::acquireNextImage(VkSemaphore semaphore, uint32_t* imageIndex) const
	{
		return vkAcquireNextImageKHR(device.getLogicalDevice(), swapChain, NO_TIMEOUT, semaphore, VK_NULL_HANDLE, imageIndex);
	}

	void SwapChain::presentImage(uint32_t imageIndex, VkSemaphore* waitSemaphores) const
	{

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemaphores;

        presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;

        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);
	}

	VkSwapchainKHR SwapChain::getSwapChain() const
	{
		return swapChain;
	}

	const std::vector<VkFramebuffer>& SwapChain::getFrameBuffers() const
	{
		return frameBuffers;
	}

	VkExtent2D SwapChain::pickSwapExtent2D(VkSurfaceCapabilitiesKHR& capabilities, const Window& window) const
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D actualExtent = window.getExtent();

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
	VkFormat SwapChain::findDepthFormat() const
	{
		return device.getSupportedFormat(
		  {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		  VK_IMAGE_TILING_OPTIMAL,
		  VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}
}
