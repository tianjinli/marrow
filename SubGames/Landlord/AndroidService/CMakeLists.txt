file(GLOB AndroidService_ROOT_Cpp *.cpp *.c)
file(GLOB AndroidService_ROOT_Hpp *.h)

add_library(AndroidService SHARED
        ${AndroidService_ROOT_Cpp}
        ${AndroidService_ROOT_Hpp})

set_target_properties(AndroidService PROPERTIES OUTPUT_NAME_DEBUG "LandAndroidService")
set_target_properties(AndroidService PROPERTIES PREFIX "")
set_target_properties(AndroidService PROPERTIES FOLDER ${SGFolderName})

target_link_libraries(AndroidService PRIVATE KernelEngine ServiceCore GameService)

target_compile_definitions(AndroidService PRIVATE ELPP_NO_DEFAULT_LOG_FILE)
