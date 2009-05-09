# == Overview ==
#
# Doxygen configurator.  Does its best to get a working build of doxygen
# wherever you are.  You should use this to make the developer docs, and
# then CPack to put it in the pakage -- CPack will copy destinations of
# symlinks into the pakcer.
#
# In the case of the source package, then it's up to you to either
# unconditionally install built docs or distributed docs, or some combination.
#
# This file finds() all the necessary rubbish that doxygen needs.
#
# If you want to see what's going on, set DOXYGEN_CMAKE_VERBOSE to a true
# value.
#
# == Quick guide ==
#
#   add_doxygen_directives(
#     TARGET doxygen
#     INPUTS "src/"
#     ARGS_VAR ad_args
#     DOCS_MIRROR "${CMAKE_SOURCE_DIR}/doc"
#     DEFAULT_DOXYFILE "${CMAKE_SOURCE_DIR}/Doxyfile.default"
#     INSTALL ${WANT_DOCS}
#   )
#   add_doxygen(${ad_args})
#
#
# This gives you a bunch of doxygen-related targets, plus the ability to
# either install docs that you distributed or build new ones.  Also note
# that this method allows you to have multiple doxygen builds, provided
# you do not interlace the doxygen_* macros; install needs variables defined
# by flags.
#
# == Quick Warnings ==
#
# Explained fully elsewhere, but just for clarity:
#
# - you can't directly put the outputted HTML dir as an install target; you
#   have to make it a dependancy of ALL with add_custom_target (for some reason)
# - it is a good idea to configure GENERATE_x through cmake, not the doxyfile.
# - watch the console for output from add_doxygen - it will turn certain things
#   off if they cannot be fullfiled and it will tell you so.
# - the macro doxygen_install.. requires some flags set by doxygen_setup...
#   so don't overwrite them until doxygen_install is finished.
#
# == Complete Documentation ==
#
#########################
# FUNCTION: add_doxygen()
#
# add_doxygen(
#   TARGET target
#   DOXYFILE template_file
#   [INPUTS dir...]
#   [OUTPUTS type...]
#   [OVERRIDES "var = value" ...]
#   [INSTALL_FROM directory]
#   [INSTALL_TO directory]
#   [NO_INSTALL]
#   [MAKE_ALL]
# )
#
# This will generate make targets and install targets for a doxygen doc.
#
# OUTPUTS is just an alias for GENERATE_x = YES where add_doxygen() is allowed
# to override it if it cannot be build for some reason (this is a good idea).
#
# Currently outputs may be:
# - html
# - latex (gives you pdf)
# - man (doesn't actually install yet).
#
# OUTPUTS does not accept things which are not directly implemented by
# add_doxygen() since most GENERATE_x require some external programs which must
# be checked for.
#
# Warning: add_doxygen() will write an error message if one of the OUTPUTs
# was turned off because a dependency was missing *however* if you later turn
# it back on in OVERRIDES the message will be invalid.  Therefore it is a *bad
# idea* to put GENERATE_x in the overrides (unless x is not supported).
#
# The special keyword "none" can be used to tell add_doxygen to generate no
# targets and (crucially) to force then GENERATE_x args to NO.
#
# If you do not supply OUTPUT, then GENERATE_x will not be forced - it is
# left to the doxyfile to decide.  Generally this is more difficult to manage,
# so specifying OUTPUT is encouraged.
#
# INPUTS is an alias for a list of INPUTS in Doxyfile.  They are converted to
# be relative to the srcdir; otherwise you get full paths in the documentation.
# They always override what is in the doxyfile.
#
# INSTALL_FROM assumes you have a mirror directory to install from.  The same
# install rules are used # but the documentation is not actually bound to
# them, so you need to respect the output directory conf. values.  As such
# it's a good idea to force them in the directives list.  If you don't then
# add_doxygen() will force its own defaults.  The parameter is is intended for
# when you distribute your built docs in your source tarball so the user
# doesn't have to build them.
#
# INSTALL_TO is the base directory to put installs into, as INSTALL_TO/x_OUTPUT.
# For example doc/html/.  If not set, it uses a sensible default (differeing
# for win32 and unix).
#
# OVERRIDES is a key-value list of things which *always* override the the
# preferences of add_doxygen and the doxyfile.  OVERRIDES can be created be
# created by add_doxygen_directives().
#
# If NO_INSTALL is present, then install targets won't be generated at all.
#
# MAKE_ALL will cause the doxygen targets to be added to the `all' target.
# If INSTALL_FROM is set, then MAKE_ALL defaults to off.
#
# Add doxygen can override any value in the doxyfile, but it will never
# override values in OVERRIDES.  It is easy to make things unbuildable by
# using overrides however; for example if you force GENERATE_LATEX to YES
# but there is no makeindex, the latex output will not be buildable.
#
#################################
# MACRO: add_doxygen_directives()
#
# Populates ${output_var} with a var=val list for use by add_doxygen assuming
# you have several variables defined in advance.
#
# Essentially this is a way of configuring doxygen semi-automatically, and
# with error checking in advance.
#
# It also adds various cache options to help out:
# - WANT_${TARGET} - whether to install docs or now (determines NO_INSTALL
#   in FLAGS_VAR)
# - WANT_${TARGET}_REBUILD - whether to install from DOCS_MIRROR or the binary
#   directory.  Determines MAKE_ALL in FLAGS_VAR.  This option is not generated
#   if
# - ${TARGET}_DOXYFILE - filepath for the template file.  Sets DOXYFILE for
#   add_doxygen args.
# - WANT_${TARGET}_PDF - whether to build/install pdfs - affects OUTPUT.
# - WANT_${TARGET}_HTML - whether to build/install html - affects OUTPUT.
#
# add_doxygen_directives(
#   [ARGS_VAR] args_for_add_doxygen_varnane
#   TARGET target_name
#   [OVERRIDES_VAR overrides_varname]
#   [NO_USER_DOXYFILE]
#   [NO_INSTALL | INSTALL <true|false>]
#   [PDF_DEFAULT <true|false>]
#   [HTML_DEFAULT <true|false>]
#
#   [INPUTS dir...]
#   [VERSION project_version]
#   [PROJECT project_name]
#   [DOCS_MIRROR path]
#   [DEFAULT_DOXYFILE file]
#   [EXAMPLES dir]
#   [SYSTEM]
# )
#
# ARGS_VAR      - arguments which are passed directly to add_doxygen().
# OVERRIDES_VAR - var=val list for add_doxygen OVERRIDES.  If not given, these
#   are added to ARGS_VAR.  This is seperate because sometimes you want to add
#   your own to the list.
#
# TARGET - TARGET argument to add_doxygen.  This is used to determine the name
#   of various options.
# DOCS_MIRROR - mirror directory of docs.  Usually you would write a script to
#   copy the bindir docs into the srcdir before making you package.  If not
#   WANT_DOXYGEN_REBUILD, this sets INSTALL_FROM in the args.
# NO_USER_DOXYFILE - if set, then the ${TARGET}_DOXYFILE option is not set and
#   you must give your own to add_doxygen.
# NO_INSTALL - overrides whatever this macro sets and passes NO_INSTALL to the
#   ARGS_VAR.  This is useful if you have an extra option() which turns off
#   installing if all docs, instead of just pdf, html etc. for this one target.
# INSTALL - an alias for NO_INSTALL so you can use some other variable to
#   determine the value directly.
#
# PDF_DEFAULT - default value for WANT_PDF option
# HTML_DEFAULT - and for html.
#
# VERSION - when not present it uses ${PROJECT_VERSION}.
# PROJECT - when not present it uses ${PROJECT_NAME}
# SYSTEM  - when present the target system name is added to the version.
# DEFAULT_DOXYFILE - sets DOXYFILE in ARGS_VAR if the user didn't specify one.
#   It's also the default for ${TARGET}_DOXYFILE cache option.
# EXAMPLES - if this is set then it is used for the examples path.  By default
#   srcdir/examples is used.  It is OK if this doesn't exist.
# INPUTS - directly alias INPUTS in add_doxygen(), purely for completeness.
#
# Example:
#
#   add_doxygen_directives(
#     ARGS_VAR ad_args
#     TARGET doxygen
#     OVERRIDES_VAR ad_overrides
#     DEFAULT_DOXYFILE ${srcdir}/Doxyfile
#     DOCS_MIRROR ${srcdir}/doc/doxygen/
#     INSTALL ${WANT_DOCS}
#   )
#
#   add_doxygen(
#     INPUTS "include"
#     OVERRIDES ${ad_overrides}
#     ${ad_args}
#   )


include("${CMAKE_SOURCE_DIR}/build-aux/butil.cmake")

find_package(Doxygen)
find_package(LATEX)

find_program(CAT_EXE cat)
if (NOT CAT_EXE)
  find_program(CAT_EXE type)
endif()

find_program(MAKE_EXECUTABLE make)
if (NOT MAKE_EXECUTABLE)
  find_program(MAKE_EXECUTABLE gmake)
  if (NOT MAKE_EXECUTABLE)
    find_program(MAKE_EXECUTABLE nmake)
  endif()
endif()
mark_as_advanced(CAT_EXE MAKE_EXECUTABLE)

# Parse a "var = value" list and turns them into cmake vars UNLESS they are blank.
# Names of variables found get put in vars_found if it is given.
macro(add_doxygen_parse_conf_list var_vals vars_found)
  foreach (vv ${var_vals})
    string(REGEX REPLACE "[\t\n ]*([A-Z0-9_a-z]+)[\t ]*=.*" "\\1" var "${vv}")
    # TODO:
    #   this regexp is wrong in the case that we have multiple values specified
    #   like x = "v1" "v2"  I guess I need
    #
    #     "[^=]*=[\t ]*([^#]+).*"
    #
    string(REGEX REPLACE "[\t\n ]*[A-Z0-9_a-z]+[\t ]*=[\t\n ]*((\"[^\"]*\")|([^\t\n ]*))[\t\n ]*" "\\1" val "${vv}")
    string(TOUPPER "${var}" var)
    if (NOT vars_found STREQUAL "")
      list(APPEND ${vars_found} "${var}")
    endif()

    if (NOT "${val}" STREQUAL "")
      add_doxygen_override_var("${var}" "${val}")
    endif()
  endforeach()
endmacro()

# Sets each var = val to a conf_${var} = ${val}
macro(add_doxygen_parse_conf template_file)
  file(READ ${template_file} file)

  string(
    REGEX MATCHALL "(^|\n)[A-Za-z_0-9]+[^=]*=[\t ]*[^#]*(\n|$)"
    var_vals ${file}
  )

  add_doxygen_parse_conf_list("${var_vals}" "")
endmacro()

# Reset a var and warn that we are doing it if it is defined and not blank.
macro(add_doxygen_override_var varname value)
  set(var "conf_${varname}")
  if (DOXYGEN_CMAKE_VERBOSE)
    if (DEFINED ${var})
      if (NOT ${var} STREQUAL "${value}")
        message(STATUS "add_doxygen(): warning: overriding var ${varname}: '${${var}}' with '${value}'.")
      endif()
    endif()
  endif()
  set(${var} "${value}")
endmacro()

# Retroativlely set a default.  This way the conf_list function won't print
# warnings for overriding default values (which is totally Ok)
macro(add_doxygen_set_ifndef varname value)
  if (NOT DEFINED conf_${varname})
    set(conf_${varname} ${value})
  endif()
endmacro()

# Override the union of the vars given as option arguments.
function(add_doxygen_write_overrides file)
  set(vars ${ARGV})
  list(REMOVE_AT vars 0 "${vars}")
  foreach (var ${vars})
    if (NOT visited_${var})
      set(visited_${var} TRUE)
      set(value ${conf_${var}})
      file(APPEND ${file} "${var} = ${value}\n")
    endif()
  endforeach()
endfunction()

#
macro(add_doxygen_check_var varname reserve_varname)
  if (NOT DEFINED ${varname})
    if (NOT DEFINED ${other_var})
      message(FATAL_ERROR "Missing required argument '${varname}'.")
    else()
      set(${varname} ${other_var})
    endif()
  endif()
endmacro()

# Sets variables for a subsequent call to add_doxygen.
macro(add_doxygen_directives)
  # TODO:
  #   add the ability to specify defaults for the option()'s.
  butil_parse_args(
    "ARGS_VAR;TARGET;OVERRIDES_VAR;VERSION;PROJECT;DOCS_MIRROR;DEFAULT_DOXYFILE;EXAMPLES;INSTALL;INPUTS;PDF_DEFAULT;HTML_DEFAULT"
    "NO_USER_DOXYFILE;SYSTEM;NO_INSTALL"
    ""
    "${ARGV}"
  )

  if (arg_INSTALL AND arg_NO_INSTALL)
    message(FATAL_ERROR "add_doxygen_directives(): INSTALL TRUE and NO_INSTALL may not be set together.")
  endif()

  if (NOT arg_ARGS_VAR)
    set(arg_ARGS_VAR ${ARGV0})
  endif()

  if (NOT arg_ARGS_VAR)
    message(FATAL_ERROR "add_doxygen_directives(): ARGS_VAR argument is required.")
  endif()

  if (NOT arg_TARGET)
    message(FATAL_ERROR "add_doxygen_directives(): TARGET argument is required.")
  endif()

  list(APPEND ${arg_ARGS_VAR} "TARGET" "${arg_TARGET}")

  if (NOT arg_PROJECT)
    set(arg_PROJECT "${CMAKE_PROJECT_NAME}")
  endif()

  if (NOT arg_VERSION)
    set(arg_VERSION "${PROJECT_VERSION}")
  endif()

  if (arg_SYSTEM)
    set(arg_VERSION "${arg_VERSION}-${CMAKE_SYSTEM_NAME}")
  endif()

  set(overrides)
  list(APPEND overrides "PROJECT_NAME = ${arg_PROJECT}")
  list(APPEND overrides "PROJECT_NUMBER = ${arg_VERSION}")

  if (DEFINED arg_EXAMPLES)
    list(APPEND overrides "EXAMPLE_PATH = ${arg_EXAMPLES}")
  else()
    list(APPEND overrides "EXAMPLE_PATH = ${CMAKE_SOURCE_DIR}/examples")
  endif()

  if (arg_INPUTS)
    list(APPEND ${arg_ARGS_VAR} "INPUTS" ${arg_INPUTS})
  endif()

  if (arg_NO_INSTALL OR NOT arg_INSTALL)
    message(STATUS "${arg_TARGET}: install targets will be disabled.")
    list(APPEND ${arg_ARGS_VAR} NO_INSTALL)
  else()
    message(STATUS "${arg_TARGET}: install targets are enabled.")
  endif()

  string(TOUPPER "${arg_TARGET}" target_upcase)
  set(pdf_cachevar WANT_${target_upcase}_PDF)
  set(html_cachevar WANT_${target_upcase}_HTML)
  set(rebuild_cachevar "WANT_${target_upcase}_REBUILD")
  set(doxyfile_cachevar "${target_upcase}_DOXYFILE_TEMPLATE")

  option(${rebuild_cachevar} "Regenerate doxygen and install that instead of the distributed docs." NO)

  # Be specific.
  if (arg_PDF_DEFAULT)
    set(val "YES")
  else()
    set(val "NO")
  endif()
  option(${pdf_cachevar} "Install PDF from doxygen (needs pdfLaTeX)" ${val})

  if (arg_HTML_DEFAULT)
    set(val "YES")
  else()
    set(val "NO")
  endif()
  option(${html_cachevar} "Install HTML made by doxygen" ${val})

  mark_as_advanced(
    ${rebuild_cachevar}
    ${pdf_cachevar}
    ${html_cachevar}
  )

  set(add_empty_string TRUE)
  list(APPEND ${arg_ARGS_VAR} OUTPUTS)

  if (${pdf_cachevar})
    message(STATUS "${arg_TARGET}: LaTeX is on.")
    set(add_empty_string FALSE)
    list(APPEND ${arg_ARGS_VAR} "latex")
  endif()

  if (${html_cachevar})
    message(STATUS "${arg_TARGET}: HTML is on.")
    set(add_empty_string FALSE)
    list(APPEND ${arg_ARGS_VAR} "html")
  endif()

  # Bit of a hack.  We need outputs set to *something* or it won't work :)
  if (add_empty_string)
    list(APPEND ${arg_ARGS_VAR} "none")
  endif()

  if (${rebuild_cachevar})
    message(STATUS "${arg_TARGET}: dox will rebuild with make all.")
    list(APPEND ${arg_ARGS_VAR} MAKE_ALL)
  else()
    message(STATUS "${arg_TARGET}: dox will not rebuild with make all.")
  endif()

  if (arg_OVERRIDES_VAR)
    set(${arg_OVERRIDES_VAR} ${overrides})
  else()
    list(APPEND ${arg_ARGS_VAR} OVERRIDES ${overrides})
  endif()

  if (arg_DEFAULT_DOXYFILE)
    set(default_dox "${CMAKE_SOURCE_DIR}/Doxyfile.default")
  else()
    set(default_dox "${arg_DEFAULT_DOXYFILE}")
  endif()

  if (arg_DOCS_MIRROR AND NOT ${rebuild_cachevar})
    message(STATUS "${arg_TARGET}: install will come from ${arg_DOCS_MIRROR} (if enabled).")
    list(APPEND ${arg_ARGS_VAR} INSTALL_FROM "${arg_DOCS_MIRROR}")
  endif()

  if (arg_NO_USER_DOXYFILE)
    list(APPEND ${arg_ARGS_VAR} DOXYFILE "${default_dox}")
  else()
    set("${doxyfile_cachevar}" "${default_dox}" CACHE PATH
      "Base doxyfile.  Mostly useful for developers.  Certain vars will be overwritten by cmake (in/output paths mostly).")

    message(STATUS "${arg_TARGET}: doxyfile selected: '${${doxyfile_cachevar}}'")

    mark_as_advanced(${doxyfile_cachevar})
    list(APPEND ${arg_ARGS_VAR} DOXYFILE "${${doxyfile_cachevar}}")
  endif()

  # message("${${arg_ARGS_VAR}}")
  # message("${${arg_OVERRIDES_VAR}}")
endmacro()

# Create doxygen targets based on $target_name and the doxyfile $template_file.
# $directives_list overrides values from the template_file.
function(add_doxygen target_name template_file directives_list)
  # TODO: remove these args when I'm done porting.

  # TODO:
  #   Update todos for changes based on 1.5.8 doxyfile:
  #
  #   git diff eeff48fe0c426b47ec67086544e391ee545ea431..772a7796fe54fb539a7f3d6d4c7acabdbb9e44ba  Doxyfile.default
  #
  #   Some new generates are added, lots of new dependency type files.

  # TODO:
  #   Handle confs relevant for dependencies.  Most of these are just for
  #   erroring when it's not present instead of waiting for doxygen to do it.
  #   Since it should all build with make all, there's not much point in
  #   putting rebuild depends.
  #
  # - MSCGEN_PATH
  # - RTF_STYLESHEET_FILE
  # - RTF_EXTENSIONS_FILE
  # - HTML_STYLESHEET - we must simply check if it exists I think... perhaps also move it
  #   into the dir?  Don't know what dooxygen does with it.
  # - HTML_HEADER - main target must depend on this
  # - HTML_FOOTER - and this
  # - LATEX_HEADER - latex target must depend on this (so main, and make pdf)

  # TODO:
  # more options to add support for:
  #
  # - GENERATE_RTF , RTF_OUTPUT
  # - GENERATE_HTMLHELP , CHM_FILE
  #   - GENERATE_CHI (means a seperate file comes with HTMLHELP)
  # - GENERATE_AUTOGEN_DEF
  # - GENERATE_DOCSET (something to do with xcode `doxygen will generate a Makefile
  #   in the HTML output directory'.  It has a make install)
  #   - maybe force this off since I don't know how it works.
  # - GENERATE_XML ,XML_OUTPUT

  butil_parse_args(
    "TARGET;DOXYFILE;INPUTS;OUTPUTS;OVERRIDES;INSTALL_FROM;INSTALL_TO"
    "MAKE_ALL;NO_INSTALL"
    ""
    "${ARGV}"
  )

  if (arg_TARGET)
    set(target_name ${arg_TARGET})
  elseif(NOT target_name)
    message(FATAl_ERROR "doxygen_add(): requires a TARGET name.")
  endif()

  message(STATUS "Adding doxygen target ${target_name} from template file ${template_file}.")

  if (arg_DOXYFILE)
    set(template_file ${arg_DOXYFILE})
  elseif(NOT template_file)
    message(FATAl_ERROR "doxygen_add(): requires a DOXYFILE name.")
  endif()

  if (arg_OVERRIDES)
    set(directives_list ${arg_OVERRIDES})
  endif()

  # For compatibility we don't fail here
  if (NOT arg_OUTPUTS)
    message(STATUS "add_doxygen(): warning: no outputs selected.")
  endif()

  if (NOT DOXYGEN_EXECUTABLE)
    message(STATUS "Ignoring doxygen targets due to no doxygen exe.")
    return()
  endif()

  # The full conf which doxygen will use.
  set(doxygen_conf_file "${CMAKE_BINARY_DIR}/${target_name}-doxyfile-generated")
  # Forced values which we make the full conf from.
  set(additions_file "${CMAKE_BINARY_DIR}/${target_name}-doxyfile-forced")

  if (DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "Doxygen forced vars are in ${additions_file}.")
  endif()

  # Stuff which the configuration means we have to force (will be generated as we go)
  set(extra_force)

  # Stuff that we must always force.
  set(always_force
    MAKEINDEX_CMD_NAME LATEX_CMD_NAME DOT_PATH
    OUTPUT_DIRECTORY
    HTML_OUTPUT LATEX_OUTPUT MAN_OUTPUT
    USE_PDFLATEX LATEX_BATCHMODE
    INPUT
  )

  if (DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "add_doxygen(): parsing configuration file.")
  endif()
  add_doxygen_parse_conf(${template_file})

  # We force these vars so it's easier to install but the user can still override them
  # if necessary.
  if (DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "add_doxygen(): overriding with doxygen.cmake's forced values.")
  endif()
  add_doxygen_override_var(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${target_name}")
  add_doxygen_override_var(HTML_OUTPUT      "html")
  add_doxygen_override_var(LATEX_OUTPUT     "latex")
  add_doxygen_override_var(MAN_OUTPUT       "man")
  # This is forced because otherwise the makefile blocks forever if there is ever
  # some kind of error.
  add_doxygen_override_var(LATEX_BATCHMODE  "YES")
  # TODO: here override extensions and so on - anything else which affects paths.

  # User overides which we are allowed to override right back if we want :).
  # For compatibility, we allow the doxyfile to specify this.

  if (arg_OUTPUTS)
    # TODO: validate these names.
    set(is_none FALSE)
    foreach (o ${arg_OUTPUTS})
      if (o STREQUAL "none")
        if(is_none)
          message(FATAL_ERROR "add_doxygen(): OUTPUT contains `none' as well as other values.")
        endif()
        set(is_none TRUE)
      endif()
      string(TOUPPER ${o} o)
      set(out_${o} "YES")
    endforeach()

    if (DOXYGEN_CMAKE_VERBOSE AND is_none)
      message(STATUS "add_doxygen(): OUTPUTS have the value \"none\" - everthing will be forced off.")
    endif()

    set(possible_outs "HTML" "MAN" "LATEX")
    foreach (o ${possible_outs})
      list(APPEND extra_force GENERATE_${o})
      if (is_none)
        add_doxygen_override_var(GENERATE_${o} "NO")
      elseif (out_${o})
        add_doxygen_override_var(GENERATE_${o} "YES")
      else()
        add_doxygen_override_var(GENERATE_${o} "NO")
      endif()
    endforeach()
  endif()

  if (DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "add_doxygen(): overriding confs based on external programs.")
  endif()

  # This allows the user to set HAVE_DOT
  if (NOT DOXYGEN_DOT_PATH)
    list(APPEND extra_force "HAVE_DOT")
    if (conf_HAVE_DOT)
      message("add_doxygen(): ${target_name}: warning: HAVE_DOT was on but no dot exe could be found.  HAVE_DOT will be forced off.")
    endif()
    add_doxygen_override_var(HAVE_DOT  "NO")
  else()
    # Actually this is usually just /usr/bin
    add_doxygen_override_var(DOT_PATH "${DOXYGEN_DOT_PATH}")
  endif()

  set(original_cmd_conf "${conf_LATEX_CMD_NAME}")
  if (LATEX_COMPILER AND PDFLATEX_COMPILER)
    # We still need to force PDFLATEX later or the cmd name will be wrong.
    if (conf_USE_PDFLATEX OR NOT DEFINED conf_USE_PDFLATEX OR conf_USE_PDFLATEX STREQUAL "")
      add_doxygen_override_var(USE_PDFLATEX    "YES")
      add_doxygen_override_var(LATEX_CMD_NAME  "${PDFLATEX_COMPILER}")
    else()
      add_doxygen_override_var(USE_PDFLATEX    "NO")
      add_doxygen_override_var(LATEX_CMD_NAME  "${LATEX_COMPILER}")
    endif()
  elseif(PDFLATEX_COMPILER)
    add_doxygen_override_var(USE_PDFLATEX    "YES")
    add_doxygen_override_var(LATEX_CMD_NAME  "${PDFLATEX_COMPILER}")
  elseif(LATEX_COMPILER)
    add_doxygen_override_var(USE_PDFLATEX    "NO")
    add_doxygen_override_var(LATEX_CMD_NAME  "${LATEX_COMPILER}")
  else()
    if (conf_GENERATE_LATEX)
      message("add_doxygen(): ${target_name}: error: LaTeX was on but no LaTeX compilers could be found.  LaTeX will be forced off.")
    endif()
    add_doxygen_override_var(GENERATE_LATEX  "NO")
    list(APPEND extra_force "GENERATE_LATEX")
  endif()

  if (NOT original_cmd_conf STREQUAL "${conf_LATEX_CMD_NAME}")
    message(STATUS "add_doxygen(): ${target_name}: automatically selected ${conf_LATEX_CMD_NAME} to build LaTeX.")
    message(STATUS "               Set the doxyfile variables LATEX_COMPILER and/or PDFLATEX_COMPILER and")
    message(STATUS "               USE_PDFLATEX to override.")
  endif()

  # This bit is interesting because it implies that the template file might be allowed
  # to make GENERATE_x overrides.  Certainly it seems possible.
  if (MAKEINDEX_COMPILER)
    add_doxygen_override_var(MAKEINDEX_CMD_NAME "${MAKEINDEX_COMPILER}")
  else()
    if (conf_GENERATE_LATEX)
      message("add_doxygen(): ${target_name}: warning: LaTeX was on but no makeindex could be found.  LaTeX will be forced off.")
    endif()
    add_doxygen_override_var(GENERATE_LATEX  "NO")
    list(APPEND extra_force "GENERATE_LATEX")
  endif()

  # TODO:
  #   LaTeX graphics.  We need to force a bunch of options if there is no
  #   suitable graphics conversion program.  Eps2pdf is one of the files
  #   that we need.  Also dvips is referenced.
  #   - HAVE_DOT, CLASS_GRAPH, COLLABORATION_GRAPH, GROUP_GRAPHS, INCLUDED_BY_GRAPH, INCLUDE_GRAPH...
  #   - ehh... too much.  Just have-dot= off will do.
  #   - then CLASS_DIAGRAMS (class diagrams is a fallback case) and HAVE_DOT = off
  #   - also need to warn that NO graphics is on when LaTeX is emabled or the user
  #     won't know wtf.

  # if (NOT EPS2PDF_EXE)
  #   if (conf_ENABLE_LATEX)
  #     if (conf_HAVE_DOT OR conf_CLASS_DIAGRAMS)
  #       message(STATUS "add_doxygen(): warning: no eps2pdf.  Graphial output is disabled unless LaTeX is turned off.")
  #     endif()
  #
  #     force have_dot = NO
  #     force CLASS_DIAGRAMS = NO
  #   endif()
  # endif()
  #


  # TODO:
  #   There are other issues with latex output.  Mostly that pdflatex will
  #   *only* put out a pdf.  It's only if we have other values that it works
  #   with pdf and dvi.


  # This is of course allowed to override the stuff we already though was sensible.
  if (DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "add_doxygen(): overriding with the user's forced values.")
  endif()
  add_doxygen_parse_conf_list("${directives_list}" overridden_vars)

  if (NOT IS_ABSOLUTE "${conf_OUTPUT_DIRECTORY}")
    add_doxygen_override_var(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${conf_OUTPUT_DIRECTORY}")
  endif()

  # We got this as an argument, bloody ages ago :)
  if (arg_INPUTS)
    set(input_over)
    foreach (i ${arg_INPUTS})
      set(input_over "${input_over} \"${i}\"")
    endforeach()

    if (DOXYGEN_CMAKE_VERBOSE)
      message(STATUS "add_doxygen(): input from INPUT argument: ${conf_INPUT}")
    endif()

    add_doxygen_override_var(INPUT "${input_over}")
    list(APPEND extra_force "INPUT")
  elseif(DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "add_doxygen(): input from doxyfile template: ${conf_INPUT}")
  endif()

  # Retroactively set the defaults now to avoid getting warnings earlier.
  add_doxygen_set_ifndef(GENERATE_LATEX  "YES")
  add_doxygen_set_ifndef(GENERATE_HTML   "YES")
  add_doxygen_set_ifndef(GENERATE_MAN    "NO")
  add_doxygen_set_ifndef(USE_PDFLATEX    "YES")
  add_doxygen_set_ifndef(INPUT           "include/")

  if (NOT conf_GENERATE_HTML AND NOT conf_GENERATE_MAN AND NOT conf_GENERATE_LATEX)
    message(STATUS "add_doxygen(): warning: doxygen doesn't seem to be set to generate anything.")
  endif()

  if (NOT conf_INPUT)
    message(FATAL_ERROR "add_doxygen(): no INPUT paths given.")
  endif()

  # TODO:
  #   here override conf_INPUT to be relative to the srcdir.
  #   Tricky because there can be multiple of them.
  # string(REGEX MATCHALL "(\"[^\"]+\")|([^ ]+)" inputs "${conf_INPUT}")
  # foreach (i ${inputs})
  #   string(REGEX REPLACE "\"" "" i ${i})
  #   set relative to srcdir.
  #   append to another list like \"${i}\"
  #   message("${i}")
  # endforeach()
  # add_doxygen_override_var(INPUT  "")

  # Write the additions file with the things we must force if they are set.
  if (DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "add_doxygen(): writing conf force file: ${additions_file}.")
  endif()
  file(WRITE ${additions_file} "")

  # Force the union of all these vars.
  add_doxygen_write_overrides(
    "${additions_file}"
    ${always_force}
    ${extra_force}
    ${overridden_vars}
  )

  if (DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "add_doxygen(): generate latex: ${conf_GENERATE_LATEX}")
    message(STATUS "add_doxygen(): generate html:  ${conf_GENERATE_HTML}")
    message(STATUS "add_doxygen(): generate man:   ${conf_GENERATE_MAN}")
  endif()

  # TODO: add this lot to another function - much easier to read :)
  set(absolute_doxygen_path "${conf_OUTPUT_DIRECTORY}")
  string(REGEX REPLACE "\\/[\\/]+" "/" absolute_doxygen_path ${absolute_doxygen_path})

  # TODO: how does doxygen deal with a complete path for, eg latex?  Is it allowed?
  #   If so we must detect it.

  # Configure i/o paths.

  # This always invalid output lets us depend on the main target and have the
  # main target always rebuild.  Normally it's not possible to have an always
  # invalid target which a normal target depends on unless they are both top-
  # level targets
  # TODO:
  #   mark this target phony.  It works fine, but maybe it will break
  #   other generators.
  string(RANDOM LENGTH 32 rand)
  set(main_output  "${target_name}/${rand}")
  set(latex_output "${absolute_doxygen_path}/${conf_LATEX_OUTPUT}")
  set(html_output  "${absolute_doxygen_path}/${conf_HTML_OUTPUT}")
  set(man_output   "${absolute_doxygen_path}/${conf_MAN_OUTPUT}")

  set(pdf_output  "${latex_output}/refman.pdf")
  set(ps_output   "${latex_output}/refman.ps")
  set(dvi_output  "${latex_output}/refman.dvi")

  if (DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "add_doxygen(): adding doxygen output commands.")
  endif()


  # Applies to whatever the cwd is .
  set_property(
    DIRECTORY APPEND
    PROPERTY "ADDITIONAL_MAKE_CLEAN_FILES"
    ${additions_file} ${doxygen_conf_file} ${absolute_doxygen_path}
  )

  # Finally we can get going with the targets.
  # Initial builder target:
  add_custom_command(
    OUTPUT  "${additions_file}"
    DEPENDS "${template_file}"
    COMMAND ${CMAKE_COMMAND} .
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
    COMMENT "Regenerate build system for ${additions_file}."
  )

  # Make the generated doxyfile.
  add_custom_command(
    OUTPUT ${doxygen_conf_file}
    COMMAND ${CAT_EXE} ${template_file} > ${doxygen_conf_file}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CAT_EXE} ${additions_file} >> ${doxygen_conf_file}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS "${template_file}" "${additions_file}"
    COMMENT "Generating doxygen config file based on the template and forced values."
    VERBATIM
  )

  # Always invalid target to call doxygen.
  add_custom_command(
    OUTPUT  "${main_output}"
    COMMAND "${DOXYGEN_EXECUTABLE}" "${doxygen_conf_file}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${doxygen_conf_file}
    COMMENT "Generating main doxygen (html, .tex files ...)"
    VERBATIM
  )

  add_custom_target(
    ${target_name}_main
    DEPENDS "${main_output}"
    COMMENT "Generating HTML, LaTeX, man..."
  )

  # Deal with LaTeX outputs.
  if (conf_GENERATE_LATEX AND NOT MAKE_EXECUTABLE)
    message(STATUS "Make exe can't be found: doxygen LaTeX is not buildable by cmake.")
  elseif (conf_GENERATE_LATEX)
    # TODO:
    #   if not conf_USE_PDFLATEX then we have a bunch of other targets, but I
    #   would have to emulate themin that ase.

    # Retroactively add the latex output to the main target.
    add_custom_command(
      OUTPUT  "${latex_output}"
      DEPENDS "${main_output}"
    )

    add_custom_command(
      OUTPUT "${pdf_output}"
      COMMAND ${MAKE_EXECUTABLE} "pdf"
      WORKING_DIRECTORY "${latex_output}"
      DEPENDS "${latex_output}"
      COMMENT "Calling doxygen's generated makefile for pdf (this is error prone!)."
      VERBATIM
    )

    add_custom_target(
      "${target_name}_pdf" DEPENDS  ${pdf_output}
    )
  endif()

  set(makeall)
  if (arg_MAKE_ALL)
    if (DOXYGEN_CMAKE_VERBOSE)
      message(STATUS "Doxygen targets will be made with make all.")
    endif()
    set(makeall ALL)
  endif()

  # Finally the _all target
  set(deps ${main_output})
  if (conf_GENERATE_LATEX)
    list(APPEND deps ${pdf_output})
  endif()
  add_custom_target(${target_name}_all ${makeall} DEPENDS ${deps})

  # And now add install targets.
  if (NOT arg_NO_INSTALL)
    # TODO:
    #   won't work if the slashes are wrong or they're not correctly relative... need an
    #   is_the_same_file.
    if (NOT arg_INSTALL_FROM STREQUAL "${conf_OUTPUT_DIRECTORY}")
      set(from_bindir FALSE)
    else()
      set(from_bindir TRUE)
    endif()

    if (NOT arg_INSTALL_FROM OR from_bindir)
      if (NOT arg_MAKE_ALL)
        message(STATUS "add_doxygen(): warning: doxygen is not building with make all, but install from is the built docs.  Build must be done manually *before* install.")
      endif()
    endif()

    if (arg_INSTALL_FROM)
      set(inst_from ${arg_INSTALL_FROM})
    else()
      set(inst_from ${conf_OUTPUT_DIRECTORY})
    endif()

    if (arg_INSTALL_TO)
      set(inst_to "${arg_INSTALL_TO}")
    else()
      message(STATUS "add_doxygen(): warning: using a default value for install location.")
      if (UNIX)
        set(inst_to "share/doc/${CMAKE_PROJECT_NAME}/")
      else()
        # On win, the installer is rooted in its own directory.
        set(inst_to "doc")
      endif()
    endif()

    if (DOXYGEN_CMAKE_VERBOSE)
      message(STATUS "add_doxygen(): doxygen will install from ${inst_from}.")
      message(STATUS "add_doxygen(): doxygen will install to ${inst_to}.")
    endif()

    # Ensure the last dir HAS got a slash on the end, otherwise we end up
    # copying the entire dir instead of just the contents.
    set(file "${inst_from}/${conf_HTML_OUTPUT}/")
    set(inst "${inst_to}")

    if (conf_GENERATE_HTML)
      # TODO: what if have_dot is off?
      if (conf_DOT_IMAGE_FORMAT)
        set(image_fmt_pat "*.${conf_DOT_IMAGE_FORMAT}")
      else()
        message("add_doxygen(): ${target_name}: warning: no DOT_IMAGE_FORMAT supplied; guessing png.")
        set(image_fmt_pat "*.png")
      endif()

      if (conf_DOT_IMAGE_FORMAT)
        # this conf includes the period.
        set(html_pat "*${conf_HTML_FILE_EXTENSION}")
      else()
        message("add_doxygen(): ${target_name}: warning: no HTML_FILE_EXTENSION supplied; guessing html.")
        set(html_pat "*.html")
      endif()

      install(
        DIRECTORY   "${file}"
        DESTINATION "${inst}/${conf_HTML_OUTPUT}"
        FILES_MATCHING
        PATTERN "${image_pat}"
        PATTERN "${html_ext}"
        PATTERN "*.css"
        PATTERN "*.gif"
      )
    endif()

    if (conf_GENERATE_LATEX)
      set(file "${inst_from}/${conf_LATEX_OUTPUT}/refman.pdf")
      set(inst "${inst_to}/${conf_LATEX_OUTPUT}")
      install(
        FILES "${file}"
        DESTINATION "${inst}"
      )
    endif()

    # TODO: add manpages.

  elseif (DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "add_doxygen(): not adding install targets.")
  endif()
endfunction()

# Set up a load of variables for a later call to doxygen_install.
macro(doxgyen_setup_flags flags_var target wants)
  message(STATUS "Setting doxygen flags.")
  message("doxgyen_setup_flags(): this function is deprected")

  set(DOXYGEN_OUT_DIR "${CMAKE_BINARY_DIR}/${target}")

  set(DOXYGEN_REL_HTML_DIR "html")
  set(DOXYGEN_REL_LATEX_DIR "latex")
  set(DOXYGEN_REL_PDF_FILE "latex/refman.pdf")

  set(DOXYGEN_HTML_DIR "${DOXYGEN_OUT_DIR}/${DOXYGEN_REL_HTML_DIR}")
  set(DOXYGEN_LATEX_DIR "${DOXYGEN_OUT_DIR}/${DOXYGEN_REL_LATEX_DIR}")
  set(DOXYGEN_PDF_FILE "${DOXYGEN_OUT_DIR}/${DOXYGEN_REL_PDF_FILE}")

  if (DOXYGEN_EXAMPLE_PATH)
    list(APPEND ${flags_var} "EXAMPLE_PATH = ${DOXYGEN_EXAMPLE_PATH}/")
  else()
    list(APPEND ${flags_var} "EXAMPLE_PATH = ${CMAKE_SOURCE_DIR}/examples/")
  endif()

  if (DOXYGEN_INPUT_PATHS)
    list(APPEND ${flags_var} "INPUT = ${DOXYGEN_INPUT_PATHS}")
  else()
    list(APPEND ${flags_var} "INPUT = ${CMAKE_SOURCE_DIR}/include/")
  endif()

  if (PROJECT_NAME)
    list(APPEND ${flags_var} "PROJECT_NAME = ${PROJECT_NAME}")
  endif()
  if (PROJECT_VERSION)
    list(APPEND ${flags_var} "PROJECT_NUMBER = ${PROJECT_VERSION}")
  endif()

  list(APPEND ${flags_var} "OUTPUT_DIRECTORY = ${DOXYGEN_OUT_DIR}")

  foreach(iter ${wants})
    if (${iter} MATCHES "pdf")
      list(APPEND ${flags_var} "GENERATE_LATEX = yes")
      list(APPEND ${flags_var} "LATEX_OUTPUT = ${DOXYGEN_REL_LATEX_DIR}")
    elseif(${iter} MATCHES "html")
      list(APPEND ${flags_var} "GENERATE_HTML = yes")
      list(APPEND ${flags_var} "HTML_OUTPUT = ${DOXYGEN_REL_HTML_DIR}")
    else()
      message(FATAL_ERROR "doxygen_setup_flags(): unknown type of want '${iter}'")
    endif()
  endforeach()

  if (DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "doxgyen_setup_flags() finished, set values:")
    message(STATUS "- DOXYGEN_OUT_DIR = ${DOXYGEN_OUT_DIR}")

    message(STATUS "- DOXYGEN_REL_HTML_DIR = ${DOXYGEN_REL_HTML_DIR}")
    message(STATUS "- DOXYGEN_REL_LATEX_DIR = ${DOXYGEN_REL_LATEX_DIR}")
    message(STATUS "- DOXYGEN_REL_PDF_FILE = ${DOXYGEN_REL_PDF_FILE}")

    message(STATUS "- DOXYGEN_HTML_DIR = ${DOXYGEN_HTML_DIR}")
    message(STATUS "- DOXYGEN_LATEX_DIR = ${DOXYGEN_LATEX_DIR}")
    message(STATUS "- DOXYGEN_PDF_FILE = ${DOXYGEN_PDF_FILE}")
  endif()
endmacro()

function(doxygen_install_targets doxygen_target wants install_to install_docs_from)
  message("doxygen_install_targets(): this function is deprected")
  message(STATUS "Adding doxygen make install targets.")

  # One hour of trial and error tells me that yes, you really do need to test it with
  # the 'x' in front.
  if ("x${install_docs_from}" STREQUAL "x")
    set(install_from "${DOXYGEN_OUT_DIR}")
    set(rebuild TRUE)
  else()
    set(install_from "${install_docs_from}")
    set(rebuild FALSE)
  endif()

  if (DOXYGEN_CMAKE_VERBOSE)
    message(STATUS "Details of doxygen install:")
    message(STATUS "* wants: '${wants}'")
    message(STATUS "* base target: '${doxygen_target}'")
    message(STATUS "* install from: '${install_from}'")
    message(STATUS "* install to: '${install_to}'")
    message(STATUS "* rebuild: ${rebuild}")
  endif()

  foreach(iter ${wants})
    if (iter MATCHES "pdf")
      set(file "${install_from}/${DOXYGEN_REL_PDF_FILE}")
      set(inst "${install_to}/${DOXYGEN_REL_LATEX_DIR}")

      if (rebuild)
        add_custom_target(depend_${doxygen_target}_pdf ALL DEPENDS "${DOXYGEN_PDF_FILE}")
      endif()

      install(
        FILES "${file}"
        DESTINATION "${inst}"
      )
    elseif(iter MATCHES "html")
      set(file "${install_from}/${DOXYGEN_REL_HTML_DIR}")
      set(inst "${install_to}")

      if (rebuild)
        add_custom_target(depend_${doxygen_target}_html ALL DEPENDS "${DOXYGEN_HTML_DIR}")
      endif()

      install(
        DIRECTORY "${file}"
        DESTINATION ${inst}
        FILES_MATCHING
        PATTERN "*.png"
        PATTERN "*.html"
        PATTERN "*.css"
        PATTERN "*.gif"
      )
    else()
      message(FATAL_ERROR "doxygen_install_targets(): unknown type of want '${iter}'")
    endif()

  endforeach()

endfunction()

