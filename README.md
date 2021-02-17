![is2-ci](https://github.com/VerizonDigital/is2/workflows/is2-ci/badge.svg)

# is2
> _Event driven http(s) API server library -in C++_


## Table of Contents

- [Background](#background)
- [Install](#install)
- [Usage](#usage)
- [Contribute](#contribute)
- [License](#license)

## Background

### Abstract
`is2` is a reasonably performant event driven http(s) API server library written in C++ with support for url routing, tls (with openssl), serving static files, proxying, and subrequests.

### Supports
- threading
- [r3](https://github.com/c9s/r3) like url routing with support for slugs 
- URL handler registration inspired by [flask blueprints](https://flask.pocoo.org/docs/0.12/blueprints/)
- tls with [openssl](https://www.openssl.org/)
- handler support for files, proxies, and subrequests.

### Architecture Overview

- `is2` starts with a `server` object.
- `listener` objects -listening on a particular port with a given scheme (TCP/TLS) can be registered with a `server` object.
- `handler` objects can be routed to the `listener` objects at specified urls.

A basic example of instantation/registration:

```cpp
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
```
### The Reactor

`is2` has an [event system](https://github.com/VerizonDigital/is2/tree/master/src/evr)_ similar to [libevent](http://libevent.org/) or [libuv](https://libuv.org/).  The reactor is a `std::priority_queue` of timer events with inverted ordering, for dequeue'ing the next nearest timeout.  The "next nearest timeout" is used for the subsequent call to wait for events (`epoll_wait`, `select`, `kqueue`).

Reactor loop psuedo code:

```cpp
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
```

## Install

### Build requirements (tested on Ubuntu 16.04/18.04/20.04)

#### Packages
- openssl
- pthread

#### Building example
```sh
g++ ./test.cc -lis2 -lssl -lcrypto -lpthread -o test
```

## Usage

### Basic example

#### Code
```c++
#include <is2/srvr/srvr.h>
#include <is2/srvr/lsnr.h>
#include <is2/srvr/default_rqst_h.h>
#include <is2/srvr/api_resp.h>

// define handler
class base_handler: public ns_is2::default_rqst_h
{
public:
        // GET
        ns_is2::h_resp_t do_get(ns_is2::session &a_session,
                                ns_is2::rqst &a_rqst,
                                const ns_is2::url_pmap_t &a_url_pmap)
        {
                // send json response
                return send_json_resp(a_session,
                                      true,
                                      ns_is2::HTTP_STATUS_OK,
                                      "{\"msg\": \"Hello World\"}");
        }
};

int main(void)
{
        // create server
        ns_is2::srvr *l_srvr = new ns_is2::srvr();

        // set server name -for server response
        l_srvr->set_server_name("hello world server");

        // create listener
        ns_is2::lsnr *l_lsnr = new ns_is2::lsnr(12345, ns_is2::SCHEME_TCP);

        // create handler
        ns_is2::rqst_h *l_rqst_h = new base_handler();

        // add route to listener
        l_lsnr->add_route("/hello", l_rqst_h);

        // register listener with server
        l_srvr->register_lsnr(l_lsnr);

        // num_threads == 0 means run single thread in foreground
        l_srvr->set_num_threads(0);
        l_srvr->run();
        
        // cleanup
        if(l_srvr) {delete l_srvr; l_srvr = NULL;}
        if(l_rqst_h) {delete l_rqst_h; l_rqst_h = NULL;}
        return 0;
}
```

#### Running
```sh
./test
```

#### Testing
```sh
>curl 'http://127.0.0.1:12345/hello' -v
*   Trying 127.0.0.1...
* Connected to 127.0.0.1 (127.0.0.1) port 12345 (#0)
> GET /hello HTTP/1.1
> Host: 127.0.0.1:12345
> User-Agent: curl/7.47.0
> Accept: */*
> 
< HTTP/1.1 200 OK
< Connection: keep-alive
< Content-Length: 22
< Content-type: application/json
< Date: Thu, 15 Feb 2018 00:42:40 GMT
< Server: hello world server
< 
* Connection #0 to host 127.0.0.1 left intact
{"msg": "Hello World"}
```

#### Performance with [hurl](https://github.com/VerizonDigital/hurl)
```sh
>hurl 'http://127.0.0.1:12345/hello' -t1 -p100 -l5
Running 1 threads 100 parallel connections per thread with infinite requests per connection
+-----------/-----------+-----------+-----------+--------------+-----------+-------------+-----------+
| Completed / Requested |    IdlKil |    Errors | kBytes Recvd |   Elapsed |       Req/s |      MB/s |
+-----------/-----------+-----------+-----------+--------------+-----------+-------------+-----------+
|     58456 /     58556 |         0 |         0 |     11064.94 |     1.00s |   62255.49s |    10.81s |
|     89685 /     89785 |         0 |         0 |     11100.93 |     1.50s |   62458.00s |    10.84s |
|    120803 /    120903 |         0 |         0 |     11061.48 |     2.00s |   62236.00s |    10.80s |
|    151836 /    151936 |         0 |         0 |     11031.26 |     2.50s |   62066.00s |    10.77s |
|    182631 /    182731 |         0 |         0 |     10946.66 |     3.00s |   61590.00s |    10.69s |
|    213663 /    213763 |         0 |         0 |     11030.91 |     3.50s |   62064.00s |    10.77s |
|    244744 /    244844 |         0 |         0 |     11048.32 |     4.00s |   62162.00s |    10.79s |
|    275814 /    275914 |         0 |         0 |     11044.41 |     4.50s |   62140.00s |    10.79s |
|    306890 /    306990 |         0 |         0 |     11046.55 |     5.00s |   62152.00s |    10.79s |
| RESULTS:             ALL
| fetches:             306890
| max parallel:        100
| bytes:               7.949221e+07
| seconds:             5.001000
| mean bytes/conn:     259.025090
| fetches/sec:         61365.726855
| bytes/sec:           1.589526e+07
| HTTP response codes: 
| 200 -- 306890
```


## Contribute

- We welcome issues, questions and pull requests.


## License

This project is licensed under the terms of the Apache 2.0 open source license. Please refer to the `LICENSE-2.0.txt` file for the full terms.
