# TODO: work out WTF this function is doing and port it to my  
#       syntax
#   boost_test_compile_fail(testname 
#                           [source1]
#                           [COMPILE_FLAGS compileflags])

# macro(boost_test_compile_fail testname)
#   boost_test_parse_args(${testname} ${ARGN})
#   Determine the include directories to pass along to the underlying
#   project.
#   get_directory_property(BOOST_TEST_INCLUDE_DIRS INCLUDE_DIRECTORIES)
#   set(BOOST_TEST_INCLUDES "")
#   foreach(DIR ${BOOST_TEST_INCLUDE_DIRS})
#     set(BOOST_TEST_INCLUDES "${BOOST_TEST_INCLUDES}:${DIR}")
#   endforeach(DIR ${BOOST_TEST_INCLUDE_DIRS})
#     
#   if(BOOST_TEST_OKAY)
#     add_test("${PROJECT_NAME}::${testname}"
#        ${CMAKE_CTEST_COMMAND}
#              --build-and-test
#              ${Boost_SOURCE_DIR}/tools/build/CMake/CompileTest
#              ${Boost_BINARY_DIR}/tools/build/CMake/CompileTest
#              --build-generator ${CMAKE_GENERATOR}
#              --build-makeprogram ${MAKEPROGRAM}
#        --build-project CompileTest
#        --build-options -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${BOOST_TEST_SOURCES} -DINCLUDES=${BOOST_TEST_INCLUDES} -DCOMPILE_FLAGS=${BOOST_TEST_COMPILE_FLAGS})
#     set_tests_properties("${PROJECT_NAME}::${testname}" PROPERTIES WILL_FAIL ON)
#   endif(BOOST_TEST_OKAY)
# endmacro(boost_test_compile_fail)


# Find wine.
if (WIN32 AND CMAKE_CROSSCOMPILING AND CMAKE_HOST_UNIX)
  find_program(WINE_EXE wine)
  if (WINE_EXE)
    message(STATUS "Detected a Unix->Windows cross compile, so tests will be run by ${WINE_EXE}")
    set(TEST_RUNNER "${WINE_EXE}")
  else()
    message(STATUS "Detected a Unix->Windows cross compile, but wine could not be found: test will not run!")
  endif()
endif()

find_program(RUBY_EXE ruby) 
mark_as_advanced(RUBY_EXE WINE_EXE)

# optional param: libraries, cppflags, test program arguments
function(wrap_test target sources)
  add_executable(${target} ${sources})

  if (ARGV2)
    target_link_libraries(${target} ${ARGV2})
  endif()

  if (ARGV3)
    set_target_properties("${target}" PROPERTIES COMPILE_DEFINITIONS "${ARGV3}")
  endif()

  add_test(test_${target} ${TEST_RUNNER} ${target}${CMAKE_EXECUTABLE_SUFFIX} ${ARGV4})
endfunction()

# Optional params: libraries, cppflags, arguments
function(wrap_test_xfail target sources)
  wrap_test("${target}" "${sources}" "${ARGV2}" "${ARGV3}" "${ARGV4}")
  set_tests_properties("test_${target}" PROPERTIES WILL_FAIL YES)
endfunction()


# Argv3 is link flags, Argv4 is defines, Argv5 is extra depends.
function(rubygen_test target script args)
  if (NOT RUBY_EXE)
    return()
  endif()

  if (ARGV5)
    set(more_deps "${ARGV5}")
  endif()

  set(dir "${CMAKE_BINARY_DIR}/generated_tests")
  if(NOT IS_DIRECTORY "${dir}")
    file(MAKE_DIRECTORY "${dir}")

    set_property(
      DIRECTORY APPEND
      PROPERTY "ADDITIONAL_MAKE_CLEAN_FILES"
      "${dir}"
    )
  endif()

  set(script_out "${dir}/${target}.cpp")
  add_custom_command(
    OUTPUT "${script_out}"
    COMMAND "${RUBY_EXE}" "-w" "${script}" "${args}" "${script_out}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${script}" ${more_deps}
    COMMENT "Ruby generating ${target} with args ${args}"
    VERBATIM
  )

  wrap_test("${target}" "${script_out}" "${ARGV3}" "${ARGV4}")
endfunction()

