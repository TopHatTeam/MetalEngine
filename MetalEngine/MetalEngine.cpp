// MetalEngine.cpp : Defines the entry point for the application.
//

#include "MetalEngine.h"
#include "src/headers/MVulkanRenderer.hpp"

#if defined(WIN32)
	#include <windows.h>
#elif defined(__linux__)

#endif

using namespace std;
using namespace engine::vulkan;

int main(int argc, char* argv[])
{
	MetalVulkanWindow* win = new MetalVulkanWindow();
	
	if (win->CreateSDLWindow(1280, 720, "MetalEngine") != 0)
	{
		return -1;
	}

	bool running = true;
	SDL_Event event;

	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				running = false;
			}
		}
	}

	/*The class decontructor does the SDL_DestroyWindow and SDL_Quit for us*/
	delete win;

	return 0;
}