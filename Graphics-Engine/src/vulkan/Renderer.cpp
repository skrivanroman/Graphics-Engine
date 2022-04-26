#include "Renderer.hpp"
#include "../utils/assert.hpp"
#include "Cube.hpp"
#include "../input/KeyboardMouse.hpp"
#include "../utils/radom.hpp"

namespace Vk 
{
	
	Renderer::Renderer(const Window& window, const Device& device, SwapChain& swapChain, const Pipeline& pipeline,
		uint32_t maxFramesInFlight, const std::vector<std::shared_ptr<Image>>& images,
		const std::vector<std::shared_ptr<Renderable>>& renderObjects
	)
		:window(window), device(device), swapChain(swapChain), pipeline(pipeline), 
		maxFramesInFlight(maxFramesInFlight), currentFrame(0), renderObjects(renderObjects), images(images)
	{
		init();
	}

	Renderer::~Renderer()
	{
		vkDestroyDescriptorPool(device.getLogicalDevice(), descriptorPool, nullptr);
		for (size_t i = 0; i < maxFramesInFlight; ++i)
		{
			vkDestroySemaphore(device.getLogicalDevice(), imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device.getLogicalDevice(), renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device.getLogicalDevice(), inFlightFences[i], nullptr);
		}
		vkDestroyCommandPool(device.getLogicalDevice(), commandPool, nullptr);
	}

	void Renderer::drawFrame(const Camera& camera)
	{
		vkWaitForFences(device.getLogicalDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, NO_TIMEOUT);

		uint32_t imageIndex;
		VkResult result = swapChain.acquireNextImage(imageAvailableSemaphores[currentFrame], &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || window.wasResized())
		{
			LOG_INFO("window was resized");
			swapChain.recreateSwapChain(pipeline.getRenderPass());
			window.resetWasResized();
			return;
		}

        vkResetFences(device.getLogicalDevice(), 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex, camera);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

		assert(vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) == VK_SUCCESS, "cant submit command buffer");

		swapChain.presentImage(imageIndex, &renderFinishedSemaphores[currentFrame]);

		currentFrame = (currentFrame + 1) % maxFramesInFlight;
	}

	void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const Camera& camera)
	{
		VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		assert(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS, "cant start command buffer");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = pipeline.getRenderPass();
		renderPassInfo.framebuffer = swapChain.getFrameBuffers()[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = swapChain.getExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		VkExtent2D extent = swapChain.getExtent();
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = extent;


		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipeline());

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getLayout(), 0, 1, &descriptorSets[currentFrame], 0, nullptr);

		//VkBuffer vertexBuffers[] = {vertexBuffer->getBuffer()};
		//VkDeviceSize offsets[] = {0};
		//vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		//vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

		//PushConstant push{};
		//push.model = glm::scale(push.model, glm::vec3(0.3f, 0.3, 0.3));
		//push.model = glm::translate(push.model, glm::vec3(1.0f, 0, 0));
		//push.model = glm::rotate(push.model, glm::radians(45.0f), glm::vec3(0, 0, 1));
		//vkCmdPushConstants(commandBuffer, pipeline.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &push);

        //vkCmdDrawIndexed(commandBuffer, indexBuffer->getVertexCount(), 1, 0, 0, 0);

		for (auto& object : renderObjects)
		{
			object->draw(commandBuffer, pipeline.getLayout(), camera);
		}

        vkCmdEndRenderPass(commandBuffer);

		assert(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS, "cant end command buffer");
	}

	void Renderer::init()
	{
		createCommandPool();

			images.push_back(std::make_unique<Image>(device, "C:/Users/gewes/Pictures/mai.jpg", glm::vec2{ 1.0f }, commandPool));
			images[0]->transform.position.z += 2;

		renderObjects.push_back(images[0]);

		createCommandBuffers();
		createSyncObjects();
		createDescriptorPool();
		createDescriptorSets();
	}

	void Renderer::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = device.getQueueFamilies(device.getPhysicalDevice());

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		assert(vkCreateCommandPool(device.getLogicalDevice(), &poolInfo, nullptr, &commandPool) == VK_SUCCESS, "cant create command pool");
	}

	void Renderer::createCommandBuffers()
	{
		commandBuffers.resize(maxFramesInFlight);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = maxFramesInFlight;

		assert(vkAllocateCommandBuffers(device.getLogicalDevice(), &allocInfo, commandBuffers.data()) == VK_SUCCESS, "cant allocate command buffer");
	}

	void Renderer::createUniformBuffers() 
	{
		uniformBuffers.reserve(maxFramesInFlight);

		for (size_t i = 0; i < maxFramesInFlight; ++i)
		{
			uniformBuffers.push_back(std::make_unique<Buffer>(device, 128, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
		}
	}

	void Renderer::addRenderObject(std::shared_ptr<Renderable> object)
	{
		renderObjects.push_back(std::move(object));
	}

	const VkCommandPool Renderer::getCommandPool() const noexcept
	{
		return commandPool;
	}

	void Renderer::createSyncObjects()
	{
		imageAvailableSemaphores.resize(maxFramesInFlight);
		renderFinishedSemaphores.resize(maxFramesInFlight);
		inFlightFences.resize(maxFramesInFlight);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		
		for (size_t i = 0; i < maxFramesInFlight; ++i)
		{
			assert(vkCreateSemaphore(device.getLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VK_SUCCESS, "cant create semaphore");
			assert(vkCreateSemaphore(device.getLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) == VK_SUCCESS, "cant create semaphore");
			assert(vkCreateFence(device.getLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i]) == VK_SUCCESS, "cant create fence");
		}
	}

	void Renderer::createDescriptorPool()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = static_cast<uint32_t>(maxFramesInFlight);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(maxFramesInFlight);

		assert(vkCreateDescriptorPool(device.getLogicalDevice(), &poolInfo, nullptr, &descriptorPool) == VK_SUCCESS, "cant create descriptro pool");
	}

	void Renderer::createDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, pipeline.getDescriptorSetLayout());
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(maxFramesInFlight);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(maxFramesInFlight);
		assert(vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, descriptorSets.data()) == VK_SUCCESS, "cant allocate descriptor sets");

		for (size_t i = 0; i < maxFramesInFlight; ++i)
		{
			std::vector<VkDescriptorImageInfo> imageInfos(images.size());
			std::vector<VkWriteDescriptorSet> descriptorWrites(images.size());
			
			for (size_t j = 0; j < images.size(); ++j)
			{
				imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfos[j].imageView = images[j]->getImageView();
				imageInfos[j].sampler = images[j]->getSampler();
			
				descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[j].dstSet = descriptorSets[i];
				descriptorWrites[j].dstBinding = 1;
				descriptorWrites[j].dstArrayElement = 0;
				descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[j].descriptorCount = 1;
				descriptorWrites[j].pImageInfo = &imageInfos[j];
			}

			vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
}
