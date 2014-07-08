// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
 * \file
 * \brief Aggregator for process library.
 * \ingroup grp_proc
 */

#ifndef PARA_PROCESS_HPP_ll5sau5t
#define PARA_PROCESS_HPP_ll5sau5t

#include "process/exceptions.hpp"
#include "process/programs.hpp"
#include "process/process.hpp"
#include "process/ipc_pipe.hpp"
#ifndef WIN32
#  include "process/unix_fork.hpp"
#  include "process/unix_named_process.hpp"
#endif
#include "process/named_process.hpp"
#include "process/redirection.hpp"

#endif
