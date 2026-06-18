#pragma once
#include "imgui.h"
#include "logic/data/Scenenode.h"
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Flux
{

class Model;

class Heiarchy
{
  public:
    std::vector<SceneNode> nodes;
    int selectedIndex = -1;

    std::vector<int> selectedIndices;
    int lastClickedIndex = -1;

    bool isSelected(int index) const;
    void selectNode(int index, bool ctrlDown, bool shiftDown);

    std::vector<std::vector<SceneNode>> undoStack;
    std::vector<std::vector<SceneNode>> redoStack;

    void setup();
    void renderHeiarchy(const std::filesystem::path &activeProjectPath);

    void AddModel(const std::string &path, const std::string &name = "");
    void AddLight(NodeType type, const std::string &name = "");

    void AddCamera(const std::string &name = "");

    void AddFolder(const std::string &name = "");
    void AddEmpty(const std::string &name = "");

    std::string GetUniqueName(const std::string &baseName);

    SceneNode *GetLightingNode();

    void PushUndoState();
    void Undo();
    void Redo();

    bool pendingDelete = false;
    void proccessPendingDeletes();

    bool pendingDuplicate = false;
    void proccessPendingDuplicates();

    std::vector<int> GetVisualOrder() const;
    void BuildVisualOrder(int parentIndex, std::vector<int> &order) const;
    std::vector<int> DuplicateNodeRecursive(int originalIndex, int newParentIndex);

  private:
    int renamingIndex = -1;
    char renameBuffer[128] = {};
    char searchBuffer[128] = {};

    std::unordered_map<std::string, std::shared_ptr<Model>> modelRegistry;
    std::shared_ptr<Model> GetOrLoadModel(const std::string &path);

    void DrawNode(int index);
    void FixIndicesAfterRemoval(int removedIndex);

    bool isDescendant(int nodeIndex, int potentialAncestorIndex) const;
};

} // namespace Flux