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

# TODO: wrap this in butil - also use a way to output the  exenames for butil_cpack.
# TODO: backport that to script lib.
#
# butil_auto_install(
#   [BINARIES_VAR varname]
#   [RUNNABLES_VAR varname]
#   [BINS binary pretty_name...]
#   [LIBS library...]
#   [HEADER_EXCLUDE pattern... | NO_HEADERS]
#   [WINDOWS_LIBS library...]
#   [AUX_DLLS]
# )
#
# This function uses some introspection to automatically generate install rules 
# for the project, as well as produce some output useful for cpack.
#
# TODO: 
# - can we determine if BINS/LIBS is a lib/bin automatically?
# - how do I get the pretty name of the binary
# - can I automatically find all teh used dlls?  This would be really handy
#   because if it doesn't happen right then the bins are not runnable.  Maybe
#   get_target_properties of each bins/libs?
# - remember to warn if dlls in build-aux are bad.
#
# Atomatically installs:
#
# - using the manpage lib, any manpage in doc/man.
# - include/ if it exists.
# - any .dll in build-aux if win32 is the target unless WINDOWS_LIBS is 
#   specified and AUX_DLLS is *not* specified.
# - README*, COPYING, etc.
# 
# BINARIES_VAR, RUNNABLES_VAR variables for use as their respective 
# butil_cpack_setup argument.
#
# BINS is required if BINARIES_VAR or RUNNABLES_VAR is given.  You do not need
# to specify the installpath of the binaries.
#
# LIBS is any library target.  Again, no need to specify the full path.
# 
# HEADER_EXCLUDE excludes a regexp from the install of include/.  NO_HEADERS
# disables install of headers entirely even if there exists an include/.
#
# WINDOWS_LIBS is a specific list of dlls to install on windows (also they
# get put in binary packages).  If this is specified then build-aux isn't 
# searched for dlls.
#
# AUX_DLLS - if present, then build-aux is searched for dlls regardless of
# whether WINDOWS_LIBS is specified.


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
  DESCRIPTION "Sinewave generator."
  VENDOR      "bunker"
  EMAIL       "bunker@bunkerprivate.com"
  URL         "http://www.bunkerprivate.com/"
  BINARIES    "bin/tune"
)
