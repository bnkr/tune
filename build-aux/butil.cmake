# Utility routines.
#
# butil_parse_args - parse named arguments.
# butil_join - join a list into a string.
#
# butil_standard_setup - set up a bunch of vars and some default options.
# butil_auto_install - automatically generate install targets and provide some
#                      values for cpack_setup.
# butil_cpack_setup - set up cpack, with a lot of error checking and good
#                     defaults.
#
# butil_add_library - wrapper of add_library() and  properties.
# butil_add_executable - wrapper of add_exeutable() and properties.
#
# butil_find_lib - search for libraries given a list of possible names.

# TODO: add butil_check_arg stuff.

###############################
# MACRO: butil_standard_setup()
#
# butil_standard_setup(
#   [NO_TESTING | TESTING <true|false>]
#   [DISABLE_CTAGS]
# )
#
# Set up some variables used later by butil; also provide some standard options
# etc.
#
# Options are:
# - WANT_DOCS (default yes)
# - WANT_DOCS_MAN (default yes if unix)
#
# Also resets the cflags and adds some expected include dirs.
#
# It builds ctags if ctags is found and the build type is debug.
#
#############################
# MACRO: butil_auto_install()
#
# butil_auto_install(
#   [RUNNABLES_VAR varname]
#   [RUNNABLES target pretty_name...]
#   [BINARIES_VAR varname]
#   [TARGETS target...]
#   [INCLUDE_DIRS dir...]
#   [HEADER_EXCLUDE pattern... | NO_HEADERS]
#   [WINDOWS_LIBS library...]
#   [BUILD_AUX_DLLS]
#   [HARDLINK_AUX_DLLS]
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
# RUNNABLES_VAR - list of path, name pairs to use as cpack executables.  This
# is generated from RUNNABLES, and can be passed directly to butil_setup_cpack().
#
# RUNNABLES - list of *target*, pretty name pairs.  Required if RUNNABLES_VAR.
#
# BINARIES_VAR - list of all files for cpack to strip.  This can be passed
# directly to butil_setup_cpack()
#
# TARGETS - any *target* created by add_executable or add_library.  Required if
# BINARIES_VAR is given.  It is used for making packages and for finding things
# to strip.
#
# INCLUDE_DIRS - directories to install into the include path.   Make sure they
# it DOES have the trailing foreward slash.
#
# HEADER_EXCLUDE - excludes a regexp from the install of include/.  NO_HEADERS
# disables install of headers entirely even if there exists an include/.
# You must manually install any extra headers, for exampe those made with
# configure_file.
#
# WINDOWS_LIBS is a specific list of dlls to install on windows (also they
# get put in binary packages).  If this is specified then build-aux isn't
# searched for dlls.  These libs will be copied into the binary directory so
# that they can be symlinks but we can still install them.
#
# BUILD_AUX_DLLS - if present, then build-aux/ is searched for dlls regardless
# of whether WINDOWS_LIBS is specified.
#
# HARDLINK_AUX_DLLS - try to use ln to put the dlls into the binary dir isntead
# of doing a copy.  Note that this only works if the target dll is *not* a
# symlink.  Otherwise you hardlink to the symlink and it doesn't install
# properly.
#
############################
# MACRO: butil_cpack_setup()
#
# butil_cpack_setup(
#   [URL package_homepage]
#   [VENDOR name]
#   [DESCRIPTION description]
#   [LONG_DESCRIPTION descr]
#   [DESCRIPTION_FILE file]
#   [EMAIL address]
#   [BINARIES binpath...]
#   [ICON bmpfile]
#   [RUNNABLES name path ...]
#   [DEB_ARCH arch_name]
#   [DEB_SECTION name]
#   [DEB_DEPENDS "package (op version)"... ]
#   [ AUTO_DEPENDS TARGETS target...]
# )
#
# Assumes PROJECT_MAJOR/MINOR/PATCH is defined already as well as
# PROJECT_VERSION.
#
# Sensible binary outputs are enabled for different systems if you gave enough
# information for them.  The user can still turn them on or off later if they
# want.
#
# It requires README, COPYING, and build-aux/install-readme.txt.  The latter is
# for additional installation instructions, and may be empty.
#
# DESCRIPTION defaults to the project version and should be very short.  It's
# used as the title of the installer.
#
# LONG_DESCRIPTION - defaults to DESCRIPTION and is used for the short package
# summary.  Intended to be about a line line (so 80 chrs ish).
#
# DESCRIPTION_FILE - used for things like a .deb description.  Indented to be a
# paragraph or two long.  Uses build-aux/description.txt if not specified.
#
# ICON is the icon of the installer program.
#
# BINARIES is a list of things to strip in bin packages.  It should be the
# install path (eg, bin/mybin).
#
# RUNNABLES is used to make start menu entries etc.
#
# DEB_ARCH - architecture for debian.  This is necessary when cross-compiling,
# but otherwise is determined using dpkg if available.
#
# DEB_SECTION - defaults to devel.  You need to look through aptitude for an
# apropriate section to use.
#
# DEB_DEPENDS - a list of depends which look like "package (op version)".
#
# AUTO_DEPENDS - if given, then dependncies are worked out based on TARGETS.
# This can only detect libraries which are linked to the TARGETs' output.
# Note that this makes the build *really slow*!  DEB_DEPENDS is also used as
# an extra depenencies.  Using AUTO_DEPENDS will cause the cache to regenerate
# one more time after make all is run because otherwise we don't have the
# TARGETS built to get depends from!
#
# TARGETS - binary targets which are installed.
#

############################
# MACRO: butil_add_library()
#
# Wrap add_library to add a versioned unix style .a and .so.
#
# butil_add_library(
#   TARGET name
#   SOURCES source...
#   [TARGETS_VAR var]
#   [LIBS lib...]
#   [CPPDEFS define...]
#   [CFLAGS flag...]
#   [LDFLAGS flag...]
#   [AR_CPPFLAGS define...]
#   [AR_CFLAGS flag...]
#   [AR_LDFLAGS flag...]
# )
#
# TARGET - outputs ${target}_shared and ${target}_static.  Also becomes the
# output name of the lib.
#
# TARGETS_VAR - variable to *append* generated targets to.
#
# SOURCES - src files.
#
# LIBS - link libraries.
#
# *CPPDEFS
# *CFLAGS (note that the -I goes here if you absolutely must pass an -I!)
# *LDFLAGS
# - flags which are *appended* to the target.  AR_* is for the archive; if not
#   present then the soflags are used.
#
# Requires that PROJECT_MAJOR/MINOR/PATCH is already defined.
#
###############################
# FUNCTION: butil_add_executable()
#
# Basically just a named parameter wrapper over add_executable and target
# properties.
#
# butil_add_executable(
#   TARGET name
#   SOURCES file...
#   [LIBS library...]
#   [CPPDEFS define...]
#   [CFLAGS compile_flag...]
#   [LDFLAGS link_flag...]
# )
#

#####################
# MACRO: butil_join()
#
# Join a list with a given string.
#
# butil_join(
#   [OUTPUT_VAR] output_var
#   [LIST_VAR] list_var
#   [STRING join_string]
# )
#
# Remember that the parameters are VARS, not lists!
#
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
#########################
# macro: butil_find_lib()
#
# Find a library given a list of possible names.
#
# butil_find_lib(
#   VAR output
#   NAMES name...
#   [REQUIRED]
# )
#
# Self-explanitory.
#
# NAMES is optional.


# TODO: write examples of the ignore list thing.
# TODO: a simple unit test for this would be useful.
macro(butil_parse_args allowed flags ignore arglist)
  # Clear anything which may be in the scope already
  foreach (a ${allowed} ${flags} ${ignore})
    set(arg_${a})
  endforeach()
  set(PA_IGNORED)
  set(PA_OTHER)

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

  butil_parse_args(
    "TESTING"
    "NO_TESTING;DISABLE_CTAGS"
    ""
    "${ARGV}"
  )

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

  set(build_ctags TRUE)
  if (NOT arg_DISABLE_CTAGS)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
      find_program(CTAGS_EXE ctags)
      mark_as_advanced(CTAGS_EXE)

      if (CTAGS_EXE)
        add_custom_target(
          ctags ALL
          COMMAND ctags -R src/ include/
          WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        )
      endif()
    endif()
  endif()

  include_directories("${CMAKE_SOURCE_DIR}/include/" "${CMAKE_BINARY_DIR}/include/")

  option(WANT_DOCS "Install documentation - when off, all other docs options are off." YES)
  set(default "NO")
  if (UNIX)
    set(default "YES")
  endif()
  option(WANT_DOCS_MAN "Install manpages if there are any." ${default})

  mark_as_advanced(WANT_DOCS_MAN)

  if (NOT arg_NO_TESTING AND NOT arg_TESTING)
    enable_testing()
  endif()
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

# private tidyness function for making deb.  Completely reliant on being called from
# butil_cpack_setup.
#
# TODO:
#   This function could be seperate so we can make multiple packages.  Really
#   tricky because cpacke_setup already sets a whole heap of stuff that this
#   function really needs.  Maybe we add SETUP_DEB_ARGS_VAR?
#
# TODO:
#   add option: DEB_PRUNE_DEPENDS to get rid of packages that you know you
#   shouldn't depend on (eg libc-i686).  Or better find a way to automatically
#   remove such libs; they are in /usr/lib/cmov/$arch/.
macro(butil_cpack_setup_deb)
  if (arg_DEB_ARCH)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${arg_DEB_ARCH})
  elseif (DPKG_EXE)
    set(system_arch)
    execute_process(
      COMMAND "${DPKG_EXE}" --print-architecture
      OUTPUT_VARIABLE system_arch
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${system_arch})
  else()
    message("butil_cpack_setup(): warning: could not determine the system's architecture.  Give DEB_ARCH argument or define DPKG_EXE.")
  endif()

  if (arg_DEB_SECTION)
    set(CPACK_DEBIAN_PACKAGE_SECTION "${arg_DEB_SECTION}")
  endif()

  if (arg_EMAIL)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${arg_EMAIL}")
  endif()

  if (EXISTS "${arg_DESCRIPTION_FILE}")
    file(READ "${arg_DESCRIPTION_FILE}" descr)
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${arg_LONG_DESCRIPTION}\n${descr}")
  elseif(arg_LONG_DESCRIPTION)
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${arg_LONG_DESCRIPTION}")
  endif()

  string(LENGTH "${arg_LONG_DESCRIPTION}" len)
  if (len GREATER 60)
    message("butil_cpack_setup(): warning: LONG_DESCRIPTION is ${len}; should be 60 max.")
  endif()

  # Cpack doesn't do this automatically for some reason.  The package is not
  # installable unless you do.

  # TODO:
  #   also we need to wordwrap the description at 80ch - the indent.
  string(STRIP "${CPACK_DEBIAN_PACKAGE_DESCRIPTION}" temp)
  string(REPLACE "\r\n\r\n" "\n" temp "${temp}")
  string(REPLACE "\n\n" "\n.\n" temp "${temp}")
  string(REPLACE "\n" "\n " temp "${temp}")
  set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${temp}")

  if(arg_AUTO_DEPENDS)
    # Registers the last time we ran make all.
    set(make_all_marker "butil-make-all-has-run")

    set(targ_locations)
    set(location_prop "LOCATION_${CMAKE_BUILD_TYPE}")
    foreach (targ ${arg_TARGETS})
      get_target_property(loc "${targ}" "${location_prop}")
      list(APPEND targ_locations "${loc}")
    endforeach()

    # If we've never run make all then reconfigure.
    set(cmd)
    set(cmd "${cmd} if test ! -f \"${make_all_marker}\"; then")
    set(cmd "${cmd}   \"${CMAKE_COMMAND}\" -E touch \"${make_all_marker}\";")
    set(cmd "${cmd}   \"${CMAKE_COMMAND}\" \"${CMAKE_BINARY_DIR}\";")
    set(cmd "${cmd} fi")

    add_custom_command(
      OUTPUT "${make_all_marker}"
      DEPENDS ${targ_locations}
      COMMAND sh -c "${cmd}"
      VERBATIM
      COMMENT "Regenerated cache to get dependencies of newly built binaries."
    )
    # Mostly just here so make_all_marker gets deleted with make clean.
    add_custom_target(regenerate_package_dependencies ALL DEPENDS "${make_all_marker}")

    if (NOT EXISTS "${make_all_marker}")
      message(STATUS "butil_cpack_setup(): cache will rebuild once after make all to find dependencies.")
      # touch the file to notify a rebuild
    else()
      # stop the rebuild from happening.
      message(STATUS "butil_cpack_setup(): searching for dependencies - this can be slow.  Turn off AUTO_DEPENDS to avoid this...")

      foreach (bin ${targ_locations})

        # TODO: should find_program all of these and fail  if it's not right.
        execute_process(
          COMMAND sh -c
                  "objdump -p \"${bin}\" | grep NEEDED | awk '{ print \"*\"$2 }' |
                  xargs --max-args=1 dpkg --search | awk '{ print $1 }' |
                  sort | uniq | sed 's/:$//' "
          ERROR_QUIET
          OUTPUT_VARIABLE output
        )

        message(STATUS "butil_cpack_setup(): found depends for '${bin}' ...")
        string(REGEX MATCHALL "[^ \n\r\t]+" names "${output}")

        if (names)
          foreach (pkg ${names})
            if (CPACK_DEBIAN_PACKAGE_DEPENDS)
              set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, ${pkg}")
            else()
              set(CPACK_DEBIAN_PACKAGE_DEPENDS "${pkg}")
            endif()
          endforeach()
        endif()
      endforeach()

    endif()
  endif()

  if(arg_DEB_DEPENDS)
    foreach (d ${arg_DEB_DEPENDS})
      if (CPACK_DEBIAN_PACKAGE_DEPENDS)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${depstring}, ${d}")
      else()
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${d}")
      endif()
    endforeach()
  endif()

  # Not supported in cpack yet.
  # if (arg_URL)
  #   set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${arg_URL}")
  # endif()

  # CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
  # This variable allow advanced user to add custom script to the control.tar.gz (inside the .deb archive)
  # Typical examples are:
  # - conffiles
  # - postinst
  # - postrm
  # - prerm"
  # Usage:
  # SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
  #    "${CMAKE_CURRENT_SOURCE_DIR/prerm;${CMAKE_CURRENT_SOURCE_DIR}/postrm")

  # TODO: Add args DEB_SUGGESTS DEB_RECOMMENDS.
  # TODO: add DEBIAN_PACKAGE_RECOMMENDS, DEBIAN_PACKAGE_SUGGESTS in the same way.
endmacro()

# Helper function for nsis.  Must only be called from cpack_setup
macro(butil_cpack_setup_nsis)
  set(display)
  if (arg_DESCRIPTION)
    set(display " - ${arg_DESCRIPTION}")
  endif()
  set(CPACK_NSIS_DISPLAY_NAME "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}${display}")

  if (arg_EMAIL)
    set(CPACK_NSIS_CONTACT "${arg_EMAIL}")
  endif()

  # CPACK_PACKAGE_EXECUTABLES has stupid behavior because it always prepends
  # bin/ and always appends .exe!  There's currently a bug where the menu links
  # don't show up unless there is an entry in there so we can't stop using it.
  set(CPACK_NSIS_MENU_LINKS)
  if (arg_RUNNABLES)
    if (NOT CPACK_PACKAGE_EXECUTABLES)
      set(proglist "${arg_RUNNABLES}")
      set(found)
      while (proglist)
        list(GET proglist 0 path)
        list(GET proglist 1 name)
        list(REMOVE_AT proglist 0 1)

        # Here is the hack to fix it!
        if (path MATCHES "^bin[/\\]" AND NOT found)
          string(REGEX REPLACE "bin[/\\]" "" path "${path}")
          set(CPACK_PACKAGE_EXECUTABLES "${path}" "${name}")
          set(found TRUE)
        else()
          list(APPEND CPACK_NSIS_MENU_LINKS "${path}" "${name}")
        endif()
      endwhile()

      if (NOT found)
        set(longmsg "butil_setup_cpack(): warning: no RUNNABLES in bin/ so a hack value is used for")
        set(longmsg "${longmsg} CPACK_PACKAGE_EXECUTABLES which NSIS *should* ignore.  If it breaks,")
        set(longmsg "${longmsg} then define CPACK_PACKAGE_EXECUTABLES to a list of the form \"$path;$name\"")
        set(longmsg "${longmsg} and then delete the dummy start menu link yourself.")
        message("${longmsg}")
        # thanks to daedalusfall for this workaround.
        set(CPACK_PACKAGE_EXECUTABLES "I AM CONFUSED" "I DO NOTHING")
      else()
        message(STATUS "butil_setup_cpack(): using '${CPACK_PACKAGE_EXECUTABLES}' as CPACK_PACKAGE_EXECUTABLES to fix a CPack bug.")
      endif()
    else()
      list(APPEND CPACK_NSIS_MENU_LINKS ${arg_RUNNABLES})
    endif()
  endif()

  if (arg_ICON)
    set(CPACK_PACKAGE_ICON "${arg_ICON}")
  endif()

  # TODO: what is this for?  We could use the first name in arg_RUNNABLES, or use arg_ICON?
  # set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  # CPACK_NSIS_INSTALLER_ICON_CODE
  # CPACK_NSIS_INSTALLER_MUI_ICON_CODE

  # TODO: what does this do?
  set(CPACK_NSIS_MODIFY_PATH ON)

  if (arg_URL)
    # Note: the docs said that you need at least on path with \\\\ in it, but that bug
    # seems to be fixed now.
    set(CPACK_NSIS_HELP_LINK "${arg_URL}")
    set(CPACK_NSIS_URL_INFO_ABOUT "${arg_URL}")
  endif()
endmacro()

# TODO:
#   other stuff:
#
#   - how do icons really work?
#   - CPACK_PACKAGE_INSTALL_REGISTRY_KEY (seems to be defaulted anyway)
#   - CPACK_NSIS_INSTALLED_ICON_NAME - what is it for?  It's normally set
#     to a binary...
macro(butil_cpack_setup)
  message(STATUS "Setting up CPack stuff.")
  butil_parse_args(
    "URL;VENDOR;DESCRIPTION;DESCRIPTION_FILE;EMAIL;BINARIES;ICON;LONG_DESCRIPTION;RUNNABLES;DEB_ARCH;DEB_SECTION;DEB_DEPENDS;TARGETS"
    "AUTO_DEPENDS"
    ""
    "${ARGV}"
  )

  # TODO: validate arguments properly (butil_check_arg)
  if (NOT arg_LONG_DESCRIPTION)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
      message("butil_cpack_setup(): warning: it is recommended to give LONG_DESCRIPTION.")
    endif()
    set(arg_LONG_DESCRIPTION "${arg_DESCRIPTION}")
  endif()

  if (arg_AUTO_DEPENDS)
    if (NOT arg_TARGETS)
      message(FATAL_ERROR "butil_cpack_setup(): if AUTO_DEPENDS is specified TARGETS must be given.")
    endif()
  endif()

  find_program(RPMBUILD_EXE rpmbuild)
  find_program(DPKG_EXE dpkg)
  mark_as_advanced(DPKG_EXE RPMBUILD_EXE)

  # TODO:
  #   Other generators:
  #   CPACK_BINARY_BUNDLE
  #   CPACK_BINARY_CYGWIN
  #   CPACK_BINARY_DRAGNDROP
  #   CPACK_BINARY_OSXX11
  #   CPACK_BINARY_PACKAGEMAKER

  # Declare default enable values.  Aparently cpack doesn't make these caches in the
  # normal way -- it overrides them every time so we create a cache option..
  if (NOT BUTIL_CPACK_DEFAULTS_DONE)
    message(STATUS "Setting default cpack generators.")

    set(CPACK_SOURCE_TGZ  "ON")
    set(CPACK_SOURCE_TBZ2 "ON")
    set(CPACK_SOURCE_TZ   "OFF")

    set(CPACK_BINARY_TZ   "OFF")
    set(CPACK_BINARY_STGZ "OFF")

    # Use can still force these on, but we default them off.
    set(CPACK_BINARY_RPM  "OFF")
    # This is always off by default because the generation is very slow.
    set(CPACK_BINARY_DEB  "OFF")

    if (UNIX)
      if (RPMBUILD_EXE)
        set(CPACK_BINARY_RPM  "ON")
      endif()

      set(CPACK_BINARY_TBZ2 "ON")
      set(CPACK_BINARY_TGZ  "ON")
      set(CPACK_BINARY_ZIP  "OFF")
      set(CPACK_BINARY_NSIS "OFF")
    else()
      set(CPACK_BINARY_TBZ2 "OFF")
      set(CPACK_BINARY_TGZ  "OFF")
      set(CPACK_BINARY_ZIP  "ON")
      set(CPACK_BINARY_NSIS "OFF")

      find_program(NSIS_EXE nsis)
      mark_as_advanced(NSIS_EXE)
      if (NSIS_EXE)
        set(CPACK_BINARY_NSIS "ON")
      endif()
    endif()

  endif() # BUTIL_CPACK_DEFAULTS_DONE
  set(BUTIL_CPACK_DEFAULTS_DONE TRUE CACHE INTERNAL "So we don't reset the values every time.")

  # Version related stuff
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
  set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

  # Generic variables
  if (arg_VENDOR)
    set(CPACK_PACKAGE_VENDOR "${arg_VENDOR}")
  endif()

  if (arg_EMAIL)
    set(CPACK_PACKAGE_CONTACT "${arg_EMAIL}")
  endif()

  set(CPACK_STRIP_FILES "${arg_BINARIES}")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${arg_LONG_DESCRIPTION}")
  if (NOT arg_DESCRIPTION_FILE)
    set(arg_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/build-aux/description.txt")
  endif()
  set(CPACK_PACKAGE_DESCRIPTION_FILE "${arg_DESCRIPTION_FILE}")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
  # This is for `additional installation instructions'.
  set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/build-aux/install-readme.txt")

  # TODO: CPACK_PACKAGE_INSTALL_REGISTRY_KEY almost certainly needs to be handled because
  # win32 as some weird limitations about what you can put in a registry key.

  # Source package stuff
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")
  set(CPACK_SOURCE_IGNORE_FILES "/\\\\..*/;~$;.*\\\\.swp$;^${CMAKE_BINARY_DIR}.*")

  # Debian variables.
  if (CPACK_BINARY_DEB)
    message(STATUS "butil_cpack_setup(): Setting up DEB.  Careful!  CPack is not very good at this!.")
    butil_cpack_setup_deb()
  else()
    message(STATUS "butil_cpack_setup(): DEB is disabled.")
  endif()

  # TODO: proper support for RPM here - can't test it without an rpm system :)


  # NSIS stuff
  if (CPACK_BINARY_NSIS)
    message(STATUS "butil_cpack_setup(): Setting up NSIS.")
    butil_cpack_setup_nsis()
  else()
    message(STATUS "butil_cpack_setup(): NSIS is disabled.")
  endif()

  include(CPack)

  # We do this down here just in case CPack modifies them.
  if (CPACK_BINARY_DEB)
    if (CMAKE_CROSS_COMPILING AND NOT arg_DEB_ARCH)
      message("butil_auto_install(): warning: DEB_ARCH should be specified when cross compiling a .deb")
    endif()

    if (NOT arg_DEB_DEPENDS)
      if (NOT arg_AUTO_DEPENDS)
        message("butil_cpack_setup(): warning: DEB_DEPENDS or AUTO_DEPENDS should be given when building a .deb.")
      endif()
    endif()

    if (NOT arg_DEB_SECTION)
      message("butil_cpack_setup(): warning: DEB_SECTION should be given when building a .deb.")
    endif()

    if (NOT UNIX)
      message("butil_cpack_setup(): warning: building a deb is not likely to work when not on UNIX!")
    endif()
  endif()
endmacro()



# Join a list with a given string, default comma.
macro(butil_join)
  butil_parse_args(
    "LIST_VAR;STRING;OUTPUT_VAR"
    ""
    ""
    "${ARGV}"
  )

  list(GET PA_OTHER 0 butil_arg1)
  list(GET PA_OTHER 1 butil_arg2)

  if (NOT arg_OUTPUT_VAR)
    set(arg_OUTPUT_VAR "${butil_arg1}")
    if (NOT arg_OUTPUT_VAR)
      message(FATAL_ERROR "butil_join(): OUTPUT_VAR or the first argument is required")
    endif()
  endif()

  if (NOT arg_LIST_VAR)
    set(arg_LIST_VAR "${butil_arg2}")
    if (NOT arg_LIST_VAR)
      message(FATAL_ERROR "butil_join(): LIST_VAR or the second argument is required")
    endif()
  endif()

  if (NOT arg_STRING)
    set(arg_STRING ", ")
  endif()

  set(butil_list)
  foreach(butil_elt ${${arg_LIST_VAR}})
    if (butil_list)
      set(butil_list "${butil_list}${arg_STRING}${butil_elt}")
    else()
      set(butil_list "${butil_elt}")
    endif()
  endforeach()

  set(${arg_OUTPUT_VAR} ${butil_list})

  # Clean up.
  set(butil_elt)
  set(butil_list)
  set(butil_arg1)
  set(btuil_arg2)
endmacro()

# Append to $target's $property $value_list if defined; else $other_value_list
# if defined else do nothing.
function(butil_append_target_prop target property value_list other_value_list)
  if (NOT "x${value_list}" STREQUAL "x")
    # message("set ${target}.${property} with values '${value_list}'")
    set_property(TARGET ${target} APPEND PROPERTY ${property} ${value_list})
  elseif (NOT "x${other_value_list}" STREQUAL "x")
    set_property(TARGET ${target} APPEND PROPERTY ${property} ${other_value_list})
  endif()
endfunction()

# Wrap adding an executable.
#
# TODO: interesting idea: have this function run for different configs/compilers,
# like add_exe(TARGET t GCC_COMPILERS CONFIGURATION Debug SOURCES srcfiles CFLAGS...)
# It would detect whether the target has been added yet and update rules/props accordingly.
# Maybe better butil_add_executable(TARGET t SOURCES ... CFLAGS for all...)
# butil_target_properties(CONFIG ... COMPILER ... ).  Anyway, I don't need it yet so
# we'll get on with it.
function(butil_add_executable)
  butil_parse_args(
    "TARGET;CFLAGS;CPPDEFS;LDFLAGS;SOURCES;LIBS"
    ""
    ""
    "${ARGV}"
  )

  if (NOT arg_TARGET OR NOT arg_SOURCES)
    message(FATAL_ERROR "butil_add_executable(): TARGET and SOURCES are required.")
  endif()

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Add exe: ${arg_TARGET}")
    if (arg_LIBS)
      set(basenames)
      foreach (lib ${arg_LIBS})
        get_filename_component(lib "${lib}" NAME)
        list(APPEND basenames "${lib}")
      endforeach()
      butil_join(pretty_list basenames)

      message(STATUS "+ LIBS = ${pretty_list}")
      set(list)
    endif()

    if (arg_CFLAGS)
      butil_join(LIST_VAR arg_CFLAGS OUTPUT_VAR pretty_list)
      message(STATUS "+ CFLAGS = ${pretty_list}")
    endif()

    if (arg_CPPDEFS)
      butil_join(LIST_VAR arg_CPPDEFS OUTPUT_VAR pretty_list)
      message(STATUS "+ CPPDEFS = ${pretty_list}")
    endif()

    if (arg_LDFLAGS)
      butil_join(LIST_VAR arg_LDFLAGS OUTPUT_VAR pretty_list)
      message(STATUS "+ LDFLAGS = ${pretty_list}")
    endif()
  endif()

  add_executable(${arg_TARGET} ${arg_SOURCES})

  if (arg_LIBS)
    target_link_libraries(${arg_TARGET} ${arg_LIBS})
  endif()

  butil_append_target_prop(${arg_TARGET} COMPILE_DEFINITIONS "${arg_CPPDEFS}" "")
  butil_append_target_prop(${arg_TARGET} COMPILE_FLAGS "${arg_CFLAGS}"  "")
  butil_append_target_prop(${arg_TARGET} LINK_FLAGS "${arg_LDFLAGS}" "")

  # Not necessary.
  # if (CMAKE_BUILD_TYPE)
  #   butil_append_target_prop(${arg_TARGET} COMPILE_DEFINITIONS_${CMAKE_BUILD_TYPE} "${arg_CPPDEFS}" "")
  #   butil_append_target_prop(${arg_TARGET} COMPILE_FLAGS_${CMAKE_BUILD_TYPE} "${arg_CFLAGS}"  "")
  #   butil_append_target_prop(${arg_TARGET} LINK_FLAGS_${CMAKE_BUILD_TYPE} "${arg_LDFLAGS}" "")
  # endif()
endfunction()

# Wrap adding a lib.
macro(butil_add_library)
  butil_parse_args(
    "TARGET;SOURCES;LIBS;CPPDEFS;CFLAGS;LDFLAGS;AR_CPPFLAGS;AR_CFLAGS;AR_LDFLAGS;TARGETS_VAR"
    ""
    ""
    "${ARGV}"
  )

  if (NOT DEFINED PROJECT_MAJOR OR NOT DEFINED PROJECT_MINOR OR NOT DEFINED PROJECT_PATCH)
    message(FATAL_ERROR "butil_add_library(): PROJECT_MAJOR, PROJECT_MINOR, and PROJECT_PATCH must be defined.")
  endif()

  if (NOT arg_TARGET)
    message(FATAL_ERROR "butil_add_library(): TARGET is required.")
  endif()

  if (NOT arg_SOURCES)
    message(FATAL_ERROR "butil_add_library(): SOURCES is required.")
  endif()

  set(archive "${arg_TARGET}_static")
  set(sharedlib "${arg_TARGET}_shared")

  if (arg_TARGETS_VAR)
    list(APPEND ${arg_TARGETS_VAR} ${archive})
    list(APPEND ${arg_TARGETS_VAR} ${sharedlib})
  endif()

  add_library(${archive} STATIC ${arg_SOURCES})
  set_target_properties(${archive} PROPERTIES OUTPUT_NAME "${arg_TARGET}")
  butil_append_target_prop(${sharedlib} COMPILE_DEFINITIONS "${arg_AR_CPPDEFS}" "${arg_CPPDEFS}")
  butil_append_target_prop(${sharedlib} COMPILE_FLAGS       "${arg_AR_CFLAGS}"  "${arg_CFLAGS}")
  butil_append_target_prop(${sharedlib} LINK_FLAGS          "${arg_AR_LDFLAGS}" "${arg_LDFLAGS}")

  add_library(${sharedlib} SHARED ${arg_SOURCES})
  set_target_properties(${sharedlib} PROPERTIES OUTPUT_NAME "${arg_TARGET}")
  set_target_properties(${sharedlib} PROPERTIES SOVERSION "${PROJECT_MAJOR}")
  set_target_properties(${sharedlib} PROPERTIES VERSION "${PROJECT_MAJOR}.${PROJECT_MINOR}.${PROJECT_PATCH}")

  if (arg_LIBS)
    target_link_libraries(${sharedlib} ${arg_LIBS})
  endif()

  butil_append_target_prop(${sharedlib} COMPILE_DEFINITIONS "${arg_CPPDEFS}" "")
  butil_append_target_prop(${sharedlib} COMPILE_FLAGS       "${arg_CFLAGS}"  "")
  butil_append_target_prop(${sharedlib} LINK_FLAGS          "${arg_LDFLAGS}" "")

  set(sharedlib)
  set(archive)
endmacro()

# Message that $var is deprecated.  Assign it to $replacement (if given)
macro(butil_warn_deprecated var replace_with append)
  if (arg_${var})
    set(use)
    if (NOT replace_with STREQUAL "")
      if (append)
        set(arg_${replace_with} "${arg_${replace_with}}arg_${var}")
      # Don't mess with it if it's already set.
      elseif (NOT arg_${replacement})
        set(arg_${replace_with} "${arg_${var}}")
      endif()

      set(use "  Use ${replace_with}.")
    endif()

    message("butil_auto_install(): warning: ${var} is deprecated.${use}")
  endif()
endmacro()

# Install lots of stuff.
macro(butil_auto_install)
  message(STATUS "Generating install targets.")

  butil_parse_args(
    "BINARIES_VAR;RUNNABLES_VAR;BINS;LIBS;HEADER_EXCLUDE;WINDOWS_LIBS;INCLUDE_DIRS;EXECUTABLES;RUNNABLES;TARGETS"
    "NO_HEADERS;AUX_DLLS;BUILD_AUX_DLLS;HARDLINK_AUX_DLLS"
    ""
    "${ARGV}"
  )

  butil_warn_deprecated(AUX_DLLS BUILD_AUX_DLLS TRUE)
  butil_warn_deprecated(BINS TARGETS TRUE)
  butil_warn_deprecated(EXECUTABLES TARGETS TRUE)
  butil_warn_deprecated(LIBS TARGETS TRUE)

  if (arg_BINARIES_VAR AND NOT arg_TARGETS)
    message(FATAL_ERROR "butil_auto_install(): TARGETS is required if BINARIES_VAR is given.")
  endif()

  if (arg_RUNNABLES_VAR AND NOT arg_RUNNABLES)
    message(FATAL_ERROR "butil_auto_install(): RUNNABLES is required if RUNNABLES_VAR is given.")
  endif()

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
    file(
      GLOB basic_docs "${CMAKE_SOURCE_DIR}/README*" "${CMAKE_SOURCE_DIR}/COPYING*"
      "${CMAKE_SOURCE_DIR}/ChangeLog" "${CMAKE_SOURCE_DIR}/AUTHORS" "${CMAKE_SOURCE_DIR}/CHANGELOG"
    )

    if (basic_docs)
      install(
        FILES ${basic_docs}
        DESTINATION ${DOCROOT}
      )
    endif()

    # TODO:
    #   Could be better to manually have arg_NO_MANPAGES and arg_MANPAGES; then there's no
    #   depends.
    if (WANT_DOCS_MAN)
      set(src_man "${CMAKE_SOURCE_DIR}/doc/man")
      if (IS_DIRECTORY "${src_man}")
        file(GLOB manpages "${src_man}/*.*")

        # TODO: this should be (INSTALL_ROOT ${mandir} PAGES page...).  (api is not finished yet)
        if (UNIX AND manpages)
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
      # Make sure of the trailing slash, otherwise we might end up
      # installing ${dir} instead od its contents.
      install(
        DIRECTORY   "${dir}/"
        DESTINATION "${INCLUDEDIR}/"
        FILES_MATCHING
        ${exclude_patterns}
        PATTERN "*.hpp"
        PATTERN "*.hxx"
        PATTERN "*.H"
        PATTERN "*.h"
      )
    endforeach()
  endif()

  # Search build aux and use WINDOWS_LIBS if on win32.
  if (WIN32)
    # TODO:
    #   finding libraries:
    #   - for each program use i586-mingw-objdump -p $prog | grep 'DLL Name'.
    #     - has too many, eg mscvrt.dll.
    #   - LINK_INTERFACE_LIBRARIES property
    #     - not sure.

    if (arg_BUILD_AUX_DLLS OR NOT arg_WINDOWS_LIBS)
      file(GLOB extra_libs "${CMAKE_SOURCE_DIR}/build-aux/*.dll")
      if (extra_libs)
        list(APPEND arg_WINDOWS_LIBS ${extra_libs})
      endif()
    endif()

    if (NOT arg_WINDOWS_LIBS)
      if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("butil_auto_install(): no windows binary dlls have been given to install")
      endif()
    else()

      set(names)
      set(winlib)
      foreach(winlib ${arg_WINDOWS_LIBS})
        get_filename_component(winlib "${winlib}" NAME)
        list(APPEND names ${winlib})
      endforeach()
      butil_join(OUTPUT_VAR out LIST_VAR names)
      message(STATUS "butil_auto_install(): will use dlls: ${out}")

      set(names)
      set(out)

      # There's no way to check from within cmake!
      if (arg_HARDLINK_AUX_DLLS)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
          message(STATUS "butil_auto_install(): notice: hardlinking dlls only works if the target is NOT a symlink!")
        endif()
      endif()

      set(depends "DEPENDS")
      foreach(winlib ${arg_WINDOWS_LIBS})
        if (NOT EXISTS "${winlib}")
          message(FATAL_ERROR "butil_auto_install(): specified windows lib ${winlib} does not exist.")
        endif()

        if (arg_HARDLINK_AUX_DLLS)
          find_program(LN_EXE ln)
          find_program(READLINK_EXE readlink)

          mark_as_advanced(LN_EXE READLINK_EXE)

          if (NOT READLINK_EXE)
            set(arg_HARDLINK_AUX_DLLS FALSE)
          endif()

          if (NOT LN_EXE)
            message("butil_auto_install(): warning: bin/ln not found.  Will copy windows dlls to bindir instead.")
            set(arg_HARDLINK_AUX_DLLS FALSE)
          endif()
        endif()

        # Always readlink if we can.  That way the names are right.
        if (READLINK_EXE)
          execute_process(
            COMMAND "${READLINK_EXE}" "-f" "${winlib}"
            OUTPUT_VARIABLE realpath
            RESULT_VARIABLE res
            OUTPUT_STRIP_TRAILING_WHITESPACE
          )

          if (NOT res EQUAL 0)
            message(FATAL_ERROR "butil_auto_install(): readlink failed with code: ${res}.")
          endif()

          set(winlib ${realpath})
        else()
          message("butil_auto_install(): warning: readlink is not found.  If DLLS are symlinks, then packages might contain the wrong DLLname!")
        endif()

        # Calculate this here so that we get the right dll name if the cross
        # compile environment is full of symlinks :)
        get_filename_component(basename "${winlib}" NAME)
        set(output "${CMAKE_BINARY_DIR}/${basename}")

        if (arg_HARDLINK_AUX_DLLS)
          # Don't rename it, or the link will be wrong.
          set(command "${LN_EXE}" "${winlib}")
          set(action "Hard link")
        else()
          # This *does* dereference symlinks however the
          set(command "${CMAKE_COMMAND}" -E copy_if_different "${winlib}" "${CMAKE_BINARY_DIR}")
          set(action "Copy")
        endif()

        add_custom_command(
          OUTPUT  "${output}"
          DEPENDS "${winlib}"
          COMMAND ${command}
          WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
          COMMENT "${action} ${basename} to the binary dir."
          VERBATIM
        )

        list(APPEND depends "${output}")

        install(
          FILES "${output}"
          DESTINATION "${BINDIR}"
        )

      endforeach()

      add_custom_target(update_windows_dlls ALL ${depends})
    endif()
  else()
    set(arg_WINDOWS_LIBS)
  endif()

  if (arg_TARGETS)
    install(
      TARGETS ${arg_TARGETS}
      RUNTIME DESTINATION "${BINDIR}"
      ARCHIVE DESTINATION "${ARCHIVEDIR}"
      LIBRARY DESTINATION "${LIBDIR}"
    )
  endif()

  set(location_prop "LOCATION_${CMAKE_BUILD_TYPE}")

  # Make runnables paths.
  if (arg_RUNNABLES_VAR AND arg_RUNNABLES)
    list(LENGTH arg_RUNNABLES len)
    math(EXPR val "${len} % 2")
    if (NOT val EQUAL 0)
      message(FATAL_ERROR "butil_auto_install(): RUNNABLES must be a list of target, pretty name pairs.")
    endif()

    # TODO: also check that the target is actually a target.
    set(pairs ${arg_RUNNABLES})
    while(pairs)
      list(GET pairs 0 target)
      list(GET pairs 1 name)
      list(REMOVE_AT pairs 0 1)

      get_target_property(path_to_file "${target}" "${location_prop}")

      if (NOT path_to_file)
        message("butil_auto_install(): error: RUNNABLES target '${target}' did not have a location (property ${location_prop}).  Not really a target?")
      else()
        get_filename_component(filename "${path_to_file}" NAME)
        set(installed_path "${BINDIR}/${filename}")
        list(APPEND ${arg_RUNNABLES_VAR} "${installed_path}" "${name}")
      endif()
    endwhile()
  endif()

  # Things to strip
  if (arg_BINARIES_VAR AND arg_TARGETS)
    # TODO:
    #   could use a member property on the lib target here; not sure what's normally
    #   in it.
    set(archive_name "^${CMAKE_STATIC_LIBRARY_PREFIX}.*${CMAKE_STATIC_LIBRARY_SUFFIX}$")
    set(shared_name "^${CMAKE_SHARED_LIBRARY_PREFIX}.*${CMAKE_SHARED_LIBRARY_SUFFIX}$")

    foreach (target ${arg_TARGETS})
      # TODO: what if it's not a target?

      # Note: OUTPUT_NAME doesn't have the prefix/suffix which we need in this case.
      get_target_property(path_to_file ${target} ${location_prop})

      if (NOT path_to_file)
        message("butil_auto_install(): error: target '${target}' did not have a location (property ${location_prop}).  Not really a target?")
      else()

        get_filename_component(filename "${path_to_file}" NAME)

        set(matched_already)
        if (filename MATCHES "${archive_name}")
          list(APPEND ${arg_BINARIES_VAR} "${ARCHIVEDIR}/${filename}")
          set(matched_already TRUE)
        endif()

        if (filename MATCHES "${shared_name}")
          if (matched_already)
            message("butil_auto_install(): warning: '${filename}' matches a .so and a .a.  CPack might not work now...")
          endif()

          # This only gives you *.so for some reason so we have to reconstruct the full
          # binary name.
          get_target_property(version ${target} "VERSION")
          if (version)
            set(version ".${version}")
          endif()

          set(strip_path "${LIBDIR}/${filename}${version}")
          list(APPEND ${arg_BINARIES_VAR} "${strip_path}")
        else()
          list(APPEND ${arg_BINARIES_VAR} "${BINDIR}/${filename}")
        endif()
      endif()
    endforeach()
  endif()
endmacro()

# TODO: probably better in a module called bbuild
macro(butil_find_lib)
  butil_parse_args("VAR;NAMES" "REQUIRED" "" "${ARGV}")

  if (NOT arg_VAR)
    message(FATAL_ERROR "butil_find_lib(): VAR is required.")
  endif()

  if (NOT arg_NAMES)
    set(arg_NAMES ${PA_OTHER})

    if (NOT arg_NAMES)
      message(FATAL_ERROR "butil_find_lib(): NAMES is required.")
    endif()
  endif()

  foreach (butil_name ${arg_NAMES})
    # TODO: implement arg_REQUIRED.
    find_library(${arg_VAR} "${butil_name}")
    mark_as_advanced(${arg_VAR})
    if (${arg_VAR})
      break()
    endif()
  endforeach()

  if (arg_REQUIRED)
    if (NOT ${arg_VAR})
      message(FATAL_ERROR "butil_find_lib(): ${arg_NAMES} were not found.")
    endif()
  endif()
endmacro()


