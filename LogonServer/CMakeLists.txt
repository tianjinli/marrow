file(GLOB LogonServer_ROOT_Cpp *.cpp)
file(GLOB LogonServer_ROOT_Hpp *.h)

add_executable(LogonServer
        ${LogonServer_ROOT_Cpp}
        ${LogonServer_ROOT_Hpp})

set_target_properties(LogonServer PROPERTIES OUTPUT_NAME_DEBUG "LogonServer")
set_target_properties(LogonServer PROPERTIES PREFIX "")
set_target_properties(LogonServer PROPERTIES
        FOLDER "LogonServer"
        ARCHIVE_OUTPUT_DIRECTORY ${NFOutputDir}
        RUNTIME_OUTPUT_DIRECTORY ${NFOutputDir}
        LIBRARY_OUTPUT_DIRECTORY ${NFOutputDir})

target_link_libraries(LogonServer PRIVATE KernelEngine ServiceCore)
target_compile_definitions(LogonServer PRIVATE ELPP_NO_DEFAULT_LOG_FILE
        $<$<BOOL:${WIN32}>:WIN32_LEAN_AND_MEAN>
        $<$<BOOL:${WIN32}>:SERVICE_CORE_DLL>)
