import re
import sys

from .PyMaterialX import *

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
                'matrix33'  : Matrix3x3,
                'matrix44'  : Matrix4x4,
                'string'    : str }
_typeToName = dict(reversed(i) for i in _nameToType.items())

_integerTypeAliases = [long] if sys.version_info[0] < 3 else []
_stringTypeAliases = [unicode] if sys.version_info[0] < 3 else [bytes]
_typeToName.update(dict.fromkeys(_integerTypeAliases, 'integer'))
_typeToName.update(dict.fromkeys(_stringTypeAliases, 'string'))


#--------------------------------------------------------------------------------
def typeToName(t):
    "Return the name of the given MaterialX type."
    if t in _typeToName:
        return _typeToName[t]
    raise TypeError('Unknown MaterialX type: ' + str(t))


#--------------------------------------------------------------------------------
def nameToType(name):
    "Convert a name to its corresponding MaterialX type."
    if name in _nameToType:
        return _nameToType[name]
    return str


#--------------------------------------------------------------------------------
def valueToString(value):
    "Convert a value of MaterialX type to a string."
    method = globals()['TypedValue_' + typeToName(type(value))].createValue
    return method(value).getValueString()


#--------------------------------------------------------------------------------
def stringToValue(string, t):
    "Convert a string to a value of MaterialX type."
    return Value.createValueFromStrings(string, typeToName(t)).getData()


#--------------------------------------------------------------------------------
def isColorType(t):
    "Return True if the given type is a MaterialX color."
    return t in (Color2, Color3, Color4)


#--------------------------------------------------------------------------------
def isColorValue(value):
    "Return True if the given value is a MaterialX color."
    return isColorType(type(value))
