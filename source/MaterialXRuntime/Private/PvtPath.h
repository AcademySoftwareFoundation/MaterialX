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
    PvtPath();
    PvtPath(PvtDataHandle obj);

    bool isValid() const
    {
        return getObject() != nullptr;
    }

    PvtDataHandle getObject() const;

    void setObject(PvtDataHandle obj);

    string getPathString()
    {
        string str;
        for (const RtToken& elem : _path)
        {
            str += "/" + elem.str();
        }
        return str;
    }

    void push(const RtToken& childName)
    {
        _path.push_back(childName);
    }

    void pop()
    {
        _path.pop_back();
    }

    bool operator==(const PvtPath& other) const
    {
        return _root == other._root && _path == other._path;
    }

private:
    PvtDataHandle _root;
    vector<RtToken> _path;
};

}

#endif
