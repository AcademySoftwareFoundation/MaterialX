void mx_backface_util( in bool backface_cullingbool, in float opacity, out vec3 result )
{ 
    bool isBackface = !gl_FrontFacing && backface_cullingbool;
    result = vec3(bool(opacity) || isBackface);
}
