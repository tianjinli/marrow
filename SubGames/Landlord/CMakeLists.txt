cmake_minimum_required(VERSION 3.15)
project(LandlordRoot)

set(SGFolderName "SubGames/Landlord")

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(AndroidService)
add_subdirectory(GameComponent)
add_subdirectory(GameControl)
