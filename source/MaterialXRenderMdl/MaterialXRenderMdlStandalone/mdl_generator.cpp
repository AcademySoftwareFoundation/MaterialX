/******************************************************************************
 * Copyright 2025 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/

#include "mdl_generator.h"
#include "utils/example_shared.h"

#include <mi/mdl_sdk.h>

#include <MaterialXCore/Material.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>


namespace mx = MaterialX;


class MdlStringResolver;
using MdlStringResolverPtr = std::shared_ptr<MdlStringResolver>;

class MdlStringResolver : public mx::StringResolver
{
    MdlStringResolver(mi::neuraylib::IMdl_configuration* mdl_configuration)
    {
        m_mdl_configuration = mi::base::make_handle_dup(mdl_configuration);
    }

public:

    /// Create a new string resolver.
    static MdlStringResolverPtr create(mi::neuraylib::IMdl_configuration* mdl_configuration)
    {
        return MdlStringResolverPtr(new MdlStringResolver(mdl_configuration));
    }

    ~MdlStringResolver()
    {
        m_mdl_configuration = nullptr;
    }

    void initialize(mx::DocumentPtr document)
    {
        // remove duplicates and keep order by using a set
        auto less = [](const mx::FilePath& lhs, const mx::FilePath& rhs) { return lhs.asString() < rhs.asString(); };
        std::set<mx::FilePath, decltype(less)> mtlx_paths(less);
        m_mtlx_document_paths.clear();
        m_mdl_search_paths.clear();

        // use the source search paths as base
        mx::FilePath p = mx::FilePath(document->getSourceUri()).getParentPath().getNormalized();
        mtlx_paths.insert(p);
        m_mtlx_document_paths.append(p);

        for (auto sp : mx::getSourceSearchPath(document))
        {
            sp = sp.getNormalized();
            if(sp.exists() && mtlx_paths.insert(sp).second)
                m_mtlx_document_paths.append(sp);
        }

        // add all search paths known to MDL
        for (size_t i = 0, n = m_mdl_configuration->get_mdl_paths_length(); i < n; i++)
        {
            mi::base::Handle<const mi::IString> sp_istring(m_mdl_configuration->get_mdl_path(i));
            p = mx::FilePath(sp_istring->get_c_str()).getNormalized();
            if (p.exists() && mtlx_paths.insert(p).second)
                m_mtlx_document_paths.append(p);

            // keep a list of MDL search paths for resource resolution
            m_mdl_search_paths.append(p);
        }
    }

    std::string resolve(const std::string& str, const std::string& type) const override
    {
        mx::FilePath normalizedPath = mx::FilePath(str).getNormalized();
        std::string resource_path;

        // in case the path is absolute we need to find a proper search path to put the file in
        if (normalizedPath.isAbsolute())
        {
            // find the highest priority search path that is a prefix of the resource path
            for (const auto& sp : m_mdl_search_paths)
            {
                if (sp.size() > normalizedPath.size())
                    continue;

                bool isParent = true;
                for (size_t i = 0; i < sp.size(); ++i)
                {
                    if (sp[i] != normalizedPath[i])
                    {
                        isParent = false;
                        break;
                    }
                }

                if (!isParent)
                    continue;

                // found a search path that is a prefix of the resource
                resource_path = normalizedPath.asString(mx::FilePath::FormatPosix).substr(
                    sp.asString(mx::FilePath::FormatPosix).size());
                if (resource_path[0] != '/')
                    resource_path = "/" + resource_path;
                return resource_path;
            }
        }
        else
        {
            // for relative paths we can try to find them in the MDL search paths, assuming
            // they are specified "relative" to a search path root.
            mi::base::Handle<mi::neuraylib::IMdl_entity_resolver> resolver(
                m_mdl_configuration->get_entity_resolver());

            resource_path = str;
            if (resource_path[0] != '/')
                resource_path = "/" + resource_path;

            mi::base::Handle<const mi::neuraylib::IMdl_resolved_resource> result(
                resolver->resolve_resource(resource_path.c_str(), nullptr, nullptr, 0, 0));

            if (result && result->get_count() > 0)
                return resource_path;
        }

        mi::examples::log::error("[MtlX2Mdl] MaterialX resource can not be accessed through an MDL search path. "
                                 "Dropping the resource from the Material. Resource Path: " +
                                 normalizedPath.asString());

        // drop the resource by returning the empty string.
        // alternatively, the resource could be copied into an MDL search path,
        // maybe even only temporary.
        return "";
    }

    // Get the MaterialX paths used to load the current document as well the current MDL search
    // paths in order to resolve resources by the MaterialX SDK.
    const mx::FileSearchPath& get_search_paths() const { return m_mtlx_document_paths; }

private:

    // SDK to get access to the entity resolver and the search path config
    mi::base::Handle<mi::neuraylib::IMdl_configuration> m_mdl_configuration;

    // List of paths from which MaterialX can locate resources.
    // This includes the document folder and the search paths used to load the document.
    mx::FileSearchPath m_mtlx_document_paths;

    // List of MDL search paths from which we can locate resources.
    // This is only a subset of the MaterialX document paths and needs to be extended by using the
    // `--mdl_path` option when starting the application if needed.
    mx::FileSearchPath m_mdl_search_paths;
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

MdlGenerator::MdlGenerator()
    : _mtlxSearchPaths()
    , _mtlxRelativeLibraryPaths()
    , _mdlVersion(mx::GenMdlOptions::MdlVersion::MDL_LATEST)
{
}

// ------------------------------------------------------------------------------------------------

void MdlGenerator::AddMaterialxSearchPath(const std::string& mtlx_path)
{
    _mtlxSearchPaths.push_back(mtlx_path);
    std::replace(_mtlxSearchPaths.back().begin(), _mtlxSearchPaths.back().end(), '/', '\\');
}

// ------------------------------------------------------------------------------------------------

void MdlGenerator::AddMaterialxLibrary(const std::string& mtlx_library)
{
    _mtlxRelativeLibraryPaths.push_back(mtlx_library);
    std::replace(_mtlxRelativeLibraryPaths.back().begin(),
        _mtlxRelativeLibraryPaths.back().end(), '/', '\\');
}

// ------------------------------------------------------------------------------------------------

void MdlGenerator::SetMdlVersion(mx::GenMdlOptions::MdlVersion target_version)
{
    _mdlVersion = target_version;
}

// ------------------------------------------------------------------------------------------------

void MdlGenerator::SetFileTextureVerticalFlip(bool flip)
{
    _fileTextureVerticalFlip = flip;
}

// ------------------------------------------------------------------------------------------------

bool MdlGenerator::SetSource(const std::string& mtlx_material, const std::string& material_name)
{
    if (!mi::examples::io::file_exists(mtlx_material))
    {
        mi::examples::log::error("[MtlX2Mdl] Material path does not exist: " + mtlx_material);
        return false;
    }

    _mtlxSource = mtlx_material;
    _mtlxMaterialName = material_name;
    std::replace(_mtlxSource.begin(), _mtlxSource.end(), '/', '\\');
    return true;
}

// ------------------------------------------------------------------------------------------------

bool MdlGenerator::Generate(mi::neuraylib::IMdl_configuration* mdl_configuration, Result& inout_result) const
{
    // Initialize the standard library
    mx::DocumentPtr mtlx_std_lib;
    mx::StringSet mtlx_include_files;
    mx::FilePathVec mtlx_library_folders = { "libraries" };
    mx::FileSearchPath mtlx_search_path;
    mtlx_search_path.append(
        mx::FilePath{ mi::examples::io::get_executable_folder() + "/autodesk_materialx" });

    // add additional search paths
    for (auto& p : _mtlxSearchPaths)
        mtlx_search_path.append(mx::FilePath{ p });

    // add additional relative library paths
    for (auto& l : _mtlxRelativeLibraryPaths)
        mtlx_library_folders.push_back(mx::FilePath{ l });

    try
    {
        mtlx_std_lib = mx::createDocument();
        mtlx_include_files = mx::loadLibraries(
            mtlx_library_folders, mtlx_search_path, mtlx_std_lib);
        if (mtlx_include_files.empty())
        {
            mi::examples::log::error("[MtlX2Mdl] Could not find standard data libraries on the given search path: " +
                                     mtlx_search_path.asString());
        }

    }
    catch (std::exception& e)
    {
        mi::examples::log::error("[MtlX2Mdl] Failed to initialize standard libraries:", e);
    }

    // Initialize unit management.
    mx::UnitConverterRegistryPtr mtlx_unit_registry = mx::UnitConverterRegistry::create();
    mx::UnitTypeDefPtr distanceTypeDef = mtlx_std_lib->getUnitTypeDef("distance");
    mx::LinearUnitConverterPtr _distanceUnitConverter = mx::LinearUnitConverter::create(distanceTypeDef);
    mtlx_unit_registry->addUnitConverter(distanceTypeDef, _distanceUnitConverter);
    mx::UnitTypeDefPtr angleTypeDef = mtlx_std_lib->getUnitTypeDef("angle");
    mx::LinearUnitConverterPtr angleConverter = mx::LinearUnitConverter::create(angleTypeDef);
    mtlx_unit_registry->addUnitConverter(angleTypeDef, angleConverter);

    // Create the list of supported distance units.
    mx::StringVec _distanceUnitOptions;
    auto unitScales = _distanceUnitConverter->getUnitScale();
    _distanceUnitOptions.resize(unitScales.size());
    for (auto unitScale : unitScales)
    {
        int location = _distanceUnitConverter->getUnitAsInteger(unitScale.first);
        _distanceUnitOptions[location] = unitScale.first;
    }

    // Initialize the generator contexts.
    mx::GenContext generator_context = mx::MdlShaderGenerator::create();

    // Initialize search paths.
    for (const mx::FilePath& path : mtlx_search_path)
    {
        for (const auto folder : mtlx_library_folders)
        {
            if (folder.size() > 0)
                generator_context.registerSourceCodeSearchPath(path / folder);
        }
    }

    // Initialize color management.
    mx::DefaultColorManagementSystemPtr cms = mx::DefaultColorManagementSystem::create(
        generator_context.getShaderGenerator().getTarget());
    cms->loadLibrary(mtlx_std_lib);
    generator_context.getShaderGenerator().setColorManagementSystem(cms);
    generator_context.getOptions().targetColorSpaceOverride = "lin_rec709";
    // The MDL and Mtlx spec define the origin (0,0) of the uv spaces at the bottom left.
    // No flipping requiered.
    generator_context.getOptions().fileTextureVerticalFlip = false; 

    // Initialize unit management.
    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(
        generator_context.getShaderGenerator().getTarget());
    unitSystem->loadLibrary(mtlx_std_lib);
    unitSystem->setUnitConverterRegistry(mtlx_unit_registry);
    generator_context.getShaderGenerator().setUnitSystem(unitSystem);
    generator_context.getOptions().targetDistanceUnit = "meter";

    // load the actual material
    if (_mtlxSource.empty())
    {
        mi::examples::log::error("[MtlX2Mdl] Source file not specified.");
        return false;
    }

    // Set up read options.
    mx::XmlReadOptions readOptions;
    readOptions.readXIncludeFunction = [](mx::DocumentPtr doc, const mx::FilePath& filename,
        const mx::FileSearchPath& searchPath, const mx::XmlReadOptions* options)
    {
        mx::FilePath resolvedFilename = searchPath.find(filename);
        if (resolvedFilename.exists())
        {
            readFromXmlFile(doc, resolvedFilename, searchPath, options);
        }
        else
        {
            mi::examples::log::error("[MtlX2Mdl] Include file not found: " + filename.asString());
        }
    };

    // Clear user data on the generator.
    generator_context.clearUserData();

    // Specify the MDL target version.
    // Using the latest by default.
    mx::GenMdlOptionsPtr genMdlOptions = std::make_shared<mx::GenMdlOptions>();
    genMdlOptions->targetVersion = _mdlVersion;
    generator_context.pushUserData(mx::GenMdlOptions::GEN_CONTEXT_USER_DATA_KEY, genMdlOptions);

    // Note, this should only be needed when image space and texture coordinate space of the vertex data
    // are not aligned properly.
    generator_context.getOptions().fileTextureVerticalFlip = _fileTextureVerticalFlip;

    // Load source document.
    mx::DocumentPtr material_document = mx::createDocument();
    mx::FilePath material_filename = _mtlxSource;
    mx::readFromXmlFile(material_document, _mtlxSource, mtlx_search_path, &readOptions);

    // Import libraries.
    material_document->importLibrary(mtlx_std_lib);

    // flatten the resource paths of the document using a custom resolver allows
    // the change the resource URIs into valid MDL paths.
    auto custom_resolver = MdlStringResolver::create(mdl_configuration);
    custom_resolver->initialize(material_document);
    mx::flattenFilenames(material_document, custom_resolver->get_search_paths(), custom_resolver);

    // Validate the document.
    std::string message;
    if (!material_document->validate(&message))
    {
        // materialX validation failures do not mean that content can not be rendered.
        // it points to mtlx authoring errors but rendering could still be fine.
        // since MDL is robust against erroneous code we just continue. If there are problems
        // in the generated code, we detect it on module load and use a fall-back material.
        mi::examples::log::warning("[MtlX2Mdl] Validation warnings for '" + _mtlxSource + "'\n" + message);
    }

    // find (selected) renderable nodes
    mx::TypedElementPtr element_to_generate_code_for;
    if (!_mtlxMaterialName.empty())
    {
        mx::ElementPtr elem = material_document->getRoot();
        std::vector<std::string> path = mi::examples::strings::split(_mtlxMaterialName, '/');
        for (size_t i = 0; i < path.size(); ++i)
        {
            elem = elem->getChild(path[i]);
            if (!elem)
                break;
        }
        // if a node is specified properly, there is only one
        if (elem)
        {
            mx::TypedElementPtr typedElem = elem ? elem->asA<mx::TypedElement>() : nullptr;
            if (typedElem)
                element_to_generate_code_for = typedElem;
        }
    }
    else
    {
        // find the first render-able element
        std::vector<mx::TypedElementPtr> elems;
        elems = mx::findRenderableElements(material_document);
        if (elems.size() > 0)
        {
            element_to_generate_code_for = elems[0];
        }
    }

    if (!element_to_generate_code_for)
    {
        if (!_mtlxMaterialName.empty())
            mi::examples::log::error("[MtlX2Mdl] Code generation failure: no material named '" +
                _mtlxMaterialName + "' found in '" + _mtlxSource + "'");
        else
            mi::examples::log::error("[MtlX2Mdl] Code generation failure: no material found in '"
                + _mtlxSource + "'");

        return false;
    }

    // Clear cached implementations, in case libraries on the file system have changed.
    generator_context.clearNodeImplementations();

    std::string material_name = element_to_generate_code_for->getNamePath();
    material_name = mi::examples::strings::replace(material_name, '/', '_');

    mx::ShaderPtr shader = nullptr;
    try
    {
        shader =
            generator_context.getShaderGenerator().generate(material_name, element_to_generate_code_for, generator_context);
    }
    catch (mx::Exception& e)
    {
        mi::examples::log::exception("[MtlX2Mdl] Code generation failure: ", e);
        return false;
    }

    if (!shader)
    {
        mi::examples::log::error("[MtlX2Mdl] Failed to generate shader for element: " + material_name);
        return false;
    }

    auto generated = shader->getSourceCode("pixel");
    if (generated.empty())
    {
        mi::examples::log::error("[MtlX2Mdl] Failed to generate source code for stage.");
        return false;
    }

    inout_result.materialxFilename = _mtlxSource;
    inout_result.materialxMaterialName = material_name;
    inout_result.generatedMdlCode = 
        std::string("// generated from MaterialX using the SDK version ") + MaterialX::getVersionString() + "\n\n" +
        generated;
    inout_result.generatedMdlName = shader->getStage("pixel").getFunctionName();
    return true;
}
