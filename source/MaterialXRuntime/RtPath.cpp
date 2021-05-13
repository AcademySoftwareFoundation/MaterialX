//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtPath.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtObject.h>

namespace MaterialX
{

RtPath::RtPath() :
    _ptr(new PvtPath())
{
}

RtPath::RtPath(const RtObject& obj) :
    _ptr(new PvtPath(PvtObject::cast<PvtObject>(obj)))
{
}

RtPath::RtPath(const string& path) :
    _ptr(new PvtPath(path))
{
}

RtPath::RtPath(const char* path) :
    _ptr(new PvtPath(string(path)))
{
}

RtPath::RtPath(const RtPath& other) :
    _ptr(new PvtPath(*static_cast<PvtPath*>(other._ptr)))
{
}

RtPath& RtPath::operator=(const RtPath& other)
{
    const PvtPath* otherPath = static_cast<const PvtPath*>(other._ptr);
    static_cast<PvtPath*>(_ptr)->operator=(*otherPath);
    return *this;
}

RtPath::~RtPath()
{
    delete static_cast<PvtPath*>(_ptr);
}

void RtPath::setObject(const RtObject& obj)
{
    static_cast<PvtPath*>(_ptr)->setObject(PvtObject::cast<PvtObject>(obj));
}

const RtString& RtPath::getName() const
{
    return static_cast<PvtPath*>(_ptr)->getName();
}

string RtPath::asString() const
{
    return static_cast<PvtPath*>(_ptr)->asString();
}

void RtPath::push(const RtString& childName)
{
    return static_cast<PvtPath*>(_ptr)->push(childName);
}

void RtPath::pop()
{
    return static_cast<PvtPath*>(_ptr)->pop();
}

bool RtPath::operator==(const RtPath& other) const
{
    const PvtPath* otherPath = static_cast<const PvtPath*>(other._ptr);
    return static_cast<PvtPath*>(_ptr)->operator==(*otherPath);
}

bool RtPath::isRoot() const
{
    return static_cast<PvtPath*>(_ptr)->isRoot();
}

}
