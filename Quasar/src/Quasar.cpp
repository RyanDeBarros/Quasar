#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "user/Machine.h"
#include "variety/IO.h"

static void glfw_error_callback(int error, const char* description)
{
	LOG << LOG.error << LOG.start_gl(error) << description << LOG.endl;
	QUASAR_ASSERT(false);
}

int main()
{
	IO.load_quasar_settings();
	stbi_set_flip_vertically_on_load(true);
	stbi_flip_vertically_on_write(true);
	//IO.load_workspace_preferences("D:/Projects/Visual Studio/Quasar/Quasar/ex/workspace1.toml", "workspace1"); // LATER workspace preferences GUI menu option
	if (glfwInit() != GLFW_TRUE)
		return -1;
	glfwSetErrorCallback(glfw_error_callback);
	if (!Machine.create_main_window())
	{
		glfwTerminate();
		return -1;
	}

	LOG.specify_logfile(FileSystem::workspace_path("quasar.log").c_str(), true) << LOG.target_logfile;
	QUASAR_GL(LOG << "Welcome to Quasar - GL_VERSION: " << glGetString(GL_VERSION) << LOG.nl << LOG.endl);
	// LATER add info and debug logging on most operations.

	Machine.init_renderer();

	Machine.recent_files = {"a.qua", "b.qua", "c.qua"};
	Machine.recent_image_files = {"1.png", "2.gif", "3.jpg"};

	// NOTE only one window, so no need to call bind_gui() at each frame.
	MainWindow->bind_gui();
	for (;;)
	{
		glfwPollEvents();
		if (Machine.should_exit())
			break;
		Machine.on_render();
	}
	Machine.destroy();
	glfwTerminate();
	return 0;
	// LATER when terminating in error, write log.
}
