//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTTYPESCHEMA_H
#define MATERIALX_RTTYPESCHEMA_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtIdentifier.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

/// @class RtTypeInfo
/// Class holding type information for typed schemas.
class RtTypeInfo
{
public:
    /// Constructor setting the type hierarchy
    /// for this typename. The string should list
    /// the typenames in the hierarchy with a ':' 
    /// separator, e.g.: "node:nodegraph".
    RtTypeInfo(const char* typeNameHierachy);

    /// Destructor
    ~RtTypeInfo();

    /// Return the single typename for the class.
    const RtIdentifier& getShortTypeName() const;

    /// Return the complete typename for the class including base classes.
    const RtIdentifier& getLongTypeName() const;

    /// Return the number of base classes for this class.
    size_t numBaseClasses() const;

    /// Return the short typename for a specific base class.
    const RtIdentifier& getBaseClassType(size_t index) const;

    /// Return true if the given typename is part of the
    /// class hierarchy for this type.
    bool isCompatible(const RtIdentifier& typeName) const;

private:
    // Private data.
    void* _ptr;
};

}

#endif
