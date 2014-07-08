// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
\file
\brief Base sync tuple structures and adaptor patterns.
*/

#ifndef PARA_LOCKING_TUPLES_HPP_gvbbxrz8
#define PARA_LOCKING_TUPLES_HPP_gvbbxrz8

namespace para {
  //! \ingroup grp_tuples
  //! \brief T which is guarded by a lockable (which is always mutable).
  //!
  //! T can be a reference, but only on gcc ~=4.4.3.  mingw 4.2 can't deal with
  //! it.
  //
  //TODO:
  //  Fix this -- it'll take ages I bet, but I think the type traits lib has
  //  enough to work around this.
  template <class T, class Mutex>
  class sync_tuple {
    public:
      // TODO: mutex_type is more obvious, but less accurate :)
      typedef Mutex lockable_type;
      typedef T     value_type;

      sync_tuple(const value_type &data = value_type()) : data_(data) {}

      lockable_type &mutex() { return mutex_; }
      const value_type &data() const { return data_; }
      value_type &data() { return data_; }

    private:
      mutable lockable_type mutex_;
      value_type data_;
  };

  //! \ingroup grp_tuples
  //! \brief Adds a condition to a \link sync_tuple \endlink.
  //!
  //! Use this to bind multiple condition variables to one synchronised
  //! data tuple.
  //!
  //! If you only need one condition, then \link monitor_tuple \endlink is
  //! just as good and quicker to write.
  template <class SyncType, class Condition>
  class monitor_bind {
    public:
      typedef typename SyncType::lockable_type lockable_type;
      typedef typename SyncType::value_type    value_type;
      typedef Condition               condition_type;

      monitor_bind(SyncType &sync) : sync_(sync) {}

      value_type &data() { return sync_.data(); }
      const value_type &data() const { return sync_.data(); }
      lockable_type &mutex() { return sync_.mutex(); }
      condition_type &wait_condition() { return cond_; }

    private:
      SyncType &sync_;
      mutable Condition cond_;
  };

  //! \ingroup grp_tuples
  //! \brief A composed \link sync_tuple \endlink and a \link monitor_bind \endlink.
  template<class T, class Mutex, class Condition>
  class monitor_tuple {
    public:
      typedef sync_tuple<T, Mutex>                     sync_tuple_type;
      typedef monitor_bind<sync_tuple_type, Condition> monitor_bind_type;

      typedef typename monitor_bind_type::condition_type condition_type;
      typedef typename monitor_bind_type::lockable_type  lockable_type;
      typedef typename monitor_bind_type::value_type     value_type;

      monitor_tuple(const T &data = T()) : sync_(data), monitor_sync_(sync_) {}

      const T &data() const { return monitor_sync_.data(); }
      T &data() { return monitor_sync_.data(); }
      lockable_type &mutex() { return monitor_sync_.mutex(); }
      condition_type &wait_condition() { return monitor_sync_.wait_condition(); }

    private:
      sync_tuple_type sync_;
      monitor_bind_type monitor_sync_;
  };

  namespace detail {
    //! \brief Created by \link get_monitor_adaptor() \endlink.
    template<class Mutex, class Condition>
    struct monitor_tuple_adaptor {
      typedef Mutex lockable_type;
      typedef Condition condition_type;

      Mutex &m;
      Condition &c;

      monitor_tuple_adaptor(Mutex &m, Condition &c) : m(m), c(c) {}
      Mutex &mutex() { return m; }
      Condition &wait_condition() { return c; }
    };

    //! \brief As \link ::para::detail::monitor_tuple_adaptor monitor_tuple_adaptor \endlink.
    template<class Mutex>
    struct sync_tuple_adaptor {
      Mutex &m;
      sync_tuple_adaptor(Mutex &m) : m(m) {}
      Mutex &mutex() { return m; }
    };
  }

  /*!
  \ingroup grp_locking
  \brief Returns a minimal monitor_tuple concept to initialise a monitor.

  This is to facilitate the use of there monitor patterns without using the sync framework-ish
  classes.
  */
  template <class Mutex, class Condition>
  detail::monitor_tuple_adaptor<Mutex,Condition> get_monitor_adaptor(Mutex &m, Condition &c) {
    return detail::monitor_tuple_adaptor<Mutex,Condition>(m,c);
  }

  //! \ingroup grp_locking
  //! \brief As \link para::get_monitor_adaptor() \endlink, for sync_tuple.
  template <class Mutex>
  detail::sync_tuple_adaptor<Mutex> get_sync_adaptor(Mutex &m) {
    return detail::sync_tuple_adaptor<Mutex>(m);
  }
}

#endif
