#include <iostream>
#include "engine/Window.h"
#include <string>
#include <cstdlib>
#include <filesystem>

using namespace std;

int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	Flux::Window window(1920, 1080, "Flux");

	while (!window.shouldClose()) {
		window.clear(0.1f, 0.1f, 0.1f, 1.0f);
		window.update();
	}

	SDL_Quit();
	return 0;
}
