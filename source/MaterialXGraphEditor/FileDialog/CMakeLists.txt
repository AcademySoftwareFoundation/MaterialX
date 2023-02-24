set(materialx_filedialog_sources ${CMAKE_CURRENT_SOURCE_DIR}/FileDialog.h ${CMAKE_CURRENT_SOURCE_DIR}/FileDialog.cpp)
set(materialx_filedialog_libraries "")

if (APPLE)
    list(APPEND materialx_filedialog_sources ${CMAKE_CURRENT_SOURCE_DIR}/FileDialog.mm)
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/FileDialog.mm" PROPERTIES LANGUAGE CXX)
    find_library(COREFOUNDATION Foundation REQUIRED)
    list(APPEND materialx_filedialog_libraries ${COREFOUNDATION})
endif ()

set(materialx_filedialog_sources ${materialx_filedialog_sources} PARENT_SCOPE)
set(materialx_filedialog_libraries ${materialx_filedialog_libraries} PARENT_SCOPE)
