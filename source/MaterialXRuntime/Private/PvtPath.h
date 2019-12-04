//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTPATH_H
#define MATERIALX_PVTPATH_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>

#include <MaterialXRuntime/Private/PvtElement.h>

namespace MaterialX
{

class PvtPath
{
public:
    // Empty constructor.
    PvtPath() :
        _root(nullptr)
    {
    }

    // Construct from a data handle.
    PvtPath(PvtDataHandle obj) :
        _root(nullptr)
    {
        setObject(obj);
    }

    // Copy constructor.
    PvtPath(const PvtPath& other) :
        _root(other._root),
        _path(other._path)
    {
    }

    // Assignment operator.
    PvtPath& operator=(const PvtPath& other)
    {
        _root = other._root;
        _path = other._path;
        return *this;
    }

    bool isValid() const
    {
        return getObject() != nullptr;
    }

    bool isRoot() const
    {
        return _path.empty();
    }

    PvtDataHandle getObject() const;

    void setObject(PvtDataHandle obj);

    string getPathString()
    {
        string str;
        for (const RtToken& elem : _path)
        {
            str += SEPARATOR + elem.str();
        }
        return str.empty() ? SEPARATOR : str;
    }

    void push(const RtToken& childName)
    {
        _path.push_back(childName);
    }

    void pop()
    {
        if (!_path.empty())
        {
            _path.pop_back();
        }
    }

    bool operator==(const PvtPath& other) const
    {
        return _root == other._root && _path == other._path;
    }

    static const string SEPARATOR;

private:
    PvtDataHandle _root;
    vector<RtToken> _path;
};

}

#endif
