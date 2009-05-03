/*!
\file
\brief Test of the private implementation of the note_sequence.
*/

#include "../src/note_sequence.hpp"
#include "../src/notes.hpp"

#include <cstdlib>
#include <cassert>

int main() {
  // Simple 0..12 step 1.
  {
    double concert_pitch = 440.0;
    int start = 0;
    int stop =  12;
    int step = 1;
    detail::generated_sequence gs(concert_pitch, start, stop, step);
    unsigned int count = 0;
    while (! gs.done()) {
      double freq = gs.next_frequency();
      double expected = offset_to_frequency(concert_pitch, count);
      assert(freq == expected);
      ++count;
    }
    // 13 notes from 0..12.
    assert(count == 13);
  }

  // TODO:
  //   test generated sequence with start == end,

  // TODO:
  //   test generated seq. with step == 0

  // TODO:
  //   test generated sequence with a different concert pitch.

  {
    // detail::listed_sequence ls();
    // TODO: test this.
  }

  return EXIT_SUCCESS;
}
