// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Exceptions used by the multi-processing library.
 */

#ifndef PARA_PROCESS_EXCEPTIONS_HPP_244gji1b
#define PARA_PROCESS_EXCEPTIONS_HPP_244gji1b

#include <exception>
#include <stdexcept>
#include <boost/system/error_code.hpp>

namespace para {
  namespace process {

    //! \ingroup grp_proc
    //! Base class for errors using boost system for the message.
    struct process_error : public std::runtime_error {
      process_error(const char *message, const boost::system::error_code &ec)
      : runtime_error(message), ec_(ec) {}

      //! Accessor to the system error code.  Of note, value() and message().
      const boost::system::error_code &error_code() const { return ec_; }

      //! Returns a constructed on first use message from boost system and the
      //! message supplied in the constructor, which looks like:
      //!
      //!   reason: error message
      //!
      //! For instance:
      //!
      //!   coudln't launch process: no such file or directory.
      const char *what() const throw() {
        try {
          if (message_.empty()) {
            message_ = this->runtime_error::what();
            if (! message_.empty()) {
              message_ += ": ";
            }

            const std::size_t pos = message_.length();
            message_ += error_code().message();
            // lowercase it :)
            if (message_[pos] >= 'A' && message_[pos] <= 'Z') {
              message_[pos] = message_[pos] + 32;
            }
          }

          return message_.c_str();
        }
        catch(...) {
          return this->std::runtime_error::what();
        }
      }

      ~process_error() throw() {}

      private:
        mutable std::string message_;
        boost::system::error_code ec_;
    };

    //! \ingroup grp_proc
    //! Thrown when a process can't be created.
    struct create_error : public process_error {
      create_error(const char *m, const boost::system::error_code &ec) : process_error(m, ec) {}
      ~create_error() throw() {}
    };

    //! \ingroup grp_proc
    //! Thrown on errors creating or redirecting pipes.
    struct pipe_error : public process_error {
      pipe_error(const char *m, const boost::system::error_code &ec) : process_error(m, ec) {}
      ~pipe_error() throw() {}
    };

    //! \ingroup grp_proc
    //! Missing program to exec.
    struct program_error : public process_error {
      program_error(const char *m, const boost::system::error_code &ec) : process_error(m, ec) {}
      ~program_error() throw() {}
    };

    //! \ingroup grp_proc
    //! Couldn't wait on a process.
    struct wait_error : public process_error {
      wait_error(const char *m, const boost::system::error_code &ec) : process_error(m, ec) {}
      ~wait_error() throw() {}
    };

    //! \ingroup grp_proc
    //! Couldn't kill/terminate/signal a process.
    struct signal_error : public process_error {
      signal_error(const char *m, const boost::system::error_code &ec) : process_error(m, ec) {}
      ~signal_error() throw() {}
    };

    //! \ingroup grp_proc
    //! Couldn't open/find the pid.
    struct pid_error : public process_error {
      pid_error(const char *m, const boost::system::error_code &ec) : process_error(m, ec) {}
      ~pid_error() throw() {}
    };

    namespace detail {
      //! Throws a portable error based on a non-portable system error code.
      template<class ParaError>
      void throw_system_error(int error_val, const char *reason) {
        namespace sys = boost::system;
        throw ParaError(reason, sys::error_code(error_val, sys::get_system_category()));
      }
    }
  }
}

#endif
