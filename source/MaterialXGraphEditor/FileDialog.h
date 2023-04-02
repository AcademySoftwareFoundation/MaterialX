//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <imgui.h>
#include <imfilebrowser.h>

#include <filesystem>

// A class to substitute the Imgui file browser with a native implementation.
// Implements just the class methods used in the GraphEditor to reduce code divergence
class ImguiFileDialogAdapter
{
  public:
    ImguiFileDialogAdapter(ImGuiFileBrowserFlags flags = 0);
    void SetTitle(std::string title);
    void SetTypeFilters(const std::vector<std::string>& typeFilters);
    void Open();
    bool IsOpened();
    void Display();
    bool HasSelected();
    std::filesystem::path GetSelected();
    void ClearSelected();

  private:
    ImGuiFileBrowserFlags flags_;
    std::string title_;
    bool openFlag_ = false;
    bool isOpened_ = false;
    std::set<std::filesystem::path> selectedFilenames_;
    std::vector<std::pair<std::string, std::string>> filetypes_;
};

// Copied from NanogUI/src/common.cpp
std::string file_dialog(const std::vector<std::pair<std::string, std::string>>& filetypes, bool save);
std::vector<std::string>
file_dialog(const std::vector<std::pair<std::string, std::string>>& filetypes,
            bool save, bool multiple);