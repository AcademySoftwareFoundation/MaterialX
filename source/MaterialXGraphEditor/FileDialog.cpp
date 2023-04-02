//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "FileDialog.h"

#if defined(_WIN32)
    #  ifndef NOMINMAX
        #  define NOMINMAX 1
    #  endif
    #  include <windows.h>
    #  include <commdlg.h>
#endif

ImguiFileDialogAdapter::ImguiFileDialogAdapter(ImGuiFileBrowserFlags flags) :
    flags_(flags)
{
}

void ImguiFileDialogAdapter::SetTitle(std::string title)
{
    title_ = title;
}

void ImguiFileDialogAdapter::SetTypeFilters(const std::vector<std::string>& typeFilters)
{
    filetypes_.clear();

    for (auto typefilter : typeFilters)
    {
        std::string minus_ext = typefilter.substr(1, typefilter.size() - 1);
        std::pair<std::string, std::string> filterPair = { minus_ext, minus_ext };
        filetypes_.push_back(filterPair);
    }
}

void ImguiFileDialogAdapter::Open()
{
    ClearSelected();
    openFlag_ = true;
}

bool ImguiFileDialogAdapter::IsOpened()
{
    return isOpened_;
}

bool ImguiFileDialogAdapter::HasSelected()
{
    return !selectedFilenames_.empty();
}

std::filesystem::path ImguiFileDialogAdapter::GetSelected()
{
    if (selectedFilenames_.empty())
    {
        return {};
    }

    return *selectedFilenames_.begin();
}

void ImguiFileDialogAdapter::ClearSelected()
{
    selectedFilenames_.clear();
}

void ImguiFileDialogAdapter::Display()
{
    // Only call the dialog if it's not already displayed
    if (!openFlag_ || isOpened_)
    {
        return;
    }
    openFlag_ = false;

    // Check if we want to save or open
    bool save = !(flags_ & ImGuiFileBrowserFlags_SelectDirectory) &&
                (flags_ & ImGuiFileBrowserFlags_EnterNewFilename);

    auto path = file_dialog(filetypes_, save);
    if (!path.empty())
    {
        selectedFilenames_.insert(path);
    }

    isOpened_ = false;
}

// Copied from NanogUI/src/common.cpp
std::string file_dialog(const std::vector<std::pair<std::string, std::string>>& filetypes, bool save)
{
    auto result = file_dialog(filetypes, save, false);
    return result.empty() ? "" : result.front();
}

#if !defined(__APPLE__)
std::vector<std::string> file_dialog(const std::vector<std::pair<std::string, std::string>>& filetypes, bool save, bool multiple)
{
    static const int FILE_DIALOG_MAX_BUFFER = 16384;
    if (save && multiple)
    {
        throw std::invalid_argument("save and multiple must not both be true.");
    }

    #if defined(EMSCRIPTEN)
    throw std::runtime_error("Opening files is not supported when NanoGUI is compiled via Emscripten");
    #elif defined(_WIN32)
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    char tmp[FILE_DIALOG_MAX_BUFFER];
    ofn.lpstrFile = tmp;
    ZeroMemory(tmp, FILE_DIALOG_MAX_BUFFER);
    ofn.nMaxFile = FILE_DIALOG_MAX_BUFFER;
    ofn.nFilterIndex = 1;

    std::string filter;

    if (!save && filetypes.size() > 1)
    {
        filter.append("Supported file types (");
        for (size_t i = 0; i < filetypes.size(); ++i)
        {
            filter.append("*.");
            filter.append(filetypes[i].first);
            if (i + 1 < filetypes.size())
                filter.append(";");
        }
        filter.append(")");
        filter.push_back('\0');
        for (size_t i = 0; i < filetypes.size(); ++i)
        {
            filter.append("*.");
            filter.append(filetypes[i].first);
            if (i + 1 < filetypes.size())
                filter.append(";");
        }
        filter.push_back('\0');
    }
    for (auto pair : filetypes)
    {
        filter.append(pair.second);
        filter.append(" (*.");
        filter.append(pair.first);
        filter.append(")");
        filter.push_back('\0');
        filter.append("*.");
        filter.append(pair.first);
        filter.push_back('\0');
    }
    filter.push_back('\0');
    ofn.lpstrFilter = filter.data();

    if (save)
    {
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
        if (GetSaveFileNameA(&ofn) == FALSE)
            return {};
    }
    else
    {
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (multiple)
            ofn.Flags |= OFN_ALLOWMULTISELECT;
        if (GetOpenFileNameA(&ofn) == FALSE)
            return {};
    }

    size_t i = 0;
    std::vector<std::string> result;
    while (tmp[i] != '\0')
    {
        result.emplace_back(&tmp[i]);
        i += result.back().size() + 1;
    }

    if (result.size() > 1)
    {
        for (i = 1; i < result.size(); ++i)
        {
            result[i] = result[0] + "\\" + result[i];
        }
        result.erase(begin(result));
    }

    if (save && ofn.nFilterIndex > 0)
    {
        auto ext = filetypes[ofn.nFilterIndex - 1].first;
        if (ext != "*")
        {
            ext.insert(0, ".");

            auto& name = result.front();
            if (name.size() <= ext.size() ||
                name.compare(name.size() - ext.size(), ext.size(), ext) != 0)
            {
                name.append(ext);
            }
        }
    }

    return result;
    #else
    char buffer[FILE_DIALOG_MAX_BUFFER];
    buffer[0] = '\0';

    std::string cmd = "zenity --file-selection ";
    // The safest separator for multiple selected paths is /, since / can never occur
    // in file names. Only where two paths are concatenated will there be two / following
    // each other.
    if (multiple)
        cmd += "--multiple --separator=\"/\" ";
    if (save)
        cmd += "--save ";
    cmd += "--file-filter=\"";
    for (auto pair : filetypes)
        cmd += "\"*." + pair.first + "\" ";
    cmd += "\"";
    FILE* output = popen(cmd.c_str(), "r");
    if (output == nullptr)
        throw std::runtime_error("popen() failed -- could not launch zenity!");
    while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
        ;
    pclose(output);
    std::string paths(buffer);
    paths.erase(std::remove(paths.begin(), paths.end(), '\n'), paths.end());

    std::vector<std::string> result;
    while (!paths.empty())
    {
        size_t end = paths.find("//");
        if (end == std::string::npos)
        {
            result.emplace_back(paths);
            paths = "";
        }
        else
        {
            result.emplace_back(paths.substr(0, end));
            paths = paths.substr(end + 1);
        }
    }

    return result;
    #endif
}
#endif
