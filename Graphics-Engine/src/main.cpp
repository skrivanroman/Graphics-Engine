#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <cmath>
#include "utils/Logger.hpp"
#include "Window.hpp"
#include "vulkan/Vulkan.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Debugger.hpp"
#include "vulkan/SwapChain.hpp"
#include "vulkan/Shader.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/Buffer.hpp"

glm::vec2 middle(const glm::vec2& first, const glm::vec2& second)
{
	return (first + second) / 2.0f;
}

std::vector<Vk::Vertex> generateTriangle(const glm::vec2& left, const glm::vec2& up, const glm::vec2& right, uint32_t depth)
{
	std::vector<glm::vec2> triangles{ left, up, right };
	triangles.reserve(static_cast<size_t>(pow(3, depth)));

	for (uint32_t i = 0; i < depth; ++i)
	{
		std::vector<glm::vec2> toAdd;
		toAdd.reserve(static_cast<size_t>(pow(3, i + 2)));

		for (uint32_t j = 0; j < triangles.size(); j += 3)
		{
			toAdd.push_back(triangles[j]);
			toAdd.push_back(middle(triangles[j], triangles[j + 1]));
			toAdd.push_back(middle(triangles[j], triangles[j + 2]));

			toAdd.push_back(middle(triangles[j], triangles[j + 1]));
			toAdd.push_back(triangles[j + 1]);
			toAdd.push_back(middle(triangles[j + 1], triangles[j + 2]));

			toAdd.push_back(middle(triangles[j], triangles[j + 2]));
			toAdd.push_back(middle(triangles[j + 1], triangles[j + 2]));
			toAdd.push_back(triangles[j + 2]);
		}

		triangles = toAdd;
	}

	std::vector<Vk::Vertex> vertices(triangles.size());
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		//vertices[i].position = triangles[i];
		if (i % 3 == 0)
			vertices[i].color = { 1.0f, 0, 0 };
		else if (i % 3 == 1)
			vertices[i].color = { 0, 1.0f, 0 };
		else
			vertices[i].color = { 0, 0, 1.0f };
	}
	
	return vertices;
}

void run()
{
	WindowCreateInfo createInfo{};
	createInfo.aspectWidth = 16;
	createInfo.aspectHeight = 9;
	createInfo.width = 1200;
	createInfo.useCursor = true;
	createInfo.title = "vulkan";
	createInfo.fullScreen = false;
	createInfo.onResize = Window::onResize;

//	const std::vector<Vk::Vertex> vertices = {
//		{{0.0f, -0.8f}, {1.0f, 0.0f, 0.0f}},
//		{{0.8f, 0.8f}, {0.0f, 1.0f, 0.0f}},
//		{{-0.8f, 0.8f}, {0.0f, 0.0f, 1.0f}}
//	};
//
	const std::vector<Vk::Vertex> rect(4);
	//{
	//	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	//	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	//	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	//	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	//};

	const std::vector<uint32_t> indices = {
		0, 1, 2, 2, 3, 0
	};

	Window window(createInfo);
	Vk::Vulkan vulkan(window);
	USE_DEBUGGER(vulkan.getInstance());
	Vk::Device device(vulkan.getInstance(), window);
	Vk::SwapChain swapChain(device, window);
	Vk::Pipeline pipeline(device, swapChain);
	//auto recTri = generateTriangle({ -0.8, 0.8f }, { 0.0f, -0.8f }, { 0.8f, 0.8f }, 6);
	Vk::Renderer renderer(window, device, swapChain, pipeline, rect, indices, 2);

	while (!window.shouldClose())
	{
		glfwPollEvents();
		
		if (!window.isMinimized())
			renderer.drawFrame();

	}

	vkDeviceWaitIdle(device.getLogicalDevice());
}

int main()
{
	glfwInit();
	glfwSetErrorCallback(Logger::glfwCallback);

	try 
	{
		run();
	}
	catch (const std::runtime_error& error)
	{
		LOG_ERROR(error.what());
	}
	glfwTerminate();
}