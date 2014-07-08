// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Definitions of monitors which use times.
 */

#ifndef PARA_LOCKING_TIMED_MONITORS_HPP_psgixs93
#define PARA_LOCKING_TIMED_MONITORS_HPP_psgixs93

#include <para/locking/monitors.hpp>

namespace para {
  // TODO:
  //   The names are too long here (deferred_).


  typedef boost::system_time default_deadline_time_type;

  //! \ingroup grp_monitors
  //!
  //! A deferred monitor which uses absolute deadline times, though you can use
  //! a separate one for the lock and condition steps.
  //!
  //! Note the template argument exists so that all times are bound to the same
  //! type.  This makes it unsuitable for duration types.
  //!
  //! You may not mix timed and non-timed concepts using this class because
  //! that is complicated and confusing.  Use deferred_monitor for that kind of
  //! stuff (in other words, you can't use try_lock and adopt_lock selectors).
  //! Ditto duration and absolute times.
  //
  //TODO:
  //  MonitorTraits is a bit redundant; also AbsTime might be better got from a
  //  traits class (because it's 90% going to be the boost type anyway).  In
  //  fact, I may as well remove the traits class in this case becuase it's not
  //  used at all -- maybe I can make a null traits class so I don't need to
  //  include boost?
  template <class Lock, class Condition,
            class AbsoluteTime = default_deadline_time_type,
            class MonitorTraits = detail::default_monitor_traits>
  class deferred_deadline_monitor : protected deferred_monitor<Lock, Condition, MonitorTraits> {
    typedef deferred_monitor<Lock, Condition, MonitorTraits> base_type;

    public:
      using base_type::lock_type;
      using base_type::condition_type;

      template<class Sync>
      deferred_deadline_monitor(Sync &s, const AbsoluteTime &lock_deadline)
      : base_type(s, lock_deadline, typename base_type::deadline_time_selector()) { }

      template<class Predicate>
      bool head(Predicate continue_pred, const AbsoluteTime &wait_deadline) {
        return base_type::head(continue_pred, wait_deadline, typename base_type::deadline_time_selector());
      }

      using base_type::wait_condition;
      using base_type::lock;
  };

  //! \ingroup grp_monitors
  //!
  //! A checked monitor with an absolute time deadline for the lock *and* the
  //! wait to be completed in.
  template <class Lock, class Condition,
            class AbsoluteTime = default_deadline_time_type,
            class MonitorTraits = detail::default_monitor_traits>
  class deadline_monitor : protected deferred_monitor<Lock, Condition, MonitorTraits> {
    typedef deferred_monitor<Lock, Condition, MonitorTraits> base_type;

    public:
      using base_type::lock_type;
      using base_type::condition_type;

      template<class Sync, class Predicate>
      deadline_monitor(Sync &s, const AbsoluteTime &lock_and_wait_deadline, Predicate continue_pred)
      : base_type(s, lock_and_wait_deadline, typename base_type::deadline_time_selector()) {
        typedef lock_timeout_error lk_err;
        typedef wait_timeout_error wt_err;
        base_type::template checked_head_all_failable_timed
          <lk_err,wt_err>(continue_pred, lock_and_wait_deadline);
      }

      using base_type::lock;
      using base_type::wait_condition;
  };

  //! \ingroup grp_monitors
  //!
  //! An unchecked monitor with a duration with for each step.  The assumption
  //! is that you'll use the seconds, microseconds etc. aliases.
  template <class Lock, class Condition,
            class MonitorTraits = detail::default_monitor_traits>
  class deferred_duration_monitor : protected deferred_monitor<Lock, Condition, MonitorTraits> {
    typedef deferred_monitor<Lock, Condition, MonitorTraits> base_type;

    public:
      using base_type::lock_type;
      using base_type::condition_type;

      template<class Sync, class Duration>
      deferred_duration_monitor(Sync &s, const Duration &lock_duration)
      : base_type(s, lock_duration, typename base_type::duration_time_selector()) {}

      template<class Predicate, class Duration>
      bool head(Predicate continue_pred, const Duration &d) {
        return base_type::head(continue_pred, d, typename base_type::duration_time_selector());
      }

      using base_type::wait_condition;
      using base_type::lock;
  };

  //! \ingroup grp_monitors
  //!
  //! A checked monitor which uses the same timeout value for each step.  The
  //! timeout is *not* time(lock) + time(wait).
  template <class Lock, class Condition,
            class MonitorTraits = detail::default_monitor_traits>
  class duration_monitor : protected deferred_monitor<Lock, Condition, MonitorTraits> {
    typedef deferred_monitor<Lock, Condition, MonitorTraits> base_type;

    public:
      using base_type::lock_type;
      using base_type::condition_type;

      template<class Sync, class Predicate, class Duration>
      duration_monitor(Sync &s, const Duration &lock_and_wait_duration, Predicate continue_pred)
      : base_type(s, lock_and_wait_duration, typename base_type::duration_time_selector()) {
        typedef lock_timeout_error lk_err;
        typedef wait_timeout_error wt_err;
        base_type::template checked_head_all_failable_timed
          <lk_err,wt_err>(continue_pred, lock_and_wait_duration);
      }

      using base_type::lock;
      using base_type::wait_condition;
  };

} // ns para


#endif
