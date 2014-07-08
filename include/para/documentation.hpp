// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
\file
\brief Doxygen stuff.
*/

/*!
\mainpage

\section s_mp_intro Introduction

Para is a template library for parallel programming.  It currently deals with locking patterns
using blocking mutexes as defined in boost and the STL, generic lock-free data structures, and
portable multi-processing.

The documentation is separated into two sections.  The tutorial section uses doxygen pages
to provide a fairly basic overview of the functionality, and to provide copy-pasteable
example code which is useful for learning the library.  The reference section provides a more
complete documentation of all the components of the library and is intended to be for continued
use.  The tutorials link to the reference in order to provide starting points for implementing
programs with this library.  Tutorial style documentation can be found via the `related pages'
tab when this manual is in HTML format, or as each chapter when this is in LaTeX format.

\section s_mp_copying License

Para is licensed under a 3-clause BSD license.  Most is Copyright (C) 2008-2009, James
Webber.  Other contributors are noted in the source.

\section s_mp_deps Dependencies

The multi-processing library must be linked to the boost.system library and the locking parts
depend on boost.thread.  On windows, boost.thread needs to be linked against boost system.  The
entire library uses various header-only parts of boost.  Para itself is header only.

\section s_mp_versions Version History

\par 0.0.0
- non-timed monitors
- locked<T>
- sync tuples
- multi-processing api
- timed monitors

\section s_mp_plans Future Versions

- \b ++y  monitored<T>
- \b ++y  locked<T> with deferred locking (etc.)
- \b ++y  generic lock-free design, lock free list
- \b ++y  further lock free structures
*/

/******************
 * Tutorial Pages *
 ******************/

/*!
\page pg_locking Locking

\section s_locking_intro Introduction

The locking library is a concept-based template library for lock-based programming.  It
implements monitors and locked-access types, aiming to provide inter-related scoped types
which remove the possibility for common errors in lock-based programming.  Given the
concept-based design, it also aims to be flexible to the extent that, if necessary, separate
types can be defined when the available ones are limited.

This chapter is a documentation of the basic concepts for the locking module, and
for the most general part of it -- the \ref grp_sync_tuples "Sync Tuples" sub-module --
since the rest cannot be used without that.

\section s_locking_depends Dependencies

An implied dependency is on boost::thread, but any library which satisfies the concepts
documented here and in boost will work (this includes parts of the C++0x standard library).

\section s_locking_review A Brief Review of Boost

This is a quick and dirty section to review the boost stuff.  Hopefully it should
save you some of the perplexing compile errors that can happen:

Mutex concepts:

- Lockable -- exclusive ownership mutex.  Has lock(), try_lock() etc.
- TimedLockable -- adds timed_lock() to above.
- SharedLockable -- adds to above *_shared();
- UpgradeLockable -- refinement of SharedLockable.  You gain "upgrade ownership"
  in order to request unique access.  Adds the *_upgrade() functions.

Each of these have the following very handy typedefs which implement the lock
guard scoped locker.

- scoped_lock -- normally becomes unique_lock
- scoped_try_lock
- scoped_timed_lock (only TimedLockable)

Lock types:

- lock_guard<Lockable> -- the very basic scoped lock; allows an adopt_lock selector
- unique_lock<Lockable> -- more complicated lock which allows the "defer", "try"
  and "adopt" lock selectors.
- shared_lock<SharedLockable> -- just like unique lock but aquires non-exclusive
  ownership (so it calls lock_shared());
- upgrade_lock<UpgradeLockable> -- a Lockable which acquires "upgrade ownership"
- upgrade_to_unique_lock -- takes an upgrade_lock, in effect turning it into a unique
  lock.
- Mutex::scoped_try_lock -- takes the adopt, defer, or try_to_lock selectors.

Note that some lock types are models of a Lockable concept themselves.

Mutex types:

- mutex -- implements lockable.
- timed_mutex -- implements TimedLockable.
- reursive_mutex -- Lockable which can be locked twice in the same thread.
- timed_recursive_mutex.
- shared_mutex -- the SharedLockable implementation

Condition variables:

- condition_variable -- takes only a unique_lock<mutex> in its wait().  Also has
  timed_wait().
- condition_variable_any -- takes any kind of lockable.

Note that, for all monitor classes apropriate, the selector types are typdefed
into the class's scope.

\section s_locking_conventions Conventions

- Synchronisation types are always in the order: Mutex, Lock, Condition.
- They are always named as such.
- The sync tuples come first.
- Predicates come last in the constructor; lock-related stuff comes before it.
- Parameters are in the same order in overloads (so in deferred head() functions,
  the predicate is first).

\section s_locking_tuples Synchronisation Tuples

The `core' concept is the synchronisation tuple.  This is a simple container which holds
the synchronisation types.  The following member functions are expected:

- mutex(), which returns any of the boost.thread Mutex concepts.
- wait_condition(), which returns any of the boost.thread Condition concepts.  This is only
  used by the monitor types.
- data(), which returns the data type to work with.  This is only used by the locked-access
  types.

The provided types are \link ::para::sync_tuple \endlink, \link ::para::monitor_bind \endlink,
and \link ::para::monitor_tuple \endlink.

The doxygen module is \ref grp_sync_tuples "Synchronisation Tuples".

\section Monitors

The documentation for monitors is on its own page: \ref pg_monitors "Monitors"

\section s_locking_meta Meta-Construction

Since the types used in the locking library can be rather complicated, the
\link ::para::sync_traits \endlink class composes the four template parameters --- T, Mutex, Lock,
and Condition --- and has member typedefs which give the requested monitor, locked, and tuple
types.  Most of this won't mean much to you until you read about the various types
used here, but it is worth bearing in mind as a simpligied interface when the
tutorial examples write things out "long-hand".

Example:

\include locking/meta_types.cpp

*/

/*!
 *
 * \page pg_monitors Monitors
 *
 * \section s_monitors_intro Introduction
 *
 * The monitor pattern is a powerful synchronisation and inter-thread communication
 * pattern.  A monitor uses a condition variable and a lock to synchronise access to
 * a variable and also to provide a way to cause a thread to become active.  The general
 * form is:
 *
\verbatim
// Obtain mutual exclusion.
lock_type lock(mutex);
while (! data_ready) {
  // release lock and wait until notified; after notify, re-acquire
  // the lock.  Since this process may not be entirely atomic, it is
  // in a loop.
  cond.wait(lock);
}
// work with the shared resource here
// then (usually) inform the other thread that this one is finished.
\endverbatim
  *
  * The loop here is referred to as the monitor head.  In this library, when the head
  * is done manually, we call it ``deferred''.
  *
  * The most simple type, \link para::monitor \endlink, implements this directly.  In the
  * following example, we assume that the calling thread is reading from \c some_structure
  * until \c stream_finished is \c true and the structure is empty.
  *
  * \include monitors/general_monitor_pattern.cpp
  *
  * This quick example covers the most common usage of the library.
  *
  * \warning
  *   The wait_pred should also contain the check to terminate the algorithm
  *   if there is one.  Otherwise, you can receive a notify() which tells you
  *   to quit whilst you are doing work.  Then the algorithm will block forever
  *   on the condition wait.  So to reiterate: it is crucial that you check
  *   whether to terminate before waiting on the condition.
  *
  * Note that we declare both a sync_tuple and a monitor_bind.  This is so that
  * monitors can use the same mutex object to protect the data, but a different
  * condition to wait on it, as is common when having multiple threads and a
  * required ordering (eg, a producer-consumer thread model).
  *
  *
  * \section s_monitors_usage Usage
  *
  * This section is a simple by-example overview of the available classes and
  * their usage.  There is usually more detail in the per-class reference.
  *
  * In general, a monitor is an RAII type which contains a lock.  It holds a
  * reference to the synchronisation data (see the section on
  * \ref s_locking_tuples "Synchronisation Tuples") and either has a
  * deferred predicate-based head() or calls same from the constructor.
  * Here the deferred_ classes are unchecked while the others check the locks/waits
  * and throw exceptions if they fail.
  *
  * Note that the predicate is always inverted, since this is consistent with the
  * implementation of boost: it is a `do I continue?' predicate, not a `do I
  * wait?' predicate.
  *
  * In this section, let's assume that you have the following header included:
  *
  * \include monitors/type_header.hpp
  *
\subsection ss_mon_overview_deferred_mon Class deferred_monitor

The \link para::deferred_monitor \endlink represents the most complicated monitor, and
the most error-prone.  It allows the selector types to construct the lock type which
defer locking until a later point or can fail in some way.  The monitor head must
be run manually.  All the other monitors are implemented in terms of this monitor
so it is encouraged that you simply use one of its protected-inhert subclasses.

It is the only monitor which allows you to mix absolute times, durations, and lock
selectors.  In particular, mixing the two different types of time can get very
confusing.

\include monitors/deferred_monitor.cpp

\subsection ss_mon_overview_checked_mon Class checked_monitor

The \link para::checked_monitor \endlink checks the lock and wait was successful and
throws an exception if it was not.  The monitor head is called immediately in this
class, so it is less error prone.  It does not cover timed locks.

\include monitors/checked_monitor.cpp

\subsection ss_mon_overview_def_dur_mon Class deferred_duration_monitor

\link para::deferred_duration_monitor \endlink takes one of the duration aliases
to time the lock and wait separately.

Note that the boost docs for the synchronisation types explicitly states that
using duration types is very difficult and not really advised.

\include monitors/deferred_duration_monitor.cpp

TODO:
  I'm not sure of the semantics of durations.  Will the duration time be
  editted by reference?  I need to make it clear (eg, when using a unix time
  to wait for a socket or something, it will decremet the time value to show
  how much time was taken to wait.

\subsection ss_mon_overview_dur_mon Class duration_monitor

A checked variation of the \ref para::deferred_deadline_monitor "deferred_duration_monitor".
This throws \ref para::lock_timeout_error "lock_timeout_error" or
\ref para::wait_timeout_error wait_timeout_error on timeouts.

\include monitors/duration_monitor.cpp

\subsection ss_mon_overview_def_dead_mon Class deferred_deadline_monitor

\ref para::deferred_deadline_monitor "deferred_deadline_monitor" uses an absolute
time to check the wait and the lock.

\include monitors/deferred_deadline_monitor.cpp

\subsection ss_mon_overview_dead_mon Class deadline_monitor

\link para::deadline_monitor \endlink is as above but checks for timeouts the same
way that the \link para::checked_monitor \endlink does.  It uses the same deadline
for the lock and the wait.

\include monitors/duration_monitor.cpp

\subsection ss_mon_overview_monitor Class monitor

The basic class \link para::monitor \endlink simply implements the basic monitor pattern, and only
allows access to the condition and lock.  This monitor is most efective with blocking locks and
waits; there is intentionally no way to check that a lock succeeded.  It is also compatible with
failable lock models which throw exceptions if they are not successful.

\include monitors/monitor.cpp

\section s_monitor_compat Monitor Compatibility

The earlier examples all assumed the use of the framework-like synchronised tuple types, however
they are intrusive, and therefore usage may not be desired.  In this case the function
\link para::get_monitor_adaptor \endlink will return a minimal concept which addresses the mutex and
condition passed as parameters.  It is intended that this would be used as a by-value temporary
expression as the compiler should be able to optimise it out.

*/

/*!
 * \page pg_access Locked Access

\section s_access_intro Introduction

This module aims to restrict access to shared data to a single smart-pointer based
interface.  The reference module is \ref grp_access "Locked Access"

\section s_access_locked Simple Locking

Simple locking is the most straight-forward part of the library.   The \link para::locked \endlink
type is an expansion on \link para::sync_tuple \endlink and provides a member type
\link para::locked::access_ptr \endlink which allows access to the variable.  Access is only
obtainable through an instance of this type, and synchronisation is only based on the
sync_tuple held within the locked type.  In this way synchronised access is controlled
very strongly with a single consistent interface to the data which is guarded by the
synchronisation primitives.

\include locking/locked.cpp

TODO:
  There is currently no design for this, but I need a way of doing failable
  locks with a locked<T>-like class.

\section s_access_monitored Monitored Types

TODO:
  The implementation here is completely bruk, so these docs will certainly change.

Like \link para::locked \endlink, \link para::monitored \endlink implements a monitor pattern
where access is required to be via a smart pointer.  Usage is the same as locked, but the
constructor also takes the necessary functors to initialise the internal monitor object.

TODO: more here about monitored types using differed monitor concepts.



*/



// nb: sometimes doxygen won't like to stuff when it's given a fully qualified
// namespace and sometimes it will.  Therefore we have to be very careful about
// how \ref names are specified.  In particular, it seems it won't link to
// typedefs when they are fully qualified.
//
// In other words: don't change the links!

/*!
 * \page pg_process Multi-Processing
 *
 * \section s_process_intro Introduction
 *
 * The multi-processing library gives a portable thread-like interface to
 * process launching.  There are three main classes:
 *
 * - \ref para::process::process "process::process"
 *   -- reference to an existing process.
 * - \ref para::process::named_process "process::named_process"
 *   -- launch a binary with optional redirection.
 *
 * In addition, we have the following classes only on unix:
 *
 * - \ref ::para::process::unix_fork "process::unix_fork"
 *   -- run a functor in another process.
 * - \ref ::para::process::unix_named_process "process::unix_named_process"
 *   -- as named_process, but has a more general hooks class instead of a fixed
 *      redirector, and the ability to supply a functor to deal with exec
 *      failure.
 *
 * Finally, there are classes for i/o redirection:
 *
 * - \ref para::process::ipc_pipe "process::ipc_pipe" -- an asynchronous,
 *   anonymous pipe.
 * - \ref para::process::stdin_redirect "process::stdin_redirect",
 *   \ref para::process::stdout_redirect "process::stdout_redirect",
 *   \ref para::process::both_redirect "process::both_redirect",
 *   -- redirectors for each of the standard file streams to a pipe.
 *
 * Note that most classes are hidden behind typedefs because of the differences
 * between Unix and win32.
 *
 * \section s_process_portability Portability
 *
 * The interface of unix forking and windows process creation are very difficult
 * to make compatible.  They are even more difficult to make them semantically
 * equivalent.  Unfortunately, Windows does not have the power of the Unix API
 * and making a common API would lose this power.  Therefore, what we have is a
 * completely portable interface and a very similar interface with differing
 * semantics and class names.  This way, one is limited to a minimal ifdef and
 * the use of a separate binary if you do multi-level forks.
 *
 * All unix-only classes and functions begin unix_.  These are the more generic
 * thread-like classes which can't be implemented on win32, but act as the basis
 * of the unix version of the portable interface.
 *
 * There are also performance differences where certain behavior has been
 * emulated, particularly when dealing with different binary name semantics
 * which are documented below.
 *
 * \section s_process_tag_rationale Rationale For Tags
 *
 * TODO:
 *   copy this rationale from TODO.redirection-api (near the bottom, I think)
 *
 * \section s_process_interface Process Interface
 *
 * All process classes inherit the basic interface of the process class.  This
 * class can also be used on its own.
 *
 * \include process/process.cpp
 *
 * Note that opening a process by pid is always a race condition because the
 * state of a process is necessarily temporal and you cannot (at least I hope
 * not!) lock the entire system.
 *
 * \section s_process_basic Basic Process Launching
 *
 * Process specification is rather complicated since there are different
 * semantics for searching for binaries on the windows and unix platform.  Para
 * attempts to leave you the option of a system-dependant search, or a portable
 * one, as well as keeping the door open for a very customised semantic.
 *
 * To begin with, I will explain quickly for the impatient:
 *
 * - use one of the *_binary classes
 * - if you want to set the environment block then wrap it in an environment_binary
 *   class.
 * - you can use a char** or a vector<char*> as the container parameter (so
 *   basically any contiguous block of char pointers).
 *
 * Here are a couple of examples:
 *
 * \include process/named_process.cpp
 *
 * Here is a list of all the binary classes available:
 *
 *   TODO: write them here (I haven't finished doing it ^^)
 *
 * \section s_process_errors Errors
 *
 * Para uses the boost.system library to report errors with the non-portable
 * API.  The errors thrown are all in exceptions.hpp; they are trivial to
 * understand.  Of note is that the portable error code can be accessed with the
 * error_code() member, and that what() is a message constructed from a reason
 * and the error_code's message.
 *
 * The following exceptions are used:
 *
 * - create_error -- when a process can't be created.
 * - program_error -- when the program in a named_process can't be found or
 *   execute.
 * - pipe_error -- couldn't create a pipe.  Note that you should also check for
 *   this when creating a process, since Unix uses pipes to detect errors.
 * - pid_error -- couldn't open a process (ie, the pid didn't exist or similar).
 * - signal_error -- couldn't terminate an error.
 * - wait_error -- failure waiting on process.
 *
 * Note that the para library throws the same errors in the same places on
 * windows and unix, but this is not the case in the non-portable unix classes.
 * Specifically, the failure functor and redirectors control throwing exceptions
 * for errors which happen in the child process.
 *
 * Naturally, the precise conditions of errors are not the same, but the aim is
 * to ensure a process which is running with the same guarantees on both
 * platforms (eg, an exception if exec() fails on unix because the same happens
 * on windows).  If you're interested, grep for throw_system_error to see what
 * I do to make error conditions as similar as possible..
 *
 * \section s_process_redirecting Redirecting I/O
 *
 * TODO:
 *   This is a bit bruken.  See TODO.redirection
 *
 * I/O redirection uses a symbolic class *_redirect.  Since the piping is so
 * different on unix and windows, these classes do not have a public API.
 * Instead the system's implementation of redirected_process deals with them in
 * a non-portable way which, again, is why they are hidden behind typedefs.
 * They are template arguments to avoid lots of overloads, copying, or virtual
 * functions.
 *
 * On unix the redirects are implemented using hooks (see later), so you can also
 * use this API to redirect i/o with the unix_fork class and the
 * unix_named_process class.
 *
 * TODO:
 *   Above statement might be false later.
 *
 * \include process/named_process_redirected.cpp
 *
 * \section s_process_unix_fork Unix Functor-Based
 *
 * Here is a thread-like process class which, naturally, only works on systems
 * with a fork() call.
 *
 * \include process/unix_fork.cpp
 *
 * \section s_process_unix_hooks Unix Hooks
 *
 * Note that the Unix version of redirected_process is implemented exactly like
 * this so there is no need to use special hooks for doing i/o redirection.  In
 * other words, unix_fork can take the redirect_* classes as the hook argument.
 *
 * \include process/unix_hooks.cpp
 *
 * \section s_process_unix_named Unix Named Process
 *
 * Due to the way fork works, Unix can offer some more advanced features with
 * launching process.
 *
 * A failure functor can be provided for the named process which runs
 * if exec fails.  It returns with exit status of the process.  This is useful
 * if you are completely sure of the return statuses the process will exit with
 * as it avoids doing the expensive checking that the portable named process
 * does.
 *
 * TODO:
 *   Document usage of redirection with a unix named process.  This is relevant
 *   because the usage might change due to issues of compatibility between hooks
 *   and redirectors.
 *
 * \include process/unix_named_process.cpp
 *
 * \section s_process_advanced Advanced Process Launching
 *
 * It gets more complicated here because we begin to do some meta-programming.
 * The entire API is based on the following tag classes and concept:
 *
 * TODO:
 *   This needs to change a bit wrt the string/array args stuff which I havem't
 *   quite finished implementing yet.  There is a full spec in
 *   TODO.binary-specification so I can just copy that basically.
 *
 *
\code
struct set_env_tag;
struct no_env_tag;

struct platform_bin_tag;
struct path_bin_tag;
struct exact_bin_tag;
struct windows_bin_tag;

struct string_args_tag;
struct array_args_tag;

class binary_details {
  typedef <whatever> bin_tag;
  typedef <whatever> env_tag;
  typedef <whatever> args_tag;

  // depending on args_tag
  typedef <char** or char *> return_type;

  // Receiver interprets this based on bin_tag.
  const char *path();
  return_type args();
  // Only used if env_tag is set_env_tag:
  return_type envs();
};
\endcode
 *
 * The process classes take a BinaryDetails template argument and then delegate
 * to functions which are overloaded on the tag classes.  This means that all
 * these different combinations of searching and environment'ing can be
 * supported using some reasonably simple conversions before the "grunt work" of
 * launching the process actually happens.  (It also means that I don't need to
 * write a billion overloaded constructors and individual binarydetails classes
 * to pass to them.)
 *
 * The *_binary classes are implemented by inheriting a simple base class which
 * deals with the container type and the member variables.  Then the _tag
 * typedefs are set.
 *
 * In the same way, the environment class inherits as base environment container
 * class and additionally inherits an existing binary class by template
 * parameter and finally sets its own tags.  This means that the actual
 * implementation of these classes is very short and simple, if a little
 * repetitive.
 *
 * The upshot of all this is that you can define your own binary details
 * concepts if you need to, so long as you get the tags right.
 *
 * The tags have the following meaning:
 *
 * - set_env_tag -- use envs() to get an array of environment variables.
 * - no_env_tag -- there is no envs() wanted.
 *
 * - path_searched_bin_tag -- search $PATH to find the binary.  This requires
 *   emulation on windows.
 * - exact_bin_tag -- don't do a search; base the binary on the pwd as though it
 *   was opened with fopen().  This has no system dependent implications.
 * - windows_searched_bin_tag -- windows style search: the app's location dir,
 *   system dirs (only exist on windows), the PATH, and finally the pwd.
 *   This requires a lot of emulation on Unix.
 * - platform_searched_bin_tag -- equivalent to path_searched_bin on Unix and
 *   windows_searched_bin on windows.
 *
 * - string_args_tag -- use envs() to get an array of environment variables.
 *   On unix this must be converted to a char*[].
 * - array_args_tag -- args are a null-terminated array of char*s.  On windows
 *   this must be converted to a string.
 *
 *
 * TODO:
 *   again, need to write the string/array args stuff here.
 *
 * For all searches, if the path contains a slash then searching is disabled and
 * also note that this library does not deal with non-portable paths!
 *
 * Note: at the current time, not all of these tags are implemented for both
 * platforms, however, if you use one other than platform_searched_bin_tag which
 * is not implemented on the other platform, even the platform where it is
 * supported will fail to compile (IOW, platform_seached_bin_tag is not a
 * typedef -- it is a completely separate overload).
 *
 * \section s_process_complete_eg Complete Portable Example
 *
 * This example implements using the unix and the win32 API:
 *
\verbatim
cat * | preprocessor | compiler
\endverbatim
 *
 * This stuff is either used in the main binary on unix or the child on win32.
 *
 * \include process/complete_shared.hpp
 *
 * Here is the main binary:
 *
 * \include process/complete_main.cpp
 *
 * Here is the child process on win32 only.
 *
 * \include process/complete_w32child.cpp
 *
 * TODO:
 *   make this work when win32 interface is finished (or when xxsc is finished,
 *   really);
 */


/*!
 * \page pg_lfds Lock-Free Data Structures
 *
 * \section s_lfds_intro Introduction
 *
 * The lfds module aims to provide a generic lock-free library analagous to the
 * standard template library's structures.
 *
 * TODO:
 *   this.
 *
 */

/******************
 * Namespace Docs *
 ******************/

/*!
\namespace para::detail
\brief Private implementation which will change often.
*/

/*!
\namespace para
\brief All components of the library.
*/

/*!
\namespace para::process
\brief All components of multi-processing module.
\ingroup grp_proc
*/

 /*!
\namespace para::process::detail
\brief Private implementation.
*/

/**********************
 * Module Definitions *
 **********************/

/*!
 * \defgroup grp_locking Locking
 * \brief Locking patterns based on blocking mutex and lock concepts.
*/

/*!
 * \defgroup grp_access Locked Access
 * \ingroup grp_locking
 * \brief Data access via restricted locking interfaces.
 */

 /*!
 * \defgroup grp_sync_tuples Sync Tuples
 * \ingroup grp_locking
 * \brief Encapsulated synchronisation interfaces.
 */

 /*!
 * \defgroup grp_monitors Monitor Pattern
 * \ingroup grp_locking
 * \brief RAII monitor classes.
 */

/*!
 * \defgroup grp_proc Multi-Processing
 * \brief Launching processes in a boost::thread like interface.
 *
 * See the documentation for the class fork and named_process to get started.
 */

/*!
 * \defgroup grp_lfds Lock-Free Data Structures
 * \brief Generic lock-free structures.
 */

#error This file is just for documentation.
