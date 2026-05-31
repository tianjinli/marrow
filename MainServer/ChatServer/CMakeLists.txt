file(GLOB ChatServer_ROOT_Cpp *.cpp)
file(GLOB ChatServer_ROOT_Hpp *.h)

add_executable(ChatServer
        ${ChatServer_ROOT_Cpp}
        ${ChatServer_ROOT_Hpp})

set_target_properties(ChatServer PROPERTIES OUTPUT_NAME_DEBUG "ChatServer")
set_target_properties(ChatServer PROPERTIES PREFIX "")
set_target_properties(ChatServer PROPERTIES FOLDER ${MSFolderName})

target_link_libraries(ChatServer PRIVATE KernelEngine ServiceCore)
target_compile_definitions(ChatServer PRIVATE ELPP_NO_DEFAULT_LOG_FILE)
