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
#include "vulkan/Camera.hpp"
#include "input/KeyboardMouse.hpp"
#include "vulkan/Cube.hpp"

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

	Window window(createInfo);
	Vk::Vulkan vulkan(window);
	USE_DEBUGGER(vulkan.getInstance());
	Vk::Device device(vulkan.getInstance(), window);
	Vk::SwapChain swapChain(device, window);
	Vk::Pipeline pipeline(device, swapChain);
	Vk::Renderer renderer(window, device, swapChain, pipeline, 2);
	Vk::Camera camera({ -2.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 4.0f }, window.getAspectRatio(), glm::radians(50.0));
	KeyboardMouse controlls(.5, .5);

	auto cube = std::make_shared<Vk::Cube>(Vk::Cube::createCube(device, glm::vec3{ 0.5, .5, .5 }, glm::vec3{ 0.5f, 0 , 4 }, glm::vec3{ 0 }, renderer.getCommandPool()));
	renderer.addRenderObject(cube);

	while (!window.shouldClose())
	{
		glfwPollEvents();
		
		if (!window.isMinimized())
		{
			renderer.drawFrame(camera);

			auto change = controlls.getUpdate(window.getWindowPtr(), 0.006, camera.position.y);
			if (change.has_value() )
			{
				camera.move(change.value());
				camera.update();
			}
		}
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