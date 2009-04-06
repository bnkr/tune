# Utility routines.

# Sets any marked variable to that of the varname.  Everything which is not 
# marked by a name (like VARNAME val1 ...) goes in a var called OTHER.  Only
# those vars in $allowed are searched for.
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
# Args:
# - allowed   = which arguments are allowed.  Vars with their name will be set to
#               their value.
# - flags     = which arguments are flags (they take no values).  They will be set
#               boolean.
# - ignore    = which things are arguments, but we shouldn't actually assign them.
#               IOW, this causes parse of a subset of the total args.  Unparsed args
#               go in PA_IGNORED.
# - argslist  = 
#
# TODO: write examples of the ignore list thing.  
# TODO: a simple unit test for this would be useful.
macro(butil_parse_args allowed flags ignore arglist)
  set(pa_var)
  set(pa_add "TRUE")
  foreach(arg ${arglist})
    # Set the varptr to PA_IGNORED if applicable.
    foreach (search_arg ${ignore}) 
      if (${arg} STREQUAL ${search_arg})
        set(pa_var "PA_IGNORED")
        # Yes, we DO want this one!
        set(pa_add TRUE)
        break()
      endif()
    endforeach()

    if (NOT pa_var STREQUAL "PA_INGNORED") 
      # Set the varptr to the marker var if applicable.
      foreach (search_arg ${allowed})
        if (${arg} STREQUAL "${search_arg}")
          set(pa_var ${arg})
          # Don't add this to the last var's arglist
          set(pa_add)
          break()
        endif()
      endforeach()

      # Set directly set the var to true if present and we didn't just set ignored.
      foreach (search_arg ${flags}) 
        if (${arg} STREQUAL ${search_arg})
          set(${arg} TRUE)
          set(pa_add)
          break()
        endif()
      endforeach()
    endif()

    # If this arg is not a marker var, then add it to the varptr.
    if (pa_add) 
      if (NOT pa_var) 
        list(APPEND "PA_OTHER" ${arg})
      else()
        list(APPEND ${pa_var} ${arg})
      endif()
    endif()

    set(pa_add "TRUE")
  endforeach()
endmacro()
