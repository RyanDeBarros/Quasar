#pragma once

#include <vector>
#include <functional>
#include <variant>

#include "Macros.h"

namespace Callback
{
	struct WindowSize
	{
		int width;
		int height;
		WindowSize(int width, int height) : width(width), height(height) {}
	};
	struct PathDrop
	{
		int num_paths;
		const char** paths;
		PathDrop(int num_paths, const char** paths) : num_paths(num_paths), paths(paths) {}
	};
	struct Key
	{
		int key;
		int scancode;
		int action;
		int mods;
		Key(int key, int scancode, int action, int mods) : key(key), scancode(scancode), action(action), mods(mods) {}
	};
	struct MouseButton
	{
		int button;
		int action;
		int mods;
		MouseButton(int button, int action, int mods) : button(button), action(action), mods(mods) {}
	};
	struct Scroll
	{
		double xoff;
		double yoff;
		Scroll(double xoff, double yoff) : xoff(xoff), yoff(yoff) {}
	};
}

struct WindowArgs
{
	const char* title;
	int width;
	int height;
	GLFWmonitor* monitor = nullptr;
	GLFWwindow* share = nullptr;

	WindowArgs(const char* title, int width, int height) : title(title), width(width), height(height) {}
};

struct StandardCursorArgs
{
	int shape;
	StandardCursorArgs(int shape) : shape(shape) {}
};

struct CustomCursorArgs
{
	GLFWimage* image;
	int xhot;
	int yhot;
	CustomCursorArgs(GLFWimage* image, int xhot, int yhot) : image(image), xhot(xhot), yhot(yhot) {}
};

struct CursorArgs
{
	std::variant<StandardCursorArgs, CustomCursorArgs> c;
	CursorArgs(int shape) : c(StandardCursorArgs(shape)) {}
	CursorArgs(GLFWimage* image, int xhot, int yhot) : c(CustomCursorArgs(image, xhot, yhot)) {}
	GLFWcursor* create() const;
};

struct Window
{
	GLFWwindow* window = nullptr;
	GLFWcursor* cursor = nullptr;

	std::vector<std::function<void(const Callback::WindowSize&)>> clbk_window_size;
	std::vector<std::function<void(const Callback::PathDrop&)>> clbk_path_drop;
	std::vector<std::function<void(const Callback::Key&)>> clbk_key;
	std::vector<std::function<void(const Callback::MouseButton&)>> clbk_mouse_button;
	std::vector<std::function<void(const Callback::Scroll&)>> clbk_scroll;

	Window(const WindowArgs& wargs, const CursorArgs& cargs);
	~Window();

	operator bool() const { return window != nullptr; }

	void set_cursor(const CursorArgs& cargs);
	void set_width(int width) const;
	void set_height(int height) const;
	void set_size(int width, int height) const;
	void set_title(const char* title) const;

	void focus() const { glfwMakeContextCurrent(window); }
	int width() const { int w, h; glfwGetWindowSize(window, &w, &h); return w; }
	int height() const { int w, h; glfwGetWindowSize(window, &w, &h); return h; }
	glm::ivec2 size() const { int w, h; glfwGetWindowSize(window, &w, &h); return { w, h }; }
	bool should_close() const { return glfwWindowShouldClose(window); }
	void swap_buffers() const { glfwSwapBuffers(window); }
	float cursor_x() const { double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos); return float(xpos); }
	float cursor_y() const { double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos); return float(ypos); }
	glm::vec2 cursor_pos() const { double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos); return { float(xpos), float(ypos) }; }

private:
	void destroy();
};

inline std::unordered_map<GLFWwindow*, Window*> Windows;
