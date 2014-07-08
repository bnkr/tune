// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Portability wrapper to implementations of named_process.
 */

#ifndef PARA_PROCESS_NAMED_PROCESS_HPP_8m3v8k2z
#define PARA_PROCESS_NAMED_PROCESS_HPP_8m3v8k2z

#ifdef WIN32
#  include "detail/named_process_win32.hpp"
#  define PARA_SYSTEM_NAMED_PROCESS_TYPE detail::named_process_win32
#else
#  include "detail/named_process_unix.hpp"
#  define PARA_SYSTEM_NAMED_PROCESS_TYPE detail::named_process_unix
#endif

#ifdef __GNUC__
#  define PARA_ATTRIBUTE_DEPRECATED __attribute__((deprecated))
#else
#  define PARA_ATTRIBUTE_DEPRECATED
#endif


namespace para {
  namespace process {
    //! \ingroup grp_proc
    //! Portability typedef to the system's named process implementation.  See
    //! docs of \link detail::named_process_unix \endlink.
    typedef PARA_SYSTEM_NAMED_PROCESS_TYPE named_process;

    typedef named_process redirected_process PARA_ATTRIBUTE_DEPRECATED;
  }
}

#undef PARA_SYSTEM_NAMED_PROCESS_TYPE

#endif
