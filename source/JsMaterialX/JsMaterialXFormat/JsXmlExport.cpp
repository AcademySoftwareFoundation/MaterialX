//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include "../Helpers.h"
#include "./StrContainerTypeRegistration.h"
#include <MaterialXFormat/XmlExport.h>

#include <emscripten.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(xmlexport)
{
  ems::constant("MTLX_EXTENSION", mx::MTLX_EXTENSION);

  ems::class_<mx::XmlExportOptions, ems::base<mx::XmlWriteOptions>>("XmlExportOptions")
      .constructor<>()
      .property("mergeLooks", &mx::XmlExportOptions::mergeLooks)
      .property("lookGroupToMerge", &mx::XmlExportOptions::lookGroupToMerge)
      .property("flattenFilenames", &mx::XmlExportOptions::flattenFilenames)
      .property("resolvedTexturePath", &mx::XmlExportOptions::resolvedTexturePath)
      .property("stringResolver", &mx::XmlExportOptions::stringResolver);

  BIND_FUNC_RAW_PTR("exportToXmlFile", mx::exportToXmlFile, 2, 3, mx::DocumentPtr, const mx::FilePath&, const mx::XmlExportOptions*);
  BIND_FUNC_RAW_PTR("exportToXmlString", mx::exportToXmlString, 1, 2, mx::DocumentPtr, const mx::XmlExportOptions*);
}
