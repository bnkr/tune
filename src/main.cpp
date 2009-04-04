/*!
\file
\brief main()
*/

// TODO:
//   move this and the sdl wrapper to the main tune repos.  Hopefully I can keep
//   the sdl wrapper synced in case of bugs...

#include <tune_config.hpp>
#include "sdl.hpp"
#include "settings.hpp"
// #include "data.hpp"

#include <iostream>

#include <cstdlib>
#include <cmath>
#include <cstdio>

#include <boost/thread.hpp>

#include <limits>

//! \brief Stateful calculation context.
class sine_calculation {
  public:
    sine_salculation(int channels, double output_frequency, double amplitide = 0.75)
    : channels_(channels), output_frequency_(output_frequency), amplitide_(amplitude), sine_pos_(0), sine_speed_(0) { }

    //! \brief Set sound properties and note properties; recalculate state.
    void reset(int channels, double output_frequency, double note_frequency, double amplitude) {
      reset_sound(channels, output_frequency);
      reset_wave(note_frequency, amplitude);
      reset_state();
    }

    //! \brief Set stuff related to the sound output, sample size etc.
    //! You *must* call reset_state() after this or the changes won't take affect.
    void reset_sound(int channels, double frequency) {
      output_frequency_ = frequency;
      channels_ = channels;
    }

    //! \brief Set stuff related to the sine wave.  reset_state() is called.
    void reset_wave(double frequency, double amplitude) {
      note_frequency_ = frequency;
      amplitude_ = amplitude;
      reset_state();
    }

    //! \brief Reset the sine wave without changing the amplitude.
    void reset_wave(double frequency) {
      note_frequency_ = frequency;
      reset_state();
    }

    //! \brief Based on the properties, recalculate the speed and set sine position to 0.
    void reset_state() {
      sine_pos_ = 0;
      sine_speed_ = 2 * M_PI * note_frequency_ / output_frequency_;
    }

    //! \brief Normalised sample using std::numeric_limits.
    template <class SampleUnit>
    SampleUnit next_sample() const {
      SampleUnit ret = y() * std::numeric_limits<SampleUnit>::max();
      increment();
      return ret;
    }

  protected:
    //! \brief Value of y between -1 and 1.
    double y() const {
      return amplitude_ * std::sin(sine_pos_);
    }

    //! \brief Move to the next x-axis sample position.
    void increment() {
      sine_pos_ += sine_speed_;
    }

  private:
    double output_frequency_;
    double note_frequency_;

    int channels_;
    double amplitude_;

    double sine_pos_;
    double sine_speed_;
};

// TODO:
//   look at the manpage for stuff I need to support.  The major one is
//   using frequencies directly as notes, especially with --start.

// TODO:
//   a handy thing would be an --offset parameter which gives a frequency
//   skew for when the tuning is slightly off.  Problem is it's logarithmic
//   so I don't know it... I guess you need to give concert pitch version
//   of A since we work out all our notes from that?

// TODO: monitor properly
bool finished = false;
boost::mutex finished_mutex;
boost::condition_variable finished_cond;

// really the periods should be generated elsewhere and this func
// just pulls them.
void sine_callback(void *, uint8_t *stream, int len) {
  static double sine_pos = 0;

  // TODO:
  //   this should be got from the sdl obj ofc.
  const double sample_frequency = 44100;
  const int channels = 1;

  // TODO:
  //   should be dynamic to... what?  I gues this func
  //   must access a global.  We could use an object which
  //   also contains the audio_spec.
  const double note_frequency = 220.0;
  const double amp = 0.75;

  const double sine_speed = 2 * M_PI * note_frequency / sample_frequency;

  // Interpret the buffer as 16 bit samples.
  // TODO: make this work with an arbitrary size sample (easy with templates)
  std::size_t samples_len = len / sizeof(int16_t);
  int16_t *samples = (int16_t *) stream;
  std::size_t i = 0;
  while (i < samples_len) {
    double y = amp * std::sin(sine_pos);
    int16_t normalised = y * std::numeric_limits<int16_t>::max();
    for (int c = 0; c < channels; ++c) {
      samples[i++] = normalised;
    }
    sine_pos += sine_speed;
  }

  // finished = true;
  // finish_cond.notify_all();
  // boost::thread::yield();
}

void make_sine(uint8_t *stream, int length) {
  sine_callback(NULL, stream, length);
}


void sine_file(const char *filename, std::size_t length) {
  // should be able to specify size etc.
  FILE *fh = fopen(filename, "w");
  uint16_t *samples = (uint16_t *) std::malloc(length * sizeof(int16_t));
  make_sine((uint8_t*)samples, length * sizeof(int16_t) / sizeof(uint8_t));
  fwrite(samples, sizeof(int16_t), length, fh);
  std::free(samples);
  fclose(fh);
}


int main(int argc, char **argv) {
  try {
    settings set(argc, argv);
    if (set.exit()) {
      return set.exit_status();
    }

    // TODO: this should be dynamic - also we aren't looping the values asked for.
    std::size_t miliseconds = 5 * 10;
    std::size_t seconds = miliseconds / 10;

    std::size_t frequency = set.sample_rate();
    std::size_t channels = set.channels();

    std::size_t samples = frequency * seconds;
    std::size_t buffer_size = channels * samples * sizeof(int16_t);

    if (set.dump_to_file()) {
    // sine_file("sinewave.raw", samples);
    }
    //else if (set.start_note()) {
    //}
    else {
    }


    return 0;

    sdl::audio aud;
    // TODO:
    //   put this in a global object along with some other stuff which
    //   I guess has to come from settings...  we could make settings
    //   global I suppose... the *obtained* value is what we need though.
    //   I guess our global will be like:
    //
    //     wave_properties;
    //     // ..
    //     p.initialise(set, dev.obtained());
    sdl::audio_spec out_spec(sine_callback);
    out_spec.frequency(set.sample_rate());
    out_spec.channels(set.channels());
    sdl::device dev(aud, out_spec);
    if (set.verbosity_level() >= set.verbosity_verbose) {
      std::cout << "Audio spec:" << std::endl;
      dev.obtained().dump(std::cout, "  ") << std::endl;
    }

    if (dev.obtained() != out_spec) {
      std::cerr << "warning: could not get the requested audio spec - parameters not supported?" << std::endl;
    }

    dev.unpause();

    // sine_calculation calc(dev.obtained().channels(), dev.obtained().sample_rate());
    // note_sequence note_seq(set);
    //
    //
    // // we need to take a lot of stuff into account when calculating the right time
    // // period of buffer.
    // // TODO: memory pool this later.
    // sample_generator buffer(set.duration(), dev.buffer_size(), dev.buffer_samples(), dev.buffer_size());
    // if (set.dump_to_file()) {
    //   buffer.set_file(set.dump_file());
    // }
    //
    // do {
    //   note_seq::iterator i = note_seq.begin() ...
    //   while (i != note_seq.end()) {
    //     freq = note_seq.next_frequency
    //     // can use iterator?
    //     calc.reset_wave(freq);
    //
    //     samples = NULL;
    //     while ((samples = buffer.get_samples(calc)) != NULL) {
    //       q.push(samples);
    //       if (keypress) { // nonblocking i/o somehow?
    //         flush the buffer
    //         goto next_note;
    //       }
    //     }
    //
    //     // TODO:
    //     //   duped code, also tricky to calculate the right values for silence.  Also, do we
    //     //   want to wait for interrupts here?
    //     //
    //     //   We could cause the interrupt to stop teh silence?  I guess that is right.. we must
    //     //   say this behaviour though because people might think it will go direct to the thingy.
    //
    //
    //     if (set.pause_ms()) {
    //       while ((samples = buffer.get_pause(pause_ms)) {
    //         q.push
    //       }
    //     }
    //   }
    // } while (set.loop());
    //
    //
    // // would be nice to make this a special kind of flipflop monitor for
    // // ipc between two processes: notify a value and wait for the other process
    // // to change it again, then do something as a result (in both cases here
    // // we do nothing).
    // lock quit
    // quit = true
    // notify quit condition
    // unlock quit
    //
    // // while the sdl thread hasnt flipped it back again
    // lock quit lk
    // while (quit == true) {
    //   wait on quit condition(lk)
    // }
    //
    //
    // // Avoid needlessly calling the output while we're shutting down
    // dev.pause()
    //
    //

    // TODO:
    //   somehow set up a list of notes to iterate
    //
    //   and call something like play_notes(container, duration);
    //   also play_single_note(note_id);
    //

    // So how do I switch notes?
    // - Switch it every T ms by timer or something?  Nice because I can
    //   use ctrl-c to change then or even arrow keys to go back and forth.
    // - use any key for next note
    // - also have a time between note mode - could use SDL timer?  Or just
    //   thread sleep easy enough.

    boost::unique_lock<boost::mutex> lk(finished_mutex);
    while (! finished) {
      finished_cond.wait(lk);
    }

    return EXIT_SUCCESS;
  }
  catch (sdl::error &e) {
    std::cerr << "Error initialising sdl: " << e.what() << std::endl;
  }
  // TODO: be more specific
  catch (std::exception &e) {
    std::cerr << "Error parsing arguments: " << e.what() << std::endl;
  }
  return EXIT_FAILURE;
}
