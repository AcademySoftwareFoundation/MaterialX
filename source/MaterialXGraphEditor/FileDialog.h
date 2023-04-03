//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <MaterialXCore/Library.h>

#include <filesystem>

enum FileDialogFlags
{
    FileDialogFlags_SelectDirectory   = 1 << 0, // select directory instead of regular file
    FileDialogFlags_EnterNewFilename  = 1 << 1, // allow user to enter new filename when selecting regular file
    FileDialogFlags_NoModal           = 1 << 2, // file browsing window is modal by default. specify this to use a popup window
    FileDialogFlags_NoTitleBar        = 1 << 3, // hide window title bar
    FileDialogFlags_NoStatusBar       = 1 << 4, // hide status bar at the bottom of browsing window
    FileDialogFlags_CloseOnEsc        = 1 << 5, // close file browser when pressing 'ESC'
    FileDialogFlags_CreateNewDir      = 1 << 6, // allow user to create new directory
    FileDialogFlags_MultipleSelection = 1 << 7, // allow user to select multiple files. this will hide FileDialogFlags_EnterNewFilename
};

// A class to substitute the Imgui file browser with a native implementation.
// Implements just the class methods used in the GraphEditor to reduce code divergence
class FileDialog
{
  public:
    FileDialog(int flags = 0);
    void SetTitle(std::string title);
    void SetTypeFilters(const std::vector<std::string>& typeFilters);
    void Open();
    bool IsOpened();
    void Display();
    bool HasSelected();
    std::filesystem::path GetSelected();
    void ClearSelected();

  private:
    int flags_;
    std::string title_;
    bool openFlag_ = false;
    bool isOpened_ = false;
    std::set<std::filesystem::path> selectedFilenames_;
    std::vector<std::pair<std::string, std::string>> filetypes_;
};

// Copied from NanogUI/src/common.cpp
std::string launchFileDialog(const std::vector<std::pair<std::string, std::string>>& filetypes, bool save);
std::vector<std::string> launchFileDialog(const std::vector<std::pair<std::string, std::string>>& filetypes, bool save, bool multiple);
