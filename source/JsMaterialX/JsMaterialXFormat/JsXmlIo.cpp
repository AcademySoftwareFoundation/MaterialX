//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include "../Helpers.h"
#include "./StrContainerTypeRegistration.h"
#include <MaterialXFormat/XmlIo.h>

#include <emscripten.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(xmlio)
{
  ems::constant("MTLX_EXTENSION", mx::MTLX_EXTENSION);
  ems::class_<mx::XmlReadOptions>("XmlReadOptions")
      .constructor<>()
      .property("readXIncludeFunction", &mx::XmlReadOptions::readXIncludeFunction)
      .property("readComments", &mx::XmlReadOptions::readComments)
      .property("generateUniqueNames", &mx::XmlReadOptions::generateUniqueNames)
      .property("parentXIncludes", &mx::XmlReadOptions::parentXIncludes);

  ems::class_<mx::XmlWriteOptions>("XmlWriteOptions")
      .constructor<>()
      .property("writeXIncludeEnable", &mx::XmlWriteOptions::writeXIncludeEnable)
      .property("elementPredicate", &mx::XmlWriteOptions::elementPredicate);

  ems::class_<mx::XmlExportOptions, ems::base<mx::XmlWriteOptions>>("XmlExportOptions")
      .constructor<>()
      .property("mergeLooks", &mx::XmlExportOptions::mergeLooks)
      .property("lookGroupToMerge", &mx::XmlExportOptions::lookGroupToMerge)
      .property("flattenFilenames", &mx::XmlExportOptions::flattenFilenames)
      .property("userDefinitionPath", &mx::XmlExportOptions::userDefinitionPath)
      .property("userTexturePath", &mx::XmlExportOptions::userTexturePath)
      .property("stringResolver", &mx::XmlExportOptions::stringResolver);

  BIND_FUNC_RAW_PTR("readFromXmlString", mx::readFromXmlString, 2, 3, mx::DocumentPtr , const std::string& , const mx::XmlReadOptions*);
  BIND_FUNC_RAW_PTR("writeToXmlFile", mx::writeToXmlFile, 2, 3, mx::DocumentPtr, const mx::FilePath&, const mx::XmlWriteOptions *);
  BIND_FUNC_RAW_PTR("writeToXmlString", mx::writeToXmlString, 1, 2, mx::DocumentPtr, const mx::XmlWriteOptions *);
  BIND_FUNC_RAW_PTR("exportToXmlFile", mx::exportToXmlFile, 2, 3, mx::DocumentPtr, const mx::FilePath&, const mx::XmlExportOptions*);
  BIND_FUNC_RAW_PTR("exportToXmlString", mx::exportToXmlString, 1, 2, mx::DocumentPtr, const mx::XmlExportOptions*);
  ems::function("prependXInclude", &mx::prependXInclude);
}
