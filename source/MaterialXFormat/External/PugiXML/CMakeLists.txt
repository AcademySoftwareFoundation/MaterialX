add_library(pugixml STATIC)
add_library(pugixml::pugixml ALIAS pugixml)

target_sources(pugixml
    PRIVATE
        pugixml.cpp
    PUBLIC
        pugixml.hpp
        pugiconfig.hpp
)

set_target_properties(pugixml PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(pugixml PROPERTIES CMAKE_VISIBILITY_INLINES_HIDDEN 1)
