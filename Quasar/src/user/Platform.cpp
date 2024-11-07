#include "Platform.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "Machine.h"
#include "GUI.h"

static void window_size_callback(GLFWwindow* window, int width, int height)
{
	auto win = Windows[window];
	WindowSizeEvent event(width, height);
	win->root_window_size.on_callback(event);
}

static void path_drop_callback(GLFWwindow* window, int num_paths, const char** paths)
{
	auto win = Windows[window];
	PathDropEvent event(num_paths, paths);
	win->root_path_drop.on_callback(event);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto win = Windows[window];
	KeyEvent event(key, scancode, action, mods);
	win->root_key.on_callback(event);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	auto win = Windows[window];
	MouseButtonEvent event(button, action, mods);
	win->root_mouse_button.on_callback(event);
}

static void scroll_callback(GLFWwindow* window, double xoff, double yoff)
{
	auto win = Windows[window];
	ScrollEvent event(xoff, yoff);
	win->root_scroll.on_callback(event);
}

static void window_maximize_callback(GLFWwindow* window, int maximized)
{
	auto win = Windows[window];
	WindowMaximizeEvent event(maximized);
	win->root_window_maximize.on_callback(event);
}

static void display_scale_callback(GLFWwindow* window, float x_scale, float y_scale)
{
	auto win = Windows[window];
	DisplayScaleEvent event(x_scale, y_scale);
	win->root_display_scale.on_callback(event);
}

Cursor::Cursor(StandardCursor standard_cursor)
	: cursor(glfwCreateStandardCursor(int(standard_cursor)))
{
}

Cursor::Cursor(unsigned char* rgba_pixels, int width, int height, int xhot, int yhot)
{
	GLFWimage image{};
	image.pixels = rgba_pixels;
	image.width = width;
	image.height = height;
	cursor = glfwCreateCursor(&image, xhot, yhot);
}

Cursor::Cursor(Cursor&& other) noexcept
	: cursor(other.cursor)
{
	other.cursor = nullptr;
}

Cursor& Cursor::operator=(Cursor&& other) noexcept
{
	if (this != &other) // TODO add more safeguards in project
	{
		glfwDestroyCursor(cursor);
		cursor = other.cursor;
		other.cursor = nullptr;
	}
	return *this;
}

Cursor::~Cursor()
{
	glfwDestroyCursor(cursor);
}

Window::Window(const char* title, int width, int height, bool enable_gui, ImFontAtlas* gui_font_atlas, Cursor&& cursor)
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

	if (cursor.cursor)
	{
		glfwSetCursor(window, cursor.cursor);
		current_cursor = std::move(cursor);
	}
	else
	{
		current_cursor = StandardCursor::ARROW;
	}
	prev_cursor = StandardCursor::ARROW;


	if (enable_gui)
	{
		gui_context = ImGui::CreateContext(gui_font_atlas);
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init();
	}

	root_window_maximize.callback = [this](const WindowMaximizeEvent& wm) {
		maximized = wm.maximized;
		};

	// first callback in root to force
	window_maximizer.callback = [this](const KeyEvent& k) {
		if (k.key == Key::F11 && k.action == IAction::PRESS && !(k.mods & Mods::SHIFT))
		{
			k.consumed = true;
			if (!is_maximized())
				toggle_fullscreen();
			Machine.on_render();
		}
		else if (k.key == Key::ENTER && k.action == IAction::PRESS && k.mods & Mods::ALT)
		{
			k.consumed = true;
			if (!is_fullscreen())
				toggle_maximized();
			Machine.on_render();
		}
		};
	root_key.children.push_back(&window_maximizer);

	root_key.callback = [this](const KeyEvent& k) {
		if (k.key == Key::ESCAPE && k.action == IAction::PRESS && !(k.mods & Mods::SHIFT))
		{
			k.consumed = true;
			set_fullscreen(false);
			set_maximized(false);
			Machine.on_render();
		}
		};
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

bool Window::is_cursor_available(const WindowHandle* owner) const
{
	return owner && (!cursor_owner || cursor_owner == owner);
}

bool Window::owns_cursor(const WindowHandle* owner) const
{
	return owner == cursor_owner;
}

void Window::request_cursor(WindowHandle* owner, Cursor&& cursor)
{
	if (!owner)
		return;
	if (!cursor_owner || owner == cursor_owner)
	{
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		cursor_owner = owner;
		prev_cursor = std::move(current_cursor);
		current_cursor = std::move(cursor);
		glfwSetCursor(window, current_cursor.cursor);
		owner->flags |= WindowHandle::OWN_CURSOR;
	}
	else
		owner->flags &= ~WindowHandle::OWN_CURSOR;
}

void Window::release_cursor(WindowHandle* owner)
{
	if (owner && owner == cursor_owner)
	{
		owner->flags &= ~WindowHandle::OWN_CURSOR;
		cursor_owner = nullptr;
		current_cursor = std::move(prev_cursor);
		glfwSetCursor(window, current_cursor.cursor);
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
	}
}

void Window::eject_cursor()
{
	if (cursor_owner)
	{
		cursor_owner = nullptr;
		current_cursor = std::move(prev_cursor);
		glfwSetCursor(window, current_cursor.cursor);
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
	}
}

bool Window::is_mouse_mode_available(const WindowHandle* owner) const
{
	return owner && (!mouse_mode_owner || mouse_mode_owner == owner);
}

bool Window::owns_mouse_mode(const WindowHandle* owner) const
{
	return owner == mouse_mode_owner;
}

void Window::request_mouse_mode(WindowHandle* owner, MouseMode mouse_mode)
{
	if (!owner)
		return;
	if (!mouse_mode_owner || owner == mouse_mode_owner)
	{
		mouse_mode_owner = owner;
		prev_mouse_mode = current_mouse_mode;
		current_mouse_mode = mouse_mode;
		glfwSetInputMode(window, GLFW_CURSOR, int(current_mouse_mode));
		owner->flags |= WindowHandle::OWN_MOUSE_MODE;
	}
	else
		owner->flags &= ~WindowHandle::OWN_MOUSE_MODE;
}

void Window::release_mouse_mode(WindowHandle* owner)
{
	if (owner && owner == mouse_mode_owner)
	{
		owner->flags &= ~WindowHandle::OWN_MOUSE_MODE;
		mouse_mode_owner = nullptr;
		current_mouse_mode = prev_mouse_mode;
		glfwSetInputMode(window, GLFW_CURSOR, int(current_mouse_mode));
	}
}

void Window::eject_mouse_mode()
{
	if (mouse_mode_owner)
	{
		mouse_mode_owner = nullptr;
		current_mouse_mode = prev_mouse_mode;
		glfwSetInputMode(window, GLFW_CURSOR, int(current_mouse_mode));
	}
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

void Window::set_size_limits(int minwidth, int minheight, int maxwidth, int maxheight) const
{
	glfwSetWindowSizeLimits(window, minwidth, minheight, maxwidth, maxheight);
}
