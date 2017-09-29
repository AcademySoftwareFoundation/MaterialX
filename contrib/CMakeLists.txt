option(MATERIALX_BUILD_MAYA_EXPORTER "Build MaterialX Exporter for Maya" ON)

if (MATERIALX_BUILD_MAYA_EXPORTER)	
  add_subdirectory(MayaExporter)
endif()
