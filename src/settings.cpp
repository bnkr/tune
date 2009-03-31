#include "settings.hpp"

// TODO: the config
#if 0
#include <tune_config.hpp>
#endif

#define TUNE_VERSION "0.0"

#include <boost/program_options.hpp>

#include <iostream>
#include <cstdlib>

namespace {
  const char *usage_message() {
    return
      "usage: tune [-l] [-m S] [-s N -d D] [abcdefg[#|B][N]]... [F]...\n"
      "Play one or more notes in order.  Notes are a-g with a # or B suffix or a\n"
      "frequency value.  The N suffix to a note decids which octave to play on.  \n"
      "Options and arguments can be in any order.   With no notes, it defaults to c.\n"
      ;
  }

  const char *version_message() {
    return
      "Copyright (C) 2009, James Webber.\n"
      "Version " TUNE_VERSION ".  Under a 3 clause BSD license."
      ;
  }
}

void settings::print_help(boost::program_options::options_description &d) {
  std::cout << usage_message() << std::endl;
  std::cout << d << std::endl;
  std::cout << version_message() << std::endl;
}

void settings::print_version() {
  std::cout << version_message() << std::endl;
}

void settings::parse_args(int argc, char **argv) {
  namespace po = boost::program_options;

  std::string start_note;

  po::options_description all_opts;
  all_opts.add_options()
    ("help,h", "This message and quit.")
    ("verbose,v", "Be verbose.")
    ("loop,l", "Loop playing.")
    ("time,t", po::value<int>(&duration_),
     "Time for each note in miliseconds.  Defaults: " DEFAULT_NOTE_DURATION_STR)
    ("dump,D", po::value<std::string>(&dump_file_),
     "Dump raw samples to a file.")
    ("start,s", po::value<std::string>(&start_note),
     "Note name to start with.")
    ("distance,d", po::value<int>(&note_distance_),
     "Half notes between notes starting from -s, --start.  Default: " DEFAULT_NOTE_DISTANCE_STR)
    ("pause", po::value<int>(&pause_time_),
     "Milisecond pause time between notes.  Default: " DEFAULT_PAUSE_TIME_STR)
    ("rate", po::value<int>(&sample_rate_),
     "Sample rate.  Default: " DEFAULT_SAMPLE_RATE_STR)
    ("channels", po::value<int>(&channels_),
     "Channels in the sample (1, for mono, 2 for stero etc).  Default: " DEFAULT_CHANNELS_STR)
    ;

  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv).options(all_opts).run();
  po::store(parsed, vm);
  po::notify(vm); //< do I need that?

  if (vm.count("help")) {
    exit_status_ = exit_ok;
    print_help(all_opts);
    return;
  }
  else if (vm.count("version")) {
    exit_status_ = exit_ok;
    print_version();
    return;
  }

  // validate:
  dump_file_; // if vm.count etc etc
  duration_;
  note_distance_;
  sample_rate_;
  start_note; // generate the array from this I guess.  so no more args allowed if this is set.

  if (vm.count("verbose")) { verbosity_level_ = verbosity_verbose; }

  if (vm.count("loop")) { flags_[fl_loop] = true; }

  // now loop the non-options values
}

