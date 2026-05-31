file(GLOB LogonServer_ROOT_Cpp *.cpp)
file(GLOB LogonServer_ROOT_Hpp *.h)

add_executable(LogonServer
        ${LogonServer_ROOT_Cpp}
        ${LogonServer_ROOT_Hpp})

set_target_properties(LogonServer PROPERTIES OUTPUT_NAME_DEBUG "LogonServer")
set_target_properties(LogonServer PROPERTIES PREFIX "")
set_target_properties(LogonServer PROPERTIES FOLDER ${MSFolderName})

target_link_libraries(LogonServer PRIVATE KernelEngine ServiceCore)
target_compile_definitions(LogonServer PRIVATE ELPP_NO_DEFAULT_LOG_FILE)
