#include "settings.hpp"

#include <tune_config.hpp>

#include <boost/program_options.hpp>

#include <iostream>
#include <cstdlib>

namespace {
  const char *usage_message() {
    return
      "usage: tune [option]... [note[#|B][+|-]|freq]...\n"
      "Play one or more notes in order.  Notes are a-g with a # or B suffix optionally\n"
      "followed by a '+' or '-' to denote offset from concert pitch octave.  Otherwise\n"
      "a frequency value may be supplied directly.  Options and arguments can be in any\n"
      "order.   With no notes, it defaults to A at 440hz.\n"
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

  po::options_description all_opts("Options");
  all_opts.add_options()
    ("help,h", "This message and quit.")
    ("verbose,v", "Be verbose.")
    ("loop,l", "Loop playing.")
    ("time,t", po::value<int>(&duration_),
     "Time for each note in milliseconds.  Defaults: " DEFAULT_NOTE_DURATION_STR)
    ("pause", po::value<int>(&pause_time_),
     "Millisecond pause time between notes.  Default: " DEFAULT_PAUSE_TIME_STR)
    ("dump,D", po::value<std::string>(&dump_file_),
     "Dump raw samples to a file.")
    ("start,s", po::value<std::string>(&start_note_),
     "Note name or frequency to start with.")
    ("distance,d", po::value<int>(&note_distance_),
     "Half notes between notes starting from -s, --start.  Default: " DEFAULT_NOTE_DISTANCE_STR)
    ("end,e", po::value<std::string>(&end_note_),
     "Note name or frequency to end on with.")
    ("number,n", po::value<int>(&num_increments_),
     "How many notes to play from --start with.  Default: keep going until an octave.")
    ("concert-pitch,p", po::value<double>(&concert_pitch_),
     "Frequency of the base note that we work all other notes out from.  Default: 400hz (A).")
    ("volume,a", po::value<int>(&volume_),
     "Amplitude number between 0 and 100.  Default: " DEFAULT_VOLUME_STR)
    ("rate", po::value<int>(&sample_rate_),
     "Sample rate.  Default: " DEFAULT_SAMPLE_RATE_STR)
    ("channels", po::value<int>(&channels_),
     "Channels in the sample (1, for mono, 2 for stereo etc).  Default: " DEFAULT_CHANNELS_STR)
    ;

  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv).options(all_opts).run();
  po::store(parsed, vm);
  po::notify(vm);

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

  // validate and assign
  dump_file_; // if vm.count etc etc
  duration_; // above 0.
  sample_rate_; // warn if it's not common mabe?
  // TODO: somehow I have to work out how to make it stop after one octave
  num_increments_;


  if (vm.count("volume")) {
    if (volume_ < 0 || volume_ > 100) {
      // TODO: throw an error here
    }
    else if (volume_ == 0) {
      std::cout << "warning: volume is 0.  Are you sure?  :)" << std::endl;
    }
  }

  if (vm.count("verbose")) {
    verbosity_level_ = verbosity_verbose;
  }

  if (vm.count("loop")) { flags_[fl_loop] = true; }

  // now loop the non-options values which are in 'parsed'
  // also note_mode_list/note_mode_start.

  // if (vm.count("start") && there_are_notes_specified) {
  //   throw std::runtime_error("--start and specifying notes conflict");
  // }

  if (vm.count("start")) {
    note_mode_ = note_mode_start;
    if (vm.count("number") && num_increments_ <= 0) {
      throw std::runtime_error("value is less than 1 for --number");
    }

    // Translate number of notes into number of increments.
    --num_increments_;

    if (note_distance_ == 0) {
      std::cout << "warning: --distance is 0: the note won't change." << std::endl;
    }

    if (vm.count("number") && vm.count("end")) {
      throw std::runtime_error("--number and --end conflict");
    }
  }
  else {
    // validate notes list
  }
}

