// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Wrapper for the base process classes.
 */

#ifndef PARA_PROCESS_PROCESS_HPP_7fuelkix
#define PARA_PROCESS_PROCESS_HPP_7fuelkix

#ifdef WIN32
#  include "detail/process_win32.hpp"
#  define PARA_SYSTEM_PROCESS_TYPE detail::process_win32
#  define PARA_SYSTEM_STATUS_TYPE detail::status_win32
#  define PARA_SYSTEM_GETPID() detail::current_pid_win32()
#else
#  include "detail/process_unix.hpp"
#  define PARA_SYSTEM_PROCESS_TYPE detail::process_unix
#  define PARA_SYSTEM_STATUS_TYPE detail::status_unix
#  define PARA_SYSTEM_GETPID() detail::current_pid_unix()
#endif

namespace para {
  namespace process {
    //! \ingroup grp_proc
    //! See \link detail::process_unix \endlink for docs.
    typedef PARA_SYSTEM_PROCESS_TYPE process;
    //! \ingroup grp_proc
    //! See \link detail::status_unix \endlink.
    typedef PARA_SYSTEM_STATUS_TYPE status;

    //! \ingroup grp_proc
    //! Portable pid.
    typedef detail::pid_type pid_type;

    //! \ingroup grp_proc
    inline pid_type current_pid() { return PARA_SYSTEM_GETPID(); }
  }
}

#undef PARA_SYSTEM_PROCESS_TYPE
#undef PARA_SYSTEM_STATUS_TYPE
#undef PARA_SYSTEM_GETPID

#endif
