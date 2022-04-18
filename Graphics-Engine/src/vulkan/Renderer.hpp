#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "Device.hpp"
#include "SwapChain.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"
#include "Renderable.hpp"
#include "Cube.hpp"

namespace Vk 
{
	class Renderer
	{
	public:
		explicit Renderer(const Window& window, const Device& device, SwapChain& swapChain, const Pipeline& pipeline,
			uint32_t maxFramesInFlight = 2, const std::vector<std::shared_ptr<Renderable>>& renderObjects = {}
		);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void drawFrame(const Camera& camera);
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const Camera& camera);
		void addRenderObject(std::shared_ptr<Renderable> object);
		const VkCommandPool getCommandPool() const noexcept;

	private:
		void init();
		void createCommandPool();
		void createCommandBuffers();
		void createUniformBuffers();
		void createSyncObjects();
		void createDescriptorPool();

	private:
		const Window& window;
		const Device& device;
		SwapChain& swapChain;
		const Pipeline& pipeline;
		const uint32_t maxFramesInFlight;
		uint32_t currentFrame;
		VkCommandPool commandPool;
		VkDescriptorPool descriptorPool;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<std::shared_ptr<Renderable>> renderObjects;
		std::vector<std::unique_ptr<Buffer>> uniformBuffers;
	};
}
