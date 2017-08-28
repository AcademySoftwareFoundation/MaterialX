option(MATERIALX_BUILD_MAYA_EXPORTER "Build MaterialX Exporter for Maya" OFF)

if (MATERIALX_BUILD_MAYA_EXPORTER)	
  add_subdirectory(MayaExporter)
endif()
