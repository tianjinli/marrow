file(GLOB Collocates_ROOT_Cpp *.cpp)
file(GLOB Collocates_ROOT_Hpp *.h)

add_executable(Collocates
        ${Collocates_ROOT_Cpp}
        ${Collocates_ROOT_Hpp})

set(SRC_INI ${MSProjectDir}/Collocates/ServerParameter.ini)
set(DST_INI ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ServerParameter.ini)

if(NOT EXISTS "${DST_INI}")
    message(STATUS "Copying ${SRC_INI} -> ${DST_INI}")
    file(COPY "${SRC_INI}" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
    message(STATUS "Skip copy: ${DST_INI} already exists")
endif()


set_target_properties(Collocates PROPERTIES OUTPUT_NAME_DEBUG "Collocates")
set_target_properties(Collocates PROPERTIES PREFIX "")
set_target_properties(Collocates PROPERTIES FOLDER ${MSFolderName})

target_link_libraries(Collocates PRIVATE ServiceCore)
target_compile_definitions(Collocates PRIVATE ELPP_NO_DEFAULT_LOG_FILE)
