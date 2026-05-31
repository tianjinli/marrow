cmake_minimum_required(VERSION 3.15)
project(SubGamesRoot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (APPLE)
    set(SGSystemDir "osx")
    #OSX上强制生成so后缀的动态库而不是dylib后缀动态库
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
elseif (UNIX)
    set(SGSystemDir "linux")
else ()
    set(SGSystemDir "windows")
endif ()

if (UNIX)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++20 -pthread -O0 -Wall -g -ggdb -DDEBUG")
    else ()
        set(CMAKE_BUILD_TYPE "Release")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++20 -pthread -O3 -Wall")
    endif ()

    set(LDFLAGS "$ENV{LDFLAGS} -stdlib=libc++")
else ()
    if (MSVC)
        add_compile_options(/utf-8 /EHsc)
        if (MSVC_VERSION LESS 1940)
            message(STATUS "MSVC < 19.40 detected (VS2022). Enabling /await.")
            add_compile_options(/await)
            add_compile_definitions(_SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS)
        endif ()
        add_compile_definitions(_X64 _WINDOWS NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
    else ()
        add_compile_options("-fexec-charset=UTF-8" "-finput-charset=UTF-8")
        add_compile_definitions(_WINDOWS NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG")
        else ()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
        endif ()
    endif ()
endif ()

set(SGOutBinDir "${CMAKE_CURRENT_SOURCE_DIR}/../_Out/bin/${CMAKE_BUILD_TYPE}/RYServer")
set(SGOutLibDir "${CMAKE_CURRENT_SOURCE_DIR}/../_Out/lib/${CMAKE_BUILD_TYPE}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SGOutBinDir})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SGOutBinDir})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SGOutLibDir})

set(SGLibraryDir "${SGOutLibDir}")
set(SGIncludeDir "${CMAKE_CURRENT_SOURCE_DIR}/../MainServer")

message(STATUS "SGIncludeDir:${SGIncludeDir}")
message(STATUS "SGLibraryDir:${SGLibraryDir}")

add_compile_definitions(FMT_HEADER_ONLY=1)

# 以下两个尚未找到合适的替换
include_directories(${SGIncludeDir})
link_directories(${SGLibraryDir})

# 引入 SubGames 自己的模块
add_subdirectory(Landlord)
