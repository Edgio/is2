**is2** library
---------------

**is2** is an event driven http(s) API server library with support for url routing, tls (with openssl), serving static files, proxying, and subrequests.

Architecture Overview
=====================

**is2** starts with a ``server`` object. ``listener`` objects -listening on a particular port with a given scheme (TCP/TLS) can be registered with a ``server`` object.  ``handler`` objects can be routed to the ``listener`` objects at specified urls.

A basic example of instantation/registration:

.. code-block:: c

  // create a TCP listener on port 12345
  ns_is2::lsnr *lsnr =
    new ns_is2::lsnr(12345, ns_is2::SCHEME_TCP);
  
  // create request handler --based on ns_is2::default_rqst_h
  ns_is2::rqst_h *rqst_h = new request_handler();

  // specify route to request handler on the listener
  lsnr->add_route("/path1/path2", rqst_h);  

  // create a server object
  ns_is2::srvr *srvr = new ns_is2::srvr();

  // register listener with server object
  srvr->register_lsnr(lsnr);


The Reactor
===========

**is2** has an `event system <https://github.com/VerizonDigital/is2/tree/master/src/evr>`_ similar to `libevent <http://libevent.org/>`_ or `libuv <http://libuv.org/>`_.  The reactor is a std::priority_queue of timer events with inverted ordering, for dequeue'ing the next nearest timeout.  The "next nearest timeout" is used for the subsequent call to wait for events (``epoll_wait``, ``select``, ``kqueue``).

Reactor loop psuedo code:

.. code-block:: c

  while true:

    // dequeue / handle timer events until timer > now
    do 
      last_timer = timer_queue.dequeue
    while(last_timer < now)

    // wait for an event -up to the next timer event
    events = wait_event(last_timer)

    // handle events if any
    for each event
      handle (readable/writeable/etc)


