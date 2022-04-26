#pragma once

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SwapChain.hpp"

namespace Vk 
{
	struct PushConstant
	{
		glm::mat4 model{ 1.0f };
		glm::mat4 viewProjection{ 1.0f };
	};

	class Pipeline
	{
	public:
		explicit Pipeline(const Device& device, SwapChain& swapChain);
		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		VkRenderPass getRenderPass() const;
		VkPipeline getPipeline() const;
		VkPipelineLayout getLayout() const noexcept;
		const VkDescriptorSetLayout getDescriptorSetLayout() const noexcept;

	private:
		void init();
		void createDescriptorLayout();
		void createPipelineLayout();
		void createPipeline();
		void createRenderPass();
		VkFormat findDepthFormat() const;

	private:
		const Device& device;
		SwapChain& swapChain;
		VkDescriptorSetLayout descriptorLayout;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
		VkRenderPass renderPass;
	};
}
