// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Portability wrapper for the $STREAM_redirect classes.
 */

#ifndef PARA_PROCESS_REDIRECTION_HPP_2tw0g8yd
#define PARA_PROCESS_REDIRECTION_HPP_2tw0g8yd

#ifdef WIN32
#  include "detail/redirection_win32.hpp"
#  define PARA_SYSTEM_IN_REDIRECT_TYPE detail::stdin_redirect_win32
#  define PARA_SYSTEM_OUT_REDIRECT_TYPE detail::stdout_redirect_win32
#  define PARA_SYSTEM_BOTH_REDIRECT_TYPE detail::both_redirect_win32
#else
#  include "detail/redirection_unix.hpp"
#  define PARA_SYSTEM_IN_REDIRECT_TYPE detail::stdin_redirect_unix
#  define PARA_SYSTEM_OUT_REDIRECT_TYPE detail::stdout_redirect_unix
#  define PARA_SYSTEM_BOTH_REDIRECT_TYPE detail::both_redirect_unix
#endif

namespace para {
  namespace process {
    //! \ingroup grp_redir
    //! Portable wrapper to redirection class.  See docs for
    //! \link para::process::named_process \endlink.
    typedef PARA_SYSTEM_IN_REDIRECT_TYPE   stdin_redirect;
    //! \ingroup grp_redir
    //! Portable wrapper to redirection class.  See docs for
    //! \link para::process::named_process \endlink.
    typedef PARA_SYSTEM_OUT_REDIRECT_TYPE  stdout_redirect;
    //! \ingroup grp_redir
    //! Portable wrapper to redirection class.  See docs for
    //! \link para::process::named_process \endlink.
    typedef PARA_SYSTEM_BOTH_REDIRECT_TYPE both_redirect;
  }
}

#undef PARA_SYSTEM_IN_REDIRECT_TYPE
#undef PARA_SYSTEM_OUT_REDIRECT_TYPE
#undef PARA_SYSTEM_BOTH_REDIRECT_TYPE

#endif
