import sys

from .PyMaterialXCore import *

"""
Native Python helper functions for MaterialX data types.
"""


#--------------------------------------------------------------------------------
_nameToType = { 'integer'   : int,
                'float'     : float,
                'boolean'   : bool,
                'color2'    : Color2,
                'color3'    : Color3,
                'color4'    : Color4,
                'vector2'   : Vector2,
                'vector3'   : Vector3,
                'vector4'   : Vector4,
                'matrix33'  : Matrix33,
                'matrix44'  : Matrix44,
                'string'    : str }
_typeToName = dict(reversed(i) for i in _nameToType.items())

_integerTypeAliases = [long] if sys.version_info[0] < 3 else []
_stringTypeAliases = [unicode] if sys.version_info[0] < 3 else [bytes]
_typeToName.update(dict.fromkeys(_integerTypeAliases, 'integer'))
_typeToName.update(dict.fromkeys(_stringTypeAliases, 'string'))


#--------------------------------------------------------------------------------
def typeToName(t):
    """Return the MaterialX type string associated with the given Python type
       If the given Python type is not recognized by MaterialX, then None is
       returned.

       Examples:
           typeToName(int) -> 'integer'
           typeToName(mx.Color3) -> 'color3'"""

    if t in _typeToName:
        return _typeToName[t]
    return None


#--------------------------------------------------------------------------------
def nameToType(name):
    """Return the Python type associated with the given MaterialX type string.
       If the given type string is not recognized by MaterialX, then the Python
       str type is returned.

       Examples:
           nameToType('integer') -> int
           nameToType('color3') -> mx.Color3"""

    if name in _nameToType:
        return _nameToType[name]
    return str


#--------------------------------------------------------------------------------
def valueToString(value):
    """Convert a Python value of any supported type to its correponding
       MaterialX value string.  If the Python type of the value is not
       recognized by MaterialX, then None is returned.

       Examples:
           valueToString(0.1) -> '0.1'
           valueToString(mx.Color3(0.1, 0.2, 0.3)) -> '0.1, 0.2, 0.3'"""

    typeString = typeToName(type(value))
    if not typeString:
        return None
    method = globals()['TypedValue_' + typeString].createValue
    return method(value).getValueString()


#--------------------------------------------------------------------------------
def stringToValue(string, t):
    """Convert a MaterialX value string and Python type to the corresponding
       Python value.  If the given conversion cannot be performed, then None
       is returned.

       Examples:
           stringToValue('0.1', float) -> 0.1
           stringToValue('0.1, 0.2, 0.3', mx.Color3) -> mx.Color3(0.1, 0.2, 0.3)"""

    typeString = typeToName(t)
    if not typeString:
        return None
    valueObj = Value.createValueFromStrings(string, typeString)
    if not valueObj:
        return None
    return valueObj.getData()


#--------------------------------------------------------------------------------
def isColorType(t):
    "Return True if the given type is a MaterialX color."
    return t in (Color2, Color3, Color4)


#--------------------------------------------------------------------------------
def isColorValue(value):
    "Return True if the given value is a MaterialX color."
    return isColorType(type(value))
