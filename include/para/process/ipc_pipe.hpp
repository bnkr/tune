// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Portability wrapper to the unix/win32 implementation of ipc_pipe.
 */

#ifndef PARA_PROCESS_IPC_PIPE_HPP_ksffv19l
#define PARA_PROCESS_IPC_PIPE_HPP_ksffv19l

#ifdef WIN32
#  include "detail/ipc_pipe_win32.hpp"
#  define PARA_PROC_SYSTEM_IPC_PIPE_TYPE detail::ipc_pipe_win32
#else
#  include "detail/ipc_pipe_unix.hpp"
#  define PARA_PROC_SYSTEM_IPC_PIPE_TYPE detail::ipc_pipe_unix
#endif

namespace para {
  namespace process {
    //! \ingroup grp_proc
    //! Portable typedef to the ipc_pipe implementation.  See docs for
    //! \link detail::ipc_pipe_unix \endlink.
    typedef PARA_PROC_SYSTEM_IPC_PIPE_TYPE ipc_pipe;

    //! \ingroup grp_proc
    //! Portable typedef to the system's handle type.
    typedef detail::handle_type handle_type;
  }
}

#undef PARA_PROC_SYSTEM_IPC_PIPE_TYPE

#endif
