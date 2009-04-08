#ifndef NOTE_SEQUENCE_HPP_1mte9pqi
#define NOTE_SEQUENCE_HPP_1mte9pqi

#include "settings.hpp"

// TODO:
//   there's a slight problem here: list mode obv. requires computing a list
//   while note mode can be generated on the fly.  We're wasting a bit if we
//   do both somehow...
//! \brief Sequence of note frequencies based on settings note_list() or start_note().
class note_sequence {
  public:
    note_sequence(settings &set) {
      if (set.note_mode() == settings::note_mode_list) {
        // generate a seq. from note_list()
      }
      else {
        assert(set.note_mode() == settings::note_mode_start);
        // and another from
      }

    }
};


#endif
