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
#   option(WANT_DOXYGEN ...
#   option(WANT_DOXYGEN_REBUILD...
#   if (WANT_DOXYGEN)
#     set(DOXYGEN_WANTS "pdf" "html")
#   endif()
#
#   set(DOXYGEN_TARGET "doxygen")
#   doxygen_setup_flags(DOXYGEN_FLAGS "${DOXYGEN_TARGET}" "${DOXYGEN_WANTS}")
#   add_doxygen("${DOXYGEN_TARGET}" "${CMAKE_SOURCE_DIR}/Doxyfile.base"
#               "${DOXYGEN_FLAGS}")
#
#   if (WANT_DOXYGEN_REBUILD)
#     set(INSTALL_FROM "")
#   else()
#     set(INSTALL_FROM "${CMAKE_SOURCE_DIR}/doc/")
#   endif()
#
#   doxygen_install_targets(
#     "${DOXYGEN_TARGET}" 
#     "${DOXYGEN_WANTS}" 
#     "${DOCDIR}" 
#     "${INSTALL_FROM}"
#   )
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
# - watch the console for output from add_doxygen - it will turn certain things
#   off if they cannot be fullfiled and it will tell you so.
# - the macro doxygen_install.. requires some flags set by doxygen_setup...
#   so don't overwrite them until doxygen_install is finished. 
# 
# == Complete Documentation ==
#
# FUNCTION: 
# * add_doxygen(target_name template_file directives_list)
# 
#   Make ${target_name}, and ${target_name}_pdf,dvi,ps.  Generate rules which
#   will build the doxygen LaTeX stuff.  add_doxygen() attempts to depend
#   on everything that doxygen needs (eg additional header/footer files)
#   but if it didn't detect them all, touch the generated doxyfile, which is 
#   always depended on:
#   
#     ${CMAKE_BINARY_DIR}/${target_name}-doxyfile-generated
#   
#   WARNING: 
#     add_doxygen() forces some config options based on the dependancies
#     found and configuration directives.  (Eg USE_PDFLATEX if turned off if 
#     the pdflatex bin was not found).  However, in the interests of flexibility
#     I don't force all that many and (crucially) I don't check the template file
#     for changes.  This means that in the previous example, you could edit the 
#     additions file and the changes won't be noticed.
#     
#     The real meaning of this is that if the template file has an output turned
#     off then the install target for it will naturally be unable to work.  You
#     should therefore probably use the directives_list to force all options to do
#     with paths and directories.
#     
#     So in summary to the summary: it's a good idea to have a WANT_DOXYGEN_DOCS 
#     and WANT_PDF etc, use the directives_list to force the values which 
#     depend on them, and finally only generate install targets with respect to
#     the WANT_x variables.  If X is on, then force it in the conf and generate
#     the install target; otherwise force it off and don't generate the install.
#     
#     Note: the other doxygen_* functions/macros help with this task.
#
#   The doxygen binary chooses the path for outputs.  Its working directory is the 
#   source dir, therefore if you wish to build to the binary directory (good idea) 
#   then you need OUTPUT_DIRECTORY to be an absolute path.  Other output directories
#   are are recommended be relative to this (although it *shouldn't* matter).
#   Same goes for using empty output directories (doxygen will put the output
#   in the main output dir but it doesn't actually *say* it will work).
#   
#   Look at ${CMAKE_BINARY_DIR}/${target_name}-doxyfile-forced for the options 
#   which have been forced: your directives_list is at the start of this, with 
#   add_doxygen()'s overrides after.
#   
#   target_name
#     The targets ${target_name}, ${target_name}_pdf, ${target_name}_dvi,
#     and ${target_name}_pdf are added.  Note that ${target_name} generates
#     everything.  The other targets call the makefile for the latex outputs.
#   
#     If you want to have the docs built as part of your ordinary build, depend
#     a target on one of these
#     
#   template_file
#     The basis of doxygen's configuration.  The resultant configuration is 
#     the template file  with the directives_list appended to it.
#     
#   directives_list
#     Things which will always be overridden.  The directives list is simply 
#     lines which go in a doxyfile:
#     
#       set(DIRECTIVES "OUTPUT_DIRECTORY = doc/" "INPUTS = include/")
#       
#     ${directives_list} does not need to contain anything, but it is normally useful
#     to make option()s which control things like GENERATE_LATEX.  Note though that
#     add_doxygen() might override them if it thinks they won't work.
#   
#   Warning: things may not function properly if doxygen builds output type
#   into the same directory.
#   
#   Note: be sure to put list argument in quotes!
# 
#
# MACRO:
# * doxygen_setup_flags(flags_var target wants)
#   
#   Sets up a list in ${flags_var} which sets up a default set of sensible
#   overrides for the directives_list parameter of add_doxygen().
#
#   The flags var will contain overrides for all paths related to $wants.
#
#   The macro will set several useful paths (but remember that subsequent
#   calls to this macro will wipe them!).  They are mostly designed for 
#   having the documentaion mirrored in the sourcedir and installing from
#   that unless the user requests a rebuild specifically.
#
#   - DOXYGEN_OUT_DIR = $binary_dir/$target
#   - DOXYGEN_REL_HTML_DIR = relative path to html output
#   - DOXYGEN_REL_LATEX_DIR = relative path to latex output
#   - DOXYGEN_REL_PDF_FILE = relative path to outputted pdf file
#   - DOXYGEN_HTML_DIR = $doxygen_out/$rel_html_dir
#   - DOXYGEN_LATEX_DIR = $doxygen_out/$rel_latex_dir
#   - DOXYGEN_PDF_FILE = $doxygen_out/$rel_pdf_file
#
#   This macro also responds to the following variables:
#   - DOXYGEN_EXAMPLE_PATH = a path to the example dir; default
#                            $srcdir/examples
#   - DOXYGEN_INPUT_PATHS = a *string* containing paths to the
#                           dirs for doxygen to read from, default is
#                           $srcdir/include
#   - PROJECT_NAME = used for the PROJECT_NAME configuration
#   - PROJECT_VERSION = used for the PROJECT_NUNBER configuration
#
#   flags_var
#     name of the variable to *append* to.
#
#   target
#     name of the target that will be passed to add_doxygen
#
#   wants
#     List of keys that are wanted.  Recognised values are:
#     - pdf
#     - html
#
# MACRO: 
# * doxygen_install_targets(doxygen_target wants output_dir)
#
#   Adds "make install" targets for generated doxygen files using the
#   variables which are set by doxygen_setup_flags.
#   
#   doxygen_target
#     A target that was set up using add_doxygen()
#
#   wants
#     Same as doxygen_setup_flags
#
#   output_dir
#     Use this instead of DOXYGEN_OUT_DIR.  This is for installing from the
#     source dir and means that the doxygen *will not* automatically be build
#     with make all.
# 
#   doxygen_root
#
#   The install locations are share/doc/${CMAKE_PROJECT_NAME}/.  Directories
#   automatically have the useless stuff left over from doxygen stripped 
#   out.



find_package(Doxygen)
find_package(LATEX)

find_program(CAT_EXE cat type)
find_program(MAKE_EXECUTABLE make gmake nmake)
mark_as_advanced(CAT_EXE MAKE_EXECUTABLE)


function(add_doxygen target_name template_file directives_list)
  # TODO: could do with the ability to specify further dependancies,
  #       eg force a rebuild whenever srcdir changes.

  # TODO: need to use a relative dir as the input dirs because otherwise the
  #       include paths appear with the full /home/... path in there.  Maybe
  #       that's supposed to happen in another function?

  # TODO: if doxyfile-force is deleted then the build breaks; we need to
  #       set that to depend on the re-configure build depend.  It is rebuild_cache
  #       for me but is that guaranteed?

  if (NOT DOXYGEN_EXECUTABLE)
    message(STATUS "Ignoring doxygen targets due to no doxygen exe.")
  endif()

  message(STATUS "Adding doxygen target ${target_name} from template file ${template_file}.")
  message("Note: output paths and generators should not be changed in the template file without re-configuring")

  # TODO silly limitation -- need to cause the buildsystem to reconfigure if the base doxyfile changes

  if(NOT CAT_EXE) 
    message("Guessing the name of cat/type as `cat'.  If doxygen doesn't work, manually define all paths in the base doxyfile.")
    set(CAT_EXE cat)
  endif()
  
  set(doxygen_conf_file "${CMAKE_BINARY_DIR}/${target_name}-doxyfile-generated")
  set(additions_file "${CMAKE_BINARY_DIR}/${target_name}-doxyfile-forced")
 
  message("Doxygen forced vars are in ${additions_file}.")

  ## Detect paths and vars defined by the template file and the overrides ##
  set(conf_output_dir "doxygen")
  set(conf_generate_latex YES)
  set(conf_generate_html YES)
  set(conf_generate_man NO)
  
  set(conf_html_dir "html")
  set(conf_latex_dir "latex")
  set(conf_man_dir "man")
  set(conf_use_pdflatex YES)
  
  # Stuff to match (code written which depends on it):
  # - OUTPUT_DIRECTORY
  # - GENERATE_LATEX LATEX_OUTPUT USE_PDFLATEX
  # - GENERATE_HTML HTML_OUTPUT
  # - GENERATE_MAN MAN_OUTPUT
  #  
  # Confs relevant for installation:
  # - HTML_FILE_EXTENSION DOT_IMAGE_FORMAT 
  # 
  # add_doxygen doesn't have explicit support for this yet:
  # - GENERATE_RTF RTF_OUTPUT
  # - GENERATE_HTMLHELP CHM_FILE
  #   - GENERATE_CHI (means a seperate file comes with HTMLHELP)
  # - GENERATE_AUTOGEN_DEF 
  # - GENERATE_DOCSET (something to do with xcode `doxygen will generate a Makefile 
  #   in the HTML output directory'.  It has a make install)
  #   - maybe force this off since I don't know how it works.
  # - GENERATE_XML XML_OUTPUT
  # 
  # There are other things, like header files etc which we should depend
  # on: MSCGEN_PATH, RTF_STYLESHEET_FILE RTF_EXTENSIONS_FILE HTML_STYLESHEET
  # HTML_HEADER HTML_FOOTER LATEX_HEADER 
  # 
  # TODO: 
  #   it might be better to just require that all these parameters are forced... it
  #   will probably break install targets otherwise... the main tricky thing is that
  #   we turn stuff off that *might* have been on initially which is confusing...
  #   
  #   Solutions?
  #   - require all contentious things to be forced
  #   - detect all contentious things everywhere and error immediately if they are not 
  #     enforcable
  #     - best, but the user needs to have a way to work around it 
  #       - suggestion in docs that the maintainer adds a WANT_x doc?
  #     - hard work
  # TODO:
  # - need to deal with it properly when the paths are not complete (they are assumed to 
  #   be in the srcdir.  This is necesasry or the dependancies might not work properly.

  file(READ ${template_file} file)
  
  # TODO: seriously?  This  should at least be done in a function somehow.
  #  - first we should 
  string(
    REGEX MATCH
    "[Oo][Uu][Tt][Pp][Uu][Tt]_[Dd][Ii][Rr][Ee][Cc][Tt][Oo][Rr][Yy][^=]*=[\t\r ]*([^#]*).*" 
    "\\1" out "${file}"
  )
  
  string(
    REGEX MATCH
    "USE_PDFLATEX[^=]*=[\t\r ]*([^#]*).*" 
    "\\1" out "${file}"
  )
  
#   message("${out}")
  
  ## Generate the overrides file (also detect some configs in it) ##
  
  file(WRITE ${additions_file} "")
  foreach(iter ${directives_list}) 
    # TODO: match all the required vars again
    
    if (${iter} MATCHES "[Oo][Uu][Tt][Pp][Uu][Tt]_[Dd][Ii][Rr][Ee][Cc][Tt][Oo][Rr][Yy]")
      string(
        REGEX REPLACE 
        "[^=]*=[\t\r ]*([^#]*).*" 
        "\\1" doxygen_output_dir "${iter}"
      )
    else() 
      message("not it: ${iter}")
    endif()
    
    file(APPEND ${additions_file} "${iter}\n")
  endforeach()

  # TODO: also check that if latex, latexdir set etc etc.  We allow a blank
  #   latexdir tho because doxygen will just put it in the maindir (check this)

  if (NOT doxygen_output_dir)
    message("No doxygen output dir given.  Doxygen targets can't be added.")
    return()
  endif()

  if (NOT conf_generate_html AND NOT conf_generate_man AND NOT conf_generate_latex)
    message(STATUS "I'm confused; doxygen doesn't seem to be set to generate anything.")
    return()
  endif()

  ## Config forces based on found programs ##

  if (NOT DOXYGEN_DOT_PATH)
    file(APPEND ${additions_file} "HAVE_DOT = no\n")
  else()
    file(APPEND ${additions_file} "DOT_PATH = ${DOXYGEN_DOT_PATH}\n")
  endif()
  file(APPEND ${additions_file} "LATEX_BATCHMODE = yes\n")
  
  if (conf_generate_latex) 
    if (LATEX_COMPILER AND PDFLATEX_COMPILER) 
      # Let the template file choose.  Vars are forced anyway because if they get
      # changed later we can't detect it.
      if (conf_use_pdflatex)
        file(APPEND ${additions_file} "USE_PDFLATEX = yes\n")
        file(APPEND ${additions_file} "LATEX_CMD_NAME = ${PDFLATEX_COMPILER}\n")
      else()
        file(APPEND ${additions_file} "USE_PDFLATEX = no\n")
        file(APPEND ${additions_file} "LATEX_CMD_NAME = ${LATEX_COMPILER}\n")
      endif()
    elseif(PDFLATEX_COMPILER)
      file(APPEND ${additions_file} "USE_PDFLATEX = yes\n")
      file(APPEND ${additions_file} "LATEX_CMD_NAME = ${PDFLATEX_COMPILER}\n")
    elseif(LATEX_COMPILER)
      file(APPEND ${additions_file} "USE_PDFLATEX = no\n")
      file(APPEND ${additions_file} "LATEX_CMD_NAME = ${LATEX_COMPILER}\n")
    else()
      message("Generate LaTeX was on but no compiler was found.")
      file(APPEND ${additions_file} "GENERATE_LATEX = no\n")
      set(conf_generate_latex NO)
    endif()
    
    # TODO: there's other stuff: if it doesn't have a grapghics converter we need to
    #   turn off *all* graphics.  I can't remember what teh graphics prog was anyway.
    #   This only applies for LaTeX generation!
  endif()
  
  ## Configure paths ##
  
  
  # TODO: only if doxygen_output_dir is relative, then prepend binary_dir to it.
  set(absolute_doxygen_path "${CMAKE_BINARY_DIR}/${conf_output_dir}")
  string(REGEX REPLACE "\\/[\\/]+" "/" absolute_doxygen_path ${absolute_doxygen_path})
  
  # TODO: how does doxygen deal with a complete path for, eg latex?  Is it allowed?  
  #   If so we must detect it.
  
  set(absolute_latex_path "${absolute_doxygen_path}/${conf_latex_dir}")
  set(absolute_html_path "${absolute_doxygen_path}/${conf_html_dir}")
  set(absolute_man_path "${absolute_doxygen_path}/${conf_man_dir}")
  
  set(pdf_output "${absolute_latex_path}/refman.pdf")
  set(ps_output "${absolute_latex_path}/refman.ps")
  set(dvi_output "${absolute_latex_path}/refman.dvi")
  
  set(main_outputs "${absolute_doxygen_path}")
  if (conf_generate_man) 
    set(main_outputs "${main_outputs} ${absolute_man_path}")
  endif()
  if (conf_generate_html)
    set(main_outputs "${main_outputs} ${absolute_html_path}")
  endif()
  if (conf_generate_latex)
    set(main_outputs "${main_outputs} ${absolute_latex_path}")
  endif()
  
  
  message("** LATEX: ${absolute_latex_path}")
  message("** PDF:   ${pdf_output}")
 
  ## Add targets based on the paths ##
  
  # Applies to whatever the cwd is .
  set_property(
    DIRECTORY APPEND 
    PROPERTY "ADDITIONAL_MAKE_CLEAN_FILES" 
    ${additions_file} ${doxygen_conf_file} ${absolute_doxygen_path}
  )
  
  add_custom_command(
    OUTPUT ${doxygen_conf_file}
    COMMAND ${CAT_EXE} ${template_file} > ${doxygen_conf_file}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CAT_EXE} ${additions_file} >> ${doxygen_conf_file}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS ${template_file}
    COMMENT "Generating doxygen config file based on the template and forced values."
    VERBATIM
  )
  
  add_custom_command(
    OUTPUT ${absolute_doxygen_path} 
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxygen_conf_file}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${doxygen_conf_file}
    COMMENT "Generating main doxygen (html, .tex files ...)"
    VERBATIM 
  )
  
  # TODO dynamic paths; also if this works, then always use directories as the output.
  add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/doxygen/html"
    DEPENDS ${absolute_doxygen_path} 
    COMMENT "Phony target to generate HTML."
    VERBATIM 
  )
  
  # TODO: is this really needed?  Also, make the path dynamic.  Also it's wrong
  #   anyway because the makefile might not get modified
  add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/doxygen/latex/Makefile"
    DEPENDS ${absolute_doxygen_path} 
    COMMENT "Phony target to generate Makefile."
    VERBATIM 
  )
  
  add_custom_target(
    ${target_name}_main
    DEPENDS "${absolute_doxygen_path}"
    COMMENT "Generating HTML, LaTeX, man (if doxyfile changed)."
  )
  
  if (conf_generate_latex)
    if (NOT LATEX_COMPILER AND NOT PDFLATEX_COMPILER) 
      message(STATUS "No LaTeX found.  Doxygen LaTeX-based targets not added.")
    else()
      message(STATUS "Adding doxygen LaTeX targets.")
      
      if (NOT MAKE_EXECUTABLE)
        message("Make exe can't be found: doxygen LaTeX is not buildable.")
      else()
        # TODO: only the pdf command works!  Could it be an issue with USE_PDFLATEX?
        #   I think not.  It's prolly because pdflatex directly outputs the pdf and
        #   the others only output a dvi which can be converted either way.  Otherwise
        #   I can use ps2pdf... I need to test exactly what is outputted when not 
        #   pdflatex.
        #   
        add_custom_command(
          OUTPUT ${dvi_output}
          COMMAND ${MAKE_EXECUTABLE} dvi
          WORKING_DIRECTORY "${absolute_latex_path}"
          DEPENDS "doxygen/latex/Makefile"
          COMMENT "Calling doxygen's generated makefile for dvi (this is error prone!)."
          VERBATIM 
        )
      
        add_custom_command(
          OUTPUT ${pdf_output}
          COMMAND ${MAKE_EXECUTABLE} pdf
          WORKING_DIRECTORY "${absolute_latex_path}"
          DEPENDS "${CMAKE_BINARY_DIR}/doxygen/latex/Makefile"
          COMMENT "Calling doxygen's generated makefile for pdf (this is error prone!)."
          VERBATIM 
        )
        
        add_custom_command(
          OUTPUT ${ps_output}
          COMMAND ${MAKE_EXECUTABLE} ps
          WORKING_DIRECTORY "${absolute_latex_path}"
          DEPENDS "doxygen/latex/Makefile"
          COMMENT "Calling doxygen's generated makefile for ps (this is error prone!)."
          VERBATIM 
        )
  
        add_custom_target(
          ${target_name}_ps
          DEPENDS ${ps_output}
          COMMENT "Call ps makefile if ps isn't built."
        )
        
        add_custom_target(
          ${target_name}_dvi
          DEPENDS ${dvi_output}
          COMMENT "Call dvi makefile if dvi isn't built."
        )
        
        add_custom_target(
          ${target_name}_pdf
          DEPENDS ${pdf_output}
          COMMENT "Call pdf makefile if pdf isn't built."
        )
      endif()
    endif()
  endif()
  
endfunction()

macro(doxgyen_setup_flags flags_var target wants)
  message(STATUS "Setting doxygen flags.")

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
    message("doxgyen_setup_flags() finished, set values:")
    message("- DOXYGEN_OUT_DIR = ${DOXYGEN_OUT_DIR}")
  
    message("- DOXYGEN_REL_HTML_DIR = ${DOXYGEN_REL_HTML_DIR}")
    message("- DOXYGEN_REL_LATEX_DIR = ${DOXYGEN_REL_LATEX_DIR}")
    message("- DOXYGEN_REL_PDF_FILE = ${DOXYGEN_REL_PDF_FILE}")

    message("- DOXYGEN_HTML_DIR = ${DOXYGEN_HTML_DIR}")
    message("- DOXYGEN_LATEX_DIR = ${DOXYGEN_LATEX_DIR}")
    message("- DOXYGEN_PDF_FILE = ${DOXYGEN_PDF_FILE}")
  endif()
endmacro()

function(doxygen_install_targets doxygen_target wants install_to install_docs_from)
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

