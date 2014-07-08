// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

#ifndef PARA_PROCESS_DETAIL_PROCESS_WIN32_go36wbfb
#define PARA_PROCESS_DETAIL_PROCESS_WIN32_go36wbfb

#include "status_base.hpp"

// noncopyable
#include <boost/utility.hpp>
#include <boost/system/system_error.hpp>
#include <windows.h>

#include <cassert>

namespace para {
  namespace process {
    namespace detail {
      typedef DWORD pid_type;

      //! The status of a running/stopped process.
      //!
      //! Note: this needs to be public-inherited to avoid an inaccessable base
      //! error on mingw (which doesn't make sense to me because the unix one
      //! works fine)
      class status_win32 : public status_base<status_win32> {
        protected:
          typedef status_base<status_win32> base;

        public:
          explicit status_win32(DWORD exit_code, bool exited)
          : status_(exit_code), exited_(exited) { }

          bool exited() const { return exited_; }
          int exit_status() const { return status_; }

          using base::success;
          using base::signaled;
          using base::signal;

        private:
          // We don't use a handle to it because this gets returned by value and
          // if the process goes out of scope the handle gets closed.
          DWORD status_;
          // This one is necessary because win32 uses the status to show that
          // it's still running and the code it uses is also a valid code under
          // exit.
          bool exited_;
      };

      inline pid_type current_pid_win32() { return GetCurrentProcessId(); }

      class process_win32 {
        public:
          typedef status_win32 status_type;

          process_win32() {
            pid_ = current_pid_win32();
            process_ = GetCurrentProcess();
          }

          process_win32(pid_type p) : pid_(p) { open_process(p); }

          ~process_win32() {
            // Aparently closing a pseudo-handle generates an exception when
            // debugging.
            if (process_ != GetCurrentProcess()) {
              CloseHandle(process_);
            }
          }

          status_win32 wait() {
            DWORD ret = WaitForSingleObject(process_, INFINITE);
            if (ret == WAIT_FAILED) {
              throw_system_error<wait_error>(::GetLastError(), "could not blocking wait");
            }
            assert(ret == WAIT_OBJECT_0);

            DWORD exit_code;
            GetExitCodeProcess(process_, &exit_code);
            const bool exited = true;
            return status_win32(exit_code, exited);
          }

          status_win32 wait_nb();

          //! Note that it will exit with status 1.
          bool kill(std::nothrow_t) {
            return (TerminateProcess(process_, 1) == 0);
          }

          //! Throws errors if the process doesn't exist of is a bad handle.
          void kill() {
            if (! kill(std::nothrow)) {
              throw_system_error<signal_error>(::GetLastError(), "could not kill process");
            }
          }

          //! Throws
          void terminate() { kill(); }
          bool terminate(std::nothrow_t v) { return kill(v); }

          pid_type pid() const { return pid_; }

        protected:
          struct no_init {};

          //! So subclasses can set their own pids.
          process_win32(no_init) {}

          void pid(pid_type p) { pid_ = p; }

          //! Again, because sub-processes need to set this.
          HANDLE &process_handle() { return process_; }

          void open_process(pid_type p) {
            const BOOL children_inherit = FALSE;
            // note: this means we aren't allowed to self-wait.
            const DWORD access_rights = PROCESS_ALL_ACCESS;
            process_ = OpenProcess(access_rights, children_inherit, p);

            if (process_ == NULL) {
              throw_system_error<pid_error>(::GetLastError(), "could not open process");
            }
          }

        private:
          pid_type pid_;
          HANDLE process_;
      };

    } // ns
  }
}

#endif
