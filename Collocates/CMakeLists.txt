file(GLOB Collocates_ROOT_Cpp *.cpp)
file(GLOB Collocates_ROOT_Hpp *.h)

add_executable(Collocates
        ${Collocates_ROOT_Cpp}
        ${Collocates_ROOT_Hpp})

set_target_properties(Collocates PROPERTIES OUTPUT_NAME_DEBUG "Collocates")
set_target_properties(Collocates PROPERTIES PREFIX "")
set_target_properties(Collocates PROPERTIES
        FOLDER "Collocates"
        ARCHIVE_OUTPUT_DIRECTORY ${NFOutputDir}
        RUNTIME_OUTPUT_DIRECTORY ${NFOutputDir}
        LIBRARY_OUTPUT_DIRECTORY ${NFOutputDir})

target_link_libraries(Collocates PRIVATE ServiceCore)
target_compile_definitions(Collocates PRIVATE ELPP_NO_DEFAULT_LOG_FILE
        $<$<BOOL:${WIN32}>:WIN32_LEAN_AND_MEAN>)
