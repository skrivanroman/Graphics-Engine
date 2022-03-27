#include <cstring>
#include "Vulkan.hpp"
#include "Debugger.hpp"
#include "../utils/assert.hpp"

namespace Vk 
{

	Vulkan::Vulkan(const Window& window)
		:instance(VK_NULL_HANDLE)
	{
		#if ENABLE_VALIDATION_LAYERS 
			validationLayers.push_back("VK_LAYER_KHRONOS_validation");
		#endif

		init(window);
	}
	
	Vulkan::~Vulkan()
	{
		vkDestroyInstance(instance, nullptr);
	}

	VkInstance Vulkan::getInstance() const
	{
		return instance;
	}

	void Vulkan::init(const Window& window)
	{
		if (ENABLE_VALIDATION_LAYERS)
			assert(checkValidationSupport(), "validation layers not supported");

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "RayTracer";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		
		std::vector<const char*> extensions = getRequiredExtensions(window);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		
		#if ENABLE_VALIDATION_LAYERS
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		#else
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		#endif
		
		auto debuggerCreateInfo = Debugger::getCreateInfo();
		createInfo.pNext = &debuggerCreateInfo;

		assert(vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS, "cant create vulkan instance");

	}

	bool Vulkan::checkValidationSupport() const
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) 
				return false;
		}
		return true;
	}

	std::vector<const char*> Vulkan::getRequiredExtensions(const Window& window) const
	{
		auto [glfwExtensions, glfwExtensionCount] = window.getRequiredExtensions();

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		#if ENABLE_VALIDATION_LAYERS
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		#endif

		return extensions;
	}
}
