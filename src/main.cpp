#include <cstdlib>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gpu.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

int main(int argc, char* argv[])
{
	// SDL initializers
	SDL_Window* window = SDL_CreateWindow("Flux Game Engine", 1270, 720, NULL);
	SDL_GPUShaderFormat shaderFormats = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL; // Supported SDL_GPU shader formats/files for rendering
	SDL_GPUDevice* gpu_device = SDL_CreateGPUDevice(shaderFormats, true, NULL);
	SDL_Event event;

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't start SDL3, error: %s", SDL_GetError());
		return -1;
	}

	if (!window) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation error: %s", SDL_GetError());
		SDL_DestroyGPUDevice(gpu_device);
		SDL_DestroyWindow(window);
		SDL_Quit();

		return -1;
	}

	if (!gpu_device) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "gpu_device is NULL! Reason: %s", SDL_GetError());
		SDL_DestroyGPUDevice(gpu_device);
		SDL_DestroyWindow(window);
		SDL_Quit();

		return -1;
	}

	if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window failed to claim GPU Device: %s", SDL_GetError());
		SDL_DestroyGPUDevice(gpu_device);
		SDL_DestroyWindow(window);
		SDL_Quit();

		return -1;
	}

	// ImGui initializers
	IMGUI_CHECKVERSION();
	ImGui::CreateContext(); // Context for SDL_GPU
	
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplSDL3_InitForSDLGPU(window);
	ImGui_ImplSDLGPU3_InitInfo initInfo = {};
	initInfo.Device = gpu_device;
	initInfo.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);
	initInfo.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;

	ImGui_ImplSDLGPU3_Init(&initInfo);

	while (1) { // Main Loop (obviously... -_-)
		while(SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);

			if (event.type == SDL_EVENT_QUIT) {
				break;
			}
		}

		ImGui_ImplSDLGPU3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow(); // Demo ImGui window

		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();
		SDL_GPUTexture* swapchainTexture;
		SDL_GPUCommandBuffer* cmd_buf = SDL_AcquireGPUCommandBuffer(gpu_device);

		SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buf, window, &swapchainTexture, NULL, NULL);
		ImGui_ImplSDLGPU3_PrepareDrawData(drawData, cmd_buf);

		SDL_GPUColorTargetInfo targetInfo = {};
	}

	SDL_DestroyGPUDevice(gpu_device);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
