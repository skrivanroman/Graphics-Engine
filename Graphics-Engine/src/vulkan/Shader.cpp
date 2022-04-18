#include <fstream>
#include "Shader.hpp"
#include "../utils/assert.hpp"
#include "../utils/Logger.hpp"
#include "../utils/path.hpp"

namespace Vk 
{

	Shader::Shader(const VkDevice logicalDevice, const std::string& path, const VkShaderStageFlagBits stage)
		:shaderModule(VK_NULL_HANDLE), logicalDevice(logicalDevice)
	{
		init(path, stage);
	}

	Shader::~Shader()
	{
		vkDestroyShaderModule(logicalDevice, shaderModule, nullptr);
	}

	VkShaderModule Shader::getModule() const noexcept
	{
		return shaderModule;
	}

	VkPipelineShaderStageCreateInfo Shader::getCreateInfo() const noexcept
	{
		return createInfo;
	}

	void Shader::init(const std::string& path, const VkShaderStageFlagBits stage)
	{
		auto shaderCode = loadShader(path);

		VkShaderModuleCreateInfo moduleInfo{};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleInfo.codeSize = shaderCode.size();
        moduleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		assert(vkCreateShaderModule(logicalDevice, &moduleInfo, nullptr, &shaderModule) == VK_SUCCESS, "cant create shader module");
		
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.stage = stage;
		createInfo.module = shaderModule;
        createInfo.pName = "main";
	}

	std::vector<char> Shader::loadShader(const std::string& path) const
	{
		std::string shaderFolder = getFileDir(__FILE__) + "\\..\\shaders\\";
		std::ifstream file(shaderFolder + path, std::ios::ate | std::ios::binary);

		assert(file.is_open(), "cant open shader file");

		std::vector<char> buffer(static_cast<size_t>(file.tellg()));

		file.seekg(0);
		file.read(buffer.data(), buffer.size());
		file.close();

		return buffer;
	}

}
