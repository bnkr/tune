// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

#ifndef PARA_PROCESS_DETAIL_IPC_PIPE_WIN32_HPP_3mckwchm
#define PARA_PROCESS_DETAIL_IPC_PIPE_WIN32_HPP_3mckwchm

#include <windows.h>

namespace para {
  namespace process {
    namespace detail {
      typedef HANDLE handle_type;

      //! Windows pipe implementation.
      //
      // Note: I'm aware of the crappy non-portable public interface.  See todo in the
      // windows design file.
      class ipc_pipe_win32 {
        public:
          ipc_pipe_win32() {
            SECURITY_ATTRIBUTES sec_attr;

            sec_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
            // cause pipe handles to be inherited in child processes
            sec_attr.bInheritHandle = TRUE;
            sec_attr.lpSecurityDescriptor = NULL;

            const DWORD buffer_size_hint = 0;
            if (! CreatePipe(&read_, &write_, &sec_attr, buffer_size_hint)) {
              throw_system_error<pipe_error>(::GetLastError(), "couldn't create pipe");
            }
          }

          ~ipc_pipe_win32() {
            close_read(std::nothrow);
            close_write(std::nothrow);
          }

          handle_type read_end() { return read_; }
          handle_type write_end() { return write_; }

          void close_read() {
            if (! close_read(std::nothrow)) {
              throw_system_error<pipe_error>(::GetLastError(), "couldn't close pipe read end");
            }
          }

          void close_write() {
            if (! close_write(std::nothrow)) {
              throw_system_error<pipe_error>(::GetLastError(), "couldn't close pipe read end");
            }
          }

          bool close_read(std::nothrow_t) { return ::CloseHandle(read_end()); }
          bool close_write(std::nothrow_t) { return ::CloseHandle(write_end()); }

          // deprecated -- usage is prolly broken anyway.
          void disable_inherit(HANDLE h) {
            if (! SetHandleInformation(h, HANDLE_FLAG_INHERIT, 0)) {
              throw_system_error<pipe_error>(::GetLastError(), "couldn't disable inheritance on pipe");
            }
          }

        private:
          HANDLE read_;
          HANDLE write_;
      };


    }
  }
}

#endif
