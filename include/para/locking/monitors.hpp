// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
\file
\brief Monitor classes.
*/


#ifndef PARA_LOCKIGN_MONIDOTR_CANTTYPE_99jzz0zx
#define PARA_LOCKIGN_MONIDOTR_CANTTYPE_99jzz0zx

#include <para/locking/detail/boost.hpp>

#include <exception>
#include <stdexcept>
#include <string>

namespace para {
  //! \ingroup grp_locking
  //! \brief General failure locking.
  struct lock_fail_error : public std::runtime_error {
    lock_fail_error(const char *e = "") : runtime_error(e) {}
  };

  //! \ingroup grp_locking
  //! \brief General failure waiting.
  struct wait_fail_error : public std::runtime_error {
    wait_fail_error(const char *e = "") : runtime_error(e) {}
  };

  //! \ingroup grp_locking
  //! \brief The lock was already locked (from a try lock)
  struct lock_owned_error : public lock_fail_error {
    lock_owned_error(const char *e = "") : lock_fail_error(e) {}
  };

  //! \ingroup grp_locking
  //! \brief Timeout when getting a lock.
  struct lock_timeout_error : public lock_fail_error {
    lock_timeout_error(const char *e = "") : lock_fail_error(e) {}
  };

  //! \ingroup grp_locking
  //! \brief Wait on some condition timed out.
  struct wait_timeout_error : public wait_fail_error {
    wait_timeout_error(const char *e = "") : wait_fail_error(e) {}
  };

  /*!
  \ingroup grp_monitors
  \brief Most configurable monitor type in which the lock is allowed to fail.

  Note that the timed constructors time the locking only.  The condition is then
  timed separately with the head() function.

  Note that this class is intentionally left very complicated so that other,
  more simple monitors can be implemented in terms of it.  In particular, it
  is strongly suggested that you use the timed monitors (timed_monitors.hpp)
  to organise timed locking/condition waiting because the interface is much
  less confusing.

  Aside: you might notice that the timed functions are equivilent except for the
  selector types.  I decided that the API was simply too confusing unless you
  specify which type of time you are using, so the more verbose yet semantically
  equivilent method is used.  For a more terse yet still decodeable method,
  try the timed monitors instead.
  */
  /*
  TODO:
    Having the data embeded as it is using template parameters for lock and
    condition is good.

    However, the checking strategy is giving us a lot of classes, so why not
    use a functional/interface design?  It doesn't offer anything more than
    the current design except that it does give the ability to easily specify
    *unchecked* monitors, and slightly more general flexibility:

      // checker has check_wait(monitor), check_lock(monitor)
      some_monitor mon(sync, param, predicate, unckecked_checker());
      // now I must check whether it worked

    Hum... this is not mutually exclusive to the current design.  I could simply
    remove the checked heads from deferred monitor, and implement the current
    aliases in terms of that.  It means another constructor parameter to the
    deferred monitor, but it's *supposed* to be complicated so hm.

    The trick is working out how do keep the usage complexity down.  I want
    reasonable defaults for the checking strategies, of course.  If I play it
    right, I might just be able to add it.  Then I could get rid of any redundant
    classes (prolly remove monitor and rename checked_monitor to be 'monitor'
    -- those two don't feel right as it is).  That way I'll be almost completely
    compatible.
  */
  template <class Lock, class Condition, class MonitorTraits = detail::default_monitor_traits>
  class deferred_monitor : boost::noncopyable {
    public:
      //! \name Template parameter type aliases.
      //@{
      typedef Lock lock_type;
      typedef Condition condition_type;
      //@}

      //! \name Monitor traits types abbreviated.
      //@{
      typedef typename MonitorTraits::defer_lock_type    defer_lock_type;
      typedef typename MonitorTraits::try_to_lock_type   try_to_lock_type;
      typedef typename MonitorTraits::adopt_lock_type    adopt_lock_type;
      //@}

      //! \name Time type selectors.
      //@{
      struct deadline_time_selector {};
      struct duration_time_selector {};
      //@}

      //! \name Constructors/Destructors
      //@{
      //! Initialise the lock and the wait condition for a later head() call.
      template <class Sync>
      explicit deferred_monitor(Sync &s) : lock_(s.mutex()), cond_(s.wait_condition()) {}

      //! Initialise the lock and condition, but defer the locking until later.
      template <class Sync>
      deferred_monitor(Sync &s, defer_lock_type v) : lock_(s.mutex(), v), cond_(s.wait_condition()) {}

      //! Initialise the lock and condition and use the try lock constructor.
      template <class Sync>
      deferred_monitor(Sync &s, try_to_lock_type v) : lock_(s.mutex(), v), cond_(s.wait_condition()) {}

      //! Initialise the lock and condition and adopt a lock which is already owned by this thread.
      template <class Sync>
      deferred_monitor(Sync &s, adopt_lock_type v) : lock_(s.mutex(), v), cond_(s.wait_condition()) {}

      //! A duration for the *lock*.
      template <class Sync, class Duration>
      deferred_monitor(Sync &s, const Duration &d, duration_time_selector)
      : lock_(s.mutex(), d), cond_(s.wait_condition()) {}

      //! A deadline for the lock.
      template <class Sync, class AbsoluteTime>
      deferred_monitor(Sync &s, const AbsoluteTime &t, deadline_time_selector)
      : lock_(s.mutex(), t), cond_(s.wait_condition()) {}

      //! Release the lock.
      ~deferred_monitor() {}
      //@}

      //! \name Implementing the monitor head, returning false if there was a timeout.
      //@{

      //! Wait on the condition while predicate is NOT true.
      template <class Predicate>
      bool head(Predicate continue_pred) {
        while (! continue_pred()) {
          cond_.wait(lock_);
        }
        return true;
      }

      //! Perform a timed wait using a duration type.
      template <class Predicate, class Duration>
      bool head(Predicate continue_pred, const Duration &d, duration_time_selector) {
        return timed_head(continue_pred, d);
      }

      //! Timed wait with a deadline time.
      template <class Predicate, class AbsoluteTime>
      bool head(Predicate continue_pred, const AbsoluteTime &t, deadline_time_selector) {
        return timed_head(continue_pred, t);
      }

      //@}

      //! \name Accessors
      //@{

      //! \brief The condition which will be waited on.
      Condition &wait_condition() { return cond_; }
      const Condition &wait_condition() const { return cond_; }

      //! \brief The lock which is providing mutual exclusion during this instance's lifetime.
      //! Use this to get at the mutex.
      Lock &lock() { return lock_; }
      const Lock &lock() const { return lock_; }

      //@}

    protected:
      //! Timed wait.  Wrapper which just ignored our selector parameters.
      template <class Predicate, class Time>
      bool timed_head(Predicate continue_pred, const Time &t) {
        bool ok = true;
        while (! continue_pred()) {
          ok = cond_.timed_wait(lock_, t);
        }
        return ok;
      }

      //! \name Checked initialisers for subclasses
      //@{
      template <class LockError, class Predicate>
      void checked_head_failable_lock(Predicate cont_pred) {
        if (! lock().owns_lock()) throw LockError();
        head(cont_pred);
      }

      template <class LockError, class CondError, class Predicate>
      void checked_head_all_failable(Predicate cont_pred) {
        if (! lock().owns_lock()) throw LockError();
        if (! head(cont_pred)) throw CondError();
      }

      template <class LockError, class CondError, class Predicate, class Time>
      void checked_head_all_failable_timed(Predicate cont_pred, const Time &time) {
        if (! lock().owns_lock()) throw LockError();
        if (! timed_head(cont_pred, time)) throw CondError();
      }

      template <class CondError, class Predicate, class HeadParam>
      void checked_head_failable_wait(Predicate cont_pred, HeadParam time) {
        if (! timed_head(cont_pred, time)) throw CondError();
      }
      //@}

    private:
      Lock lock_;
      Condition &cond_;
  };


  /*!
  \ingroup grp_monitors
  \brief Monitor for failable locks/conditions which checks each operation is successful.

  Constructors do the same tasks as \link deferred_monitor \endlink; the descriptions here
  therefore simply indicate which exceptions are thrown.

  If the concept is known to succeed, then it is not checked.  This is the case with
  the conditions since they are not timed.  If you want to time the wait, then
  you must use either the deferred monitor or the exclusively timed monitors.


  TODO:
    perhaps it sould be called failable monitor?
  */
  template <class Lock, class Condition, class MonitorTraits = detail::default_monitor_traits>
  class checked_monitor : protected deferred_monitor<Lock,Condition,MonitorTraits> {
    typedef deferred_monitor<Lock,Condition,MonitorTraits> base;

    public:
      using base::condition_type;
      using base::lock_type;

      //! \name Aliases of MonitorTraits.
      //@{
      typedef typename base::try_to_lock_type try_to_lock_type;
      typedef typename base::adopt_lock_type adopt_lock_type;
      //@}

      //! \name Constructors/Destructors
      //@{

      //! \brief Initialise the lock and condition and use the try lock constructor.
      template <class Sync, class Predicate>
      checked_monitor(Sync &s, try_to_lock_type v, Predicate continue_pred) : base(s, v) {
        base::template checked_head_failable_lock<lock_owned_error>(continue_pred);
      }

      //! \brief Initialise the lock and condition and adopt a lock which is already owned by this thread.
      template <class Sync, class Predicate>
      checked_monitor(Sync &s, adopt_lock_type v, Predicate continue_pred) : base(s, v) {
        // TODO:
        //   Can the adopt lock fail?
        base::template checked_head_failable_lock<lock_owned_error>(continue_pred);
      }

      //! \brief Release the lock.
      ~checked_monitor() {}
      //@}

      using base::lock;
      using base::wait_condition;
  };

  /*!
  \ingroup grp_monitors
  \brief A simple monitor which waits on construction.

  This class is not intended for locks and conditions which can fail since the monitor
  head is called immediately, however, using lock models which throw exceptions will
  work.

  It could also be used for upgradable locks due to the mutex/lock accessors, providing
  the condition model supports it.
  */
  template <class Lock, class Condition, class MonitorTraits = detail::default_monitor_traits>
  class monitor : protected deferred_monitor<Lock,Condition,MonitorTraits> {
    typedef deferred_monitor<Lock,Condition,MonitorTraits> base;

    public:
      using base::condition_type;
      using base::lock_type;

      //! \brief Lock and wait on the condition while (! continue_predicate()).
      template <class Sync, class Predicate>
      monitor(Sync &s, Predicate continue_predicate) : base(s) {
        base::head(continue_predicate);
      }

      using base::lock;
      using base::wait_condition;
  };
}

#endif
