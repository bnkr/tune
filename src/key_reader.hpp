class key_reader_interface {
  virtual bool pressed() = 0;
};

#if WIN32
#  include "key_reader_win32.hpp"
#  define KEY_READER_TYPE key_reader_win32
#else
#  include "key_reader_posix.hpp"
#  define KEY_READER_TYPE key_reader_posix
#endif

typedef KEY_READER_TYPE key_reader;
