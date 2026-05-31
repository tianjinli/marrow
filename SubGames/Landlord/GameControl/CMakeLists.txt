file(GLOB GameControl_ROOT_Cpp *.cpp *.c)
file(GLOB GameControl_ROOT_Hpp *.h)

add_library(GameControl SHARED
        ${GameControl_ROOT_Cpp}
        ${GameControl_ROOT_Hpp})

set_target_properties(GameControl PROPERTIES OUTPUT_NAME_DEBUG "LandGameControl")
set_target_properties(GameControl PROPERTIES PREFIX "")
set_target_properties(GameControl PROPERTIES FOLDER ${SGFolderName})

target_link_libraries(GameControl PRIVATE KernelEngine ServiceCore GameService)

target_compile_definitions(GameControl PRIVATE ELPP_NO_DEFAULT_LOG_FILE)
