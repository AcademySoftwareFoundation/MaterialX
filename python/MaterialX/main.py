from .PyMaterialX import *
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
    method = getattr(self.__class__, "_setValue" + typeToName(value.__class__))
    method(self, value, typeString)
    
def _getValue(self):
    "Return the typed value of an element."
    value = self._getValue()
    return value.getData() if value else None

ValueElement.setValue = _setValue
ValueElement.getValue = _getValue


#
# InterfaceElement
#

def _setParameterValue(self, name, value, typeString = ''):
    """Set the value of a parameter by its name, creating a child element
       to hold the parameter if needed."""
    method = getattr(self.__class__, "_setParameterValue" + typeToName(value.__class__))
    return method(self, name, value, typeString)

def _getParameterValue(self, name):
    """Return the value instance of a parameter by its name.  If the given parameter
       is not present, then None is returned."""
    valuePtr = self._getParameterValue(name)
    if (valuePtr is not None):
        return valuePtr.getData()
    return None

InterfaceElement.setParameterValue = _setParameterValue
InterfaceElement.getParameterValue = _getParameterValue


#
# NodeGraph
#

NodeGraph.addNode = NodeGraph._addNode


#
# Material
#

def _setOverrideValue(self, name, value, typeString = ''):
    """Set the value of an override by its name, creating a child element
       to hold the override if needed."""
    method = getattr(self.__class__, "_setOverrideValue" + typeToName(value.__class__))
    return method(self, name, value, typeString)

Material.setOverrideValue = _setOverrideValue
Material.addOverride = Material._addOverride
Material.addShaderRef = Material._addShaderRef


#
# GeomInfo
#

def _setGeomAttrValue(self, name, value, typeString = ''):
    """Set the value of a geomattr by its name, creating a child element
       to hold the geomattr if needed."""
    method = getattr(self.__class__, "_setGeomAttrValue" + typeToName(value.__class__))
    return method(self, name, value, typeString)

GeomInfo.setGeomAttrValue = _setGeomAttrValue
GeomInfo.addGeomAttr = GeomInfo._addGeomAttr


#
# Value
#

objectToString = valueToString
stringToObject = stringToValue


#
# XmlIo
#

readFromXmlFile = readFromXmlFileBase
