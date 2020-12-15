import warnings

from .PyMaterialXCore import *
from .PyMaterialXFormat import *
from .datatype import *

"""
Native Python wrappers for PyMaterialX, providing a more Pythonic interface
for Elements and Values.
"""


#
# Element
#

def _isA(self, elementClass, category = ''):
    """Return True if this element is an instance of the given subclass.
       If a category string is specified, then both subclass and category
       matches are required."""
    if not isinstance(self, elementClass):
        return False
    if category and self.getCategory() != category:
        return False
    return True

def _addChild(self, elementClass, name, typeString = ''):
    "Add a child element of the given subclass, name, and optional type string."
    method = getattr(self.__class__, "_addChild" + elementClass.__name__)
    return method(self, name, typeString)

def _getChild(self, name):
    "Return the child element, if any, with the given name."
    if (name == None):
        return None
    return self._getChild(name)

def _getChildOfType(self, elementClass, name):
    "Return the child element, if any, with the given name and subclass."
    method = getattr(self.__class__, "_getChildOfType" + elementClass.__name__)
    return method(self, name)

def _getChildrenOfType(self, elementClass):
    """Return a list of all child elements that are instances of the given type.
       The returned list maintains the order in which children were added."""
    method = getattr(self.__class__, "_getChildrenOfType" + elementClass.__name__)
    return method(self)

def _removeChildOfType(self, elementClass, name):
    "Remove the typed child element, if any, with the given name."
    method = getattr(self.__class__, "_removeChildOfType" + elementClass.__name__)
    method(self, name)

Element.isA = _isA
Element.addChild = _addChild
Element.getChild = _getChild
Element.getChildOfType = _getChildOfType
Element.getChildrenOfType = _getChildrenOfType
Element.removeChildOfType = _removeChildOfType


#
# ValueElement
#

def _setValue(self, value, typeString = ''):
    "Set the typed value of an element."
    method = getattr(self.__class__, "_setValue" + getTypeString(value))
    method(self, value, typeString)

def _getValue(self):
    "Return the typed value of an element."
    value = self._getValue()
    return value.getData() if value else None

def _getDefaultValue(self):
    """Return the default value for this element."""
    value = self._getDefaultValue()
    return value.getData() if value else None

ValueElement.setValue = _setValue
ValueElement.getValue = _getValue
ValueElement.getDefaultValue = _getDefaultValue


#
# InterfaceElement
#

def _setParameterValue(self, name, value, typeString = ''):
    """Set the typed value of a parameter by its name, creating a child element
       to hold the parameter if needed."""
    method = getattr(self.__class__, "_setParameterValue" + getTypeString(value))
    return method(self, name, value, typeString)

def _getParameterValue(self, name, target = ''):
    """Return the typed value of a parameter by its name, taking both the
       calling element and its declaration into account.  If the given
       parameter is not found, then None is returned."""
    value = self._getParameterValue(name, target)
    return value.getData() if value else None

def _getParameterValueString(self, name):
    """(Deprecated) Return the value string of a parameter by its name.  If the
       given parameter is not present, then an empty string is returned."""
    warnings.warn("This function is deprecated; call InterfaceElement.getParameter() and Parameter.getValueString() instead.", DeprecationWarning, stacklevel = 2)
    param = self.getParameter(name)
    return param.getValueString() if param else ""

def _setInputValue(self, name, value, typeString = ''):
    """Set the typed value of an input by its name, creating a child element
       to hold the input if needed."""
    method = getattr(self.__class__, "_setInputValue" + getTypeString(value))
    return method(self, name, value, typeString)

def _getInputValue(self, name, target = ''):
    """Return the typed value of an input by its name, taking both the
       calling element and its declaration into account.  If the given
       input is not found, then None is returned."""
    value = self._getInputValue(name, target)
    return value.getData() if value else None

InterfaceElement.setParameterValue = _setParameterValue
InterfaceElement.getParameterValue = _getParameterValue
InterfaceElement.getParameterValueString = _getParameterValueString
InterfaceElement.setInputValue = _setInputValue
InterfaceElement.getInputValue = _getInputValue


#
# Node
#

def _getReferencedNodeDef(self):
    "(Deprecated) Return the first NodeDef that declares this node."
    warnings.warn("This function is deprecated; call Node.getNodeDef instead.", DeprecationWarning, stacklevel = 2)
    return self.getNodeDef()

Node.getReferencedNodeDef = _getReferencedNodeDef


#
# GraphElement
#

def _addNode(self, category, name = '', typeString = DEFAULT_TYPE_STRING):
    "Add a node to the graph."
    return self._addNode(category, name, typeString)

GraphElement.addNode = _addNode


#
# PropertySet
#

def _setPropertyValue(self, name, value, typeString = ''):
    """Set the typed value of a property by its name, creating a child element
       to hold the property if needed."""
    method = getattr(self.__class__, "_setPropertyValue" + getTypeString(value))
    return method(self, name, value, typeString)

def _getPropertyValue(self, name, target = ''):
    """Return the typed value of a property by its name.  If the given property
       is not found, then None is returned."""
    value = self._getPropertyValue(name)
    return value.getData() if value else None

PropertySet.setPropertyValue = _setPropertyValue
PropertySet.getPropertyValue = _getPropertyValue


#
# GeomInfo
#

def _setGeomPropValue(self, name, value, typeString = ''):
    """Set the value of a geomprop by its name, creating a child element
       to hold the geomprop if needed."""
    method = getattr(self.__class__, "_setGeomPropValue" + getTypeString(value))
    return method(self, name, value, typeString)

GeomInfo.setGeomPropValue = _setGeomPropValue

GeomInfo.addGeomAttr = GeomInfo.addGeomProp
GeomInfo.setGeomAttrValue = GeomInfo.setGeomPropValue


#
# Document
#

def _applyStringSubstitutions(self, filename, geom = '/'):
    """(Deprecated) Given an input filename and geom string, apply any string
        substitutions that have been defined for the given geom to the filename,
        returning the modified filename."""
    warnings.warn("This function is deprecated; call Element.createStringResolver() instead.", DeprecationWarning, stacklevel = 2)
    return self.createStringResolver(geom).resolve(filename, 'filename')

def _generateRequireString(self):
    """(Deprecated) Generate the require string for a document."""
    warnings.warn("Require strings are no longer supported in MaterialX.", DeprecationWarning, stacklevel = 2)

Document.applyStringSubstitutions = _applyStringSubstitutions
Document.generateRequireString = _generateRequireString


#
# Value
#

def _typeToName(t):
    "(Deprecated) Return the MaterialX type string associated with the given Python type."
    warnings.warn("This function is deprecated; call MaterialX.getTypeString instead.", DeprecationWarning, stacklevel = 2)
    return getTypeString(t())

def _valueToString(value):
    "(Deprecated) Convert a Python value to its correponding MaterialX value string."
    warnings.warn("This function is deprecated; call MaterialX.getValueString instead.", DeprecationWarning, stacklevel = 2)
    return getValueString(value)

def _stringToValue(string, t):
    "(Deprecated) Convert a MaterialX value string and Python type to the corresponding Python value."
    warnings.warn("This function is deprecated; call MaterialX.createValueFromStrings instead.", DeprecationWarning, stacklevel = 2)
    return createValueFromStrings(string, getTypeString(t()))

typeToName = _typeToName
valueToString = _valueToString
stringToValue = _stringToValue


#
# XmlIo
#

readFromXmlFile = readFromXmlFileBase
