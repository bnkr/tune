# This file is a simple interface to manpage generation.
#
###################################################
# MACRO: add_manpage_gz(OUTPUT_VAR, INPUT_FILE):
#   Build a gzipped manpage.  Should be installed manually.  Will always be
#   built into ${MANPAGE_OUTPUT_DIR}.
#
#   Example:
#     add_manpage_gz(PAGE "doc/something.1")
#
#     install(FILES ${PAGE} DESTINATION ${MANDIR})
#
##########################################################
# FUNCTION: add_and_install_manpages(MANDIR, PAGE1, [...]):
#   Uses add_manpage_gz to build manpages and adds install targets for them.
#
#   The install directory is:
#
#     ${MANDIR}/man$N
#
#   Where $N is the last letter  of the input filename.
#

set(MANPAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/man-gz")
set(MANPAGE_PHONY_DEPEND "manpages-phony")

find_program(GZIP_EXE "gzip")
mark_as_advanced(GZIP_EXE)

set_property(
  DIRECTORY APPEND
  PROPERTY "ADDITIONAL_MAKE_CLEAN_FILES"
  "${MANPAGE_OUTPUT_DIR}"
)

# This is a giant hack to remove the necessity for a target for each
# manpage.  Every manpage will append itself as a dependency to this.
# The command is just a noop because we can't have a custom command
# without an actual shellcmd to run.  We must make the target here
# because an APPEND will only work on an existing target.
add_custom_command(
  OUTPUT  "${MANPAGE_PHONY_DEPEND}"
  COMMAND "${CMAKE_COMMAND}" -E touch_nocreate "${CMAKE_BINARY_DIR}"
  COMMENT "Checked manpage dependencies."
)

set_source_files_properties(
  "${MANPAGE_PHONY_DEPEND}" PROPERTIES SYMBOLIC TRUE
)

# Finally we give a top-level name.
add_custom_target(
  manpages ALL
  DEPENDS "${MANPAGE_PHONY_DEPEND}"
)


macro(add_manpage_gz output_var input)
  message(STATUS "Add manpage ${input}")
  get_filename_component(ampgz_basename ${input} NAME)
  set(ampgz_out "${MANPAGE_OUTPUT_DIR}/${ampgz_basename}.gz")
  set("${output_var}" "${ampgz_out}")

  set(ampgz_dir_depend "${MANPAGE_OUTPUT_DIR}/dir-created-stamp")

  add_custom_command(
    OUTPUT "${ampgz_dir_depend}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${MANPAGE_OUTPUT_DIR}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E touch "${ampgz_dir_depend}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Creating compressed manpage dir."
    VERBATIM
  )

  add_custom_command(
    OUTPUT  "${ampgz_out}"
    DEPENDS "${input}"
            "${ampgz_dir_depend}"
    COMMAND "${GZIP_EXE}" -c "${input}" > "${ampgz_out}"
    COMMENT "Gzip ${ampgz_basename}"
    VERBATIM
  )

  # Build up a dependency target.
  add_custom_command(
    APPEND
    OUTPUT  "${MANPAGE_PHONY_DEPEND}"
    DEPENDS "${ampgz_out}"
  )
endmacro(add_manpage_gz)

function(add_and_install_manpages mandir)

  # TODO:
  #   Make it like:
  #   add_and_install_manpages(ROOT manroot PAGES page...)

  set(pages "${ARGV}")
  list(REMOVE_AT pages 0)
  if (NOT pages)
    message(FATAL_ERROR "add_and_install_manpages(): at least one manpage is required.")
  endif()

  foreach (arg ${pages})
    string(LENGTH "${arg}" len)
    math(EXPR begin "${len} - 1")
    string(SUBSTRING "${arg}" "${begin}" "1" last_char)
    add_manpage_gz(output ${arg})
    set(dest "${mandir}/man${last_char}")
    install(FILES "${output}" DESTINATION ${dest})
  endforeach()
endfunction()

