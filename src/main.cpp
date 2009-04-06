/*!
\file
\brief main()
*/

#include <tune_config.hpp>

#include "sdl.hpp"
#include "settings.hpp"
// #include "data.hpp"

#include <iostream>

#include <cstdlib>
#include <cmath>
#include <cstdio>

#include <para/locking.hpp>
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

// TODO:
//   bleh - data has to be all global an such.  Also, it would be
//   nice to pass the T to this func.  That would solve a lot, and it
//   seems to be a very normal condition... the only problem is that it
//   forces some knowledge of the T, and also that every monitor param
//   needs to have a data() method, which it currently doesn't.  Bind
//   obv works the best.
bool push_continue_predicate() {
  return ! queue.toobig() || quit;
}

//! \brief Abstract locked pushing and flushing the buffer.
//TODO:
//  How could I make this kind of pattern easier to use?  Monitored<T> would be
//  a bit easier, but not much more...
template<class MonitoredType>
class queue_pusher {
  public:
    queue_pusher(SyncType &sync) : flush_(false), sync_(sync) {}

    void flush_next_push() { flush_ = true; }

    //! \brief Blocking operation to push the buffer.
    void push(void *buffer) {
      typedef MonitoredType::mutex_type     mutex_type;
      typedef MonitoredType::lock_type      lock_type;
      // TODO:
      //   Design needs a monitor bind... I guess no time like the present.
      //   Use a sync_tuple, and joing a monitor_sync to it.  Then back-port it to
      //   para.  Should be easy enough.
      //
      //   note: an extension.  Perhaps we sould extend the bind concept
      //   to encapsulate the predicates?  The predicate_bind would take
      //   the monitor bind, though, because there's a chance we'll use
      //   a different bind.  How doe the constructors work like that?
      //   We might end up with inconsistancies...
      typedef MonitoredType::condition_type condition_type;

      typedef monitor<mutex_type, lock_type, condition_type> monitor_type;

      //TODO:
      //  actually, it would be less skippy if we iterate the queue and pop
      //  from the *back*, locking and unlocking each time.

      monitor_type monitor(sync_, push_continue_predicate);
      // would be nicer if I could get this by monitor::data().
      SyncType::value_type &q = sync_.data();
      if (flush_) {
        q.clear();
      }
      q.push(buffer);
      flush_ = false;
    }

  private:
    bool flush_;
    SyncType &sync_;


};

// TODO:
//   there's a slight problem here: list mode obv. requires computing a list
//   while note mode can be generated on the fly.  We're wasting a bit if we
//   do both somehow...
class note_sequence {
  public:
    note_sequence(settings &set) {
      if (set.note_mode() == settings::note_mode_list) {
        // generate a seq. from note_list()
      }
      else {
        assert(set.note_mode() == settings::note_mode_start);
        // and another from
      }

    }
};

// TODO:
//   memory pool this later - return a special auto ptr with a ref to the
//   memory pool perhaps?
class sample_generator {
  public:
    void reset(int64_t time_ms) {
    }

    // TODO: better to have the calculation in the ctor or this func?
    //
    //! \brief Return output samples until the time is fullfiled.
    void *get_samples(since_calculation &calc) {}
    //! \brief Return silence samples until the time is fullfiled.
    void *get_silence() {}
};

//  TODO: again, passing the data here would be really useful.

#include <para/locking.hpp>

// TODO:
//   My conclusion here is to implement it without the class and then generalise
//   it back to this class.
// TODO: port this back to para, incl. this docs to the main doc page..
//
//
// cat <<EOF > docs
// This pattern is necessary when, for some reason, it is not possible to
// join a thread.  For example, when the thread control is obscured by some
// other library like SDL.  It ensures the order:
//
// - th1: flag
// - th2: acknowledge
// - th1: acknowledge
// - th1, th2: continue
//
// The following example describes the usage of monitored_flag:
//
// monitored_flag<
//   boost::mutex,
//   boost::unique_lock<boost::mutex>,
//   boost::condition_variable
// > fl;
//
// void controller_thread() {
//   while (work_exists()) give_work_to_worker_thread();
//
//   // Tell the worker to exit and wait for it to say "ok".
//   fl.flip();
//   // worker is certainly terminated now.
//   exit_program();
// }
//
// void worker_thread() {
//   bool finished = false;
//   while (true) {
//     // TODO:
//     //   no, this won't work because we hit that point multiple
//     //   times.
//     if (finished) { return; }
//     else if (fl.active()) {
//       terminate_gracefully();
//       // tell the controller it's ok to exit now and wait until
//       // they have acknowledged.
//       fl.flip();
//       // Don't hit the activate again.
//       finished = true;
//       return;
//     }
//
//     process_work_given();
//   }
// }
// EOF
//
// Observations:
// - isn't it basically a semaphore?
//   - check this
// - could it be done more effictively with two condition vars?
//   - no, I think you always need the flag.
// - all I have here is a monitored boolean.  The controller could just wait
//   on a condition
// - this would be a lot faster with atomic variable.  The sync_ would not
//   be needed, but the condition might be?  Or we could just spin.. hm.
//   This is a tricky proposition.
// - it's error prone.  You can use the wrong call in the wrong function
//   and it all breaks horribly.
#if 0
template<class Mutex, class Lock, class Condition>
class monitored_flag {
  public:
    typedef Mutex     lockable_type;
    typedef Lock      lock_type;
    typedef Condition condition_type;

    monitored_flag() : sync_(false), monitor_sync_(sync_) {}

    //! \brief Has the switch been flipped?
    bool activated() const {
      lock_type lk(sync_.mutex());
      return sync_.data();
    }

    //! \brief Flip the flag and wait for it to change back.
    void flip() {
      lock_type lk(sync_.mutex());
      sync_.data() = ! sync_.data();
      monitor_sync_.notify_one();
      wait_for_change(lk);
    }

  private:
    void wait_for_change(lock_type &lk) {
      // TODO:
      //   problem.  We duplicate the code of a monitor here.  It seems
      //   we find the need to a monitor to be able to take over an existing
      //   lock.  Maybe this indicates the necesity for a monitor_head
      //   function?   Or a lockless monitor which takes an existing lock?
      //
      //   lockless_monitor mon(lk, sync, pred);
      //
      //   That's a decent idea.
      bool current = sync_.data();
      while (sync_.data() == current) {
        monitor_sync_.wait_condition().wait(lk);
      }
    }

    para::sync_tuple<int, mutex_type> sync_;
    para::monitor_bind<condition_type> monitor_sync_;
};
#endif

int main(int argc, char **argv) {
  try {
    settings set(argc, argv);
    if (set.exit()) {
      return set.exit_status();
    }

    // declare this quick because it does a lot of validation
    note_sequence note_seq(set);

    sdl::audio aud;
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

    sine_calculation calc(dev.obtained().channels(), dev.obtained().sample_rate());

    // we need to take a lot of stuff into account when calculating the right time
    // period of buffer.
    sample_generator buffer(set.duration(), dev.buffer_size(), dev.buffer_samples(), dev.buffer_size());
    if (set.dump_to_file()) {
      buffer.set_file(set.dump_file());
    }

    int16_t samples = NULL;
    queue_pusher pusher;
    do {
      note_seq::iterator i = note_seq.begin() ...
      while (i != note_seq.end()) {
        freq = note_seq.next_frequency (or *i if I get that wokring)
        // can use iterator?
        calc.reset_wave(freq);
        buffer.reset(set.duration_ms());

        samples = NULL;
        while ((samples = buffer.get_samples(calc)) != NULL) {
          pusher.push(samples);

          // TODO: how keypress?
          if (keypress()) { // nonblocking i/o somehow?
            // flush next time we have a full buffer.
            pusher.flush_next_push();
            break;
          }
        }

        if (set.pause_ms()) {
          buffer.reset(set.pause_ms());

          while ((samples = buffer.get_silence()) != NULL) {
            pusher.push(samples);
            if (keypress()) {
              break;
            }
          }
        }
      }
    } while (set.loop());

    // final period
    samples = buffer.get_silence();
    if (samples) {
      pusher.push(buffer.get_silence())
    }

    // TODO:
    //   Generalise this pattern as monitored_flag (see above).  (First I need the quit
    //   strategy in the SDL thread).
    boost::unique_lock<boost::scoped_lock> lk;
    quit = true;
    quit_cond.notify_one();

    // while the sdl thread hasnt flipped it back again
    while (quit == true) {
      quit_cond.wait(lk);
    }
    lk.unlock();

    // Avoid needlessly calling the output while we're shutting down
    dev.pause()

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
