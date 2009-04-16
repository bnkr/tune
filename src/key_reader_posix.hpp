/*!
\file
\brief Non-Portable POSIX non-blocking keypress detector.
*/
#ifndef KEY_READER_HEADER
#  error Do not include this file directly.  Use key_reader.hpp instead.
#endif

#include <fcntl.h>
#include <cerrno>

//! \brief Non-blocking reader of stdin.
class key_reader_posix : private detail::key_reader_interface {
  public:
    key_reader_posix() {
      int flags = fcntl(fileno(stdin), F_GETFL, 0);
      flags |= O_NONBLOCK;
      fcntl(fileno(stdin), F_SETFL, flags);
    }

    ~key_reader_posix() {
      int flags = fcntl(fileno(stdin), F_GETFL, 0);
      flags &= ~O_NONBLOCK;
      fcntl(fileno(stdin), F_SETFL, flags);
    }

    bool pressed() {
      const std::size_t buf_sz = 1024;
      char buf[buf_sz];
      ssize_t v = read(fileno(stdin), buf, 1);
      if (v == -1) {
        if (errno == EAGAIN) {
          // trc("key.pressed(): op would block.");
          return false;
        }
        else {
          trc("key.pressed(): another kind of error.")
          // arses.
          return false;
        }
      }
      else {
        // purge the stream until wouldblock or it's empty
        do {
          v = read(fileno(stdin), buf, buf_sz);
        }
        while (v > 0);
        trc("key.pressed(): there was stuff in the buffer - a key was pressed.")
        return true;
      }
    }
};

