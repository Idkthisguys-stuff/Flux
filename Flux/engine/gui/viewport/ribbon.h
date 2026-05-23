#pragma once
#include <string>
#include <vector>

#include "luaEngine.h"
#include "../texteditor.h"

#include "explorer.h"
#include "heiarchy.h"
#include "mechanics/SceneSerializer.h"

#include "viewport.h"
class TextEditor;	
class SceneSerializer;

namespace Flux {
	enum ToolMode {
		TOOL_MOVE = 0,
		TOOL_ROTATE = 1,
		TOOL_SCALE = 2
	};

	struct ProjectSettings {
		char    startupScene[128]  = "main.fscn";
		char    currentScene[128]  = "main.fscn";
		bool    useStartupScene    = false;
		int     runtimeWidth       = 1280;
		int     runtimeHeight      = 720;
	};

	class LuaEngine;

	class Explorer;

	class Viewport;

	class Ribbon {
		public:
			void renderRibbon();

			bool playToggledFrame = false;
			bool editorLocked = false;

			LuaEngine* luaEnginePtr = nullptr;
			::TextEditor* textEditorPtr = nullptr;
			Explorer* explorerPtr = nullptr;
			Heiarchy* heiarchyPtr = nullptr;
			Viewport* viewportPtr = nullptr;
			ProjectSettings projectSettings;

			bool showPreferences = false;
			bool showProjectSettings = false;

			void LoadPreferences();
			void SavePreferences();
			void SaveProjectSettings(const std::filesystem::path& projectRoot);
			void LoadProjectSettings(const std::filesystem::path& projectRoot);

		private:
			void drawFileMenu();
			void drawEditMenu();
			void drawProjectControls();
			void drawTransformTools();
			float camSpeed = 10.0f;
			float camSens  = 0.25f;
			bool  vsync    = true;
			int   theme    = 0;
	};
}