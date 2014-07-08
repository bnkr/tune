##############
## Prologue ##
##############
cmake_minimum_required(VERSION 2.6)
# For ease of use later
set(PROJECT_NAME tune)
set(PROJECT_VERSION "0.0.0")
project(${PROJECT_NAME})

include_directories("${CMAKE_SOURCE_DIR}/include")

####################################################
## Checking for required headers; making config.h ##
####################################################

find_file(HAVE_SDL_H "SDL.h")
mark_as_advanced(HAVE_SDL_H)

set(TUNE_VERSION "${PROJECT_VERSION}")

set(CONFIG_HPP_OUTPUT "${CMAKE_BINARY_DIR}/include/tune_config.hpp")
configure_file("${CMAKE_SOURCE_DIR}/build-aux/tune_config.hpp.cmake" "${CONFIG_HPP_OUTPUT}")
include_directories("${CMAKE_BINARY_DIR}/include")

#######################
## Finding libraries ##
#######################

find_package(SDL REQUIRED)
find_package(Boost 1.45.0 COMPONENTS thread program_options REQUIRED)

###########################
## Building normal stuff ##
###########################

set(BIN_TUNE "tune")
add_executable(
  ${BIN_TUNE}
  "src/main.cpp" "src/settings.cpp"
)
target_link_libraries(${BIN_TUNE} ${SDL_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_THREAD_LIBRARY})

# Do tests
# add_subdirectory("test")

##################
## Installation ##
##################

# no install yet

#################
## CPack Stuff ##
#################

# no binary or source packages
