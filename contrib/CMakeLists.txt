option(BUILD_MATERIALX_FOR_MAYA "BUild MaterialX Exporter for Maya" OFF)

if (BUILD_MATERIALX_FOR_MAYA)	
  add_subdirectory(MaterialXForMaya)
endif()
