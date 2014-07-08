// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Unix implementation of the named_process class.
 */

#ifndef PARA_PROCESS_DETAIL_NAMED_PROCESS_UNIX_HPP_e42w96wo
#define PARA_PROCESS_DETAIL_NAMED_PROCESS_UNIX_HPP_e42w96wo

#include <unistd.h>
#include <fcntl.h>

namespace para {
  namespace process {
    namespace detail {
      //! Error data in a pipe.  Used to report an error with i/o redirection
      //! or executing a process on unix.
      class failure_pipe : boost::noncopyable {
        public:
          enum errors { err_exec, err_redirect };

          failure_pipe() {
            ::fcntl(p_.read_end(), F_SETFD, FD_CLOEXEC);

            // Only this is critical if this doesn't work because the parent might
            // block forever.
            if (::fcntl(p_.write_end(), F_SETFD, FD_CLOEXEC) == -1) {
              throw_system_error<create_error>(errno, "could not create pipe for error data ipc");
            }
          }

          //! Called by the child.
          void report_error(enum errors error, int error_number) {
            error_data_[0] = (int) error;
            error_data_[1] = error_number;
            ::write(p_.write_end(), &error_data_[0], sizeof(int) * 2);
            ::close(p_.write_end());
          }

          //! Called by the parent (deals with closing etc).  This will block until
          //! exec is called or until
          void read_error_data() {
            ::close(p_.write_end());
            error_data_[1] = 0;
            ssize_t bytes = ::read(p_.read_end(), &error_data_[0], sizeof(int) * 2);
            if (bytes == -1) {
              throw_system_error<pipe_error>(errno, "could not read error data from child");
            }
          }

          //! Return place identifier.
          enum errors error() const { return (enum errors) error_data_[0]; }
          //! Return errno value.  Zero if no error.
          int error_number() const { return error_data_[1];  }

        private:
          ::para::process::ipc_pipe p_;
          int error_data_[2];
      };

      //! Write error data to the pipe.
      struct piped_error_onfail {
        piped_error_onfail(failure_pipe &p) : p_(p) {}

        int operator()(int num, const char *, char *const[], char *const[]) {
          p_.report_error(failure_pipe::err_exec, num);
          return num;
        }

        private:
          failure_pipe &p_;
      };

      //! Hooks for writing redirection failure data to a pipe.
      template<class Redirector>
      struct redirecting_hooks : public null_hooks {
        redirecting_hooks(failure_pipe &fp, Redirector &rd)
        : fp_(fp), rd_(rd) {}

        //! Won't throw.
        void fork_start() {
          if (! rd_.redirect()) {
            fp_.report_error(failure_pipe::err_redirect, errno);
            _exit(1);
          }
        }

        //! might throw
        void post_fork() { rd_.close_child_ends(); }

        private:
          failure_pipe &fp_;
          Redirector rd_;
      };

      //! Unix implementation of the named process exec.
      class named_process_unix : public ::para::process::unix_named_process {
        public:
          typedef ::para::process::unix_named_process::status_type status_type;

          //! Simple constructor specifying the binary to run.
          template<class BinaryDetails>
          named_process_unix(const BinaryDetails &prog) {
            ::para::process::null_hooks hooks;
            failure_pipe p;
            select_exec_variant(prog, hooks, piped_error_onfail(p), typename BinaryDetails::details_tag());
            p.read_error_data();
            if (p.error_number() != 0) {
              throw_system_error<program_error>(p.error_number(), "could not exec program");
            }
          }

          //! As above, but with a function int (*f)(int) which decides what to do
          //! when exec fails (see process::fork for more)
          template<class BinaryDetails, class Redirector>
          named_process_unix(const BinaryDetails &prog, Redirector redirector) {
            failure_pipe p;
            redirecting_hooks<Redirector> hooks(p, redirector);
            select_exec_variant(prog, hooks, piped_error_onfail(p), typename BinaryDetails::details_tag());
            p.read_error_data();
            if (p.error_number() != 0) {
              switch (p.error()) {
                case failure_pipe::err_exec:
                  throw_system_error<program_error>(p.error_number(), "could not exec program");
                  break; // not reached
                default:
                  assert(p.error() == failure_pipe::err_redirect);
                  throw_system_error<pipe_error>(p.error_number(), "could not redirect handles");
                  break; // not reached
              }
            }
          }
      };
    }
  }
}

#endif
