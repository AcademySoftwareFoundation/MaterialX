//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_FILEDIALOG_H
#define MATERIALX_FILEDIALOG_H

#include <MaterialXFormat/File.h>

namespace mx = MaterialX;

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

// A native file browser class, based on the implementation in NanoGUI.
class FileDialog
{
  public:
    FileDialog(int flags = 0);
    void setTitle(const std::string& title);
    void setTypeFilters(const mx::StringVec& typeFilters);
    void open();
    bool isOpened();
    void display();
    bool hasSelected();
    mx::FilePath getSelected();
    void clearSelected();

  private:
    int _flags;
    std::string _title;
    bool _openFlag = false;
    bool _isOpened = false;
    std::vector<mx::FilePath> _selectedFilenames;
    std::vector<std::pair<std::string, std::string>> _filetypes;
};

std::string launchFileDialog(const std::vector<std::pair<std::string, std::string>>& filetypes, bool save);
mx::StringVec launchFileDialog(const std::vector<std::pair<std::string, std::string>>& filetypes, bool save, bool multiple);

#endif
