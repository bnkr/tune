# Utility routines.
#
# butil_parse_args - parse named arguments.
# butil_standard_setup - set up a bunch of vars etc.
# butil_cpack_setup - set up cpack, with a lot of error checking and good 
#                     defaults.

# TODO: add butil_check_arg stuff.

###########################
# MACRO: butil_parse_args()
#
# Sets any marked variable to that of the varname.  Everything which is not 
# marked by a name (like VARNAME val1 ...) goes in a var called PA_OTHER.  
# Only those vars in $allowed or $flags are searched for.
#
# Currently parse_args undefines all its output variables before parsing and
# ensures that non-flag arguments must have a value, though the value may be
# empty.
#
# Flags are always precisely TRUE or FALSE.
#
# butil_parse_args(
#   allowed_args_list
#   allowed_flags_list
#   ignored_args_or_flags_list
#   arguments_list
#   [allow_duplicates]
# )
#
# Args:
# - allowed   = which arguments are allowed.  Vars with their name will be set to
#               their value.
# - flags     = which arguments are flags (they take no values).  They will be set
#               boolean.
# - ignore    = which things are arguments, but we shouldn't actually assign them.
#               IOW, this causes parse of a subset of the total args.  Unparsed args
#               go in PA_IGNORED.
# - argslist  = argv
# 
# The ignore parameter will cause markers to put stuff in he PA_OTHER into
# PA_IGNORED.
#
# Duplicate variables append, so ARG x y ARG z => ARG = x,y,z if allow_duplicates
# is true.
#
# Duplicate flags have no special effects.
#
# Empty arguments cause an error - they should be specified as flags.
#
# TODO: write examples of the ignore list thing.  
# TODO: a simple unit test for this would be useful.
macro(butil_parse_args allowed flags ignore arglist)
  # Clear anything which may be in the scope already
  foreach (a ${allowed} ${flags}) 
    set(arg_${a})
  endforeach()

  if (ARG4)
    set(allow_duplicates TRUE)
  endif()

  set(pa_found)
  set(pa_found_flags)
  set(pa_var)
  set(pa_add TRUE)
  set(pa_skip FALSE)
  foreach(arg ${arglist})
    # Set the varptr to PA_IGNORED if applicable.
    foreach (search_arg ${ignore}) 
      if ("x${arg}" STREQUAL "x${search_arg}")
        set(pa_var "PA_IGNORED")
        # Yes, we DO want this one!
        set(pa_add TRUE)
        set(pa_skip TRUE)
        break()
      endif()
    endforeach()

    # pa_skip says the work is already done
    if (NOT pa_skip) 
      # Set the varptr to the marker var if applicable.
      foreach (search_arg ${allowed})
        if ("x${arg}" STREQUAL "x${search_arg}")

          if (pa_${arg}_found AND NOT allow_duplicates)
            message(FATAL_ERROR "error parsing arguments: ${arg} was already given.")
          endif()

          set(pa_var arg_${arg})
          set(pa_skip TRUE)
          list(APPEND pa_found "${arg}")

          # Don't add this to the last var's arglist
          set(pa_add)
          set(pa_${arg}_found TRUE)
          break()
        endif()
      endforeach()
    endif()

    if (NOT pa_skip)
      # Set directly set the var to true if present and we didn't just set ignored.
      foreach (search_arg ${flags}) 
        if ("x${arg}" STREQUAL "x${search_arg}")
          set(arg_${arg} TRUE)
          list(APPEND pa_found_flags "${arg}")
          set(pa_add)
          break()
        endif()
      endforeach()
    endif()

    # If this arg is not a marker var, then add it to the varptr.
    if (pa_add) 
      if (NOT pa_var) 
        list(APPEND "PA_OTHER" "${arg}")
      else()
        list(APPEND ${pa_var} "${arg}")
      endif()
    endif()

    set(pa_add "TRUE")
    set(pa_skip "FALSE")
  endforeach()

  # Ensure args always have values.
  if (pa_found)
    foreach (arg ${pa_found})
      # Clean up
      set(pa_${arg}_found)

      if (NOT DEFINED "arg_${arg}")
        message(FATAL_ERROR "error parsing arguments: ${arg} requires a value.")
      endif()
    endforeach()
  endif()

  # Ensure flags are TRUE xor FALSE.
  if (pa_found_flags)
    foreach (arg ${pa_found_flags})
      set(pa_var "arg_${arg}")
      if (NOT DEFINED "${pa_var}")
        set(${pa_var} FALSE)
      endif()
    endforeach()
  endif()

  # Clean up
  set(pa_found)
  set(pa_found_flags)
  set(pa_add)
  set(pa_var)
  set(pa_skip)
endmacro()

# Set up standard values.  Add warnings and provide some extra error checking.  Also adds the
# WANT_DOCS, WANT_DOCS_MAN options.
macro(butil_standard_setup)
  message(STATUS "Setting up standard build vars.")

  if (NOT PROJECT_VERSION)
    message(FATAL_ERROR "PROJECT_VERSION is not defined (blame maintiner).")
  endif()

  if (NOT PROJECT_NAME)
    message(FATAL_ERROR "PROJECT_NAME is not defined (blame maintainer).")
  endif()

  if (CMAKE_BUILD_TYPE STREQUAL "")
    message("Warning: CMAKE_BUILD_TYPE is empty.  This might not work.  Usually you want Release or RelWithDebInfo.")
  endif()

  string(REGEX REPLACE "^([0-9]+).*" "\\1" PROJECT_MAJOR ${PROJECT_VERSION})
  string(REGEX REPLACE "^[^.]+\\.([0-9]+).*" "\\1" PROJECT_MINOR ${PROJECT_VERSION})
  string(REGEX REPLACE "^[^.]+\\.[^.]+\\.([0-9]+).*" "\\1" PROJECT_PATCH ${PROJECT_VERSION})
  string(REGEX REPLACE "^[^.]+\\.[^.]+\\.[^\\-]-([0-9]+).*" "\\1" PROJECT_SNAPSHOT ${PROJECT_VERSION})

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

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Warning: 'Debug' build type is designed for maintainers.")
    set(DOXYGEN_CMAKE_VERBOSE "YES")
  endif()


  include_directories("${CMAKE_SOURCE_DIR}/include/" "${CMAKE_BINARY_DIR}/include/")

  option(WANT_DOCS "Install documentation - when off, all other docs options are off." YES)
  option(WANT_DOCS_MAN "Install manpages if there are any." YES)
 
  mark_as_advanced(WANT_DOCS_MAN)
endmacro()

# Check arguments by parse_args were correct.
#
# Example:
#
# butil_check_arg(
#   ARG YES NO YES ""
# )
# 
# ARG may be undefined, can be a list, but may not be empty, can be any value.
macro(butil_check_arg name allow_undef allow_empty allow_list allowed_values default_value)
endmacro()

# Check an optional argument; give default if not defined.
macro(butil_check_arg_opt name default allowed_values)
endmacro()

# Check an agrument which is an enum
macro(butil_check_arg_enum name allow_undef allowed_values)
endmacro()

# Validate a list argument.  Max_size = 0 for no limit, allowed = empty for
# any values.
macro(butil_check_arg_list name min_size max_size allowed_values)
endmacro()

##########################
# MACRO: butil_cpack_setup
#
# butil_cpack_setup(
#   [URL package_homepage]
#   [VENDOR name]
#   [DESCRIPTION description]
#   [LONG_DESCRIPTION descr]
#   [EMAIL address]
#   [BINARIES binpath...]
#   [ICON bmpfile]
#   [RUNNABLES name path ...]
# )
#
# Assumes PROJECT_MAJOR/MINOR/PATCH is defined already as well as 
# PROJECT_VERSION.
#
# All outputs are turned off by default, except binary NSIS, binary ZIP,
# and source TARBZ.
#
# DESCRIPTION defaults to the project version and should be very short.  It's 
# used as the title of the installer, etc.
#
# ICON is the icon of the installer program.
#
# BINARIES is a list of things to strip in bin packages.  It should be the 
# install path (eg, bin/mybin).
#
# RUNNABLES is used to make start menu entries etc.
#
# TODO: 
#   other stuff:
#
#   - how do icons really work?
#   - CPACK_PACKAGE_INSTALL_REGISTRY_KEY
#   - CPACK_NSIS_INSTALLED_ICON_NAME - what is it for?  It's normally set 
#     to a binary...
#
# TODO:
#   Specific bin support.  Different vars are required for different targets.
#   For completeness we should require them all, *unless* something has been
#   flagged as turned off.  See http://www.cmake.org/Wiki/CMake:CPackPackageGenerators
#
# TODO:
#   Executable_name stuff.  This is problematic.  It a list of pairs ("path" "name").
#
# TODO: can we autodetect binaries?
#
# TODO: arg_RUNNABLES is really messy.
macro(butil_cpack_setup)
  message(STATUS "Setting up CPack stuff.")
  butil_parse_args(
    "URL;VENDOR;DESCRIPTION;EMAIL;BINARIES;ICON;LONG_DESCRIPTION;RUNNABLES" 
    "" 
    "" 
    "${ARGV}"
  )

  # TODO: validate arguments properly (butil_check_arg)

  if (arg_VENDOR)
    set(CPACK_PACKAGE_VENDOR "${arg_VENDOR}")
  endif()

  if (NOT arg_LONG_DESCRIPTION)
    set(arg_LONG_DESCRIPTION "${arg_DESCRIPTION}")
  endif()

  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${arg_LONG_DESCRIPTION}")
  set(CPACK_SOURCE_IGNORE_FILES "/\\\\..*/;~$;.*\\\\.swp$;${CMAKE_BINARY_DIR}.*")
  set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/build-aux/install-readme.txt")

  set(CPACK_SOURCE_TGZ  "ON")
  set(CPACK_SOURCE_TBZ2 "ON")
  set(CPACK_SOURCE_TZ   "OFF")

  set(CPACK_BINARY_STGZ "OFF")
  set(CPACK_BINARY_TBZ2 "OFF")
  set(CPACK_BINARY_TGZ  "OFF")

  if (WIN32)
    set(CPACK_BINARY_ZIP  "ON")
    set(CPACK_BINARY_NSIS "ON")
  else()
    set(CPACK_BINARY_ZIP  "OFF")
    set(CPACK_BINARY_NSIS "OFF")
  endif()

  if (NOT DEFINED PROJECT_MAJOR)
    message(FATAL_ERROR "butil_cpack_setup(): PROJECT_MAJOR must be defined.")
  endif()

  if (NOT DEFINED PROJECT_MINOR)
    message(FATAL_ERROR "butil_cpack_setup(): PROJECT_MINOR must be defined.")
  endif()

  if (NOT DEFINED PROJECT_PATCH)
    message(FATAL_ERROR "butil_cpack_setup(): PROJECT_PATCH must be defined.")
  endif()

  set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_PATCH})

  if (NOT PROJECT_VERSION)
    message(FATAL_ERROR "butil_cpack_setup(): PROJECT_VERSION must be defined and non-empty.")
  endif()

  if (arg_ICON)
    set(CPACK_PACKAGE_ICON "${arg_ICON}")
    # TODO: what is this for?  We could use the first name in arg_RUNNABLES. 
    # set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  endif()

  set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")
  set(display)
  if (arg_DESCRIPTION)
    set(display " - ${arg_DESCRIPTION}")
  endif()
  set(CPACK_NSIS_DISPLAY_NAME "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}${display}")

  if (arg_URL)
    # TODO: replace http:// with http:\\\\\\\\
    set(CPACK_NSIS_HELP_LINK "${arg_URL}")
    set(CPACK_NSIS_URL_INFO_ABOUT "${arg_URL}")
  endif()

  if (arg_EMAIL)
    set(CPACK_NSIS_CONTACT "${arg_EMAIL}")
  endif()

  # what does this do?
  set(CPACK_NSIS_MODIFY_PATH ON)

  if (arg_RUNNABLES)
    set(CPACK_PACKAGE_EXECUTABLES ${arg_RUNNABLES})
  endif()
  set(CPACK_STRIP_FILES "${arg_BINARIES}")

  include(CPack)
endmacro()

