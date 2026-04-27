file(GLOB ServiceCore_ROOT_Cpp *.cpp *.c)
file(GLOB ServiceCore_ROOT_Hpp *.h)

add_library(ServiceCore SHARED
        ${ServiceCore_ROOT_Cpp}
        ${ServiceCore_ROOT_Hpp})

set_target_properties(ServiceCore PROPERTIES OUTPUT_NAME_DEBUG "ServiceCore")
set_target_properties(ServiceCore PROPERTIES PREFIX "")
set_target_properties(ServiceCore PROPERTIES
        FOLDER "ServiceCore"
        ARCHIVE_OUTPUT_DIRECTORY ${NFOutputDir}
        RUNTIME_OUTPUT_DIRECTORY ${NFOutputDir}
        LIBRARY_OUTPUT_DIRECTORY ${NFOutputDir})

target_link_libraries(ServiceCore PRIVATE $<$<BOOL:${UNIX}>:z>)
target_compile_definitions(ServiceCore PRIVATE ELPP_NO_DEFAULT_LOG_FILE
        $<$<BOOL:${WIN32}>:WIN32_LEAN_AND_MEAN>
        $<$<BOOL:${WIN32}>:SERVICE_CORE_DLL>)
