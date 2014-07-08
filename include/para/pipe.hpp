// Copyright (C) 2008-2009, James Webber.
// Distributed under a 3-clause BSD license.  See COPYING.

/*!
\file
\brief Multi-threaded pipe-framework.
*/

/*
TODO:
  We aim for

  stage(prev, next)
    x = pipe.pop
    process(x)
    pipe.push(y)
  end

  Here's how:

  typedef std::queue<T> container_type; // or
  typedef para::lfds::queue<T> container_type;
  typedef unlocked_affector affector_type;
  typedef monitor_affector affector_type;
  typedef pipe<container_type, affector_type> pipe_type;

  Tricky things:
  - flushing the pipe
  - being ok if nothing is returned
    - we will use the same interface as lfds I suppose.
  - checking attributes of the pipe
  - limit the size of a pipe

  Extensions:
  - multiple people taking the same thing from one pipe
    - needs a different pipe type I think.
*/

/*
Flushing the pipe:

Method 1:
- at the start of each stage monitor a `flush' variable.
- stage must check the flushing status (they will prolly
  need to check it anyway).
- flush monitor could be done lock-free maybe?  It should
  only be a boolean.
- the trick is *stopping* processing while we flush rather
  than actually notifying that the flush has happened.

Method 2:
- like method 1 but every stage has its own flush/pause
  variable.
*/


/*
Branched stages.  One stage outputs to multiple pipes. (Of course
taking from multiple pipes is easy).

Method 1:
- insert a coppier stage which writes to multiple pipes.
*/

