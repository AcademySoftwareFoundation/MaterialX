//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_XMLEXPORT_H
#define MATERIALX_XMLEXPORT_H

#include <MaterialXFormat/XmlIo.h>

namespace MaterialX
{

/// @class XmlExportOptions
/// A set of options for controlling the behavior of XML export functions.
class MX_FORMAT_API XmlExportOptions : public XmlWriteOptions
{
  public:
    XmlExportOptions();
    ~XmlExportOptions() { }

    /// Whether to merge all of the looks/lookgroups into a single look
    bool mergeLooks;

    /// The name of the lookgroup to merge
    std::string lookGroupToMerge;

    /// Whether to flatten filenames
    bool flattenFilenames;

    /// Resolved texture path for flattening filenames
    FileSearchPath resolvedTexturePath;

    /// String resolver applied during flattening filenames
    StringResolverPtr stringResolver;
};

/// @name Export Functions
/// @{

/// Export a Document as XML to the given output stream.
/// @param doc The Document to be written.
/// @param stream The output stream to which data is written.
/// @param exportOptions An optional pointer to an XmlExportOptions object.
///    If provided, then the given options will affect the behavior of the
///    export function.  Defaults to a null pointer.
MX_FORMAT_API void exportToXmlStream(DocumentPtr doc, std::ostream& stream, const XmlExportOptions* exportOptions = nullptr);

/// Export a Document as XML to the given filename.
/// @param doc The Document to be written.
/// @param filename The filename to which data is written.  This argument can
///    be supplied either as a FilePath or a standard string.
/// @param exportOptions An optional pointer to an XmlExportOptions object.
///    If provided, then the given options will affect the behavior of the
///    write function.  Defaults to a null pointer.
MX_FORMAT_API void exportToXmlFile(DocumentPtr doc, const FilePath& filename, const XmlExportOptions* exportOptions = nullptr);

/// Export a Document as XML to a new string, returned by value.
/// @param doc The Document to be written.
/// @param exportOptions An optional pointer to an XmlExportOptions object.
///    If provided, then the given options will affect the behavior of the
///    write function.  Defaults to a null pointer.
/// @return The output string, returned by value
MX_FORMAT_API string exportToXmlString(DocumentPtr doc, const XmlExportOptions* exportOptions = nullptr);

/// @}

} // namespace MaterialX

#endif