include(GoogleTest)

add_executable(MaterialXFuzz XmlIo_fuzz.cpp)

target_link_libraries(MaterialXFuzz PRIVATE
    MaterialXFormat
    MaterialXCore)

target_include_directories(MaterialXFuzz PRIVATE
    ${CMAKE_SOURCE_DIR}/source)

# link_fuzztest selects the fuzzing or unit-test backend based on FUZZTEST_FUZZING_MODE.
link_fuzztest(MaterialXFuzz)

# FuzzTest's CMakeLists.txt calls add_compile_options(-Werror) globally, which propagates here.
# FuzzTest's own headers then trigger -Wshadow/-Wreturn-type during template instantiation.
# Override with -Wno-error so warnings from third-party headers don't fail the build.
if(NOT MSVC)
    target_compile_options(MaterialXFuzz PRIVATE
        -Wno-error          # FuzzTest's CMakeLists propagates -Werror globally
        -Wno-return-type    # meta.h:167 falls off end without return; GCC 14 hard-errors this
        -Wno-shadow         # meta.h:67 lambda captures shadow outer parameter
        -Wno-sign-compare)  # value_mutation_helpers.h:62 int vs size_t comparison

    # FuzzTest compiles abseil with -fsanitize=address by default in fuzzing mode.
    # Link the ASan runtime into the final executable to resolve those symbols.
    if(FUZZTEST_FUZZING_MODE)
        target_link_options(MaterialXFuzz PRIVATE -fsanitize=address)
    endif()
endif()

# Always copy resources to build/bin/resources/
# add_custom_target(ALL) ensures this runs on every build, not just when the
# binary is relinked.
add_custom_target(MaterialXFuzzResources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    ${CMAKE_SOURCE_DIR}/resources ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources
    COMMENT "Copying resources to build/bin/resources/ for fuzz tests")
add_dependencies(MaterialXFuzz MaterialXFuzzResources)

gtest_discover_tests(MaterialXFuzz
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    DISCOVERY_MODE PRE_TEST)
