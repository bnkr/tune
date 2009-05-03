#include "../src/notes.hpp"

#include <cstdlib>
#include <cassert>

int main() {
  double concert_pitch = 440.0;
  assert(offset_to_frequency(concert_pitch, 0) == 440.0);
  assert(offset_to_frequency(concert_pitch, 12) == 880.0);
  assert(offset_to_frequency(concert_pitch, -12) == 220.0);

  assert(nearbyint(offset_to_frequency(concert_pitch, 1)) == 466);
  assert(nearbyint(offset_to_frequency(concert_pitch, 2)) == 494);

  concert_pitch = 220.0;
  assert(offset_to_frequency(concert_pitch, 0) == 220.0);
  assert(offset_to_frequency(concert_pitch, -12) == 110.0);
  assert(offset_to_frequency(concert_pitch, 12) == 440.0);
  return EXIT_SUCCESS;
}
