// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
\file
\brief Type traits and meta programming for locking library.
*/

#ifndef PARA_LOCKING_TRAITS_HPP_p16eecnl
#define PARA_LOCKING_TRAITS_HPP_p16eecnl

#include <para/locking/monitors.hpp>
#include <para/locking/timed_monitors.hpp>

namespace para {
  //! \ingroup grp_locking
  //! \brief Meta-programming class to generate the correct classes for a synchronisation tuple.
  //
  //TODO:
  //  First, traits is not really the right word.
  //
  //  Second, we need template typedefs to do this properly.  Perhaps a better
  //  way would be to:
  //
  //  - have this as part of the sync_tuple
  //    - tuples can typedef to this anyway
  //    - so: the todo is link to the traits class  from here.
  //  - maybe I should have sync_traits<Tuple> and sync_types<T,...> ?
  //    - nice idea, but the names are too similar
  //    - also the implementations are identical, I think.
  //
  //  sync_traits<my_sync_type> is much more pretty than the current way.  But
  //  it's a problem for the sync vs. monitor sync concepts because sync doesn't
  //  have anything.  Maybe I need sync_traits, monitor_sync_traits, and keep
  //  this as meta_types.
  //
  //  I could do my_sync_type::sync_types_type -- that is surely even nicer.
  //
  //  Third, the name is wrong.  It should be types.hpp, and
  //  sync_types<whatever>.
  template<class T, class Mutex, class Lock, class Condition>
  struct sync_traits {
    //! \name Synchronization types.
    //@{
    typedef T         value_type;
    typedef Mutex     lockable_type;
    typedef Lock      lock_type;
    typedef Condition condition_type;
    //@}

    //! \name Sync Tuple Types
    //@{
    typedef sync_tuple<value_type, lockable_type>                    sync_tuple_type;
    typedef monitor_bind<sync_tuple_type, condition_type>            monitor_bind_type;
    typedef monitor_tuple<value_type, lockable_type, condition_type> monitor_tuple_type;
    //@}

    //! \name Monitors
    //@{
    typedef deferred_monitor<lock_type, condition_type> deferred_monitor_type;
    typedef monitor<lock_type, condition_type> monitor_type;
    typedef checked_monitor<lock_type, condition_type> checked_monitor_type;
    //@}

    //! \name Timed Monitors
    //@{

    //! Essentially a template typedef.
    template<class AbsoluteTime>
    struct deferred_deadline_monitor {
      typedef para::deferred_deadline_monitor<lock_type, condition_type, AbsoluteTime> type;
    };

    template<class AbsoluteTime>
    struct deadline_monitor {
      typedef para::deadline_monitor<lock_type, condition_type, AbsoluteTime> type;
    };

    typedef deferred_duration_monitor<lock_type, condition_type> deferred_duration_monitor_type;
    typedef duration_monitor<lock_type, condition_type> duration_monitor_type;
    //@}

    //! \name Locked Access
    //@{
    typedef locked<value_type, lock_type, lockable_type> locked_type;
    /// TODO: add open_lcoked when that's done

    // TODO: add monitored (however that's going to work) when it's done.
    // typedef monitored<value_type, lockable_type, condition_type, monitor_tuple_type> monitored_type;
    // TODO: access pointers if I do that.

    //@}
  };
}


#endif
