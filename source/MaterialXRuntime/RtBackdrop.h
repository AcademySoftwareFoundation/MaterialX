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
    /// Constructor.
    RtBackdrop(const RtPrim& prim) : RtTypedSchema(prim) {}

    /// Return the contains relationship.
    RtRelationship getContains() const;

    /// Set a value for the note attribute.
    void setNote(const string& note);

    /// Returns the note attribute.
    const string& getNote() const;

    /// Set a value for the width attribute.
    void setWidth(float height);

    /// Returns the weight attribute.
    float getWidth() const;

    /// Set a value for the height attribute.
    void setHeight(float height);

    /// Returns the height attribute.
    float getHeight() const;
};

}

#endif
