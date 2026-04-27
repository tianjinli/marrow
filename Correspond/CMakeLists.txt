file(GLOB Correspond_ROOT_Cpp *.cpp)
file(GLOB Correspond_ROOT_Hpp *.h)

add_executable(Correspond
        ${Correspond_ROOT_Cpp}
        ${Correspond_ROOT_Hpp})

set_target_properties(Correspond PROPERTIES OUTPUT_NAME_DEBUG "Correspond")
set_target_properties(Correspond PROPERTIES PREFIX "")
set_target_properties(Correspond PROPERTIES
        FOLDER "Correspond"
        ARCHIVE_OUTPUT_DIRECTORY ${NFOutputDir}
        RUNTIME_OUTPUT_DIRECTORY ${NFOutputDir}
        LIBRARY_OUTPUT_DIRECTORY ${NFOutputDir})

target_link_libraries(Correspond PRIVATE KernelEngine ServiceCore)
target_compile_definitions(Correspond PRIVATE ELPP_NO_DEFAULT_LOG_FILE
        $<$<NOT:$<BOOL:${UNIX}>>:WIN32_LEAN_AND_MEAN>)
