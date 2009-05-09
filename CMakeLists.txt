##############
## Prologue ##
##############
cmake_minimum_required(VERSION 2.6)
# For ease of use later
set(PROJECT_NAME tune)
set(PROJECT_VERSION "0.0.0")
project(${PROJECT_NAME})

include("build-aux/doxygen.cmake")
include("build-aux/manpage.cmake")
include("build-aux/btest.cmake")
include("build-aux/butil.cmake")

butil_standard_setup()

####################################################
## Checking for required headers; making config.h ##
####################################################

# include(CheckFunctionExists)

find_file(HAVE_SDL_H "SDL.h")
mark_as_advanced(HAVE_SDL_H)

set(TUNE_VERSION "${PROJECT_VERSION}")

set(CONFIG_HPP_OUTPUT "${CMAKE_BINARY_DIR}/include/tune_config.hpp")
configure_file("${CMAKE_SOURCE_DIR}/build-aux/tune_config.hpp.cmake" "${CONFIG_HPP_OUTPUT}")

#######################
## Finding libraries ##
#######################

find_package(SDL REQUIRED)

butil_find_lib(
  VAR BOOST_PROGOPT_LIB
  NAMES "boost_program_options-mt" "boost_program_options"
)
butil_find_lib(
  VAR BOOST_THREAD_LIB
  NAMES "boost_thread-mt" "boost_thread"
)

###########################
## Building normal stuff ##
###########################

set(BIN_TUNE "tune")
butil_add_executable(
  TARGET ${BIN_TUNE}
  SOURCES "src/main.cpp" "src/settings.cpp"
  LIBS ${SDL_LIBRARY} "${BOOST_PROGOPT_LIB}" "${BOOST_THREAD_LIB}"
)

# Do tests
add_subdirectory("test")

############################
## Building Documentation ##
############################

add_doxygen_directives(
  TARGET doxygen
  INPUTS "src/"
  ARGS_VAR ad_args
  DOCS_MIRROR "${CMAKE_SOURCE_DIR}/doc"
  DEFAULT_DOXYFILE "${CMAKE_SOURCE_DIR}/Doxyfile.default"
  INSTALL ${WANT_DOCS}
)
# TODO:
#   depends are wrong somewhere here - it always rebuilds the cache.
#   prolly depending on doxyfile-forced is wrong?
add_doxygen(${ad_args})

##################
## Installation ##
##################

butil_auto_install(
  RUNNABLES_VAR runnables
  RUNNABLES "${BIN_TUNE}" "tune"
  BINARIES_VAR installed_binaries
  TARGETS ${BIN_TUNE}
  # INCLUDE_DIRS dir...
  NO_HEADERS # [HEADER_EXCLUDE pattern... | NO_HEADERS]
  # WINDOWS_LIBS library...]
  HARDLINK_AUX_DLLS
  BUILD_AUX_DLLS
)

#################
## CPack Stuff ##
#################

butil_cpack_setup(
  DESCRIPTION "Sinewave generator."
  VENDOR      "bunker"
  EMAIL       "bunker@bunkerprivate.com"
  URL         "http://www.bunkerprivate.com/"
  BINARIES    ${installed_binaries}
  RUNNABLES   ${runnables}
)

