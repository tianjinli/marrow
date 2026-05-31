file(GLOB Benchmark_ROOT_Cpp *.cpp)
file(GLOB Benchmark_ROOT_Hpp *.h *.hpp)

add_executable(Benchmark
        ${Benchmark_ROOT_Cpp}
        ${Benchmark_ROOT_Hpp}
        BaseSessionLogic.cpp
        BaseSessionLogic.h
        ../KernelEngine/EventDelegate.h)

set_target_properties(Benchmark PROPERTIES OUTPUT_NAME_DEBUG "Benchmark")
set_target_properties(Benchmark PROPERTIES PREFIX "")
set_target_properties(Benchmark PROPERTIES FOLDER ${MSFolderName})

target_link_libraries(Benchmark PRIVATE KernelEngine ServiceCore)
target_compile_definitions(Benchmark PRIVATE ELPP_NO_DEFAULT_LOG_FILE)
