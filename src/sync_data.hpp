/*!
\file
\brief Regarding data which must be synchronised over the threads.
*/
#ifndef SYNC_DATA_HPP_te4d67aw
#define SYNC_DATA_HPP_te4d67aw

#include <queue>

#include <para/locking.hpp>
#include <boost/thread.hpp>

namespace {
  typedef para::sync_traits<
    std::queue<void*>,
    boost::mutex,
    boost::mutex::scoped_lock,
    boost::condition_variable
  > traits;
}

typedef traits::monitor_tuple_type sync_queue_type;
sync_queue_type queue;
// TODO: do something about  this global.
bool quitting = false;

// TODO:
//   maybe I could put the whole lot in the a struct and then use boost
//   bind like bind(struct::*push_predicate, *sync.data()).  If such a
//   thing is possible, it is an important pattern which should be an
//   example in the para lib.
//
//   I either put the struct in the sync type or put the sync type in the
//   struct - not sure yet.  I guess it makes sense to put safe stuff in
//   the struct while the unsafe stuff is hidden inside and only accessed
//   using the sync_type.
//
//   This is an important design thing for para, so we'll leave it until
//   I have finished the rest of this stuff.

bool push_continue_predicate() { return queue.data().size() > 10 || quitting; }
bool pop_continue_predicate() { return ! queue.data().empty() || quitting; }



//! \brief Thread-safe wrapper for the queue.
// TODO:
//   How could I make this kind of pattern easier to use?  Monitored<T> would be
//   a bit easier, but not much more...  Perhaps add to the synced struct.  I could
//   even call this sync_data, no?
template<class TupleType>
class queue_pusher {
  private:
    typedef typename TupleType::lockable_type  mutex_type;
    typedef typename mutex_type::scoped_lock   lock_type;
    typedef typename TupleType::condition_type condition_type;

    typedef para::monitor<mutex_type, lock_type, condition_type> monitor_type;

  public:
    queue_pusher(TupleType &sync) : flush_(false), sync_(sync) {}

    void flush_next_push() { flush_ = true; }

    //! \brief Blocking operation to push the buffer.
    void push(void *buffer) {
      //TODO:
      //  actually, it would be less skippy if we iterate the queue and pop
      //  from the *back*, locking and unlocking each time.

      monitor_type monitor(sync_, push_continue_predicate);
      // would be nicer if I could get this by monitor::data().
      typename TupleType::value_type &q = sync_.data();
      if (flush_) {
        q.clear();
      }
      q.push(buffer);
      flush_ = false;
    }

    void *pop() {
      monitor_type monitor m(sync_, pop_continue_predicate);
      return sync_.data().pop();
    }

  private:
    bool flush_;
    // we could easily enouhg carry this by value, and also
    // have the *_continue_predicates as members and use boost
    // bind to sort it out.
    TupleType &sync_;


};

#endif
