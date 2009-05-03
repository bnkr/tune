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
  //   Test note conversions (first need to implement them in settings).

  return EXIT_SUCCESS;
}
