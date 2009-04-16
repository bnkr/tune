/*!
\file
\brief Based on settings, compute a sequence of frequencies.
*/
#ifndef NOTE_SEQUENCE_HPP_1mte9pqi
#define NOTE_SEQUENCE_HPP_1mte9pqi

#include "settings.hpp"
#include "notes.hpp"

#include <boost/lexical_cast.hpp>

#include <cassert>

#include <vector>
#include <stdexcept>
#include <memory>
#include <string>

#ifndef trc
#  define trc(x)
#endif

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
        // Special case: this is the first run through when have start and stop as
        // the same note.  We need to return false at least once so that note does
        // get played!  This also means that it loops forever if the distance is
        // set to zero (which is what we want).
        if (offset_ == stop_ && start_ == stop_) {
          // trc("offset_ == stop_ && start_ == stop_");
          return false;
        }
        else if (start_ < stop_) {
          // trc("start_ < stop_");
          return offset_ > stop_;
        }
        else if (start_ > stop_) {
          // trc("start_ > stop_");
          return offset_ < stop_;
        }
        else if (start_ == stop_) {
          // trc("start_ == stop_");
          return true;
        }
        else {
          // shouldn't get here!
          assert(false);
        }
      }

      double next_frequency() {
        double x = offset_to_frequency(concert_pitch_, offset_);
        // trc("we're on " << offset_);
        offset_ += step_;
        // trc("increment to " << offset_);
        return x;
      }

    private:
      const double concert_pitch_;
      const int start_;
      int offset_;
      int step_;
      const int stop_;
  };

  //! \brief Based on a list of strings
  class listed_sqeuence : public sequence_engine {
    public:
      template<class InputIterator>
      listed_sqeuence(double concert_pitch, InputIterator begin, InputIterator end, std::size_t reserve = 0) {
        frequencies_.reserve(reserve);
        while (begin != end) {
          if (is_floating_point<typename InputIterator::value_type>(*begin)) {
            double f = boost::lexical_cast<double>(*begin);
            frequencies_.push_back(f);
          }
          else {
            int offset = parse_note(begin->c_str());
            frequencies_.push_back(offset_to_frequency(concert_pitch, offset));
          }
          ++begin;
        }
        iter_ = frequencies_.begin();
      }

      // TODO: better to use the iterators directly.
      bool done() {
        return iter_ == frequencies_.end();
      }

      double next_frequency() {
        freq_list_type::iterator i = iter_;
        ++iter_;
        return *i;
      }

      template<class String>
      bool is_floating_point(const String &s) {
        typedef typename String::const_iterator iter_type;
        iter_type i = s.begin();
        bool found_frac = false;
        for (;i < s.end(); ++i) {
          if (*i == '.') {
            if (found_frac) {
              return false;
            }
            else {
              found_frac = true;
            }
          }
          else if (*i < '0' || *i > '9') {
            return false;
          }
        }
        return true;
      }

    private:
      typedef std::vector<double> freq_list_type;
      freq_list_type frequencies_;
      freq_list_type::iterator iter_;
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
// TODO:
//   could be interesting to try waiting until the sine wave is near its lowest
//   frequency before terminating the buffer.  That way we wouldn't get the pop on
//   the sound card.
class note_sequence {
  public:
    note_sequence(settings &set) {
      if (set.note_mode() == settings::note_mode_list) {
        impl_.reset(new detail::listed_sqeuence(
              set.concert_pitch(),
              set.note_list().begin(), set.note_list().end(),
              set.note_list().size()));
      }
      else {
        assert(set.note_mode() == settings::note_mode_start);
        trc("using a start note and distance");
        int start_offset = parse_note(set.start_note().c_str());

        int stop_offset;
        if (! set.end_note().empty()) {
          stop_offset = parse_note(set.end_note().c_str());
        }
        else if (set.num_notes() >= 0) {
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
