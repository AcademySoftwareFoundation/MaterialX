//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/Util.h>

#include <MaterialXCore/Util.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

ShaderPtr createShader(const string& shaderName, GenContext& context, ElementPtr elem)
{
    return context.getShaderGenerator().generate(shaderName, elem, context);
}

ShaderPtr createConstantShader(GenContext& context,
                               DocumentPtr stdLib,
                               const string& shaderName,
                               const Color3& color)
{
    // Construct the constant color nodegraph
    DocumentPtr doc = createDocument();
    doc->importLibrary(stdLib);
    NodeGraphPtr nodeGraph = doc->addNodeGraph();
    NodePtr constant = nodeGraph->addNode("constant");
    constant->setInputValue("value", color);
    OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);

    // Generate the shader
    return createShader(shaderName, context, output);
}

ShaderPtr createDepthShader(GenContext& context,
                            DocumentPtr stdLib,
                            const string& shaderName)
{
    // Construct a dummy nodegraph.
    DocumentPtr doc = createDocument();
    doc->importLibrary(stdLib);
    NodeGraphPtr nodeGraph = doc->addNodeGraph();
    NodePtr constant = nodeGraph->addNode("constant");
    OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);

    // Generate the shader
    GenContext depthContext = context;
    depthContext.getOptions().hwWriteDepthMoments = true;
    ShaderPtr shader = createShader(shaderName, depthContext, output);

    return shader;
}

ShaderPtr createAlbedoTableShader(GenContext& context,
                                  DocumentPtr stdLib,
                                  const string& shaderName)
{
    // Construct a dummy nodegraph.
    DocumentPtr doc = createDocument();
    doc->importLibrary(stdLib);
    NodeGraphPtr nodeGraph = doc->addNodeGraph();
    NodePtr constant = nodeGraph->addNode("constant");
    OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);

    // Generate the shader
    GenContext tableContext = context;
    tableContext.getOptions().hwWriteAlbedoTable = true;
    ShaderPtr shader = createShader(shaderName, tableContext, output);

    return shader;
}

ShaderPtr createBlurShader(GenContext& context,
                           DocumentPtr stdLib,
                           const string& shaderName,
                           const string& filterType,
                           float filterSize)
{
    // Construct the blur nodegraph
    DocumentPtr doc = createDocument();
    doc->importLibrary(stdLib);
    NodeGraphPtr nodeGraph = doc->addNodeGraph();
    NodePtr imageNode = nodeGraph->addNode("image", "image");
    NodePtr blurNode = nodeGraph->addNode("blur", "blur");
    blurNode->setConnectedNode("in", imageNode);
    blurNode->setInputValue("size", filterSize);
    blurNode->setInputValue("filtertype", filterType);
    OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(blurNode);

    // Generate the shader
    GenContext blurContext = context;
    blurContext.getOptions().fileTextureVerticalFlip = false;
    return createShader(shaderName, blurContext, output);
}

unsigned int getUIProperties(ConstValueElementPtr nodeDefElement, UIProperties& uiProperties)
{
    if (!nodeDefElement)
    {
        return 0;
    }

    unsigned int propertyCount = 0;
    uiProperties.uiName = nodeDefElement->getAttribute(ValueElement::UI_NAME_ATTRIBUTE);
    if (!uiProperties.uiName.empty())
        propertyCount++;

    uiProperties.uiFolder = nodeDefElement->getAttribute(ValueElement::UI_FOLDER_ATTRIBUTE);
    if (!uiProperties.uiFolder.empty())
        propertyCount++;

    if (nodeDefElement->getIsUniform())
    {
        string enumString = nodeDefElement->getAttribute(ValueElement::ENUM_ATTRIBUTE);
        if (!enumString.empty())
        {
            uiProperties.enumeration = splitString(enumString, ",");
            if (!uiProperties.enumeration.empty())
                propertyCount++;
        }

        const string& enumerationValues = nodeDefElement->getAttribute(ValueElement::ENUM_VALUES_ATTRIBUTE);
        if (!enumerationValues.empty())
        {
            const string& elemType = nodeDefElement->getType();
            const TypeDesc* typeDesc = TypeDesc::get(elemType);
            if (typeDesc->isScalar() || typeDesc->isFloat2() || typeDesc->isFloat3() ||
                typeDesc->isFloat4())
            {
                StringVec stringValues = splitString(enumerationValues, ",");
                string valueString;
                size_t elementCount = typeDesc->getSize();
                elementCount--;
                size_t count = 0;
                for (const string& val : stringValues)
                {
                    if (count == elementCount)
                    {
                        valueString += val;
                        uiProperties.enumerationValues.push_back(Value::createValueFromStrings(valueString, elemType));
                        valueString.clear();
                        count = 0;
                    }
                    else
                    {
                        valueString += val + ",";
                        count++;
                    }
                }
            }
            else
            {
                uiProperties.enumerationValues.push_back(Value::createValue(enumerationValues));
            }
            if (uiProperties.enumeration.size() != uiProperties.enumerationValues.size())
            {
                throw std::runtime_error("Every enum must have a value!");
            }
            propertyCount++;
        }
    }

    const string& uiMinString = nodeDefElement->getAttribute(ValueElement::UI_MIN_ATTRIBUTE);
    if (!uiMinString.empty())
    {
        ValuePtr value = Value::createValueFromStrings(uiMinString, nodeDefElement->getType());
        if (value)
        {
            uiProperties.uiMin = value;
            propertyCount++;
        }
    }

    const string& uiMaxString = nodeDefElement->getAttribute(ValueElement::UI_MAX_ATTRIBUTE);
    if (!uiMaxString.empty())
    {
        ValuePtr value = Value::createValueFromStrings(uiMaxString, nodeDefElement->getType());
        if (value)
        {
            uiProperties.uiMax = value;
            propertyCount++;
        }
    }

    const string& uiSoftMinString = nodeDefElement->getAttribute(ValueElement::UI_SOFT_MIN_ATTRIBUTE);
    if (!uiSoftMinString.empty())
    {
        ValuePtr value = Value::createValueFromStrings(uiSoftMinString, nodeDefElement->getType());
        if (value)
        {
            uiProperties.uiSoftMin = value;
            propertyCount++;
        }
    }

    const string& uiSoftMaxString = nodeDefElement->getAttribute(ValueElement::UI_SOFT_MAX_ATTRIBUTE);
    if (!uiSoftMaxString.empty())
    {
        ValuePtr value = Value::createValueFromStrings(uiSoftMaxString, nodeDefElement->getType());
        if (value)
        {
            uiProperties.uiSoftMax = value;
            propertyCount++;
        }
    }

    const string& uiStepString = nodeDefElement->getAttribute(ValueElement::UI_STEP_ATTRIBUTE);
    if (!uiStepString.empty())
    {
        ValuePtr value = Value::createValueFromStrings(uiStepString, nodeDefElement->getType());
        if (value)
        {
            uiProperties.uiStep = value;
            propertyCount++;
        }
    }

    const string& uiAdvancedString = nodeDefElement->getAttribute(ValueElement::UI_ADVANCED_ATTRIBUTE);
    uiProperties.uiAdvanced = (uiAdvancedString == "true");
    if (!uiAdvancedString.empty())
    {
        propertyCount++;
    }

    return propertyCount;
}

unsigned int getUIProperties(const string& path, DocumentPtr doc, const string& target, UIProperties& uiProperties)
{
    ValueElementPtr valueElement = findNodeDefChild(path, doc, target);
    if (valueElement)
    {
        return getUIProperties(valueElement, uiProperties);
    }
    return 0;
}

void createUIPropertyGroups(ElementPtr uniformElement, DocumentPtr contentDocument, TypedElementPtr materialElement,
                            const string& pathSeparator, UIPropertyGroup& groups,
                            UIPropertyGroup& unnamedGroups, ShaderPort* uniform)
{
    if (uniformElement && uniformElement->isA<ValueElement>())
    {
        UIPropertyItem item;
        item.variable = uniform;
        item.value = uniformElement->asA<ValueElement>()->getValue();
        getUIProperties(uniformElement->getNamePath(), contentDocument, EMPTY_STRING, item.ui);

        string parentLabel;
        ElementPtr parent = uniformElement->getParent();
        if (parent && parent != contentDocument && parent != materialElement)
        {
            parentLabel = parent->getNamePath();
        }
        if (!materialElement || parentLabel == materialElement->getAttribute(PortElement::NODE_NAME_ATTRIBUTE))
        {
            parentLabel.clear();
        }
        if (!parentLabel.empty())
        {
            parentLabel += pathSeparator;
        }

        if (!item.ui.uiName.empty())
        {
            item.label = parentLabel + item.ui.uiName;
        }
        if (item.label.empty())
        {
            item.label = parentLabel + uniformElement->getName();
        }

        if (!item.ui.uiFolder.empty())
        {
            groups.emplace(item.ui.uiFolder, item);
        }
        else
        {
            unnamedGroups.emplace(EMPTY_STRING, item);
        }
    }
}

void createUIPropertyGroups(const VariableBlock& block, DocumentPtr contentDocument, TypedElementPtr materialElement,
                            const string& pathSeparator, UIPropertyGroup& groups, UIPropertyGroup& unnamedGroups, bool addFromDefinition)
{
    const vector<ShaderPort*>& blockVariables = block.getVariableOrder();
    for (const auto& blockVariable : blockVariables)
    {
        const string& path = blockVariable->getPath();

        if (!blockVariable->getPath().empty())
        {
            // Optionally add the input if it does not exist
            ElementPtr uniformElement = contentDocument->getDescendant(path);
            if (!uniformElement && addFromDefinition)
            {
                string nodePath = parentNamePath(path);
                ElementPtr uniformParent = contentDocument->getDescendant(nodePath);
                if (uniformParent)
                {
                    NodePtr uniformNode = uniformParent->asA<Node>();
                    if (uniformNode)
                    {
                        StringVec pathVec = splitNamePath(path);
                        uniformNode->addInputFromNodeDef(pathVec[pathVec.size() - 1]);
                    }
                }
            }

            uniformElement = contentDocument->getDescendant(path);
            if (uniformElement && blockVariable->getValue())
            {
                createUIPropertyGroups(uniformElement, contentDocument, materialElement, pathSeparator, groups, unnamedGroups, blockVariable);
            }
        }
    }
}

} // namespace MaterialX
