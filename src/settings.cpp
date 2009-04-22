#include "settings.hpp"

#include "notes.hpp"

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
      "order.   With no notes and no --start, it defaults to playing 'a'.\n"
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

  std::string root_note;
  po::options_description all_opts("Options");
  all_opts.add_options()
    ("help,h", "Show this help message and quit.")
    ("verbose,v", "Be verbose.")
    ("loop,l", "Replay the sequence when it's done.")
    ("time,t", po::value<int>(&duration_),
     "Time for each note in milliseconds; 0 for forever.  Defaults: " DEFAULT_NOTE_DURATION_STR " unless no notes "
     "or --start specified in which case it is forever.")
    ("pause", po::value<int>(&pause_time_),
     "Millisecond pause time between notes.  Default: " DEFAULT_PAUSE_TIME_STR)
    ("dump,D", po::value<std::string>(&dump_file_),
     "Dump raw samples to a file.  The file usually need to be converted to a playable format with ffmpeg(1) or similar.")
    ("start,s", po::value<std::string>(&start_note_),
     "Note name or frequency to start with.")
    ("distance,d", po::value<int>(&note_distance_),
     "Half notes between notes starting from -s, --start.  Default: " DEFAULT_NOTE_DISTANCE_STR)
    ("end,e", po::value<std::string>(&end_note_),
     "Note name or frequency to end on with.  Default: as though --end ${start}+.")
    ("number,n", po::value<int>(&num_increments_),
     "How many notes to play from --start with.  Default: as though --end ${start}+.")
    ("concert-pitch,p", po::value<double>(&concert_pitch_),
     "Frequency of the base note that we work all other notes out from.  Default: 440hz (A).")
    ("concert-pitch-note", po::value<std::string>(&root_note),
     "Note name of the base note that we work all other notes out from (eg, aB to tune "
     "down half a step).  The corresponding note frequency is worked out using a concert "
     "pitch of 440hz.")
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

  if (vm.count("dump")) {
    // TODO: I need a --overwrite?
    // if (boost::filesystem::exists(dump_file_)) {
    //   throw std::runtime_error("--dump file already exists");
    // }
  }

  // TODO:
  //   perfer some way of --time-forever so we don't have to do --time=0 which makes
  //   no sense.
  if (duration_ < 0) {
    throw std::runtime_error("--time must be greater than 1.");
  }
  else if (duration_ == 0) {
    std::cerr << "warning: when --time is 0 each note will play forever." << std::endl;
  }

  if (vm.count("volume")) {
    if (volume_ < 0 || volume_ > 100) {
      // TODO: throw an error here
    }
    else if (volume_ == 0) {
      std::cout << "warning: volume is 0.  Are you sure?  :)" << std::endl;
    }
  }

  if (pause_time_ < 0) {
    throw std::runtime_error("--pause must be at least 0");
  }

  if (vm.count("verbose")) {
    verbosity_level_ = verbosity_verbose;
  }

  if (vm.count("concert-pitch") && vm.count("concert-pitch-note")) {
    throw std::runtime_error("--concert-pitch and --concert-pitch-note conflict");
  }
  else if (vm.count("concert-pitch-note")) {
    // TODO:
    //   if I'm parsing this note here, why not parse all the other notes too?
    //   Would make it easier to keep the semantics the cli stuff the same as what's
    //   written here.  Similar stuff goes for deciding the ending octave stuff.
    const double default_pitch = 440.0;
    concert_pitch_ = offset_to_frequency(default_pitch, parse_note(root_note.c_str()));
  }

  if (vm.count("loop")) { flags_[fl_loop] = true; }

  // now loop the non-options values which are in 'parsed'
  // also note_mode_list/note_mode_start.

  // if (vm.count("start") && there_are_notes_specified) {
  //   throw std::runtime_error("--start and specifying notes conflict");
  // }

  // TODO:
  //   if not start and not notes specified, then start == a.
  //
  // TODO: if no notes and no --start and no duration then duration == forever
  //

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
    // TODO: assign parsed stuff to notes
  }
}

