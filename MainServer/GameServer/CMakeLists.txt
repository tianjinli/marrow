file(GLOB GameServer_ROOT_Cpp *.cpp)
file(GLOB GameServer_ROOT_Hpp *.h)

add_executable(GameServer
        ${GameServer_ROOT_Cpp}
        ${GameServer_ROOT_Hpp})

set_target_properties(GameServer PROPERTIES OUTPUT_NAME_DEBUG "GameServer")
set_target_properties(GameServer PROPERTIES PREFIX "")
set_target_properties(GameServer PROPERTIES FOLDER ${MSFolderName})

target_link_libraries(GameServer PRIVATE KernelEngine ServiceCore GameService MatchService ModuleManager)
target_compile_definitions(GameServer PRIVATE ELPP_NO_DEFAULT_LOG_FILE)
