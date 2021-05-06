#include "../helpers.h"
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXCore/Document.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(xmlio)
    {
        ems::class_<mx::XmlReadOptions>("XmlReadOptions")
            .constructor<>()
            .property("readXIncludeFunction", &mx::XmlReadOptions::readXIncludeFunction)
            .property("parentXIncludes", &mx::XmlReadOptions::parentXIncludes);
        ems::class_<mx::XmlWriteOptions>("XmlWriteOptions")
            .constructor<>()
            .property("writeXIncludeEnable", &mx::XmlWriteOptions::writeXIncludeEnable)
            .property("elementPredicate", &mx::XmlWriteOptions::elementPredicate);

        ems::function("readFromXmlString", ems::optional_override([](mx::DocumentPtr doc, std::string str, mx::XmlReadOptions readOptions = mx::XmlReadOptions()) {
                     return mx::readFromXmlString(doc, (const std::string &)str, (const mx::XmlReadOptions *)&readOptions);
                 }));

        ems::function("writeToXmlString", ems::optional_override([](mx::DocumentPtr doc, mx::XmlWriteOptions writeOptions = mx::XmlWriteOptions()) {
                     return mx::writeToXmlString(doc, (const mx::XmlWriteOptions *)&writeOptions);
                 }));
    }
}
