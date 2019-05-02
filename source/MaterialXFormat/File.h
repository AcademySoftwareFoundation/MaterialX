//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_FILE_H
#define MATERIALX_FILE_H

/// @file
/// Cross-platform support for file and search paths

#include <MaterialXCore/Util.h>

namespace MaterialX
{

class FilePath;
using FilePathVec = vector<FilePath>;

extern const string PATH_LIST_SEPARATOR;
extern const string MATERIALX_SEARCH_PATH_ENV_VAR;

/// @class FilePath
/// A generic file path, supporting both syntactic and file system operations.
class FilePath
{
  public:
    enum Type
    {
        TypeRelative = 0,
        TypeAbsolute = 1,
        TypeNetwork = 2
    };

    enum Format
    {
        FormatWindows = 0,
        FormatPosix = 1,
    #if defined(_WIN32)
        FormatNative = FormatWindows
    #else
        FormatNative = FormatPosix
    #endif
    };

  public:
    FilePath() :
        _type(TypeRelative),
        _format(FormatNative)
    {
    }
    ~FilePath() { }
    
    bool operator==(const FilePath& rhs) const
    {
        return _vec == rhs._vec &&
               _type == rhs._type &&
               _format == rhs._format;
    }
    bool operator!=(const FilePath& rhs) const
    {
        return !(*this == rhs);
    }

    /// @name Syntactic Operations
    /// @{

    /// Construct a path from a standard string.
    FilePath(const string& str)
    {
        assign(str);
    }

    /// Convert a path to a standard string.
    operator string() const
    {
        return asString();
    }

    /// Assign a path from a standard string with the given format.
    void assign(const string& str, Format format = FormatNative);

    /// Return this path as a standard string with the given format.
    string asString(Format format = FormatNative) const;

    /// Return true if the given path is empty.
    bool isEmpty() const
    {
        return _vec.empty();
    }

    /// Return true if the given path is absolute.
    bool isAbsolute() const
    {
        return _type != TypeRelative;
    }

    /// Return the base name of the given path, with leading directory
    /// information removed.
    string getBaseName() const
    {
        if (isEmpty())
        {
            return EMPTY_STRING;
        }
        return _vec[_vec.size() - 1];
    }

    /// Return the file extension of the given path.
    string getExtension() const
    {
        string baseName = getBaseName();
        size_t i = baseName.rfind('.');
        return i != string::npos ? baseName.substr(i + 1) : EMPTY_STRING;
    }

    /// Concatenate two paths with a directory separator, returning the
    /// combined path.
    FilePath operator/(const FilePath& rhs) const;

    /// Set the path to the parent directory if one exists. 
    void pop();

    /// @}
    /// @name File System Operations
    /// @{

    /// Return true if the given path exists on the file system.
    bool exists() const;

    /// Return true if the given path is a directory on the file system.
    bool isDirectory() const;

    /// Return a vector of all files in the given directory with the given extension.
    FilePathVec getFilesInDirectory(const string& extension) const;

    /// Return a vector of all directories at or beneath the given path.
    FilePathVec getSubDirectories() const;

    /// Create a directory on the file system at the given path.
    void createDirectory();

    /// @}

    /// Return the current working directory of the file system.
    static FilePath getCurrentPath();

  private:
    StringVec _vec;
    Type _type;
    Format _format;
};

/// @class FileSearchPath
/// A sequence of file paths, which may be queried to find the first instance
/// of a given filename on the file system.
class FileSearchPath
{
  public:
    FileSearchPath()
    {
        append(FilePath::getCurrentPath());
    }
    ~FileSearchPath() { }

    /// Construct a search path from a string.
    /// @param searchPath A string containing a sequence of file paths joined
    ///    by separator characters.
    /// @param sep The set of separator characters used in the search path.
    ///    Defaults to the semicolon character.
    FileSearchPath(const string& searchPath, const string& sep = PATH_LIST_SEPARATOR) :
        FileSearchPath()
    {
        parse(searchPath, sep);
    }

    /// Parse a given path and append to the sequence
    void parse(const string& searchPath, const string& sep = PATH_LIST_SEPARATOR)
    {
        if (searchPath.empty())
        {
            return;
        }

        for (const string& path : splitString(searchPath, sep))
        {
            if (!path.empty())
            {
                append(FilePath(path));
            }
        }
    }

    /// Append the given path to the sequence.
    void append(const FilePath& path)
    {
        _paths.push_back(path);
    }

    /// Append the given search path to the sequence.
    void append(const FileSearchPath& searchPath)
    {
        for (const FilePath& path : searchPath.paths())
        {
            _paths.push_back(path);
        }
    }

    /// Get list of paths in the search path.
    const FilePathVec& paths() const
    {
        return _paths;
    }

    /// Prepend the given path to the sequence.
    void prepend(const FilePath& path)
    {
        _paths.insert(_paths.begin(), path);
    }
    
    /// Return the number of paths in the sequence.
    size_t size() const
    {
        return _paths.size();
    }

    /// Return the path at the given index.
    FilePath& operator[](size_t index)
    {
        return _paths[index];
    }
    
    /// Return the const path at the given index.
    const FilePath& operator[](size_t index) const
    {
        return _paths[index];
    }

    /// Given an input filename, iterate through each path in this sequence,
    /// returning the first combined path found on the file system.
    /// On success, the combined path is returned; otherwise the original
    /// filename is returned unmodified.
    FilePath find(const FilePath& filename) const
    {
        if (_paths.empty()) 
        {
            return filename;
        }
        if (!filename.isAbsolute())
        {
            for (const FilePath& path : _paths)
            {
                FilePath combined = path / filename;
                if (combined.exists())
                {
                    return combined;
                }
            }
        }
        return filename;
    }

  private:
    FilePathVec _paths;
};

/// Return a FileSearchPath object from search path environment variable.
FileSearchPath getEnvironmentPath(const string& sep = PATH_LIST_SEPARATOR);

} // namespace MaterialX

#endif
