#include "explorer.h"
#include "imgui.h"
#include "logic/Textureloader.h"
#include <cstring>
#include <iostream>

#ifdef _WIN32
#include <shellapi.h>
#include <windows.h>
#endif

namespace Flux
{
std::filesystem::path Assets::resolveUniqueName(const std::filesystem::path &parentDir, const std::string &baseStem,
                                                const std::string &ext) const
{
    std::filesystem::path candidate = parentDir / (baseStem + ext);
    int counter = 1;
    while (std::filesystem::exists(candidate))
    {
        candidate = parentDir / (baseStem + " (" + std::to_string(counter) + ")" + ext);
        ++counter;
    }
    return candidate;
}

void Assets::openInNativeExplorer(const std::filesystem::path &path)
{
#ifdef _WIN32
    std::wstring parameters = L"";

    if (!std::filesystem::is_directory(path))
    {
        parameters = L"/select,\"" + path.wstring() + L"\"";
        ShellExecuteW(NULL, L"open", L"explorer.exe", parameters.c_str(), NULL, SW_SHOWNORMAL);
    }
    else
    {
        ShellExecuteW(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
#endif
}

void Assets::DrawAssetIcon(ImDrawList *drawList, ImVec2 pos, ImVec2 size, fileType type, unsigned int texID)
{
    ImU32 folderCol = IM_COL32(230, 180, 50, 255); // Yellow/gold
    ImU32 fileCol = IM_COL32(200, 200, 200, 255);  // Grey
    ImU32 scriptCol = IM_COL32(80, 160, 240, 255); // Blue
    ImU32 modelCol = IM_COL32(50, 190, 100, 255);  // Green

    if (type == fileType::Texture && texID != 0)
    {
        drawList->AddImage(reinterpret_cast<void *>(static_cast<intptr_t>(texID)), pos,
                           ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(0, 1), ImVec2(1, 0));
        return;
    }

    if (type == fileType::Folder)
    {
        // Folder tab
        drawList->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x * 0.4f, pos.y + size.y * 0.2f), folderCol,
                                2.f);
        // Folder body
        drawList->AddRectFilled(ImVec2(pos.x, pos.y + size.y * 0.15f), ImVec2(pos.x + size.x, pos.y + size.y),
                                folderCol, 3.f);
    }
    else if (type == fileType::Script)
    {
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), scriptCol, 2.f);
        float lineY = pos.y + size.y * 0.3f;
        for (int i = 0; i < 3; i++)
        {
            drawList->AddLine(ImVec2(pos.x + size.x * 0.2f, lineY), ImVec2(pos.x + size.x * 0.8f, lineY),
                              IM_COL32(255, 255, 255, 200), 2.f);
            lineY += size.y * 0.2f;
        }
    }
    else if (type == fileType::Model)
    {
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), modelCol, 2.f);
        ImVec2 center(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
        float r = size.x * 0.25f;
        drawList->AddRect(ImVec2(center.x - r, center.y - r), ImVec2(center.x + r, center.y + r),
                          IM_COL32(255, 255, 255, 220), 0.f, 0, 1.5f);
    }
    else
    {
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), fileCol, 2.f);
        float lineY = pos.y + size.y * 0.3f;
        for (int i = 0; i < 3; i++)
        {
            drawList->AddLine(ImVec2(pos.x + size.x * 0.2f, lineY), ImVec2(pos.x + size.x * 0.8f, lineY),
                              IM_COL32(50, 50, 50, 200), 1.5f);
            lineY += size.y * 0.2f;
        }
    }
}

virtualFile *Assets::FindFolderNode(virtualFile *current, const std::filesystem::path &path)
{
    if (current->path == path)
        return current;
    for (auto &child : current->children)
    {
        if (child.type == fileType::Folder)
        {
            virtualFile *found = FindFolderNode(&child, path);
            if (found)
                return found;
        }
    }
    return nullptr;
}

void Assets::DrawFolderTree(virtualFile &file)
{
    if (file.type != fileType::Folder)
        return;
    if (file.name == ".flux")
        return;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
    bool isSelected = (activeFolderPath == file.path);
    if (isSelected)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool hasFolderChildren = false;
    for (const auto &child : file.children)
    {
        if (child.type == fileType::Folder && child.name != ".flux")
        {
            hasFolderChildren = true;
            break;
        }
    }
    if (!hasFolderChildren)
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    std::string uid = "##tree_" + std::to_string(reinterpret_cast<uintptr_t>(&file));
    bool node_open = ImGui::TreeNodeEx((file.name + uid).c_str(), flags);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        activeFolderPath = file.path;
    }

    if (node_open)
    {
        for (auto &child : file.children)
        {
            DrawFolderTree(child);
        }
        ImGui::TreePop();
    }
}

void Assets::renderExplorer(Viewport &viewport)
{
    ImGui::Begin("Assets");

    if (ImGui::IsWindowHovered())
    {
        ImGui::SetWindowFocus();
    }

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Delete) &&
        !selectedAssetPath.empty())
    {
        pathToDelete = selectedAssetPath;
        selectedAssetPath = "";
    }

    // --- Assets Control Bar ---
    bool hasProj = !activeFolderPath.empty();
    if (!hasProj)
        ImGui::BeginDisabled();

    if (ImGui::Button("Import..."))
    {
        auto selection = pfd::open_file("Import Assets", "",
                                        {"3D Models and Textures", "*.obj *.fbx *.png *.jpg *.jpeg *.bmp *.tga"})
                             .result();
        if (!selection.empty())
        {
            for (const auto &sel : selection)
            {
                std::filesystem::path srcPath(sel);
                std::string ext = srcPath.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                std::filesystem::path destFolder = activeFolderPath;

                std::filesystem::create_directories(destFolder);
                std::filesystem::path destPath = destFolder / srcPath.filename();
                try
                {
                    std::filesystem::copy_file(srcPath, destPath, std::filesystem::copy_options::overwrite_existing);
                    if (ext == ".obj")
                    {
                        std::filesystem::path mtlSrc = srcPath;
                        mtlSrc.replace_extension(".mtl");
                        if (std::filesystem::exists(mtlSrc))
                        {
                            std::filesystem::path mtlDest = destFolder / mtlSrc.filename();
                            std::filesystem::copy_file(mtlSrc, mtlDest,
                                                       std::filesystem::copy_options::overwrite_existing);
                        }
                    }
                    refreshRequested = true;
                    Output::addLog("Imported asset: " + srcPath.filename().string());
                }
                catch (const std::exception &e)
                {
                    Output::addLog("Import failed: " + std::string(e.what()));
                }
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Create Folder"))
    {
        createNewFolder("NewFolder");
    }
    ImGui::SameLine();
    if (ImGui::Button("Create Script"))
    {
        copyTemplateItem("scripts", "TemplateScript.lua", "NewScript", ".lua");
    }

    ImGui::SameLine();
    bool hasSelectedAsset = !selectedAssetPath.empty();
    if (!hasSelectedAsset)
        ImGui::BeginDisabled();
    if (ImGui::Button("Delete"))
    {
        pathToDelete = selectedAssetPath;
        selectedAssetPath = "";
    }
    if (!hasSelectedAsset)
        ImGui::EndDisabled();

    if (!hasProj)
        ImGui::EndDisabled();

    ImGui::Separator();
    // --------------------------

    static float refreshTimer = 0.0f;
    refreshTimer += ImGui::GetIO().DeltaTime;

    if (refreshTimer > 2.0f && std::filesystem::exists(activeFolderPath))
    {
        auto currentTime = std::filesystem::last_write_time(activeFolderPath);
        if (currentTime != lastFolderTime)
        {
            refreshRequested = true;
            lastFolderTime = currentTime;
        }
        refreshTimer = 0.0f;
    }

    if (hasProj)
    {
        // Back navigation bar & Breadcrumbs
        bool isAtRoot = (activeFolderPath == projectRoot.path);
        if (isAtRoot)
            ImGui::BeginDisabled();
        if (ImGui::Button("<- Back"))
        {
            activeFolderPath = activeFolderPath.parent_path();
            refreshRequested = true;
        }
        if (isAtRoot)
            ImGui::EndDisabled();

        ImGui::SameLine();
        std::string relPathStr = std::filesystem::relative(activeFolderPath, projectRoot.path).string();
        if (relPathStr == ".")
            relPathStr = "Project Root";
        ImGui::Text("Path: %s", relPathStr.c_str());
        ImGui::Separator();

        // Two-pane split view using Tables
        if (ImGui::BeginTable("##assetsSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
        {
            ImGui::TableSetupColumn("Folders", ImGuiTableColumnFlags_WidthFixed, 200.f);
            ImGui::TableSetupColumn("Content", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextRow();

            // Left Pane: Folder Sidebar
            ImGui::TableSetColumnIndex(0);
            ImGui::BeginChild("##folderTreeView", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            ImGuiTreeNodeFlags rootFlags =
                ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow;
            if (activeFolderPath == projectRoot.path)
                rootFlags |= ImGuiTreeNodeFlags_Selected;

            bool rootOpen = ImGui::TreeNodeEx("Project", rootFlags);
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                activeFolderPath = projectRoot.path;
            }
            if (rootOpen)
            {
                for (auto &child : projectRoot.children)
                {
                    DrawFolderTree(child);
                }
                ImGui::TreePop();
            }
            ImGui::EndChild();

            // Right Pane: Grid area
            ImGui::TableSetColumnIndex(1);
            ImGui::BeginChild("##contentGridView");

            virtualFile *currentFolderNode = FindFolderNode(&projectRoot, activeFolderPath);
            if (currentFolderNode)
            {
                float itemWidth = 80.f;
                float itemHeight = 100.f;
                float spacing = 16.f;
                float panelWidth = ImGui::GetContentRegionAvail().x;
                bool useCompactListView = (panelWidth < 300.f);
                int columns = useCompactListView ? 1 : std::max(1, (int)(panelWidth / (itemWidth + spacing)));

                if (ImGui::BeginTable("##gridTable", columns, ImGuiTableFlags_NoSavedSettings))
                {
                    if (useCompactListView)
                    {
                        ImGui::TableSetupColumn("##col", ImGuiTableColumnFlags_WidthStretch);
                    }
                    else
                    {
                        for (int i = 0; i < columns; i++)
                        {
                            ImGui::TableSetupColumn("##col", ImGuiTableColumnFlags_WidthFixed, itemWidth + spacing);
                        }
                    }

                    if (currentFolderNode && !currentFolderNode->children.empty())
                    {
                        for (size_t i = 0; i < currentFolderNode->children.size(); ++i)
                        {
                            if (i >= currentFolderNode->children.size())
                                break;

                            virtualFile &child = currentFolderNode->children[i];

                            if (child.name == ".flux" && child.type == fileType::Folder)
                                continue;

                            ImGui::TableNextColumn();
                            ImGui::PushID(child.name.c_str());

                            ImVec2 cardSize =
                                useCompactListView ? ImVec2(panelWidth, 24.f) : ImVec2(itemWidth, itemHeight);
                            ImVec2 startPos = ImGui::GetCursorScreenPos();
                            ImVec2 rectMax = useCompactListView ? ImVec2(startPos.x + panelWidth, startPos.y + 24.f)
                                                                : ImVec2(startPos.x + itemWidth,
                                                                         startPos.y + itemHeight); // rectmaxxing :)

                            if (renamingNode == &child)
                            {
                                ImGui::SetNextItemWidth(itemWidth);
                                ImGui::SetKeyboardFocusHere();
                                bool commit = ImGui::InputText("##inlineRename", renameBuffer, sizeof(renameBuffer),
                                                               ImGuiInputTextFlags_EnterReturnsTrue |
                                                                   ImGuiInputTextFlags_AutoSelectAll);
                                bool cancelled = ImGui::IsItemDeactivated() && !ImGui::IsItemActivated();
                                if (commit && renameBuffer[0] != '\0')
                                {
                                    std::filesystem::path parentDir = child.path.parent_path();
                                    std::string stem(renameBuffer);
                                    std::filesystem::path typedPath(stem);
                                    std::string typedStem = typedPath.stem().string();
                                    std::string typedExt = typedPath.extension().string();

                                    if (child.type == fileType::Folder)
                                    {
                                        typedStem = stem;
                                        typedExt = "";
                                    }
                                    if (typedExt.empty() && child.type != fileType::Folder)
                                        typedExt = child.path.extension().string();

                                    std::filesystem::path newPath = resolveUniqueName(parentDir, typedStem, typedExt);
                                    if (newPath != child.path)
                                    {
                                        try
                                        {
                                            std::filesystem::rename(child.path, newPath);
                                        }
                                        catch (const std::filesystem::filesystem_error &e)
                                        {
                                            std::cerr << "Rename failed: " << e.what() << "\n";
                                        }
                                    }
                                    renamingNode = nullptr;
                                    refreshRequested = true;
                                }
                                else if (cancelled)
                                {
                                    renamingNode = nullptr;
                                }
                            }
                            else
                            {
                                bool isSelected = (selectedAssetPath == child.path);

                                ImDrawList *drawList = ImGui::GetWindowDrawList();
                                ImVec2 rectMin = startPos;

                                ImVec2 mousePos = ImGui::GetIO().MousePos;
                                bool isHovered = (mousePos.x >= rectMin.x && mousePos.x <= rectMax.x &&
                                                  mousePos.y >= rectMin.y && mousePos.y <= rectMax.y) &&
                                                 ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

                                if (isSelected)
                                {
                                    drawList->AddRectFilled(rectMin, rectMax, ImColor(30, 100, 200, 80), 4.f);
                                    drawList->AddRect(rectMin, rectMax, ImColor(50, 150, 250, 255), 4.f, 0, 1.5f);
                                }
                                else if (isHovered)
                                {
                                    drawList->AddRectFilled(rectMin, rectMax, ImColor(100, 100, 100, 40), 4.f);
                                }

                                ImGui::BeginGroup();

                                if (useCompactListView)
                                {
                                    ImVec2 iconPos = ImGui::GetCursorScreenPos();
                                    iconPos.x += 4.f;
                                    iconPos.y += 2.f;
                                    ImGui::Dummy(ImVec2(20.f, 20.f));

                                    DrawAssetIcon(drawList, iconPos, ImVec2(16.f, 16.f), child.type, 0);

                                    ImGui::SameLine();
                                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.f);
                                    ImGui::TextUnformatted(child.name.c_str());
                                }
                                else
                                {
                                    // 1. Draw Icon
                                    ImVec2 iconPos = ImGui::GetCursorScreenPos();
                                    float iconSize = 48.f;
                                    iconPos.x += (itemWidth - iconSize) * 0.5f;

                                    ImGui::Dummy(ImVec2(itemWidth, iconSize));

                                    unsigned int texID = 0;
                                    if (child.type == fileType::Texture)
                                    {
                                        texID = TextureLoader::Load(child.path.string());
                                    }
                                    DrawAssetIcon(drawList, iconPos, ImVec2(iconSize, iconSize), child.type, texID);

                                    // 2. Draw Label below icon
                                    ImGui::Spacing();
                                    std::string dispName = child.name;
                                    bool hasBackup = std::find(filesWithBackups.begin(), filesWithBackups.end(),
                                                               child.path) != filesWithBackups.end();
                                    bool isCurrentlyOpen = (activeFilePath == child.path);
                                    if (isCurrentlyOpen && textEditor != nullptr && textEditor->IsTextChanged())
                                    {
                                        isEditorUnsaved = true;
                                    }
                                    if ((isCurrentlyOpen && isEditorUnsaved) || hasBackup)
                                    {
                                        dispName += " *";
                                    }

                                    std::string truncatedName = dispName;
                                    if (truncatedName.size() > 11)
                                    {
                                        truncatedName = truncatedName.substr(0, 9) + "..";
                                    }

                                    float textW = ImGui::CalcTextSize(truncatedName.c_str()).x;
                                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (itemWidth - textW) * 0.5f);
                                    ImGui::TextUnformatted(truncatedName.c_str());
                                }

                                ImGui::EndGroup();

                                if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                                {
                                    selectedAssetPath = child.path;
                                }

                                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                                {
                                    if (child.type == fileType::Folder)
                                    {
                                        activeFolderPath = child.path;
                                        refreshRequested = true;
                                    }
                                    else if (child.type == fileType::Script || child.type == fileType::Text)
                                    {
                                        if (textEditor != nullptr)
                                        {
                                            activeScriptName = child.name;
                                            activeFilePath = child.path;
                                            std::ifstream ifs(child.path);
                                            if (ifs.is_open())
                                            {
                                                std::string content((std::istreambuf_iterator<char>(ifs)),
                                                                    (std::istreambuf_iterator<char>()));
                                                textEditor->SetText(content);
                                                isEditorVisible = true;
                                                ifs.close();
                                            }
                                            ImGui::SetWindowFocus("Text Editor");
                                        }
                                    }
                                    else if (child.path.extension() == ".fscn")
                                    {
                                        if (ribbonPtr && ribbonPtr->heiarchyPtr)
                                        {
                                            SceneSerializer::Load(*ribbonPtr->heiarchyPtr, child.path,
                                                                  projectRoot.path);
                                            Output::addLog("Opened scene: " + child.name);
                                        }
                                    }
                                }

                                // Drag & drop source
                                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                                {
                                    std::string payloadPath = child.path.string();
                                    if (child.type == fileType::Model)
                                    {
                                        ImGui::SetDragDropPayload("MODEL_FILE", payloadPath.c_str(),
                                                                  payloadPath.size() + 1);
                                    }
                                    ImGui::SetDragDropPayload("EXPLORER_FILE", payloadPath.c_str(),
                                                              payloadPath.size() + 1);
                                    ImGui::Text("Moving %s", child.name.c_str());
                                    ImGui::EndDragDropSource();
                                }

                                if (child.type == fileType::Folder && ImGui::BeginDragDropTarget())
                                {
                                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("EXPLORER_FILE"))
                                    {
                                        std::filesystem::path srcPath((const char *)payload->Data);
                                        std::filesystem::path destPath = child.path / srcPath.filename();

                                        if (srcPath != child.path && srcPath != destPath)
                                        {
                                            try
                                            {
                                                std::filesystem::rename(srcPath, destPath);
                                                refreshRequested = true;
                                            }
                                            catch (const std::filesystem::filesystem_error &e)
                                            {
                                                std::cerr << "Failed to move file via drag/drop: " << e.what() << "\n";
                                            }
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }

                                // Context Menu
                                if (ImGui::BeginPopupContextItem("ItemContextMenu"))
                                {
                                    selectedAssetPath = child.path;

                                    if (ImGui::MenuItem("Show in File Explorer"))
                                    {
                                        openInNativeExplorer(child.path);
                                    }

                                    ImGui::Separator();

                                    if (ImGui::MenuItem("Rename", "F2"))
                                    {
                                        renamingNode = &child;
                                        std::string stemOnly =
                                            (child.type == fileType::Folder) ? child.name : child.path.stem().string();
                                        std::strncpy(renameBuffer, stemOnly.c_str(), sizeof(renameBuffer) - 1);
                                        renameBuffer[sizeof(renameBuffer) - 1] = '\0';
                                    }

                                    ImGui::Separator();

                                    if (ImGui::MenuItem("Copy", "Ctrl+C"))
                                    {
                                        m_clipboardSourcePath = child.path;
                                        m_clipboardOp = ClipboardOp::Copy;
                                    }

                                    if (ImGui::MenuItem("Cut", "Ctrl+X"))
                                    {
                                        m_clipboardSourcePath = child.path;
                                        m_clipboardOp = ClipboardOp::Cut;
                                    }

                                    ImGui::Separator();

                                    if (ImGui::MenuItem("Delete", "Del"))
                                    {
                                        pathToDelete = child.path;
                                        selectedAssetPath = "";
                                    }

                                    ImGui::EndPopup();
                                }
                            }
                            ImGui::PopID();
                        }
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();
            ImGui::EndTable();

            // Allow right-click on empty space inside the grid to create files
            if (ImGui::BeginPopupContextWindow("##emptyGridArea",
                                               ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::MenuItem("Show Current Folder in Explore"))
                {
                    openInNativeExplorer(activeFolderPath);
                }

                ImGui::Separator();

                if (ImGui::BeginMenu("Create.."))
                {
                    if (ImGui::MenuItem("Folder"))
                        createNewFolder("NewFolder");
                    if (ImGui::MenuItem("Script"))
                        copyTemplateItem("scripts", "TemplateScript.lua", "NewScript", ".lua");
                    ImGui::EndMenu();
                }

                ImGui::Separator();

                bool canPaste = !m_clipboardSourcePath.empty() && std::filesystem::exists(m_clipboardSourcePath);
                if (!canPaste)
                    ImGui::BeginDisabled();

                if (ImGui::MenuItem("Paste", "Ctrl+V"))
                {
                    std::filesystem::path destPath = activeFolderPath / m_clipboardSourcePath.filename();
                    destPath =
                        resolveUniqueName(activeFolderPath, destPath.stem().string(), destPath.extension().string());

                    try
                    {
                        if (m_clipboardOp == ClipboardOp::Copy)
                        {
                            std::filesystem::copy(m_clipboardSourcePath, destPath,
                                                  std::filesystem::copy_options::recursive);
                        }
                        else if (m_clipboardOp == ClipboardOp::Cut)
                        {
                            std::filesystem::rename(m_clipboardSourcePath, destPath);
                            m_clipboardSourcePath = ""; // clear clipboard after a cut-paste move
                            m_clipboardOp = ClipboardOp::None;
                        }
                        refreshRequested = true;
                    }
                    catch (const std::filesystem::filesystem_error &e)
                    {
                        std::cerr << "Paste operation failed: " << e.what() << "\n";
                    }
                }

                if (!canPaste)
                    ImGui::EndDisabled();

                ImGui::EndPopup();
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("EXPLORER_FILE"))
                {
                    std::filesystem::path srcPath((const char *)payload->Data);
                    std::filesystem::path destPath = activeFolderPath / srcPath.filename();

                    if (srcPath.parent_path() != activeFolderPath)
                    {
                        try
                        {
                            std::filesystem::rename(srcPath, destPath);
                            refreshRequested = true;
                        }
                        catch (const std::filesystem::filesystem_error &e)
                        {
                            std::cerr << "Failed to drop file into window background: " << e.what() << "\n";
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }
    }
    else
    {
        ImGui::TextDisabled("No project loaded.");
    }

    if (showNewProjectModal)
    {
        ImGui::OpenPopup("Name Your Project");
        showNewProjectModal = false;
    }

    float modalW = ImGui::GetMainViewport()->Size.x * 0.30f;
    if (modalW < 320.f)
        modalW = 320.f;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(modalW, 0.f), ImGuiCond_Always);

    if (ImGui::BeginPopupModal("Name Your Project", nullptr, ImGuiWindowFlags_NoResize))
    {
        if (projectLocationBuf[0] == '\0')
        {
            const char *home = std::getenv("HOME");
            if (!home)
                home = std::getenv("USERPROFILE");

            if (home)
            {
                std::filesystem::path defaultPath = std::filesystem::path(home) / "FluxProjects";
                std::strncpy(projectLocationBuf, defaultPath.string().c_str(), sizeof(projectLocationBuf) - 1);
            }
        }

        ImGui::Text("Project Name:");
        ImGui::InputText("##projname", newProjectNameBuf, sizeof(newProjectNameBuf));

        ImGui::Spacing();
        ImGui::Text("Save Location:");
        ImGui::InputText("##projlocation", projectLocationBuf, sizeof(projectLocationBuf));

        ImGui::SameLine();

        if (ImGui::Button("Browse..."))
        {
            auto selection = pfd::select_folder("Choose where to save your project").result();

            if (!selection.empty())
            {
                std::strncpy(projectLocationBuf, selection.c_str(), sizeof(projectLocationBuf) - 1);
            }
        }

        ImGui::Spacing();
        bool nameEmpty = (newProjectNameBuf[0] == '\0');
        float btnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

        if (nameEmpty)
            ImGui::BeginDisabled();

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            std::filesystem::path docsBase = std::filesystem::path(projectLocationBuf);
            std::filesystem::path docsPath = resolveUniqueName(docsBase, std::string(newProjectNameBuf), "");

            try
            {
                std::filesystem::create_directories(docsPath);
                std::filesystem::copy(pendingTemplateRoot, docsPath, std::filesystem::copy_options::recursive);

                activeFolderPath = docsPath;
                projectRoot.path = docsPath;
                projectRoot.name = docsPath.filename().string();
                syncFiles(docsPath, projectRoot);

                if (ribbonPtr)
                {
                    ribbonPtr->lastProjectPath = docsPath.string();
                    ribbonPtr->SavePreferences();

                    std::strncpy(ribbonPtr->projectSettings.startupScene, "scene.fscn",
                                 sizeof(ribbonPtr->projectSettings.startupScene) - 1);
                    std::strncpy(ribbonPtr->projectSettings.currentScene, "scene.fscn",
                                 sizeof(ribbonPtr->projectSettings.currentScene) - 1);
                    ribbonPtr->projectSettings.useStartupScene = false;
                    ribbonPtr->projectSettings.runtimeWidth = 1280;
                    ribbonPtr->projectSettings.runtimeHeight = 720;
                    ribbonPtr->SaveProjectSettings(docsPath);

                    if (ribbonPtr->heiarchyPtr)
                    {
                        ribbonPtr->heiarchyPtr->nodes.clear();
                        ribbonPtr->heiarchyPtr->setup();
                        std::filesystem::path scenePath = docsPath / "scene.fscn";
                        SceneSerializer::Save(*ribbonPtr->heiarchyPtr, scenePath.string(), docsPath);
                    }
                }
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                std::cerr << "FAILED: " << e.what() << "\n";
            }
            ImGui::CloseCurrentPopup();
        }

        if (nameEmpty)
            ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(btnW, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();

    if (refreshRequested)
    {
        if (!projectRoot.path.empty() && std::filesystem::exists(projectRoot.path))
        {
            renamingNode = nullptr;
            syncFiles(projectRoot.path, projectRoot);
        }
        refreshRequested = false;
    }

    if (!pathToDelete.empty())
    {
        try
        {
            if (std::filesystem::exists(pathToDelete))
                std::filesystem::remove_all(pathToDelete);
            if (!projectRoot.path.empty() && std::filesystem::exists(projectRoot.path))
            {
                renamingNode = nullptr;
                syncFiles(projectRoot.path, projectRoot);
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << "Delete failed: " << e.what() << "\n";
        }
        pathToDelete = "";
    }
}

void Assets::syncFiles(const std::filesystem::path &path, virtualFile &node)
{
    node.children.clear();
    node.path = path;

    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        virtualFile child;
        child.name = entry.path().filename().string();
        child.path = entry.path();

        if (entry.is_directory())
        {
            child.type = fileType::Folder;
            syncFiles(entry.path(), child);
        }
        else
        {
            std::string ext = entry.path().extension().string();
            if (ext == ".lua")
                child.type = fileType::Script;
            else if (ext == ".txt")
                child.type = fileType::Text;
            else if (ext == ".obj" || ext == ".fbx")
                child.type = fileType::Model;
            else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
                child.type = fileType::Texture;
            else if (ext == ".fscn")
                child.type = fileType::Scene;
            else if (ext == ".flux")
                child.type = fileType::Project;
            else
                child.type = fileType::Text;
        }
        node.children.push_back(child);
    }
}

void Assets::copyTemplateItem(const std::string &folderType, const std::string &templateName,
                              const std::string &targetBaseName, const std::string &ext)
{
    std::filesystem::path searchPath = std::filesystem::current_path();
    std::filesystem::path absoluteTemplatePath;
    bool found = false;

    for (int i = 0; i < 5; ++i)
    {
        auto p = PathHelper::GetAssetPath("templates/File_Templates/scripts/TemplateScript.lua");
        if (std::filesystem::exists(p))
        {
            absoluteTemplatePath = p;
            found = true;
            break;
        }
        if (searchPath.has_parent_path())
            searchPath = searchPath.parent_path();
        else
            break;
    }

    if (!found)
    {
        std::cerr << "ERROR: Could not find template: " << templateName << "\n";
        return;
    }

    std::filesystem::path targetPath = resolveUniqueName(activeFolderPath, targetBaseName, ext);

    try
    {
        std::filesystem::copy_file(absoluteTemplatePath, targetPath, std::filesystem::copy_options::overwrite_existing);
        refreshRequested = true;
        refreshPath = activeFolderPath;
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "File Delivery Failed: " << e.what() << "\n";
    }
}

void Assets::createNewFolder(const std::string &name)
{
    std::filesystem::path targetPath = resolveUniqueName(activeFolderPath, name, "");

    try
    {
        std::filesystem::create_directories(targetPath);
        refreshRequested = true;
        refreshPath = activeFolderPath;
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "Folder Creation Failed: " << e.what() << "\n";
    }
}

void Assets::scanForBackups()
{
    filesWithBackups.clear();
    std::filesystem::path backupDir = activeFolderPath / ".flux" / "backups";

    if (std::filesystem::exists(backupDir))
    {
        for (const auto &entry : std::filesystem::directory_iterator(backupDir))
        {
            if (entry.path().extension() == ".tmp")
            {
                std::string originalName = entry.path().stem().string();
                filesWithBackups.push_back(activeFolderPath / originalName);
            }
        }
    }
}

void Assets::TriggerCreateNewProject()
{
    std::filesystem::path searchPath = std::filesystem::current_path();
    bool found = false;
    for (int i = 0; i < 5; ++i)
    {
        auto candidate = PathHelper::GetAssetPath("templates/Project_Templates/base_game_folder_lua");
        if (std::filesystem::exists(candidate))
        {
            pendingTemplateRoot = candidate;
            found = true;
            break;
        }
        if (searchPath.has_parent_path())
            searchPath = searchPath.parent_path();
        else
            break;
    }
    if (found)
    {
        std::strncpy(newProjectNameBuf, "NewGame", sizeof(newProjectNameBuf) - 1);
        newProjectNameBuf[sizeof(newProjectNameBuf) - 1] = '\0';
        showNewProjectModal = true;
    }
    else
    {
        std::cerr << "ERROR: template folder not found near " << std::filesystem::current_path() << "\n";
    }
}

void Assets::TriggerOpenProject()
{
    auto selection = pfd::open_file("Open Flux Project", "", {"Flux Project", "*.flux"}).result();
    if (!selection.empty())
    {
        std::filesystem::path selectedFile(selection[0]);
        std::filesystem::path selectedPath = selectedFile.parent_path();

        activeFolderPath = selectedPath;
        projectRoot.path = selectedPath;
        projectRoot.name = selectedPath.filename().string();

        selectedAssetPath = "";
        renamingNode = nullptr;

        syncFiles(selectedPath, projectRoot);
        scanForBackups();

        if (ribbonPtr)
        {
            ribbonPtr->lastProjectPath = selectedPath.string();
            ribbonPtr->SavePreferences();
            ribbonPtr->LoadProjectSettings(selectedPath);
            std::string sceneName = std::strlen(ribbonPtr->projectSettings.currentScene) > 0
                                        ? ribbonPtr->projectSettings.currentScene
                                        : "scene.fscn";
            std::filesystem::path scenePath = selectedPath / sceneName;
            if (std::filesystem::exists(scenePath) && ribbonPtr->heiarchyPtr)
            {
                SceneSerializer::Load(*ribbonPtr->heiarchyPtr, scenePath, selectedPath);
            }
        }
    }
}
} // namespace Flux