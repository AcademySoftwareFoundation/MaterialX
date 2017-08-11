#ifndef MATERIALXFORMAYA_IMAGEFILE_H
#define MATERIALXFORMAYA_IMAGEFILE_H

#include <NodeTranslators/TextureNode.h>

class ImageFile : public TextureNode
{
    DECLARE_NODE_TRANSLATOR(ImageFile)
public:
    virtual mx::NodePtr exportNode(const MObject& mayaNode, const string& outputType, mx::NodeGraphPtr parent, TranslatorContext& context);
};


#endif // MATERIALXFORMAYA_IMAGEFILE_H
