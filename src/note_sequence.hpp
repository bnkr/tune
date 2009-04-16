/*!
\file
\brief Based on settings, compute a sequence of frequencies.
*/
#ifndef NOTE_SEQUENCE_HPP_1mte9pqi
#define NOTE_SEQUENCE_HPP_1mte9pqi

#include "settings.hpp"

#include <cmath>
#include <cassert>
#include <cstring>

#include <stdexcept>
#include <memory>

#ifndef trc
#  define trc(x)
#endif

//! \brief Turn a note into steps-from-concert-a.
int parse_note(const char *note_str) {
  assert(note_str);
  std::size_t s = std::strlen(note_str);
  assert(s > 0);

  int name = note_str[0];
  if (name >= 'a' && name <= 'g') {
    name -= 0x20;
  }

  if (name < 'A' || name > 'G') {
    // TODO: better exception
    throw std::runtime_error("bad note");
  }

  int dists[] = {
     0, // a
     2, // b
     4, // c
     5, // d
     7, // e
     9, // f
    10, // g
  };

  name -= 'A';
  // add sharps
  name = dists[name];
  // trc("distance to a: " << name);

  ++note_str;
  if (*note_str == '\0') {
    return name;
  }
  // TODO: what about e# or fB ?
  else if (*note_str == 'b' || *note_str == 'B') {
    name -= 1;
    ++note_str;
  }
  else if (*note_str == '#') {
    name += 1;
    ++note_str;
  }

  // trc("sharp/flat modified to: " << name);

  while (*note_str != '\0') {
    if (*note_str == '-') {
      name -= 12;
      // trc("take 12 steps for an octave: " << name);
    }
    else if (*note_str == '+') {
      name += 12;
      // trc("add 12 steps for an octave: " << name);
    }
    else {
      // TODO: better exception
      throw std::runtime_error("unknown note modifier");
    }
    ++note_str;
  }

  return name;
}

namespace detail {

  //! \brief Abstract base for the implementation of a note sequence.
  struct sequence_engine {
    virtual ~sequence_engine() {}
    virtual double next_frequency() = 0;
    virtual bool done() = 0;
  };

  //! \brief Sequence based on a start, step, and stop.
  class generated_sequence : public sequence_engine {
    public:
      //! \brief stop = -1 for never.  Step may be 0.
      generated_sequence(double concert_pitch, int start_offset, int stop, int step)
      : concert_pitch_(concert_pitch), start_(start_offset), offset_(start_offset), step_(step), stop_(stop) {
        assert((start_ < stop_ && step_ >= 0) || (start_ > stop_ && step_ <= 0) || (start_ == stop_));
      }

      // TODO: no need for this to be virtual - just assign a bool in sequence_engine.
      bool done() {
        if (offset_ == stop_) {
          trc("this is only one note: it's always done.");
          return false;
        }
        else if (start_ < stop_) {
          return offset_ <= stop_;
        }
        else {
          return offset_ >= stop_;
        }
      }

      double next_frequency() {
        double x = std::pow(2, (offset_/12.0)) * concert_pitch_;
        trc("we're on " << offset_);
        offset_ += step_;
        trc("increment to " << offset_);
        return x;
      }

    private:
      const double concert_pitch_;
      const int start_;
      int offset_;
      int step_;
      const int stop_;
  };

}


//! \brief Sequence of note frequencies based on settings note_list() or start_note().
//! This also does a lot of validation of settings which is left out of the settings
//! class because it's techincal stuff to do with calculating the frequencies.  This
//! class controls a lot of the semantics of the program.
// TODO:
//   would it be faster to do a comparison each time frequencey is called instead of
//   the virtaul function call?  Even using a switch might be nicer due to inlining...
//   have to benchmark it.
//
class note_sequence {
  public:
    note_sequence(settings &set) {
      if (set.note_mode() == settings::note_mode_list) {
        throw std::logic_error("not implemented");
      }
      else {
        assert(set.note_mode() == settings::note_mode_start);
        trc("using a start note and distance");
        int start_offset = parse_note(set.start_note().c_str());

        int stop_offset;
        if (! set.end_note().empty()) {
          stop_offset = parse_note(set.end_note().c_str());
        }
        else if (set.num_notes() <= 0) {
          stop_offset = start_offset + set.num_notes() * set.note_distance();
        }
        else {
          stop_offset = start_offset + 12;
        }

        int step = set.note_distance();
        if (start_offset > stop_offset && step > 0) {
          std::cerr << "warning: making the step negative since the end note is lower than the start note." << std::endl;
          step = -step;
        }
        else if (start_offset < stop_offset && step < 0) {
          std::cerr << "warning: making the step positive since the end note is higher than the start note." << std::endl;
          step = -step;
        }

        trc("start: " << start_offset);
        trc("stop:  " << stop_offset);
        trc("step:  " << step);

        impl_.reset(
          new detail::generated_sequence(
            set.concert_pitch(), start_offset, stop_offset, step
          )
        );
        trc("done");
        assert(impl_.get());
      }
    }

    bool done() { return impl_->done(); }
    double next_frequency() { return impl_->next_frequency(); }

  private:
    std::auto_ptr<detail::sequence_engine> impl_;
};


#endif
