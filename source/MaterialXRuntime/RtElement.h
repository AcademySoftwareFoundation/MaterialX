//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTELEMENT_H
#define MATERIALX_RTELEMENT_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtAttribute.h>
#include <MaterialXRuntime/RtTraversal.h>

namespace MaterialX
{

/// @class RtElement
/// API for accessing an element. This API can be
/// attached to objects of all element types.
class RtElement : public RtApiBase
{
public:
    /// Constructor attaching an object to the API.
    RtElement(const RtObject& obj);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Get element name.
    const RtToken& getName() const;

    /// Set element name.
    /// The name may be changed to form a unique name within
    /// the scope of this element and its siblings.
    void setName(const RtToken& name);

    /// Add an attribute.
    RtAttribute* addAttribute(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Return an attribute by name, or a nullptr
    /// if no such attribute exists.
    const RtAttribute* getAttribute(const RtToken& name) const;

    /// Return an attribute by name, or a nullptr
    /// if no such attribute exists.
    RtAttribute* getAttribute(const RtToken& name);

    /// Return an attribute by index, or a nullptr
    /// if no such attribute exists.
    const RtAttribute* getAttribute(size_t index) const;

    /// Return an attribute by name, or a nullptr
    /// if no such attribute exists.
    RtAttribute* getAttribute(size_t index);

    /// Return the attribute count.
    size_t numAttributes() const;

    /// Return an iterator traversing all children
    /// of this element.
    /// If a filter is set it will be called to restrict
    /// which objects to return.
    RtTreeIterator traverseTree(RtTraversalFilter filter = nullptr);
};

}

#endif
