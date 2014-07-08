// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Anonymous process forker.
 */

#ifndef PARA_PROCESS_FORK_HPP_9fe1do7q
#define PARA_PROCESS_FORK_HPP_9fe1do7q

#include "process.hpp"
#include "exceptions.hpp"

#include <boost/utility.hpp>

namespace para {
  namespace process {

    //! \ingroup grp_proc
    //! For the unix fork hooks argument.
    struct null_hooks {
      void pre_fork() {}
      void fork_end() {}
      void fork_start() {}
      void post_fork() {}
    };

/*!
 * \ingroup grp_proc
 *
 * Unix-specific process launcher which uses a functor.  The return value of the
 * functor specifies the return of the process.  This class also takes
 * generalised hooks which are called at given positions in the forking
 * process: see \link process::null_hooks \endlink.
 *
 *
 * Note that the redirect_* classes are implemented as hooks on unix, so a fork
 * can take the redirectors.
 *
 * TODO:
 *   In all these places, if we used an rvalue-reference for hooks, it wouldn't
 *   need to be on the stack.  In any case, we should overload for a const &
 *   because that has the same effect.
 */
class unix_fork : public ::para::process::process, boost::noncopyable {
  public:
    //! Intergral type used to identify processes.
    typedef pid_t pid_type;

    //! \name Constructors/Detructors
    //@{

    // TODO:
    //   The nothrow is problematic becuase windows doesn't support it, yet,

    //! Run the function \c func in a sub-process and *don't* throw any
    //! exceptions.  You *must* check this->error().
    template<class Function>
    unix_fork(Function func, std::nothrow_t)
    : process(process::no_init()) {
      launch_no_hooks(func);
    }

    //! Run the fork; throw an exception if there could not be a fork().
    template<class Function>
    unix_fork(Function func)
    : process(process::no_init()) {
      launch_no_hooks(func);
      throw_if_error();
    }

    //! Run the function \c func in a sub-fork and *don't* throw any
    //! exceptions.  You *must* check this->error().
    template<class Function, class Hooks>
    unix_fork(Function func, Hooks &hooks, std::nothrow_t)
    : process(process::no_init()) {
      launch(func, hooks);
    }

    //! Run the process; throw an exception if there could not be a fork().
    template<class Function, class Hooks>
    unix_fork(Function func, Hooks &hooks)
    : process(process::no_init()) {
      launch(func, hooks);
      throw_if_error();
    }

    //@}

    //! \name Accessors
    //@{

    //! Only necessary if you used the nothrow constructor; returns true if the
    //! process wasn't forked.
    bool error() const { return pid() == -1; }

    //@}


  private:
    //! Shortens the constructors a bit.
    void throw_if_error() {
      int errno_saved = errno;
      if (error()) {
        detail::throw_system_error<create_error>(errno_saved, "couldn't fork process");
      }
    }

  protected:
    //! Needed by subclasses which set their own pic.
    unix_fork() : process(process::no_init()) { }

    /*!
     * \name Forking code
     *
     * Note: I would have used vfork() until I read the manpage which says "it is
     * unfortunate this spetre of the past has been resurrected".  As such the fork
     * fork_func paramter for these functions is a bit redundant.  Nevertheless,
     * it might be the case that a subclass will use it at a later time.
     */
    //@{

    // Wrappers for a default fork() -- it seems you need this rather than
    // default arguments or the function is never found.
    template <class Function>
    void launch_no_hooks(Function func) { launch_no_hooks(func, &::fork); }

    template <class Function, class Hooks>
    void launch(Function func, Hooks &hooks) { launch(func, hooks, &::fork); }

    template <class Function, class ForkFunc>
    void launch_no_hooks(Function func, ForkFunc fork_func) {
      null_hooks hk;
      launch(func, hk, fork_func);
    }

    // Nb: don't call functions which set errno after until the error is checked
    // for.
    template <class Function, class Hooks, class ForkFunc>
    void launch(Function func, Hooks &hooks, ForkFunc fork_func) {
      hooks.pre_fork();
      pid(fork_func());
      if (pid() == 0) {
        hooks.fork_start();
        int code = func();
        hooks.fork_end();
        _exit(code);
      }
      else if (pid() > 0) {
        hooks.post_fork();
      }
    }

    //@}
};

  }
} // para::process

#endif

