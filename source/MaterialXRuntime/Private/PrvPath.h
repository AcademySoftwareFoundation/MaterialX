//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PRVPATH_H
#define MATERIALX_PRVPATH_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>

#include <MaterialXRuntime/Private/PrvElement.h>

namespace MaterialX
{

class PrvPath
{
public:
    PrvPath();
    PrvPath(PrvObjectHandle obj);

    bool isValid() const
    {
        return getObject() != nullptr;
    }

    PrvObjectHandle getObject() const;

    void setObject(PrvObjectHandle obj);

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

    bool operator==(const PrvPath& other) const
    {
        return _root == other._root && _path == other._path;
    }

private:
    PrvObjectHandle _root;
    vector<RtToken> _path;
};

}

#endif
