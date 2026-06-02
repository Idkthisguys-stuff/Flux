#pragma once
#include "imgui.h"

namespace Flux {
	class Heiarchy;

	class Properties {
	public:
		void renderProperties(Heiarchy* h = nullptr);
	};
}