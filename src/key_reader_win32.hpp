//   Info on winders.
//   http://www.gamedev.net/community/forums/topic.asp?topic_id=285863

#include <conio.h>

//! \brief Non-blocking reader of stdin.
class key_reader_win32 : private key_reader_interface {
  public:
    bool pressed() { return _kbhit(); }
};
