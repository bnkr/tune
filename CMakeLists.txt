##############
## Prologue ##
##############
cmake_minimum_required(VERSION 2.6)
# For ease of use later
set(PROJECT_NAME tune)
set(PROJECT_VERSION "0.0")
project(${PROJECT_NAME})

include("build-aux/doxygen.cmake")
include("build-aux/manpage.cmake")
include("build-aux/btest.cmake")

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

# find_library(SOME_LIB "some")
# mark_as_advanced(SOME_LIB)
find_package(SDL REQUIRED)
find_library(BOOST_PROGOPT_LIB "boost_program_options-mt")
if (NOT BOOST_PROGOPT_LIB) 
  find_library(BOOST_PROGOPT_LIB "boost_program_options")
endif()
find_library(BOOST_THREAD_LIB "boost_thread-mt")
if (NOT BOOST_THREAD_LIB) 
  find_library(BOOST_THREAD_LIB "boost_thread")
endif()
mark_as_advanced(BOOST_THREAD_LIB)

###########################
## Building normal stuff ##
###########################

# Example follows:
# add_library(${LIB_BDBG_TRACE_A} STATIC "src/lib_bdbg_trace.cpp")
# set_target_properties(${LIB_BDBG_TRACE_A} PROPERTIES OUTPUT_NAME "bdbg_trace")
# add_library(${LIB_BDBG_TRACE_S} SHARED "src/lib_bdbg_trace.cpp")
# set_target_properties(${LIB_BDBG_TRACE_S} PROPERTIES OUTPUT_NAME "bdbg_trace")
# set_target_properties(${LIB_BDBG_TRACE_S} PROPERTIES SOVERSION "${PROJECT_MAJOR}")
# set_target_properties(${LIB_BDBG_TRACE_S} PROPERTIES VERSION "${PROJECT_VERSION}")
# target_link_libraries(${LIB_BDBG_TRACE_S} "-lboost_thread")

set(BIN_TUNE "tune")
set(BIN_TUNE_SRCS
  "src/main.cpp"
  "src/settings.cpp"
  # "src/notes.cpp"
)
add_executable(tune ${BIN_TUNE_SRCS})
target_link_libraries(${BIN_TUNE} ${SDL_LIBRARY} "${BOOST_PROGOPT_LIB}" "${BOOST_THREAD_LIB}")

# Do tests
add_subdirectory("test")

############################
## Building Documentation ##
############################

add_doxygen_directives(
  TARGET doxygen
  INPUTS "include/"
  ARGS_VAR ad_args
  DOCS_MIRROR ${CMAKE_SOURCE_DIR}/doc
  DEFAULT_DOXYFILE ${CMAKE_SOURCE_DIR}/Doxyfile.default
  INSTALL ${WANT_DOCS}
)
add_doxygen(${ad_args})

##################
## Installation ##
##################

message(STATUS "Adding install targets.")

set(INCLUDEDIR "include")
set(BINDIR "bin")
set(DATADIR "share")
set(LIBDIR "lib")
set(MANDIR "${DATADIR}/man")
set(DOCDIR "${DATADIR}/doc/${PROJECT_NAME}")
set(HTMLDIR "${DATADIR}/doc/${PROJECT_NAME}")
set(PDFDIR "${DATADIR}/doc/${PROJECT_NAME}")
set(DVIDIR "${DATADIR}/doc/${PROJECT_NAME}")
set(PSDIR "${DATADIR}/doc/${PROJECT_NAME}")


if (WANT_DOCS) 
  # Documentation
  install(
    FILES README COPYING 
    DESTINATION ${DOCDIR}
  )

  if (WANT_DOCS_MAN)
    add_and_install_manpages("${MANDIR}" "${CMAKE_SOURCE_DIR}/doc/man/tune.1")
  endif()
endif(WANT_DOCS)

# Header files
# install(
#   DIRECTORY "include/"
#   DESTINATION "${INCLUDEDIR}/"
#   FILES_MATCHING
#   PATTERN "detail/config.hpp" EXCLUDE
#   PATTERN "*.hpp"
# )

# install(
#   FILES ${CONFIG_HPP_OUTPUT}
#   DESTINATION "${INCLUDEDIR}/para/detail/"
# )

# Binaries
install(
  TARGETS ${BIN_TUNE}
  RUNTIME DESTINATION ${BINDIR}
  ARCHIVE DESTINATION ${LIBDIR}
  LIBRARY DESTINATION ${LIBDIR}
)


#################
## CPack Stuff ##
#################

butil_cpack_setup(
  DESCRIPTION "Stuff."
  VENDOR      "bunker"
  EMAIL       "bunker@bunkerprivate.com"
  URL         "http://www.bunkerprivate.com/"
  BINARIES    "bin/tune"
)
