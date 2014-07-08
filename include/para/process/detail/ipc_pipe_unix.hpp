// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief IPC pipe interfaces.
 */

#ifndef PARA_PROCESS_DETAIL_IPC_PIPE_UNIX_HPP_7itc8sy4
#define PARA_PROCESS_DETAIL_IPC_PIPE_UNIX_HPP_7itc8sy4

#include <unistd.h>
#include <cstdio>

namespace para {
  namespace process {
    namespace detail {

      typedef int handle_type;

//! \ingroup grp_proc
//! Interface to an inter-process pipe.  Note that when redirecting, you must
//! close the unused end in each process.
class ipc_pipe_unix {
  private:
    enum pipe_ends {pipe_read_end = 0, pipe_write_end = 1};

  public:
    //! Create a pipe.
    ipc_pipe_unix() {
      int ret = ::pipe(fds_);
      if (ret == -1) {
        throw_system_error<pipe_error>(errno, "couldn't create pipe");
      }
    }

    //! Close the pipe.
    ~ipc_pipe_unix() {
      close_read(std::nothrow);
      close_write(std::nothrow);
    }

    //! Close the write end of the pipe.
    bool close_write(std::nothrow_t) { return ::close(write_end()) != -1; }

    void close_write() {
      if (! close_write(std::nothrow)) {
        throw_system_error<pipe_error>(errno, "couldn't close read end");
      }
    }

    //! Close the read end of the pipe.
    bool close_read(std::nothrow_t) { return ::close(read_end()) != -1; }

    void close_read() {
      if (! close_read(std::nothrow)) {
        throw_system_error<pipe_error>(errno, "couldn't close read end");
      }
    }

    //! Return the read file handle.
    handle_type read_end() { return fds_[pipe_read_end]; }
    //! Return the write file handle.
    handle_type write_end() { return fds_[pipe_write_end]; }

// Deprecated:

    //! Make the read end of the pipe become this process's stdin.
    void redirect_stdin() { do_redirect(read_end(), fileno(stdin)); }
    //! Make the write end of the pipe become the current process's stdout.
    void redirect_stdout() { do_redirect(write_end(), fileno(stdout)); }

    void do_redirect(int fd, int replace_fd) {
      int ret = dup2(fd, replace_fd);
      if (ret == -1) {
        throw_system_error<pipe_error>(errno, "couldn't replace stream with pipe");
      }
    }
// end deprecated

  private:
    int fds_[2];
};

    } // detail
  } // process
} // ns para

#endif

