#include <vector>
#include <string>
#include <set>
#include <cstring>
#include "Device.hpp"
#include "../utils/Logger.hpp"
#include "SwapChain.hpp"
#include "../utils/assert.hpp"

namespace Vk
{

	Device::Device(const VkInstance instance, const Window& window)
		:instance(instance), surface(VK_NULL_HANDLE), physicalDevice(VK_NULL_HANDLE), device(VK_NULL_HANDLE),
		graphicsQueue(VK_NULL_HANDLE), presentQueue(VK_NULL_HANDLE)
	{
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		init(window);
	}

	Device::~Device()
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyDevice(device, nullptr);
	}

	VkPhysicalDevice Device::getPhysicalDevice() const
	{
		return physicalDevice;
	}

	VkDevice Device::getLogicalDevice() const
	{
		return device;
	}

	VkSurfaceKHR Device::getSurface() const
	{
		return surface;
	}

	void Device::init(const Window& window)
	{
		createSurface(window);
		pickPhysicalDevice();
		createLogicalDevice();
	}

	void Device::pickPhysicalDevice()
	{
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

		assert(physicalDeviceCount != 0, "no physical devices found");

		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

		uint32_t bestDeviceScore = 0;
		size_t bestDeviceIdx = 0;
		for (size_t i = 0; i < physicalDevices.size(); ++i)
		{
			uint32_t score = ratePhysicalDevice(physicalDevices[i]);
			if (score > bestDeviceScore)
			{
				bestDeviceScore = score;
				bestDeviceIdx = i;
			}
		}

		physicalDevice = physicalDevices[bestDeviceIdx];
	}

	uint32_t Device::ratePhysicalDevice(VkPhysicalDevice physicalDevice) const
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

		uint32_t deviceScore = 0;

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			deviceScore += 10000;

		if (!deviceFeatures.geometryShader || !getQueueFamilies(physicalDevice).hasValues())
			return 0;
		if (!checkDeviceExtensionSupport(physicalDevice))
			return 0;

		auto swapChainSupport = SwapChain::querySwapChainSupport(physicalDevice, surface);
		if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
			return 0;

		deviceScore += VK_API_VERSION_MAJOR(deviceProperties.apiVersion) * 500;
		deviceScore += VK_API_VERSION_MINOR(deviceProperties.apiVersion) * 100;

		deviceScore += deviceProperties.limits.maxImageDimension2D;

		if (deviceFeatures.shaderFloat64)
			deviceScore += 300;

		return deviceScore;
	}

	QueueFamilyIndices Device::getQueueFamilies(VkPhysicalDevice deviceToRate) const
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(deviceToRate, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(deviceToRate, &queueFamilyCount, queueFamilies.data());

		QueueFamilyIndices indices;
		for (uint32_t i = 0; i < queueFamilies.size(); ++i)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(deviceToRate, i, surface, &presentSupport);

			if (presentSupport)
				indices.presentFamily = i;

			if (indices.hasValues())
				break;

		}

		return indices;
	}

	VkQueue Device::getGraphicsQueue() const
	{
		return graphicsQueue;
	}

	VkQueue Device::getPresentQueue() const
	{
		return presentQueue;
	}

	VkFormat Device::getSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
	{
		for (VkFormat format : candidates) 
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
			{
				return format;
			} 
			else if ( tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
			{
			 return format;
			}
		}
		assert(false, "failed to find supported format!");
	}

	uint32_t Device::getMemoryTypeIdx(VkFlags requiredTypes, VkMemoryPropertyFlags properties) const
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			if (!(requiredTypes & (1 << i))) 
				continue;

			if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
			  return i;
		}	

		assert(false, "cant find required memory type");
	}

	void Device::createLogicalDevice()
	{
		auto indices = getQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(std::move(queueCreateInfo));
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		assert(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) == VK_SUCCESS, "cant create logical device");

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	void Device::createSurface(const Window& window)
	{
		surface = window.getSurface(instance);
	}


	bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) const 
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		for (auto& requiredExtension : deviceExtensions)
		{
			bool found = false;
			for (auto& available : availableExtensions)
			{
				if (strcmp(requiredExtension, available.extensionName) == 0)
				{
					found = true;
					break;
				}

			}

			if (!found)
				return false;
		}

		return true;
	}

	bool QueueFamilyIndices::hasValues() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
}