file(GLOB GameService_ROOT_Cpp *.cpp *.c)
file(GLOB GameService_ROOT_Hpp *.h)

add_library(GameService SHARED
        ${GameService_ROOT_Cpp}
        ${GameService_ROOT_Hpp})

set_target_properties(GameService PROPERTIES OUTPUT_NAME_DEBUG "GameService")
set_target_properties(GameService PROPERTIES PREFIX "")
set_target_properties(GameService PROPERTIES FOLDER ${MSFolderName})

target_include_directories(GameService PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GameService PRIVATE KernelEngine ServiceCore
        $<$<BOOL:${UNIX}>:z>)

target_compile_definitions(GameService PRIVATE ELPP_NO_DEFAULT_LOG_FILE GAME_SERVICE_DLL)
