#include "Platform.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "Machine.h"
#include "GUI.h"

GLFWcursor* create_cursor(StandardCursor standard_cursor)
{
	return glfwCreateStandardCursor(int(standard_cursor));
}

GLFWcursor* create_cursor(unsigned char* rgba_pixels, int width, int height, int xhot, int yhot)
{
	GLFWimage image{};
	image.pixels = rgba_pixels;
	image.width = width;
	image.height = height;
	return glfwCreateCursor(&image, xhot, yhot);
}

static void window_size_callback(GLFWwindow* window, int width, int height)
{
	auto win = Windows[window];
	Callback::WindowSize args(width, height);
	for (const auto& f : win->clbk_window_size.vec)
		f.f(args);
}

static void path_drop_callback(GLFWwindow* window, int num_paths, const char** paths)
{
	auto win = Windows[window];
	Callback::PathDrop args(num_paths, paths);
	for (const auto& f : win->clbk_path_drop.vec)
		f.f(args);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto win = Windows[window];
	Callback::Key args(key, scancode, action, mods);
	for (const auto& f : win->clbk_key.vec)
		f.f(args);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	auto win = Windows[window];
	Callback::MouseButton args(button, action, mods);
	for (const auto& f : win->clbk_mouse_button.vec)
		f.f(args);
}

static void scroll_callback(GLFWwindow* window, double xoff, double yoff)
{
	auto win = Windows[window];
	Callback::Scroll args(xoff, yoff);
	for (const auto& f : win->clbk_scroll.vec)
		f.f(args);
}

static void window_maximize_callback(GLFWwindow* window, int maximized)
{
	auto win = Windows[window];
	Callback::WindowMaximize args(maximized);
	for (const auto& f : win->clbk_window_maximize.vec)
		f.f(args);
}

static void display_scale_callback(GLFWwindow* window, float x_scale, float y_scale)
{
	auto win = Windows[window];
	Callback::DisplayScale args(x_scale, y_scale);
	for (const auto& f : win->clbk_display_scale.vec)
		f.f(args);
}

Window::Window(const char* title, int width, int height, bool enable_gui, ImFontAtlas* gui_font_atlas, GLFWcursor* cursor)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window)
	{
		destroy();
		return;
	}
	Windows[window] = this;
	focus_context();
	if (glewInit() != GLEW_OK)
	{
		destroy();
		return;
	}

	// NOTE due to potential conflict with ImGui input handling, only set glfw callbacks in Window constructor, since IMGui is initialized on window after.
	// For any and all actual callbacks, use clbk vectors.
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetDropCallback(window, path_drop_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetWindowMaximizeCallback(window, window_maximize_callback);
	glfwSetWindowContentScaleCallback(window, display_scale_callback);
	// LATER use better input system than callback vectors. Instead, use input hierarchy that can consume input events at different nodes.

	if (cursor)
		glfwSetCursor(window, cursor);

	if (enable_gui)
	{
		gui_context = ImGui::CreateContext(gui_font_atlas);
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init();
	}

	clbk_key.push_back([this](const Callback::Key& k) {
		if (k.action == IAction::RELEASE)
			--keys_pressed[k.key];
		else
			++keys_pressed[k.key];
		if (keys_pressed[k.key] == 0)
			keys_pressed.erase(k.key);
		});
	clbk_mouse_button.push_back([this](const Callback::MouseButton& mb) {
		if (mb.action == IAction::RELEASE)
			--mouse_buttons_pressed[mb.button];
		else
			++mouse_buttons_pressed[mb.button];
		if (mouse_buttons_pressed[mb.button] == 0)
			mouse_buttons_pressed.erase(mb.button);
		});
	clbk_window_maximize.push_back([this](const Callback::WindowMaximize& wm) {
		maximized = wm.maximized;
		});
	clbk_key.push_back([this](const Callback::Key& k) {
		if (k.key == Key::F11 && k.action == IAction::PRESS && !(k.mods & Mod::SHIFT))
		{
			if (!is_maximized())
				toggle_fullscreen();
			Machine.on_render();
		}
		else if (k.key == Key::ENTER && k.action == IAction::PRESS && k.mods & Mod::ALT)
		{
			if (!is_fullscreen())
				toggle_maximized();
			Machine.on_render();
		}
		else if (k.key == Key::ESCAPE && k.action == IAction::PRESS && !(k.mods & Mod::SHIFT))
		{
			set_fullscreen(false);
			set_maximized(false);
			Machine.on_render();
		}
		});
}

Window::~Window()
{
	destroy();
	if (gui_context)
	{
		ImGui::SetCurrentContext(gui_context);
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui::DestroyContext(gui_context);
	}
}

void Window::toggle_fullscreen()
{
	fullscreen = !fullscreen;
	if (fullscreen)
	{
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
		swap_buffers();
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
		glfwGetWindowPos(window, &pre_fullscreen_x, &pre_fullscreen_y);
		glfwGetWindowSize(window, &pre_fullscreen_width, &pre_fullscreen_height);
		glfwSetWindowMonitor(window, monitor, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
		swap_buffers();
	}
	else
	{
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
		swap_buffers();
		glfwSetWindowMonitor(window, nullptr, pre_fullscreen_x, pre_fullscreen_y, pre_fullscreen_width, pre_fullscreen_height, 0);
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
		swap_buffers();
	}
}

void Window::set_fullscreen(bool fullscreen_)
{
	if (fullscreen != fullscreen_)
		toggle_fullscreen();
}

void Window::toggle_maximized()
{
	maximized = !maximized;
	if (maximized)
	{
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
		swap_buffers();
		glfwMaximizeWindow(window);
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
		swap_buffers();
	}
	else
	{
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
		swap_buffers();
		glfwRestoreWindow(window);
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
		swap_buffers();
	}
}

void Window::set_maximized(bool maximized_)
{
	if (maximized != maximized_)
		toggle_maximized();
}

void Window::override_gui_cursor_change(bool _override) const
{
	ImGui::SetCurrentContext(gui_context);
	if (_override)
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	else
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
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
}

void Window::set_cursor(GLFWcursor* cursor) const
{
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

bool Window::bind_gui() const
{
	if (gui_context)
	{
		ImGui::SetCurrentContext(gui_context);
		return true;
	}
	return false;
}

void Window::new_frame() const
{
	QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	//focus_context(); NOTE only one window
}

void Window::end_frame() const
{
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	swap_buffers();
}

void Window::send_down_events() const
{
	int mods = 0;
	if (is_shift_pressed())
		mods |= Mod::SHIFT;
	if (is_ctrl_pressed())
		mods |= Mod::CONTROL;
	if (is_alt_pressed())
		mods |= Mod::ALT;
	if (is_super_pressed())
		mods |= Mod::SUPER;
	if (is_key_pressed(Key::CAPS_LOCK))
		mods |= Mod::CAPS_LOCK;
	if (is_key_pressed(Key::NUM_LOCK))
		mods |= Mod::NUM_LOCK;
		
	for (auto iter = keys_pressed.begin(); iter != keys_pressed.end(); ++iter)
	{
		Callback::Key key_event(iter->first, glfwGetKeyScancode(int(iter->first)), IAction::DOWN, mods);
		for (const auto& f : clbk_key_down.vec)
			f.f(key_event);
	}
	for (auto iter = mouse_buttons_pressed.begin(); iter != mouse_buttons_pressed.end(); ++iter)
	{
		Callback::MouseButton mouse_button_event(iter->first, IAction::DOWN, mods);
		for (const auto& f : clbk_mouse_button_down.vec)
			f.f(mouse_button_event);
	}
}

void Window::set_size_limits(int minwidth, int minheight, int maxwidth, int maxheight) const
{
	glfwSetWindowSizeLimits(window, minwidth, minheight, maxwidth, maxheight);
}
