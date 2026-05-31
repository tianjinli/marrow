file(GLOB ModuleManager_ROOT_Cpp *.cpp *.c)
file(GLOB ModuleManager_ROOT_Hpp *.h)

add_library(ModuleManager SHARED
        ${ModuleManager_ROOT_Cpp}
        ${ModuleManager_ROOT_Hpp})

set_target_properties(ModuleManager PROPERTIES OUTPUT_NAME_DEBUG "ModuleManager")
set_target_properties(ModuleManager PROPERTIES PREFIX "")
set_target_properties(ModuleManager PROPERTIES FOLDER ${MSFolderName})

target_link_libraries(ModuleManager PRIVATE KernelEngine ServiceCore GameService)

target_compile_definitions(ModuleManager PRIVATE ELPP_NO_DEFAULT_LOG_FILE MODULE_MANAGER_DLL)
