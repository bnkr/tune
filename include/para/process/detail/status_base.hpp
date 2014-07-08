// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Some common stuff between win32 and unix
 */

#ifndef PARA_PROCESS_STATUS_BASE_HPP_fukvktle
#define PARA_PROCESS_STATUS_BASE_HPP_fukvktle

#include <cerrno>
#include <cstring>

namespace para {
  namespace process {
    namespace detail {

      //! Simple class to ensure portability.  We use the static polymorphism
      //! trick so the subclass can implement exited and exit_status() and,
      //! presumably override signal stuff in the case of unix.
      template<class Derrived>
      class status_base {
        public:
          bool success() const {
            // static cast sometimes complains and sometimes it doesn't --
            // depends on version of gcc.  If any more problems I guess use a
            // c-cast.
            const Derrived *const d = reinterpret_cast<const Derrived *>(this);
            return d->exited() && d->exit_status() == 0;
          }

          bool signaled() const { return false; }
          int signal() const { return 0; }
      };
    }
  }
}

#endif
