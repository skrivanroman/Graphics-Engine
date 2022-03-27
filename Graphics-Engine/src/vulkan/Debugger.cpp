#include "Debugger.hpp"
#include "../utils/Logger.hpp"
#include "../utils/assert.hpp"

namespace Vk 
{

	Debugger::Debugger(VkInstance instance)
		:instance(instance), debugger(VK_NULL_HANDLE)
	{
		init();
	}

	Debugger::~Debugger()
	{
		destroyDebugger();
	}

	VkDebugUtilsMessengerCreateInfoEXT Debugger::getCreateInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = (
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
			);
		createInfo.messageType = (
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
			);

		createInfo.pfnUserCallback = Debugger::vulkanCallback;

		return createInfo;
	}

	void Debugger::init()
	{
		auto createInfo = Debugger::getCreateInfo();
		auto createFunc = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

		assert(createFunc != nullptr, "Debugger creation func is null");

		assert(createFunc(instance, &createInfo, nullptr, &debugger) == VK_SUCCESS, "cant create debugger");
	}

	void Debugger::destroyDebugger()
	{
		auto destroyFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
	   
	   assert(destroyFunc != nullptr, "Debugger destroy func is null");

	   destroyFunc(instance, debugger, nullptr);
	}
	
	VKAPI_ATTR VkBool32 VKAPI_CALL Debugger::vulkanCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
		)
	{
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			//LOG_INFO(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LOG_WARNING(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LOG_ERROR(pCallbackData->pMessage);
			break;
		default:
			LOG_INFO(pCallbackData->pMessage);
			break;
		}

		return VK_FALSE;
	}
}
