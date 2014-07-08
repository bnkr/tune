// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
\file
\brief Definition of a simple base class for smart pointers.
*/

#ifndef PARA_DETAIL_PTR_BASE_HPP_x1q097ze
#define PARA_DETAIL_PTR_BASE_HPP_x1q097ze

namespace para {
  namespace detail {
    /*!
    \brief Simple base class for smart pointer.

    Does not provide release() or reset() because they are part of the semantics of
    derrived classes.
    */
    template <class T>
    class ptr_base {
      public:
        ptr_base(T *v) : ptr_(v) {}

        //! \brief Reference to T
        T &operator*() { return *get(); }
        //! \brief Return the pointer (see get())
        const T &operator*() const { return *get(); }

        //! Return the pointer (see get())
        T *operator->() { return get(); }
        //! \brief Return the pointer (see get())
        const T *operator->() const { return get(); }

        //! \brief Return the pointer
        T *get() { return ptr_; }
        const T *get() const { return ptr_; }

        //! \brief Compares the value of the pointer.
        bool operator==(const ptr_base<T> &other) const {
          return (get() == other.get());
        }

        //! \brief See operator==()
        bool operator!=(const ptr_base<T> &other) const {
          return ! (*this == other);
        }

      protected:
        T *ptr_;
    };
  }
}

#endif
