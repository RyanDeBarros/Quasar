#pragma once

#include <imgui/imgui.h>

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
	template<typename Clbck>
	struct Handler
	{
		std::function<void(const Clbck&)> f;
		void* owner = nullptr;
	};
	template<typename Clbck>
	struct HandlerVector
	{
		std::vector<Handler<Clbck>> vec;

		HandlerVector() = default;

		void push_back(const std::function<void(const Clbck&)>& f) { vec.push_back({ f }); }
		void push_back(std::function<void(const Clbck&)>&& f) { vec.push_back({ std::move(f) }); }
		void push_back(const std::function<void(const Clbck&)>& f, void* owner) { vec.push_back({ f, owner }); }
		void push_back(std::function<void(const Clbck&)>&& f, void* owner) { vec.push_back({ std::move(f), owner }); }
		void remove_at_owner(void* owner)
		{
			vec.erase(std::remove_if(vec.begin(), vec.end(), [owner](const Handler<Clbck>& h) { return h.owner == owner; }), vec.end());
		}
	};
	struct WindowSize
	{
		int width;
		int height;
		WindowSize(int width, int height) : width(width), height(height) {}
		bool operator==(const WindowSize&) const = default;
	};
	struct PathDrop
	{
		int num_paths;
		const char** paths;
		PathDrop(int num_paths, const char** paths) : num_paths(num_paths), paths(paths) {}
		bool operator==(const PathDrop&) const = default;
	};
	struct Key
	{
		::Key key;
		int scancode;
		::IAction action;
		::Mod mods;
		Key(int key, int scancode, int action, int mods) : key(::Key(key)), scancode(scancode), action(::IAction(action)), mods(::Mod(mods)) {}
		bool operator==(const Key&) const = default;
	};
	struct MouseButton
	{
		::MouseButton button;
		::IAction action;
		::Mod mods;
		MouseButton(int button, int action, int mods) : button(::MouseButton(button)), action(::IAction(action)), mods(::Mod(mods)) {}
		bool operator==(const MouseButton&) const = default;
	};
	struct Scroll
	{
		float xoff;
		float yoff;
		Scroll(double xoff, double yoff) : xoff(float(xoff)), yoff(float(yoff)) {}
		bool operator==(const Scroll&) const = default;
	};
	struct WindowMaximize
	{
		bool maximized;
		WindowMaximize(bool maximized) : maximized(maximized) {}
		bool operator==(const WindowMaximize&) const = default;
	};
	struct DisplayScale
	{
		glm::vec2 scale;
		DisplayScale(float x_scale, float y_scale) : scale(x_scale, y_scale) {}
		bool operator==(const DisplayScale&) const = default;
	};
}

struct Window
{
	GLFWwindow* window = nullptr;
	ImGuiContext* gui_context = nullptr;

	Callback::HandlerVector<Callback::WindowSize> clbk_window_size;
	Callback::HandlerVector<Callback::PathDrop> clbk_path_drop;
	Callback::HandlerVector<Callback::Key> clbk_key;
	Callback::HandlerVector<Callback::Key> clbk_key_down;
	Callback::HandlerVector<Callback::MouseButton> clbk_mouse_button;
	Callback::HandlerVector<Callback::MouseButton> clbk_mouse_button_down;
	Callback::HandlerVector<Callback::Scroll> clbk_scroll;
	Callback::HandlerVector<Callback::WindowMaximize> clbk_window_maximize;
	Callback::HandlerVector<Callback::DisplayScale> clbk_display_scale;

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

	void send_down_events() const;
	
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

	bool is_key_pressed(Key key) const { return glfwGetKey(window, int(key)) == int(IAction::PRESS); }
	bool is_shift_pressed() const { return is_key_pressed(Key::LEFT_SHIFT) || is_key_pressed(Key::RIGHT_SHIFT); }
	bool is_ctrl_pressed() const { return is_key_pressed(Key::LEFT_CTRL) || is_key_pressed(Key::RIGHT_CTRL); }
	bool is_alt_pressed() const { return is_key_pressed(Key::LEFT_ALT) || is_key_pressed(Key::RIGHT_ALT); }

	void override_gui_cursor_change(bool _override) const;

private:
	int pre_fullscreen_x = 0;
	int pre_fullscreen_y = 0;
	int pre_fullscreen_width = 0;
	int pre_fullscreen_height = 0;
	bool fullscreen = false;
	bool maximized = false;

	void destroy();

	std::vector<Callback::Key> key_downs;
	std::vector<Callback::MouseButton> mouse_downs;
};

inline std::unordered_map<GLFWwindow*, Window*> Windows;
