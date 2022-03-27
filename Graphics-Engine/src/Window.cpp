#include "Window.hpp"
#include <memory>
#include <GLFW/glfw3native.h>
#include "utils/assert.hpp"

Window::Window(const WindowCreateInfo& createInfo)
	:glfwWindow(nullptr), glfwCursor(nullptr), windowedWidth(createInfo.width), isFullScreen(createInfo.fullScreen),
	_isMinimized(false), _wasResized(false)
{
	aspectRatio = static_cast<double>(createInfo.aspectWidth) / static_cast<double>(createInfo.aspectHeight);
	windowedHeight = static_cast<uint32_t>(createInfo.width / aspectRatio);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	fullScreenWidth = mode->width;
	fullScreenHeight = mode->height;
		
	init(createInfo);
}

Window::~Window()
{
	glfwDestroyCursor(glfwCursor);
	glfwDestroyWindow(glfwWindow);
}

GLFWcursor* Window::getBlankCursor() const
{
	GLFWimage cursorTexture;
	cursorTexture.width = 16;
	cursorTexture.height = 16;

	uint8_t pixels[16 * 16 * 4];
	memset(pixels, 0, sizeof(pixels));
	cursorTexture.pixels = pixels;

	return glfwCreateCursor(&cursorTexture, 0, 0);
}

VkExtent2D Window::getExtent() const
{
	int32_t width, height;
	glfwGetFramebufferSize(glfwWindow, &width, &height);
	return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(glfwWindow);
}

VkSurfaceKHR Window::getSurface(const VkInstance instance) const
{
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	assert(glfwCreateWindowSurface(instance, glfwWindow, nullptr, &surface) == VK_SUCCESS, "cant create surface");
	return surface;
}

float Window::getAspectRatio() const noexcept
{
	return static_cast<float>(aspectRatio);
}

std::pair<const char**, uint32_t> Window::getRequiredExtensions() const
{
	uint32_t count;
	const char** extensions = glfwGetRequiredInstanceExtensions(&count);
	return std::make_pair(extensions, count);
}

void Window::setFullScreen(bool fullScreen)
{
	if (fullScreen && !isFullScreen)
	{
		resize(fullScreenWidth, fullScreenHeight);
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		glfwSetWindowMonitor(glfwWindow, monitor, 0, 0, fullScreenWidth, fullScreenHeight, 60);
		isFullScreen = true;
	}
	else if (!fullScreen && isFullScreen)
	{
		resize(windowedWidth, windowedHeight);

		int32_t xPosition = static_cast<int32_t>((fullScreenWidth - windowedWidth) / 2.0);
		int32_t yPosition = static_cast<int32_t>((fullScreenHeight - windowedHeight) / 2.0);
		glfwSetWindowMonitor(glfwWindow, nullptr, xPosition, yPosition, windowedWidth, windowedHeight, 60);
		isFullScreen = false;
	}
}

void Window::init(const WindowCreateInfo& createInfo)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
	glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
	glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);
	
	if (createInfo.fullScreen) 
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		glfwWindow = glfwCreateWindow(fullScreenWidth, fullScreenHeight, createInfo.title, monitor, nullptr);
	}
	else
	{
		glfwWindow = glfwCreateWindow(windowedWidth, windowedHeight, createInfo.title, nullptr, nullptr);
	}

	assert(glfwWindow != nullptr, "cant create window");
	
	glfwSetWindowAspectRatio(glfwWindow, createInfo.aspectWidth, createInfo.aspectHeight);

	glfwSetWindowUserPointer(glfwWindow, this);
	glfwSetWindowSizeCallback(glfwWindow, createInfo.onResize);

	if (createInfo.icon != nullptr)
	{
		GLFWimage icon = loadIcon(createInfo.icon);
		glfwSetWindowIcon(glfwWindow, 1, &icon);
	}
	
	if (!createInfo.useCursor)
	{
		glfwCursor = getBlankCursor();
		glfwSetCursor(glfwWindow, glfwCursor);
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

GLFWimage Window::loadIcon(const char* path) const
{
	return {};
}

void Window::resize(uint32_t width, uint32_t height)
{
	aspectRatio = static_cast<double>(width) / static_cast<double>(height);
	glfwSetWindowSize(glfwWindow, width, height);
}

bool Window::isMinimized() const
{
	return _isMinimized;
}

bool Window::wasResized() const
{
	return _wasResized;
}

void Window::resetWasResized() const
{
	_wasResized = false;
}

void Window::onResize(GLFWwindow* glfwWindow, int32_t width, int32_t height)
{
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

	if (window->isFullScreen)
	{
		window->windowedWidth = width;
		window->windowedHeight = height;
	}
	else
	{
		window->fullScreenWidth = width;
		window->fullScreenHeight = height;
	}
	
	if (width != 0 && height != 0 && !window->_isMinimized)
		window->_wasResized = true;
	
	if (width == 0 && height == 0)
		window->_isMinimized = true;
	else
		window->_isMinimized = false;
}
