// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
\file
\brief The locked type.
*/

#ifndef PARA_LOCKING_LOCKED_HPP_59ilof1i
#define PARA_LOCKING_LOCKED_HPP_59ilof1i

#include <para/detail/ptr_base.hpp>
#include <para/locking/tuples.hpp>

namespace para {
  //! \ingroup grp_access
  //! \brief Class which forces access to only be through an access pointer.
  //!
  //! This is initialised with a copy of T -- there is only ever one access to
  //! it and that is via. the pointer.
  //!
  // TODO:
  //   Maybe take a reference to the sync_tuple.  It kind of messes the concept
  //   of this thing up, though, but it would make stuff much more flexible.  It
  //   might even be possible to meta-program a class access_ptr_protected<Sync>
  //   where Sync can be monitor or lock.  I think this is a little unclear,
  //   though.
  //
  // TODO:
  //   we still need locking with different types of lock (defer, timed etc.),
  //   timed_locked, checked_locked - perhaps these should just be different
  //   types of pointers?
  template <class T, class Mutex, class Lock>
  class locked {
    public:
      //! \brief Scoped access pointer.
      class access_ptr : protected detail::ptr_base<T> {
        typedef detail::ptr_base<T> base;

        public:
          template<class Locked>
          access_ptr(Locked &data)
          : base(&data.sync().data()),
            lock_(data.sync().mutex()) {
          }

          //! \brief Accessor to the mutex model.  Always prefer to use a lock guard concept.
          Mutex &mutex() { return lock_.mutex(); }
          const Mutex &mutex() const { return lock_.mutex(); }

          //! \brief Accessor to the lock model.
          Lock &lock() { return lock_; }
          const Lock &lock() const { return lock_; }

          using base::get;
          using base::operator==;
          using base::operator!=;
          using base::operator*;
          using base::operator->;

        private:
          Lock lock_;
      };

      locked(const T &data = T()) : sync_(data) {}

    protected:
      typedef sync_tuple<T, Mutex> sync_type;
      sync_type sync_;

      sync_type &sync() { return sync_; }
  };
}

#endif
