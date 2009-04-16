/*!
\file
\brief Non-Portable win32 non-blocking keypress detector.
*/

#ifndef KEY_READER_HEADER
#  error Don't include this file directly.  Use key_reader.hpp instead.
#endif

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <iostream>

//! \brief Non-blocking reader of stdin.
class key_reader_win32 : private detail::key_reader_interface {
  public:
    key_reader_win32()
    : th_(boost::bind(&key_reader_win32::reader, this)), pressed_(false) {
    }
    ~key_reader_win32() {
      // can't join, it's a blocking read.
      // th_.join();
    }

    // TODO: bleh.  Doesn't work.
    bool pressed() {
      bool b = pressed_;
      // TODO: use atomics of course.
      {
        boost::mutex::socoped_lock lk(mut_);
        pressed_ = false;
      }
      return b;
    }

    void reader() {
      char c;
      std::cin >> c;
      {
        boost::mutex::socoped_lock lk(mut_);
        pressed_ = true;
      }
    }

    boost::mutex mut_;
    bool pressed_;
    boost::thread th_;
};
