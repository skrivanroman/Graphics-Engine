#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <Vulkan/Vulkan.h>
#include <utility>

struct WindowCreateInfo
{
	uint32_t width;
	uint32_t aspectWidth;
	uint32_t aspectHeight;
	const char* title;
	bool useCursor = true;
	bool fullScreen = false;
	const char* icon;
	GLFWwindowsizefun onResize;
};

class Window
{
public:
	explicit Window(const WindowCreateInfo& createInfo);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	bool shouldClose() const;
	bool isMinimized() const;
	bool wasResized() const;
	void resetWasResized() const;
	VkExtent2D getExtent() const;
	VkSurfaceKHR getSurface(const VkInstance instance) const;
	float getAspectRatio() const noexcept;
	std::pair<const char**, uint32_t> getRequiredExtensions() const;
	void setFullScreen(bool fullScreen);
	void resize(uint32_t width, uint32_t height);
	static void onResize(GLFWwindow* glfwWindow, int32_t with, int32_t height);

private:
	void init(const WindowCreateInfo& createInfo);
	GLFWimage loadIcon(const char* path) const;
	GLFWcursor* getBlankCursor() const;

private:
	GLFWwindow* glfwWindow;
	GLFWcursor* glfwCursor;
	uint32_t windowedWidth, windowedHeight;
	uint32_t fullScreenWidth, fullScreenHeight;
	double aspectRatio;
	bool isFullScreen, _isMinimized;
	mutable bool _wasResized;
};

