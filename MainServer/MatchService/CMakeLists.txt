file(GLOB MatchService_ROOT_Cpp *.cpp *.c)
file(GLOB MatchService_ROOT_Hpp *.h)

add_library(MatchService SHARED
        ${MatchService_ROOT_Cpp}
        ${MatchService_ROOT_Hpp})

set_target_properties(MatchService PROPERTIES OUTPUT_NAME_DEBUG "MatchService")
set_target_properties(MatchService PROPERTIES PREFIX "")
set_target_properties(MatchService PROPERTIES FOLDER ${MSFolderName})

target_link_libraries(MatchService PRIVATE KernelEngine ServiceCore GameService)

target_compile_definitions(MatchService PRIVATE ELPP_NO_DEFAULT_LOG_FILE MATCH_SERVICE_DLL)
