/*!
\file
\brief Portable interface to keypress detection.
*/
#ifndef KEY_READER_HPP_s9dbdcti
#define KEY_READER_HPP_s9dbdcti

// allow including the nonportable headers now
#define KEY_READER_HEADER

namespace detail {
  //! \brief For compiler help.
  class key_reader_interface {
    virtual bool pressed() = 0;
  };
}

#if WIN32
#  include "key_reader_win32.hpp"
#  define KEY_READER_TYPE key_reader_win32
#else
#  include "key_reader_posix.hpp"
#  define KEY_READER_TYPE key_reader_posix
#endif

typedef KEY_READER_TYPE key_reader;

#endif
