file(GLOB KernelEngine_ROOT_Cpp *.cpp)
file(GLOB KernelEngine_ROOT_Hpp *.h)

add_library(KernelEngine SHARED
        ${KernelEngine_ROOT_Cpp}
        ${KernelEngine_ROOT_Hpp})

set_target_properties(KernelEngine PROPERTIES OUTPUT_NAME_DEBUG "KernelEngine")
set_target_properties(KernelEngine PROPERTIES PREFIX "")
set_target_properties(KernelEngine PROPERTIES
        FOLDER "KernelEngine"
        ARCHIVE_OUTPUT_DIRECTORY ${NFOutputDir}
        RUNTIME_OUTPUT_DIRECTORY ${NFOutputDir}
        LIBRARY_OUTPUT_DIRECTORY ${NFOutputDir})

target_link_libraries(KernelEngine PRIVATE ServiceCore $<$<BOOL:${UNIX}>:odbc>)
target_compile_definitions(KernelEngine PRIVATE ELPP_NO_DEFAULT_LOG_FILE
        $<$<BOOL:${WIN32}>:KERNEL_ENGINE_DLL>
        $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0601>
        $<$<BOOL:${WIN32}>:PUBLIC SPDLOG_WCHAR_FILENAMES>)

if (WIN32)
    target_link_libraries(KernelEngine PRIVATE odbc32 odbccp32)
endif()
