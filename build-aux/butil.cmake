# Utility routines.

# Sets any marked variable to that of the varname.  Everything which is not 
# marked by a name (like VARNAME val1 ...) goes in a var called PA_OTHER.  
# Only those vars in $allowed or $flags are searched for.
#
# Variables arg_VAR are set to their value.
#
# Examples:
#
#   butil_parse_args("ARGS" "" "" "ARGS;x;y")
#   ARGS = x;y
#
#   butil_parse_args("ARGS;SRCS" "" "" "ARGS;x;y;SRCS;x;y;sdsdsd")
#   ARGS = x;y, SRCS = x;y;sdsdsd
#
#   butil_parse_args("ARGS;SRCS" "" "" "blah1;blah2;ARGS;x;y;SRCS;x;y;sdsdsd")
#   ARGS = x;y, SRCS = x;y;sdsdsd, OTHER = blah1;blah2
#
# The ignore parameter will cause markers to put stuff in he PA_OTHER into
# PA_IGNORED.
#
# Duplicate variables append, so ARG x y ARG z => ARG = x,y,z.
#
# Duplicate flags have no special effects.
#
# Empty arguments have no effect.
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
# TODO: write examples of the ignore list thing.  
# TODO: write a way to error if duplicate args defined (NO_DUPLICATES), also a 
#       NO_REDEFINE would be good to avoid overwriting important vars but there 
#       seems to be a problem with variable scoping.
# TODO: a simple unit test for this would be useful.#
# TODO: a way to force no empty arguments - makes sense since we have flags vs.
#       allowed.
# TODO: it would be sensible to prefix the args, like arg_NAME = val.
macro(butil_parse_args allowed flags ignore arglist)
  # Clear anything which may be in the scope already
  foreach (a ${allowed} ${flags}) 
    set(arg_${a})
  endforeach()

  set(pa_var)
  set(pa_add "TRUE")
  foreach(arg ${arglist})
    # Set the varptr to PA_IGNORED if applicable.
    foreach (search_arg ${ignore}) 
      if ("x${arg}" STREQUAL "x${search_arg}")
        set(pa_var "PA_IGNORED")
        # Yes, we DO want this one!
        set(pa_add TRUE)
        break()
      endif()
    endforeach()

    if (NOT pa_var STREQUAL "PA_IGNORED") 
      # Set the varptr to the marker var if applicable.
      foreach (search_arg ${allowed})
        if ("x${arg}" STREQUAL "x${search_arg}")
          set(pa_var arg_${arg})
          # Don't add this to the last var's arglist
          set(pa_add)
          break()
        endif()
      endforeach()

      # Set directly set the var to true if present and we didn't just set ignored.
      foreach (search_arg ${flags}) 
        if ("x${arg}" STREQUAL "x${search_arg}")
          set(arg_${arg} TRUE)
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
  endforeach()
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
