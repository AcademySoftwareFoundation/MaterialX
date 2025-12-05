if (NOT MATERIALX_BUILD_OIIO)
    set(OIIO_EXCLUDE_FILES PyOiioImageLoader.cpp)
endif()

mx_add_python_module(PyMaterialXRender
    LIBRARIES
        PyMaterialXGenShader
        MaterialXRender
    EXCLUDE_FILES
        ${OIIO_EXCLUDE_FILES})
