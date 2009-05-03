#include "../src/calculations.hpp"

#include <cstdlib>
#include <cassert>

// TODO:
//   test that the time => bytes calculation is working
//
// TODO:
//   test that a partial buffer gets appended to, not overwritten (for silence
//   and for normal).  Also use a sentinel to check that no data is overrun.
//   We prolly need to make a subclass for this stuff.
//
// TODO:
//   test what happens with <= 0 ms buffer length (we need an infinite buffer
//   somehow)

int main() {
  return EXIT_SUCCESS;
}
