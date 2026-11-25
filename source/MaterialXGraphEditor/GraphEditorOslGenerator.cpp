//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGraphEditor/GraphEditorOslGenerator.h>

#include <MaterialXGenOsl/OslShaderGenerator.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXFormat/Util.h>

#include <exception>
#include <vector>

GraphEditorOslGenerator::GraphEditorOslGenerator(mx::DocumentPtr document,
                                                 const mx::FileSearchPath& searchPath) :
    _document(document),
    _searchPath(searchPath),
    _context(std::make_unique<mx::GenContext>(mx::OslShaderGenerator::create()))
{
    initContext();
}

GraphEditorOslGenerator::~GraphEditorOslGenerator() = default;

void GraphEditorOslGenerator::setDocument(mx::DocumentPtr document)
{
    _document = document;
    initContext();
}

void GraphEditorOslGenerator::initContext()
{
    if (!_document || !_context)
    {
        return;
    }

    mx::GenContext& context = *_context;

    // Initialize search paths, including the folder holding the MaterialX OSL
    // support headers (e.g. mx_funcs.h).
    context.registerSourceCodeSearchPath(_searchPath);
    mx::FilePath genOslIncludePath = _searchPath.find("libraries/stdlib/genosl/include");
    if (!genOslIncludePath.isEmpty())
    {
        context.registerSourceCodeSearchPath(genOslIncludePath);
    }

    // Initialize unit management.
    mx::UnitTypeDefPtr distanceTypeDef = _document->getUnitTypeDef("distance");
    mx::LinearUnitConverterPtr distanceConverter = mx::LinearUnitConverter::create(distanceTypeDef);
    mx::UnitConverterRegistryPtr unitRegistry = mx::UnitConverterRegistry::create();
    unitRegistry->addUnitConverter(distanceTypeDef, distanceConverter);
    mx::UnitTypeDefPtr angleTypeDef = _document->getUnitTypeDef("angle");
    mx::LinearUnitConverterPtr angleConverter = mx::LinearUnitConverter::create(angleTypeDef);
    unitRegistry->addUnitConverter(angleTypeDef, angleConverter);

    // Initialize color management.
    mx::ColorManagementSystemPtr cms = mx::DefaultColorManagementSystem::create(
        context.getShaderGenerator().getTarget());
    cms->loadLibrary(_document);
    context.getShaderGenerator().setColorManagementSystem(cms);

    // Initialize unit management.
    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(context.getShaderGenerator().getTarget());
    unitSystem->loadLibrary(_document);
    unitSystem->setUnitConverterRegistry(unitRegistry);
    context.getShaderGenerator().setUnitSystem(unitSystem);
    context.getOptions().targetDistanceUnit = "meter";

    // Publish only the shader's nodedef interface as parameters.  The graph
    // editor regenerates the shader whenever values change, so per-node runtime
    // uniforms are not needed.  The reduced interface also keeps the generated
    // parameter list small enough for backends with fixed socket limits.
    context.getOptions().shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;

    // Register type definitions.
    context.getShaderGenerator().registerTypeDefs(_document);
}

GraphEditorOslGenerator::Result
GraphEditorOslGenerator::generate(mx::TypedElementPtr element)
{
    _lastError.clear();

    if (!_document || !_context)
    {
        _lastError = "no document";
        return {};
    }

    try
    {
        mx::TypedElementPtr typedElem = element;
        if (!typedElem)
        {
            std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(_document);
            if (!elements.empty())
            {
                typedElem = elements[0];
            }
        }

        // Skip material nodes without upstream shaders.
        mx::NodePtr node = typedElem ? typedElem->asA<mx::Node>() : nullptr;
        if (node && node->getCategory() == mx::SURFACE_MATERIAL_NODE_STRING &&
            mx::getShaderNodes(node).empty())
        {
            typedElem = nullptr;
        }

        if (!typedElem)
        {
            return {};
        }

        mx::DocumentPtr genDoc = _document->copy();
        mx::FileSearchPath sourceSearchPath = _searchPath;
        sourceSearchPath.append(mx::getSourceSearchPath(_document));
        mx::flattenFilenames(genDoc, sourceSearchPath);
        mx::ElementPtr resolvedElem = genDoc->getDescendant(typedElem->getNamePath());
        mx::TypedElementPtr genElem = resolvedElem ? resolvedElem->asA<mx::TypedElement>() : nullptr;
        if (!genElem)
        {
            genDoc = _document;
            genElem = typedElem;
        }

        _context->clearUserData();
        const std::string shaderName = genElem->getNamePath();
        mx::ShaderPtr shader = _context->getShaderGenerator().generate(
            shaderName, genElem, *_context);

        const mx::ShaderStage& stage = shader->getStage(mx::Stage::PIXEL);
        const mx::VariableBlock& outputs = stage.getOutputBlock(mx::OSL::OUTPUTS);
        if (outputs.empty())
        {
            _lastError = "generated shader has no outputs";
            return {};
        }

        Result result;
        result.outputName = outputs[0]->getVariable();
        result.source = shader->getSourceCode(mx::Stage::PIXEL);
        return result;
    }
    catch (mx::Exception& e)
    {
        _lastError = e.what();
    }
    catch (std::exception& e)
    {
        _lastError = e.what();
    }

    return {};
}
