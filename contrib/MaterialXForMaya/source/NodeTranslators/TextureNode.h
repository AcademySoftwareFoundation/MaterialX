// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#ifndef MATERIALXFORMAYA_TEXTURENODE_H
#define MATERIALXFORMAYA_TEXTURENODE_H

#include <NodeTranslator.h>

namespace MaterialXForMaya
{

/// @class TextureNode
/// Base translators class for texture nodes
class TextureNode : public NodeTranslator
{
public:
    /// Export color balance node
    mx::NodePtr exportColorBalance(const MObject& mayaNode, mx::NodePtr node);
};

} // namespace MaterialXForMaya


#endif // MATERIALXFORMAYA_TEXTURENODE_H
