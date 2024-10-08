#pragma once

#include <vector>
#include <functional>

#include "Macros.h"

enum class StandardCursor
{
	ARROW = GLFW_ARROW_CURSOR,
	IBEAM = GLFW_IBEAM_CURSOR,
	CROSSHAIR = GLFW_CROSSHAIR_CURSOR,
	HAND = GLFW_POINTING_HAND_CURSOR,
	RESIZE_EW = GLFW_RESIZE_EW_CURSOR,
	RESIZE_NS = GLFW_RESIZE_NS_CURSOR,
	RESIZE_NW_SE = GLFW_RESIZE_NWSE_CURSOR,
	RESIZE_NE_SW = GLFW_RESIZE_NESW_CURSOR,
	RESIZE_OMNI = GLFW_RESIZE_ALL_CURSOR,
	CANCEL = GLFW_NOT_ALLOWED_CURSOR
};

enum class MouseMode
{
	VISIBLE = GLFW_CURSOR_NORMAL,
	HIDDEN = GLFW_CURSOR_HIDDEN,
	VIRTUAL = GLFW_CURSOR_DISABLED,
	CAPTURED = GLFW_CURSOR_CAPTURED
};

extern GLFWcursor* create_cursor(StandardCursor standard_cursor);
extern GLFWcursor* create_cursor(unsigned char* rgba_pixels, int width, int height, int xhot, int yhot);

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

struct Window
{
	GLFWwindow* window = nullptr;

	std::vector<std::function<void(const Callback::WindowSize&)>> clbk_window_size;
	std::vector<std::function<void(const Callback::PathDrop&)>> clbk_path_drop;
	std::vector<std::function<void(const Callback::Key&)>> clbk_key;
	std::vector<std::function<void(const Callback::MouseButton&)>> clbk_mouse_button;
	std::vector<std::function<void(const Callback::Scroll&)>> clbk_scroll;

	Window(const char* title, int width, int height, GLFWcursor* cursor = nullptr, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);
	Window(const Window&) = delete;
	Window(Window&&) noexcept = delete;
	~Window();

	operator bool() const { return window != nullptr; }

	void set_cursor(GLFWcursor* cursor) const;
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

	MouseMode mouse_mode() const { return MouseMode(glfwGetInputMode(window, GLFW_CURSOR)); }
	void set_mouse_mode(MouseMode mouse_mode) const { glfwSetInputMode(window, GLFW_CURSOR, int(mouse_mode)); }
	bool raw_mouse_motion() const { return glfwGetInputMode(window, GLFW_RAW_MOUSE_MOTION) == GLFW_TRUE; }
	void set_raw_mouse_motion(bool raw_mouse_motion) const
	{ if (glfwRawMouseMotionSupported()) glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, raw_mouse_motion ? GLFW_TRUE : GLFW_FALSE); }

private:
	void destroy();
};

inline std::unordered_map<GLFWwindow*, Window*> Windows;
