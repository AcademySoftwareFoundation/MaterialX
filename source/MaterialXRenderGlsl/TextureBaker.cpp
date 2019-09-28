//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/TextureBaker.h>

#include <MaterialXRenderGlsl/GLTextureHandler.h>
#include <MaterialXRenderGlsl/GlslValidator.h>

#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/Util.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXFormat/XmlIo.h>

namespace MaterialX
{

void TextureBaker::init(GenOptions& options, ElementPtr input, const std::string udim)
{
    std::string outputStr = input->getAttribute("nodegraph") + "_" + input->getAttribute("output");
    outputStr += (!udim.empty()) ? ("_" + udim) : "";
    options.textureSpaceInput = input->getName();
    options.textureSpaceInputType = input->getAttribute("type");
    options.textureSpaceOutput = outputStr;
}

void TextureBaker::bakeAllInputTextures(const FileSearchPath& searchPath, ElementPtr elem, GenContext context, const string& udim, const FilePath& outputFolder)
{
    context.getOptions().textureSpaceRender = true;
    _searchPath = searchPath;
    for (ElementPtr input : elem->getChildren())
    {
        if (!input->getAttribute("nodegraph").empty() && !input->getAttribute("output").empty())
        {
            // If the output from the nodegraph hasn't been baked yet
            std::string outputStr = input->getAttribute("nodegraph") + "_" + input->getAttribute("output");
            if (!udim.empty())
            {
                outputStr += "_" + udim;
            }
            if (_bakedOutputs.count(outputStr) == 0)
            {
                init(context.getOptions(), input, udim);
                bakeTextureFromElementInput(elem, context, outputFolder);
                recordNodegraphInput(outputStr, input->getAttribute("type"));
            }
            recordBakedTexture(input->getName(), outputStr);
        }
    }
}

void TextureBaker::bakeTextureFromElementInput(ElementPtr elem, GenContext& context, const FilePath& outputFolder)
{
    _rasterizer = GlslValidator::create(_frameBufferRes);
    StbImageLoaderPtr stbLoader = StbImageLoader::create();
    GLTextureHandlerPtr imageHandler = GLTextureHandler::create(stbLoader);
    imageHandler->setSearchPath(_searchPath);
    _rasterizer->setImageHandler(imageHandler);
    _rasterizer->initialize();

    _generator = GlslShaderGenerator::create();

    const std::string name = "" + elem->getName() + "_" + context.getOptions().textureSpaceOutput;
    ShaderPtr shader = _generator->generate("" + name + "_baker", elem, context);
    std::string vertexShader = shader->getSourceCode(Stage::VERTEX);
    std::string pixelShader = shader->getSourceCode(Stage::PIXEL);

    _rasterizer->validateCreation(shader);
    _rasterizer->renderScreenSpaceQuad(context);

    FilePath filename = outputFolder / FilePath(name + "." + _fileSuffix);
    _rasterizer->save(filename, false);
}

void TextureBaker::writeDocument(DocumentPtr& origDoc, TypedElementPtr elem, const FilePath& filename)
{
    // create doc
    DocumentPtr bakedTextureDoc = createDocument();

    // copy over all geominfo
    GeomInfoPtr newGeom;
    for (GeomInfoPtr geom : origDoc->getGeomInfos())
    {
        newGeom = bakedTextureDoc->addGeomInfo(geom->getName(), geom->getGeom());
        newGeom = geom;
        for (GeomAttrPtr attr : geom->getGeomAttrs())
        {
            bakedTextureDoc->getGeomInfo(geom->getName())->setGeomAttrValue(attr->getName(), attr->getType(), attr->getValueString());
        }
    }

    // create nodegraph
    NodeGraphPtr ng = bakedTextureDoc->addNodeGraph("NG_imgs");
    ng->setColorSpace("srgb_texture");
    for (std::map<std::string, std::string>::iterator it = _bakedOutputs.begin(); it != _bakedOutputs.end(); ++it)
    {
        // add the image node in the node graph
        NodePtr img_node = ng->addNode("image", "" + it->first + "_image",it->second);
        ParameterPtr param = img_node->addParameter("file", "filename");
        param->setValueString(elem->getName() + "_" + it->first + "." + _fileSuffix);
        std::string outputStr = it->first;
        std::string nodeName = img_node->getName();

        if (it->first.find("normal") != std::string::npos)
        {
            NodePtr normalmap_node = ng->addNode("normalmap", "" + it->first + "_normalmap", it->second);
            InputPtr input = normalmap_node->addInput("in", it->second);
            input->setNodeName(nodeName);
            nodeName = normalmap_node->getName();
        }

        // add the output node in the node graph
        OutputPtr output = ng->addOutput(outputStr, it->second);
        output->setNodeName(nodeName);
    }

    // create translated mat & shaderref
    MaterialPtr baked_mat = bakedTextureDoc->addMaterial("baked_material");
    ShaderRefPtr shaderRef = baked_mat->addShaderRef("" + elem->getName() + "_baked", elem->getAttribute("Shading model"));

    // fill in doc contents
    for (ElementPtr input : elem->getChildren())
    {
        std::string name = input->getName();
        std::string type = input->getAttribute("type");
        BindInputPtr bindInput = shaderRef->addBindInput(name, type);
        if (_bakedTextures.find(name) != _bakedTextures.end())
        {
            // add the bind input in shaderref
            bindInput->setOutputString(_bakedTextures[name]);
            bindInput->setNodeGraphString(ng->getName());
        }
        else
        {
            bindInput->setValueString(input->getAttribute("value"));
        }
    }

    XmlWriteOptions writeOptions;
    writeOptions.writeXIncludeEnable = false;
    writeToXmlFile(bakedTextureDoc, filename, &writeOptions);
}

} // namespace MaterialX
