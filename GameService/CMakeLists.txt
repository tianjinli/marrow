file(GLOB GameService_ROOT_Cpp *.cpp *.c)
file(GLOB GameService_ROOT_Hpp *.h)

add_library(GameService SHARED
        ${GameService_ROOT_Cpp}
        ${GameService_ROOT_Hpp})

set_target_properties(GameService PROPERTIES OUTPUT_NAME_DEBUG "GameService")
set_target_properties(GameService PROPERTIES PREFIX "")
set_target_properties(GameService PROPERTIES
        FOLDER "GameService"
        ARCHIVE_OUTPUT_DIRECTORY ${NFOutputDir}
        RUNTIME_OUTPUT_DIRECTORY ${NFOutputDir}
        LIBRARY_OUTPUT_DIRECTORY ${NFOutputDir})

target_link_libraries(GameService PRIVATE $<$<BOOL:${UNIX}>:z>)
target_compile_definitions(GameService PRIVATE ELPP_NO_DEFAULT_LOG_FILE
        $<$<BOOL:${WIN32}>:WIN32_LEAN_AND_MEAN>
        $<$<BOOL:${WIN32}>:GAME_SERVICE_DLL>)
