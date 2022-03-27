#pragma once

#include <vulkan/vulkan.h>
#include "Device.hpp"

namespace Vk 
{

	#if ENABLE_VALIDATION_LAYERS
		#define USE_DEBUGGER(instance) Vk::Debugger debugger(instance)
	#else
		#define USE_DEBUGGER(instance) 
	#endif
	
	class Debugger
	{
	public:
		explicit Debugger(const VkInstance instance);
		~Debugger();

		Debugger(const Debugger&) = delete;
		Debugger& operator=(const Debugger&) = delete;

		static VkDebugUtilsMessengerCreateInfoEXT getCreateInfo();

	private:
		void init();
		void destroyDebugger(); 
		static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		);

	private:
		const VkInstance instance;
		VkDebugUtilsMessengerEXT debugger;
	};
}