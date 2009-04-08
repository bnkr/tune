/*!
\file
\brief Regarding calculating the stuff.
*/
#ifndef CALCULATIONS_HPP_kxk4jjvl
#define CALCULATIONS_HPP_kxk4jjvl

#include <cstdlib>
#include <cmath>
#include <limits>

#include "sdl.hpp"

//! \brief Stateful calculation context.
class sine_calculation {
  public:
    sine_calculation(int channels, double output_frequency, double amplitude = 0.75)
    : channels_(channels), output_frequency_(output_frequency), amplitude_(amplitude), sine_pos_(0), sine_speed_(0) { }

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
    int channels_;
    double output_frequency_;

    double note_frequency_;
    double amplitude_;

    double sine_pos_;
    double sine_speed_;
};



// TODO:
//   memory pool this later - return a special auto ptr with a ref to the
//   memory pool perhaps?
// TODO:
//   need sample rate and stuff.
//! \brief Keep popping correct-sized buffers until we've made up the right timespan of sinewaves.
class sample_generator {
  public:
    sample_generator(sine_calculation &calc, const sdl::audio_spec &spec)
    : calc_(calc), buffer_length_(spec.buffer_size()), buffer_samples_(spec.buffer_samples()), buffer_index_(0) {
      buffer_ = std::malloc(spec.buffer_size());
    }
    ~sample_generator() { std::free(buffer_); }

    void reset(int64_t time_ms) {
    }

    //! \brief Return output samples until the time is fullfiled.
    void *get_samples() {
      return NULL;
    }

    //! \brief Return silence samples until the time is fullfiled.
    void *get_silence() {
      return NULL;
    }

  private:
    sine_calculation &calc_;
    std::size_t buffer_length_;
    std::size_t buffer_samples_;

    void *buffer_;
    std::size_t buffer_index_;

};

#endif
