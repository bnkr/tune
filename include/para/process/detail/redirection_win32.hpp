// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

#ifndef PARA_PROCESS_DETAIL_REDIRECTION_HPP_8w6dyoqu
#define PARA_PROCESS_DETAIL_REDIRECTION_HPP_8w6dyoqu

#include <windows.h>

namespace para {
  namespace process {
    namespace detail {
      //! Using this as a template removes the necesity for virtual functions and for
      //! copying the handles into this class somewhere.
      struct stdin_redirect_win32 {
        //! Used to say whether this is a noop.  Obviously everything here is
        //! not a noop.
        typedef enable_redirects_tag redirects_tag;

        stdin_redirect_win32(ipc_pipe &new_stdin) : in_pipe_(new_stdin) {
          // Turn off inheritance of (what will be) stdin's write end.
          //
          // TODO:
          //   is this OK?  What if I create a process that has this fd as its
          //   stdout end?
          in_pipe_.disable_inherit(in_pipe_.write_end());
        }

        //! Get rid of the end that was given to the child proces.  Note: this
        //! definitely needs to be called -- nobody can use this end of the pipe
        //! any more; the process will deadlock otherwise also.
        void close_child_ends() { CloseHandle(in_pipe_.read_end()); }
        HANDLE stdin_handle() { return in_pipe_.read_end(); }
        HANDLE stdout_handle() { return ::GetStdHandle(STD_OUTPUT_HANDLE); }

        private:
          ipc_pipe &in_pipe_;
      };

      //! See stdin_redirect_win32
      struct stdout_redirect_win32 {
        typedef enable_redirects_tag redirects_tag;

        stdout_redirect_win32(ipc_pipe &new_stdout) : out_pipe_(new_stdout) {
          out_pipe_.disable_inherit(out_pipe_.read_end());
        }

        void close_child_ends() { out_pipe_.close_write(); }
        HANDLE stdin_handle() { return ::GetStdHandle(STD_INPUT_HANDLE); }
        HANDLE stdout_handle() { return out_pipe_.write_end(); }

        private:
          ipc_pipe &out_pipe_;
      };

      //! See stdin_redirect_win32
      struct both_redirect_win32 {
        typedef enable_redirects_tag redirects_tag;

        both_redirect_win32(ipc_pipe &new_stdin, ipc_pipe &new_stdout) : in_(new_stdin), out_(new_stdout) {}

        void close_child_ends() {
          out_.close_write();
          in_.close_read();
          CloseHandle(out_.write_end());
          CloseHandle(in_.read_end());
        }

        HANDLE stdout_handle() { return out_.write_end(); }
        HANDLE stdin_handle() { return in_.read_end(); }

        private:
          ipc_pipe &in_;
          ipc_pipe &out_;
      };


    }
  }
}

#endif
