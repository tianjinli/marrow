file(GLOB ServiceCore_ROOT_Cpp *.cpp *.c)
file(GLOB ServiceCore_ROOT_Hpp *.h)

add_library(ServiceCore SHARED
        ${ServiceCore_ROOT_Cpp}
        ${ServiceCore_ROOT_Hpp})

if (WIN32)
    add_custom_command(TARGET ServiceCore POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MSProjectDir}/Dependencies/bin/${CMAKE_BUILD_TYPE}/zlib$<$<CONFIG:Debug>:d>1.dll
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zlib$<$<CONFIG:Debug>:d>1.dll
    )
endif()

set_target_properties(ServiceCore PROPERTIES OUTPUT_NAME_DEBUG "ServiceCore")
set_target_properties(ServiceCore PROPERTIES PREFIX "")
set_target_properties(ServiceCore PROPERTIES FOLDER ${MSFolderName})

target_link_libraries(ServiceCore PRIVATE $<$<BOOL:${UNIX}>:z>)
target_compile_definitions(ServiceCore PRIVATE ELPP_NO_DEFAULT_LOG_FILE SERVICE_CORE_DLL)
