file(GLOB GameComponent_ROOT_Cpp *.cpp *.c)
file(GLOB GameComponent_ROOT_Hpp *.h)

add_library(GameComponent SHARED
        ${GameComponent_ROOT_Cpp}
        ${GameComponent_ROOT_Hpp})

set_target_properties(GameComponent PROPERTIES OUTPUT_NAME_DEBUG "LandGameComponent")
set_target_properties(GameComponent PROPERTIES PREFIX "")
set_target_properties(GameComponent PROPERTIES FOLDER ${SGFolderName})

target_link_libraries(GameComponent PRIVATE KernelEngine ServiceCore GameService)

target_compile_definitions(GameComponent PRIVATE ELPP_NO_DEFAULT_LOG_FILE)
