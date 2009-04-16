/*!
\file
\brief Non-Portable win32 non-blocking keypress detector.
*/


//   Info on winders.
//   http://www.gamedev.net/community/forums/topic.asp?topic_id=285863
#ifndef KEY_READER_HEADER
#  error Don't include this file directly.  Use key_reader.hpp instead.
#endif

#include <conio.h>

//! \brief Non-blocking reader of stdin.
class key_reader_win32 : private detail::key_reader_interface {
  public:
    // TODO: bleh.  Doesn't work.
    bool pressed() { return _kbhit(); }
};
