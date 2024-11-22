/******************************************************************************
 * Copyright (c) 2021-2024, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

 // examples/mdl_sdk/shared/utils/io.h
 //
 // Code shared by all examples

#ifndef EXAMPLE_SHARED_UTILS_IO_H
#define EXAMPLE_SHARED_UTILS_IO_H

#include <fstream>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <iostream>
#include <functional>
#include <vector>
#include <stack>

#include <mi/base/config.h>
#include "strings.h"

#ifdef MI_PLATFORM_WINDOWS
    #include <windows.h>
    #include <commdlg.h>
    #include <direct.h>
    #include <Shlobj.h>
    #include <Knownfolders.h>
#else
    #include <dlfcn.h>
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
#endif

#ifdef MI_PLATFORM_MACOSX
    #include <mach-o/dyld.h>   // _NSGetExecutablePath
#endif

namespace mi { namespace examples { namespace io
{
    /// Returns the native path separator.
    inline char sep()
    {
#ifdef MI_PLATFORM_WINDOWS
        return '\\';
#else
        return '/';
#endif
    }

    /// Normalize a path.
    /// On windows, this turns backslashes into forward slashes. No changes on Linux and Mac.
    inline std::string normalize(std::string path, bool remove_dir_ups = false)
    {
        if (path.empty())
            return path;

        #ifdef MI_PLATFORM_WINDOWS
            std::replace(path.begin(), path.end(), '\\', '/');
        #endif

        if (remove_dir_ups)
        {
            bool isAbsolute = path[0] == '/';
            bool isAbsoluteUNC = isAbsolute && path[1] == '/';
            std::vector<std::string> chunks = mi::examples::strings::split(path, '/');
            std::stack<std::string> pathStack;
            for (const std::string& c : chunks)
            {
                if (c.empty() || c == ".")
                    continue;
                if (c == "..")
                {
                    if (pathStack.empty())
                        pathStack.push(c);
                    else
                        pathStack.pop();
                    continue;
                }
                pathStack.push(c);
            }
            if (pathStack.empty())
                return "";

            path = pathStack.top();
            pathStack.pop();

            while (!pathStack.empty())
            {
                path = pathStack.top() + "/" + path;
                pathStack.pop();
            }

            if (isAbsolute)
                path = "/" + path;
            if (isAbsoluteUNC)
                path = "/" + path;
        }
        return path;
    }

    // --------------------------------------------------------------------------------------------


    /// true if the path exists, file or directory.
    inline bool exists(const std::string& path)
    {
        struct stat info;
        return stat(path.c_str(), &info) == 0;
    }

    // --------------------------------------------------------------------------------------------

    /// true if the path exists and it points to a file.
    inline bool file_exists(const std::string& filepath)
    {
        struct stat info;
        if (stat(filepath.c_str(), &info) != 0)
            return false;
        return !(info.st_mode & S_IFDIR);
    }

    // --------------------------------------------------------------------------------------------

    /// true if the path exists and it points to a directory.
    inline bool directory_exists(const std::string& dirpath)
    {
        struct stat info;
        if (stat(dirpath.c_str(), &info) != 0)
            return false;
        return (info.st_mode & S_IFDIR) ? true : false;
    }

    // --------------------------------------------------------------------------------------------
    std::string dirname(const std::string& path);

    /// creates a directory (not recursively, by default)
    /// return true if the directory was created successfully or if it already existed.
    inline bool mkdir(const std::string& dirpath, bool recursively = false)
    {
        if (recursively)
        {
            std::function<bool(const std::string&)> mkdir_recursively =
                [&mkdir_recursively](const std::string& dirpath) -> bool
            {
                if (dirpath.empty())
                    return false;

                if (directory_exists(dirpath))
                    return true;

                // check if the parent exists, or can be created
                const std::string parent = dirname(dirpath);
                bool parent_exits = directory_exists(parent);
                if (!parent_exits)
                    parent_exits = mkdir_recursively(parent);

                // create the current folder
                return parent_exits && mkdir(dirpath, false);
            };

            return mkdir_recursively(dirpath);
        }

#ifdef MI_PLATFORM_WINDOWS
        _set_errno(0);
        if (_mkdir(dirpath.c_str()) == 0)
            return true;

        errno_t err;
        _get_errno(&err);
        return err == EEXIST;
#else
        errno = 0;
        if (::mkdir(dirpath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0)
            return true;
        return errno == EEXIST;
#endif
    }

    // --------------------------------------------------------------------------------------------

    /// Reads the content of the given file.
    inline std::string read_text_file(const std::string& filename)
    {
        std::ifstream file(filename.c_str());

        if (!file.is_open())
        {
            std::cerr << "Cannot open file: \"" << filename << "\".\n";
            return "";
        }

        std::stringstream string_stream;
        string_stream << file.rdbuf();

        return string_stream.str();
    }

    // --------------------------------------------------------------------------------------------

    /// Reads the content of the given binary file.
    inline std::vector<char> read_binary_file(const std::string& filename)
    {
        std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);

        std::vector<char> data;

        if (!file.is_open())
        {
            std::cerr << "Cannot open file: \"" << filename << "\".\n";
            return data;
        }

        file.seekg(0, std::ios::end);
        data.resize(file.tellg());

        file.seekg(0, std::ios::beg);
        file.read(data.data(), data.size());

        return data;
    }

    // --------------------------------------------------------------------------------------------
    /// checks if a path absolute or relative.
    inline bool is_absolute_path(const std::string& path)
    {
        std::string npath = normalize(path);

#ifdef MI_PLATFORM_WINDOWS
        if (npath.size() < 2) // no absolute path of length 1 or
            return false;
        if (npath[0] == '/' && npath[1] == '/') // UNC paths
            return true;
        if (isalpha(npath[0]) && npath[1] == ':') // drive letter
            return true;
        return false;
#else
        return npath[0] == '/';
#endif
    }

    // --------------------------------------------------------------------------------------------

    /// Get directory name a given file or sub-folder is located in or an empty
    /// string if there is no parent directory in \c path.
    inline std::string dirname(const std::string& path)
    {
        std::string npath = normalize(path);

        size_t pos = npath.rfind('/');
        return pos == std::string::npos ? "" : npath.substr(0, pos);
    }

    // --------------------------------------------------------------------------------------------

    /// Get filename (with extension if selected) of a given path without its parent folders or
    /// \c path (with or without extension) if there is no parent directory in \c path.
    inline std::string basename(const std::string& path, bool with_extension = true)
    {
        std::string npath = normalize(path);

        size_t pos = npath.rfind('/');
        if (pos != std::string::npos)
            npath = npath.substr(pos + 1);

        if (!with_extension)
        {
            pos = npath.rfind('.');
            npath = pos == std::string::npos ? npath : npath.substr(0, pos);
        }
        return npath;
    }

    // --------------------------------------------------------------------------------------------

    /// Get the extension of a given path or an empty
    /// string if there is no file extension in the last path segment (i.e. basename).
    inline std::string extension(const std::string& path)
    {
        std::string name = basename(path);

        size_t pos = name.rfind('.');
        return pos == std::string::npos ? "" : name.substr(pos + 1);
    }

    // --------------------------------------------------------------------------------------------

    // get the current working directory.
    inline std::string get_working_directory()
    {
        char current_path[FILENAME_MAX];
        #ifdef MI_PLATFORM_WINDOWS
            _getcwd(current_path, FILENAME_MAX);
        #else
            getcwd(current_path, FILENAME_MAX); // TODO
        #endif
        return normalize(current_path);
    }

    // --------------------------------------------------------------------------------------------

    // Returns the folder path of the current executable.
    inline std::string get_executable_folder()
    {
        #ifdef MI_PLATFORM_WINDOWS
            char path[MAX_PATH];
            if (!GetModuleFileNameA(nullptr, path, MAX_PATH))
                return "";

            const char sep = '\\';
        #else  // MI_PLATFORM_WINDOWS
            char path[4096];

            #ifdef MI_PLATFORM_MACOSX
                uint32_t buflen(sizeof(path));
                if (_NSGetExecutablePath(path, &buflen) != 0)
                    return "";
            #else  // MI_PLATFORM_MACOSX
                char proc_path[64];
                snprintf(proc_path, sizeof(proc_path), "/proc/%d/exe", getpid());

                ssize_t written = readlink(proc_path, path, sizeof(path));
                if (written < 0 || size_t(written) >= sizeof(path))
                    return "";
                path[written] = 0;  // add terminating null
            #endif // MI_PLATFORM_MACOSX

            const char sep = '/';
        #endif // MI_PLATFORM_WINDOWS

        char *last_sep = strrchr(path, sep);
        if (last_sep == nullptr) return "";

        return normalize(std::string(path, last_sep));
    }

    // --------------------------------------------------------------------------------------------

    // get the query substring of an url if present otherwise returns an empty string.
    // also, drops the hash segment in case it is present.
    inline std::string get_url_query(const std::string& url)
    {
        std::string query = url;
        size_t pos = query.find_first_of('?');
        if (pos == std::string::npos)
            return "";
        else
            query = query.substr(pos + 1);

        // drop the hash if present at at the end of the query
        pos = query.find_first_of('#');
        if (pos != std::string::npos)
            query = query.substr(0, pos);
        return query;
    }

    // --------------------------------------------------------------------------------------------

    // parses a query of an url and returns a map of key value pairs
    inline std::map<std::string, std::string> parse_url_query(const std::string& query)
    {
        std::map<std::string, std::string> result;
        std::vector<std::string> chunks = mi::examples::strings::split(query, '&');
        for (auto& c : chunks)
        {
            size_t eqPos = c.find_first_of('=');
            if (eqPos == std::string::npos)
            {
                result.insert({ c, "" });
            }
            else
            {
                result.insert({ c.substr(0, eqPos), c.substr(eqPos + 1) });
            }
        }
        return result;
    }

    // --------------------------------------------------------------------------------------------

    // removes the query substring of an url if present.
    inline std::string drop_url_query(const std::string& url)
    {
        size_t pos = url.find_first_of('?');
        if (pos == std::string::npos)
            return url;
        else
            return url.substr(0, pos);
    }

    // --------------------------------------------------------------------------------------------

    /// helper to open a file open dialog.
    struct open_file_name_dialog
    {
        explicit open_file_name_dialog(std::string title = "Open file..")
            : m_title(mi::examples::strings::str_to_wstr(title))
            , m_add_all_supported_types_entry(true)
#ifdef MI_PLATFORM_WINDOWS
            , m_parent_window(NULL)
#endif
        {
        }

        /// add an file filter, e.g.: " dialog.add_type("Encapsulated MDL", "mdle");
        /// separate multiple extensions by semicolon in the /c extension argument.
        void add_type(const std::string& name, const std::string& extension)
        {
            auto types = mi::examples::strings::split(extension, ';');
            std::wstring description = mi::examples::strings::str_to_wstr(name) + L" (";
            std::wstring file_types = L"";
            for (size_t i = 0; i < types.size(); ++i)
            {
                if (i > 0)
                {
                    description.append(L", ");
                    file_types.append(L";");
                }
                std::wstring ext = mi::examples::strings::str_to_wstr("*." + types[i]);
                description.append(ext);
                file_types.append(ext);
            }
            description += L")";
            m_type_map.push_back({ description, file_types });
        }

#ifdef MI_PLATFORM_WINDOWS
        /// if available, add a parent window for positioning and ownership.
        void set_parent_window(HWND window_handle)
        {
            m_parent_window = window_handle;
        }
#endif

        /// Add another options at the bottom of the list to be able to select
        /// all file types that haven been added. This is only done when there is more
        /// than one entry.
        void add_all_supported_types_entry(bool value)
        {
            m_add_all_supported_types_entry = value;
        }

        /// shows the dialog
        /// return the selected file path or an empty string in case failure/abort.
        std::string show()
        {
            std::wstring all_file_types = L"";
            if (m_add_all_supported_types_entry)
            {
                for (size_t i = 0; i < m_type_map.size(); ++i)
                {
                    if (i > 0)
                        all_file_types.append(L";");
                    all_file_types.append(m_type_map[i].second);
                }
            }

#ifdef MI_PLATFORM_WINDOWS
            wchar_t filename[MAX_PATH];

            OPENFILENAMEW ofn;
            memset(&filename, '\0', MAX_PATH);
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = m_parent_window;

            ofn.lpstrFile = filename;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = MAX_PATH;

            std::wstringstream filter_ss;
            for (const auto& pair : m_type_map)
                filter_ss << pair.first << '|' << pair.second << '|';

            if (m_add_all_supported_types_entry)
            {
                filter_ss << "All supported types" << '|' << all_file_types << '|';
                ofn.nFilterIndex = DWORD(m_type_map.size() + 1);
            }
            else
                ofn.nFilterIndex = 0;

            filter_ss << '|';
            std::wstring filter = filter_ss.str();
            std::replace(filter.begin(), filter.end(), '|', '\0');
            ofn.lpstrFilter = m_type_map.empty() ? nullptr : filter.c_str();
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;

            ofn.lpstrTitle = m_title.c_str();
            ofn.Flags = OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            // this will cause messages in the VS debugger log:
            // mincore\com\oleaut32\dispatch\ups.cpp(2125)\OLEAUT32.dll!00007FF8CDE93C5B: ...
            // ReturnHr(1) tid(549c) 8002801D Library not registered.
            // not able to resolve this. parameter seem okay. several similar posts on google.
            if (GetOpenFileNameW(&ofn) == TRUE)
                return normalize(mi::examples::strings::wstr_to_str(filename));
#else
            std::cerr << "mi::examples::io::open_file_name_dialog is not implemented for this platform.\n";
#endif
            return "";
        }

    private:
        std::wstring m_title;
        std::vector<std::pair<std::wstring, std::wstring>> m_type_map;
        bool m_add_all_supported_types_entry;

        #ifdef MI_PLATFORM_WINDOWS
            HWND m_parent_window;
        #endif
    };

}}}
#endif
