import re
import sys

from MaterialX.PyMaterialX import *

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

_stringTypeAliases = [unicode] if sys.version_info[0] < 3 else [bytes]
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
def isColorType(t):
    "Return True if the given type is a MaterialX color."
    return t == Color2 or t == Color3 or t == Color4


#--------------------------------------------------------------------------------
def isColorValue(val):
    "Return True if the given value is a MaterialX color."
    return isColorType(val.__class__)
