// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#ifndef MATERIALXFORMAYA_PLACE2DTEXTURE_H
#define MATERIALXFORMAYA_PLACE2DTEXTURE_H

#include <NodeTranslator.h>

namespace MaterialXForMaya
{

/// @class ImageFile
/// Place2D texture node translator
class Place2dTexture : public NodeTranslator
{
    DECLARE_NODE_TRANSLATOR(Place2dTexture)
public:
    /// Custom node export
    mx::NodePtr exportNode(const MObject& mayaNode, const string& outputType, mx::NodeGraphPtr parent, TranslatorContext& context) override;
};

} // namespace MaterialXForMaya

#endif // MATERIALXFORMAYA_PLACE2DTEXTURE_H
