#include "../src/note_sequence.hpp"

#include <cstdlib>
#include <cassert>
#include <iostream>

int main() {
  assert(parse_note("a") == 0);
  assert(parse_note("a#") == 1);
  assert(parse_note("aB") == -1);
  assert(parse_note("e--") == -17);
  assert(parse_note("e+") == 19);
  assert(parse_note("e") == 7);
  assert(parse_note("ab") == -1);

  assert(parse_note("E--") == -17);
  assert(parse_note("A") == 0);
  assert(parse_note("E+") == 19);
  assert(parse_note("A#") == 1);
  assert(parse_note("Ab") == -1);
  assert(parse_note("AB") == -1);

  return EXIT_SUCCESS;
}
