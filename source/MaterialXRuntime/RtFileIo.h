//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTFILEIO_H
#define MATERIALX_RTFILEIO_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtStage.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

/// @class RtReadOptions
/// A set of options for controlling the behavior of read functions.
class RtReadOptions
{
  public:
    using ElementFilter = std::function<bool(const ElementPtr& elem)>;
    using LookModifier = std::function<DocumentPtr(const DocumentPtr& stageDoc, const DocumentPtr& loadedDoc)>;

  public:
    RtReadOptions();
    ~RtReadOptions() { }

    /// Filter function type used for filtering elements during read.
    /// If the filter returns false the element will not be read.
    ElementFilter elementFilter;

    /// Callback function for modifying the look information.
    LookModifier lookModifier;

    /// Read look information. The default value is false.
    bool readLookInformation;
};
    
/// @class RtWriteOptions
/// A set of options for controlling the behavior of write functions.
class RtWriteOptions
{
  public:
    /// Filter function type for filtering objects during write.
    using ObjectFilter = std::function<bool(const RtObject& obj)>;

    /// Filter function type for filtering attributes on object during write.
    using AttributeFilter = std::function<bool(const RtObject& obj, const RtString& name, const RtTypedValue* value)>;

  public:
    RtWriteOptions();
    ~RtWriteOptions() { }

    /// If true, elements with source file markings will be written as
    /// includes rather than explicit data.  Defaults to true.
    bool writeIncludes;

    /// If true, writes out nodegraph inputs. Default value is false.
    bool writeNodeGraphInputs;

    /// Write out default input values. The default value is false.
    bool writeDefaultValues;

    /// Filter function used for filtering objects during write.
    /// If the filter returns false the object will not be written.
    ObjectFilter objectFilter;

    /// Filter function used for filtering attributes during write.
    /// If the filter returns false the attribute will not be written.
    AttributeFilter attributeFilter;

    /// The desired major version
    unsigned int desiredMajorVersion;

    /// The desired minor version
    unsigned int desiredMinorVersion;
};

/// @class RtExportOptions
/// A set of options for controlling the behavior of export.
class RtExportOptions : public RtWriteOptions
{
  public:
    RtExportOptions();

    ~RtExportOptions() { }

    /// Whether to merge all of the looks/lookgroups into a single look
    bool mergeLooks;

    /// The name of the lookgroup to merge
    std::string lookGroupToMerge;

    /// Whether to flatten filenames
    bool flattenFilenames;

    /// User definition path used for flattening filenames
    FileSearchPath userDefinitionPath;

    /// User texture path used for flattening filenames
    FileSearchPath userTexturePath;

    /// String resolver applied during flattening filenames
    StringResolverPtr stringResolver;
};

/// API for read and write of data from MaterialX files
/// to runtime stages.
class RtFileIo
{
public:
    /// Constructor attaching this API to a stage.
    RtFileIo(RtStagePtr stage) :
        _stage(stage)
    {
    }

    /// Attach this API instance to a new stage.
    void setStage(RtStagePtr stage)
    {
        _stage = stage;
    }

    /// Read contents from a stream.
    /// If a filter is used only elements accepted by the filter
    /// will be red from the document.
    void read(std::istream& stream, const RtReadOptions* options = nullptr);

    /// Write all stage contents to stream.
    /// If a filter is used only elements accepted by the filter
    /// will be written to the document.
    void write(std::ostream& stream, const RtWriteOptions* options = nullptr);

    /// Read contents from a file path.
    /// If a filter is used only elements accepted by the filter
    /// will be read from the document.
    void read(const FilePath& documentPath, const FileSearchPath& searchPaths, const RtReadOptions* options = nullptr);

    /// Write all stage contents to a document.
    /// If a filter is used only elements accepted by the filter
    /// will be written to the document.
    void write(const FilePath& documentPath, const RtWriteOptions* options = nullptr);

    void writeDefinitions(std::ostream& stream, const RtStringVec& names, const RtWriteOptions* options = nullptr);
    void writeDefinitions(const FilePath& documentPath, const RtStringVec& names, const RtWriteOptions* options = nullptr);

    /// Read a prim from a stream.
    RtPrim readPrim(std::istream& stream, const RtPath& parentPrimPath, std::string& outOriginalPrimName, const RtReadOptions* options = nullptr);

    /// Write a prim to a stream.
    void writePrim(std::ostream& stream, const RtPath& primPath, const RtWriteOptions* options = nullptr);

    // Export to a stream
    void exportDocument(std::ostream& stream, const RtExportOptions* options = nullptr);

    // Export to a document
    void exportDocument(const FilePath& documentPath, const RtExportOptions* options = nullptr);

private:
    /// Read all contents from a file or folder into the attached stage.
    /// If path is a directory all MaterialX files under this folder will be read.
    /// Returns a set of all files read.
    StringSet readLibrary(const FilePath& path, const FileSearchPath& searchPaths, const RtReadOptions* options = nullptr);

    RtStagePtr _stage;
    friend class PvtApi;
};

}

#endif
