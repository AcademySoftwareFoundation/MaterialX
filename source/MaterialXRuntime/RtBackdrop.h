//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTBACKDROP_H
#define MATERIALX_RTBACKDROP_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtSchema.h>

namespace MaterialX
{

/// @class RtBackdrop
/// Schema for backdrop prims.
class RtBackdrop : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtBackdrop)

public:
    /// Return the contains relationship.
    RtRelationship contains() const;

    /// Returns the note attribute.
    RtAttribute note() const;

    /// Returns the weight attribute.
    RtAttribute width() const;

    /// Returns the height attribute.
    RtAttribute height() const;
};

}

#endif
