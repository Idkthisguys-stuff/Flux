#pragma once

#include "../lib/portable-file-dialogs.h"
#include "imgui.h"
#include "texteditor.h"
#include "viewport.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "core/SceneSerializer.h"

#include "heiarchy.h"
#include "ribbon.h"

class TextEditor;

namespace Flux
{
enum class fileType
{
    Folder,
    Script,
    Text,
    Model,
    Texture,
    Scene,
    Project
};

struct virtualFile
{
    std::string name;
    fileType type;
    std::filesystem::path path;
    std::vector<virtualFile> children;
};

struct creationTask
{
    std::string defaultName;
    std::string extension;
    bool pending = false;
};

class Viewport;
class SceneSerializer;

class Ribbon;

class Assets
{
  public:
    void renderExplorer(Viewport &viewport);
    void TriggerCreateNewProject();
    void TriggerOpenProject();
    void TriggerSaveScene();
    void TriggerSaveSceneAs();
    void TriggerOpenScene();

    std::filesystem::path selectedAssetPath;
    void syncFiles(const std::filesystem::path &path, virtualFile &node);

    bool refreshRequested = false;
    std::filesystem::path refreshPath;
    std::filesystem::path activeFolderPath;
    virtualFile projectRoot = {"Project", fileType::Folder};
    creationTask pendingCreationTask;
    std::filesystem::path pathToDelete = "";

    std::vector<std::filesystem::path> filesWithBackups;

    void scanForBackups();

    ::TextEditor *textEditor = nullptr;

    Heiarchy h;

    Ribbon *ribbonPtr = nullptr;

    std::filesystem::path activeFilePath;
    std::string activeScriptName;
    bool isEditorVisible = false;
    bool isEditorUnsaved = false;

  private:
    void DrawVirtualNodes(virtualFile &file);
    void DrawFolderTree(virtualFile &file);
    void DrawAssetIcon(ImDrawList *drawList, ImVec2 pos, ImVec2 size, fileType type, unsigned int texID);
    virtualFile *FindFolderNode(virtualFile *current, const std::filesystem::path &path);

    void copyTemplateItem(const std::string &folderType, const std::string &templateName,
                          const std::string &targetBaseName, const std::string &ext);
    void createNewFolder(const std::string &name);

    std::filesystem::path resolveUniqueName(const std::filesystem::path &parentDir, const std::string &baseStem,
                                            const std::string &ext) const;

    virtualFile *renamingNode = nullptr;
    char renameBuffer[256] = {};

    bool showNewProjectModal = false;
    bool showOpenProjectModal = false;
    char newProjectNameBuf[256] = "NewGame";
    std::filesystem::path pendingTemplateRoot;
    std::filesystem::file_time_type lastFolderTime;
    char projectLocationBuf[512] = "";

    enum class ClipboardOp
    {
        None,
        Copy,
        Cut
    };
    ClipboardOp m_clipboardOp = ClipboardOp::None;
    std::filesystem::path m_clipboardSourcePath = "";

    void openInNativeExplorer(const std::filesystem::path& path);
};
} // namespace Flux