// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#ifndef MATERIALXFORMAYA_IMAGEFILE_H
#define MATERIALXFORMAYA_IMAGEFILE_H

#include <NodeTranslators/TextureNode.h>

namespace MaterialXForMaya
{

/// @class ImageFile
/// Image file translator
class ImageFile : public TextureNode
{
    DECLARE_NODE_TRANSLATOR(ImageFile)
public:
    /// Custom node export
    virtual mx::NodePtr exportNode(const MObject& mayaNode, const string& outputType, mx::NodeGraphPtr parent, TranslatorContext& context);
};

} // namespace MaterialXForMaya

#endif // MATERIALXFORMAYA_IMAGEFILE_H
