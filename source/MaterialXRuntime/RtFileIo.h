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

  public:
    RtReadOptions();
    ~RtReadOptions() { }

    /// Filter function type used for filtering elements during read.
    /// If the filter returns false the element will not be read.
    ElementFilter elementFilter;

    /// Read look information. The default value is false.
    bool readLookInformation;

    /// Apply the latest MaterialX feature updates. The default value is true.
    bool applyFutureUpdates;
};
    
/// @class RtWriteOptions
/// A set of options for controlling the behavior of write functions.
class RtWriteOptions
{
  public:
    /// Filter function type for filtering objects during write.
    using ObjectFilter = std::function<bool(const RtObject& obj)>;

    /// Filter function type for filtering metadata on object during write.
    using MetadataFilter = std::function<bool(const RtObject& obj, const RtToken& name, const RtTypedValue* value)>;

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

    /// Filter function used for filtering metadata during write.
    /// If the filter returns false the metadata will not be written.
    MetadataFilter metadataFilter;

    /// The desired major version
    unsigned int desiredMajorVersion;

    /// The desired minor version
    unsigned int desiredMinorVersion;

    /// Write uniforms as parameters
    bool writeUniformsAsParameters;
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

    /// Read contents from a stream
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

    void writeDefinitions(std::ostream& stream, const RtTokenVec& names, const RtWriteOptions* options = nullptr);
    void writeDefinitions(const FilePath& documentPath, const RtTokenVec& names, const RtWriteOptions* options = nullptr);

protected:
    /// Read all contents from one or more libraries.
    /// All MaterialX files found inside the given libraries will be read.
    void readLibraries(const FilePathVec& libraryPaths, const FileSearchPath& searchPaths, const RtReadOptions& options);
    friend class PvtApi;

private:
    RtStagePtr _stage;
};

}

#endif
