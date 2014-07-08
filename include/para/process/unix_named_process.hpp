// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Home of class unix_named_process.
 */

#ifndef PARA_PROCESS_NAMED_PROCESS_HPP_doau4lg6
#define PARA_PROCESS_NAMED_PROCESS_HPP_doau4lg6

#include "unix_fork.hpp"

#include <boost/bind.hpp>

#include <unistd.h>

#include <vector>
#include <cassert>
#include <cerrno>

namespace para {
  namespace process {

    namespace detail {
      //! Used as an on-fail function.
      struct return_errno {
        int operator()(int num, const char *, char *const[], char *const[]) {
          return num;
        }
      };

      //! call execvp || on_fail
      struct searching_exec {
        template<class Function>
        int operator()(const char *name, char * const argv[], Function on_fail) {
          execvp(name, argv);
          return on_fail(errno, name, argv, NULL);
        }
      };

      //! call execvp or execve || on_fail
      struct non_searching_exec {
        template<class Function>
        int operator()(const char *name, char * const argv[], Function on_fail) {
          execv(name, argv);
          return on_fail(errno, name, argv, NULL);
        }

        template<class Function>
        int operator()(const char *name, char * const argv[], char *const env[], Function on_fail) {
          execve(name, argv, env);
          return on_fail(errno, name, argv, env);
        }
      };

    }


/*!
 * \ingroup grp_proc
 *
 * Unix-only process executor which works on named binaries.  It gives a general
 * hooks class and an optional on-failure functor.
 *
 * See the tutorial pages for information on the concept requirements,
 */
class unix_named_process : public unix_fork {
  public:
    //! For a little more simplicity.
    typedef struct {} default_onfail_type;

    //! \name Constructors/Destructors
    //@{

    //! Pass hooks to process::fork, and use prog for details of the program.
    template<class BinaryDetails, class Function>
    unix_named_process(const BinaryDetails &prog, Function on_fail) {
      null_hooks hooks;
      select_exec_variant(prog, hooks, on_fail, typename BinaryDetails::details_tag());
    }

    //! As above but adds the function to be called on exec fail.
    template<class BinaryDetails, class Hooks, class Function>
    unix_named_process(const BinaryDetails &prog, Hooks &hooks, Function on_fail) {
      // select_onfail(prog, hooks, on_fail);
      select_exec_variant(prog, hooks, on_fail, typename BinaryDetails::details_tag());
    }

    //! In order that the the constructors don't conflict.
    template<class BinaryDetails, class Hooks>
    unix_named_process(const BinaryDetails &prog, Hooks &hooks, default_onfail_type) {
      select_exec_variant(prog, hooks, detail::return_errno(), typename BinaryDetails::details_tag());
    }
    //@}

  protected:
    //! Lets us use select_exec_variant directly from a subclass.  Nb: unix_fork
    //! calls the no init constructor of process.
    unix_named_process() {}

    template<class BinaryDetails, class Hooks, class Function>
    void select_onfail(const BinaryDetails &prog, Hooks hooks, Function on_fail) {
      select_exec_variant(prog, hooks, on_fail, typename BinaryDetails::details_tag());
    }

    template<class BinaryDetails, class Hooks>
    void select_onfail(const BinaryDetails &prog, Hooks hooks, default_onfail_type) {
      select_exec_variant(prog, hooks, detail::return_errno(), typename BinaryDetails::details_tag());
    }

    //! \name Based on the tag class, decide which exec functor to use.
    //@{

    template<class BinaryDetails, class Hooks, class Function>
    void select_exec_variant(const BinaryDetails &prog, Hooks &hooks, Function on_fail, search_no_env_tag) {
      launch(boost::bind<int>(detail::searching_exec(), prog.path(), prog.args(), on_fail), hooks);
    }

    template<class BinaryDetails, class Hooks, class Function>
    void select_exec_variant(const BinaryDetails &prog, Hooks &hooks, Function on_fail, no_search_no_env_tag) {
      launch(boost::bind<int>(detail::non_searching_exec(), prog.path(), prog.args(), on_fail), hooks);
    }

    template<class BinaryDetails, class Hooks, class Function>
    void select_exec_variant(const BinaryDetails &prog, Hooks &hooks, Function on_fail, no_search_env_tag) {
      launch(boost::bind<int>(detail::non_searching_exec(), prog.path(), prog.args(), prog.env(), on_fail), hooks);
    }

    //@}
};

  }
} // ns para::process

#endif

