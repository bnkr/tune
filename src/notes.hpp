/*!
\file
\brief Functions for dealing with note offsets and frequencies.
*/

#ifndef NOTES_HPP_la1870xc
#define NOTES_HPP_la1870xc

#include <stdexcept>

#include <cstring>
#include <cassert>
#include <cmath>

//! \brief Turn a note into steps-from-concert-a.
inline int parse_note(const char *note_str) {
  assert(note_str);
  assert(std::strlen(note_str) > 0);

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

//! \brief Based on concert_pitch, find the  note offset half steps away.
inline double offset_to_frequency(double concert_pitch, int offset) {
  return std::pow(2, (offset/12.0)) * concert_pitch;
}

#endif
