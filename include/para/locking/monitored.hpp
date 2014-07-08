// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
\file
\brief The monitored guardian types.
*/
#ifndef PARA_LOCKING_MONITORED_HPP_rzsq1emp
#define PARA_LOCKING_MONITORED_HPP_rzsq1emp

// this isn't working yet
#if 0


namespace para {
  /*!
  \ingroup grp_access
  \brief Uses the sync tuples to force there to be a single way of accessing the data.

  The SyncTuple is changable here to allow a different organisation of conditions, which is
  often advantegeous in order to have seperate conditions for seperate tasks but still
  declare them in the same data structure.

TODO:
  frequently I need to access T without the condition.  Therefore, I need to
  be able to have a locked access as well as a monitored access.  For example,
  if T is a queue, then size() needs to be locked but probably not monitored.
  */
  template <class T, class Mutex, class Lock, class Condition,
            class SyncTuple = monitor_tuple<T, Mutex, Condition>,
            class WaitPredicate = boost::function0<bool> >
  class monitored {
    public:
      // TODO: add a locked_ptr

      //! \brief Scoped access pointer which uses a scoped_monitor.
      class monitored_ptr : protected detail::ptr_base<T> {
        typedef detail::ptr_base<T> base;

        public:

          // TODO:
          //   design problem.  Sometimes you want to use a different wait condition.  It would
          //   be nicer to have a parameter here for the condition.  It's annoying though.  Perhaps
          //   different forms of smart pointers are used?

          template <class Monitored>
          monitored_ptr(Monitored &data)
          : base(&data.sync().data()),
            lock_(data.sync().mutex()),
            mon_(lock_, data.sync().wait_condition(), data.monitor_head()) {
          }

          using base::get;
          using base::operator==;
          using base::operator!=;
          using base::operator*;
          using base::operator->;

          //! \brief Accessor to the lock guard.  Mostly for upgradable locks.
          Lock &lock() { return cond_; }
          const Lock &lock() const { return cond_; }

        private:
          Lock lock_;
          monitor<Lock, Condition> mon_;
      };

      //! \brief Use pred as the continue_predicate of the monitor.
      monitored(monitor::predicate_type pred, const T &data = T())
      : sync_(data), monitor_head_(boost::bind(predicate_wait<Lock,Condition>(), _1, _2, pred)) {}

      //! \brief Use monitor_head as the entire head of the monitor.
      monitored(monitor::monitor_head_type monitor_head, const T &data = T())
      : sync_(data), monitor_head_(monitor_head) {}

    protected:
      SyncTuple sync_;
      MonitorHead monitor_head_;

      //! \brief Return the SyncTuple assosciated with this object.
      SyncTuple &sync() { return sync_; }
      //! \brief Accessor for the wait predicate.
      WaitPredicate monitor_head() const { return monitor_head_; }
  };
}

#endif

#endif

