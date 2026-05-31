file(GLOB PersonalRoomCorrespond_ROOT_Cpp *.cpp)
file(GLOB PersonalRoomCorrespond_ROOT_Hpp *.h)

add_executable(PersonalRoomCorrespond
        ${PersonalRoomCorrespond_ROOT_Cpp}
        ${PersonalRoomCorrespond_ROOT_Hpp})

set_target_properties(PersonalRoomCorrespond PROPERTIES OUTPUT_NAME_DEBUG "PersonalRoomCorrespond")
set_target_properties(PersonalRoomCorrespond PROPERTIES PREFIX "")
set_target_properties(PersonalRoomCorrespond PROPERTIES FOLDER ${MSFolderName})

target_link_libraries(PersonalRoomCorrespond PRIVATE KernelEngine ServiceCore GameService MatchService ModuleManager)
target_compile_definitions(PersonalRoomCorrespond PRIVATE ELPP_NO_DEFAULT_LOG_FILE)
