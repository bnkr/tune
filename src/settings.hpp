/*!
\file
\brief Command-line option parsing.
*/
#ifndef SETTINGS_HPP_g0vr9k40
#define SETTINGS_HPP_g0vr9k40

#include "notes.hpp"

#include <vector>
#include <string>
#include <bitset>
#include <stdexcept>

#include <cstdlib>

// so we can string it in the help text
#define DEFAULT_NOTE_DURATION     200
#define DEFAULT_NOTE_DURATION_STR "200"
#define DEFAULT_CHANNELS          2
#define DEFAULT_CHANNELS_STR      "2"
#define DEFAULT_SAMPLE_RATE       44100
#define DEFAULT_SAMPLE_RATE_STR   "44100"
#define DEFAULT_NOTE_DISTANCE     8
#define DEFAULT_NOTE_DISTANCE_STR "8"
#define DEFAULT_PAUSE_TIME        5
#define DEFAULT_PAUSE_TIME_STR    "5"

namespace boost {
  namespace program_options {
    class options_description;
  }
}

//! \brief Thrown for any vailidation error.
class invalid_setting : public std::runtime_error {
  public:
    invalid_setting(const std::string &m) : runtime_error(m) {}
};

class settings {
  public:
    // TODO:
    //   need to use the frequency value here because I'm going to allow frequencies
    //   on the cmdline.  Annoying to have to deal with the working notes out in a settings
    //   class tho.  Perhaps settings should take a global_settings object or something?
    //   This would be good because we need to initialise a global state for sdl.
    typedef notes::notes_type notes_type;
    typedef std::vector<notes::notes_type> notes_list_type;

    //! \brief Throws program_options::error subclasses.
    settings(int argc, char **argv) {
      set_defaults();
      parse_args(argc, argv);
    }

    const notes_list_type &notes() const { return notes_; }

    //! \brief How long to play each note for.  Can be \link forever \endlink.
    int duration_ms() const { return duration_; }

    bool loop() const { return flag(fl_loop); }
    int verbosity_level() const { return verbosity_level_; }

    int sample_rate() const { return sample_rate_; }
    int channels() const { return channels_; }

    bool exit() const { return exit_status_ != no_exit; }
    int exit_status() const { return (int) exit_status_; }

    bool dump_to_file() const { return dump_file_.length(); }
    const std::string &dump_file() const { return dump_file_; }

    static const int verbosity_quiet = 0;
    static const int verbosity_normal = 1;
    static const int verbosity_verbose = 2;

    static const int forever = -1;

  private:
    enum exit_code {
      no_exit = -1,
      exit_ok = EXIT_SUCCESS,
      exit_fail = EXIT_FAILURE
    };

    enum exit_code exit_status_;

    enum options {
      fl_loop,
      fl_size
    };
    std::bitset<fl_size> flags_;

    bool flag(enum options o) const { return flags_[o]; }

    notes_list_type notes_;

    int duration_;
    int channels_;
    int sample_rate_;
    int note_distance_;
    int verbosity_level_;
    int pause_time_;

    std::string dump_file_;

    void set_defaults() {
      exit_status_ = no_exit;
      duration_ = DEFAULT_NOTE_DURATION;
      channels_ = DEFAULT_CHANNELS;
      sample_rate_ = DEFAULT_SAMPLE_RATE;
      note_distance_ = DEFAULT_NOTE_DISTANCE;
      pause_time_ = DEFAULT_PAUSE_TIME;
      verbosity_level_ = verbosity_normal;
    }

    void parse_args(int argc, char **argv);
    void print_help(boost::program_options::options_description &d);
    void print_version();
};

#endif
