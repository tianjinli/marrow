file(GLOB PersonalRoomService_ROOT_Cpp *.cpp *.c)
file(GLOB PersonalRoomService_ROOT_Hpp *.h)

add_library(PersonalRoomService SHARED
        ${PersonalRoomService_ROOT_Cpp}
        ${PersonalRoomService_ROOT_Hpp})

set_target_properties(PersonalRoomService PROPERTIES OUTPUT_NAME_DEBUG "PersonalRoomService")
set_target_properties(PersonalRoomService PROPERTIES PREFIX "")
set_target_properties(PersonalRoomService PROPERTIES FOLDER ${MSFolderName})

target_include_directories(PersonalRoomService PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(PersonalRoomService PRIVATE KernelEngine ServiceCore
        $<$<BOOL:${UNIX}>:z>)

target_compile_definitions(PersonalRoomService PRIVATE ELPP_NO_DEFAULT_LOG_FILE PERSONAL_ROOM_SERVICE_DLL)
