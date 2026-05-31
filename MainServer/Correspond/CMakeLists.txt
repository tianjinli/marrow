file(GLOB Correspond_ROOT_Cpp *.cpp)
file(GLOB Correspond_ROOT_Hpp *.h)

add_executable(Correspond
        ${Correspond_ROOT_Cpp}
        ${Correspond_ROOT_Hpp})

set_target_properties(Correspond PROPERTIES OUTPUT_NAME_DEBUG "Correspond")
set_target_properties(Correspond PROPERTIES PREFIX "")
set_target_properties(Correspond PROPERTIES FOLDER ${MSFolderName})

target_link_libraries(Correspond PRIVATE KernelEngine ServiceCore)
target_compile_definitions(Correspond PRIVATE ELPP_NO_DEFAULT_LOG_FILE)
