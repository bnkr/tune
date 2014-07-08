// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Redirecting std* using pipes.  This must only be included on unix.
 */

#ifndef PARA_PROCESS_DETAIL_REDIRECTION_HPP_kw1o6kxx
#define PARA_PROCESS_DETAIL_REDIRECTION_HPP_kw1o6kxx

#include <unistd.h>

namespace para {
  namespace process {
    namespace detail {
      //! Replace replace_fd with new_fd.  Return -1 on error.
      inline int redirect(int new_fd, int replace_fd) {
        return ::dup2(new_fd, replace_fd);
      }

      //! Make the read end of the pipe become this process's stdin.
      inline int redirect_stdin(ipc_pipe &p) {
        ::close(p.write_end());
        return redirect(p.read_end(), fileno(stdin));
      }

      //! Make the write end of the pipe become the current process's stdout.
      inline int redirect_stdout(ipc_pipe &p) {
        ::close(p.read_end());
        return redirect(p.write_end(), fileno(stdout));
      }

      //! Fork hooks for redirecting stdin to be a pipe and closing the end
      //! given to the child process.
      struct stdin_redirect_unix : public null_hooks {
        stdin_redirect_unix(ipc_pipe &new_stdin) : pipe_(new_stdin) { }

        //! Called in the child process.  Returns -1 on error, and errno is set.
        //! This doesn't throw because the caller needs to work out what exception
        //! to throw.
        bool redirect() { return redirect_stdin(pipe_) != -1; }

        //! Called in the parent.  Closes the end which the parent will use.  Deadlock
        //! ensues if this isn't called.  As such, this throws.
        void close_child_ends() { pipe_.close_read(); }

        void fork_start() {
          pipe_.redirect_stdin();
          pipe_.close_write();
        }

        void post_fork() {
          pipe_.close_read();
        }

        private:
          ipc_pipe &pipe_;
      };

      //! Fork hooks for redirecting stdout to be a pipe.
      struct stdout_redirect_unix : public null_hooks {
        stdout_redirect_unix(ipc_pipe &new_stdin) : pipe_(new_stdin) { }

        bool redirect() { return redirect_stdout(pipe_) != -1; }
        void close_child_ends() { pipe_.close_write(); }

        void fork_start() {
          pipe_.redirect_stdout();
          pipe_.close_read();
        }

        void post_fork() {
          pipe_.close_write();
        }

        private:
          ipc_pipe &pipe_;
      };

      //! Fork hooks for redirecting stdout and stdin to be a pipe.
      struct both_redirect_unix : public null_hooks {
        both_redirect_unix(ipc_pipe &new_stdin, ipc_pipe &new_stdout)
        : in_pipe_(new_stdin), out_pipe_(new_stdout) { }

        bool redirect() {
          return redirect_stdout(out_pipe_) && redirect_stdin(in_pipe_);
        }

        void close_child_ends() {
          out_pipe_.close_write();
          in_pipe_.close_read();
        }

        void fork_start() {
          in_pipe_.redirect_stdin();
          in_pipe_.close_write();
          out_pipe_.redirect_stdout();
          out_pipe_.close_read();
        }

        void post_fork() {
          in_pipe_.close_read();
          out_pipe_.close_write();
        }

        private:
          ipc_pipe &in_pipe_;
          ipc_pipe &out_pipe_;

      };

    // TODO:
    //   We also want where you don't have to declare the pipe in advance
    //
    //   I could just have a hooks which has it by value.  Then the next
    //   hook takes it as a constructor paramter.
    //
    //   Example:
    //
    //     auto_redirect_stdout pipe_hook;
    //     fork p1(func, pipe_hook);
    //
    //     fork p2(func2, redirect_stdin(pipe_hook.pipe()));
    //
    //   Meh... not all that nice really.

    }
  }
} // ns

#endif
