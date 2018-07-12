//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_OBSERVER
#define MATERIALX_OBSERVER

/// @file
/// Observer classes

#include <MaterialXCore/Document.h>

namespace MaterialX
{

class Observer;
class ObservedDocument;

/// A shared pointer to an Observer
using ObserverPtr = shared_ptr<Observer>;
/// A shared pointer to a const Observer
using ConstObserverPtr = shared_ptr<const Observer>;

/// A shared pointer to an ObservedDocument
using ObservedDocumentPtr = shared_ptr<ObservedDocument>;
/// A shared pointer to a const ObservedDocument
using ConstObservedDocumentPtr = shared_ptr<const ObservedDocument>;

/// @class Observer
/// An observer of a MaterialX Document.
///
/// An observer may be registered with a supporting document, and will receive
/// callbacks when the document is modified.
class Observer
{
  public:
    Observer() { }
    virtual ~Observer() { }

    /// Called when an element is added to the element tree.
    virtual void onAddElement(ElementPtr /*parent*/, ElementPtr /*elem*/) { }

    /// Called when an element is removed from the element tree.
    virtual void onRemoveElement(ElementPtr /*parent*/, ElementPtr /*elem*/) { }

    /// Called when an attribute of an element is set to a new value.
    virtual void onSetAttribute(ElementPtr /*elem*/, const string& /*attrib*/, const string& /*value*/) { }

    /// Called when an attribute of an element is removed.
    virtual void onRemoveAttribute(ElementPtr /*elem*/, const string& /*attrib*/) { }

    /// Called when a document is initialized.
    virtual void onInitialize() { }

    /// Called when data is read into the current document.
    virtual void onRead() { }

    /// Called when data is written from the current document.
    virtual void onWrite() { }

    /// Called before a set of document updates is performed.
    virtual void onBeginUpdate() { }

    /// Called after a set of document updates is performed.
    virtual void onEndUpdate() { }
};

/// @class ObservedDocument
/// A MaterialX document with support for registering observers
class ObservedDocument : public Document
{
  public:
    ObservedDocument(ElementPtr parent, const string& name) :
        Document(parent, name),
        _updateScope(0),
        _callbacksEnabled(true)
    {
    }
    virtual ~ObservedDocument() { }

    /// @name Observers
    /// @{

    /// Add an observer.
    bool addObserver(const string& name, ObserverPtr observer)
    {
        if (_observerMap.find(name) != _observerMap.end())
        {
            return false;
        }
        _observerMap[name] = observer;
        return true;
    }

    /// Remove an observer.
    bool removeObserver(const string& name)
    {
        auto it = _observerMap.find(name);
        if (it == _observerMap.end())
        {
            return false;
        }

        _observerMap.erase(it);
        return true;
    }

    /// Clear all observers.
    void clearObservers()
    {
        _observerMap.clear();
    }

    /// @}
    /// @name Updates
    /// @{

    /// Return the scope depth of update notifications.  This can be used
    /// to prevent nested update notifications when multiple document changes
    /// are made.
    int getUpdateScope() const
    {
        return _updateScope;
    }

    /// @}
    /// @name Document Overrides
    /// @{

    void initialize() override
    {
        Document::initialize();
        _updateScope = 0;
    }

    void enableCallbacks() override
    {
        _callbacksEnabled = true;
    }

    void disableCallbacks() override
    {
        _callbacksEnabled = false;
    }

    DocumentPtr copy() override
    {
        DocumentPtr doc = createDocument<ObservedDocument>();
        doc->copyContentFrom(getSelf());
        return doc;
    }

    void onAddElement(ElementPtr parent, ElementPtr elem) override
    {
        Document::onAddElement(parent, elem);

        if (_callbacksEnabled)
        {
            for (auto& item : _observerMap)
            {
                item.second->onAddElement(parent, elem);
            }
        }
    }

    void onRemoveElement(ElementPtr parent, ElementPtr elem) override
    {
        Document::onRemoveElement(parent, elem);
        if (_callbacksEnabled)
        {
            for (auto& item : _observerMap)
            {
                item.second->onRemoveElement(parent, elem);
            }
        }
    }

    void onSetAttribute(ElementPtr elem, const string& attrib, const string& value) override
    {
        Document::onSetAttribute(elem, attrib, value);
        if (_callbacksEnabled)
        {
            for (auto& item : _observerMap)
            {
                item.second->onSetAttribute(elem, attrib, value);
            }
        }
    }

    void onRemoveAttribute(ElementPtr elem, const string& attrib) override
    {
        Document::onRemoveAttribute(elem, attrib);
        if (_callbacksEnabled)
        {
            for (auto& item : _observerMap)
            {
                item.second->onRemoveAttribute(elem, attrib);
            }
        }
    }

    void onInitialize() override
    {
        if (_callbacksEnabled)
        {
            for (auto& item : _observerMap)
            {
                item.second->onInitialize();
            }
        }
    }

    void onRead() override
    {
        if (_callbacksEnabled)
        {
            for (auto& item : _observerMap)
            {
                item.second->onRead();
            }
        }
    }

    void onWrite() override
    {
        if (_callbacksEnabled)
        {
            for (auto& item : _observerMap)
            {
                item.second->onWrite();
            }
        }
    }

    void onBeginUpdate() override
    {
        // Only send notification for the outermost scope.
        if (!getUpdateScope())
        {
            if (_callbacksEnabled)
            {
                for (auto& item : _observerMap)
                {
                    item.second->onBeginUpdate();
                }
            }
        }

        _updateScope++;
    }

    void onEndUpdate() override
    {
        _updateScope = std::max(_updateScope - 1, 0);

        // Only send notification for the outermost scope.
        if (!getUpdateScope())
        {
            if (_callbacksEnabled)
            {
                for (auto& item : _observerMap)
                {
                    item.second->onEndUpdate();
                }
            }
        }
    }

    /// @}

  private:
    std::unordered_map<string, ObserverPtr> _observerMap;
    int _updateScope;
    bool _callbacksEnabled;
};

} // namespace MaterialX

#endif
