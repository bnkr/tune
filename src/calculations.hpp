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
    : output_frequency_(output_frequency), amplitude_(amplitude), sine_pos_(0), sine_speed_(0) { }

    //! \brief Set sound properties and note properties; recalculate state.
    void reset(double output_frequency, double note_frequency, double amplitude) {
      reset_sound(output_frequency);
      reset_wave(note_frequency, amplitude);
      reset_state();
    }

    //! \brief Set stuff related to the sound output, sample size etc.
    //! You *must* call reset_state() after this or the changes won't take affect.
    void reset_sound(double frequency) {
      output_frequency_ = frequency;
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
    SampleUnit next_sample() {
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
    double amplitude_;

    double sine_pos_;
    double sine_speed_;
};

#include <cmath> // nearbyint
#include <algorithm> // max()

// TODO:
//   memory pool this later - return a special auto ptr with a ref to the
//   memory pool perhaps?
// TODO:
//   need sample rate and stuff.
//! \brief Keep popping correct-sized buffers until we've made up the right timespan of sinewaves.
class sample_generator {
  public:
    sample_generator(sine_calculation &calc, const sdl::audio_spec &spec)
    : calc_(calc), channels_(spec.channels()), buffer_size_(spec.buffer_size()),
      buffer_samples_(spec.buffer_samples() * spec.channels()), buffer_index_(0),
      total_samples_(0) {
      buffer_ = (uint8_t*)std::malloc(spec.buffer_size());
    }
    ~sample_generator() { std::free(buffer_); }


    //! \brief Change the remaining time to play the sine wave.
    void reset_time(int64_t time_ms) {
      // trc("reset buffer to " << time_ms << " ms");
      assert(time_ms > 0);
      // this could be optimised - we are recalculating the buffer size every time here.  Better to just
      // use reset_bytes()

      total_samples_ = nearbyint((time_ms * 44100) / 1000);
      // trc("total_samples: " << total_samples_);

      // TODO: due to rounding errors (?) this equality doesn't always hold.
      wassert_eq((total_samples_ * 1000) / 44100, time_ms);

    }

    // TODO:
    //   these get_ functions should take a functor which does the pushing, instead of
    //   us pulling from here and then pushing back again.

    //! \brief Return output samples until the time is fullfiled.
    void *get_samples() {
      // trc("get a sample period.  Index = " << buffer_index_ << " / " << buffer_size_);

      // TODO: different sized samples?

      int16_t *samples = (int16_t *) buffer_;
      while (buffer_index_ < buffer_samples_) {
        int16_t samp = calc_.next_sample<int16_t>();
        for (std::size_t ch = 0; ch < channels_; ++ch) {
          samples[buffer_index_ ++] = samp;
        }
        total_samples_--;

        if (total_samples_ == 0) {
          return NULL;
        }
      }

      return reset();
    }

    //! \brief Return silence samples until the time is fullfiled.
    void *get_silence() {
      // TODO: use the sdl audio silence value (not that it matters since we're doing it with bytes)
      const int16_t silence_value = 0;

      // TODO: it's a recalculation to work out bytes here.
      assert(buffer_samples_ >= buffer_index_);
      const uint32_t available = buffer_samples_ - buffer_index_;
      const uint32_t silence_samples = std::min(available, total_samples_);
      const std::size_t silence_bytes = silence_samples * sizeof(int16_t);

      // trc("left over bytes in the buffer: " << available);
      // trc("total samples to fill: " << total_samples_);
      // trc("samples to write: " << silence_samples);
      // trc("bytes to write: " << silence_bytes);

      assert(total_samples_ >= silence_samples);

      int16_t *samples = (int16_t*) buffer_;
      void *start = samples + buffer_index_;
      std::memset(start, silence_value, silence_bytes);
      total_samples_ -= silence_samples;

      if (total_samples_ == 0) {
        return NULL;
      }
      else {
        return reset();
      }
    }

  protected:
    //! \brief Reset and get buffer.
    void *reset() {
      void *b = buffer_;
      buffer_ = (uint8_t*) std::malloc(buffer_size_);
      assert(buffer_ != NULL);
      buffer_index_ = 0;
      return b;
    }

  private:
    sine_calculation &calc_;
    unsigned int channels_;
    const std::size_t buffer_size_;
    // complete size in samples of int16_t
    std::size_t buffer_samples_;

    uint8_t *buffer_;
    // TODO:
    //   Organise this better.  Depending on how we end up doing different sized sample outputs.  We
    //   only have available 8 and 16 bit samples, but they can be unsigned.  Not sure how that works
    //   so have to work it out.  Might be that I need to have a void* buffer and perform the counting
    //   by some magic, and use memset to apply the samples.

    // index up to buffer_samples * channels_, *not* buffer_size.
    std::size_t buffer_index_;
    // Samples per period.
    uint32_t total_samples_;

};

#endif
