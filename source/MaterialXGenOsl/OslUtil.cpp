//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenOsl/OslUtil.h>

#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/GenContext.h>

#include <MaterialXCore/Document.h>

MATERIALX_NAMESPACE_BEGIN

void addSetCiTerminalNode(ShaderGraph& graph, ConstDocumentPtr document, TypeSystemPtr typeSystem, GenContext& context)
{
    string setCiNodeDefName = "ND_osl_set_ci";
    NodeDefPtr setCiNodeDef = document->getNodeDef(setCiNodeDefName);

    std::unordered_map<TypeDesc, ValuePtr, TypeDesc::Hasher> outputModeMap;
    int index = 0;
    for (auto input : setCiNodeDef->getInputs())
    {
        string inputName = input->getName();
        if (stringStartsWith(inputName, "input_"))
        {
            TypeDesc inputType = typeSystem->getType(input->getType());
            outputModeMap[inputType] = std::make_shared<TypedValue<int>>(index++);
        }
    }

    for (auto output : graph.getOutputSockets())
    {
        auto outputType = output->getType();
        string typeName = outputType.getName();
        auto setCiNode = graph.inlineNodeBeforeOutput(output, "oslSetCi", setCiNodeDefName, "input_" + typeName, "out_ci", context);
        auto typeInput = setCiNode->getInput("output_mode");

        auto outputModeValue = outputModeMap[outputType];

        typeInput->setValue(outputModeValue);
    }
}

MATERIALX_NAMESPACE_END
