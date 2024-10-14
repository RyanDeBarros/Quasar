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

enum class Key
{
	LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
	RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
	SPACE = GLFW_KEY_SPACE,
	F11 = GLFW_KEY_F11,
	ROW0 = GLFW_KEY_0,
	ENTER = GLFW_KEY_ENTER,
	ESCAPE = GLFW_KEY_ESCAPE,
};

enum class MouseButton
{
	LEFT = GLFW_MOUSE_BUTTON_LEFT,
	RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
	MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE
};

enum class Action
{
	PRESS = GLFW_PRESS,
	RELEASE = GLFW_RELEASE,
	DOWN = GLFW_REPEAT
};

enum class Mod
{
	SHIFT = GLFW_MOD_SHIFT,
	CONTROL = GLFW_MOD_CONTROL,
	ALT = GLFW_MOD_ALT,
	SUPER = GLFW_MOD_SUPER,
	CAPS_LOCK = GLFW_MOD_CAPS_LOCK,
	NUM_LOCK = GLFW_MOD_NUM_LOCK
};

constexpr int operator~(Mod m1) { return ~int(m1); }
constexpr int operator&(Mod m1, Mod m2) { return int(m1) & int(m2); }
constexpr int operator|(Mod m1, Mod m2) { return int(m1) | int(m2); }

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
		::Key key;
		int scancode;
		::Action action;
		::Mod mods;
		Key(int key, int scancode, int action, int mods) : key(::Key(key)), scancode(scancode), action(::Action(action)), mods(::Mod(mods)) {}
	};
	struct MouseButton
	{
		::MouseButton button;
		::Action action;
		::Mod mods;
		MouseButton(int button, int action, int mods) : button(::MouseButton(button)), action(::Action(action)), mods(::Mod(mods)) {}
	};
	struct Scroll
	{
		float xoff;
		float yoff;
		Scroll(double xoff, double yoff) : xoff(float(xoff)), yoff(float(yoff)) {}
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

	void focus_context() const { glfwMakeContextCurrent(window); }
	void focus() const { glfwFocusWindow(window); }
	bool should_close() const { return glfwWindowShouldClose(window); }
	void swap_buffers() const { glfwSwapBuffers(window); }
	
	int width() const { int w, h; glfwGetWindowSize(window, &w, &h); return w; }
	int height() const { int w, h; glfwGetWindowSize(window, &w, &h); return h; }
	glm::ivec2 size() const { int w, h; glfwGetWindowSize(window, &w, &h); return { w, h }; }
	float cursor_x() const { double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos); return float(xpos); }
	float cursor_y() const { double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos); return float(ypos); }
	glm::vec2 cursor_pos() const { double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos); return { float(xpos), height() - float(ypos)}; }
	glm::vec2 pos_center_normalized(const glm::vec2& pos) const { return glm::vec2{ -0.5f * width() + pos.x, 0.5f * height() - pos.y }; }

	MouseMode mouse_mode() const { return MouseMode(glfwGetInputMode(window, GLFW_CURSOR)); }
	void set_mouse_mode(MouseMode mouse_mode) const { glfwSetInputMode(window, GLFW_CURSOR, int(mouse_mode)); }
	bool raw_mouse_motion() const { return glfwGetInputMode(window, GLFW_RAW_MOUSE_MOTION) == GLFW_TRUE; }
	void set_raw_mouse_motion(bool raw_mouse_motion) const
	{ if (glfwRawMouseMotionSupported()) glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, raw_mouse_motion ? GLFW_TRUE : GLFW_FALSE); }
	void toggle_fullscreen();
	void set_fullscreen(bool fullscreen);
	bool is_fullscreen() const { return fullscreen; }
	void toggle_maximized();
	void set_maximized(bool maximized);
	bool is_maximized() const { return maximized; }
	bool is_key_pressed(Key key) const { return glfwGetKey(window, int(key)) == int(Action::PRESS); }
	bool is_shift_pressed() const { return is_key_pressed(Key::LEFT_SHIFT) || is_key_pressed(Key::RIGHT_SHIFT); }

private:
	int pre_fullscreen_x = 0;
	int pre_fullscreen_y = 0;
	int pre_fullscreen_width = 0;
	int pre_fullscreen_height = 0;
	bool fullscreen = false;
	bool maximized = false;

	void destroy();
};

inline std::unordered_map<GLFWwindow*, Window*> Windows;
