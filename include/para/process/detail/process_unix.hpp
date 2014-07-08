// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Unix implementation of process and status.
 */

#ifndef PARA_PROCESS_DETAIL_PROCESS_HPP_8lx17b3h
#define PARA_PROCESS_DETAIL_PROCESS_HPP_8lx17b3h

#include "../exceptions.hpp"
#include "status_base.hpp"
#include <boost/system/system_error.hpp>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace para {
  namespace process {
    namespace detail {
      //! Intergral type used to identify processes.
      typedef pid_t pid_type;

      //! Status of a process.  Wraps a bitfield so it's fine to pass by value.
      class status_unix : protected status_base<status_unix> {
        public:
          explicit status_unix(int value) : value_(value) {}

          //! Did the process exit?
          bool exited() const { return WIFEXITED(value_); }
          //! Was the process signaled?
          bool signaled() const { return WIFSIGNALED(value_); }
          //! Which code did the process exit with?
          int exit_status() const { return WEXITSTATUS(value_); }
          //! Which signal did the process get?
          int signal() const { return WTERMSIG(value_); }
          //! Wrapper for exited() && exit_status == 0
          using status_base<status_unix>::success;


        private:
          int value_;
      };

      //! Signal constants for signal().
      enum signals {
        sig_kill = SIGKILL,
        sig_term = SIGTERM,
        sig_int  = SIGINT,
        sig_usr1 = SIGUSR1,
        sig_usr2 = SIGUSR2,
      };

      //! PID integral.  This is equivilent to process_unix's default constructor
      //! but the windows version is not.
      inline pid_type current_pid_unix() { return ::getpid(); }

      //! Passed to wait for any child.
      const pid_type pid_children = -1;
      //! Passed to wait for same gid children.
      const pid_type pid_same_gid = 0;

      //! Flags can be WNOHANG | WUNTRACED.  Nohang is the nonblock variant.
      inline status_unix do_wait_pid(pid_type pid, int flags) {
        int stat;
        int ret = ::waitpid(pid, &stat, flags);

        if (ret == -1) {
          throw_system_error<wait_error>(errno, "could not wait for process");
        }

        return status_unix(stat);
      }

      //! Base class for executing processes, and also an interface to an existing
      //! process.  Note that on Windows some additional work is done to "open"
      //! the process, as well as just use the PID.  Use the \link
      //! current_pid_unix() \endlink function if you just need the pid_type value.
      class process_unix {
        public:
          //! Status code return.
          typedef status_unix status_type;

          //! \name Constructors/destructor
          //@{

          //! Construct a based on an existing process.  Note that this might
          //! possibly given a permission error instead of a resource not found
          //! error because kill(2) is used to check for pid existence.
          process_unix(pid_type p) : pid_(p) {
            // pid = 0 means signal everything in our group, but we want to
            // treat 0 as though it was a real PID, so fake the right errno.
            if (pid_ == 0) {
              throw_system_error<pid_error>(ESRCH, "could not open process");
            }

            // This is really the "official" way to check that a process exists.
            // See kill(2).
            if (::kill(pid_, 0) == -1 && errno == ESRCH) {
              throw_system_error<pid_error>(errno, "could not open process");
            }
          }

          //! Represents the current process.
          process_unix() : pid_(::getpid()) {}
          //@}

          //! \name Accessors
          //@{

          //! Process id of the sub-process.
          pid_type pid() const { return pid_; }

          //@}

          //! \name Waiting
          //!
          //! Note that these are non-const because even a non-blocking wait can
          //! collect the process and remove it from the kernel.
          //@{

          //! Non-blocking wait, returning the status code of the process.  Throws a
          //! process_error on failure.
          status_unix wait() { return do_wait_pid(pid_, 0); }

          //! Blocking wait, returning the status code.  Throws a process_error on
          //! failure.
          status_unix wait_nb() { return do_wait_pid(pid_, WNOHANG); }

          //@}

          //! \name Termination
          //@{

          //! Terminate (which can be ignored).  Actually, that only happens on
          //! unix.
          void terminate() {
            if (! do_signal(sig_term)) {
              throw_system_error<signal_error>(errno, "could not terminate process");
            }
          }

          bool terminate(std::nothrow_t) {
            return do_signal(sig_term);
          }

          //! Definite kill.
          void kill() {
            if (! do_signal(sig_kill)) {
              throw_system_error<signal_error>(errno, "could not kill process");
            }
          }

          bool kill(std::nothrow_t) {
            return do_signal(sig_kill);
          }

          //@}

        protected:
          struct no_init {};

          //! So a subclass can set the pid.
          process_unix(no_init) {}

          //! Used by subclasses that execute processes.
          void pid(pid_type p) { pid_ = p; }

        private:
          //! True on success.
          bool do_signal(enum signals s) {
            return ::kill(pid_, s) != -1;
          }

          pid_type pid_;
      };

    }
  } // ns process
} // ns para

#endif
