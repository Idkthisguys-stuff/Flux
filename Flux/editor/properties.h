#pragma once
#include "imgui.h"

#include <variant>
#include <vector>
#include <cstring>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Flux {
	class Heiarchy;
	
	class Properties {
	public:
		void renderProperties(Heiarchy* h = nullptr);
	};
}