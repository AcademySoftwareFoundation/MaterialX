#ifndef MATERIALXFORMAYA_TEXTURENODE_H
#define MATERIALXFORMAYA_TEXTURENODE_H

#include <NodeTranslator.h>

// Base translators class for texture nodes
class TextureNode : public NodeTranslator
{
public:
    mx::NodePtr exportColorBalance(const MObject& mayaNode, mx::NodePtr node);
};


#endif // MATERIALXFORMAYA_TEXTURENODE_H
