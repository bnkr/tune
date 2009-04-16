/*!
\file
\brief main()
*/

#include <bdbg/trace/short_macros.hpp>
#include <bdbg/trace/static_definitions.hpp>

#include <tune_config.hpp>

#include "sdl.hpp"
#include "settings.hpp"
#include "calculations.hpp"
#include "note_sequence.hpp"
#include "sync_data.hpp"
#include "key_reader.hpp"

#include <iostream>

#include <cstdlib>
#include <cstring>
#include <csignal>

#include <bdbg/trace/crash_detection.hpp>

bdbg::trace::crash_detector cd;


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


// TODO:
//   when ctrl+c happens, exit more safely and play some short silence at the
//   end.

#include <string>

// TODO: pretty messy
class sample_dumper {
  public:
    sample_dumper(bool enabled, const std::string &filename, std::size_t buffer_size)
    : enabled_(enabled), buffer_size_(buffer_size), out_(filename.c_str(), std::ios::out|std::ios::trunc) {}

    void dump(const void *buf) {
      if (! enabled_) return;
      out_.write((const char*) buf, buffer_size_);
    }

  private:
    bool enabled_;
    std::size_t buffer_size_;
    std::ofstream out_;
};


// another messy global... perhaps those methods should be part of the
// synced struct?  More discussion about that on the sunc_data header.
// It is an interesting observation that we basically have the same
// bool get(T &ret) format that an LFDS has.  We can generalise this as
// an adaptor pattern which locks a stl container.  Give them a functor
// for whether to push/pop/push_back etc.
queue_pusher<sync_queue_type> *qp = NULL;

void reader_callback(void *, uint8_t *stream, int length) {
  // argh! horrible messy - means  we didn't set up properly yet!
  if (! qp) return;
  // use quitting and quit_cond somehow :)
  //
  //
  // oops no good - we don't always pop something... maybe we
  // just keep going until null?  Thne a lot of problems
  // evaporate!

  // TODO: use lfds style:
  //
  //   void *buf;
  //   if (! qp->pop(buf)) ...
  //
  //   If we could lib this up into a generic sync_container or something
  //   it would be really good.
  // null buffer means it's empty
  void *buf = qp->pop();
  if (! buf) {
    // trc("no data!");
    // rather messy.

    boost::mutex::scoped_lock lk(quit_mutex);
    if (quitting) {
      trc("notify quit");
      terminated = true;
      quit_cond.notify_one();
      lk.unlock();
      // avoids popping - it would prbably be better to cuause this thread to wait I
      // guess?
      std::memset(stream, 0, length);
    }
    else {
      std::cerr << "warning: buffer underflow - computer to slow?!" << std::endl;
    }
    return;
  }

  std::memcpy(stream, buf, length);
  std::free(buf);
}



bool interrupt = false;

void notify_interrupt(int) {
  // TODO: timeout
  if (interrupt) {
    std::cerr << "error: double interrupt!  Aborting now..." << std::endl;
    abort();
  }

  std::cout << "Interrupted.  Press again if it doesn't work." << std::endl;
  interrupt = true;
}

int main(int argc, char **argv) {
  try {
    settings set(argc, argv);
    if (set.exit()) {
      return set.exit_status();
    }

    if (set.verbosity_level() >= set.verbosity_verbose) {
      set.dump_note_settings(std::cout) << std::endl;
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

    // TODO:
    //   could be nicer as a global which carries all the sync data - see sync_data.hpp
    //   there are other ways it could be done..
    queue_pusher<sync_queue_type> pusher(queue);
    qp = &pusher;

    dev.unpause();

    trc("amp: " << set.amplitude());
    trc("vol: " << set.volume());

    sine_calculation calc(dev.obtained().channels(), dev.obtained().frequency(), set.amplitude());

    if (set.verbosity_level() >= set.verbosity_verbose) {
      std::cout << "volume: " << set.volume() << std::endl;
    }

    sample_generator buffer(calc, dev.obtained());
    sample_dumper dump_file(set.dump_to_file(), set.dump_file(), dev.obtained().buffer_size());

    key_reader keys;

    signal(SIGINT, notify_interrupt);
    void *samples = NULL;
    do {
      trc("begin loop");
      // note_seq::iterator i = note_seq.begin() ...
      while (! note_seq.done()) {
        trc("get next freq.");
        double freq = note_seq.next_frequency(); // (or *i if I get that wokring)
        // can use iterator?
        trc("note " << freq << " for " << set.duration_ms() << "ms");
        calc.reset_wave(freq);
        buffer.reset_time(set.duration_ms());

        trc("note: " << freq);
        // TODO: much neater to pass a functor to do something whith each of the buffers.
        while ((samples = buffer.get_samples()) != NULL) {
          pusher.push(samples);
          dump_file.dump(samples);

          if (keys.pressed()) {
            // flush next time we have a full buffer.
            pusher.flush_next_push();
            break;
          }
          else if (interrupt) {
            pusher.flush_next_push();
            goto clean_exit;
          }
        }

        if (set.pause_ms()) {
          trc("pause between notes");
          buffer.reset_time(set.pause_ms());

          while ((samples = buffer.get_silence()) != NULL) {
            pusher.push(samples);
            dump_file.dump(samples);
            if (keys.pressed()) {
              break;
            }
            else if (interrupt) {
              goto clean_exit;
            }
          }
        }

        trc("finished this note");
      }
      trc("finished this sequence");
    } while (set.loop());

clean_exit:
    trc("clean exit");

    // final period
    samples = buffer.get_silence();
    if (samples) {
      pusher.push(samples);
    }

    trc("lock");

    // TODO:
    //   Generalise this pattern as monitored_flag (monitored_flag.hpp).  (First I need
    //   the quit strategy in the SDL thread).  I will use the standard way first.
    boost::unique_lock<boost::mutex> lk(quit_mutex);
    quitting = true;
    quit_cond.notify_one();

    trc("wait for thread to finish nicely");

    // while the sdl thread hasnt flipped it back again
    while (terminated == false) {
      quit_cond.wait(lk);
    }
    lk.unlock();

    // Avoid needlessly calling the output while we're shutting down
    dev.pause();

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
