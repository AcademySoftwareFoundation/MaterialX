//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/Util.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

ShaderPtr createShader(const string& shaderName, GenContext& context, ElementPtr elem)
{
    if (!elem)
    {
        return nullptr;
    }

    context.getOptions().hwTransparency = isTransparentSurface(elem, context.getShaderGenerator());
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
    constant->setParameterValue("value", color);
    OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);

    // Generate the shader
    return createShader(shaderName, context, output);
}

unsigned int getUIProperties(const ValueElementPtr nodeDefElement, UIProperties& uiProperties)
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

    if (nodeDefElement->isA<Parameter>())
    {
        string enumString = nodeDefElement->getAttribute(ValueElement::ENUM_ATTRIBUTE);
        if (!enumString.empty())
        {
            uiProperties.enumeration = splitString(enumString, ",");
            if (uiProperties.enumeration.size())
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
                for (size_t i = 0; i < stringValues.size(); i++)
                {
                    if (count == elementCount)
                    {
                        valueString += stringValues[i];
                        uiProperties.enumerationValues.push_back(Value::createValueFromStrings(valueString, elemType));
                        valueString.clear();
                        count = 0;
                    }
                    else
                    {
                        valueString += stringValues[i] + ",";
                        count++;
                    }
                }
            }
            else
            {
                uiProperties.enumerationValues.push_back(Value::createValue(enumerationValues));
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

void createUIPropertyGroups(const VariableBlock& block, DocumentPtr contentDocument, TypedElementPtr materialElement,
                          const string& pathSeparator, UIPropertyGroup& groups, UIPropertyGroup& unnamedGroups)
{
    for (auto uniform : block.getVariableOrder())
    {
        if (uniform->getPath().size() && uniform->getValue())
        {
            ElementPtr uniformElement = contentDocument->getDescendant(uniform->getPath());
            if (uniformElement && uniformElement->isA<ValueElement>())
            {
                UIPropertyItem item;
                item.variable = uniform;
                getUIProperties(uniform->getPath(), contentDocument, EMPTY_STRING, item.ui);

                std::string parentLabel;
                ElementPtr parent = uniformElement->getParent();
                if (parent && parent != contentDocument && parent != materialElement)
                {
                    parentLabel = parent->getNamePath();
                }
                if (parentLabel == materialElement->getAttribute(PortElement::NODE_NAME_ATTRIBUTE))
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
                    groups.insert(std::pair<std::string, UIPropertyItem>
                        (item.ui.uiFolder, item));
                }
                else
                {
                    unnamedGroups.insert(std::pair<std::string, UIPropertyItem>
                        (EMPTY_STRING, item));
                }
            }
        }
    }
}

} // namespace MaterialX

