#include "../src/calculations.hpp"

#include <cstdlib>
#include <bdbg/trace/static_definitions.hpp>

int main() {
  double frequency = 44100;
  double amplitude = 0.75;
  double note_freq = 220;

  // Check zero amplitude works.
  {
    amplitude = 0;
    sine_calculation sc(frequency, amplitude);
    sc.reset_sound(note_freq);

    std::size_t v = 500;
    while (v--) {
      int16_t sample = sc.next_sample<int16_t>();
      assert(sample == 0);
    }
  }

  // Now amplitide as 0.5 is within the range of the type.
  {
    amplitude = 0.5;
    sine_calculation sc(frequency, amplitude);
    sc.reset_sound(note_freq);

    int16_t max = amplitude * std::numeric_limits<int16_t>::max(), min = -max;

    std::size_t v = 500;
    while (v--) {
      int16_t sample = sc.next_sample<int16_t>();

      assert(sample <= max);
      assert(sample >= min);
    }
  }

  // TODO:
  //   Test with an unsigned type.  I'm pretty sure this won't actually work!
  //   Mostly because I don't know what unsigned samples are supposed to look
  //   like.

  // Manipulate the sine speed to be 1 and make sure for each iteration, the
  // output is sin(iter).
  {
    amplitude = 1;
    double note_frequency = 1;
    // so we get x/x = 1
    frequency = 2 * M_PI * note_frequency;

    sine_calculation sc(frequency, amplitude);
    sc.reset_wave(note_frequency);

    std::size_t i = 0;
    while (i < 500) {
      int16_t sample = sc.next_sample<int16_t>();
      int16_t expected = std::sin(i) * std::numeric_limits<int16_t>::max();
      assert(sample == expected);
      ++i;
    }
  }

  return EXIT_SUCCESS;
}
