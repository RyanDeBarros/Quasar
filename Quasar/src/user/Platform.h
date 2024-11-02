#pragma once

#include <imgui/imgui.h>

#include <vector>
#include <functional>
#include <unordered_map>

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
	E = GLFW_KEY_E,
	G = GLFW_KEY_G,
	I = GLFW_KEY_I,
	N = GLFW_KEY_N,
	O = GLFW_KEY_O,
	S = GLFW_KEY_S,
	Z = GLFW_KEY_Z,
	LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
	RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
	LEFT_CTRL = GLFW_KEY_LEFT_CONTROL,
	RIGHT_CTRL = GLFW_KEY_RIGHT_CONTROL,
	LEFT_ALT = GLFW_KEY_LEFT_ALT,
	RIGHT_ALT = GLFW_KEY_RIGHT_ALT,
	LEFT_SUPER = GLFW_KEY_LEFT_SUPER,
	RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER,
	CAPS_LOCK = GLFW_KEY_CAPS_LOCK,
	NUM_LOCK = GLFW_KEY_NUM_LOCK,
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

enum class IAction
{
	PRESS = GLFW_PRESS,
	RELEASE = GLFW_RELEASE,
	DOWN = GLFW_REPEAT
};

namespace Mods
{
	enum
	{
		SHIFT = GLFW_MOD_SHIFT,
		CONTROL = GLFW_MOD_CONTROL,
		ALT = GLFW_MOD_ALT,
		SUPER = GLFW_MOD_SUPER,
		CAPS_LOCK = GLFW_MOD_CAPS_LOCK,
		NUM_LOCK = GLFW_MOD_NUM_LOCK
	};
}

extern GLFWcursor* create_cursor(StandardCursor standard_cursor);
extern GLFWcursor* create_cursor(unsigned char* rgba_pixels, int width, int height, int xhot, int yhot);

struct InputEvent
{
	mutable bool consumed = false;
	~InputEvent() = default;
};

template<std::derived_from<InputEvent> Event>
struct InputEventHandler
{
	std::function<void(const Event&)> callback = [](const Event&) {};
	std::vector<InputEventHandler<Event>*> children;

	void on_callback(const Event& event) const
	{
		for (auto child : children)
		{
			child->on_callback(event);
			if (event.consumed)
				return;
		}
		callback(event);
	}

	void remove_child(const InputEventHandler<Event>* child)
	{
		children.erase(std::find(children.begin(), children.end(), child));
	}
};

struct WindowSizeEvent : public InputEvent
{
	int width;
	int height;
	WindowSizeEvent(int width, int height) : width(width), height(height) {}
	bool operator==(const WindowSizeEvent&) const = default;
};

typedef InputEventHandler<WindowSizeEvent> WindowSizeHandler;

struct PathDropEvent : public InputEvent
{
	int num_paths;
	const char** paths;
	PathDropEvent(int num_paths, const char** paths) : num_paths(num_paths), paths(paths) {}
	bool operator==(const PathDropEvent&) const = default;
};

typedef InputEventHandler<PathDropEvent> PathDropHandler;

struct KeyEvent : public InputEvent
{
	Key key;
	int scancode;
	IAction action;
	int mods;
	KeyEvent(int key, int scancode, int action, int mods) : key(Key(key)), scancode(scancode), action(IAction(action)), mods(mods) {}
	KeyEvent(Key key, int scancode, IAction action, int mods) : key(key), scancode(scancode), action(action), mods(mods) {}
	bool operator==(const KeyEvent&) const = default;
};

typedef InputEventHandler<KeyEvent> KeyHandler;

struct MouseButtonEvent : public InputEvent
{
	MouseButton button;
	IAction action;
	int mods;
	MouseButtonEvent(int button, int action, int mods) : button(MouseButton(button)), action(IAction(action)), mods(mods) {}
	MouseButtonEvent(MouseButton button, IAction action, int mods) : button(button), action(action), mods(mods) {}
	bool operator==(const MouseButtonEvent&) const = default;
};

typedef InputEventHandler<MouseButtonEvent> MouseButtonHandler;

struct ScrollEvent : public InputEvent
{
	float xoff;
	float yoff;
	ScrollEvent(double xoff, double yoff) : xoff(float(xoff)), yoff(float(yoff)) {}
	bool operator==(const ScrollEvent&) const = default;
};

typedef InputEventHandler<ScrollEvent> ScrollHandler;

struct WindowMaximizeEvent : public InputEvent
{
	bool maximized;
	WindowMaximizeEvent(bool maximized) : maximized(maximized) {}
	bool operator==(const WindowMaximizeEvent&) const = default;
};

typedef InputEventHandler<WindowMaximizeEvent> WindowMaximizeHandler;

struct DisplayScaleEvent : public InputEvent
{
	glm::vec2 scale;
	DisplayScaleEvent(float x_scale, float y_scale) : scale(x_scale, y_scale) {}
	bool operator==(const DisplayScaleEvent&) const = default;
};

typedef InputEventHandler<DisplayScaleEvent> DisplayScaleHandler;

struct Window
{
	GLFWwindow* window = nullptr;
	ImGuiContext* gui_context = nullptr;

	WindowSizeHandler root_window_size;
	PathDropHandler root_path_drop;
	KeyHandler root_key;
	MouseButtonHandler root_mouse_button;
	ScrollHandler root_scroll;
	WindowMaximizeHandler root_window_maximize;
	DisplayScaleHandler root_display_scale;

	Window(const char* title, int width, int height, bool enable_gui = true, ImFontAtlas* gui_font_atlas = nullptr, GLFWcursor* cursor = nullptr);
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
	bool bind_gui() const;
	void new_frame() const;
	void end_frame() const;
	bool should_close() const { return glfwWindowShouldClose(window); }
	void swap_buffers() const { glfwSwapBuffers(window); }
	void request_close() const { glfwSetWindowShouldClose(window, GLFW_TRUE); }

	int width() const { int w, h; glfwGetWindowSize(window, &w, &h); return w; }
	int height() const { int w, h; glfwGetWindowSize(window, &w, &h); return h; }
	glm::ivec2 size() const { int w, h; glfwGetWindowSize(window, &w, &h); return { w, h }; }
	void set_size_limits(int minwidth, int minheight, int maxwidth, int maxheight) const;

	float cursor_x() const { double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos); return float(xpos); }
	float cursor_y() const { double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos); return float(ypos); }
	glm::vec2 cursor_pos() const { double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos); return { float(xpos), height() - float(ypos)}; }
	void set_cursor_pos(const glm::vec2& pos) const { glfwSetCursorPos(window, pos.x, pos.y); }
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
	glm::vec2 display_scale() const { glm::vec2 ds{}; glfwGetWindowContentScale(window, &ds.x, &ds.y); return ds; }

	bool is_key_pressed(Key key) const { return glfwGetKey(window, int(key)) != int(IAction::RELEASE); }
	bool is_shift_pressed() const { return is_key_pressed(Key::LEFT_SHIFT) || is_key_pressed(Key::RIGHT_SHIFT); }
	bool is_ctrl_pressed() const { return is_key_pressed(Key::LEFT_CTRL) || is_key_pressed(Key::RIGHT_CTRL); }
	bool is_alt_pressed() const { return is_key_pressed(Key::LEFT_ALT) || is_key_pressed(Key::RIGHT_ALT); }
	bool is_super_pressed() const { return is_key_pressed(Key::LEFT_SUPER) || is_key_pressed(Key::RIGHT_SUPER); }
	bool is_mouse_button_pressed(MouseButton mb) const { return glfwGetMouseButton(window, int(mb)) != int(IAction::RELEASE); }

	void override_gui_cursor_change(bool _override) const;

private:
	int pre_fullscreen_x = 0;
	int pre_fullscreen_y = 0;
	int pre_fullscreen_width = 0;
	int pre_fullscreen_height = 0;
	bool fullscreen = false;
	bool maximized = false;

	KeyHandler window_maximizer;
	
	void destroy();
};

inline std::unordered_map<GLFWwindow*, Window*> Windows;
