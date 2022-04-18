#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Vk 
{

	class Shader //module and stage dont need to be members for now
	{
	public:
		explicit Shader(const VkDevice logicalDevice, const std::string& path, const VkShaderStageFlagBits stage);
		~Shader();
		
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		VkShaderModule getModule() const noexcept;
		VkPipelineShaderStageCreateInfo getCreateInfo() const noexcept;

	private:
		void init(const std::string& path, const VkShaderStageFlagBits stage);
		std::vector<char> loadShader(const std::string& path) const;

	private:
		VkShaderModule shaderModule;
		const VkDevice logicalDevice;
		VkPipelineShaderStageCreateInfo createInfo{};
	};
}
