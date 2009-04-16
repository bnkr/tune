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

enable_testing()

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
add_executable(${BIN_TUNE} ${BIN_TUNE_SRCS})
target_link_libraries(${BIN_TUNE} ${SDL_LIBRARY} "${BOOST_PROGOPT_LIB}" "${BOOST_THREAD_LIB}")

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
add_doxygen(${ad_args})

##################
## Installation ##
##################

# butil_auto_install(
#   [BINARIES_VAR varname]
#   [RUNNABLES_VAR varname]
#   [BINS binary...]
#   [LIBS library...]
#   [INCLUDE_DIRS dir...]
#   [HEADER_EXCLUDE pattern... | NO_HEADERS]
#   [WINDOWS_LIBS library...]
#   [AUX_DLLS]
# )
#
# This function uses some introspection to automatically generate install rules 
# for the project, as well as produce some output useful for butil_cpack.
#
# Atomatically installs:
#
# - using the manpage lib, any manpage in doc/man.
# - include/ if it exists.
# - any .dll in build-aux if win32 is the target unless WINDOWS_LIBS is 
#   specified and AUX_DLLS is *not* specified.
# - README*, COPYING, etc.
#
# It responds to the following cache options:
# - WANT_DOCS
# - WANT_DOCS_MAN
# 
# BINARIES_VAR - list of all files for cpack to strip.  This can be passed 
# directly to butil_setup_cpack()
#
# RUNNABLES_VAR - list of path, name pairs to use as cpack executables.  This
# is guessed from BINS, and can be passed directly to butil_setup_cpack().
#
# BINS is required if BINARIES_VAR or RUNNABLES_VAR is given.  You do not need
# to specify the installpath of the binaries - just the *target*.
#
# LIBS is any library target.  Again, no need to specify the full path.
# 
# INCLUDE_DIRS directories to install into the include path.   Make sure they
# it DOES have the trailing foreward slash.
#
# HEADER_EXCLUDE excludes a regexp from the install of include/.  NO_HEADERS
# disables install of headers entirely even if there exists an include/.
# You must manually install any extra headers, for exampe those made with 
# configure_file.
#
# WINDOWS_LIBS is a specific list of dlls to install on windows (also they
# get put in binary packages).  If this is specified then build-aux isn't 
# searched for dlls.
#
# AUX_DLLS - if present, then build-aux is searched for dlls regardless of
# whether WINDOWS_LIBS is specified.
macro(butil_auto_install)
  message(STATUS "Generating install targets.")

  butil_parse_args(
    "BINARIES_VAR;RUNNABLES_VAR;BINS;LIBS;HEADER_EXCLUDE;WINDOWS_LIBS;INCLUDE_DIRS"
    "NO_HEADERS;AUX_DLLS"
    ""
    "${ARGV}"
  )

  if (arg_BINARIES_VAR OR arg_RUNNABLES_VAR)
    if (NOT arg_BINS)
      message(FATAL_ERROR "butil_auto_install(): BINS is required if BINARIES_VAR or RUNNABLES_VAR is given.")
    endif()
  endif()

  # TODO: 
  # - can we determine if BINS/LIBS is a lib/bin automatically?  Or possibly 
  #   have TARGETS and find out which are bins and which are libs?

  # TODO:
  # - can I automatically find all teh used dlls based on link_libs of each 
  #   paths.
  #   - what if I don't want to install some of those bins?  Unlikely...
  #   - 
  # useful properties on targets:
  # - OUTPUT_NAME
  # - LINK_INTERFACE_LIBRARIES
  # - LOCATION_<CONFIG>


  # TODO:
  #   This stuff should be specified on the cmdline.  It needs to be declared as an 
  #   option in advance of this function so it can be used in configure_file (eg, 
  #   confdir).  The best solution is to use standard_setup to declare them all
  #   and then just reference them here, or use defaults if they are not defined.
  #   We also need a flag to standard setup which tells it not to create install
  #   cache vars.
  #
  #   We'll have to assert that they are defined.
  #
  #   We need to add INSTALL_x.  Then I can update the install tutorial.  Put it 
  #   in section 3, configuration.
  set(INCLUDEDIR "include")
  set(BINDIR "bin")
  set(DATADIR "share")
  set(ARCHIVEDIR "lib")

  if (UNIX)
    set(LIBDIR "lib")
    set(DOCROOT "${DATADIR}/doc/${CMAKE_PROJECT_NAME}")
    set(CONFDIR "etc/${CMAKE_PROJECT_NAME}")
  else()
    set(LIBDIR "${BINDIR}")
    set(DOCROOT "doc")
    set(CONFDIR "etc/")
  endif()

  set(MANDIR "${DATADIR}/man")

  set(HTMLDIR "${DOCROOT}")
  set(PDFDIR "${DOCROOT}")
  set(DVIDIR "${DOCROOT}")
  set(PSDIR "${DOCROOT}")

  if (WANT_DOCS)
    file(GLOB basic_docs "${CMAKE_SOURCE_DIR}/README*" "${CMAKE_SOURCE_DIR}/COPYING*")

    if (basic_docs)
      install(
        FILES ${basic_docs}
        DESTINATION ${DOCROOT}
      )
    endif()

    if (WANT_DOCS_MAN)
      set(src_man "${CMAKE_SOURCE_DIR}/doc/man")
      if (IS_DIRECTORY "${src_man}")
        file(GLOB manpages "${src_man}/*.*")

        # TODO: this should be (INSTALL_ROOT ${mandir} PAGES page...).  (api is not finished yet)
        if (UNIX)
          add_and_install_manpages("${MANDIR}" ${manpages})
        endif()
      endif()
    endif()
  endif()

  if (arg_NO_HEADERS AND arg_INCLUDE_DIRS)
    message("butil_auto_install(): warning: NO_HEADERS and INCLUDE_DIRS are both specified.  NO_HEADERS takes precedence.")
  endif()

  # Install headers from INCLUDE_DIRS if specified, otherwise src/include if exists.
  if (NOT arg_NO_HEADERS)
    set(src_include "${CMAKE_SOURCE_DIR}/include/")

    if (NOT arg_INCLUDE_DIRS AND IS_DIRECTORY "${src_include}")
      set(arg_INCLUDE_DIRS "${src_include}")
    endif()

    set(exclude_patterns)
    if (arg_HEADER_EXCLUDE)
      foreach (pat ${arg_HEADER_EXCLUDE})
        list(APPEND exclude_patterns "PATTERN")
        list(APPEND exclude_patterns "\"${pat}\"")
        list(APPEND exclude_patterns "EXCLUDE")
      endforeach()
    endif()

    foreach (dir ${arg_INCLUDE_DIRS})
      install(
        DIRECTORY ${dir}
        DESTINATION "${INCLUDEDIR}/"
        FILES_MATCHING
        ${exclude_patterns}
        PATTERN "*.hpp" "*.h"
      )
    endforeach()
  endif()

  # Search build aux an use WINDOWS_LIBS if on win32.
  if (WIN32)
    if (arg_AUX_DLLS OR NOT arg_WINDOWS_LIBS)
      file(GLOB extra_libs "${CMAKE_SOURCE_DIR}/build-aux/*.dll")
      if (extra_libs)
        list(APPEND arg_WINDOWS_LIBS ${extra_libs})
      endif()
    endif()
  else()
    set(arg_WINDOWS_LIBS)
  endif()

  if (arg_LIBS OR arg_BINS OR arg_WINDOWS_LIBS)
    install(
      TARGETS ${arg_LIBS} ${arg_BINS} ${arg_WINDOWS_LIBS}
      RUNTIME DESTINATION "${BINDIR}"
      ARCHIVE DESTINATION "${ARCHIVEDIR}"
      LIBRARY DESTINATION "${LIBDIR}"
    )
  endif()

  # Guess runnables.
  if (arg_BINS AND arg_RUNNABLES_VAR)
    # TODO: doesn't work - use output_path property
    foreach(bin ${arg_BINS})
      get_filename_component(bin_name NAME "${bin}")
      list(APPEND ${arg_RUNNABLES_VAR} "${bin_name}" "${bin}")
    endforeach()
  endif()

  # Things to strip
  if (arg_BINARIES_VAR)
    if (arg_BINS)
      # TODO: again, use output path property
      list(APPEND ${arg_BINARIES_VAR} ${arg_BINS})
    endif()

    if (arg_LIBS)
      # TODO: again, use output path property
      list(APPEND ${arg_BINARIES_VAR} ${arg_LIBS})
    endif()
  endif()
endmacro()

butil_auto_install(
  BINARIES_VAR installed_binaries
  # RUNNABLES_VAR varname
  BINS ${BIN_TUNE} 
  # LIBS library...
  # INCLUDE_DIRS dir...
  NO_HEADERS # [HEADER_EXCLUDE pattern... | NO_HEADERS]
  # WINDOWS_LIBS library...]
  # AUX_DLLS
)

# message(STATUS "Adding install targets.")

# set(INCLUDEDIR "include")
# set(BINDIR "bin")
# set(DATADIR "share")
# set(LIBDIR "lib")
# set(MANDIR "${DATADIR}/man")
# set(DOCDIR "${DATADIR}/doc/${PROJECT_NAME}")
# set(HTMLDIR "${DATADIR}/doc/${PROJECT_NAME}")
# set(PDFDIR "${DATADIR}/doc/${PROJECT_NAME}")
# set(DVIDIR "${DATADIR}/doc/${PROJECT_NAME}")
# set(PSDIR "${DATADIR}/doc/${PROJECT_NAME}")

# if (WANT_DOCS) 
  # Documentation
#   install(
#     FILES README COPYING 
#     DESTINATION ${DOCDIR}
#   )

#   if (WANT_DOCS_MAN)
#     add_and_install_manpages("${MANDIR}" "${CMAKE_SOURCE_DIR}/doc/man/tune.1")
#   endif()
# endif(WANT_DOCS)

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
# install(
#   TARGETS ${BIN_TUNE}
#   RUNTIME DESTINATION ${BINDIR}
#   ARCHIVE DESTINATION ${LIBDIR}
#   LIBRARY DESTINATION ${LIBDIR}
# )


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

