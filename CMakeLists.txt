##############
## Prologue ##
##############
cmake_minimum_required(VERSION 2.6)
# For ease of use later
set(PROJECT_NAME tune)
set(PROJECT_VERSION "0.0")
project(${PROJECT_NAME})

string(REGEX REPLACE "^([0-9]+).*" "\\1" PROJECT_MAJOR ${PROJECT_VERSION})
string(REGEX REPLACE "^[^.]+\\.([0-9]+).*" "\\1" PROJECT_MINOR ${PROJECT_VERSION})
string(REGEX REPLACE "^[^.]+\\.[^.]+\\.([0-9]+).*" "\\1" PROJECT_PATCH ${PROJECT_VERSION})
string(REGEX REPLACE "^[^.]+\\.[^.]+\\.[^\\-]-([0-9]+).*" "\\1" PROJECT_SNAPSHOT ${PROJECT_VERSION})

include("build-aux/doxygen.cmake")
include("build-aux/manpage.cmake")
include("build-aux/btest.cmake")

# General properties of the build
include_directories("${CMAKE_SOURCE_DIR}/include/" "${CMAKE_BINARY_DIR}/include/")

enable_testing()

# For different types of CMAKE_BUILD_TYPE, add -Wall.
if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall" STRING CACHE FORCE "CXX compiler parmeters when BUILD_TYPE is Debug")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall" STRING CACHE FORCE "CXX compiler parmeters when BUILD_TYPE is Debug")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -Wall" STRING CACHE FORCE "CXX compiler parmeters when BUILD_TYPE is Debug")
endif()

if (CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall" STRING CACHE FORCE "C compiler parmeters when BUILD_TYPE is Debug")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall" STRING CACHE FORCE "C compiler parmeters when BUILD_TYPE is Release")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -Wall" STRING CACHE FORCE "C compiler parmeters when BUILD_TYPE is RelWithDebInfo")
endif()

# Global consts for libraries etc.

# General options
option(WANT_DOCS "Install documentation - when off, all other docs options are off." YES)
option(WANT_DOXYGEN_HTML "Install HTML made by doxygen" YES)
option(WANT_DOXYGEN_PDF "Install PDF from doxygen (needs pdfLaTeX)" YES)
option(WANT_DOCS_MAN "Install manpages if there are any." YES)
option(WANT_DOXYGEN_REBUILD "Regenerate doxygen and install that instead of the distributed docs." NO)

set(DOXYGEN_DOXYFILE_TEMPLATE "${CMAKE_SOURCE_DIR}/Doxyfile.default" CACHE STRING 
    "Base doxyfile.  Mostly useful for developers.  Certain vars will be overwritten by cmake (in/output paths mostly).")

mark_as_advanced(
  DOXYGEN_DOXYFILE_TEMPLATE 
  WANT_DOXYGEN_REBUILD
  WANT_DOXYGEN_HTML 
  WANT_DOXYGEN_PDF
  WANT_DOCS_MAN
)

####################################################
## Checking for required headers; making config.h ##
####################################################

# include(CheckFunctionExists) 

find_file(HAVE_SDL_H "SDL.h")
mark_as_advanced(HAVE_SDL_H)

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

if (WANT_DOXYGEN_PDF)
  list(APPEND DOXYGEN_WANTS "pdf")
endif()
if (WANT_DOXYGEN_HTML)
  list(APPEND DOXYGEN_WANTS "html")
endif()

set(DOXYGEN_CMAKE_VERBOSE "yes")

set(DOXYGEN_TARGET "doxygen")
set(DOXYGEN_INPUT_PATHS "include/")
doxgyen_setup_flags(
  DOXYGEN_FLAGS 
  "${DOXYGEN_TARGET}"
  "${DOXYGEN_WANTS}"
)

add_doxygen(
  ${DOXYGEN_TARGET}
  ${DOXYGEN_DOXYFILE_TEMPLATE}
  "${DOXYGEN_FLAGS}"
)

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

  if (WANT_DOXYGEN_REBUILD)
    set(INSTALL_FROM)
  else()
    set(INSTALL_FROM "${CMAKE_SOURCE_DIR}/doc/")
  endif()

  # Note: this requires that the variables set by doxygen_setup_flags are still valid!
  doxygen_install_targets(
    "${DOXYGEN_TARGET}"
    "${DOXYGEN_WANTS}" 
    "${DOCDIR}" 
    "${INSTALL_FROM}"
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

message(STATUS "Doing CPack stuff.")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Stuff.")
set(CPACK_PACKAGE_VENDOR "bunker")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/build-aux/install-readme.txt")

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_PATCH})

set(CPACK_SOURCE_TGZ "ON")
set(CPACK_SOURCE_TBZ2 "OFF")
set(CPACK_SOURCE_TZ "OFF")

set(CPACK_BINARY_STGZ "OFF")
set(CPACK_BINARY_TBZ2 "OFF")
set(CPACK_BINARY_TGZ "OFF")
set(CPACK_BINARY_TZ "OFF")

# Yes, you really need all that escape crap.
set(CPACK_SOURCE_IGNORE_FILES "/\\\\..*/;~$;.*\\\\.swp$;/${CMAKE_BINARY_DIR}/;Makefile")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

# name, title, ...
# set(CPACK_PACKAGE_EXECUTABLES "MyExecutable" "My Executable")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}-${PROJECT_VERSION}")
if(WIN32)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  # TODO: all of this, but it requires cross-compile anyway
  # set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/build-aux\\\\InstallIcon.bmp")
  # set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.bunkerprivate.com/")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.bunkerprivate.com/")
  set(CPACK_NSIS_CONTACT "bunker@bunkerprivate.com")
  set(CPACK_NSIS_MODIFY_PATH ON)
else()
  # set(CPACK_STRIP_FILES "bin/MyExecutable")
  
  set(CPACK_SOURCE_STRIP_FILES "")
endif()


include(CPack)

