//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/XmlExport.h>

#include <MaterialXFormat/Util.h>

namespace MaterialX
{

namespace
{

void mergeLooks(DocumentPtr doc, const XmlExportOptions* exportOptions)
{
    if (exportOptions && exportOptions->mergeLooks)
    {
        doc->mergeLooks(exportOptions->lookGroupToMerge);
    }
}

} // anonymous namespace

//
// XmlExportOptions methods
//

XmlExportOptions::XmlExportOptions() :
    XmlWriteOptions(),
    mergeLooks(false)
{
}

//
// Exporting
//

void exportToXmlStream(DocumentPtr doc, std::ostream& stream, const XmlExportOptions* exportOptions)
{
    mergeLooks(doc, exportOptions);
    if (exportOptions && exportOptions->flattenFilenames)
    {
        flattenFilenames(doc, exportOptions->resolvedTexturePath, exportOptions->stringResolver);
    }
    writeToXmlStream(doc, stream, exportOptions);
}

void exportToXmlFile(DocumentPtr doc, const FilePath& filename, const XmlExportOptions* exportOptions)
{
    mergeLooks(doc, exportOptions);
    if (exportOptions && exportOptions->flattenFilenames)
    {
        flattenFilenames(doc, exportOptions->resolvedTexturePath, exportOptions->stringResolver);
    }
    writeToXmlFile(doc, filename, exportOptions);
}

string exportToXmlString(DocumentPtr doc, const XmlExportOptions* exportOptions)
{
    mergeLooks(doc, exportOptions);
    if (exportOptions && exportOptions->flattenFilenames)
    {
        flattenFilenames(doc, exportOptions->resolvedTexturePath, exportOptions->stringResolver);
    }
    return writeToXmlString(doc, exportOptions);
}

} // namespace MaterialX
