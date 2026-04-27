file(GLOB Benchmark_ROOT_Cpp *.cpp)
file(GLOB Benchmark_ROOT_Hpp *.h *.hpp)

add_executable(Benchmark
        ${Benchmark_ROOT_Cpp}
        ${Benchmark_ROOT_Hpp}
        BaseSessionLogic.cpp
        BaseSessionLogic.h)

set_target_properties(Benchmark PROPERTIES OUTPUT_NAME_DEBUG "Benchmark")
set_target_properties(Benchmark PROPERTIES PREFIX "")
set_target_properties(Benchmark PROPERTIES
        FOLDER "Benchmark"
        ARCHIVE_OUTPUT_DIRECTORY ${NFOutputDir}
        RUNTIME_OUTPUT_DIRECTORY ${NFOutputDir}
        LIBRARY_OUTPUT_DIRECTORY ${NFOutputDir})

target_link_libraries(Benchmark PRIVATE KernelEngine ServiceCore)
target_compile_definitions(Benchmark PRIVATE ELPP_NO_DEFAULT_LOG_FILE
        $<$<BOOL:${WIN32}>:WIN32_LEAN_AND_MEAN>)
