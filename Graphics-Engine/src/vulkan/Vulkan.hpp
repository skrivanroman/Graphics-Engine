#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "../Window.hpp"

namespace Vk 
{

	#define ENABLE_VALIDATION_LAYERS 1

	class Vulkan
	{
	public: 
		explicit Vulkan(const Window& window);
		~Vulkan();

		Vulkan(const Vulkan&) = delete;
		Vulkan& operator=(const Vulkan&) = delete;

		VkInstance getInstance() const;

	private:
		void init(const Window& window);
		bool checkValidationSupport() const;
		std::vector<const char*> getRequiredExtensions(const Window& window) const;

	private:
		VkInstance instance;
		std::vector<const char*> validationLayers;
	};
}
