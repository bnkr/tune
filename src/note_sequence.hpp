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

// TODO:
//   there's a slight problem here: list mode obv. requires computing a list
//   while note mode can be generated on the fly.  We're wasting a bit if we
//   do both somehow...
//! \brief Sequence of note frequencies based on settings note_list() or start_note().
//! This also does a lot of validation of settings which is left out of the settings
//! class because it's techincal stuff to do with calculating the frequencies.
class note_sequence {
  public:
    note_sequence(settings &set) {
      if (set.note_mode() == settings::note_mode_list) {
        // generate a seq. from note_list()
      }
      else {
        assert(set.note_mode() == settings::note_mode_start);
        trc("using a start note and distance");
      }

      steps_away_from_a = parse_note(set.start_note().c_str());
      trc(steps_away_from_a);
    }

    bool done() { return false; }
    double next_frequency() {
      const double frequency_of_a = 440.0;
      double x = pow(2, (steps_away_from_a/12.0)) * frequency_of_a;
      steps_away_from_a += 5;
      return x;
    }

    int steps_away_from_a; // = 0;
};


#endif
