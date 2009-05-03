// throw some argv at it?

// TODO:
//   - get all options
//   - for each 'setter' assign a value and test it is set in settings.
//   - for each validation error test, that it raises or whatever.

#include "../src/settings.hpp"

#include  <boost/program_options.hpp>

#include <cstdlib>
#include <cassert>

int main() {
  // unknown opion
  {
    const char *argv[] = {
      "prog",
      "--blah"
    };

    bool reached = false;
    try {
      settings s(2, (char **) argv);
      reached = true;
    }
    catch (boost::program_options::unknown_option &) {
    }

    assert(! reached);
  }

  // option missing argument
  {
    const char *argv[] = {
      "prog",
      "--dump"
    };

    bool reached = false;
    try { settings s(2, (char**)argv); reached = true; }
    catch (boost::program_options::invalid_command_line_syntax &) { }
    assert(! reached);
  }

  // TODO:
  //   Test the following:
  //   - existing file for --dump
  //   - wrong notes for the note list.
  //   - for each verbosity level, test that messages of the type will display.
  //
  //   And that things are set:
  //   - --amplitude
  //   - --rate
  //   - --channels
  //   - --number
  //   - --concert-pitch
  //   - --loop
  //   - --time
  //   - --pause

  // TODO:
  //   Test note conversions (first need to implement them in settings):
  //   - start note
  //   - end note
  //   - note list
  //   - concert-pitch-note

  // TODO:
  //   Test that certain things are errors:
  //   - --start and notes specified
  //   - --end without --start.
  //   - --start without --distance or --end
  //   - --distance without --start

  return EXIT_SUCCESS;
}
