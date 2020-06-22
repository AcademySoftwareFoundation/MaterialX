#include "../helpers.h"
#include <MaterialXCore/Traversal.h>

#include <MaterialXCore/Observer.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(observer)
    {
        ems::class_<mx::Observer>("Observer")
            .smart_ptr_constructor("Observer", &std::make_shared<mx::Observer>)
            .smart_ptr<std::shared_ptr<const mx::Observer>>("Observer")
            .function("onAddElement", &mx::Observer::onAddElement)
            .function("onRemoveElement", &mx::Observer::onRemoveElement)
            .function("onSetAttribute", &mx::Observer::onSetAttribute)
            .function("onRemoveAttribute", &mx::Observer::onRemoveAttribute)
            .function("onCopyContent", &mx::Observer::onCopyContent)
            .function("onClearContent", &mx::Observer::onClearContent)
            .function("onRead", &mx::Observer::onRead)
            .function("onWrite", &mx::Observer::onWrite)
            .function("onBeginUpdate", &mx::Observer::onBeginUpdate)
            .function("onEndUpdate", &mx::Observer::onEndUpdate);

        ems::function("createObservedDocument", &mx::Document::createDocument<mx::ObservedDocument>);

        ems::class_<mx::ObservedDocument, ems::base<mx::Document>>("ObservedDocument")
            .smart_ptr_constructor("ObservedDocument", &std::make_shared<mx::ObservedDocument, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::ObservedDocument>>("ObservedDocument")
            .function("copy", &mx::ObservedDocument::copy)
            .function("addObserver", &mx::ObservedDocument::addObserver)
            .function("removeObserver", &mx::ObservedDocument::removeObserver)
            .function("clearObservers", &mx::ObservedDocument::clearObservers)
            .function("getUpdateScope", &mx::ObservedDocument::getUpdateScope)
            .function("enableCallbacks", &mx::ObservedDocument::enableCallbacks)
            .function("disableCallbacks", &mx::ObservedDocument::disableCallbacks);
    }
}