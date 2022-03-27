#pragma once

#include <optional>
#include <vector>
#include "../Window.hpp"

namespace Vk 
{

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool hasValues() const;
	};

	class Device
	{
	public:
		explicit Device(const VkInstance instance, const Window& window);
		~Device();

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		VkPhysicalDevice getPhysicalDevice() const;
		VkDevice getLogicalDevice() const;
		VkSurfaceKHR getSurface() const;
		QueueFamilyIndices getQueueFamilies(VkPhysicalDevice deviceToRate) const;
		VkQueue getGraphicsQueue() const;
		VkQueue getPresentQueue() const;
		VkFormat getSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
		uint32_t getMemoryTypeIdx(VkFlags requiredTypes, VkMemoryPropertyFlags properties) const;

	private:
		void init(const Window& window);
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSurface(const Window& window);
		uint32_t ratePhysicalDevice(VkPhysicalDevice device) const;
		bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;

	private:
		const VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkQueue graphicsQueue, presentQueue;
		std::vector<const char*> deviceExtensions;
	};
}
