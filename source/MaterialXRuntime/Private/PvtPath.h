//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTPATH_H
#define MATERIALX_PVTPATH_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtIdentifier.h>
#include <MaterialXRuntime/RtPath.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

class PvtObject;

/// Class representing a unique path to an object in the stage.
///
/// TODO: Optimize this class. Use interning, where each element
/// in a path is a unique internal node in a tree and the path
/// class wraps an pointer to these nodes.
///
class PvtPath
{
public:
    // Empty constructor.
    PvtPath()
    {
    }

    // Construct from an object.
    explicit PvtPath(const PvtObject* obj)
    {
        setObject(obj);
    }

    // Construct from a single path element.
    explicit PvtPath(const RtIdentifier& elem) :
        _elements(elem != ROOT_NAME ? RtIdentifierVec({ ROOT_NAME, elem }) : RtIdentifierVec({ ROOT_NAME }))
    {
    }

    // Construct from a string path.
    explicit PvtPath(const string& path)
    {
        const StringVec elementNames = splitString(path, SEPARATOR);
        _elements.resize(elementNames.size() + 1);
        _elements[0] = PvtPath::ROOT_NAME;
        for (size_t i = 0; i < elementNames.size(); ++i)
        {
            _elements[i+1] = RtIdentifier(elementNames[i]);
        }
    }

    // Copy constructor.
    PvtPath(const PvtPath& other) :
        _elements(other._elements)
    {
    }

    // Assignment operator.
    PvtPath& operator=(const PvtPath& other)
    {
        _elements = other._elements;
        return *this;
    }

    void setObject(const PvtObject* obj);

    const RtIdentifier& getName() const
    {
        return _elements.size() ? _elements.back() : EMPTY_IDENTIFIER;
    }

    string asString() const
    {
        if (_elements.size() == 1)
        {
            return SEPARATOR;
        }
        string str;
        for (size_t i=1; i<_elements.size(); ++i)
        {
            str += SEPARATOR + _elements[i].str();
        }
        return str;
    }

    void push(const RtIdentifier& childName)
    {
        _elements.push_back(childName);
    }

    void pop()
    {
        if (!_elements.empty())
        {
            _elements.pop_back();
        }
    }

    size_t size() const
    {
        return _elements.size();
    }

    bool empty() const
    {
        return _elements.empty();
    }

    const RtIdentifier& operator[](size_t index) const
    {
        return _elements[index];
    }

    bool operator==(const PvtPath& other) const
    {
        return _elements == other._elements;
    }

    bool isRoot() const
    {
        return getName() == ROOT_NAME;
    }

    static PvtPath& get(RtPath& path)
    {
        return *static_cast<PvtPath*>(path._ptr);
    }

    static const PvtPath& get(const RtPath& path)
    {
        return *static_cast<const PvtPath*>(path._ptr);
    }

    static const string SEPARATOR;
    static const RtIdentifier ROOT_NAME;

private:
    vector<RtIdentifier> _elements;
};

}

#endif
