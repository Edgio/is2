is2 
========

Event driven http(s) API server library. [Docs](https://verizondigital.github.io/is2/ "is2 docs")

## Overview

An reasonably performant event driven http(s) API server library written in C++.

### Supports
- threading
- [r3](https://github.com/c9s/r3) like url routing with support for slugs 
- URL handler registration inspired by [flask blueprints](https://flask.pocoo.org/docs/0.12/blueprints/)
- tls with [openssl](https://www.openssl.org/)
- handler support for files, proxies, and subrequests.

### Build requirements (tested on Ubuntu 14.04/16.04)
#### Packages
- openssl
- pthread

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

#### Building example
```sh
g++ ./test.cc -lis2 -lssl -lcrypto -lpthread -o test
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

