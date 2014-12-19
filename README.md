kids
====

Kids is a log aggregation system.  

It aggregates messages like [Scribe](https://github.com/facebookarchive/scribe) and its pub/sub pattern is ported from [Redis](http://redis.io/).


Features
--------

* Real-time subscription
* Distributed collection
* Message persistence
* Multithreading
* Redis protocol
* No third-party dependencies


Installation
------------

You need a complier with C++11 support like GCC 4.7 (or later) or [Clang](http://clang.llvm.org).

	./autogen.sh
	./configure # --prefix=/your/path
	make
	make test # optional
	make install

By default, it will be installed to `/usr/local/bin/kids`.   
You can use the `--prefix` option to specify the installation location.  
Run `./configure --help` for more config options.


Quickstart
----------

In the distributed mode, first start kids with the `server.conf`:

	kids -c sample/server.conf

Next, edit `host` and `port` in `networkstore` in `sample/agent.conf` as:

	store network primary {
      host kidsserver;
      port 3388;
    }

Then, run kids with the modified config file:

	kids -c sample/agent.conf
	
Finally, use `publish` command in Redis protocol to send log to kids agent.  
All the log will be resent to your kids server and persistently stored to disk for analysis later.  
You can also use `subscribe` or `psubscribe` in Redis protocol to get real-time log from kids server.
	
Full explanation of config file, see [here](doc/config.md).

You can directly run `kids -c sample/server.conf` on single-server mode without agent, but it is NOT recommended.

Run `kids --help` for more running options.


License
-------

Kids is BSD-licensed, see LICENSE for more details.


FAQ
---

Q: What is the meaning of "kids"?  
A: "kids" is the recursive acronym of "__K__ids __I__s a __D__ata __S__tream".


Architecture
------------

![image](doc/image/arch.jpg)

You can view the Chinese version README [here](README.zh_CN.md)

