#include "Window.h"

static void window_size_callback(GLFWwindow* window, int width, int height)
{
	auto win = Windows[window];
	Callback::WindowSize args(width, height);
	for (const auto& f : win->clbk_window_size)
		f(args);
}

static void path_drop_callback(GLFWwindow* window, int num_paths, const char** paths)
{
	auto win = Windows[window];
	Callback::PathDrop args(num_paths, paths);
	for (const auto& f : win->clbk_path_drop)
		f(args);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto win = Windows[window];
	Callback::Key args(key, scancode, action, mods);
	for (const auto& f : win->clbk_key)
		f(args);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	auto win = Windows[window];
	Callback::MouseButton args(button, action, mods);
	for (const auto& f : win->clbk_mouse_button)
		f(args);
}

static void scroll_callback(GLFWwindow* window, double xoff, double yoff)
{
	auto win = Windows[window];
	Callback::Scroll args(xoff, yoff);
	for (const auto& f : win->clbk_scroll)
		f(args);
}

GLFWcursor* CursorArgs::create() const
{
	if (c.index() == 0)
		return glfwCreateStandardCursor(std::get<0>(c).shape);
	else if (c.index() == 1)
		return glfwCreateCursor(std::get<1>(c).image, std::get<1>(c).xhot, std::get<1>(c).yhot);
	else
		return nullptr;
}

Window::Window(const WindowArgs& wargs, const CursorArgs& cargs)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(wargs.width, wargs.height, wargs.title, wargs.monitor, wargs.share);
	if (!window)
	{
		destroy();
		return;
	}
	Windows[window] = this;
	focus();
	if (glewInit() != GLEW_OK)
	{
		destroy();
		return;
	}

	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetDropCallback(window, path_drop_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	cursor = cargs.create();
	glfwSetCursor(window, cursor);
}

Window::~Window()
{
	destroy();
}

void Window::destroy()
{
	if (window)
	{
		auto iter = Windows.find(window);
		if (iter != Windows.end())
			Windows.erase(iter);
		glfwDestroyWindow(window);
		window = nullptr;
	}
	if (cursor)
	{
		glfwDestroyCursor(cursor);
		cursor = nullptr;
	}
}

void Window::set_cursor(const CursorArgs& cargs)
{
	glfwDestroyCursor(cursor);
	cursor = cargs.create();
	glfwSetCursor(window, cursor);
}

void Window::set_width(int width) const
{
	glfwSetWindowSize(window, width, height());
}

void Window::set_height(int height) const
{
	glfwSetWindowSize(window, width(), height);
}

void Window::set_size(int width, int height) const
{
	glfwSetWindowSize(window, width, height);
}

void Window::set_title(const char* title) const
{
	glfwSetWindowTitle(window, title);
}
