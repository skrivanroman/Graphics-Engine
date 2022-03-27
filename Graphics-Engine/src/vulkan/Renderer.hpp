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
			const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t maxFramesInFlight = 2
		);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void drawFrame();
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void setDataBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		void addRenderObject(std::unique_ptr<Renderable> object);

	private:
		void init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		void createCommandPool();
		void createCommandBuffers();
		void createUniformBuffers();
		void createSyncObjects();

	private:
		const Window& window;
		const Device& device;
		SwapChain& swapChain;
		const Pipeline& pipeline;
		Camera camera;
		const uint32_t maxFramesInFlight;
		uint32_t currentFrame;
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<std::unique_ptr<Renderable>> renderObjects;
		std::unique_ptr<Buffer> vertexBuffer, indexBuffer;
		//std::vector<std::unique_ptr<Buffer>> uniformBuffers;
	};
}
