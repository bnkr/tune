// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

#ifndef PARA_PROCESS_DETAIL_NAMED_PROCESS_WIN32_9te6oec5
#define PARA_PROCESS_DETAIL_NAMED_PROCESS_WIN32_9te6oec5

#include <windows.h>
#include <cassert>

namespace para {
  namespace process {
    namespace detail {
      //! Used also by the redirectors.
      typedef struct {} disable_redirects_tag;
      typedef struct {} enable_redirects_tag;

      //! Noop redirector; for the constructor with no redirection.
      struct null_redirector {
        typedef disable_redirects_tag redirects_tag;

        void close_child_ends() {}
      };

      //! Wrapper that deals with redirection and the STARTUPINFO structure.
      template<class Redirector>
      struct startupinfo_wrapper {
        STARTUPINFO si_;

        startupinfo_wrapper(Redirector &rd) {
          ZeroMemory(&si_, sizeof(STARTUPINFO));
          si_.cb = sizeof(STARTUPINFO);

          typedef typename Redirector::redirects_tag if_enabled_tag;

          assign_flags(if_enabled_tag());
          assign_handles(rd, if_enabled_tag());
        }

        //! Fully initialised value.
        STARTUPINFO *startupinfo() { return &si_; }

        protected:
          // Noops when tag is false.
          void assign_flags(disable_redirects_tag) { }
          void assign_handles(Redirector &, disable_redirects_tag) { }

          void assign_flags(enable_redirects_tag) {
            si_.dwFlags |= STARTF_USESTDHANDLES;
            // So far I didn't implement stderr redirection; change is obvious.
            si_.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
          }

          void assign_handles(Redirector &rd, enable_redirects_tag) {
            assert(rd.stdin_handle() != INVALID_HANDLE_VALUE);
            si_.hStdInput = rd.stdin_handle();
            si_.hStdOutput = rd.stdout_handle();
          }
      };




//! Windows implementation of a named process.
class named_process_win32 : public ::para::process::process, boost::noncopyable {
  public:
    using process::status_type;

    template<class BinaryDetails>
    named_process_win32(const BinaryDetails &prog) : process(process::no_init()) {
      null_redirector red;
      launch(prog, red);
    }

    template<class BinaryDetails, class Redirector>
    named_process_win32(const BinaryDetails &prog, Redirector &red) : process(process::no_init()) {
      launch(prog, red);
    }

  protected:
    template<class BinaryDetails, class Redirector>
    void launch(const BinaryDetails &prog, Redirector &red) {
      // TODO:
      //   Issues with environemnt.  I would have to copuy the current
      //   environemtn and create a new one with the additional values.

      startupinfo_wrapper<Redirector> handle_settings(red);

      PROCESS_INFORMATION returned_handles;
      ZeroMemory(&returned_handles, sizeof(PROCESS_INFORMATION));

      // Any args with spaces must be double-quoted.
      // TODO:
      //   I can calculate the length in advance here -- would that be faster?
      std::string command_line;
      {
        char **i = prog.args();
        if (i) {
          while (*i) {
            command_line += '"';
            command_line += *i;
            command_line += '"';
            command_line += ' ';
            ++i;
          }
        }
        assert(command_line.length() < 32768);
      }

      // Just this to turn of inheritance of the thread/process handles which
      // will be returned.  We don't care about the descriptor stuff so leave it
      // null.
      SECURITY_ATTRIBUTES sa_no_inherit;
      ZeroMemory(&sa_no_inherit, sizeof(sa_no_inherit));
      sa_no_inherit.nLength = sizeof(sa_no_inherit);
      sa_no_inherit.bInheritHandle = FALSE;

      // Only applies to handles that are allowed to be inherited.
      const BOOL inherit_handles = TRUE;
      const DWORD create_suspended_flag = CREATE_SUSPENDED;
      const LPVOID use_parents_environment = NULL;
      const LPTSTR use_parents_pwd = NULL;

      const char *exact_image_name = prog.path();
      char *const complete_command_line = &command_line[0];

      // Create the child process.  Note that apparently using the first argument
      // means that you can't run a 16bit process since that has to be specified
      // with the command_line argument.
      BOOL ret = CreateProcess(
          exact_image_name,
          complete_command_line,
          &sa_no_inherit,   // for the process
          &sa_no_inherit,   // for the thread
          inherit_handles,
          create_suspended_flag,
          use_parents_environment,
          use_parents_pwd,
          handle_settings.startupinfo(),
          &returned_handles
      );

      if (! ret) {
        DWORD last_error = GetLastError();

        // TODO:
        //   There are more codes, too, for example permissions, not an exe,
        //   etc.  It's tricky to be very consistent with exec, but for major
        //   stuff like that it would be useful.
        if (last_error == ERROR_FILE_NOT_FOUND) {
          throw_system_error<program_error>(last_error, "named binary could not be executed");
        }
        else {
          throw_system_error<create_error>(last_error, "could not create process");
        }
      }

      red.close_child_ends();

      this->pid(returned_handles.dwProcessId);
      // Re-open the returned handles with the extra permission SYNCHRONIZE
      //
      // TODO:
      //   The docs say the current process handle gets duplicated -- does that
      //   mean that this leaks?
      ret = DuplicateHandle(
          // Process who has the old handle (ie, this process).  This is a
          // pseudo-handle, but that's ok.
          GetCurrentProcess(),
          // What we want to duplicate.  Note that handles created with
          // CreateProcess default to PROCESS_ALL_ACCESS permissions, which includes
          // PROCESS_DUP_HANDLE.
          returned_handles.hProcess,
          // Process who wants the new handle (us again).
          GetCurrentProcess(),
          // Where we will store it..
          &process_handle(),
          // Access rights -- this is the important bit that lets us wait on the
          // handle.
          PROCESS_ALL_ACCESS | SYNCHRONIZE,
          // Child processes can't inherit it.
          FALSE,
          // No special flags: this function should not close the source handle
          // because we need it for error checking
          0
      );

      // Tricky clean up stuff.
      DWORD last_error = GetLastError();
      if (! ret) {
        TerminateProcess(returned_handles.hProcess, 1);
        CloseHandle(returned_handles.hProcess);
        CloseHandle(returned_handles.hThread);
        throw_system_error<create_error>(last_error, "could not open the process");
      }
      else {
        CloseHandle(returned_handles.hProcess);
      }

      if (! ResumeThread(returned_handles.hThread)) {
        last_error = ::GetLastError();
        TerminateProcess(process_handle(), 1);
        CloseHandle(returned_handles.hThread);
        CloseHandle(process_handle());
        throw_system_error<create_error>(last_error, "could not resume the suspended process");
      }
      else {
        CloseHandle(returned_handles.hThread);
      }

    }

};


    }
  }
}

#endif
