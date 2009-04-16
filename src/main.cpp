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

#include <iostream>

#include <cstdlib>
#include <cstring>

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

// TODO: needs to handle the case where we don't want to dump anything :)
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
  void *buf = qp->pop();
  if (! buf) {
    // rather messy.
    if (quitting) terminated = false;
    return;
  }

  std::memcpy(stream, buf, length);
  std::free(buf);
}

#include <fcntl.h>

//! \brief Non-blocking reader of stdin.
//
//TODO:
//   make this portable. maybe I should jsut do it in another thread?
//   boost asio is so complicated for this... you need to set up all kinds of machinary to
//   organise it.
//
//   Thread is easiest because all we need to synchronise is the termination, and we can just
//   join this thread sinec we control it.
class key_reader {
  public:
    key_reader() {
      int flags = fcntl(fileno(stdin), F_GETFL, fileno(stdin));
      flags |= O_NONBLOCK;
      fcntl(fileno(stdin), F_SETFL, flags);
    }

    ~key_reader() {
      int flags = fcntl(fileno(stdin), F_GETFL, fileno(stdin));
      flags &= ~O_NONBLOCK;
      fcntl(fileno(stdin), F_SETFL, flags);
    }

    bool pressed() {
      const std::size_t buf_sz = 1024;
      char buf[buf_sz];
      ssize_t v = read(fileno(stdin), buf, 1);
      if (v == -1) {
        if (errno == EAGAIN) {
          trc("key.pressed(): op would block.");
          return false;
        }
        else {
          trc("key.pressed(): another kind of error.")
          // arses.
          return false;
        }
      }
      else {
        // purge the stream.
        do {
          v = read(fileno(stdin), buf, buf_sz);
        }
        while (v);
        trc("key.pressed(): there was stuff in the buffer - a key was pressed.")
        return true;
      }
    }
};

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

    trc("amp: " << set.amplitude());
    trc("vol: " << set.volume());

    sine_calculation calc(dev.obtained().channels(), dev.obtained().frequency(), set.amplitude());

    if (set.verbosity_level() >= set.verbosity_verbose) {
      std::cout << "volume: " << set.volume() << std::endl;
    }

    sample_generator buffer(calc, dev.obtained());
    sample_dumper dump_file(set.dump_to_file(), set.dump_file(), dev.obtained().buffer_size());

    key_reader keys;

    void *samples = NULL;
    // TODO:
    //   could be nicer as a global which carries all the sync data - see sync_data.hpp
    //   there are other ways it could be done..
    queue_pusher<sync_queue_type> pusher(queue);
    qp = &pusher;
    do {
      trc("begin loop");
      // note_seq::iterator i = note_seq.begin() ...
      while (! note_seq.done()) {
        double freq = note_seq.next_frequency(); // (or *i if I get that wokring)
        // can use iterator?
        trc("note " << freq << " for " << set.duration_ms() << "ms");
        calc.reset_wave(freq);
        buffer.reset_time(set.duration_ms());

        trc("note: " << freq);
        // TODO: much neater to pass a functor to do something whith each of the buffers.
        while ((samples = buffer.get_samples()) != NULL) {
          trc("got " << samples);
          pusher.push(samples);
          dump_file.dump(samples);
          trc("pushed " << samples);

          // TODO: how keypress formed?
          if (keys.pressed()) { // nonblocking i/o somehow?
            // flush next time we have a full buffer.
            pusher.flush_next_push();
            break;
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
          }
        }
      }
    } while (set.loop());

    // final period
    samples = buffer.get_silence();
    if (samples) {
      pusher.push(samples);
    }

    // TODO:
    //   Generalise this pattern as monitored_flag (monitored_flag.hpp).  (First I need
    //   the quit strategy in the SDL thread).  I will use the standard way first.
    boost::unique_lock<boost::mutex> lk(quit_mutex);
    quitting = true;
    quit_cond.notify_one();

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
