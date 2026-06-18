#pragma once
#include <string>
#include <vector>

#include "scripting/luaEngine.h"
#include "texteditor.h"

#include "explorer.h"
#include "heiarchy.h"
#include "core/SceneSerializer.h"

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

	class Assets;

	class Viewport;

	class Ribbon {
		public:
			void renderRibbon();

			bool playToggledFrame = false;
			bool editorLocked = false;

			LuaEngine* luaEnginePtr = nullptr;
			::TextEditor* textEditorPtr = nullptr;
			Assets* explorerPtr = nullptr;
			Heiarchy* heiarchyPtr = nullptr;
			Viewport* viewportPtr = nullptr;
			ProjectSettings projectSettings;
			std::string lastProjectPath;

			bool showPreferences = false;
			bool showProjectSettings = false;

			void LoadPreferences();
			void SavePreferences();
			int GetTheme() const { return theme; }
			void SaveProjectSettings(const std::filesystem::path& projectRoot);
			void LoadProjectSettings(const std::filesystem::path& projectRoot);

			void TriggerSaveScene();
			void TriggerSaveSceneAs();
			void TriggerOpenScene();

		private:
			void drawFileMenu();
			void drawEditMenu();
			void drawGameObjectMenu();
			void drawProjectControls();
			void drawTransformTools();
			float camSpeed = 10.0f;
			float camSens  = 0.25f;
			bool  vsync    = true;
			int   theme    = 0;
	};
}