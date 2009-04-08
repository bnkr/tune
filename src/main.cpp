/*!
\file
\brief main()
*/

#include <tune_config.hpp>

#include "sdl.hpp"
#include "settings.hpp"
#include "calculations.hpp"
#include "note_sequence.hpp"
#include "sync_data.hpp"

#include <iostream>

#include <cstdlib>



// TODO:
//   Monitor predicates.  All data has to be global.  This is arghness,
//   but how do we really fix it?
//
//   - pass data to the predicate
//     - means monitor needs to store sync_type
//     - means we can't use the sync adaptors any more
//   - use monitored<T>
//     - first design was not very successful.
//     - could use bind to pass the syncdata.
//
//   For now I will do it the global way, and try to make a monitored T which
//   will fix the problems.



// another messy global... perhaps those methods should be part of the
// synced struct?
queue_pusher<sync_queue_type> *qp = NULL;

void reader_callback(void *, uint8_t *stream, int length) {
  assert(qp);
  // oops no good - we don't always pop something... maybe we
  // just keep going until null?  Thne a lot of problems
  // evaporate!
  void *buf = qp->pop();
  if (! buf) {
    return;
  }

  std::memcpy(stream, buf, length);
}

int main(int argc, char **argv) {
  try {
    settings set(argc, argv);
    if (set.exit()) {
      return set.exit_status();
    }

    // declare this quick because it does a lot of validation
    note_sequence note_seq(set);

    sdl::audio aud;
    sdl::audio_spec out_spec(reader_callback);
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

    sine_calculation calc(dev.obtained().channels(), dev.obtained().frequency());

    // we need to take a lot of stuff into account when calculating the right time
    // period of buffer.
    sample_generator buffer(calc, set.duration_ms(), dev.obtained()); // , dev.obtained().buffer_samples(), dev.obtained().buffer_size());
    if (set.dump_to_file()) {
      buffer.set_file(set.dump_file());
    }

    int16_t samples = NULL;

    // TODO: could be nicer as a global which carries all the sync data.
    queue_pusher<sync_queue_type> pusher(queue);
    qp = &pusher;
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
    //   Generalise this pattern as monitored_flag (monitored_flag.hpp).  (First I need
    //   the quit strategy in the SDL thread).  I will use the standard way first.
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
