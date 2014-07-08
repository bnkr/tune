// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
\file
\brief Aliases to boost types for locking concepts.
*/

#ifndef PARA_LOCKING_DETAIL_BOOST_HPP_yapod2t3
#define PARA_LOCKING_DETAIL_BOOST_HPP_yapod2t3

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

namespace para {
  namespace detail {
    //! \brief Plugs boost's types into the monitor types.
    struct boost_monitor_traits {
      typedef boost::defer_lock_t     defer_lock_type;
      typedef boost::try_to_lock_t    try_to_lock_type;
      typedef boost::adopt_lock_t     adopt_lock_type;
      typedef boost::system_time      absolute_time_type;
    };

    typedef boost_monitor_traits default_monitor_traits;
  }

  using boost::defer_lock;
  using boost::try_to_lock;
  using boost::adopt_lock;
}

#endif
