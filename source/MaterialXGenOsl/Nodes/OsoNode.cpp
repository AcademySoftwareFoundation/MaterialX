//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenOsl/Nodes/OsoNode.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr OsoNode::create()
{
    return std::make_shared<OsoNode>();
}

ShaderNodeImplPtr OsoNode::create(const string& osoName, const string& osoPath)
{
    return std::make_shared<OsoNode>(osoName, osoPath);
}

void OsoNode::initialize(const InterfaceElement& element, GenContext& context)
{
    ShaderNodeImpl::initialize(element, context);

    if (!element.isA<Implementation>())
    {
        throw ExceptionShaderGenError("Element '" + element.getName() + "' is not an Implementation element");
    }
    const Implementation& impl = static_cast<const Implementation&>(element);

    // We guard setting up the node to allow statically created nodes to be registered.
    // We use this in the unit testing for OslNetwork generator for handling the modified
    // bitangent and tangent nodes.
    if (_osoName.empty())
    {
        // Implementation's function attr is the oso filename, file attr is its directory
        _osoName = impl.getFunction();
        _osoPath = impl.getFile();
    }

    // Set hash using the oso name.
    _hash = std::hash<string>{}(_osoName);
}

MATERIALX_NAMESPACE_END
