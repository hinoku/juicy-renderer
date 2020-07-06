#include "App.h"

#include "renderer/Framework.h"
#include "renderer/Window.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace JR {

App::App() {}

bool App::Start() {
	MM::AddModule<Window>();
	MM::AddModule<Framework>();

	if (!MM::Get<Window>().Create("Juicy", 200, 200)) {
		return false;
	}

	if (!MM::Get<Framework>().Initialize()) {
		return false;
	}

	if (!Run()) {
		return false;
	}

	MM::Unload<Framework>();
	MM::Unload<Window>();

	return true;
}

bool App::Run() {
	while (!MM::Get<Window>().ShouldClose()) {
		glfwPollEvents();

		MM::Get<Framework>().Render();
	}

	return true;
}

}  // namespace JR