/*!
\file
\brief Command-line option parsing.
*/
#ifndef SETTINGS_HPP_g0vr9k40
#define SETTINGS_HPP_g0vr9k40

#include <vector>
#include <string>
#include <bitset>
#include <stdexcept>
#include <iostream>

#include <cstdlib>
#include <cassert>

// so we can string it in the help text
#define DEFAULT_NOTE_DURATION     2000
#define DEFAULT_NOTE_DURATION_STR "2000"
#define DEFAULT_CHANNELS          2
#define DEFAULT_CHANNELS_STR      "2"
#define DEFAULT_SAMPLE_RATE       44100
#define DEFAULT_SAMPLE_RATE_STR   "44100"
#define DEFAULT_NOTE_DISTANCE     2
#define DEFAULT_NOTE_DISTANCE_STR "2"
#define DEFAULT_PAUSE_TIME        500
#define DEFAULT_PAUSE_TIME_STR    "500"
#define DEFAULT_VOLUME_INT        75
#define DEFAULT_VOLUME_STR        "75"

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

//! \brief Container for cli-data.
class settings {
  public:
    typedef std::vector<std::string> notes_list_type;

    typedef enum {
      //! \brief Use note_list()
      note_mode_list,
      //! \brief use start_note() and note_distance().
      note_mode_start} note_mode_type;

    //! \brief Throws program_options::error subclasses or invalid_setting for validation.
    settings(int argc, char **argv) {
      set_defaults();
      parse_args(argc, argv);
    }

    //! \name Program operations
    //@{
    note_mode_type note_mode() const { return note_mode_; }

    int verbosity_level() const { return verbosity_level_; }

    bool exit() const { return exit_status_ != no_exit; }
    int exit_status() const { return (int) exit_status_; }

    bool dump_to_file() const { return ! dump_file_.empty(); }
    const std::string &dump_file() const { return dump_file_; }
    //@}

    //! \name Regadring the explicit note list.
    //@{
    // \brief Use this to decide whether to use the notes() or start_note().
    const notes_list_type &note_list() const { return notes_; }
    //@}

    //! \name Regarding the start to distance, step num_steps mode
    //@{

    //! \brief Textual representation of which note we start playing on.
    const std::string &start_note() const { return start_note_; }
    //! \brief Note that *both* this and num_notes() may be invalid.
    const std::string &end_note() const { return end_note_; }
    //! \brief Distance between notes in steps.
    int note_increment() const { return note_distance_; }
    //! \brief How many note_increment()'s to make.  -1 if not given.  Can be zero.
    int num_note_increments() const { return num_increments_; }
    //@}

    //! \deprecated
    int num_notes() const { return num_note_increments(); }
    //! \deprecated
    int note_distance() const { return note_distance_; }

    //! \name Regarding the `playlist'
    //@{

    //! \brief How long to play each note for.  Can be \link forever \endlink.
    int duration_ms() const { return duration_; }
    bool loop() const { return flag(fl_loop); }
    //! \brief Pause between notes.
    int pause_ms() const { return pause_time_; }
    //@}

    //! \name Regarding the sound output.
    //@{
    int sample_rate() const { return sample_rate_; }
    int channels() const { return channels_; }
    double amplitude() const { return volume_ / 100.0; }
    int volume() const { return volume_; }
    //@}

    //! \name Regarding technicalities of music.
    //@{
    double concert_pitch() const { return concert_pitch_; }
    //@}

    static const int verbosity_quiet = 0;
    static const int verbosity_normal = 1;
    static const int verbosity_verbose = 2;

    static const int forever = -1;

    std::ostream &dump_note_settings(std::ostream &o) {
      if (note_mode() == settings::note_mode_list) {
        o << "Playing notes from a list: " << std::endl;
        // TODO: print the list (and frequency conversions?)
      }
      else {
        assert(note_mode() == settings::note_mode_start);
        o << "Starting with: " << start_note() << std::endl;
        o << "Incrementing by " << note_distance() << " half steps." << std::endl;

        if (! end_note().empty()) {
          o << "Ending with: " << end_note() << std::endl;
        }
        else if (num_notes() > 1) {
          o << "Incrementing " << num_notes() << " times." << std::endl;
        }
        else {
          o << "Incrementing until an octave is reached." << std::endl;
        }
      }

      o << "Concert pitch is: " << concert_pitch() << "hz." << std::endl;
      o << "Notes last for: " << duration_ms() << "ms." << std::endl;
      o << "Pause for: " << pause_ms() << "ms after each note." << std::endl;
      o << "Looping: " << loop() << std::endl;
      o << "Amplitude is: " << amplitude();

      return o;
    }


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
    int num_increments_;
    int volume_;
    double concert_pitch_;

    std::string start_note_;
    std::string end_note_;

    note_mode_type note_mode_;

    std::string dump_file_;

    void set_defaults() {
      exit_status_ = no_exit;
      duration_ = DEFAULT_NOTE_DURATION;
      channels_ = DEFAULT_CHANNELS;
      sample_rate_ = DEFAULT_SAMPLE_RATE;
      note_distance_ = DEFAULT_NOTE_DISTANCE;
      pause_time_ = DEFAULT_PAUSE_TIME;
      verbosity_level_ = verbosity_normal;
      volume_ = DEFAULT_VOLUME_INT;
      note_mode_ = note_mode_list;
      num_increments_ = -1;
      concert_pitch_ = 440.0;
    }

    void parse_args(int argc, char **argv);
    void print_help(boost::program_options::options_description &d);
    void print_version();
};

#endif
